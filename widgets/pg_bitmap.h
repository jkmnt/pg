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

#ifndef __PG_BITMAP_H__
#define __PG_BITMAP_H__
/** Public */

/** Accepted attributes */
/*
    A_LMARG        (int)            Left, right, top and bottom margins to bitmap from border

    A_RMARG        (int)            |

    A_TMARG        (int)            |

    A_BMARG        (int)            |

    PG_A_BMP        (pg_xbm_t*)     Set bitmap data via pointer

    A_JUST          (justes_e)      Justification
*/

pg_obj_t *pg_bitmap_create( void);
int pg_bitmap_set( pg_obj_t *me, pg_id_t id, pg_val_t val);
int pg_bitmap_get( pg_obj_t *me, pg_id_t id, pg_val_t *val);
void pg_bitmap_rend( pg_obj_t *me);

#ifdef __PG_BITMAP_C__
/** Private */

typedef struct
{
    pg_obj_t obj;
    const pg_xbm_t *bmp;
    uint8_t lmarg;
    uint8_t tmarg;
    uint8_t rmarg;
    uint8_t bmarg;
    uint8_t just;
} pg_bitmap_t;

#endif
#endif
