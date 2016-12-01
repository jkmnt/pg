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
#define __PG_PRIMITIVES_C__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "pg_syntax.h"
#include "pg_config.h"
#include "pg_utf8.h"
#include "pg_primitives.h"

// This 5 functions must be provided by underlying graphical driver.
// They should be resolved at link stage.
extern void pg_put_vblock( uint x, uint y, pg_block_t pat, pg_color_t c);
extern void pg_put_pixel(uint x, uint y, pg_color_t c);
extern void pg_put_hline( uint x0, uint x1, uint y, pg_color_t c);
extern void pg_put_vline( uint x, uint y0, uint y1, pg_color_t c);
extern void pg_put_fill( uint x0, uint y0, uint x1, uint y1, pg_color_t c);

static pg_fast_area_t clip = {.left = 0, .right = 0, .right = SCR_W - 1, .bot = SCR_H - 1};

static const uint block_nbits = 8 * sizeof(pg_block_t);

static inline uint clip_x0( uint x)   { return MAX( x, clip.left);}
static inline uint clip_x1( uint x)   { return MIN( x, clip.right);}
static inline uint clip_y0( uint y)   { return MAX( y, clip.top);}
static inline uint clip_y1( uint y)   { return MIN( y, clip.bot);}
static inline int in_clip_x( uint x0, uint x1) { return (x0 > clip.right || x1 < clip.left) ? 0 : 1; }
static inline int in_clip_y( uint y0, uint y1) { return (y0 > clip.bot || y1 < clip.top) ? 0 : 1; }
static inline int in_clip( uint x0, uint y0, uint x1, uint y1)  {    return in_clip_x(x0,x1) && in_clip_y(y0,y1); }
static inline void swap_coords( uint *c0, uint *c1) { uint tmp = *c0; *c0 = *c1; *c1 = tmp;}
static inline void sort_coords( uint *c0, uint *c1) { if ( *c0 > *c1) swap_coords(c0, c1); }

// exclusively private prototypes
static void draw_icon( const pg_xbm_t *bmp, uint x0, uint y0, uint x1, uint y1, pg_color_t c);
static void bmp8_init( bmp_iterator_t *me, const pg_xbm_t *bmp);
static uint bmp8_next( bmp_iterator_t *me, pg_block_t *data);
static void bmp8_goto_next_column( bmp_iterator_t *me, int cols);

static void draw_full_textline( const pg_utf8_font_t *fnt, const utf8 *start, const utf8 *end, uint x0, uint y0, uint x1, uint y1, pg_color_t c);
static void draw_full_textline_ascii( const pg_font_t *fnt, const char *start, const char *end, uint x0, uint y0, uint x1, uint y1, pg_color_t c);

/**-----------------------------------------------------------------------------
  Put vertical block of length block_nbits, possible clipped
------------------------------------------------------------------------------*/
static inline void putblock(uint x, uint y, pg_block_t block, pg_color_t c)
{
    const uint y0 = y;
    const uint y1 = y0 + block_nbits - 1;

    if ( !in_clip_y(y0, y1) )
        return;

    if (clip.top > y0)
    {
        // Block intersects with top clip, remove some lsbits
        block &= (pg_block_t)~0 << (clip.top - y0);
        if ( !block )
            return;
    }

    if (clip.bot < y1)
    {
        // Block intersects with bottom clip, remove some msbits
        block &= (pg_block_t)~0 >> (y1 - clip.bot);
        if ( !block )
            return;
    }

    pg_put_vblock(x, y, block, c);
}

/**-----------------------------------------------------------------------------
  Reset clip area to default
------------------------------------------------------------------------------*/
void pg_reset_clip( void)
{
    clip.left = 0;
    clip.top = 0;
    clip.right = SCR_W - 1;
    clip.bot = SCR_H - 1;
}

/**-----------------------------------------------------------------------------
  Set clip area

  @param L  - left
  @param T  - top
  @param R  - right
  @param B  - bottom
------------------------------------------------------------------------------*/
void pg_set_clip( int l, int t, int r, int b)
{
    clip.left = l;
    clip.top = t;
    clip.right = r;
    clip.bot = b;
}

/**-----------------------------------------------------------------------------
  Set clip from given area
------------------------------------------------------------------------------*/
void pg_set_clip_via_area( const pg_fast_area_t *area)    { clip = *area;}


/**-----------------------------------------------------------------------------
  Just draw line. Could be slanted.
------------------------------------------------------------------------------*/
void pg_draw_line( uint x0, uint y0, uint x1, uint y1, pg_color_t c)
{
    if ( !in_clip(x0, y0, x1, y1))
        return;

    /** Special cases */
    if ( x0 == x1)
    {
        sort_coords( &y0, &y1);
        pg_put_vline(x0, clip_y0(y0), clip_y1(y1), c);
        return;
    }

    if ( y0 == y1)
    {
        sort_coords( &x0, &x1);
        pg_put_hline(clip_x0( x0), clip_x1( x1), y0, c);
        return;
    }

    // Slanted line. Do Bresenham below
    if ( x0 > x1)
    {
        swap_coords( &x0, &x1);
        swap_coords( &y0, &y1);
    }

    // X0, X1 are sorted now, check for x-clip. y-clips are checked below
    if ( !in_clip_x(x0,x1))
        return;

    const int dx = x1 - x0;

    /** Find octant. We need only 1,2,7,8 octants */
    if ( y1 > y0)   // 1 or 2 octant
    {
        if ( !in_clip_y( y0, y1))
            return;

        const int dy = y1 - y0;
        if ( dy < dx )      // 1 octant
        {
            x1 = clip_x1( x1);

            uint y = y0;
            int err = dx/2;
            for ( uint x = x0; x <= x1; x++)
            {
                if (in_clip(x, y, x, y))
                    pg_put_pixel( x, y, c);
                err -= dy;
                if ( err < 0 )
                {
                    y++;
                    err += dx;
                }
            }
        }
        else                // 2 octant
        {
            uint x = x0;
            int err = dy/2;

            y1 = clip_y1( y1);

            for ( uint y = y0; y <= y1; y++)
            {
                if (in_clip(x, y, x, y))
                    pg_put_pixel( x, y, c);
                err -= dx;
                if (err < 0)
                {
                    x++;
                    err += dy;
                }
            }
        }
    }
    else    // 7 or 8
    {
        if (!in_clip_y( y1, y0))
            return;

        const int dy = y0 - y1;
        if ( dy < dx )      // 8 octant
        {
            uint y = y0;
            int err = dx/2;

            x1 = clip_x1( x1);

            for ( uint x = x0; x <= x1; x++)
            {
                if (in_clip(x, y, x, y))
                    pg_put_pixel( x, y, c);
                err -= dy;
                if ( err < 0 )
                {
                    y--;
                    err += dx;
                }
            }
        }
        else                // 7 octant
        {
            uint x = x1;
            int err = dy/2;

            y0 = clip_y1( y0);

            for ( uint y = y1; y <= y0; y++)
            {
                if (in_clip(x, y, x, y))
                    pg_put_pixel( x, y, c);
                err -= dx;
                if ( err < 0 )
                {
                    x--;
                    err += dy;
                }
            }
        }
    }
}

/**-----------------------------------------------------------------------------
  Render monochrome bitmap

  @param Bmp  - bitmap
  @param X0 - left corner of bitmap
  @param Y0 - top corner of bitmap
  @param X1 - max right corner of bitmap (may be used for clipping)
  @param Y1 - max bottom corner of bitmap (may be used for clipping)
  @param C  - color
------------------------------------------------------------------------------*/
void pg_draw_bitmap( const pg_xbm_t *bmp, uint x0, uint y0, uint x1, uint y1, pg_color_t c)
{
    uint max_x = MIN( x1, x0 + (uint)bmp->w - 1);   // X1, Y1 is at bitmap boundaries
    uint max_y = MIN( y1, y0 + (uint)bmp->h - 1);

    if (!in_clip(x0,y0,max_x,max_y))
        return;

    max_x = clip_x1( max_x);
    max_y = clip_y1( max_y);

    // Switch to optimized version if we fit to single vblock
    if (bmp->h <= block_nbits)
    {
        draw_icon(bmp, x0, y0, max_x, max_y, c);
        return;
    }

    bmp_iterator_t it;

    bmp8_init(&it, bmp);

    if ( x0 < clip.left)
        bmp8_goto_next_column(&it, clip.left - x0 - 1);  // Skip extra columns to the left of clip area

    for ( uint x = clip_x0( x0); x <= max_x; x++)
    {
        pg_block_t data;
        uint y = y0;
        /** Put whole vertical blocks */
        for (; (max_y - y) > (block_nbits - 1); y += block_nbits)
        {
            if (! bmp8_next(&it, &data))
                return;
            if ( data )
                putblock( x, y, data, c);
        }

        /** Put last vertical block */
        if (! bmp8_next(&it, &data))
            return;

        data &= (pg_block_t) ~0 >> (block_nbits - 1 - (max_y - y));
        if ( data )
            putblock( x, y, data, c);
        bmp8_goto_next_column(&it, 0);
    }
}

void pg_draw_image( const pg_xbm_t *bmp, uint x0, uint y0, uint x1, uint y1, pg_color_t c)
{
    const uint mode = bmp->mode;

    if (mode == PG_XBM_1_BIT)
    {
        pg_draw_bitmap(bmp, x0, y0, x1, y1, c);
        return;
    }

    x1 = MIN( x1, x0 + (uint)bmp->w - 1);   // X1, Y1 is at bitmap boundaries
    y1 = MIN( y1, y0 + (uint)bmp->h - 1);

    if (!in_clip(x0, y0, x1, y1)) return;

    const uint bmp_x_offset = x0;
    const uint bmp_y_offset = y0;

    x0 = clip_x0(x0);
    y0 = clip_y0(y0);

    const uint transp = bmp->transp_color;

    for (uint x = x0; x <= x1; x++)
    {
        uint bmp_x = x - bmp_x_offset;

        for (uint y = y0; y <= y1; y++)
        {
            uint bmp_y = y - bmp_y_offset;
            uint pos = bmp_x * bmp->h + bmp_y;
            uint cl = 0;
            if (mode == PG_XBM_4_BIT)
            {
                cl = bmp->data[pos/2];
                if (pos & 0x01)
                    cl >>= 4;
                else
                    cl &= 0x0F;
            }
            else if (mode == PG_XBM_8_BIT)
            {
                cl = bmp->data[pos];
            }
            if (cl != transp)
                pg_put_pixel(x, y, cl);
        }
    }
}

/**-----------------------------------------------------------------------------
  Optimized version of pg_draw_bitmap for small bitmaps
  with height <= block_nbits
------------------------------------------------------------------------------*/
static void draw_icon( const pg_xbm_t *bmp, uint x0, uint y0, uint x1, uint y1, pg_color_t c)
{
    bmp_iterator_t it;

    bmp8_init(&it, bmp);

    if ( clip.left > x0)
        bmp8_goto_next_column(&it, clip.left - x0 - 1);  // Skip extra columns outside of clip area

    for ( uint x = clip_x0( x0); x <= x1; x++)
    {
        pg_block_t data;
        if (! bmp8_next(&it, &data))
            return;
        data &= (pg_block_t)~0 >> (block_nbits - 1 - (y1 - y0));
        if ( data )
            putblock( x, y0, data, c);
    }
}

/**-----------------------------------------------------------------------------
  Measure length of single line text with given font

  @param Fnt  - font
  @param Str  - text strint

  @return  length or -1 if CR detected
------------------------------------------------------------------------------*/
uint pg_get_text_len_ascii( const pg_font_t *fnt, const char *str)
{
    int len = 0;
    int chr;
    if ( !fnt->widths)  // monospaced
    {
        for (; (chr = *str); str++, len += fnt->space)
        {
            if ( chr == '\n')
                return -1;
            if ( chr < fnt->ascii_offset)
                continue;
        }
    }
    else                // Proportional
    {
        while ( (chr = *str++))
        {
            if ( chr == '\n')
                return -1;

            if ( chr < fnt->ascii_offset)
                continue;

            len += fnt->widths[chr - fnt->ascii_offset] + fnt->space;
        }
    }
    return len;
}

static const pg_glyph_t *get_glyph_by_code(const pg_utf8_font_t *fnt, uint code)
{
    // binary search of glyph. Adapted version of Apple's bsearch.c
    const pg_glyph_t *base = &fnt->glyphs[0];

    for (uint lim = fnt->nglyphs; lim; lim = lim / 2)
    {
        const pg_glyph_t *g = &base[lim / 2];
        uint c = g->code;
        if (c == code) return g;    // found
        if (code > c) {
            base = g + 1;
            lim--;
        }
    }

    return &fnt->glyphs[0];     // return default symbol
}

uint pg_get_text_width(const pg_utf8_font_t *fnt, const utf8 *from, const utf8 *to)
{
    int w = 0;
    int chr;

    if (! to) // line is zero-terminated
    {
        while ((chr = pg_utf8_next(&from)))
        {
            if (chr == '\n') break;
            const pg_glyph_t *gl = get_glyph_by_code(fnt, chr);
            w += gl->width + fnt->pad;
        }
    }
    else    // line end is specified
    {
        while (from < to)
        {
            chr = pg_utf8_next(&from);
            if (chr == '\0') break;
            if (chr == '\n') break;
            const pg_glyph_t *gl = get_glyph_by_code(fnt, chr);
            w += gl->width + fnt->pad;
        }
    }

    return w;
}

uint pg_get_text_len(const pg_utf8_font_t *fnt, const utf8 *from, const utf8 *to)
{
    int len = 0;
    int chr;

    if (! to) // line is zero-terminated
    {
        while ((chr = pg_utf8_next(&from)))
        {
            if (chr == '\n') break;
            len += 1;
        }
    }
    else    // line end is specified
    {
        while (from < to)
        {
            chr = pg_utf8_next(&from);
            if (chr == '\0') break;
            if (chr == '\n') break;
            len += 1;
        }
    }

    return len;
}

/**-----------------------------------------------------------------------------
  Draw single line of text

  @param Fnt - font
  @param Str - text string
  @param X0 - start position - left
  @param Y0 - start position - top
  @param X1 - right border to trim textline
  @param Y1 - right border to trim textline
  @param C - color
------------------------------------------------------------------------------*/
void pg_draw_textline_ascii( const pg_font_t *fnt, const char *str, uint x0, uint y0, uint x1, uint y1, pg_color_t c)
{
    pg_xbm_t bmp = { fnt->w, fnt->h};
    uint x = x0;
    uint gl_size = fnt->w * ( (fnt->h + 7) / 8);        // Size of each glyph in bytes
    uint space = fnt->space;

    int chr;
    while ( (chr = *str++))
    {
        if ( chr == '\n')
            break;

        if ( chr < fnt->ascii_offset)
            continue;

        if ( fnt->widths)                       // For proportional ones
            space = fnt->widths[chr - fnt->ascii_offset] + fnt->space;


        if ( x > x1)                            //  Stop rendering if there is no space for next symbol
            return;

//         if ( (X + Space) > X1)               //  Stop rendering if there is no space for full symbol
//             return;

        if ( x > clip.right)
            return;

        bmp.data = &fnt->data[ gl_size * (chr - fnt->ascii_offset)];
        pg_draw_bitmap( &bmp, x, y0, MIN(x1, x + space), MIN(y1, y0 + (uint)fnt->h), c);

        x += space;
    }
}

/**-----------------------------------------------------------------------------
  Draw multiline text with automatic of manual (CR) breaks.

  @param Fnt - font
  @param Str - string
  @param X0 - left position of text box
  @param Y0 - top position of text box
  @param X1 - right position of text box (line will be breaked here)
  @param Y1 - bottom position of text box (line will be terminated here)
  @param C - color
  @param Vpad - additional spacing between lines (+ font height)
------------------------------------------------------------------------------*/
void pd_draw_multiline_text_ascii( const pg_font_t *fnt, const char *str, uint x0, uint y0, uint x1, uint y1, pg_color_t c, uint vpad)
{
    const char *left = str; // Left position of line
    const char *white = 0;  // Position of last whitespace

    uint w = x1 - x0 + 1;
    uint curr_w = 0;
    uint ystep = fnt->h + vpad;
    uint space = fnt->space;

    int chr;

    while ( (chr = *str))
    {
                            // Current char is hard break. Emit line now
        if (chr == '\n')
        {
                            // Draw line from Left position to (not including) Str position
            draw_full_textline_ascii( fnt, left, str, x0, y0, x1, y1, c);
            left = ++str;
        }
        else
        {
                            // Char is not printable, ignore
            if (chr < fnt->ascii_offset)
            {
                ++str;
                continue;
            }

            if (chr == ' ')// Whitespace, remember it
                white = str;

            if (fnt->widths)// Calc Space for proportional font
                space = fnt->widths[chr - fnt->ascii_offset] + fnt->space;

            curr_w += space;    // Increase current width

            if (curr_w <= w)    // More symbols could be fitted in line
            {
                ++str;
                continue;
            }
                            // No more symbols could be fitted
            if (white)      // There are whitespaces remembered
            {
                            // Print from left to whitespace (not including)
                draw_full_textline_ascii( fnt, left, white, x0, y0, x1, y1, c);
                            // Continue next loop from symbol after whitespace
                left = str = white + 1;
            }
            else            // No whitespaces - string is non-breakable. Emit as is
            {
                draw_full_textline_ascii( fnt, left, str, x0, y0, x1, y1, c);
                left = str;
            }
        }

        white = 0;
        curr_w = 0;

        y0 += ystep;
        if ( y0 > y1 || y0 > clip.bot)
            return;
    }
                            // Flush tail
    if (str >= left)
    {
        draw_full_textline_ascii( fnt, left, str, x0, y0, x1, y1, c);
    }
}

/**-----------------------------------------------------------------------------
  Helper routine for mutiline draw - emit single line
  from Start char to End char
-------------------------------------------------------------------------------*/
static void draw_full_textline_ascii( const pg_font_t *fnt, const char *start, const char *end, uint x0, uint y0, uint x1, uint y1, pg_color_t c)
{
    pg_xbm_t bmp = { fnt->w, fnt->h};
    uint x = x0;
    uint gl_size = fnt->w * ( (fnt->h + 7) / 8);                // Size of each glyph in bytes
    uint space = fnt->space;

    for (; start < end; start++)
    {
        if ( x > clip.right)
            return;

        int chr = *start;

        if ( fnt->widths)                                       // For proportional ones
            space = fnt->widths[chr - fnt->ascii_offset] + fnt->space;

        //#warning "Bugs with MIN() ?!"
        bmp.data = &fnt->data[ gl_size * (chr - fnt->ascii_offset)];
        pg_draw_bitmap( &bmp, x, y0, MIN(x1, x + space), MIN(y1, y0 + (uint) fnt->h), c);

        x += space;
    }
}


/**-----------------------------------------------------------------------------
  Draw path from separate segments, each segment defined as a point
  relative to previous one. First point is the offset from
  origin.

  @param Path  - path array
  @param X  - origin X
  @param Y  - origin Y
  @param C  - color
------------------------------------------------------------------------------*/
void pg_draw_path( const pg_path_t *path, int x, int y, pg_color_t c)
{
    int prev_x = x; // Store origin
    int prev_y = y;

    int npoints = path->npoints;
    const pg_point_t *cur_point = path->points;

    prev_x+= cur_point->x;
    prev_y+= cur_point->y;

    for ( npoints--, cur_point++; npoints > 0; npoints--, cur_point++)
    {
        x = prev_x + cur_point->x;
        y = prev_y + cur_point->y;
        pg_draw_line( prev_x, prev_y, x, y, c);
        prev_x = x;
        prev_y = y;
    }
}

/**-----------------------------------------------------------------------------
  Draw single pixel
  ------------------------------------------------------------------------------*/
void pg_draw_pixel(uint x, uint y, pg_color_t c)
{
    if ( !in_clip(x, y, x, y))
        return;

    pg_put_pixel(x, y, c);
}

/**-----------------------------------------------------------------------------
  Draw vertical line
------------------------------------------------------------------------------*/
void pg_draw_vline( uint x, uint y0, uint y1, pg_color_t c)
{
    sort_coords( &y0, &y1);

    if ( !in_clip(x, y0, x, y1))
        return;

    y0 = clip_y0( y0);
    y1 = clip_y1( y1);

    pg_put_vline(x, y0, y1, c);
}

/**-----------------------------------------------------------------------------
  Draw horizontal line
------------------------------------------------------------------------------*/
void pg_draw_hline( uint x0, uint x1, uint y, pg_color_t c)
{
    sort_coords( &x0, &x1);

    if ( !in_clip(x0, y, x1, y))
        return;

    x0 = clip_x0( x0);
    x1 = clip_x1( x1);

    pg_put_hline(x0, x1, y, c);
}

/**-----------------------------------------------------------------------------
  Draw filled rectangle. Second coordinate is included.
  ------------------------------------------------------------------------------*/
void pg_draw_fill( uint x0, uint y0, uint x1, uint y1, pg_color_t c)
{
    sort_coords( &x0, &x1);
    sort_coords( &y0, &y1);

    if ( !in_clip(x0,y0,x1,y1))
        return;

    x0 = clip_x0( x0);
    y0 = clip_y0( y0);
    x1 = clip_x1( x1);
    y1 = clip_y1( y1);

    pg_put_fill(x0, y0, x1, y1, c);
}

/**-----------------------------------------------------------------------------
  Draw non-filled rectangle. Second coordinate is included
------------------------------------------------------------------------------*/
void pg_draw_frame( uint x0, uint y0, uint x1, uint y1, pg_color_t c)
{
    sort_coords( &x0, &x1);
    sort_coords( &y0, &y1);

    if (!in_clip(x0, y0, x1, y1))
        return;

    // Process special cases
    if ( y0 == y1 )
    {
        pg_put_hline(clip_x0( x0), clip_x1( x1), y0, c);
    }
    else if ( x0 == x1 )
    {
        pg_put_vline( x0, clip_y0( y0), clip_y1( y1), c);
    }
    else
    {
        // Process normal case
        if (in_clip_x(x0, x0))
            pg_put_vline(x0, clip_y0( y0), clip_y1( y1), c);

        if (in_clip_x(x1, x1))
            pg_put_vline(x1, clip_y0( y0), clip_y1( y1), c);

        x0 += 1;
        x1 -= 1;

        sort_coords( &x0, &x1);

        // Adjusted X-es may not clip anymore
        if (!in_clip_x(x0, x1))
            return;

        x0 = clip_x0(x0);
        x1 = clip_x1(x1);

        if (in_clip_y(y0, y0))
            pg_put_hline(x0, x1, y0, c);

        if (in_clip_y(y1, y1))
            pg_put_hline(x0, x1, y1, c);
    }
}

/**-----------------------------------------------------------------------------
  Draw frame with pseudo-rounded 1 pixel edges.
------------------------------------------------------------------------------*/
void pg_draw_rounded_frame( uint x0, uint y0, uint x1, uint y1, pg_color_t c)
{
    sort_coords( &x0, &x1);
    sort_coords( &y0, &y1);

    if ( !in_clip(x0,y0,x1,y1))
        return;

    // check dX
    if ( (x1 - x0) > 1 )
    {
        if (in_clip(x0 + 1, y0, x1 - 1, y0))
            pg_put_hline(clip_x0(x0 + 1), clip_x1(x1 - 1), y0, c);

        if (in_clip(x0 + 1, y1, x1 - 1, y1))
            pg_put_hline(clip_x0(x0 + 1), clip_x1(x1 - 1), y1, c);
    }
    // check dY
    if ( (y1 - y0) > 1 )
    {
        if (in_clip(x0, y0 + 1, x0, y1 - 1))
            pg_put_vline(x0, clip_y0( y0 + 1), clip_y1( y1 - 1), c);

        if (in_clip(x1, y0 + 1, x1, y1 - 1))
            pg_put_vline(x1, clip_y0( y0 + 1), clip_y1( y1 - 1), c);
    }
}

/**---------------------------------------------------------------------------
 Bmp iterator goes here
------------------------------------------------------------------------------*/

/*  Data bytes A, B, C ... are arranged in columns
 *  A0 C0 ... U0
 *  |  |      |
 *  A7 C7 ... U7
 *  B0 D0 ... W0
 *  |  |
 *  B7 D7 ... W7
 *
 *  Each call to feeder should return sizeof(pg_block_t) column bytes,
 *  packed to pg_block_t variable
 */
/**-----------------------------------------------------------------------------
  Init iterator for byte-packed bitmap

  @param me  - context
  @param Bmp  - bitmap
------------------------------------------------------------------------------*/
static void bmp8_init( bmp_iterator_t *me, const pg_xbm_t *bmp)
{
    me->bytes_per_col = me->bytes_per_col_remain = (bmp->h + 7) / 8;    // Number of bytes in column
    me->last_col = &bmp->data[bmp->w * me->bytes_per_col];            // Pointer to the column after last.
    me->p = bmp->data;
}

/**-----------------------------------------------------------------------------
  Next method of bmp8 iterator

  @param me  - context
  @param Data  - packed bytes will be put here

  @return 0 at the end of iteration
------------------------------------------------------------------------------*/
static uint bmp8_next( bmp_iterator_t *me, pg_block_t *data)
{
    if ( me->p >= me->last_col)    // End of traverse
        return 0;

    // End of column, setup new one
    if (!me->bytes_per_col_remain)
        me->bytes_per_col_remain = me->bytes_per_col;

    // Number of bytes to output now
    uint nout = MIN(me->bytes_per_col_remain, sizeof(pg_block_t));
    me->bytes_per_col_remain -= nout;

    union
    {
        pg_block_t block;
        uint8_t bytes[sizeof(pg_block_t)];
    } tmp;

    tmp.block = 0;

    uint8_t *bp = tmp.bytes;

    do
    {
        *bp++ = *me->p++;
    } while (--nout);

    *data = tmp.block;
    return 1;
}

/**-----------------------------------------------------------------------------
  Rewind iterator to the start of next (+ Cols) column

  @param me - context
  @param Cols - number of extra columns to skip
------------------------------------------------------------------------------*/
static void bmp8_goto_next_column( bmp_iterator_t *me, int cols)
{
    me->p += me->bytes_per_col_remain + (me->bytes_per_col * cols);
    me->bytes_per_col_remain = me->bytes_per_col;
}




/// unicode stuff

void pg_draw_textline_legacy( const pg_utf8_font_t *fnt, const utf8 *str, uint x0, uint y0, uint x1, uint y1, pg_color_t c)
{
    pg_xbm_t bmp = {0, fnt->h};
    uint x = x0;

    uint chr;

    while ((chr = pg_utf8_next(&str)))
    {
        if ( chr == '\n')
            break;

        if ( chr < 0x20)
            continue;

        if ( x > x1)                            //  Stop rendering if there is no space for next symbol
            return;

//         if ( (X + Space) > X1)               //  Stop rendering if there is no space for full symbol
//             return;

        if ( x > clip.right)
            return;

        const pg_glyph_t *gl = get_glyph_by_code(fnt, chr);
        uint space = gl->width + fnt->pad;

        bmp.data = gl->data;
        bmp.w = gl->width;
        pg_draw_bitmap( &bmp, x, y0, MIN(x1, x + space), MIN(y1, y0 + (uint)fnt->h), c);

        x += space;
    }
}

/**-----------------------------------------------------------------------------
  Draw multiline text with automatic of manual (CR) breaks.

  @param Fnt - font
  @param Str - string
  @param X0 - left position of text box
  @param Y0 - top position of text box
  @param X1 - right position of text box (line will be breaked here)
  @param Y1 - bottom position of text box (line will be terminated here)
  @param C - color
  @param Vpad - additional spacing between lines (+ font height)
------------------------------------------------------------------------------*/
void pg_draw_multiline_text_legacy( const pg_utf8_font_t *fnt, const utf8 *str, uint x0, uint y0, uint x1, uint y1, pg_color_t c, uint vpad)
{
    const char *left = str; // Left position of line
    const char *white = NULL;  // Position of last whitespace
    const char *cur = str;

    uint w = x1 - x0 + 1;
    uint curr_w = 0;
    uint ystep = fnt->h + vpad;

    uint chr;

    while (1)
    {
        cur = str;
        chr = pg_utf8_next(&str);
        if (chr == '\0')
            break;

        // Current char is hard break. Emit line now
        if (chr == '\n')
        {
            draw_full_textline( fnt, left, cur, x0, y0, x1, y1, c); // Draw line from Left position to (not including) current position
            left = str;
        }
        else
        {
            if (chr == ' ')// Whitespace, remember it
                white = cur;

            const pg_glyph_t *gl = get_glyph_by_code(fnt, chr);
            uint space = gl->width + fnt->pad;
            curr_w += space;    // Increase current width

            if (curr_w <= w)    // More symbols could be fitted in line
                continue;

            // No more symbols could be fitted
            if (white)      // There are whitespaces remembered
            {
                            // Print from left to whitespace (not including)
                draw_full_textline( fnt, left, white, x0, y0, x1, y1, c);
                            // Continue next loop from symbol after whitespace
                pg_utf8_next(&white);
                left = str = white;
            }
            else            // No whitespaces - string is non-breakable. Emit as is
            {
                draw_full_textline( fnt, left, cur, x0, y0, x1, y1, c);
                left = cur;
            }
        }

        white = 0;
        curr_w = 0;

        y0 += ystep;
        if ( y0 > y1 || y0 > clip.bot)
            return;
    }
                            // Flush tail
    if (cur >= left)
    {
        draw_full_textline( fnt, left, cur, x0, y0, x1, y1, c);
    }
}

/**-----------------------------------------------------------------------------
  Helper routine for mutiline draw - emit single line
  from Start char to End char
-------------------------------------------------------------------------------*/
static void draw_full_textline( const pg_utf8_font_t *fnt, const utf8 *start, const utf8 *end, uint x0, uint y0, uint x1, uint y1, pg_color_t c)
{
    if (y0 + fnt->h < clip.top) return;     // render only visible strings

    pg_xbm_t bmp = { 0, fnt->h};
    uint x = x0;

    while (start < end)
    {
        if ( x > clip.right) return;

        uint chr = pg_utf8_next(&start);

        const pg_glyph_t *gl = get_glyph_by_code(fnt, chr);
        uint space = gl->width + fnt->pad;

        //#warning "Bugs with MIN() ?!"
        bmp.data = gl->data;
        bmp.w = gl->width;
        pg_draw_bitmap( &bmp, x, y0, MIN(x1, x + space), MIN(y1, y0 + (uint) fnt->h), c);
        x += space;
    }
}

void pg_line_splitter_init(pg_line_splitter_iterator_t *it, const pg_utf8_font_t *fnt, const utf8 *start, uint box_w)
{
    it->str = start;
    it->box_w = box_w;
    it->fnt = fnt;
}

static int may_broke_here(int c)
{
    if (c == ' ') return 1;
    if (c == '.') return 1;
    if (c == ',') return 1;
    if (c == ':') return 1;
    if (c == ';') return 1;
    if (c == '-') return 1;
    if (c == '(') return 1;
    if (c == ')') return 1;
    if (c == ')') return 1;
    return 0;
}

// returns width or -1 if none
int pg_line_splitter_next(pg_line_splitter_iterator_t *it, const utf8 **from, const utf8 **to)
{
    if (!it->str) return -1;                // str == NULL signals end of iteration at previous pass

    uint curr_w = 0;                        // current line width
    uint last_breakpoint_w = 0;             // line width at occurance of last possible breakpoint
    const char *last_breakpoint = NULL;     // position of last possible breakpoint
    const char *start = it->str;            // line start position
    const char *p;                          // current line position

    uint chr;

    while (1)
    {
        p = it->str;
        chr = pg_utf8_next(&it->str);

        if (chr == '\0')    // eol
        {
            if (p == start)         // empty string
            {
                curr_w = -1;        // signal end of iteration right now
            }
            else
            {
                *from = start;
                *to = p;
            }
            it->str = NULL;
            return curr_w;
        }

        if (chr == '\n')            // hard break. emit line now
        {
            *from = start;          // line from left position to current position (excluding)
            *to = p;                //
        }
        else
        {

            uint space = get_glyph_by_code(it->fnt, chr)->width + it->fnt->pad;

            curr_w += space;            // increase current width

            if (may_broke_here(chr))      // possible breakpoint. remember position and width
            {
                last_breakpoint = p;
                last_breakpoint_w = curr_w;
            }

            if (curr_w <= it->box_w)    // where are a space for more symbols in line, continue
                continue;

            // no more symbols could be fitted
            if (last_breakpoint)             // there was a breakpoint found
            {
                *from = start;               // print from start to break symbol (including)
                p = last_breakpoint;
                pg_utf8_next(&p);
                *to = p;
                it->str = p;            // continue splitting from symbol after break
                curr_w = last_breakpoint_w;
            }
            else                        // string is non-breakable. rollback one symbol and emit as is
            {
                *from = start;
                *to = p;
                it->str = p;
                curr_w -= space;
            }
        }
        return curr_w;
    }
}

void pg_draw_multiline_text( const pg_utf8_font_t *fnt, const utf8 *str, uint x0, uint y0, uint x1, uint y1, pg_color_t c, int halign, int valign, int vpad)
{
    const utf8 *line_from;
    const utf8 *line_to;

    // pass 0: calculate a number of lines in text
    int width;
    int nlines = 0;

    const utf8 *f;
    const utf8 *t;

    const int box_w = x1 - x0 + 1;
    int w;

    // store first width as an optimization for single line case
    pg_line_splitter_iterator_t it;
    pg_line_splitter_init(&it, fnt, str, box_w);
    width = w = pg_line_splitter_next(&it, &line_from, &line_to);

    while(w >= 0)
    {
        nlines++;
        w = pg_line_splitter_next(&it, &f, &t);
    }

    if (!nlines) return;

    int box_h = y1 - y0 + 1;
    uint y = y0;

    if (valign >= 0)    // middle or bottom justification
    {
        uint text_h = nlines * (fnt->h + vpad);

        if (text_h < box_h) // align is valid
        {
            if (valign == 0)    // align == middle
                y += (box_h - text_h) / 2;
            else    // align > 0, bottom
                y += box_h - text_h;
        }
    }

    // Pass 1: render
    int ystep = fnt->h + vpad;

    if (nlines > 1) // recaculate line widths again only for multiline text (text width for single line/first line is already known)
    {
        pg_line_splitter_init(&it, fnt, str, box_w);
        width = pg_line_splitter_next(&it, &line_from, &line_to);
    }
    while (1)
    {
        if (width < 0) return;
        if (y > y1) return;
        if (y > clip.bot) return;

        int x = x0;
        if (halign >= 0)        // center or right justification
        {
            if (width < box_w) // align is valid
            {
                if (halign == 0)    // align == center
                    x += (box_w - width) / 2;
                else    // align > 0, right
                    x += box_w - width;
            }
        }

        draw_full_textline(fnt, line_from, line_to, x, y, x1, y1, c);
        if (! --nlines) return;
        width = pg_line_splitter_next(&it, &line_from, &line_to);
        y += ystep;
    }
}

void pg_draw_textline( const pg_utf8_font_t *fnt, const utf8 *str, uint x0, uint y0, uint x1, uint y1, pg_color_t c)
{
    if (y0 + fnt->h < clip.top) return;     // render only visible strings

    pg_xbm_t bmp = {0, fnt->h};
    uint x = x0;

    uint chr;
    while ((chr = pg_utf8_next(&str)))
    {
        if (chr == '\n') break;
        if (x > x1) return;                     //  Stop rendering if there is no space for next symbol
//         if ( (x + space) > X1)               //  Stop rendering if there is no space for full symbol
//             return;
        if ( x > clip.right) return;

        const pg_glyph_t *gl = get_glyph_by_code(fnt, chr);
        uint space = gl->width + fnt->pad;

        bmp.data = gl->data;
        bmp.w = gl->width;
        pg_draw_bitmap( &bmp, x, y0, MIN(x1, x + space), MIN(y1, y0 + (uint)fnt->h), c);

        x += space;
    }
}
