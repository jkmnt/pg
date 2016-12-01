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

#ifndef __PG_TXTEDIT_H__
#define __PG_TXTEDIT_H__
/** Public */

pg_obj_t *pg_txtedit_create( void);
void pg_txtedit_init( pg_obj_t *me);
int pg_txtedit_set( pg_obj_t *me, pg_id_t id, pg_val_t val);
int pg_txtedit_get( pg_obj_t *me, pg_id_t id, pg_val_t *val);
void pg_txtedit_act( pg_obj_t *me, pg_id_t id, pg_val_t val);
void pg_txtedit_rend( pg_obj_t *me);
void pg_txtedit_del( pg_obj_t *me);

#ifdef __PG_TXTEDIT_C__
/** Private */

enum eol_states
{
    EOL_NONE,
    EOL_BEFORE_SYMBOL,
    EOL_AFTER_SYMBOL,
    EOL_BEFORE_LF,
    EOL_AFTER_LF,
};

typedef struct
{
    pg_obj_t obj;
    char *buf;              // pointer to allocated work buffer
    pg_obj_t *cursor;       // pointer to linked cursor object
    char *top_line;         // position of top visible line
    char *cursor_pos;       // cursor position in string

    uint16_t max_txtlen;    // 65k of symbols should be enough
    uint16_t txtlen;        // running text length in symbols (not bytes)
    uint16_t row;           // column of cursor
    uint16_t top_row;       // top visible_row
    uint16_t nrows;         // summary number of rows

    uint8_t lmarg;
    uint8_t tmarg;
    uint8_t rmarg;
    uint8_t bmarg;
    int8_t leading;
    uint8_t col;            // row of cursor
    uint8_t lines_per_page;
    uint8_t eol_state;
} pg_txtedit_t;

static void recalc_cursor(pg_txtedit_t *me, int favor_prev);

#endif
#endif
