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

#ifndef	__PG_SYNTAX_H__
#define __PG_SYNTAX_H__

#include <stdint.h>

typedef unsigned int uint;

#define MIN(x,y)    ((x)<(y)?(x):(y))
#define MAX(x,y)    ((x)>(y)?(x):(y))

#define UMIN( x, y)    ( (uint) (x)< (uint) (y)?(x):(y))
#define UMAX( x, y)    ( (uint) (x)> (uint) (y)?(x):(y))

#define SMIN(x,y)    ( (int) (x)< (int) (y)?(x):(y))
#define SMAX(x,y)    ( (int) (x)> (int) (y)?(x):(y))

#define countof( arg)       ( sizeof(arg)/sizeof(arg[0]))

static inline int apply_flag32( uint32_t *targ, uint32_t flag, int val)
{
    uint32_t prevTarg = *targ;
    *targ = val ? *targ | flag : *targ & ~flag;
    return *targ != prevTarg;
}

static inline int32_t limit32( int32_t val, int32_t min, int32_t max)
{
    if (val < min)
        return min;
    if (val > max)
        return max;
    return val;
}

static inline uint32_t ulimit32( uint32_t val, uint32_t min, uint32_t max)
{
    if (val < min)
        return min;
    if (val > max)
        return max;
    return val;
}

#endif
