/*
PG embedded graphic library

Copyright (c) 2009 jkmnt

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#define __PG_MATRIX_C__
#define __PG_C__

#include <string.h>
#include "pg_syntax.h"
#include "pg_config.h"
#include "pg_primitives.h"
#include "pg.h"
#include "pg_res.h"
#include "pg_mempool.h"
#include "pg_matrix.h"

static void def_printer( pg_obj_t *slot, int item){;}

static const pg_matrix_t def_matrix =
{
    .obj.type = PG_O_MATRIX,
    .obj.theme = &pg_def_theme,
    .obj.flags = PG_F_MATRIX_SCROLL,
    .ncols = -1, .nrows = -1,   // Unlim for default
    .nitems = 1, .sel_item = 0, .print_hook = def_printer,
};

static const mx_init_t def_init =
{
    .slot_type = PG_O_TXTLAB,
    .lmarg = 2, .rmarg = 2,
    .tmarg = 2, .bmarg = 2,
    .slot_w = 11, .slot_h = 11,
    .slot_xpad = 1, .slot_ypad = 1,
};

pg_obj_t *pg_matrix_create( void)
{
    pg_obj_t *me = (pg_obj_t *) pg_alloc( sizeof( pg_matrix_t));   // Allocate memory from pool
    if (!me)
        return 0;
    memcpy( me, &def_matrix, sizeof( pg_matrix_t));                 // Just allocate, do not create slots
    pg_matrix_t *mx = (pg_matrix_t *) me;
    mx->init = (mx_init_t *) pg_alloc( sizeof( mx_init_t));
    if (!mx->init)
    {
        pg_free( me);
        return 0;
    }
    memcpy( mx->init, &def_init, sizeof( mx_init_t));               // Allocate temp storage to store init data
    return me;
}

void pg_matrix_del( pg_obj_t *me)
{
    pg_matrix_t *mx = (pg_matrix_t*) me;
    if ( mx->init)
        pg_free( mx->init);
}

static void init_scroller( pg_matrix_t *mx)
{
    if ( !mx->scroller)
        return;

    pg_set_via_tags( mx->scroller, PG_A_NITEMS, (mx->nitems + mx->ncols)/ mx->ncols,
                                   PG_A_CITEM, mx->sel_item / mx->ncols,
                                   PG_A_PAGELEN, mx->nrows);
}

static void upd_scroller(  pg_matrix_t *mx)
{
    if (!mx->scroller)
        return;

    pg_set( mx->scroller, PG_A_CITEM, mx->sel_item / mx->ncols);
}

/*-----------------------------------------------
    Set
-----------------------------------------------*/
static void reinit_slots( pg_matrix_t *mx, pg_ctaglist_p tl )
{
    pg_obj_t *slot_p = mx->obj.first_child;
    int nslots = mx->nrows * mx->ncols;
    int top_item = mx->top_row * mx->ncols;
    for ( int slot = 0; slot < nslots; slot++, slot_p = slot_p->next_bro)
    {
        if ( tl)
            pg_set_via_taglist( slot_p, tl);

        pg_set( slot_p, PG_A_HL, slot != mx->sel_item - top_item ? 0: 1);
        mx->print_hook( slot_p, top_item + slot);
    }
}

static void print_slots( pg_matrix_t *mx)
{
    pg_obj_t *slot_p = mx->obj.first_child;
    int nslots = mx->nrows * mx->ncols;
    int top_item = mx->top_row * mx->ncols;
    for ( int slot = 0; slot < nslots; slot++, slot_p = slot_p->next_bro)
    {
        pg_set( slot_p, PG_A_HL, slot != mx->sel_item - top_item ? 0: 1);
        mx->print_hook( slot_p, top_item + slot);
    }
}

static void set_selection( pg_matrix_t *mx, int new_item)
{
    new_item = limit32(new_item, 0, mx->nitems - 1);      // Trim

    int prev_item = mx->sel_item;                           // Previous selected item

    if ( new_item == prev_item)                           // Nothing changed
        return;

    int nslots = mx->nrows * mx->ncols;
    int top_item = mx->top_row * mx->ncols;               // Top (first) item in page

    int bot_item = top_item + nslots - 1;                 // Bottom (last) item in page

    mx->sel_item = new_item;                                // Change

    if ( new_item >= top_item && new_item <= bot_item)      // Page unchanged
    {
        // Traverse slot list and apply two hooks( unselect prev, select new)
        uint todo = 2;
        pg_obj_t *slot = mx->obj.first_child;
        for ( int cur_item = top_item; cur_item <= bot_item && todo; cur_item++, slot = slot->next_bro)
        {
            if ( cur_item == new_item)
            {
                pg_set( slot, PG_A_HL, 1);
                todo--;
            }
            else if ( cur_item == prev_item)
            {
                pg_set( slot, PG_A_HL, 0);
                todo--;
            }
        }
    }
    else    // Do a page scroll here
    {
        if ( mx->obj.flags & PG_F_MATRIX_SCROLL)
        {
            mx->top_row = new_item < prev_item ? new_item / mx->ncols : new_item / mx->ncols - mx->nrows + 1;
        }
        else
        {
            mx->top_row = new_item / nslots * mx->nrows;
        }
        print_slots( mx);
    }
    upd_scroller( mx);
}

void pg_matrix_init( pg_obj_t *me)
{
    pg_matrix_t *mx = (pg_matrix_t*) me;
    mx_init_t *init = mx->init;

    int y1 = mx->obj.area.bot - mx->obj.area.top - init->bmarg;
    int x1 = mx->obj.area.right - mx->obj.area.left - init->rmarg;

    pg_fast_area_t coords =
    {
        init->lmarg,
        init->tmarg,
        init->lmarg + init->slot_w - 1,
        init->tmarg + init->slot_h - 1 ,
    };

    int col = 0;
    int row = 0;
    int xstep = init->slot_w + init->slot_xpad;
    int ystep = init->slot_h + init->slot_ypad;

    for ( row = 0; row < mx->nrows && coords.bot <= y1; row++, coords.top += ystep, coords.bot += ystep)            // Row loop
    {
        coords.left = init->lmarg;
        coords.right = init->lmarg + init->slot_w - 1;
        for ( col = 0; col < mx->ncols && coords.right <= x1; col++, coords.left += xstep, coords.right += xstep)   // Col loop
        {
            if (init->init_slot_tags)
                pg_create( init->slot_type, me, coords.left, coords.top, coords.right, coords.bot, init->init_slot_tags) ;
            }
            }

    mx->ncols = col;
    mx->nrows = row;

    mx->sel_item = MIN( mx->sel_item, mx->nitems - 1);
    if ( me->flags & PG_F_MATRIX_SCROLL)
    {
        mx->top_row = mx->sel_item / col;
    }
    else
    {
        mx->top_row = mx->sel_item / mx->ncols / mx->nrows * mx->nrows;
    }
    print_slots( mx);
    init_scroller( mx);
    pg_free( mx->init);             // Dealloc temp storage
    mx->init = 0;
}


int pg_matrix_set( pg_obj_t *me, pg_id_t id, pg_val_t val)
{
    pg_matrix_t *mx = (pg_matrix_t*) me;

    switch ( id)
    {
    case PG_IA_LMARG:
        mx->init->lmarg = val;
        return 1;

    case PG_IA_TMARG:
        mx->init->tmarg = val;
        return 1;

    case PG_IA_RMARG:
        mx->init->rmarg = val;
        return 1;

    case PG_IA_BMARG:
        mx->init->bmarg = val;
        return 1;

    case PG_IA_NCOLS:
        mx->ncols = MAX( val, 1);   // Safe limit number of columns
        return 0;

    case PG_IA_NROWS:
        mx->nrows = MAX( val, 1);   // Safe limit number of rows
        return 0;

    case PG_IA_SLOT_W:
        mx->init->slot_w = val;
        return 0;

    case PG_IA_SLOT_H:
        mx->init->slot_h = val;
        return 0;

    case PG_IA_SLOT_XPAD:
        mx->init->slot_xpad = val;
        return 0;

    case PG_IA_SLOT_YPAD:
        mx->init->slot_ypad = val;
        return 0;

    case PG_A_NITEMS:
        mx->nitems = val;
        mx->sel_item = UMIN( mx->sel_item, val - 1);                    // Safe-trim
        if ( me->flags & PG_F_MATRIX_SCROLL)
        {
            mx->top_row = mx->sel_item / mx->ncols;
        }
        else
        {
            mx->top_row = mx->sel_item / mx->ncols / mx->nrows * mx->nrows;
        }
        if ( is_inited( me))
        {
            print_slots( mx);
            init_scroller( mx);
        }
        return 0;

    case PG_A_CITEM:
        if ( is_inited( me))
            set_selection( mx, val);
        else
            mx->sel_item = val;
        return 0;

    case PG_IA_SLOT_TYPE:
        mx->init->slot_type = (enum pg_objs_e)val;
        return 0;

    case PG_A_SLOT_TAGLIST:
        if ( !is_inited( me))
        {
            mx->init->init_slot_tags = (pg_ctaglist_p) val;
        }
        else
        {
            reinit_slots( mx, (pg_ctaglist_p) val);
        }
        return 0;

    case PG_A_UPD_HOOK:
        mx->print_hook = (pg_updhook_t) val;
        if ( is_inited( me))
            print_slots( mx);
        return 0;

    case PG_A_WRAP_CURSOR:
        apply_flag32( &me->flags, PG_F_MATRIX_WRAP, val);
        return 0;

    case PG_A_USE_SCROLLING:
        return apply_flag32( &me->flags, PG_F_MATRIX_SCROLL, val);

    case PG_A_SCROLLER:
        mx->scroller = (pg_obj_t *) val;
        init_scroller( mx);
        return 0;
    }
    return pg_base_set( me,id,val);
}

int pg_matrix_get( pg_obj_t *me, pg_id_t id, pg_val_t *val)
{
    pg_matrix_t *mx = (pg_matrix_t*) me;

    switch ( id)
    {
    case PG_ROA_ROW:             *val = mx->sel_item / mx->ncols;         return 1;
    case PG_ROA_COL:             *val = mx->sel_item % mx->ncols;         return 1;
    case PG_IA_NROWS:            *val = mx->nrows;                     return 1;

    case PG_IA_NCOLS:            *val = mx->ncols;                     return 1;
    case PG_A_NITEMS:            *val = mx->nitems;                    return 1;
    case PG_A_CITEM:             *val = mx->sel_item;                     return 1;
    case PG_A_UPD_HOOK:          *val = (pg_val_t) mx->print_hook;     return 1;
    case PG_A_WRAP_CURSOR:       *val = me->flags & PG_F_MATRIX_WRAP;       return 1;
    case PG_A_USE_SCROLLING:     *val = me->flags & PG_F_MATRIX_SCROLL;           return 1;
    case PG_A_SCROLLER:          *val = (pg_val_t) mx->scroller;      return 1;
    }
    return pg_base_get( me,id,val);
}

static void redraw_item( pg_matrix_t *mx, int item)
{
    int nslots = mx->nrows * mx->ncols;
    int top_item = mx->top_row * mx->ncols;               // Top (first) item in page

    int bot_item = top_item + nslots - 1;                 // Bottom (last) item in page

    if ( item < top_item || item > bot_item)              // Item is from another page, ignore
        return;

    pg_obj_t *slot = mx->obj.first_child;
    for ( int cur_item = top_item; cur_item <= bot_item; cur_item++, slot = slot->next_bro)
    {
        if ( cur_item == item)
        {
            mx->print_hook( slot, cur_item);
            return;
        }
    }
}

void pg_matrix_act( pg_obj_t *me, pg_id_t id, pg_val_t val)
{
    pg_matrix_t *mx = (pg_matrix_t*) me;

    switch ( id)
    {
    case PG_M_CHANGE_SEL:
        set_selection( mx, mx->sel_item + val);
        return;

    case PG_M_REDRAW_ITEM:
        redraw_item( mx, val);
        return;

    case PG_M_REDRAW_PAGE:
        print_slots(mx);
        return;

    case PG_M_NEXT_ROW:
        {
            int new_item = mx->sel_item + mx->ncols;

            if ( new_item >= mx->nitems )
            {
                if ( ! (me->flags & PG_F_MATRIX_WRAP))
                    return;

                new_item = mx->sel_item % mx->ncols;
            }
            set_selection( mx, new_item);
        }
        return;

    case PG_M_PREV_ROW:
        {
            int new_item = mx->sel_item - mx->ncols;
            if ( new_item < 0)
            {
                if ( ! (me->flags & PG_F_MATRIX_WRAP))
                    return;

                int last_row = (mx->nitems - 1)/ mx->ncols;
                new_item = mx->sel_item % mx->ncols + last_row * mx->ncols;
                if ( new_item >= mx->nitems)     // Last raw have no such item, step to prev
                    new_item -= mx->ncols;

            }
            set_selection( mx, new_item);
        }
        return;


    case PG_M_NEXT_COL:
        {
            int old_col = mx->sel_item % mx->ncols;
            int new_item = mx->sel_item + 1;
            if ( old_col >= (mx->ncols - 1) || new_item >= mx->nitems )
            {
                if ( ! (me->flags & PG_F_MATRIX_WRAP))
                    return;

                new_item = mx->sel_item / mx->ncols * mx->ncols;
            }
            set_selection( mx, new_item);
        }
        return;

    case PG_M_PREV_COL:
        {
            int old_col = mx->sel_item % mx->ncols;
            int new_item = mx->sel_item - 1;
            if ( old_col <= 0 )
            {
                if ( !(me->flags & PG_F_MATRIX_WRAP))
                    return;

                new_item = mx->sel_item + mx->ncols - 1;
            }
            set_selection( mx, new_item);
        }
        return;

    }
    pg_base_act( me,id,val);
}
