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

#ifndef __PG_TAGITEMS_H__
#define __PG_TAGITEMS_H__
/** Public */


typedef uint32_t pg_val_t ;
typedef pg_val_t pg_id_t ;
typedef pg_val_t pg_item_t;        // Simple list, without Id/Data division

typedef struct
{
    pg_id_t id;
    pg_val_t data;
} pg_tag_t;

typedef pg_tag_t pg_taglist_t[];
typedef pg_tag_t *pg_taglist_p;
typedef const pg_tag_t *pg_ctaglist_p;

enum pg_special_tags_e
{
    PG_TAG_END = 0,                // Last tag in list for real taglist
#if PG_TAGSTACK_SIZE
    PG_TAG_NEXT_TAGLIST,
#endif
    _PG_USER_TAGS,                 // Data points to the next chained taglist
};

#ifdef __PG_TAGITEMS_C__
/** Private */

#if PG_TAGSTACK_SIZE
typedef struct
{
    pg_ctaglist_p *top;
    pg_ctaglist_p stack[PG_TAGSTACK_SIZE];
} pg_tag_rt_t;
#else
typedef struct
{
    pg_ctaglist_p curr_tag;
} pg_tag_rt_t;
#endif

void pg_tag_start( pg_tag_rt_t *rt, pg_ctaglist_p tl);
const pg_tag_t *pg_tag_next( pg_tag_rt_t *rt);

#endif
#endif
