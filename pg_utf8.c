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
#define __PG_UTF8_C__

#include "pg_syntax.h"
#include "pg_utf8.h"

uint pg_utf8_next(const utf8 **str)
{
    const char *s = *str;
    uint out = PG_ERR_UTF;
    if ((*s & 0x80) == 0)
    {
        out = *s++;
    }
    else if ((*s & 0xE0) == 0xC0)   // 2 bytes
    {
        uint c = (*s++ & 0x1F) << 6;
        if ((*s & 0xC0) == 0x80)
        {
            c |= *s++ & 0x3F;
            out = c;
        }
    }
    else if ((*s & 0xF0) == 0xE0)   // 3 bytes
    {
        uint c = (*s++ & 0x0F) << 12;
        if ((*s & 0xC0) == 0x80)
        {
            c |= (*s++ & 0x3F) << 6;
            if ((*s & 0xC0) == 0x80)
            {
                c |= *s++ & 0x3F;
                out = c;
            }
        }
    }
    else if ((*s & 0xF8) == 0xF0)   // 4 bytes
    {
        uint c = (*s++ & 0x07) << 18;
        if ((*s & 0xC0) == 0x80)
        {
            c |= (*s++ & 0x3F) << 12;
            if ((*s & 0xC0) == 0x80)
            {
                c |= (*s++ & 0x3F) << 6;
                if ((*s & 0xC0) == 0x80)
                {
                    c |= *s++ & 0x3F;
                    out = c;
                }
            }
        }
    }
    else
    {
        s++;
    }
    *str = s;
    return out;
}

uint pg_utf8_prev(const utf8* base, const utf8 **str)
{
    const char *s = *str;

    // rewind to beginning of current symbol (just to be safe)
    while ((*s & 0xC0) == 0x80)
        s--;

    // rewind to beginning of prev symbol
    s--;
    while ((*s & 0xC0) == 0x80)
        s--;

    if (s < base) return PG_ERR_UTF;

    *str = s;

    return pg_utf8_next(&s);
}

uint pg_utf8_ord(const utf8 *str)
{
    return pg_utf8_next(&str);
}

const utf8 *pg_ut8_pos(const utf8* start, const utf8 *end, uint idx)
{
    if (! end)
    {
        while (idx--)
            pg_utf8_next(&start);
    }
    else
    {
        while ((idx--) && (start < end))
            pg_utf8_next(&start);
    }
    return start;
}

uint pg_utf8_strlen(const utf8 *start, const utf8 *end)
{
    uint len = 0;

    if (! end)
    {
        while (pg_utf8_next(&start) != '\0')
            len++;
    }
    else
    {
        while (start < end && pg_utf8_next(&start) != '\0')
            len++;
    }
    return len;
}

int pg_utf8_chr(char *out, uint16_t input)
{
    char *p = out;
    if (input > 0x7FF)  // 16 bits
    {
        *p++ = ((input >> 12) & 0x0F) | 0xE0;
        *p++ = ((input >> 6) & 0x3F) | 0x80;
        *p++ = (input & 0x3F) | 0x80;
    }
    else if (input > 0x07F)
    {
        *p++ = ((input >> 6) & 0x1F) | 0xC0;
        *p++ = (input & 0x3F) | 0x80;
    }
    else
    {
        *p++ = input;
    }

    *p = '\0';
    return p - out;
}

void pg_utf8_fix(utf8 *str)
{
    uint len = strlen(str);
    if (! len) return;

    // fix last utf8 symbol
    char * const end = &str[len] - 1;
    char *p = end;
    while ((*p & 0xC0) == 0x80)   // rewind to the end of last UTF-8 sequence
        p--;
    if (p != end)   // some rewind has occured
        p = '\0';
}
