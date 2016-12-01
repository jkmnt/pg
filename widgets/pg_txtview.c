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

#define __PG_TXTVIEW_C__
#define __PG_C__
#define __PG_PRIMITIVES_C__

#include <string.h>
#include "pg_syntax.h"
#include "pg_config.h"
#include "pg_utf8.h"
#include "pg_primitives.h"
#include "pg.h"
#include "pg_res.h"
#include "pg_mempool.h"
#include "pg_txtview.h"

/** Default txtview params */
static const pg_txtview_t def_view = { .obj.type = PG_O_TXTVIEW,
                                    .obj.theme = &pg_def_theme,
                                    .lmarg = PG_DEF_TXT_LMARG,
                                    .tmarg = PG_DEF_TXT_TMARG,
                                    .rmarg = PG_DEF_TXT_RMARG,
                                    .bmarg = PG_DEF_TXT_BMARG,
                                    };

// determine a starting position of text in page
static void recalc_visible_page(pg_txtview_t *tv)
{
    uint box_w = tv->obj.area.right - tv->obj.area.left - tv->rmarg - tv->lmarg + 1;
    uint tr = tv->top_row;

    pg_line_splitter_iterator_t it;
    pg_line_splitter_init(&it, sel_deco(&tv->obj)->font, tv->text, box_w);

    const utf8 *from = tv->text;    // be on a safe side
    const utf8 *to = tv->text;
    int w = 0;

    for (uint i = 0; i <= tr && w >= 0; i++)
        w = pg_line_splitter_next(&it, &from, &to);

    tv->top_line = (char *)from;    // remember top line start pointer for fast render
}

static void recalc_all(pg_txtview_t *tv)
{
    const uint line_h = sel_deco(&tv->obj)->font->h + tv->leading;
    const uint box_h = tv->obj.area.bot - tv->obj.area.top - tv->bmarg - tv->tmarg + 1;
    const uint box_w = tv->obj.area.right - tv->obj.area.left - tv->rmarg - tv->lmarg + 1;

    //TODO: exclude last leading from height
    const int lpp = box_h / line_h;
    tv->lines_per_page = MAX(lpp, 1);

    pg_line_splitter_iterator_t it;
    pg_line_splitter_init(&it, sel_deco(&tv->obj)->font, tv->text, box_w);

    const utf8 *f;
    const utf8 *t;

    uint nrows = 0;
    while (pg_line_splitter_next(&it, &f, &t) >= 0)
        nrows++;

    tv->nrows = nrows;
    recalc_visible_page(tv);
}

static int change_row(pg_txtview_t *tv, int delta)
{
    int new_row = tv->top_row + delta;
    int max_row = tv->nrows - tv->lines_per_page;
    max_row = MAX(max_row, 0);
    new_row = limit32(new_row, 0, max_row);
    if (new_row == tv->top_row) return 0;
    tv->top_row = new_row;

    recalc_visible_page(tv);
    return 1;
}

pg_obj_t *pg_txtview_create( void)
{
    pg_obj_t *me = (pg_obj_t *) pg_alloc( sizeof( pg_txtview_t));    // Allocate memory from pool
    if (!me)
        return 0;
    memcpy( me, &def_view, sizeof( pg_txtview_t));
    return me;
}

void pg_txtview_init( pg_obj_t *me)
{
    pg_txtview_t *tv = (pg_txtview_t*) me;
    recalc_all(tv);
}

void pg_btxtview_del( pg_obj_t *me)
{
    pg_txtview_t *tv = (pg_txtview_t*) me;
    if ( tv->text)
        pg_free( tv->text);
}

int pg_txtview_set( pg_obj_t *me, pg_id_t id, pg_val_t val)
{
    pg_txtview_t *tv = (pg_txtview_t*) me;
    switch ( id)
    {
    case PG_A_LMARG:
        tv->lmarg = val;
        if (is_inited(me))
            recalc_all(tv);
        return 1;
    case PG_A_TMARG:
        tv->tmarg = val;
        if (is_inited(me))
            recalc_all(tv);
        return 1;
    case PG_A_RMARG:
        tv->rmarg = val;
        if (is_inited(me))
            recalc_all(tv);
        return 1;
    case PG_A_BMARG:
        tv->bmarg = val;
        if (is_inited(me))
            recalc_all(tv);
        return 1;
    case PG_A_LEADING:
        tv->leading = val;
        if (is_inited(me))
            recalc_all(tv);
        return 1;
    case PG_A_TEXT:
        tv->text = (char *) val;
        if (is_inited(me))
            recalc_all(tv);
        return 1;
    case PG_A_THEME:
        pg_base_set(me, id, val);
        if (is_inited(me))
            recalc_all(tv);
        return 1;
    }
    return pg_base_set(me, id, val);
}

int pg_txtview_get( pg_obj_t *me, pg_id_t id, pg_val_t *val)
{
    pg_txtview_t *tv = (pg_txtview_t*) me;
    switch ( id)
    {
    case PG_A_LMARG:       *val = tv->lmarg;              return 1;
    case PG_A_TMARG:       *val = tv->tmarg;              return 1;
    case PG_A_RMARG:       *val = tv->rmarg;              return 1;
    case PG_A_BMARG:       *val = tv->bmarg;              return 1;

    case PG_A_LEADING:     *val = tv->leading;            return 1;
    case PG_A_TEXT:        *val = (pg_val_t) tv->text;   return 1;
    }
    return pg_base_get(me, id, val);
}

int pg_btxtview_set( pg_obj_t *me, pg_id_t id, pg_val_t val)
{
    pg_txtview_t *tv = (pg_txtview_t*) me;
    int rc = 1;

    switch ( id)
    {
    case PG_A_TEXT:
        if ( tv->text)
            pg_free( tv->text);
        tv->text = NULL;
        if (val)
        {
            tv->text = (char *) pg_alloc( strlen( (char *) val)+1);
            if (! tv->text)
            {
                rc = 0;
            }
            else
            {
                strcpy( tv->text, (char *) val);
            }
        }
        if (is_inited(me))
            recalc_all(tv);
        return rc;
    }
    return pg_txtview_set(me, id, val);
}

void pg_txtview_act( pg_obj_t *me, pg_id_t id, pg_val_t val)
{
    pg_txtview_t *tv = (pg_txtview_t*) me;

    switch ( id)
    {
    case PG_M_PREV_ROW:
        if (change_row(tv, -1))
            pg_force_update(me);
        return;

    case PG_M_NEXT_ROW:
        if (change_row(tv, +1))
            pg_force_update(me);
        return;

    case PG_M_REDRAW_PAGE:
        recalc_all(tv);
        pg_force_update(me);
        return;
    }
    pg_base_act( me,id,val);
}

void pg_txtview_rend( pg_obj_t *me)
{
    const pg_txtview_t *tv = (pg_txtview_t*) me;
    const pg_deco_t *deco = sel_deco( me);
    const pg_area_t *area = &me->area;
    int x0 = area->left + tv->lmarg;
    int y0 = area->top + tv->tmarg;
    int x1 = area->right - tv->rmarg;
    int y1 = area->bot - tv->bmarg;

    if ( !tv->text)
        return;

    // trim heigth to the integer number of lines
    int line_h = deco->font->h + tv->leading;
    int box_h = line_h * tv->lines_per_page;

    y1 = MIN( (uint)y1, (uint)y0 + (uint)box_h - 1);

    pg_draw_multiline_text( deco->font, tv->top_line, x0, y0, x1, y1,
    deco->fgcol, -1, -1, tv->leading);
}
