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

#ifndef __PG_THEME_H__
#define __PG_THEME_H__
/** Public */

enum pg_theme_flags_e
{
    PG_F_TRANSP =   1U << 0,                                                        // Should background be transparent
    PG_F_BORD =     1U << 1,                                                        // Should border be rendered at all
    PG_F_ROUND_NW =  1U << 3,                                                       // Rounded border with 1 pixel radius :-) NW, NE etc is individual rounding of corners
    PG_F_ROUND_NE =  1U << 4,                                                       //
    PG_F_ROUND_SW =  1U << 5,                                                       //
    PG_F_ROUND_SE =  1U << 6,                                                       //
    PG_F_ROUNDED =  PG_F_ROUND_NW | PG_F_ROUND_NE | PG_F_ROUND_SW | PG_F_ROUND_SE,  // Round all corners
    PG_F_ROUND_TOP = PG_F_ROUND_NW | PG_F_ROUND_NE,                                 // Round sides
    PG_F_ROUND_BOT = PG_F_ROUND_SW | PG_F_ROUND_SE,                                 //
    PG_F_ROUND_LEFT = PG_F_ROUND_NW | PG_F_ROUND_SW,                                //
    PG_F_ROUND_RIGHT = PG_F_ROUND_NE | PG_F_ROUND_SE,                               //
};

/** Decoration declaration :-) */
typedef struct
{
    uint32_t flags;         // Flags for border etc
    pg_color_t bgcol;       // Background color
    pg_color_t fgcol;       // Foreground color
    pg_color_t bordcol;     // Border color
    const pg_utf8_font_t *font;  // Font
} pg_deco_t;

typedef struct
{
    pg_deco_t norm;          // Normal object
    pg_deco_t hl;            // Highlighted
    pg_deco_t off;           // Disabled
    pg_deco_t hloff;         // Highlighted and disabled
} pg_theme_t;

typedef const pg_theme_t * pg_theme_p;  // Pointer to const theme

extern const pg_theme_t pg_def_theme;   // default normal and inversed themes. Visible by user and widgets
extern const pg_theme_t pg_dev_inv_theme;

#ifdef __PG_THEME_C__
/** Private */

#endif // !__THEME_C__
#endif // !__THEME_H__
