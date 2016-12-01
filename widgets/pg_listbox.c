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

#define __PG_LISTBOX_C__
#define __PG_C__

#include <string.h>
#include "pg_syntax.h"
#include "pg_config.h"
#include "pg_primitives.h"
#include "pg.h"
#include "pg_res.h"
#include "pg_mempool.h"
#include "pg_listbox.h"

static void def_printer( pg_obj_t *slot, int item){;}

static const pg_listbox_t def_listbox =
{
    .obj.type = PG_O_HLISTBOX,
    .obj.theme = &pg_def_theme,
    .obj.flags = PG_F_WRAP,
    .nslots = 1, .nitems = 1, .sel_item = 0, .print_hook = def_printer,
};

static const lb_init_t def_init =
{
    .slot_type = PG_O_TXTLAB,
    .lmarg = 2, .rmarg = 2,
    .tmarg = 2, .bmarg = 2,
    .slot_len = 11, .slot_pad = 1,
};

pg_obj_t *pg_listbox_create( void)
{
    pg_obj_t *me = (pg_obj_t *) pg_alloc( sizeof( pg_listbox_t));     // Allocate memory from pool
    if (!me)
        return 0;
    memcpy( me, &def_listbox, sizeof( pg_listbox_t));                  // Just allocate, do not create slots
    pg_listbox_t *lb = (pg_listbox_t *) me;
    lb->init = (lb_init_t *) pg_alloc( sizeof( lb_init_t));
    if (!lb->init)
    {
        pg_free( me);
        return 0;
    }
    memcpy( lb->init, &def_init, sizeof( lb_init_t));                   // Allocate temp storage to store init data
    return me;
}

void pg_listbox_del( pg_obj_t *me)
{
    pg_listbox_t *lb = (pg_listbox_t*) me;
    if ( lb->init)
        pg_free( lb->init);
}

static void init_scroller( pg_listbox_t *lb)
{
    if ( !lb->scroller)
        return;

    pg_set_via_tags( lb->scroller, PG_A_NITEMS, lb->nitems, PG_A_CITEM, lb->sel_item, PG_A_PAGELEN, lb->nslots);
}

static void upd_scroller(  pg_listbox_t *lb)
{
    if (!lb->scroller)
        return;

    pg_set( lb->scroller, PG_A_CITEM, lb->sel_item);
}

/*-----------------------------------------------
    Set
-----------------------------------------------*/
static void reinit_slots( pg_listbox_t *lb, pg_ctaglist_p tl )
{
    pg_obj_t *slot_p = lb->obj.first_child;
    for ( int slot = 0; slot < lb->nslots; slot++, slot_p = slot_p->next_bro)
    {
        if ( tl)
            pg_set_via_taglist( slot_p, tl);

        pg_set( slot_p, PG_A_HL, slot != lb->sel_item - lb->top_item ? 0: 1);
        lb->print_hook( slot_p, lb->top_item + slot);
    }
}

static void print_slots( pg_listbox_t *lb)
{
    pg_obj_t *slot_p = lb->obj.first_child;
    for ( int slot = 0; slot < lb->nslots; slot++, slot_p = slot_p->next_bro)
    {
        pg_set( slot_p, PG_A_HL, slot != lb->sel_item - lb->top_item ? 0: 1);
        lb->print_hook( slot_p, lb->top_item + slot);
    }
}

static void set_selection( pg_listbox_t *lb, int new_item)
{
    if (lb->obj.flags & PG_F_WRAP)
        new_item = ((new_item % lb->nitems)+ lb->nitems) % lb->nitems;  // modulo tricks
    else
        new_item = limit32(new_item, 0, lb->nitems - 1);      // Trim

    int prev_item = lb->sel_item;                           // Previous selected item

    if ( new_item == prev_item)                           // Nothing changed
        return;

    int top_item = lb->top_item;                          // Top (first) item in page
    int bot_item = top_item + lb->nslots - 1;             // Bottom (last) item in page

    lb->sel_item = new_item;                                // Change

    if ( new_item >= top_item && new_item <= bot_item)      // Page unchanged
    {
        // Traverse slot list and apply two hooks( unselect prev, select new)
        uint todo = 2;
        pg_obj_t *slot = lb->obj.first_child;
        for ( int cur_item = lb->top_item; cur_item <= bot_item && todo; cur_item++, slot = slot->next_bro)
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
    else    // Do scroll
    {
        lb->top_item = new_item < prev_item ? new_item : new_item - lb->nslots + 1;
        print_slots( lb);
//        init_scroller( Lb);
    }
    upd_scroller( lb);
}

// Slot creation
void pg_vlistbox_init( pg_obj_t *me)
{
    pg_listbox_t *lb = (pg_listbox_t*) me;
    lb_init_t *init = lb->init;

    int y1 = lb->obj.area.bot - lb->obj.area.top - init->bmarg;

    pg_fast_area_t coords =
    {
        init->lmarg,
        init->tmarg,
        lb->obj.area.right - lb->obj.area.left - init->rmarg,
        init->tmarg + init->slot_len - 1,
    };

    int slot = 0;
    int step = init->slot_len + init->slot_pad;

    while ( slot < lb->nslots && coords.bot <= y1)
    {
        pg_create( init->slot_type, me, coords.left, coords.top, coords.right, coords.bot, init->init_slot_tags) ;
        slot++;
        coords.top += step;
        coords.bot += step;
    }
    lb->nslots = slot;     // Optionally trim
    lb->sel_item = MAX(MIN( lb->sel_item, lb->nitems - 1), 0);

    if ( lb->sel_item >= lb->nslots)    // Scroll to item
    {
        lb->top_item = MAX(MIN( lb->sel_item, lb->nitems - lb->nslots), 0);
    }
    print_slots( lb);
    init_scroller( lb);
    pg_free( lb->init);             // Dealloc temp storage
    lb->init = 0;
}


void pg_hlistbox_init( pg_obj_t *me)
{
    pg_listbox_t *lb = (pg_listbox_t*) me;
    lb_init_t *init = lb->init;

    int x1 = lb->obj.area.right - lb->obj.area.left - init->rmarg;

    pg_fast_area_t coords =
    {
        init->lmarg,
        init->tmarg,
        init->lmarg + init->slot_len - 1,
        lb->obj.area.bot - lb->obj.area.top - init->bmarg,
    };

    int slot = 0;
    int step = init->slot_len + init->slot_pad;

    while ( slot < lb->nslots && coords.right <= x1)
    {
        if (init->init_slot_tags)
        pg_create( init->slot_type, me, coords.left, coords.top, coords.right, coords.bot, init->init_slot_tags) ;
        slot++;
        coords.left += step;
        coords.right += step;
    }
    lb->nslots = slot;     // Optionally trim
    lb->sel_item = MIN( lb->sel_item, lb->nitems - 1);

    if ( lb->sel_item >= lb->nslots)    // Scroll to item
    {
        lb->top_item = MIN( lb->sel_item, lb->nitems - lb->nslots);
    }
    print_slots( lb);
    init_scroller( lb);
    pg_free( lb->init);             // Dealloc temp storage
    lb->init = 0;
}

int pg_listbox_set( pg_obj_t *me, pg_id_t id, pg_val_t val)
{
    pg_listbox_t *lb = (pg_listbox_t*) me;

    switch ( id)
    {
    case PG_IA_LMARG:
        lb->init->lmarg = val;
        return 1;

    case PG_IA_TMARG:
        lb->init->tmarg = val;
        return 1;

    case PG_IA_RMARG:
        lb->init->rmarg = val;
        return 1;

    case PG_IA_BMARG:
        lb->init->bmarg = val;
        return 1;

    case PG_IA_NSLOTS:
        lb->nslots = MAX( val, 1);   // Safe limit number of slots
        return 0;

    case PG_IA_SLOT_LEN:
        lb->init->slot_len = val;
        return 0;

    case PG_IA_SLOT_PAD:
        lb->init->slot_pad = val;
        return 0;

    case PG_A_SCROLLER:
        lb->scroller = (pg_obj_t *) val;
        init_scroller( lb);
        return 0;

    case PG_A_NITEMS:
        // !!! May cause out of range printing if C_ITEM setted before N_ITEMS. Same for matrix
        if (val == lb->nitems) return 0;
        if (val < lb->nitems)   // shrink of list
        {
            lb->sel_item = UMIN( lb->sel_item, val - 1);                    // Safe-trim
            lb->top_item = UMIN( lb->top_item, val - 1);                    // Safe-trim
            lb->nitems = val;
        }
        else
        {
            lb->nitems = val;
        }
        if ( is_inited( me))
        {
            print_slots( lb);                                           // Do not pass taglist
            init_scroller( lb);
        }
        return 0;

    case PG_A_CITEM:
        if ( is_inited( me))
            set_selection( lb, val);
        else
            lb->sel_item = val;
        return 0;

    case PG_IA_SLOT_TYPE:
        lb->init->slot_type = (enum pg_objs_e)val;
        return 0;

    case PG_A_SLOT_TAGLIST:
        if ( !is_inited( me))
        {
            lb->init->init_slot_tags = (pg_ctaglist_p) val;
        }
        else
        {
            reinit_slots( lb, (pg_ctaglist_p) val);
        }
        return 0;

    case PG_A_WRAP_CURSOR:
        apply_flag32( &me->flags, PG_F_WRAP, val);
        return 0;

    case PG_A_UPD_HOOK:
        lb->print_hook = (pg_updhook_t) val;
        if ( is_inited( me))
            print_slots( lb);
        return 0;
    }
    return pg_base_set( me,id,val);
}


int pg_listbox_get( pg_obj_t *me, pg_id_t id, pg_val_t *val)
{
    const pg_listbox_t *lb = (pg_listbox_t*) me;

    switch ( id)
    {
    case PG_IA_NSLOTS:           *val = lb->nslots;                    return 1;
    case PG_A_NITEMS:            *val = lb->nitems;                    return 1;
    case PG_A_CITEM:             *val = lb->sel_item;                     return 1;
    case PG_A_UPD_HOOK:          *val = (pg_val_t) lb->print_hook;     return 1;
    case PG_A_SCROLLER:          *val = (pg_val_t) lb->scroller;      return 1;
    case PG_A_WRAP_CURSOR:       *val = me->flags & PG_F_WRAP;             return 1;
    }
    return pg_base_get( me,id,val);
}

static void redraw_item( pg_listbox_t *lb, int item)
{
    int top_item = lb->top_item;                  // Top (first) item in page
    int bot_item = top_item + lb->nslots - 1;     // Bottom (last) item in page

    if ( item < top_item || item > bot_item)      // Item is from another page, ignore
        return;

    pg_obj_t *slot = lb->obj.first_child;
    for ( int cur_item = lb->top_item; cur_item <= bot_item; cur_item++, slot = slot->next_bro)
    {
        if ( cur_item == item)
        {
            lb->print_hook( slot, cur_item);
            return;
        }
    }
}

void pg_listbox_act( pg_obj_t *me, pg_id_t id, pg_val_t val)
{
    pg_listbox_t *lb = (pg_listbox_t*) me;

    switch ( id)
    {
    case PG_M_CHANGE_SEL:
        set_selection( lb, lb->sel_item + val);
        return;

    case PG_M_REDRAW_ITEM:
        redraw_item( lb, val);
        return;

    case PG_M_REDRAW_PAGE:
        print_slots( lb);
        return;
    }
    pg_base_act( me,id,val);
}
