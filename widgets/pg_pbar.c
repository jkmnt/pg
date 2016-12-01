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

#define __PG_PBAR_C__
#define __PG_C__

#include <string.h>
#include "pg_syntax.h"
#include "pg_config.h"
#include "pg_primitives.h"
#include "pg.h"
#include "pg_res.h"
#include "pg_mempool.h"
//#include "pg_txtlabel.h"
#include "pg_pbar.h"

static const pg_bar_t def_bar = { .obj.theme = &pg_def_theme, .lmarg = 1, .tmarg = 1, .rmarg = 1, .bmarg = 1};

pg_obj_t *pg_bar_create( void)
{
    pg_obj_t *me = (pg_obj_t *) pg_alloc( sizeof( pg_bar_t));    // Allocate memory from pool
    if (!me)
        return 0;
    memcpy( me, &def_bar, sizeof( pg_bar_t));
    return me;
}

int pg_bar_set( pg_obj_t *me, pg_id_t id, pg_val_t val)
{
    pg_bar_t *bar = (pg_bar_t*) me;
    switch ( id)
    {
    case PG_A_LMARG:    bar->lmarg = val;    return 1;
    case PG_A_TMARG:    bar->tmarg = val;    return 1;
    case PG_A_RMARG:    bar->rmarg = val;    return 1;
    case PG_A_BMARG:    bar->bmarg = val;    return 1;

    case PG_A_PERCENT:
        {
            int tmp = (uint) val < 100 ? val: 100;
            if ( tmp == bar->percent)
                return 0;
            bar->percent = tmp;
        }
        return 1;
    }
    return pg_base_set( me,id,val);
}

int pg_bar_get( pg_obj_t *me, pg_id_t id, pg_val_t *val)
{
    pg_bar_t *bar = (pg_bar_t*) me;
    switch ( id)
    {
    case PG_A_LMARG:   *val = bar->lmarg;    return 1;
    case PG_A_TMARG:   *val = bar->tmarg;    return 1;
    case PG_A_RMARG:   *val = bar->rmarg;    return 1;
    case PG_A_BMARG:   *val = bar->bmarg;    return 1;

    case PG_A_PERCENT: *val = bar->percent;  return 1;
    }
    return pg_base_get( me,id,val);
}

void pg_hbar_rend( pg_obj_t *me)
{
    const pg_bar_t *bar = (pg_bar_t*) me;
    const pg_deco_t *deco = sel_deco( me);
    uint width = me->area.right - me->area.left - bar->lmarg - bar->rmarg + 1;
    uint len = width * bar->percent / 100;
    if ( !len)
        return;
    uint x0 = me->area.left + bar->lmarg;
    uint y0 = me->area.top + bar->tmarg;
    uint y1 = me->area.bot - bar->bmarg;
    pg_draw_fill( x0, y0, x0 + len -1 , y1, deco->fgcol);
}

/*-----------------------------------------------
    Vertical progress bar renderer
-----------------------------------------------*/
void pg_vbar_rend( pg_obj_t *me)
{
    const pg_bar_t *bar = (pg_bar_t*) me;
    const pg_deco_t *deco = sel_deco( me);
    uint height = me->area.bot - me->area.top - bar->tmarg - bar->bmarg + 1;
    uint len = height * bar->percent / 100;
    if ( !len)
        return;
    uint x0 = me->area.left + bar->lmarg;
    uint x1 = me->area.right - bar->rmarg;
    uint y1 = me->area.bot - bar->tmarg;
    pg_draw_fill( x0, y1 - len + 1, x1, y1, deco->fgcol);
}


// Additional class - bar with text
int pg_tbar_set( pg_obj_t *me, pg_id_t id, pg_val_t val)
{
    pg_bar_t *bar = (pg_bar_t*) me;
    switch ( id)
    {
    case PG_A_PERCENT:
        {
            int tmp = (uint) val < 100 ? val: 100;
            if ( tmp == bar->percent)
                return 0;

            bar->percent = tmp;

            if ( is_inited(me) && !(me->flags & PG_F_CUSTOM_TEXT))
            {
                char buf[sizeof("100%")];
                pg_set( me->first_child, PG_A_TEXT, (pg_val_t) get_text( buf, tmp));
            }
        }
        return 1;

    case PG_A_CUSTOM_TEXT:
        return apply_flag32( &me->flags, PG_F_CUSTOM_TEXT, val);

    case PG_A_TEXT:
        if ( is_inited(me) && (me->flags & PG_F_CUSTOM_TEXT))
            pg_set( me->first_child, PG_A_TEXT, val);
        return 0;

    }

    return pg_bar_set( me, id, val);
}

int pg_tbar_get( pg_obj_t *me, pg_id_t id, pg_val_t *val)
{
    switch ( id)
    {
    case PG_A_CUSTOM_TEXT:
        *val = me->flags & PG_F_CUSTOM_TEXT;
        return 1;
    }

    return pg_bar_get( me, id, val);
}

static char *get_text( char *buf, int perc)
{
    char *inbuf = buf;
    if ( perc >= 100)
    {
        *buf++ = '1';
        *buf++ = '0';
        *buf++ = '0';
    }
    else if ( perc >= 10)
    {
        *buf++ = perc / 10 + '0';
        *buf++ = perc % 10 + '0';
    }
    else
    {
        *buf++ = perc % 10 + '0';
    }
    *buf++ = '%';
    *buf = 0;
    return inbuf;
}

void pg_tbar_init( pg_obj_t *me)
{
    pg_bar_t *bar = (pg_bar_t *) me;
    char buf[sizeof("100%")];

    static const pg_theme_t xor_theme =
    {
        { PG_F_TRANSP, PG_CL_WHITE, PG_CL_XOR, PG_CL_BLACK, &pg_unifont},
        { PG_F_TRANSP, PG_CL_BLACK, PG_CL_XOR, PG_CL_BLACK, &pg_unifont},
        { PG_F_TRANSP, PG_CL_WHITE, PG_CL_WHITE, PG_CL_WHITE, &pg_unifont},
    };

    // Hacky - spread transparent textbox to full bar
    pg_create_via_tags( PG_O_BTXTLAB, me,

                        bar->lmarg,
                        bar->tmarg,
                        me->area.right - me->area.left - bar->rmarg,
                        me->area.bot - me->area.top - bar->bmarg,

                        PG_A_JUST, PG_JUST_C,
                        PG_A_THEME, (pg_val_t) &xor_theme,
                        PG_A_TEXT, (pg_val_t) (me->flags & PG_F_CUSTOM_TEXT ? "" : get_text( buf, bar->percent)));
}
