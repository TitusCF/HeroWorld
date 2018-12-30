/*
    CrossFire, A Multiplayer game for X-windows

    Copyright (C) 2008,2010 Mark Wedel & Crossfire Development Team
    Copyright (C) 1992 Frank Tore Johansen

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    The authors can be reached via e-mail at crossfire-devel@real-time.com
*/
/** @file food.c
 * The implementation of the Food class of objects.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>
#include <math.h>

static method_ret dragon_focus_type_apply(ob_methods *context, object *focus, object *applier, int aflags);


/**
 * Initializer for the food object type.
 */
void init_type_dragon_focus(void) {
    register_apply(DRAGON_FOCUS, dragon_focus_type_apply);
}

/**
 * Handles applying food.
 * If player is applying, takes care of messages and dragon special food.
 * @param context The method context
 * @param focus - The focus to apply
 * @param applier The player attempting to apply the food
 * @param aflags Special flags (always apply/unapply)
 * @return METHOD_OK unless failure for some reason.
 */
static method_ret dragon_focus_type_apply(ob_methods *context, object *focus, object *applier, int aflags) {

    object *abil = NULL;    /* pointer to dragon ability force*/

    if (applier->type != PLAYER)
        return METHOD_ERROR;

    if (!is_dragon_pl(applier))
        return METHOD_ERROR;

    abil = object_find_by_type_and_arch_name(applier, FORCE, "dragon_ability_force");

    if (abil == NULL)
        return METHOD_ERROR;

    abil->stats.exp = focus->stats.exp;

    if (focus->face) {
        applier->face = focus->face;

        /* It only makes sense to change the animation if the
         * face is also changing.
         */
        if (focus->animation_id)
            applier->animation_id = focus->animation_id;
    }
    if (focus->title) {
        if (applier->title) free_string(applier->title);
        applier->title = add_refcount(focus->title);
    }
    return 0;
}
