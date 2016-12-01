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

#ifndef __PG_CONFIG_H__
#define __PG_CONFIG_H__
/** Global defines for PG lib */

/** Screen dimensions */
#define SCR_W               251
#define SCR_H               132
//#define SCR_W               128
//#define SCR_H               128

/** Types of color and vblock for used driver */
typedef uint32_t pg_color_t;
typedef uint32_t pg_block_t;

/** Stack for nesting of taglists. Use 0 to disable nesting and TAG_NEXT_TAGL*/
#define PG_TAGSTACK_SIZE       4

/** Minimum size of thumb for scrollbar */
#define PG_MIN_THUMBLEN        2

/** Collect mempool statistic for fine-tuning (memory-hungry option)*/
#define PG_MEMPOOL_STAT        1

/** Should we use User data field of objects (additional 4 bytes for each object) */
#define PG_USE_USERDATA        1

/** Should we use Post draw hooks (additional pointer for each object) */
#define PG_USE_PDHOOK          1

/** Should we draw screen */
#define PG_DRAW_SCREEN_BORDER  0

/** Default x and y text margins */
#define PG_DEF_TXT_LMARG       1
#define PG_DEF_TXT_TMARG       1
#define PG_DEF_TXT_RMARG       1
#define PG_DEF_TXT_BMARG       1

/** Pools for memory manager */
// Size of blocks in each pool in bytes
#define POOL0_BLOCKSIZE     8
#define POOL1_BLOCKSIZE     56
#define POOL2_BLOCKSIZE     84
#define POOL3_BLOCKSIZE     140
#define POOL4_BLOCKSIZE     0
#define POOL5_BLOCKSIZE     0
#define POOL6_BLOCKSIZE     0
#define POOL7_BLOCKSIZE     0

// Number of blocks in each pool
#define POOL0_NBLOCKS       90
#define POOL1_NBLOCKS       32
#define POOL2_NBLOCKS       32
#define POOL3_NBLOCKS       32
#define POOL4_NBLOCKS       0
#define POOL5_NBLOCKS       0
#define POOL6_NBLOCKS       0
#define POOL7_NBLOCKS       0


// Number of trailing zeroes function. This one is built in mingw.
// Replace with a good assembly routine for embedded system.
#define ntz( Arg)   __builtin_ctz( Arg)

/** Info and error logging */
#define PG_LOG( Fmt, ...)  printf( Fmt,  ## __VA_ARGS__)
#define PG_ERR( Fmt, ...)  printf( Fmt,  ## __VA_ARGS__)
#define PG_PRINT( Fmt, ...)  printf( Fmt,  ## __VA_ARGS__)

#endif
