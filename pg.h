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

#ifndef __PG_H__
#define __PG_H__
/** Public */

#include "pg_config.h"
#include "pg_tagitems.h"
#include "pg_attributes.h"
#include "pg_utf8.h"
#include "pg_primitives.h"
#include "pg_theme.h"

typedef struct pg_obj_t pg_obj_t;
typedef pg_obj_t *pg_obj_p;
typedef void (* pg_hook_t)( pg_obj_t *obj);
typedef void (* pg_updhook_t)( pg_obj_t *obj, int Pos);

extern pg_obj_t *const pg_screen;                          // User-accessed root

/**
 * Create PG object
 *
 * @param type - object type (pg_objs_e)
 * @param parent - parent object or pg_screen
 * @param left - left coordinate relative to parent left
 * @param top - top coordinate relative to parent top
 * @param right - right coordinate relative to parent left (if > 0) or right ( <= 0)
 * @param bot - bottom coordinate relative to parent top (if > 0) or bottom ( <= 0)
 * @param tl - NULL-terminated taglist of attribute/value pairs to be applied at creation
 *
 * @return pg_obj_t* dynamically allocated PG object or NULL if memory allocation failed
 */
pg_obj_t *pg_create( enum pg_objs_e type, pg_obj_t *parent, int left, int top, int right, int bot, pg_ctaglist_p tl);

/**
 * Set attribute of PG object
 *
 * @param obj - PG object
 * @param id - attribute id
 * @param val - attribute value
 */
void pg_set(pg_obj_t *obj, enum pg_attrs_e id, pg_val_t val);

/**
 * Set multiple attributes of PG object.
 * Attributes are applied in order of appearance
 *
 * @param obj - PG object
 * @param tl - NULL-terminated taglist of attribute/value pairs
 */
void pg_set_via_taglist( pg_obj_t *obj, pg_ctaglist_p tl);

/**
 * Kill PG object.
 * It is ok to pass NULL pointer. Pass pg_screen is to kill all user-created objects
 *
 * @param obj - PG object
 */
void pg_kill( pg_obj_t *obj);

/**
 * Render screen
 *
 * @return int - 0 if no render was executed (no screen update is require)
 */
int pg_render( void);

/**
 * Get child of object by index
 *
 * @param obj - object to search child of
 * @param num - child index
 *
 * @return pg_obj_t* - PG object or NULL
 */
pg_obj_t *pg_get_child( const pg_obj_t *obj, int num);

/**
 * Get parent of object
 *
 * @param me - object to search parent of
 *
 * @return pg_obj_t* - PG object or NULL
 */
pg_obj_t *pg_get_parent( const pg_obj_t *obj);

/**
 * Get next brother of PG object
 *
 * @param obj - object to search brother of
 *
 * @return pg_obj_t* - brother of object or NULL
 */
pg_obj_t *pg_get_next_brother( const pg_obj_t *obj);

/**
 * Request update of object at the next pass of render
 *
 * @param obj - PG object
 */
void pg_force_update( pg_obj_t *obj);

/**
 * Push object down
 *
 * @param obj - PG object
 */
void pg_lower( pg_obj_t *obj);

/**
 * Pull object up
 *
 * @param obj - PG object
 */
void pg_lift( pg_obj_t *obj);

/**
 * Pull object to the foreground
 *
 * @param obj - PG object
 */
void pg_tofront( pg_obj_t *obj);

/**
 * Push object to the background
 *
 * @param obj - PG object
 */
void pg_toback( pg_obj_t *obj);

/**
 * Move object by a specified amount.
 * Movement is limited by parent object
 *
 * NOTE: may fail if coordinates are out of screen bounds
 *
 * @param obj - PG object
 * @param dx - delta X
 * @param dy - delta Y
 */
void pg_move( pg_obj_t *obj, int dx, int dy);

/**
 * Move object to a new coordinates relative to parent box
 * Movement is limited by parent object
 *
 * NOTE: object may be resized or behave strange if corner cases.
 * Use with care
 *
 * @param obj - PG object
 * @param x0 - new left
 * @param y0 - new top
 */
void pg_moveto( pg_obj_t *obj, uint x0, uint y0);

/**
 * Execute a method of object
 *
 * @param obj - PG object
 * @param id - method id
 * @param val - method arguments
 */
void pg_act( pg_obj_t* obj, enum pg_acts_e id, pg_val_t val);

/**
 * Get a value of PG object's attribute
 *
 * @param obj - PG object
 * @param id - attribute
 * @param val - value output
 *
 * @return int - 0 if get failed
 */
int pg_get( pg_obj_t *obj, pg_id_t id, pg_val_t *val);

/**
 * Get current dirty rectangle (right top point is included, closed ranges)
 *
 * @return const pg_fast_area_t* dirty rectange or NULL if everything up to date
 */
const pg_fast_area_t *pg_get_dirty_rect(void);

/**
 * PG animate clock. Call periodically to advance animation of bitmaps
 */
void pg_animate(void);

void pg_print_tree( void);

// Kind-of va-arg set and create
#define pg_create_via_tags( type, parent, l, t, r, b, ...)  \
  pg_create((type), (parent), (l), (t), (r), (b), (const pg_tag_t *) (pg_item_t []){__VA_ARGS__, PG_TAG_END, 0})

#define pg_set_via_tags(obj, ...)  \
  pg_set_via_taglist((obj), (const pg_tag_t *) (pg_item_t []){__VA_ARGS__, PG_TAG_END, 0})

#ifdef __PG_C__
/** Private */

enum base_flags_e
{
    PG_F_INITED =            1U << 0,
    PG_F_HIDDEN =            1U << 1,
    PG_F_HL =                1U << 2,
    PG_F_OFF =               1U << 3,
    PG_F_PROPAGATE_DECO  =   1U << 4,
    _PG_F_USER =             1U << 5,
};

/** Description of drawable object*/
struct pg_obj_t
{
    pg_area_t area;
    enum pg_objs_e type;        // Class id
    uint32_t flags;             // Some flags
    pg_theme_p theme;           // Pointer to current theme
    pg_obj_t *parent;           // Pointer to parent. 0 if nonattached
    pg_obj_t *first_child;      // Pointer to first child. 0 if no childs
    pg_obj_t *next_bro;         // Pointer to next brother. 0 if no younger brothers exist
#if PG_USE_PDHOOK
    pg_hook_t postdraw_hook;     // Hook to be executed immidiately after render. May be used for
                                // custom draw, prining debug, posting event etc.
#endif
#if PG_USE_USERDATA
    uint32_t usr_data;          // Space to store some user-defined info
#endif
};

typedef struct
{
    pg_obj_t *(*create)( void);                             // Create function (allocate memory and set some parameters)
    int (*set)( pg_obj_t *, pg_id_t, pg_val_t);             // Set parameters
    int (*get)( pg_obj_t *, pg_id_t, pg_val_t *);           // Get parameters
    void (*act)( pg_obj_t *, pg_id_t, pg_val_t);            // Do some action
    void (*init)( pg_obj_t *);                              // Init - called after creation and doing optional sets
    void (*rend)( pg_obj_t *);                              // Renderer
    void (*del)( pg_obj_t *);                               // Free memory
} obj_handler_t;

typedef struct
{
    pg_obj_t *root;     // Root for tree iterator
    pg_obj_t *curr;     // Current object for tree iterator
} tree_iterator_t;

static inline int is_inited( const pg_obj_t *Me) { return Me->flags & PG_F_INITED; }

static inline const pg_deco_t *sel_deco( const pg_obj_t *me)
{
    switch ( me->flags & (PG_F_OFF | PG_F_HL))
    {
    default:
    case 0:                     return &me->theme->norm;
    case PG_F_HL:               return &me->theme->hl;
    case PG_F_OFF:              return &me->theme->off;
    case PG_F_HL | PG_F_OFF:    return &me->theme->hloff;
    }
}

int pg_base_set( pg_obj_t *obj, pg_id_t id, pg_val_t val);
int pg_base_get( pg_obj_t *obj, pg_id_t id, pg_val_t *val);
void pg_base_act( pg_obj_t* me, pg_id_t id, pg_val_t val);

#endif
#endif
