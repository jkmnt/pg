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

#define __DRV_FB_MONO_32_C__

#include "pg_syntax.h"
#include "pg_config.h"
#include "drv_fb_mono_32.h"

static pg_framebuffer_t fb;

#define CAT(a, b) a##b
#define LINENAME(prefix, line)  CAT( prefix, line)
#define UNIQUENAME( name )      LINENAME( name, __LINE__ )
#define PANIC_IF( arg)          extern char UNIQUENAME(AssertErrorAtLine)  [ ( arg) ? -1 : 0 ] __attribute__((unused))

PANIC_IF(sizeof(pg_block_t) != sizeof(uint32_t));
PANIC_IF(sizeof(pg_color_t) != sizeof(uint32_t));

static inline void paint( uint x, uint wordy, uint32_t mask, pg_color_t c);
static inline void fill_hblock( int x0, int x1, int wordy, uint32_t mask, pg_color_t c);

/**-----------------------------------------------------------------------------
  Append vertical bit pattern to framebuffer

  @param X  - x
  @param wordY - vertical word index (aka Y/32)
  @param Pat  - bit pattern to append
  @param C  - color, 0 for white, >0 for black, -1 for xor
------------------------------------------------------------------------------*/
static inline void paint( uint x, uint wordy, uint32_t pat, pg_color_t c)
{
    uint32_t *p = &fb.words[x][wordy];
    if (c == -1)
        *p ^= pat;
    else if (c)
        *p |= pat;
    else
        *p &= ~pat;
}

/**-----------------------------------------------------------------------------
  Fill vertical word-aligned block with bit pattern

  @param X0  - x0
  @param X1  - x1
  @param wordY  - vertical word index
  @param Pat  - bit pattern
  @param C - color
------------------------------------------------------------------------------*/
static inline void fill_hblock( int x0, int x1, int wordy, uint32_t pat, pg_color_t c)
{
    for (; x0 <= x1; x0++)
        paint(x0, wordy, pat, c);
}

/**-----------------------------------------------------------------------------
  Put vertical block of pg_block_t size (32 bits for this driver)

  @param X  - x
  @param Y  - real Y (not word index)
  @param Pat  - bit pattern
  @param C - color
------------------------------------------------------------------------------*/
void pg_put_vblock( uint x, uint y, uint32_t pat, pg_color_t c)
{
    uint wordy = y / 32;    // Y in framebuffer

    y &= 31;                // bit offset in fb word

    if (y)
    {
        // Usual case - we're not aligned to word boundary
        // Paint LSword
        uint32_t mask = pat << y;

        if (mask)
            paint( x, wordy, mask, c);

        // Paint MSword (in case it not the last one)
        if (++wordy < countof(fb.words[0]))
        {
            mask = pat >> (32 - y);
            if (mask)
                paint( x, wordy, mask, c);
        }
    }
    else
    {
        // Rare case we're perfectly aligned
        paint( x, wordy, pat, c);
    }
}

/**-----------------------------------------------------------------------------
  Put single pixel at coordinates
------------------------------------------------------------------------------*/
void pg_put_pixel(uint x, uint y, pg_color_t c)
{
    paint(x, y / 32, 1UL << (y & 31), c);
}


/**-----------------------------------------------------------------------------
  Put horizontal line
------------------------------------------------------------------------------*/
void pg_put_hline( uint x0, uint x1, uint y, pg_color_t c)
{
    uint32_t mask = 1UL << (y & 31);

    for (; x0 <= x1; x0++)
        paint(x0, y / 32, mask, c);
}

/**-----------------------------------------------------------------------------
  Put vertical line
------------------------------------------------------------------------------*/
void pg_put_vline( uint x, uint y0, uint y1, pg_color_t c)
{
    uint32_t mask;
    int wordy0 = y0 / 32;
    int wordy1 = y1 / 32;

    /** Check if we're crossing any boundaries */
    if (wordy0 == wordy1)
    {
        // Same block
        // (31 - (Y1 - Y0)) is a simplified (32 - (Y1 - Y0 + 1)), there Y1 - Y0 + 1 is a length of line
        mask = (~0UL >> (31 - (y1 - y0))) << (y0 & 31);
        paint(x, wordy0, mask, c);
    }
    else
    {
        mask = ~0UL << (y0 & 31);
        paint(x, wordy0, mask, c);

        while (++wordy0 < wordy1)
            paint( x, wordy0, ~0UL, c);

        mask = ~0UL >> (31 - (y1 & 31));
        paint( x, wordy0, mask, c);
   }
}

/**-----------------------------------------------------------------------------
  Put rectangular fill. Last point (X1, Y1) is included to paint area

  @param X0  - left
  @param Y0  - top
  @param X1  - right
  @param Y1  - bottom
  @param C
------------------------------------------------------------------------------*/
void pg_put_fill( uint x0, uint y0, uint x1, uint y1, pg_color_t c)
{
    uint32_t mask;
    int wordy0 = y0 / 32;
    int wordy1 = y1 / 32;

    /** Check if we're crossing any boundaries */
    if (wordy0 == wordy1)
    {
        mask = (~0UL >> (31 - (y1 - y0))) << (y0 & 31);
        fill_hblock(x0, x1, wordy0, mask, c);
    }
    else
    {
        // Fill starting vblock
        mask = ~0UL << (y0 & 31);
        fill_hblock(x0, x1, wordy0, mask, c);

        // Fill full blocks
        while (++wordy0 < wordy1)
            fill_hblock(x0, x1, wordy0, ~0UL, c);

        // Fill tail block
        mask = ~0UL >> (31 - (y1 & 31));
        fill_hblock(x0, x1, wordy0, mask, c);               // Fill tail vertical blocks
    }
}

const pg_framebuffer_t *pg_open_framebuffer(void)
{
    return &fb;
}

void pg_clear_framebuffer(void)
{
    for (uint32_t *p = fb.raw_words; p < (fb.raw_words + countof(fb.raw_words)); p++)
        *p = 0;
}
