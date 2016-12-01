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

#ifndef __PG_GROUP_H__
#define __PG_GROUP_H__
/** Public */

// Just experimental stuff. Maybe geometrical manager in a way :-)

/** Accepts iattributes*/
/*
    IA_NITEMS
    IA_ITEMDTA
*/



/** Accepts actions: */
/*
    M_REDRAW_ITEM
*/

typedef struct
{
    int type;
    pg_area_t coords;
    pg_ctaglist_p init_tags;
    pg_updhook_t upd;
} pg_gr_item_t;

typedef pg_gr_item_t pg_gr_items_t[];

pg_obj_t *pg_group_create( void);
int pg_group_set( pg_obj_t *me, pg_id_t id, pg_val_t val);
void pg_group_init( pg_obj_t* me);
void pg_group_act( pg_obj_t *me, pg_id_t id, pg_val_t val);

#ifdef __PG_GROUP_C__
/** Private */

typedef struct
{
    pg_obj_t obj;
    int nitems;
    pg_gr_item_t *itemdata;
} pg_group_t;

#endif // !__GROUP_C__
#endif // !__GROUP_H__
