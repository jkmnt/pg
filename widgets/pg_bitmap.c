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
#define __PG_BITMAP_C__
#define __PG_C__

#include <string.h>
#include "pg_syntax.h"
#include "pg_config.h"
#include "pg_primitives.h"
#include "pg.h"
#include "pg_res.h"
#include "pg_mempool.h"
#include "pg_bitmap.h"

static const pg_bitmap_t def_bitmap = { .obj.type = PG_O_BMP, .obj.theme = &pg_def_theme};

pg_obj_t *pg_bitmap_create( void)
{
    pg_obj_t *me = (pg_obj_t *) pg_alloc( sizeof( pg_bitmap_t));    // Allocate memory from pool
    if ( !me)
        return 0;
    memcpy( me, &def_bitmap, sizeof( pg_bitmap_t));
    return me;
}

int pg_bitmap_set( pg_obj_t *me, pg_id_t id, pg_val_t val)
{
    pg_bitmap_t *bitmap = (pg_bitmap_t*) me;
    switch ( id)
    {
    case PG_A_LMARG:
        bitmap->lmarg = val;
        return 1;

    case PG_A_TMARG:
        bitmap->tmarg = val;
        return 1;

    case PG_A_RMARG:
        bitmap->rmarg = val;
        return 1;

    case PG_A_BMARG:
        bitmap->bmarg = val;
        return 1;

    case PG_A_BMP:
        if ( bitmap->bmp == (pg_xbm_t *) val)
            return 0;
        bitmap->bmp = (pg_xbm_t *) val;
        return 1;

    case PG_A_JUST:
        if ( val == bitmap->just)
            return 0;
        bitmap->just = val;
        return 1;

    }
    return pg_base_set( me,id,val);
}

int pg_bitmap_get( pg_obj_t *me, pg_id_t id, pg_val_t *val)
{
    pg_bitmap_t *Bitmap = (pg_bitmap_t*) me;
    switch ( id)
    {
    case PG_A_LMARG:       *val = Bitmap->lmarg;               return 1;
    case PG_A_TMARG:       *val = Bitmap->tmarg;               return 1;
    case PG_A_RMARG:       *val = Bitmap->rmarg;               return 1;
    case PG_A_BMARG:       *val = Bitmap->bmarg;               return 1;

    case PG_A_BMP:         *val = (pg_val_t) Bitmap->bmp;     return 1;
    case PG_A_JUST:        *val = Bitmap->just;                return 1;
    }
    return pg_base_get( me,id,val);
}

void pg_bitmap_rend( pg_obj_t *me)
{
    const pg_bitmap_t *bitmap = (pg_bitmap_t*) me;
    if ( !bitmap->bmp)
        return;

    const pg_area_t *area = &me->area;
    const pg_deco_t *deco = sel_deco( me);

    int x0 = area->left + bitmap->lmarg;
    int y0 = area->top + bitmap->tmarg;
    int x1 = area->right - bitmap->rmarg;
    int y1 = area->bot - bitmap->bmarg;

    int just = bitmap->just;

    if ( bitmap->just)              // != JUST_NW
    {
        int offx = 0;
        int offy = 0;
        if ( just & PG_F_JCENTER)
        {
            offx = (int)(x1 - x0 + 1 - bitmap->bmp->w)/2;
        }
        else if ( just & PG_F_JRIGHT)
        {
            offx = x1 - x0 + 1 - bitmap->bmp->w;
        }

        if ( just & PG_F_JMIDDLE)
        {
            offy = (int)(y1 - y0 + 1 - bitmap->bmp->h)/2;
        }
        else if ( just & PG_F_JBOT)
        {
            offy = y1 - y0 + 1 - bitmap->bmp->h;
        }
        x0 = offx > 0 ? x0 + offx : x0;
        y0 = offy > 0 ? y0 + offy : y0;
    }

    pg_draw_image( bitmap->bmp, x0, y0, x1, y1, deco->fgcol);

}
