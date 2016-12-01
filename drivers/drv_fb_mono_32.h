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

#ifndef __DRV_FB_MONO_32_H__
#define __DRV_FB_MONO_32_H__
/** Public */

#define _SCR_H      ((SCR_H + 31) & -32)   // Ceil height to word
#define _SCR_W      SCR_W

typedef union
{
    uint32_t words[SCR_W][_SCR_H / 32];
    uint8_t bytes[SCR_W][_SCR_H / 8];
    uint32_t raw_words[SCR_W * (_SCR_H / 32)];
    uint8_t raw_bytes[SCR_W * (_SCR_H / 8)];
} pg_framebuffer_t;

const pg_framebuffer_t *pg_open_framebuffer(void);
void pg_clear_framebuffer(void);

#ifdef __DRV_FB_MONO_32_C__
/** Private */

#endif
#endif
