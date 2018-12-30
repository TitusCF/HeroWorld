/*
    CrossFire, A Multiplayer game for X-windows

    Copyright (C) 2008 Crossfire Development Team

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

/**
 * @file
 * The implementation of @ref page_type_55 "marker" objects.
 */

#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static method_ret marker_type_process(ob_methods *context, object *op);
static method_ret marker_type_trigger(ob_methods *context, object *op, object *cause, int state);

/**
 * Initializer for the @ref page_type_55 "marker" object type.
 */
void init_type_marker(void) {
    register_process(MARKER, marker_type_process);
    register_trigger(TRIGGER_MARKER, marker_type_trigger);
}

/**
 * Move function for @ref page_type_55 "marker" objects.
 *
 * When moved, a marker will search for a player sitting above
 * it, and insert an invisible, weightless force into him
 * with a specific code as the slaying field.
 *
 * At that time, it writes the contents of its own message
 * field to the player.  The marker will decrement hp to
 * 0 and then delete itself every time it grants a mark.
 * unless hp was zero to start with, in which case it is infinite.
 * @author peterm@soda.csua.berkeley.edu
 *
 * @param op
 * marker to move. Can be removed if it reached its marking limit.
 */
static void move_marker(object *op) {
    object *tmp, *tmp2;

    /*
    * markers not on a map for any reason should not crash server
    */
    if (!op->map) {
        return;
    }

    for (tmp = GET_MAP_OB(op->map, op->x, op->y); tmp != NULL; tmp = tmp->above) {
        if (tmp->type == PLAYER) { /* we've got someone to MARK */
            /* cycle through his inventory to look for the MARK we want to
            * place
            */
            tmp2 = object_find_by_type_and_slaying(tmp, FORCE, op->slaying);
            /* if we didn't find our own MARK */
            if (tmp2 == NULL) {
                object *force = create_archetype(FORCE_NAME);

                force->speed = 0;
                if (op->stats.food) {
                    force->speed = 0.01;
                    force->speed_left = -op->stats.food;
                }
                object_update_speed(force);
                /* put in the lock code */
                force->slaying = add_string(op->slaying);

                if (op->lore)
                    force->lore = add_string(op->lore);

                object_insert_in_ob(force, tmp);
                if (op->msg)
                    draw_ext_info(NDI_UNIQUE|NDI_NAVY, 0, tmp, MSG_TYPE_MISC, MSG_SUBTYPE_NONE,
                        op->msg);

                if (op->stats.hp > 0) {
                    op->stats.hp--;
                    if (op->stats.hp == 0) {
                        /* marker expires--granted mark number limit */
                        object_remove(op);
                        object_free_drop_inventory(op);
                        return;
                    }
                }
            } /* if tmp2 == NULL */
        } /* if tmp->type == PLAYER */
    } /* For all objects on this space */
}

/**
 * Processes a @ref page_type_55 "marker".
 * @param context The method context
 * @param op The marker to process
 * @retval METHOD_OK
 */
static method_ret marker_type_process(ob_methods *context, object *op) {
    move_marker(op);
    return METHOD_OK;
}

/**
 * A @ref page_type_55 "marker" is triggered.
 * @param context Ignored.
 * @param op The object being triggered
 * @param cause Ignored.
 * @param state Ignored.
 * @retval METHOD_OK
 */
static method_ret marker_type_trigger(ob_methods *context, object *op, object *cause, int state) {
    move_marker(op);
    return METHOD_OK;
}
