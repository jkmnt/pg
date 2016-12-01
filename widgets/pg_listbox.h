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

#ifndef __PG_LISTBOX_H__
#define __PG_LISTBOX_H__
/** Public */

/** Acceptable attributes */
/*
    A_SLOT_TAGLIST  (taglist_p)     Taglist to pass to slots at creation or anytime then attribute set

    A_NITEMS        (uint)          Number of items in list

    A_AITEM         (uint)          Current active item

    A_UPD_HOOK      (pg_updhook_t)  Hook to be called for painting slot. Pointer to slot and item number would
                                    be passed. Hook could call pg_set on slot and change some attributes.
*/

/** Acceptable init-only attributes */
/*
    IA_LMARG        (uint)          Margins from listbox frame to slots (inside to client area)

    IA_TMARG        (uint)          ---

    IA_RMARG        (uint)          ---

    IA_BMARG        (uint)          ---

    IA_NSLOTS       (uint)          Number of slots. Would be truncated to number of slots
                                    could be actually placed in listbox

    IA_SLOT_LEN     (uint)          Height of each slot for vlistbox, width of slot for hlistbox

    IA_SLOT_PAD     (uint)          Vertical spacing for vlistbox (or horizontal for hlistbox) between
                                    borders of each slot

    IA_SLOT_TYPE    (pg_objs_e)     Type (class) of slots objects
*/

/** Acceptable actions */
/*
    M_CHANGE_SEL    (int)           Add argument (could be positive or negative) to current item number.
                                    May be used for one-by-one item scroll

    M_REDRAW_ITEM   (uint)          Force redraw of single item (item number passed in argument).
                                    Print hook will be called if item is visible.

    M_REDRAW_PAGE                   Force redraw of all visible slots
*/

pg_obj_t *pg_listbox_create( void);
int pg_listbox_set( pg_obj_t *Me, pg_id_t Id, pg_val_t Val);
int pg_listbox_get( pg_obj_t *Me, pg_id_t Id, pg_val_t *Val);
void pg_hlistbox_init( pg_obj_t *Me);
void pg_vlistbox_init( pg_obj_t *Me);
void pg_listbox_del( pg_obj_t *Me);
void pg_listbox_act( pg_obj_t *Me, pg_id_t Id, pg_val_t Val);

#ifdef __PG_LISTBOX_C__
/** Private */

typedef struct
{
    enum pg_objs_e slot_type;
    pg_ctaglist_p init_slot_tags;
    int lmarg;
    int tmarg;
    int rmarg;
    int bmarg;
    int slot_len;
    int slot_pad;
} lb_init_t;

typedef struct
{
    pg_obj_t obj;
    pg_updhook_t print_hook;
    lb_init_t *init;
    pg_obj_t *scroller;
    uint16_t top_item;
    uint16_t nitems;
    uint16_t sel_item;
    uint8_t nslots;
}  pg_listbox_t;

enum listbox_flags_e
{
    PG_F_WRAP = _PG_F_USER << 0,
};

#endif
#endif
