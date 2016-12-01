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

#define __PG_MEMPOOL_C__

#include <string.h>
#include <stdio.h>
#include "pg_syntax.h"
#include "pg_config.h"
#include "pg_mempool.h"

static inline int bl_ntz( const uint32_t *bl)
{
    for ( int word = 0; word < (POOL_BL_SIZE * 32); word += 32, bl++)
    {
        if ( *bl != ~0)                     // Have empty slots
            return ntz( ~*bl) + word;
    }
    return -1;
}


static inline void bl_set( uint32_t *bl, int block)
{
    bl[block/32] |= 1UL << (block % 32);
}

static inline void bl_clear( uint32_t *bl, int Block)
{
    bl[Block/32] &= ~(1UL << (Block % 32));
}

#if PG_MEMPOOL_STAT
static void add_stat( pool_t *pool, int req, int block)
{
    pool->stat.cur_use[block] = req;
    if ( block >= pool->stat.maxblock)
    {
        pool->stat.maxblock++;
        memcpy( pool->stat.max_use, pool->stat.cur_use, sizeof( pool->stat.cur_use));
    }
    histo_t *hist = pool->stat.hist;
    //int Lost = Pool->wordsBlockSize - Req;
    for ( int pos = 0; pos < HIST_SIZE; pos++, hist++)
    {
        if ( hist->req == req || !hist->req)   // Already have request of this size or free slot
        {
            hist->req = req;
            hist->freq++;
            return;
        }
    }
    PG_ERR(" Out of stat slots");
}

void pg_print_mempool_stat( void)
{
    PG_PRINT("\n\nMemory allocation stats:");
    for ( int pool = 0; pool < countof( pools); pool++)
    {
        pool_t *cur_pool = &pools[pool];
        PG_PRINT("\n\n Pool%d (%d of %d blocks was ever used (block size %d bytes)",
               pool, cur_pool->stat.maxblock, cur_pool->nblocks, cur_pool->block_size_in_words * 4);

        if ( !cur_pool->stat.maxblock)
            continue;

        histo_t *hist = cur_pool->stat.hist;

        PG_PRINT("\n    Sizes of requests (bytes):");
        for (; hist->req && hist < (cur_pool->stat.hist + HIST_SIZE); hist++ )
        {
            PG_PRINT("\n    %d (%d times)", hist->req * 4, hist->freq * 4);
        }
        PG_PRINT("\n    Alloc at max load (bytes):\n    |");
        for (int block = 0; block < countof( cur_pool->stat.max_use); block++)
        {
            int use = cur_pool->stat.max_use[block];
            if (!use)
                break;
            PG_PRINT("%d|", use * 4);
        }
    }
}

#endif

/*-----------------------------------------------
    Allocate memory from pool for new object
-----------------------------------------------*/
void *pg_alloc( int size)
{
    size = (size + 3) / 4;                             // Get a size in words

    uint32_t *mem = poolmem;

    /** Search for a pool with a size enough to fit object */
    for ( pool_t *pool = pools; pool < (pools + countof(pools)); pool++)
    {
        if ( pool->block_size_in_words >= size)             // Found !
        {
            int block = bl_ntz( pool->used);           // Find first unused block in bitlist
            if (block >= 0 && block < pool->nblocks)   // Check if we're in range
            {
                bl_set( pool->used, block);
#if PG_MEMPOOL_STAT
                add_stat( pool, size, block);
#endif
                PG_LOG(" Allocated %d bytes from pool %d (block %d)\n", size * 4, pool - pools, block);
                return mem + (pool->block_size_in_words * block);
            }
        }
        mem += pool->block_size_in_words * pool->nblocks;
    }
    PG_ERR(" No free memory ! (required %d bytes)", size * 4);
    return 0;
}

/*-----------------------------------------------
    Free memory allocated for object
-----------------------------------------------*/
int pg_free( void *obj)
{
    uint32_t *optr = (uint32_t *) obj;
    uint32_t *cptr = poolmem;
    pool_t *pool = pools;

    do
    {
        uint poolsize = pool->block_size_in_words * pool->nblocks;

        if ( optr >= cptr && optr < (cptr + poolsize))      // Got it, object is from this pool
        {
            uint block = (optr - cptr)/pool->block_size_in_words;
            bl_clear( pool->used, block);
            PG_LOG(" Deallocated from pool %d (block %d)\n", pool - pools, block);
            return 1;
        }

        pool++;
        cptr += poolsize;
    } while (  pool < (pools + countof(pools)));

    PG_ERR(" No such obj!");
    return 0;
}
