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

#ifndef __PG_UTF8_H__
#define __PG_UTF8_H__
/** Public */

#define PG_ERR_UTF 0xFFFE

typedef char utf8;

uint pg_utf8_next(const utf8 **str);
uint pg_utf8_prev(const utf8* base, const utf8 **str);
uint pg_utf8_ord(const utf8 *str);
int pg_utf8_chr(char *out, uint16_t input);
const utf8 *pg_ut8_pos(const utf8* start, const utf8 *end, uint idx);
uint pg_utf8_strlen(const utf8 *start, const utf8 *end);
void pg_utf8_fix(utf8 *str);

#ifdef __PG_UTF8_C__
/** Private */

#endif
#endif
