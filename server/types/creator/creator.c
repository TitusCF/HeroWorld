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
 * The implementation of @ref page_type_42 "creator" objects.
 */

#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static method_ret creator_type_process(ob_methods *context, object *op);
static method_ret creator_type_trigger(ob_methods *context, object *op, object *cause, int state);

/**
 * Initializer for the @ref page_type_42 "creator" object type.
 */
void init_type_creator(void) {
    register_process(CREATOR, creator_type_process);
    register_trigger(CREATOR, creator_type_trigger);
}

/**
 * Move function for @ref page_type_42 "creator" objects.
 *
 * It has the creator object create it's other_arch right on top of it.
 * connected:  what will trigger it
 * - hp:  how many times it may create before stopping
 * - lifesave:  if set, it'll never disappear but will go on creating
 *    everytime it's triggered
 * - other_arch:  the object to create
 *
 * Note this can create large objects, however, in that case, it
 * has to make sure that there is in fact space for the object.
 * It should really do this for small objects also, but there is
 * more concern with large objects, most notably a part being placed
 * outside of the map which would cause the server to crash
 *
 * @param creator
 * creator to move.
 */
static void move_creator(object *creator) {
    object *new_ob;

    if (!QUERY_FLAG(creator, FLAG_LIFESAVE) && --creator->stats.hp < 0) {
        creator->stats.hp = -1;
        return;
    }

    if (creator->inv != NULL) {
        int i;
        object *ob_to_copy;

        /* select random object from inventory to copy */
        ob_to_copy = creator->inv;
        i = 1;
        FOR_BELOW_PREPARE(creator->inv, ob) {
            if (rndm(0, i) == 0) {
                ob_to_copy = ob;
            }
            i++;
        } FOR_BELOW_FINISH();
        new_ob = object_create_clone(ob_to_copy);
        CLEAR_FLAG(new_ob, FLAG_IS_A_TEMPLATE);
        object_unset_flag_inv(new_ob, FLAG_IS_A_TEMPLATE);
    } else {
        if (creator->other_arch == NULL) {
            LOG(llevError, "move_creator: Creator doesn't have other arch set: %s (%s, %d, %d)\n", creator->name ? creator->name : "(null)", creator->map->path, creator->x, creator->y);
            return;
        }

        new_ob = object_create_arch(creator->other_arch);
        fix_generated_item(new_ob, creator, 0, 0, GT_MINIMAL);
    }

    /* Make sure this multipart object fits */
    if (new_ob->arch->more && ob_blocked(new_ob, creator->map, creator->x, creator->y)) {
        object_free_drop_inventory(new_ob);
        return;
    }

    if (creator->level != 0)
        new_ob->level = creator->level;

    object_insert_in_map_at(new_ob, creator->map, creator, 0, creator->x, creator->y);
    if (QUERY_FLAG(new_ob, FLAG_FREED))
        return;

    if (creator->slaying) {
        FREE_AND_COPY(new_ob->name, creator->slaying);
        if (new_ob->title) {
            FREE_AND_CLEAR_STR(new_ob->title);
        }
    }
}

/**
 * Processes a @ref page_type_42 "creator".
 * @param context The method context
 * @param op The teleporter to process
 * @retval METHOD_OK
 */
static method_ret creator_type_process(ob_methods *context, object *op) {
    move_creator(op);
    return METHOD_OK;
}

/**
 * A @ref page_type_42 "creator" is triggered.
 * @param context Ignored.
 * @param op The object being triggered
 * @param cause Ignored.
 * @param state Ignored.
 * @retval METHOD_OK
 */
static method_ret creator_type_trigger(ob_methods *context, object *op, object *cause, int state) {
    move_creator(op);
    return METHOD_OK;
}
