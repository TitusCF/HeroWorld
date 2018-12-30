/*
    CrossFire, A Multiplayer game for X-windows

    Copyright (C) 2007 Mark Wedel & Crossfire Development Team
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

/** @file thrown_object.c
 * The implementation of the Thrown Object class of objects.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static method_ret thrown_object_type_process(ob_methods *context, object *op);

/**
 * Initializer for the THROWN_OBJ object type.
 */
void init_type_thrown_object(void) {
    register_move_on(THROWN_OBJ, common_projectile_move_on);
    register_process(THROWN_OBJ, thrown_object_type_process);
}

/**
 * Move a thrown object along its course. Uses common_process_projectile.
 * @param context The method context
 * @param op The thrown object being moved.
 * @return METHOD_ERROR if op is not in a map, otherwise METHOD_OK
 */
static method_ret thrown_object_type_process(ob_methods *context, object *op) {
    if (op->map == NULL) {
        LOG(llevError, "BUG: Thrown object had no map.\n");
        object_remove(op);
        object_free_drop_inventory(op);
        return METHOD_ERROR;
    }

    /* we need to stop thrown objects at some point. Like here. */
    if (op->type == THROWN_OBJ) {
        /* If the object that the THROWN_OBJ encapsulates disappears,
         * we need to have this object go away also - otherwise, you get
         * left over remnants on the map.  Where this currently happens
         * is if the player throws a bomb - the bomb explodes on its own,
         * but this object sticks around.  We could handle the cleanup in the
         * bomb code, but there are potential other cases where that could happen,
         * and it is easy enough to clean it up here.
         */
        if (op->inv == NULL) {
            object_remove(op);
            object_free_drop_inventory(op);
            return METHOD_OK;
        }
        if (op->last_sp-- < 0) {
            stop_projectile(op);
            return METHOD_OK;
        }
    }

    return common_process_projectile(context, op);
}
