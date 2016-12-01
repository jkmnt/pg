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

#ifndef __PG_SCROLL_H__
#define __PG_SCROLL_H__
/* Public */

pg_obj_t *pg_scroll_create( void);
int pg_scroll_set( pg_obj_t *me, pg_id_t id, pg_val_t val);
int pg_scroll_get( pg_obj_t *me, pg_id_t id, pg_val_t *val);
void pg_hscroll_rend( pg_obj_t *me);
void pg_vscroll_rend( pg_obj_t *me);

#ifdef __PG_SCROLL_C__
/** Private */

typedef struct
{
    pg_obj_t obj;
    uint16_t nitems;
    uint16_t citem;
    uint8_t page_len;
    uint8_t lmarg;
    uint8_t tmarg;
    uint8_t rmarg;
    uint8_t bmarg;
} pg_scroll_t;

#endif
#endif
