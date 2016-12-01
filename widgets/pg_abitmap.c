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
#define __PG_ABITMAP_C__
#define __PG_C__

#include <string.h>
#include "pg_syntax.h"
#include "pg_config.h"
#include "pg_primitives.h"
#include "pg.h"
#include "pg_res.h"
#include "pg_mempool.h"
#include "pg_abitmap.h"

static const pg_abitmap_t def_abitmap = { .obj.type = PG_O_ABMP, .obj.theme = &pg_def_theme, .anim_period = 1};

static pg_abitmap_t *actors_head;

static uint is_actor_in_list(const pg_abitmap_t *actor)
{
    for (pg_abitmap_t *a = actors_head; a; a = a->next_actor)
        if (a == actor) return 1;
    return 0;
}

static void add_actor(pg_abitmap_t *actor)
{
    if (! is_actor_in_list(actor))
    {
        actor->next_actor = actors_head;
        actors_head = actor;
    }
}

static void remove_actor(pg_abitmap_t *actor)
{
    if (actors_head == actor)
    {
        actors_head = actor->next_actor;
    }
    else
    {
        for (pg_abitmap_t *a = actors_head; a; a = a->next_actor)
        {
            if (a->next_actor == actor)    // Found previous connection, relink
            {
                a->next_actor = actor->next_actor;
                break;
            }
        }
    }
}

pg_obj_t *pg_abitmap_create( void)
{
    pg_obj_t *me = (pg_obj_t *) pg_alloc( sizeof( pg_abitmap_t));    // Allocate memory from pool
    if ( !me)
        return 0;
    memcpy( me, &def_abitmap, sizeof( pg_abitmap_t));
    return me;
}

void pg_abitmap_del( pg_obj_t *me)
{
    pg_abitmap_t *abmp = (pg_abitmap_t*) me;
    remove_actor(abmp);
}

int pg_abitmap_set( pg_obj_t *me, pg_id_t id, pg_val_t val)
{
    pg_abitmap_t *abmp = (pg_abitmap_t*) me;
    switch ( id)
    {
    case PG_A_LMARG:
        abmp->lmarg = val;
        return 1;

    case PG_A_TMARG:
        abmp->tmarg = val;
        return 1;

    case PG_A_RMARG:
        abmp->rmarg = val;
        return 1;

    case PG_A_BMARG:
        abmp->bmarg = val;
        return 1;

    case PG_A_FRAMESET:
        if ( abmp->frameset == (pg_frameset_t *) val)
            return 0;
        abmp->frameset = (pg_frameset_t *) val;
        return 1;

    case PG_A_CFRAME:
        if (abmp->frame_idx == val) return 0;
        abmp->frame_idx = val;
        return 1;

    case PG_A_ANIM_PERIOD:
        abmp->anim_period = val;
        return 0;

    case PG_A_JUST:
        if (val == abmp->just)
            return 0;
        abmp->just = val;
        return 1;
    }

    return pg_base_set( me,id,val);
}

int pg_abitmap_get( pg_obj_t *me, pg_id_t id, pg_val_t *val)
{
    pg_abitmap_t *abmp = (pg_abitmap_t*) me;
    switch ( id)
    {
    case PG_A_LMARG:       *val = abmp->lmarg;                  return 1;
    case PG_A_TMARG:       *val = abmp->tmarg;                  return 1;
    case PG_A_RMARG:       *val = abmp->rmarg;                  return 1;
    case PG_A_BMARG:       *val = abmp->bmarg;                  return 1;
    case PG_A_ANIM_PERIOD: *val = abmp->anim_period;            return 1;
    case PG_A_FRAMESET:    *val = (pg_val_t) abmp->frameset;    return 1;
    case PG_A_CFRAME:      *val = abmp->frame_idx;              return 1;
    case PG_A_JUST:        *val = abmp->just;                   return 1;
    }
    return pg_base_get( me,id,val);
}

void pg_abitmap_rend( pg_obj_t *me)
{
    const pg_abitmap_t *abmp = (pg_abitmap_t*) me;
    if ( !abmp->frameset) return;
    if ( !abmp->frameset->nframes) return;

    uint idx = MIN(abmp->frame_idx, abmp->frameset->nframes - 1);  // trim to be on a safe side;
    const pg_xbm_t *xbm = abmp->frameset->bmps[idx];

    if (!xbm) return;

    const pg_area_t *area = &me->area;
    const pg_deco_t *deco = sel_deco( me);

    int x0 = area->left + abmp->lmarg;
    int y0 = area->top + abmp->tmarg;
    int x1 = area->right - abmp->rmarg;
    int y1 = area->bot - abmp->bmarg;

    int just = abmp->just;

    if ( abmp->just)              // != JUST_NW
    {
        int offx = 0;
        int offy = 0;
        if ( just & PG_F_JCENTER)
            offx = (int)(x1 - x0 + 1 - xbm->w)/2;
        else if ( just & PG_F_JRIGHT)
            offx = x1 - x0 + 1 - xbm->w;
        if ( just & PG_F_JMIDDLE)
            offy = (int)(y1 - y0 + 1 - xbm->h)/2;
        else if ( just & PG_F_JBOT)
            offy = y1 - y0 + 1 - xbm->h;
        x0 = offx > 0 ? x0 + offx : x0;
        y0 = offy > 0 ? y0 + offy : y0;
    }

    pg_draw_image( xbm, x0, y0, x1, y1, deco->fgcol);

}

void pg_abitmap_act( pg_obj_t *me, pg_id_t id, pg_val_t val)
{
    pg_abitmap_t *abmp = (pg_abitmap_t*) me;

    switch (id)
    {
    case PG_M_PLAY:
        add_actor(abmp);
        abmp->period_counter = 1;   // play on the next tick
        return;

    case PG_M_STOP:
        remove_actor(abmp);
        return;

    case PG_M_NUDGE:
        {
            if (!abmp->frameset) return;
            const uint nframes = abmp->frameset->nframes;
            const uint prev_idx = abmp->frame_idx;

            uint idx = prev_idx + 1;
            if (idx >= nframes)
                idx = 0;
            abmp->frame_idx = idx;

            if (abmp->frameset->bmps[prev_idx] == abmp->frameset->bmps[idx]) return;
            pg_force_update(me);
        }
        return;
    }

    pg_base_act(me, id, val);
}

void pg_abitmaps_animate(void)
{
    for (pg_abitmap_t *a = actors_head; a; a = a->next_actor)
    {
        if (! a->period_counter) continue;
        if (!--a->period_counter)
        {
            a->period_counter = a->anim_period;
            pg_abitmap_act((pg_obj_t *)a, PG_M_NUDGE, 1);
        }
    }
}
