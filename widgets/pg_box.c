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

#define __PG_BOX_C__
#define __PG_C__

#include <string.h>
#include "pg_syntax.h"
#include "pg_config.h"
#include "pg_primitives.h"
#include "pg.h"
#include "pg_mempool.h"
#include "pg_box.h"

static const pg_obj_t def_box = { .type = PG_O_BOX, .theme = &pg_def_theme};

pg_obj_t *pg_box_create( void)
{
    pg_obj_t *me = (pg_obj_t *) pg_alloc( sizeof( pg_obj_t));    // Allocate memory from pool
    if (!me)
        return 0;
    memcpy( me, &def_box, sizeof( pg_obj_t));
    return me;
}
