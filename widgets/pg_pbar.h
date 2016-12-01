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

#ifndef __PG_PBAR_H__
#define __PG_PBAR_H__
/** Public */

/** Attributes */
// enum bar_attrs_e
// {
//     // Arrtibutes for all PGO_xxBAR are the same
//     A_BAR_XMARG = _ATTR_OFFSET( PGO_HBAR),              // uint         X margin between border and bar rectangle
//     A_BAR_YMARG,                                        // uint         Y margin ---
//     A_BAR_PERCENT,                                      // uint         Set the length of bar in percents (limited to 0-100)
//                                                         //              Will also print digits in PGO_xTBAR widgets
// };

/** Attributes for PGO_xTBAR */
// enum tbar_attrs_e
// {
//     A_TBAR_CUSTOM_TEXT = _ATTR_OFFSET( PGO_HTBAR),      // bool         Disable digits printing and use custom string.
//                                                         //              String passed to bar via A_TXTLAB_TXT will
//                                                         //              be forwarded to embedded TXTLAB object.
//                                                         //              Other TXTLAB attributes will also be forwarded -
//                                                         //              use with care, some weird effects are possible :-)
//                                                         //              Note: Custom text string could be passed
//                                                         //              only after bar creation via pg_set_xxx call
// };


pg_obj_t *pg_bar_create( void);
int pg_bar_set( pg_obj_t *me, pg_id_t id, pg_val_t val);
int pg_bar_get( pg_obj_t *me, pg_id_t id, pg_val_t *val);
void pg_hbar_rend( pg_obj_t *me);
void pg_vbar_rend( pg_obj_t *me);


int pg_tbar_set( pg_obj_t *me, pg_id_t id, pg_val_t val);
int pg_tbar_get( pg_obj_t *me, pg_id_t id, pg_val_t *val);
void pg_tbar_init( pg_obj_t *me);

#ifdef __PG_PBAR_C__
/** Private */

typedef struct
{
    pg_obj_t obj;
    uint8_t lmarg;
    uint8_t tmarg;
    uint8_t rmarg;
    uint8_t bmarg;
    uint8_t percent;
} pg_bar_t;

enum bar_flags_e
{
    PG_F_CUSTOM_TEXT = _PG_F_USER << 0,
};

static char *get_text( char *buf, int perc);

#endif
#endif
