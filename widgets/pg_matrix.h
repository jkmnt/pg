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

#ifndef __PG_MATRIX_H__
#define __PG_MATRIX_H__
/** Public */

/** Accepted attributes */
/*
    A_SLOT_TAGLIST   (taglist_p)     Taglist to pass to slots at creation or anytime then attribute set

    A_NITEMS         (uint)          Number of items in matrix

    A_AITEM          (uint)          Current active item

    A_UPD_HOOK       (pg_updhook_t)  Hook to be called for painting slot. Pointer to slot
                                     and item number would be passed. Hook could call pg_set on
                                     slot and change some attributes.
                                     Hook should expect to be asked to print out of bounds (> nItems) value.

    A_WRAP_CURSOR    (bool)          Should we wrap cursor aroung edges
    A_USE_SCROLLING  (bool)          Should we use scrolling instead of whole pages
*/

/** Accepted init-only attributes */
/*
    IA_LMARG        (uint)          Margins from matrix frame to slots (inside to client area)

    IA_TMARG        (uint)          ---

    IA_RMARG        (uint)          ---

    IA_BMARG        (uint)          ---

    IA_NCOLS        (uint)          Max number of columns (could be truncated)

    IA_NROWS        (uint)          Max number of rows (could be truncated)

    IA_SLOT_W       (uint)          Width of each slot

    IA_SLOT_H       (uint)          Height of each slot

    IA_SLOT_XPAD    (uint)          horizontal spacing
    IA_SLOT_YPAD    (uint)          vertical spacing

    IA_SLOT_TYPE    (pg_objs_e)     Type (class) of slots objects
*/

/** Read-only attributes */
/*
    ROA_COL,        (uint)          current col
    ROA_ROW,        (uint)          current row
*/

/** Accepted actions */
/*
    M_CHANGE_SEL    (int)           Add argument (could be positive or negative) to current item number.
                                    May be used for one-by-one item scroll

    M_REDRAW_ITEM   (uint)          Force redraw of single item (item number passed in argument).
                                    Print hook will be called if item is visible.

    M_REDRAW_PAGE                   Force redraw of all visible slots

    M_NEXT_COL                      Step to next column
    M_PREV_COL                      Step to prev column

    M_NEXT_ROW                      Step to next row
    M_PREV_ROW                      Step to prev row
*/

pg_obj_t *pg_matrix_create( void);
int pg_matrix_set( pg_obj_t *me, pg_id_t id, pg_val_t val);
int pg_matrix_get( pg_obj_t *me, pg_id_t id, pg_val_t *val);
void pg_matrix_init( pg_obj_t *me);
void pg_matrix_del( pg_obj_t *me);
void pg_matrix_act( pg_obj_t *me, pg_id_t id, pg_val_t val);

#ifdef __PG_MATRIX_C__
/** Private */
typedef struct
{
    enum pg_objs_e slot_type;
    pg_ctaglist_p init_slot_tags;
    int lmarg;
    int tmarg;
    int rmarg;
    int bmarg;
    int slot_w;
    int slot_h;
    int slot_xpad;
    int slot_ypad;
} mx_init_t;

typedef struct
{
    pg_obj_t obj;
    pg_updhook_t print_hook;
    mx_init_t *init;
    pg_obj_t *scroller;
    uint16_t nitems;
    uint16_t sel_item;
    uint8_t top_row;
    uint8_t ncols;
    uint8_t nrows;
} pg_matrix_t;

enum matrix_flags_e
{
    PG_F_MATRIX_WRAP = _PG_F_USER << 0,
    PG_F_MATRIX_SCROLL = _PG_F_USER << 1,
};

#endif // !__MATRIX_C__
#endif // !__MATRIX_H__
