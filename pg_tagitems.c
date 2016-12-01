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

#define __PG_TAGITEMS_C__

#include "pg_syntax.h"
#include "pg_config.h"
#include "pg_tagitems.h"

#if PG_TAGSTACK_SIZE
void pg_tag_start( pg_tag_rt_t *rt, pg_ctaglist_p tl)
{
    rt->top = rt->stack;
    *rt->top = tl;
}

const pg_tag_t *pg_tag_next( pg_tag_rt_t *rt)
{
    const pg_tag_t *tag = *rt->top;    // Get curr tag
    while ( tag->id == PG_TAG_END)    // End of list maybe
    {
        if ( rt->top == rt->stack)  // Empty stack - end of all lists
            return 0;

        tag = *(--(rt->top));       // Step to previous list
    }

    while ( tag->id == PG_TAG_NEXT_TAGLIST)
    {
        *rt->top = tag + 1;          // Store next tag from current list to stack
        rt->top++;
        tag = *rt->top = (pg_taglist_p) tag->data; // Store first tag from new list to stack

        if ( tag->id == PG_TAG_END)
            return 0;
    }
    (*rt->top)++;                  // Step to next tag
    return tag;
}

#else

void pg_tag_start( pg_tag_rt_t *rt, pg_ctaglist_p tl)
{
    rt->curr_tag = tl;
}

const pg_tag_t *pg_tag_next( pg_tag_rt_t *rt)
{
    if ( !rt->curr_tag->id)
        return 0;

    return rt->curr_tag++;
}

#endif
