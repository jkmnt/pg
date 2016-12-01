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

#ifndef __PG_ATTRIBUTES_H__
#define __PG_ATTRIBUTES_H__

#include "pg_tagitems.h"

#define _PG_IATTR_FLAG             0x10000

// TODO:
// Add description to TAG fields


// Attributes with I letter ( PG_IA_<>) could be passed only at creation

#define _PG_TAGSPACE_START _PG_USER_TAGS

enum pg_objs_e
{
    PG_O_BASE = 0,
    PG_O_BOX = PG_O_BASE,         // Box and Canvas are the same as base. Use PD_HOOK of Canvas to draw custom
    PG_O_CANVAS = PG_O_BASE,      // graphic via primitives.
    PG_O_TXTLAB,
    PG_O_BTXTLAB,
    PG_O_TXTEDIT,
    PG_O_TXTVIEW,
    PG_O_BTXTVIEW,
    PG_O_BMP,
    PG_O_ABMP,
    PG_O_HBAR,
    PG_O_VBAR,
    PG_O_HTBAR,
    PG_O_VTBAR,
    PG_O_HLISTBOX,
    PG_O_VLISTBOX,
    PG_O_MATRIX,
    PG_O_GROUP,
    PG_O_HSCROLL,
    PG_O_VSCROLL,
    //PGO_MCLIST,
    _PG_O_LAST_OBJ,
};

enum pg_attrs_e
{
    /** Base */
    PG_A_THEME = _PG_TAGSPACE_START,
    PG_A_HIDDEN,
    PG_A_USER_DATA,
    PG_A_PDHOOK,
    PG_A_HL,
    PG_A_OFF,
    PG_A_PROPAGATE_DECO,

    /** Added by txtlab*/
    PG_A_JUST,
    PG_A_LMARG,
    PG_A_TMARG,
    PG_A_RMARG,
    PG_A_BMARG,
    PG_A_LEADING,
    PG_A_TEXT,
    PG_A_SINGLE_LINE,

    /** Added by bitmap */
    PG_A_BMP,

    /** Added by abitmap */
    PG_A_FRAMESET,
    PG_A_CFRAME,
    PG_A_ANIM_PERIOD,

    /** Added by listbox*/
    PG_A_SLOT_TAGLIST,
    PG_A_NITEMS,
    PG_A_CITEM,
    PG_A_UPD_HOOK,
    PG_A_SCROLLER,
    PG_A_WRAP_CURSOR,

    /** Added by matrix*/
    PG_A_USE_SCROLLING,

    /** Added by bar*/
    PG_A_PERCENT,
    PG_A_CUSTOM_TEXT,

    /** Added by scroll*/
    PG_A_PAGELEN,

    /** Added by txtedit*/
    PG_A_CURSOR,

    _PG_A_END_ATTR,
};

enum pg_ro_attrs_e
{
    /** Base */
    PG_ROA_TYPE = _PG_A_END_ATTR,
    PG_ROA_FIRST_CHILD,
    PG_ROA_NEXT_BROTHER,
	PG_ROA_AREA,

    /** Matrix and Txtedit */
    PG_ROA_ROW,
    PG_ROA_COL,
    PG_ROA_CURSOR_TEXT,

    /** Txtedit  */
    PG_ROA_TXTLEN,
};

enum pg_iatrrs_e
{
    /** From listbox */
    PG_IA_LMARG = _PG_IATTR_FLAG + _PG_TAGSPACE_START,
    PG_IA_TMARG,
    PG_IA_RMARG,
    PG_IA_BMARG,
    PG_IA_NSLOTS,
    PG_IA_SLOT_LEN,
    PG_IA_SLOT_PAD,
    PG_IA_SLOT_TYPE,

    /** From matrix*/
    PG_IA_NCOLS,
    PG_IA_NROWS,
    PG_IA_SLOT_W,
    PG_IA_SLOT_H,
    PG_IA_SLOT_XPAD,
    PG_IA_SLOT_YPAD,

    /** From txtedit*/
    PG_IA_MAX_TEXT_LEN,

    /** From group */
    PG_IA_NITEMS,
    PG_IA_ITEMDATA,
};

enum pg_acts_e
{
    /** Base */
    PG_M_DIRTY = _PG_TAGSPACE_START,
    PG_M_HIDE,
    PG_M_UNHIDE,
    PG_M_SUICIDE,
    PG_M_LOWER,
    PG_M_LIFT,
    PG_M_TOFRONT,
    PG_M_TOBACK,

    /** From listbox */
    PG_M_CHANGE_SEL,
    PG_M_REDRAW_ITEM,
    PG_M_REDRAW_PAGE,

    /** From matrix and txtedit */
    PG_M_NEXT_COL,
    PG_M_PREV_COL,

    PG_M_NEXT_ROW,
    PG_M_PREV_ROW,

    /** From abitmap */
    PG_M_PLAY,
    PG_M_STOP,
    PG_M_NUDGE,

    /** From txtedit */
    PG_M_INSERT_SYMBOL,
    PG_M_REMOVE_SYMBOL,
};

enum pg_just_bits_e
{
    PG_F_JCENTER = 0x01,
    PG_F_JRIGHT = 0x02,
    PG_F_JMIDDLE = 0x04,
    PG_F_JBOT = 0x08,
};

enum pg_justes_e
{
    PG_JUST_NW = 0,                    // North-West
    PG_JUST_N = PG_F_JCENTER,             // North
    PG_JUST_NE = PG_F_JRIGHT,              // North-East
    PG_JUST_W = PG_F_JMIDDLE,             // West
    PG_JUST_C = PG_F_JMIDDLE | PG_F_JCENTER, // Center
    PG_JUST_E = PG_F_JMIDDLE | PG_F_JRIGHT,   // East
    PG_JUST_SW = PG_F_JBOT,             // South-West
    PG_JUST_S = PG_F_JBOT | PG_F_JCENTER,  // South
    PG_JUST_SE = PG_F_JBOT | PG_F_JRIGHT,   // South-East
};

enum pg_colors_e
{
    PG_CL_WHITE = 0,
    PG_CL_BLACK = 1,
    PG_CL_XOR = -1,
};

#endif
