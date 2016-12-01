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

#ifndef __PG_MEMPOOL_H__
#define __PG_MEMPOOL_H__
/** Public */

void *pg_alloc( int size);
int pg_free( void *obj);

void pg_print_mempool_stat( void);

#ifdef __PG_MEMPOOL_C__
/** Private */

#define POOLSIZE           (  (POOL0_BLOCKSIZE + 3)/4 * POOL0_NBLOCKS + (POOL1_BLOCKSIZE + 3)/4 * POOL1_NBLOCKS \
                            + (POOL2_BLOCKSIZE + 3)/4 * POOL2_NBLOCKS + (POOL3_BLOCKSIZE + 3)/4 * POOL3_NBLOCKS \
                            + (POOL4_BLOCKSIZE + 3)/4 * POOL4_NBLOCKS + (POOL5_BLOCKSIZE + 3)/4 * POOL5_NBLOCKS \
                            + (POOL6_BLOCKSIZE + 3)/4 * POOL6_NBLOCKS + (POOL7_BLOCKSIZE + 3)/4 * POOL7_NBLOCKS)

#define POOL_BL_SIZE    ((MAX( POOL7_NBLOCKS, MAX( POOL6_NBLOCKS, MAX( POOL5_NBLOCKS, MAX( POOL4_NBLOCKS, \
                          MAX( POOL3_NBLOCKS, MAX( POOL2_NBLOCKS, MAX( POOL1_NBLOCKS, POOL0_NBLOCKS))))))) + 31)/32)

#define HIST_SIZE       32

typedef struct
{
    int req;
    int freq;
} histo_t;

typedef struct
{
    int maxblock;
    histo_t hist[HIST_SIZE];
    int cur_use[POOL_BL_SIZE*32];
    int max_use[POOL_BL_SIZE*32];
} poolstat_t;

typedef struct
{
    int nblocks;                    // Num of blocks (<32)
    int block_size_in_words;        // Block size in words
    uint32_t used[POOL_BL_SIZE];    // Bitlist for remembering used blocks
#if PG_MEMPOOL_STAT
    poolstat_t stat;
#endif
} pool_t;

static pool_t pools[] =
{
#if ( POOL0_NBLOCKS * POOL0_BLOCKSIZE)
    { POOL0_NBLOCKS, (POOL0_BLOCKSIZE + 3)/4, },
#endif
#if ( POOL1_NBLOCKS * POOL1_BLOCKSIZE)
    { POOL1_NBLOCKS, (POOL1_BLOCKSIZE + 3)/4, },
#endif
#if ( POOL2_NBLOCKS * POOL2_BLOCKSIZE)
    { POOL2_NBLOCKS, (POOL2_BLOCKSIZE + 3)/4, },
#endif
#if ( POOL3_NBLOCKS * POOL3_BLOCKSIZE)
    { POOL3_NBLOCKS, (POOL3_BLOCKSIZE + 3)/4, },
#endif
#if ( POOL4_NBLOCKS * POOL4_BLOCKSIZE)
    { POOL4_NBLOCKS, (POOL4_BLOCKSIZE + 3)/4, },
#endif
#if ( POOL5_NBLOCKS * POOL5_BLOCKSIZE)
    { POOL5_NBLOCKS, (POOL5_BLOCKSIZE + 3)/4, },
#endif
#if ( POOL6_NBLOCKS * POOL6_BLOCKSIZE)
    { POOL6_NBLOCKS, (POOL6_BLOCKSIZE + 3)/4, },
#endif
#if ( POOL7_NBLOCKS * POOL7_BLOCKSIZE)
    { POOL7_NBLOCKS, (POOL7_BLOCKSIZE + 3)/4, },
#endif
};

static uint32_t poolmem[POOLSIZE];

#endif // !__MEMPOOL_C__
#endif // !__MEMPOOL_H__
