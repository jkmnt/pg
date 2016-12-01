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

#define __PG_TXTLABEL_C__
#define __PG_C__

#include <string.h>
#include "pg_syntax.h"
#include "pg_config.h"
#include "pg_primitives.h"
#include "pg.h"
#include "pg_res.h"
#include "pg_mempool.h"
#include "pg_txtlabel.h"

/** Default label params */
static const pg_txtlab_t def_lab = { .obj.type = PG_O_TXTLAB,
                                    .obj.theme = &pg_def_theme,
                                    .lmarg = PG_DEF_TXT_LMARG,
                                    .tmarg = PG_DEF_TXT_TMARG,
                                    .rmarg = PG_DEF_TXT_RMARG,
                                    .bmarg = PG_DEF_TXT_BMARG,
                                    .just = PG_JUST_W };

pg_obj_t *pg_txtlab_create( void)
{
    pg_obj_t *me = (pg_obj_t *) pg_alloc( sizeof( pg_txtlab_t));    // Allocate memory from pool
    if (!me)
        return 0;
    memcpy( me, &def_lab, sizeof( pg_txtlab_t));
    return me;
}

void pg_btxtlab_del( pg_obj_t *me)
{
    pg_txtlab_t *lab = (pg_txtlab_t*) me;
    if ( lab->text)
        pg_free( lab->text);
}

/*-----------------------------------------------
    Apply parameter to label
-----------------------------------------------*/
int pg_txtlab_set( pg_obj_t *me, pg_id_t id, pg_val_t val)
{
    pg_txtlab_t *lab = (pg_txtlab_t*) me;
    switch ( id)
    {
    case PG_A_LMARG:            lab->lmarg = val;            return 1;
    case PG_A_TMARG:            lab->tmarg = val;            return 1;
    case PG_A_RMARG:            lab->rmarg = val;            return 1;
    case PG_A_BMARG:            lab->bmarg = val;            return 1;
    case PG_A_LEADING:          lab->leading = val;          return 1;
    case PG_A_TEXT:             lab->text = (char *) val;    return 1;
    case PG_A_SINGLE_LINE:
        return apply_flag32( &me->flags, PG_F_SINGLE_LINE, val);

    case PG_A_JUST:
        if ( val == lab->just)
            return 0;
        lab->just = val;
        return 1;
    }
    return pg_base_set(me, id, val);
}

int pg_txtlab_get( pg_obj_t *me, pg_id_t id, pg_val_t *val)
{
    pg_txtlab_t *lab = (pg_txtlab_t*) me;
    switch ( id)
    {
    case PG_A_LMARG:       *val = lab->lmarg;              return 1;
    case PG_A_TMARG:       *val = lab->tmarg;              return 1;
    case PG_A_RMARG:       *val = lab->rmarg;              return 1;
    case PG_A_BMARG:       *val = lab->bmarg;              return 1;

    case PG_A_LEADING:     *val = lab->leading;            return 1;
    case PG_A_TEXT:        *val = (pg_val_t) lab->text;   return 1;
    case PG_A_SINGLE_LINE: *val = me->flags | PG_F_SINGLE_LINE; return 1;
    case PG_A_JUST:        *val = lab->just;               return 1;
    }
    return pg_base_get(me, id, val);
}

/*-----------------------------------------------
    Buffered version of txtlabel
-----------------------------------------------*/
int pg_btxtlab_set( pg_obj_t *me, pg_id_t id, pg_val_t val)
{
    pg_txtlab_t *lab = (pg_txtlab_t*) me;
    switch ( id)
    {
    case PG_A_TEXT:
        if ( lab->text)
            pg_free( lab->text);
        lab->text = NULL;
        if (val)
        {
            lab->text = (char *) pg_alloc( strlen( (char *) val)+1);
            if (!lab->text) return 0;
            strcpy( lab->text, (char *) val);
        }
        return 1;
    }
    return pg_txtlab_set(me, id, val);
}

/*-----------------------------------------------
    Label renderer
-----------------------------------------------*/
void pg_txtlab_rend( pg_obj_t *me)
{
    const pg_txtlab_t *lab = (pg_txtlab_t*) me;
    const pg_deco_t *deco = sel_deco( me);
    const pg_area_t *area = &me->area;
    int x0 = area->left + lab->lmarg;
    int y0 = area->top + lab->tmarg;
    int x1 = area->right - lab->rmarg;
    int y1 = area->bot - lab->bmarg;

    if ( !lab->text)
        return;

    int just = lab->just;

    if (! (me->flags & PG_F_SINGLE_LINE))   // common multiline text
    {
        int valign = just & PG_F_JMIDDLE ? 0 : (just & PG_F_JBOT ? 1 : -1);
        int halign = just & PG_F_JCENTER ? 0 : (just & PG_F_JRIGHT ? 1 : -1);
        pg_draw_multiline_text( deco->font, lab->text, x0, y0, x1, y1, deco->fgcol, halign, valign, lab->leading);
    }
    else    // single-line text forced
    {
        int txtlen = pg_get_text_width( deco->font, lab->text, NULL);

        if ( just)                  // Justification != JUST_NW
        {
            int offx = 0;
            int offy = 0;

            if ( just & PG_F_JCENTER)
                offx = (int)(x1 - x0 + 1 - txtlen)/2;
            else if ( just & PG_F_JRIGHT)
                offx = (x1 - x0 + 1 - txtlen);

            if ( just & PG_F_JMIDDLE)
                offy = (int)(y1 - y0 + 1)/2 - deco->font->meanline;
            else if ( just & PG_F_JBOT)
                offy = (y1 - y0 + 1 - deco->font->h);

            x0 = offx > 0 ? x0 + offx : x0;
            y0 = offy > 0 ? y0 + offy : y0;
        }

        pg_draw_textline( deco->font, lab->text, x0, y0, x1, y1, deco->fgcol);
    }
}
