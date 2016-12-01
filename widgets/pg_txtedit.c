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

#define __PG_TXTEDIT_C__
#define __PG_C__
#define __PG_PRIMITIVES_C__

#include <string.h>
#include "pg_syntax.h"
#include "pg_config.h"
#include "pg_utf8.h"
#include "pg_primitives.h"
#include "pg.h"
#include "pg_res.h"
#include "pg_mempool.h"
#include "pg_txtedit.h"

static const pg_txtedit_t def_tedt = { .obj.type = PG_O_TXTEDIT,
                                    .obj.theme = &pg_def_theme,
                                    .lmarg = PG_DEF_TXT_LMARG,
                                    .tmarg = PG_DEF_TXT_TMARG,
                                    .rmarg = PG_DEF_TXT_RMARG,
                                    .bmarg = PG_DEF_TXT_BMARG,
                                    .max_txtlen = 256,
                                    };

#warning "const casting is wrong. gotto never lie to compiler"

// determine a starting position of text in page
static void recalc_visible_page(pg_txtedit_t *tedt)
{
    uint box_w = tedt->obj.area.right - tedt->obj.area.left - tedt->rmarg - tedt->lmarg + 1;
    uint tr = tedt->top_row;

    pg_line_splitter_iterator_t it;
    pg_line_splitter_init(&it, sel_deco(&tedt->obj)->font, tedt->buf, box_w);

    const utf8 *from = tedt->buf;   // be on a safe side
    const utf8 *to = tedt->buf;
    int w = 0;

    for (uint i = 0; i <= tr && w >= 0; i++)
        w = pg_line_splitter_next(&it, &from, &to);

    tedt->top_line = (char *)from;    // remember top line start pointer for fast render
}

// get cursor x,y by position
static int recalc_row_and_col_by_pos(pg_txtedit_t *tedt, const char *p, int favor_next_line)
{
    uint box_w = tedt->obj.area.right - tedt->obj.area.left - tedt->rmarg - tedt->lmarg + 1;

    pg_line_splitter_iterator_t it;
    pg_line_splitter_init(&it, sel_deco(&tedt->obj)->font, tedt->buf, box_w);

    const char *from = tedt->buf;
    const char *to = tedt->buf;

    int r = 0;

    while (1)
    {
        int w = pg_line_splitter_next(&it, &from, &to);
        if (w < 0)
        {
            r -= 1; // not found at all, stick to previous line
            break;
        }
        if (p == from)
            break;
        if (p > from)
        {
            if(favor_next_line)
            {
                if (p < to) break;
            }
            else
            {
                if (p <= to) break;
            }
        }
        r += 1;
    }

    r = MAX(r, 0);
    tedt->row = r;

    // update EOL state
    if (p == from)
    {
        const char *prev = p;
        pg_utf8_prev(tedt->buf, &prev);
        tedt->eol_state = *prev == '\n' ? EOL_AFTER_LF : EOL_AFTER_SYMBOL;
    }
    else if (p == to)
    {
        tedt->eol_state = *p == '\n' ? EOL_BEFORE_LF : EOL_BEFORE_SYMBOL;
    }
    else
        tedt->eol_state = EOL_NONE;

    // so we found a row. let's find a column.
    tedt->col = pg_get_text_len(sel_deco(&tedt->obj)->font, from, p);

    return pg_get_text_width(sel_deco(&tedt->obj)->font, from, p);
}

static void recalc_cursor(pg_txtedit_t *tedt, int favor_next_line)
{
    int old_row = tedt->row;
    int col_offset = recalc_row_and_col_by_pos(tedt, tedt->cursor_pos,favor_next_line);

//  printf("col %d, row %d, eol %d\n", tedt->col, tedt->row, tedt->eol_state);

    if (tedt->row != old_row)
    {
        int tr = tedt->top_row;
        int br = tr + tedt->lines_per_page - 1;

        if (tedt->row < tr || tedt->row > br) // page was changed
        {
            tedt->top_row = tedt->row < old_row ? tedt->row : tedt->row - tedt->lines_per_page + 1;
            recalc_visible_page(tedt);
        }
    }

    int row_offset = (tedt->row - tedt->top_row) * (sel_deco(&tedt->obj)->font->h + tedt->leading);

    if (tedt->cursor)
        pg_moveto(tedt->cursor, col_offset, row_offset);
}

static void recalc_all(pg_txtedit_t *tedt)
{
    const uint line_h = sel_deco(&tedt->obj)->font->h + tedt->leading;
    uint box_h = tedt->obj.area.bot - tedt->obj.area.top - tedt->bmarg - tedt->tmarg + 1;
    uint box_w = tedt->obj.area.right - tedt->obj.area.left - tedt->rmarg - tedt->lmarg + 1;

    //TODO: exclude last leading from height
    const int lpp = box_h / line_h;
    tedt->lines_per_page = MAX(lpp, 1);

    pg_line_splitter_iterator_t it;
    pg_line_splitter_init(&it, sel_deco(&tedt->obj)->font, tedt->buf, box_w);

    const utf8 *f;
    const utf8 *t;

    uint nrows = 0;
    while (pg_line_splitter_next(&it, &f, &t) >= 0)
        nrows++;

    tedt->nrows = nrows;
//  printf("(nrows = %d)\n", tedt->nrows);

    recalc_cursor(tedt, 0);
    recalc_visible_page(tedt);
}

static int change_row(pg_txtedit_t *tedt, int delta)
{
    int new_row = tedt->row + delta;
    new_row = limit32(new_row, 0, tedt->nrows - 1);
    if (new_row == tedt->row) return 0;

    // determine line start/end for row
    const uint box_w = tedt->obj.area.right - tedt->obj.area.left - tedt->rmarg - tedt->lmarg + 1;

    pg_line_splitter_iterator_t it;
    pg_line_splitter_init(&it, sel_deco(&tedt->obj)->font, tedt->buf, box_w);

    char *from = tedt->buf;
    char *to = tedt->buf;
    while (1)
    {
        if (pg_line_splitter_next(&it, (const char **)&from, (const char **)&to) < 0) break;
        if (! new_row--) break;
    }

    if (tedt->col == 0)
        tedt->cursor_pos = (char *)from;
    else
        tedt->cursor_pos = (char *)pg_ut8_pos(from, to, tedt->col);

    recalc_cursor(tedt, tedt->col == 0);
    return 1;
}

#warning "doublecheck memmoves"
static int remove_symbol(pg_txtedit_t *tedt)
{
    if (! tedt->txtlen) return 0;
    if (tedt->buf == tedt->cursor_pos) return 0;
    tedt->txtlen--;

    char *split = tedt->cursor_pos;
    pg_utf8_prev(tedt->buf, (const char **)&tedt->cursor_pos);

    memmove(tedt->cursor_pos, split, (tedt->buf + tedt->max_txtlen * 3 + 1) - split);

//  dbgPrintDump(tedt->buf, tedt->max_txtlen * 3 + 1, 16);

//  printf("txtlen %d\n", tedt->txtlen);
    return 1;
}

static int insert_symbol(pg_txtedit_t *tedt, int symbol)
{
    if (tedt->txtlen >= tedt->max_txtlen) return 0;
    tedt->txtlen++;

    char buf[4];
    int len = pg_utf8_chr(buf, symbol);

    char *insert = tedt->cursor_pos;

    #warning "check memmoves again. please !"

    memmove(insert + len, insert, (tedt->buf + tedt->max_txtlen * 3 + 1) - (insert + len));
    memcpy(insert, buf, len);
    tedt->cursor_pos += len;

//  dbgPrintDump(tedt->buf, tedt->max_txtlen * 3 + 1, 16);

//  printf("txtlen %d\n", tedt->txtlen);
    return 1;
}

static int next_col(pg_txtedit_t *tedt)
{
    if (pg_utf8_ord(tedt->cursor_pos) == '\0') return 0;

    if (tedt->eol_state != EOL_BEFORE_SYMBOL)
        pg_utf8_next((const char **)&tedt->cursor_pos);
    //recalc_cursor(tedt, tedt->eol_state != EOL_NONE);
    recalc_cursor(tedt, tedt->eol_state == EOL_BEFORE_SYMBOL);
    return 1;
}

static int prev_col(pg_txtedit_t *tedt)
{
    if (tedt->cursor_pos == tedt->buf) return 0;

    if (tedt->eol_state != EOL_AFTER_SYMBOL)
        pg_utf8_prev(tedt->buf, (const char **)&tedt->cursor_pos);
    //recalc_cursor(tedt, tedt->eol_state == EOL_NONE);
    recalc_cursor(tedt, tedt->eol_state != EOL_AFTER_SYMBOL && tedt->eol_state != EOL_AFTER_LF);
    return 1;
}

static void alloc_txtbuf(pg_txtedit_t *tedt)
{
    if (tedt->buf)
        pg_free(tedt->buf);

    uint len = tedt->max_txtlen * 3 + 1;    // x3 is for 16-bit unicode, +1 is for null-terminator
    tedt->buf = pg_alloc(len);
    memset(tedt->buf, 0, len);
    tedt->cursor_pos = tedt->buf;
    tedt->top_line = tedt->buf;
    tedt->row = 0;
    tedt->col = 0;
}

pg_obj_t *pg_txtedit_create( void)
{
    pg_obj_t *me = (pg_obj_t *) pg_alloc( sizeof( pg_txtedit_t));    // Allocate memory from pool
    if (!me)
        return 0;
    memcpy( me, &def_tedt, sizeof( pg_txtedit_t));
    return me;
}

void pg_txtedit_init( pg_obj_t *me)
{
    pg_txtedit_t *tedt = (pg_txtedit_t*) me;

    if (! tedt->buf)
        alloc_txtbuf(tedt);
    recalc_all(tedt);
}

void pg_txtedit_del( pg_obj_t *me)
{
    pg_txtedit_t *tedt = (pg_txtedit_t*) me;
    if ( tedt->buf)
        pg_free( tedt->buf);
}

int pg_txtedit_set( pg_obj_t *me, pg_id_t id, pg_val_t val)
{
    pg_txtedit_t *tedt = (pg_txtedit_t*) me;
    switch ( id)
    {
    case PG_A_LMARG:
        tedt->lmarg = val;
        if (is_inited(me))
            recalc_all(tedt);
        return 1;

    case PG_A_TMARG:
        tedt->tmarg = val;
        if (is_inited(me))
            recalc_all(tedt);
        return 1;

    case PG_A_RMARG:
        tedt->rmarg = val;
        if (is_inited(me))
            recalc_all(tedt);
        return 1;

    case PG_A_BMARG:
        tedt->bmarg = val;
        if (is_inited(me))
            recalc_all(tedt);
        return 1;

    case PG_A_LEADING:
        tedt->leading = val;
        if (is_inited(me))
            recalc_all(tedt);
        return 1;

    case PG_IA_MAX_TEXT_LEN:
        tedt->max_txtlen = val;
        return 0;

    case PG_A_TEXT:
        alloc_txtbuf(tedt);
        {
            const char *str = (const char *)val;
            if (! str)
                str = "";
            uint len = pg_utf8_strlen(str, NULL);   // len of text in symbols
            len = MIN(len, tedt->max_txtlen);       // trim to max number of symbols
            uint copylen = strlen(str);             // len of text in bytes
            copylen = MIN(copylen, len * 3);        // trim to max number of symbols * 3 (16-bit utf max)
            memcpy(tedt->buf, str, copylen);
            tedt->buf[copylen] = '\0';              // append terminator. last symbol for may be distorted for overflowed text
            tedt->txtlen = len;

//          dbgPrintDump(tedt->buf, tedt->max_txtlen * 3 + 1, 16);

//          printf("txtlen %d\n", tedt->txtlen);

        }
        if (is_inited(me))
            recalc_all(tedt);
        return 1;

    case PG_A_CURSOR:
        tedt->cursor = (pg_obj_t *)val;
        if (is_inited(me))
            recalc_cursor(tedt, 0);
        return 1;

    case PG_A_THEME:
        pg_base_set(me, id, val);
        if (is_inited(me))
            recalc_all(tedt);
        return 1;
    }

    return pg_base_set(me, id, val);
}

int pg_txtedit_get( pg_obj_t *me, pg_id_t id, pg_val_t *val)
{
    pg_txtedit_t *tedt = (pg_txtedit_t*) me;
    switch ( id)
    {
    case PG_A_LMARG:         *val = tedt->lmarg;                        return 1;
    case PG_A_TMARG:         *val = tedt->tmarg;                        return 1;
    case PG_A_RMARG:         *val = tedt->rmarg;                        return 1;
    case PG_A_BMARG:         *val = tedt->bmarg;                        return 1;

    case PG_A_LEADING:       *val = tedt->leading;                      return 1;
    case PG_A_TEXT:          *val = (pg_val_t) tedt->buf;               return 1;
    case PG_ROA_CURSOR_TEXT: *val = (pg_val_t) tedt->cursor_pos;        return 1;
    case PG_ROA_TXTLEN:      *val = tedt->txtlen;                       return 1;

    case PG_ROA_ROW:         *val = tedt->row;                          return 1;
    case PG_ROA_COL:         *val = tedt->col;                          return 1;
    }
    return pg_base_get(me, id, val);
}

void pg_txtedit_act( pg_obj_t *me, pg_id_t id, pg_val_t val)
{
    pg_txtedit_t *tedt = (pg_txtedit_t*) me;

    switch ( id)
    {
    case PG_M_NEXT_COL:
        if (next_col(tedt))
            pg_force_update(me);
        return;

    case PG_M_PREV_COL:
        if (prev_col(tedt))
            pg_force_update(me);
        return;

    case PG_M_PREV_ROW:
        if (change_row(tedt, -1))
            pg_force_update(me);
        return;

    case PG_M_NEXT_ROW:
        if (change_row(tedt, +1))
            pg_force_update(me);
        return;

    case PG_M_REDRAW_PAGE:
        recalc_all(tedt);
        pg_force_update(me);
        return;

    case PG_M_REMOVE_SYMBOL:
        if (remove_symbol(tedt))
        {
            recalc_all(tedt);
            pg_force_update(me);
        }
        return;

    case PG_M_INSERT_SYMBOL:
        if (insert_symbol(tedt, val))
        {
            recalc_all(tedt);
            pg_force_update(me);
        }
        return;
    }
    pg_base_act( me,id,val);
}

void pg_txtedit_rend( pg_obj_t *me)
{
    const pg_txtedit_t *tedt = (pg_txtedit_t*) me;
    const pg_deco_t *deco = sel_deco( me);
    const pg_area_t *area = &me->area;
    int x0 = area->left + tedt->lmarg;
    int y0 = area->top + tedt->tmarg;
    int x1 = area->right - tedt->rmarg;
    int y1 = area->bot - tedt->bmarg;

    if ( !tedt->buf) return;

    // trim heigth to the integer number of lines
    int line_h = deco->font->h + tedt->leading;
    int box_h = line_h * tedt->lines_per_page;

    y1 = MIN( (uint)y1, (uint)y0 + (uint)box_h - 1);

    pg_draw_multiline_text( deco->font, tedt->top_line, x0, y0, x1, y1, deco->fgcol, -1, -1, tedt->leading);
}

