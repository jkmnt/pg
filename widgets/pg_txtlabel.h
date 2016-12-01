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

#ifndef __PG_TXTLABEL_H__
#define __PG_TXTLABEL_H__
/** Public */

/** Accepted attributes */

/*
    A_LMARG         (uint)          Left, right margins to text from border
                                    Use with justification option to position text exactly
                                    For example, if A_JUST == JUST_W (west) and A_LMARG = 2,
                                    text will be aligned to left border and offsetted 2 pixels to right.
                                    If A_JUST == JUST_E (East) and A_RMARG == 2, text would be
                                    aligned to right border and offsetted 2 pixels to the left. Start of
                                    text would be limited by object border (text will never start outside
                                    of object). Centered text could be shifted left or right via A_LMARG and/or A_RMARG

    A_RMARG         (uint)          ---

    A_TMARG         (uint)          Vertical margins to text. Same logic as horizonal.
                                    Top line of top-aligned text will be font ascender.
                                    Bottom line of bottom-aligned text will be font descender.
                                    Center line of vertical center aligned text will font meanline

    A_BMARG         (uint)          ---

    A_LEADING       (int)           Additional space between each line of multiline text

    A_TEXT          (char*)         Pointer to string to be rendered. It must be accessible all the time
                                    object exists if PGO_TXTLAB used. String could be discarded after set
                                    if PGO_BTXTLAB used (since this object copy it to internal buffer)

    A_MULTILINE_TEXT (bool)         Should the text be multilined. Hard wrap and \n are supported.
                                    Justification are forced to be NW (top left). If text contains CR, multiline mode are forced

    A_JUST,         (justes_e)      Text justification. Valid for non-multiline text only.
*/

pg_obj_t *pg_txtlab_create( void);
int pg_txtlab_set( pg_obj_t *me, pg_id_t id, pg_val_t val);
int pg_txtlab_get( pg_obj_t *me, pg_id_t id, pg_val_t *val);
int pg_btxtlab_set( pg_obj_t *me, pg_id_t id, pg_val_t val);
void pg_txtlab_rend( pg_obj_t *me);
void pg_txtlab_del( pg_obj_t *me);
void pg_btxtlab_del( pg_obj_t *me);

#ifdef __PG_TXTLABEL_C__
/** Private */

typedef struct
{
    pg_obj_t obj;
    char *text;
    uint8_t lmarg;
    uint8_t tmarg;
    uint8_t rmarg;
    uint8_t bmarg;
    int8_t leading;
    uint8_t just;
} pg_txtlab_t;

enum txtlab_flags_e
{
    PG_F_SINGLE_LINE = _PG_F_USER << 0,
};

#endif
#endif
