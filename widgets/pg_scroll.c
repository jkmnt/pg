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

#define __PG_SCROLL_C__
#define __PG_C__

#include <string.h>
#include "pg_syntax.h"
#include "pg_config.h"
#include "pg_mempool.h"
#include "pg_primitives.h"
#include "pg.h"
#include "pg_scroll.h"

static const pg_scroll_t def_scroll = { .obj.theme = &pg_def_theme,
                                       .lmarg = 1,
                                       .tmarg = 1,
                                       .rmarg = 1,
                                       .bmarg = 1,
                                       .page_len = 1,
                                       .nitems = 1};

pg_obj_t *pg_scroll_create( void)
{
    pg_obj_t *me = (pg_obj_t *) pg_alloc( sizeof( pg_scroll_t));    // Allocate memory from pool
    if (!me)
        return 0;
    memcpy( me, &def_scroll, sizeof( pg_scroll_t));
    return me;
}

int pg_scroll_set( pg_obj_t *me, pg_id_t id, pg_val_t val)
{
    pg_scroll_t *scroll = (pg_scroll_t*) me;
    switch ( id)
    {
    case PG_A_LMARG:    scroll->lmarg = val;                return 1;
    case PG_A_TMARG:    scroll->tmarg = val;                return 1;
    case PG_A_RMARG:    scroll->rmarg = val;                return 1;
    case PG_A_BMARG:    scroll->bmarg = val;                return 1;

    case PG_A_NITEMS:   scroll->nitems = val;                              return 1;  // Here is gotcha with set order - N_ITEMS must be set first
    case PG_A_CITEM:
        {
            int citem = limit32(val, 0, scroll->nitems - 1);
            if (citem == scroll->citem)
                return 0;
            scroll->citem = citem;
        }
        return 1;

    case PG_A_PAGELEN:  scroll->page_len = MIN( scroll->nitems, val);       return 1;
    }
    return pg_base_set( me,id,val);
}

int pg_scroll_get( pg_obj_t *me, pg_id_t id, pg_val_t *val)
{
    pg_scroll_t *scroll = (pg_scroll_t*) me;
    switch ( id)
    {
    case PG_A_LMARG:    *val = scroll->lmarg;           return 1;
    case PG_A_TMARG:    *val = scroll->tmarg;           return 1;
    case PG_A_RMARG:    *val = scroll->rmarg;           return 1;
    case PG_A_BMARG:    *val = scroll->bmarg;           return 1;
    case PG_A_NITEMS:   *val = scroll->nitems;          return 1;
    case PG_A_CITEM:    *val = scroll->citem;           return 1;
    case PG_A_PAGELEN:  *val = scroll->page_len;         return 1;
    }
    return pg_base_get( me,id,val);
}


void pg_hscroll_rend( pg_obj_t *me)
{
    const pg_scroll_t *scroll = (pg_scroll_t*) me;
    const pg_deco_t *deco = sel_deco( me);
    uint scroll_len = me->area.right - me->area.left - scroll->lmarg - scroll->rmarg + 1;

    uint x0 = me->area.left + scroll->lmarg;
    uint y0 = me->area.top + scroll->tmarg;
    uint y1 = me->area.bot - scroll->bmarg;

    uint nitems = scroll->nitems;
    uint page_len = scroll->page_len;
    uint citem = scroll ->citem;

    int thumb_len =  scroll_len * page_len / nitems;
    thumb_len = MAX( thumb_len, PG_MIN_THUMBLEN);

    int thumb_pos = (scroll_len - thumb_len) * citem / (nitems - 1);

    pg_draw_fill( x0 + thumb_pos, y0, x0 + thumb_pos + thumb_len - 1, y1, deco->fgcol);
}

void pg_vscroll_rend( pg_obj_t *me)
{
    const pg_scroll_t *scroll = (pg_scroll_t*) me;
    const pg_deco_t *deco = sel_deco( me);
    uint scroll_len = me->area.bot - me->area.top - scroll->tmarg - scroll->bmarg + 1;

    uint x0 = me->area.left + scroll->lmarg;
    uint x1 = me->area.right - scroll->rmarg;
    uint y0 = me->area.top + scroll->tmarg;

    uint nitems = scroll->nitems;
    uint page_len = scroll->page_len;
    uint citem = scroll ->citem;

    int thumb_len =  scroll_len * page_len / nitems;
    thumb_len = MAX( thumb_len, PG_MIN_THUMBLEN);

    int thumb_pos = (scroll_len - thumb_len) * citem / (nitems - 1);

    pg_draw_fill( x0, y0 + thumb_pos, x1, y0 + thumb_pos + thumb_len - 1, deco->fgcol);
}
