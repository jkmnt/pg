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

#ifndef __PG_RES_H__
#define __PG_RES_H__
/** Public */

extern const pg_utf8_font_t pg_unifont;

typedef struct
{
    const pg_xbm_t *const *bmps;
    uint8_t nframes;
} pg_frameset_t;

#define PG_DEFINE_BMP( name, file)                  const pg_xbm_t name = { file##_height, file##_width, file##_bits}

#define PG_DEFINE_FRAMESET(name, ...)               const pg_xbm_t *const name##_bitmaps_sequence[] = {__VA_ARGS__}; \
                                                    const pg_frameset_t name = {name##_bitmaps_sequence, countof(name##_bitmaps_sequence)}

#define PG_DEFINE_FRAMESET_LOCAL(name, ...)         static const pg_xbm_t *const name##_bitmaps_sequence[] = {__VA_ARGS__}; \
                                                    static const pg_frameset_t name = {name##_bitmaps_sequence, countof(name##_bitmaps_sequence)}

#ifdef __PG_RES_C__
/** Private */

#endif // !__RES_C__
#endif // !__RES_H__
