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

#define __PG_GROUP_C__
#define __PG_C__

#include <string.h>
#include "pg_syntax.h"
#include "pg_config.h"
#include "pg_primitives.h"
#include "pg.h"
#include "pg_mempool.h"
#include "pg_group.h"

static const pg_group_t def_group = { .obj.type = PG_O_GROUP, .obj.theme = &pg_def_theme};

pg_obj_t *pg_group_create( void)
{
    pg_obj_t *me = (pg_obj_t *) pg_alloc( sizeof( pg_group_t));    // Allocate memory from pool
    if (!me)
        return 0;
    memcpy( me, &def_group, sizeof( pg_group_t));
    return me;
}

int pg_group_set( pg_obj_t *me, pg_id_t id, pg_val_t val)
{
    pg_group_t *gr = (pg_group_t *) me;
    switch ( id)
    {
    case PG_IA_NITEMS:
        gr->nitems = val;
        return 0;

    case PG_IA_ITEMDATA:
        gr->itemdata = (pg_gr_item_t *) val;
        return 0;
    }
    return pg_base_set( me, id, val);
}

void pg_group_init( pg_obj_t* me)
{
    pg_group_t *gr = (pg_group_t *) me;
    if ( !gr->nitems || !gr->itemdata)     // No data for objects creation
        return;

    pg_gr_item_t *item = gr->itemdata;

    for ( int cur_item = 0; cur_item < gr->nitems; cur_item++, item++)
    {
        pg_obj_t *obj = pg_create ( (enum pg_objs_e)item->type, me, item->coords.left, item->coords.top, item->coords.right, item->coords.bot, item->init_tags);
        if (item->upd)
            item->upd( obj, cur_item );
    }
}

static void update_item( pg_group_t *gr, int num)
{
    pg_obj_t *child = pg_get_child( &gr->obj, num);
    if ( !child)
        return;

    if ( gr->itemdata[num].upd)
        gr->itemdata[num].upd( child, num );
}

void pg_group_act( pg_obj_t *me, pg_id_t id, pg_val_t val)
{
    pg_group_t *gr = (pg_group_t *) me;
    switch ( id)
    {
    case PG_M_REDRAW_ITEM:
        if ( val >= gr->nitems)
            update_item(gr, val);
        return;
    }
    pg_base_act( me, id, val);
}
