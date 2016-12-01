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

#define __PG_C__
#define __PG_TAGITEMS_C__

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "pg_syntax.h"
#include "pg_config.h"
#include "pg_mempool.h"
#include "pg_utf8.h"
#include "pg_primitives.h"
#include "pg_tagitems.h"
#include "pg_res.h"
#include "pg.h"
#include "pg_box.h"
#include "pg_txtlabel.h"
#include "pg_txtview.h"
#include "pg_txtedit.h"
#include "pg_bitmap.h"
#include "pg_abitmap.h"
#include "pg_pbar.h"
#include "pg_group.h"
#include "pg_listbox.h"
#include "pg_matrix.h"
#include "pg_scroll.h"

// exclusively private prototypes
static void init_coords( pg_obj_t *me, int left, int top, int right, int bot);
static inline int render_object( pg_obj_t *me);
static inline void add_dirty( const pg_area_t *area);
static inline void intersect_dirty( pg_fast_area_t *dirty, const pg_area_t *area);
static inline void clear_dirty_rect( void);
static inline int pg_hide( pg_obj_t*me, int hide);
static inline int in_dirty_rect( const pg_obj_t *me);
static inline void pg_delete( pg_obj_t *me);

static pg_fast_area_t dirty_rect = { 0, 0, SCR_W - 1, SCR_H - 1};

const pg_theme_t def_screen_theme =
{
#if PG_DRAW_SCREEN_BORDER
    { PG_F_BORD, PG_CL_WHITE, PG_CL_BLACK, PG_CL_BLACK, &pg_unifont},
    { PG_F_BORD, PG_CL_WHITE, PG_CL_BLACK, PG_CL_BLACK, &pg_unifont},
    { PG_F_BORD, PG_CL_WHITE, PG_CL_WHITE, PG_CL_WHITE, &pg_unifont},
#else
    { 0, PG_CL_WHITE, PG_CL_BLACK, PG_CL_BLACK, &pg_unifont},
    { 0, PG_CL_WHITE, PG_CL_BLACK, PG_CL_BLACK, &pg_unifont},
    { 0, PG_CL_WHITE, PG_CL_WHITE, PG_CL_WHITE, &pg_unifont},
#endif
};

static pg_obj_t scr_box =
{
    .area = {.left = 0, .top = 0, .right = SCR_W - 1, .bot = SCR_H - 1}, .type = PG_O_BOX, .theme = &def_screen_theme,
#if PG_USE_USERDATA
    .usr_data = 1024,
#endif
};


pg_obj_t * const pg_screen = (pg_obj_t *) &scr_box;     // User-accessed "Root"

static int need_update;

static const obj_handler_t obj_hnd[] =
{
	[PG_O_BOX] =      { pg_box_create,     pg_base_set,     pg_base_get,    pg_base_act,    0,                0,               0},
	[PG_O_TXTLAB] =   { pg_txtlab_create,  pg_txtlab_set,   pg_txtlab_get,  pg_base_act,    0,                pg_txtlab_rend,  0},
	[PG_O_BTXTLAB] =  { pg_txtlab_create,  pg_btxtlab_set,  pg_txtlab_get,  pg_base_act,    0,                pg_txtlab_rend,  pg_btxtlab_del},
	[PG_O_TXTEDIT] =  { pg_txtedit_create, pg_txtedit_set,  pg_txtedit_get, pg_txtedit_act, pg_txtedit_init,  pg_txtedit_rend, pg_txtedit_del},
	[PG_O_TXTVIEW] =  { pg_txtview_create, pg_txtview_set,  pg_txtview_get, pg_txtview_act, pg_txtview_init,  pg_txtview_rend, 0},
	[PG_O_BTXTVIEW] = { pg_txtview_create, pg_btxtview_set, pg_txtview_get, pg_txtview_act, pg_txtview_init,  pg_txtview_rend, pg_btxtview_del},
	[PG_O_BMP] =      { pg_bitmap_create,  pg_bitmap_set,   pg_bitmap_get,  pg_base_act,    0,                pg_bitmap_rend,  0},
	[PG_O_ABMP] =     { pg_abitmap_create, pg_abitmap_set,  pg_abitmap_get, pg_abitmap_act, 0,                pg_abitmap_rend, pg_abitmap_del},
	[PG_O_HBAR] =     { pg_bar_create,     pg_bar_set,      pg_bar_get,     pg_base_act,    0,                pg_hbar_rend,    0},
	[PG_O_VBAR] =     { pg_bar_create,     pg_bar_set,      pg_bar_get,     pg_base_act,    0,                pg_vbar_rend,    0},
	[PG_O_HTBAR] =    { pg_bar_create,     pg_tbar_set,     pg_tbar_get,    pg_base_act,    pg_tbar_init,     pg_hbar_rend,    0},
	[PG_O_VTBAR] =    { pg_bar_create,     pg_tbar_set,     pg_tbar_get,    pg_base_act,    pg_tbar_init,     pg_vbar_rend,    0},
	[PG_O_VLISTBOX] = { pg_listbox_create, pg_listbox_set,  pg_listbox_get, pg_listbox_act, pg_vlistbox_init, 0,               pg_listbox_del},
	[PG_O_HLISTBOX] = { pg_listbox_create, pg_listbox_set,  pg_listbox_get, pg_listbox_act, pg_hlistbox_init, 0,               pg_listbox_del},
	[PG_O_GROUP] =    { pg_group_create,   pg_group_set,    pg_base_get,    pg_group_act,   pg_group_init,    0,               0},
	[PG_O_MATRIX] =   { pg_matrix_create,  pg_matrix_set,   pg_matrix_get,  pg_matrix_act,  pg_matrix_init,   0,               pg_matrix_del},
	[PG_O_HSCROLL] =  { pg_scroll_create,  pg_scroll_set,   pg_scroll_get,  pg_base_act,    0,                pg_hscroll_rend, 0},
	[PG_O_VSCROLL] =  { pg_scroll_create,  pg_scroll_set,   pg_scroll_get,  pg_base_act,    0,                pg_vscroll_rend, 0},

};

/**-----------------------------------------------------------------------------
  Set dirty flag for object

  @param Me  - object
------------------------------------------------------------------------------*/
static inline void set_dirty( const pg_obj_t *me)
{
    need_update = 1;
    add_dirty( &me->area);
}

/**-----------------------------------------------------------------------------
  Clear dirty rectangle
------------------------------------------------------------------------------*/
static inline void clear_dirty_rect( void)
{
    dirty_rect.left = dirty_rect.top = INT32_MAX;
    dirty_rect.bot = dirty_rect.right = 0;            // Minimized
}

/**-----------------------------------------------------------------------------
  Extend current dirty rectangle to include new area

  @param Area  - new dirty coordinates to append
------------------------------------------------------------------------------*/
static inline void add_dirty( const pg_area_t *area)
{
    dirty_rect.left = MIN( dirty_rect.left, area->left);
    dirty_rect.top = MIN( dirty_rect.top, area->top);
    dirty_rect.right = MAX( dirty_rect.right, area->right);
    dirty_rect.bot = MAX( dirty_rect.bot, area->bot);
}

/**-----------------------------------------------------------------------------
  Check if object is crossing dirty rectangle

  @param Me  - object

  @return  0 if not
------------------------------------------------------------------------------*/
static inline int in_dirty_rect( const pg_obj_t *me)
{
    const pg_area_t *area = &me->area;
    if ( area->left > dirty_rect.right || area->right < dirty_rect.left)  return 0;
    if ( area->top > dirty_rect.bot  || area->bot < dirty_rect.top)       return 0;
    return 1;
}

/**-----------------------------------------------------------------------------
  Trim dirty rectangle to area boundaries

  @param Dirty - dirty area
  @param Area - clip area
------------------------------------------------------------------------------*/
static inline void intersect_dirty( pg_fast_area_t *dirty, const pg_area_t *area)
{
    dirty->left = MAX( dirty->left, area->left);
    dirty->top = MAX( dirty->top, area->top);
    dirty->right = MIN( dirty->right, area->right);
    dirty->bot = MIN( dirty->bot, area->bot);
}

/**-----------------------------------------------------------------------------
  Init iterator to traverse n-ary tree, laid out as binary tree

  @param Me  - iterator
  @param Root  - root node
------------------------------------------------------------------------------*/
static pg_obj_t *tree_iterator_preorder_init( tree_iterator_t *me, pg_obj_t *root)
{
    me->curr = me->root = root;
    return root;
}

/**-----------------------------------------------------------------------------
  Get next object from (sub)tree

  @param Me  - iterator
  @param visitChilds  - flag to visit childs (set to 0 to skip them)

  @return next object or 0 after full traverse
------------------------------------------------------------------------------*/
static pg_obj_t *tree_iterator_preorder_next( tree_iterator_t *me, int do_visit_childs)
{
    if ( do_visit_childs && me->curr->first_child)                               // Visit subtrees
        return me->curr = me->curr->first_child;

    if ( me->curr->next_bro)                                             // Visit brother tree
        return me->curr = me->curr->next_bro;

    while (1)                                                               // Climb back
    {
        pg_obj_t *parent = me->curr->parent;

        if ( (me->curr == me->root) || (parent == me->root))                // Finally traversed back to the root
            return NULL;

        if ( parent->next_bro && (parent->next_bro != me->curr))      // Check if parent have a neigh, and we're returning up not from this neigh branch
            return me->curr = parent->next_bro;

        me->curr = parent;                                                 // Go up the tree
    };
}

/**-----------------------------------------------------------------------------
   Attach object to parent

  @param Me  - object
  @param Parent  - parent object
------------------------------------------------------------------------------*/
static void _attach( pg_obj_t *me, pg_obj_t *parent)
{
    me->parent = parent;

    if ( !parent->first_child)           // Parent have no childs yet, easy stuff
    {
        parent->first_child = me;
    }
    else                                // Gotta traverse list, yap
    {
        pg_obj_t *curr_obj;
        for ( curr_obj = parent->first_child; curr_obj->next_bro; curr_obj = curr_obj->next_bro);

        curr_obj->next_bro = me;
    }
}

/**-----------------------------------------------------------------------------
  Find object by type and set attribute

  @param Me  - object
  @param Id  - attribute id
  @param Val  - attribute value

  @return  0 if error
------------------------------------------------------------------------------*/
static inline int set_attr( pg_obj_t *me, pg_id_t id, pg_val_t val)
{
    if ( is_inited( me) && (id & _PG_IATTR_FLAG))     // Discard init-only sets for already inited objects
            return 0;

    return obj_hnd[me->type].set( me, id, val);
}

/**-----------------------------------------------------------------------------
  Set several attribute of the same object in one call

  @param Me  - object
  @param Tl  - taglist of attributes/values

  @return  0 if object shouldn't be redrawn, other value if should
------------------------------------------------------------------------------*/
static inline int set_attr_taglist( pg_obj_t *me, pg_ctaglist_p tl)
{
    int ret = 0;

    pg_tag_rt_t rt;
    const pg_tag_t *tag;

    pg_tag_start( &rt, tl);

    while ( (tag = pg_tag_next( &rt)))
        ret |= set_attr( me, tag->id, tag->data);

    return ret;
}

/**-----------------------------------------------------------------------------
  Init object - convert coordinates to internal representation

  @param Me  - object
------------------------------------------------------------------------------*/
static void init_coords( pg_obj_t *me, int left, int top, int right, int bot)
{
    // Convert to absolute coordinates
    const pg_area_t *par = &me->parent->area;

    // Positive coordinates means offset from left/top, negatives from right/bot
    left += left >= 0 ? par->left : par->right;
    top += top >= 0 ? par->top : par->bot;
    right += right > 0 ? par->left : par->right;
    bot += bot > 0 ? par->top : par->bot;

    // Safe(?)-limit
    me->area.left = MIN( left, par->right);
    me->area.top = MIN( top, par->bot);
    me->area.right = MIN( right, par->right);
    me->area.bot = MIN( bot, par->bot);
}

/**-----------------------------------------------------------------------------
  Delete object and free memory

  @param Me  - object
------------------------------------------------------------------------------*/
static inline void pg_delete( pg_obj_t *me)
{
    if ( obj_hnd[me->type].del)
        obj_hnd[me->type].del( me);

    pg_free(me);
}

/**-----------------------------------------------------------------------------
  Get previous brother of object

  @param Me  - object

  @return  - prev brother or 0
------------------------------------------------------------------------------*/
static inline pg_obj_t *get_prev_brother( const pg_obj_t *me)
{
    if (!me->parent) return NULL;
    if ( me->parent->first_child == me) return NULL;   // No brothers

    pg_obj_t *tmp;
    for ( tmp = me->parent->first_child; tmp->next_bro != me; tmp = tmp->next_bro);

    return tmp;
}

static inline int pg_hide( pg_obj_t *me, int hide)
{
    if ( !(me->flags & PG_F_HIDDEN) == !hide)  // Check if state change required at all
        return 0;

    if ( hide)  // Hide
        me->flags |= PG_F_HIDDEN;
    else        // Unhide
        me->flags &= ~PG_F_HIDDEN;
    set_dirty( me);
    return 1;
}

/**-----------------------------------------------------------------------------
  Render base object (box)

  @param Me  - object
------------------------------------------------------------------------------*/
static void base_rend( const pg_obj_t *me)
{
    const pg_area_t *area = &me->area;
    const pg_deco_t * deco = sel_deco( me);
    int x0 = area->left;
    int y0 = area->top;
    int x1 = area->right;
    int y1 = area->bot;

    uint flags = deco->flags & (PG_F_TRANSP | PG_F_BORD | PG_F_ROUNDED);  // Leave only useful flags

    if ( ! (flags & PG_F_BORD))     // No need to draw border
    {
        if (! (flags & PG_F_TRANSP))
            pg_draw_fill( x0, y0, x1, y1, deco->bgcol);
        return;
    }

    if ( ! (flags & PG_F_ROUNDED))  // border, but no rounding at all, draw simple frame
    {
        if (flags & PG_F_TRANSP)    // transparent background, draw just frame and return
        {
            pg_draw_frame( x0, y0, x1, y1, deco->bordcol);
        }
        else                        // non-transparent background, draw bg and frame
        {
            if (deco->bordcol == deco->bgcol)   // same color, just a fill
            {
                pg_draw_fill( x0, y0, x1, y1, deco->bgcol);
            }
            else                                // different colors, bg and frame
            {
                pg_draw_frame( x0, y0, x1, y1, deco->bordcol);
                pg_draw_fill( x0+1, y0+1, x1-1, y1-1, deco->bgcol);
            }
        }
        return;
    }

    // So, border and rounding

    if ( !(flags & PG_F_TRANSP))
    {
        pg_draw_fill( x0 + 1, y0 + 1, x1 - 1, y1 - 1, deco->bgcol);
    }

    if ( (flags & PG_F_ROUNDED) == PG_F_ROUNDED)    // Full rounding, easy stuff
    {
        pg_draw_rounded_frame( x0, y0, x1, y1, deco->bordcol);
        return;
    }

    // So, partial rounding
    pg_draw_vline( x0, y0 + 1, y1 - 1, deco->bordcol);  // Draw this vertical lines and leave gaps
    pg_draw_vline( x1, y0 + 1, y1 - 1, deco->bordcol);

    int x00 = x0;       // X's for second line
    int x11 = x1;       //

    if ( flags & PG_F_ROUND_NW)
        x0++;

    if ( flags & PG_F_ROUND_NE)
        x1--;

    if ( flags & PG_F_ROUND_SW)
        x00++;

    if ( flags & PG_F_ROUND_SE)
        x11--;

    pg_draw_hline( x0, x1, y0, deco->bordcol);
    pg_draw_hline( x00, x11, y1, deco->bordcol);
}

/**-----------------------------------------------------------------------------
  Render object (dispatch actual work to class renderer)

  @param Me  - object

  @return  0 if rendering was not executed, 1 otherwise
------------------------------------------------------------------------------*/
static inline int render_object(pg_obj_t *me)
{
    if ( !in_dirty_rect(me))       // No need to draw
        return 0;

    if ( me->flags & PG_F_HIDDEN)      // No need to draw
        return 0;

    base_rend( me);

    if (obj_hnd[me->type].rend)
        obj_hnd[me->type].rend( me);

#if PG_USE_PDHOOK
    if (me->postdraw_hook)
    {
        pg_fast_area_t oldRect = dirty_rect;
        intersect_dirty(&dirty_rect, &me->area);
        pg_set_clip_via_area( &dirty_rect);
        me->postdraw_hook(me);
        dirty_rect = oldRect;
        pg_set_clip_via_area( &dirty_rect);
    }
#endif

    return 1;
}

int pg_render( void)
{
    if ( !need_update)
        return 0;

    pg_obj_t *obj;
    tree_iterator_t it;

    need_update = 0;
    int do_visit_childs = 1;                                        // Should we dig to subtrees

    //printf("Dirty area is %d x %d\n", dirtyRect.Right - dirtyRect.Left, dirtyRect.Bot - dirtyRect.Top);
    pg_set_clip_via_area( &dirty_rect);

    do_visit_childs = render_object( pg_screen);
    tree_iterator_preorder_init( &it, pg_screen);

    while ( (obj = tree_iterator_preorder_next( &it, do_visit_childs)))
        do_visit_childs = render_object(obj);                       // Skip whole no-need-to-render subtrees

    clear_dirty_rect();
    return 1;
}

/**-----------------------------------------------------------------------------
  Lift object to make it more foreground among brothers

  @param Me  - object
------------------------------------------------------------------------------*/
void pg_lift( pg_obj_t *me)
{
    if (!me->parent)
        return;

    if ( !me->next_bro)                  // Check if last object  (case a)
        return;

    pg_obj_t *next = me->next_bro;

    if ( me->parent->first_child == me)      // We are a first child (case b)
    {
        me->parent->first_child = next;
    }
    else
    {
        // Not effective - extra check inside
        get_prev_brother( me)->next_bro = next;
    }

    me->next_bro = next->next_bro;
    next->next_bro = me;

    set_dirty( me);
}

/**-----------------------------------------------------------------------------
  Unlift object to make it more background among brothers

  @param Me  - object
------------------------------------------------------------------------------*/
void pg_lower( pg_obj_t *me)
{
    if (!me->parent)
        return;

    if ( me->parent->first_child == me)      // Nowhere to lower (case a)
        return;

    pg_obj_t *prev = get_prev_brother( me);

    if ( me->parent->first_child == prev)    // Prev brother is a first child (case b)
    {
        me->parent->first_child = me;
    }
    else                                    // case c
    {
        // Not effective - double search !
        get_prev_brother( prev)->next_bro = me;
    }
    prev->next_bro = me->next_bro;
    me->next_bro = prev;

    set_dirty( me);
}

/**-----------------------------------------------------------------------------
  Execute object's method

  @param Me  - object
  @param Id  - method id
  @param Val  - method argument
------------------------------------------------------------------------------*/
void pg_act( pg_obj_t *me, enum pg_acts_e id, pg_val_t val)
{
    obj_hnd[me->type].act( me, id, val);
}

/**-----------------------------------------------------------------------------
  Make object most foreground among brothers

  @param Me  - object
------------------------------------------------------------------------------*/
void pg_tofront( pg_obj_t *me)
{
    if (!me->parent)
        return;

    if ( !me->next_bro)                                                      // Already
        return;

    pg_obj_t *last;
    for ( last = me->next_bro; last->next_bro; last = last->next_bro);// Get last object of same rank

    last->next_bro = me;                                                     // Attach

    if ( me->parent->first_child == me)                                          // We're a first child
    {
        me->parent->first_child = me->next_bro;
    }
    else
    {
        pg_obj_t *prev = get_prev_brother( me);
        prev->next_bro = me->next_bro;
    }

    me->next_bro = 0;
    set_dirty( me);
}

/**-----------------------------------------------------------------------------
  Make object most background among brothers

  @param Me  - object
------------------------------------------------------------------------------*/
void pg_toback( pg_obj_t *me)
{
    if (!me->parent)
        return;

    if ( me->parent->first_child == me)      // Already
        return;

    pg_obj_t *prev = get_prev_brother( me);
    prev->next_bro = me->next_bro;    // Bypass meself in chain

    me->next_bro = me->parent->first_child;
    me->parent->first_child = me;            // Become a first child
    set_dirty( me);
}

/**-----------------------------------------------------------------------------
  Force redrawing of object

  @param Me  - object
------------------------------------------------------------------------------*/
void pg_force_update( pg_obj_t *me)
{
    set_dirty( me);
}

/**-----------------------------------------------------------------------------
  Init PG
------------------------------------------------------------------------------*/
void pg_init( void)
{
   ;
}

/**-----------------------------------------------------------------------------
  Move object to new coordinates (limited by parent box)

  ! Fails for some reason if coordinates
  ! are out of screen bounds

  @param Me  - object
  @param dX  - delta X
  @param dY  - delta Y
------------------------------------------------------------------------------*/
void pg_move( pg_obj_t *me, int dx, int dy)
{
    if (!me->parent)
        return;

    pg_area_t *par_area = &me->parent->area;

    if ( dx > 0)
        dx = MIN( dx, par_area->right - me->area.right);    // Limit by right border
    else
        dx = MAX( dx, par_area->left - me->area.left);      // Limit by left border

    if ( dy > 0)
        dy = MIN( dy, par_area->bot - me->area.bot);        // Limit by bottom
    else
        dy = MAX( dy, par_area->top - me->area.top);        // Limit by top

    set_dirty( me);
    me->area.left += dx;
    me->area.right += dx;
    me->area.top += dy;
    me->area.bot += dy;
    add_dirty( &me->area);                                 // Extend dirty rect to include new position

    if ( me->first_child)
    {
        pg_obj_t *tmp_obj;
        tree_iterator_t it;
        tree_iterator_preorder_init( &it, me);

        while ( (tmp_obj = tree_iterator_preorder_next( &it, 1)))
        {
            tmp_obj->area.left += dx;
            tmp_obj->area.right += dx;
            tmp_obj->area.top += dy;
            tmp_obj->area.bot += dy;
        }
    }

}

/**-----------------------------------------------------------------------------
  Move object to new coordinates (relative to parent box)

  @param Me  - object
  @param X0  - new X0
  @param Y0  - new Y0
------------------------------------------------------------------------------*/
void pg_moveto( pg_obj_t *me, uint x0, uint y0)
{
    if (!me->parent)
        return;

    const pg_area_t *par_area = &me->parent->area;
    pg_move(me, par_area->left + x0 - me->area.left, par_area->top + y0 - me->area.top);
}

/**-----------------------------------------------------------------------------
  Find specific child of object

  @param Me  - object
  @param Num - child number

  @return pointer to child or 0
------------------------------------------------------------------------------*/
pg_obj_t *pg_get_child( const pg_obj_t *me, int num)
{
    for ( pg_obj_t *child = me->first_child; child; child = child->next_bro, num--)
    {
        if (!num)
            return child;
    }
    return NULL;
}

pg_obj_t *pg_get_parent( const pg_obj_t *obj)
{
    return obj->parent;
}

/**-----------------------------------------------------------------------------
  Get next brother of object

  @param Me  - object
  @return  pointer to brother object
------------------------------------------------------------------------------*/
pg_obj_t * pg_get_next_brother( const pg_obj_t *me)
{
    return me->next_bro;
}

static int apply_flag_recursively(pg_obj_t *root, uint32_t flag, pg_val_t val)
{
    int rc = apply_flag32( &root->flags, flag, val);

    if (root->first_child)
    {
        tree_iterator_t it;
        pg_obj_t *obj;

        tree_iterator_preorder_init(&it, root);
        while ((obj = tree_iterator_preorder_next( &it, 1)))
            rc |= apply_flag32( &obj->flags, flag, val);
    }

    return rc;
}

/**-----------------------------------------------------------------------------
  Apply base attibute to object. Each object must implement these.

  @param Me  - object
  @param Id  - attribute id
  @param Val  - attribute value

  @return  0 if object shouldn't be redrawn, other value if should
------------------------------------------------------------------------------*/
int pg_base_set( pg_obj_t *me, pg_id_t id, pg_val_t val)
{
    switch ( id)
    {
    case PG_A_THEME:
        if (me->theme == (pg_theme_t *) val)
            return 0;
        me->theme = (pg_theme_t *) val;
        return 1;

    case PG_A_HIDDEN:
        return apply_flag32( &me->flags, PG_F_HIDDEN, val);

#if PG_USE_USERDATA
    case PG_A_USER_DATA:   me->usr_data = val;                 return 1;   // Should we really redraw if tag chenged ?
#endif
#if PG_USE_PDHOOK
    case PG_A_PDHOOK:      me->postdraw_hook = (pg_hook_t) val; return 1;
#endif
    case PG_A_HL :
        return (me->flags & PG_F_PROPAGATE_DECO) ? apply_flag_recursively(me, PG_F_HL, val) : apply_flag32( &me->flags, PG_F_HL, val);

    case PG_A_OFF:
        return (me->flags & PG_F_PROPAGATE_DECO) ? apply_flag_recursively(me, PG_F_OFF, val) : apply_flag32( &me->flags, PG_F_OFF, val);

    case PG_A_PROPAGATE_DECO:
        return apply_flag32(&me->flags, PG_F_PROPAGATE_DECO, val);

//  case IA_LEFT:       Me->Area.Left = Val;                return 0;
//  case IA_TOP:        Me->Area.Top = Val;                 return 0;
//  case IA_RIGHT:      Me->Area.Right = Val;               return 0;
//  case IA_BOT:        Me->Area.Bot = Val;                 return 0;
    }
    return 0;
}


/**-----------------------------------------------------------------------------
  Get value of base attribute from object

  @param Me  - object
  @param Id  - attribute id
  @param Val  - place to store attribute value

  @return  0 if error
------------------------------------------------------------------------------*/
int pg_base_get( pg_obj_t *me, pg_id_t id, pg_val_t *val)
{
    switch ( id)
    {
    case PG_A_THEME:       *val = (pg_val_t) me->theme;           return 1;
    case PG_A_HIDDEN:      *val = me->flags & PG_F_HIDDEN;             return 1;
#if PG_USE_USERDATA
    case PG_A_USER_DATA:   *val = me->usr_data;                    return 1;
#endif
#if PG_USE_PDHOOK
    case PG_A_PDHOOK:      *val = (pg_val_t) me->postdraw_hook;    return 1;
#endif
    case PG_A_HL:          *val = me->flags & PG_F_HL;                 return 1;
    case PG_A_OFF:         *val = me->flags & PG_F_OFF;                return 1;
//  case IA_LEFT:       *Val = Me->Area.Left;                       return 1;
//  case IA_TOP:        *Val = Me->Area.Top;                        return 1;
//  case IA_RIGHT:      *Val = Me->Area.Right;                      return 1;
//  case IA_BOT:        *Val = Me->Area.Bot;                        return 1;
    case PG_ROA_TYPE:      *val = me->type;                        return 1;
    case PG_ROA_FIRST_CHILD :
                        *val = (pg_val_t) pg_get_child(me, 0); return *val;

    case PG_ROA_NEXT_BROTHER:
                        *val = (pg_val_t) pg_get_next_brother(me); return *val;
	case PG_ROA_AREA:
						*val = (pg_val_t) &me->area; return 1;
    }
    return 0;
}

/**-----------------------------------------------------------------------------
  Execute object's method

  @param Me  - object
  @param Id  - method id
  @param Val  - argument of method
------------------------------------------------------------------------------*/
void pg_base_act( pg_obj_t *me, pg_id_t id, pg_val_t val)
{
    switch ( id)
    {
    case PG_M_DIRTY:    set_dirty( me);                        return;
    case PG_M_HIDE:     pg_hide( me,1);                        return;
    case PG_M_UNHIDE:   pg_hide( me,0);                        return;
    case PG_M_LIFT:     pg_lift( me);                          return;
    case PG_M_LOWER:    pg_lower( me);                         return;
    case PG_M_SUICIDE:  pg_delete( me);                        return;
    case PG_M_TOFRONT:  pg_tofront(me);                        return;
    case PG_M_TOBACK:   pg_toback(me);                         return;
    }
}

/**-----------------------------------------------------------------------------
  Find object by type and get attribute

  @param Me  - object
  @param Id  - attribute id
  @param Val  - attribute value

  @return  0 if error
------------------------------------------------------------------------------*/
int pg_get( pg_obj_t *me, pg_id_t id, pg_val_t *val)
{
    if ( !is_inited( me))     // Couldn't return nothing - we're not inited yet
            return 0;

    return obj_hnd[me->type].get( me, id, val);
}

pg_obj_t *pg_create( enum pg_objs_e type, pg_obj_t *parent, int left, int top, int right, int bot, pg_ctaglist_p tl)
{
    pg_obj_t *me = obj_hnd[type].create();

    if ( !me) return NULL;

    me->type = type;                   // For extra safety

//  if ( Area)
//      Me->Area = *Area;

    if ( tl )
      set_attr_taglist( me, tl);

        _attach( me, parent);
    init_coords( me, left, top, right, bot);

    if (obj_hnd[me->type].init)
        obj_hnd[me->type].init( me);

    me->flags |= PG_F_INITED;
    set_dirty( me);
    return me;
}

void pg_set( pg_obj_t *me, enum pg_attrs_e id, pg_val_t val)
{
    if ( set_attr(me, id, val))
        set_dirty( me);
}

void pg_set_via_taglist( pg_obj_t *me, pg_ctaglist_p tl)
{
    if ( set_attr_taglist( me, tl))
        set_dirty( me);
}

void pg_kill(pg_obj_t *me)
{
    if (! me) return;

    /** Traverse should be postorder, but could be preorder if we guarantee freed memory will not change or become
        allocated till the end of traverse. */

    pg_obj_t *obj;

    if ( me->first_child)                            // Kill initiator's children if any
    {
        tree_iterator_t it;
        tree_iterator_preorder_init( &it, me);
        while ( (obj = tree_iterator_preorder_next( &it, 1)))
            pg_delete( obj);
    }

    obj = me->parent;

    if ( obj)                                       // Deaman was attached to something
    {
        if ( obj->first_child == me)                 // Deadman was a first child
        {
            obj->first_child = me->next_bro;      // Make a deadman's younger brother a first child
                                                    // (if no brother exists, null will be copied)
        }
        else                                        // Nope, he was linked to some other child
        {
            obj = obj->first_child;                  // Visit his brothers, looking for link
            while ( obj->next_bro != me)
                obj = obj->next_bro;

            obj->next_bro = me->next_bro;    // Link deadman's younger brother (or copy zero) to older brother
        }
    }

    set_dirty( me);

    // Dirty hack
    if ( me == pg_screen)
        me->first_child = me->next_bro = 0;

    pg_delete( me);
}

const pg_fast_area_t *pg_get_dirty_rect(void)
{
    if (! need_update) return NULL;
    return &dirty_rect;
}

void pg_animate(void)
{
    pg_abitmaps_animate();
}

// Print tree nodes in preorder
void pg_print_tree( void)
{
    pg_obj_t *obj;
    tree_iterator_t it;

    tree_iterator_preorder_init( &it, pg_screen);
    while ( (obj = tree_iterator_preorder_next( &it, 1)))
    {
        PG_LOG( "%d - x0=%d, y0=%d, x1=%d, y1=%d\n", obj->type, obj->area.left, obj->area.top, obj->area.right, obj->area.bot);
    }
}
