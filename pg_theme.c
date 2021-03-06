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

#define __PG_THEME_C__

#include "pg_syntax.h"
#include "pg_config.h"
#include "pg_primitives.h"
#include "pg_res.h"
#include "pg_attributes.h"
#include "pg_theme.h"

/** Default theme */
const pg_theme_t pg_def_theme =
{
    { PG_F_BORD, PG_CL_WHITE, PG_CL_BLACK, PG_CL_BLACK, &pg_unifont},
    { PG_F_BORD, PG_CL_BLACK, PG_CL_WHITE, PG_CL_BLACK, &pg_unifont},
    { PG_F_BORD, PG_CL_WHITE, PG_CL_WHITE, PG_CL_WHITE, &pg_unifont},
};

/** Default inversed theme */
const pg_theme_t pg_dev_inv_theme =
{
    { PG_F_BORD, PG_CL_BLACK, PG_CL_WHITE, PG_CL_BLACK, &pg_unifont},
    { PG_F_BORD, PG_CL_WHITE, PG_CL_BLACK, PG_CL_BLACK, &pg_unifont},
    { PG_F_BORD, PG_CL_WHITE, PG_CL_WHITE, PG_CL_WHITE, &pg_unifont},
};

