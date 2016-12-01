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

#ifndef __PG_PRIMITIVES_H__
#define __PG_PRIMITIVES_H__
/** Public */

#if SCR_W <= 128
typedef int8_t pg_x_t;
#else
typedef int16_t pg_x_t;
#endif

#if SCR_H <= 128
typedef int8_t pg_y_t;
#else
typedef int16_t pg_y_t;
#endif

enum pg_xbm_mode_e
{
    PG_XBM_1_BIT,
    PG_XBM_4_BIT,
    PG_XBM_8_BIT,
};

/** Some bounding box */
typedef struct
{
    pg_x_t left;
    pg_y_t top;
    pg_x_t right;
    pg_y_t bot;
} pg_area_t;

typedef struct
{
    uint32_t left;
    uint32_t top;
    uint32_t right;
    uint32_t bot;
} pg_fast_area_t;

typedef struct pg_xbm_t
{
    uint w;                 // Width
    uint h;                 // Height
    uint mode;              // pg_xbm_mode_e
    int transp_color;       // index of transparent color
    const uint8_t *data;    // Default is bytes pack
} pg_xbm_t;

typedef struct
{
    uint16_t code;          // unicode code point (symbol index)
    uint8_t width;          // width of glyph
    uint8_t height;         // height (unused)
    const uint8_t *data;    // pointer to raw bitmap data. height is common to the whole font, width is individual)
} pg_glyph_t;

typedef struct
{
    uint8_t h;                  // common height of all glyphs
    uint8_t meanline;           // distance from bitmap top to global font meanline
    uint8_t pad;                // extra padding between glyphs (unused)
    uint16_t nglyphs;           // number of glyphs in array
    const pg_glyph_t *glyphs;   // pointer to array of glyphs
} pg_utf8_font_t;

typedef struct
{
    uint w;                 // Width of character bitmap
    uint h;                 // Maximum height (height of bitmap)
    uint space;             // Spaces between each glyph (including glyph width) for monospaced fonts, or additional spacing for proportional ones
    uint baseline;          // From bitmap top to baseline
    uint meanline;          // From bitmap top to meanline
    uint ascii_offset;      // First letter in font
    const uint8_t *data;    // Font bitmaps
    const uint8_t *widths;  // For proportional fonts. If null, font is monospaced
} pg_font_t;

typedef struct
{
    pg_x_t x;
    pg_y_t y;
} pg_point_t;

typedef struct
{
    int npoints;
    pg_point_t points[];
} pg_path_t;

void pg_draw_hline( uint x0, uint x1, uint y, pg_color_t c);
void pg_draw_vline( uint x, uint y0, uint y1, pg_color_t c);

void pg_draw_line( uint x0, uint y0, uint x1, uint y1, pg_color_t c);
void pg_draw_fill( uint x0, uint y0, uint x1, uint y1, pg_color_t c);

void pg_draw_frame( uint x0, uint y0, uint x1, uint y1, pg_color_t c);
void pg_draw_rounded_frame( uint x0, uint y0, uint x1, uint y1, pg_color_t c);
void pg_draw_bitmap( const pg_xbm_t *bmp, uint x0, uint y0, uint x1, uint y1, pg_color_t c);
void pg_draw_image( const pg_xbm_t *bmp, uint x0, uint y0, uint x1, uint y1, pg_color_t c);
void pg_draw_textline( const pg_utf8_font_t *fnt, const char *str, uint x0, uint y0, uint x1, uint y1, pg_color_t c);
uint pg_get_text_width( const pg_utf8_font_t *fnt, const char *from, const char *to);
uint pg_get_text_len(const pg_utf8_font_t *fnt, const char *from, const char *to);
void pg_draw_multiline_text( const pg_utf8_font_t *fnt, const char *str, uint x0, uint y0, uint x1, uint y1, pg_color_t c, int halign, int valign, int vpad);
void pg_draw_path( const pg_path_t *path, int x0, int y0, pg_color_t c);
void pg_set_clip_via_area( const pg_fast_area_t *area);
void pg_draw_pixel( uint x, uint y, pg_color_t c);

#ifdef __PG_PRIMITIVES_C__
/** Private */

/** Runtime data for bmp reader*/
typedef struct
{
    const uint8_t *p;           // Pointer to current data
    const uint8_t *last_col;  // Pointer to the last column
    uint bytes_per_col;         // Number of bytes in column
    uint bytes_per_col_remain;  // Number of remaining bytes in column to render
} bmp_iterator_t;

typedef struct
{
    const pg_utf8_font_t *fnt;  // font
    uint box_w;                 // width of box
    const char *str;            // running utf8 string symbol position
} pg_line_splitter_iterator_t;

void pg_line_splitter_init(pg_line_splitter_iterator_t *it, const pg_utf8_font_t *fnt, const utf8 *start, uint box_w);
int pg_line_splitter_next(pg_line_splitter_iterator_t *it, const utf8 **from, const utf8 **to);

#endif // !__PRIMITIVES_C__
#endif // !__PRIMITIVES_H__
