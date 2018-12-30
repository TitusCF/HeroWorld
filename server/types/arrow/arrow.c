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

/** @file arrow.c
 * The implementation of the Arrow class of objects.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static method_ret arrow_type_process(ob_methods *context, object *op);

/**
 * Initializer for the ARROW object type.
 */
void init_type_arrow(void) {
    register_move_on(ARROW, common_projectile_move_on);
    register_process(ARROW, arrow_type_process);
}

/**
 * Move an arrow along its course. Uses common_process_projectile.
 * @param context The method context
 * @param op The arrow being moved.
 * @return METHOD_ERROR if op is not in a map, otherwise METHOD_OK
 */
static method_ret arrow_type_process(ob_methods *context, object *op) {
    if (op->map == NULL) {
        LOG(llevError, "BUG: Arrow had no map.\n");
        object_remove(op);
        object_free_drop_inventory(op);
        return METHOD_ERROR;
    }

    /* if the arrow is moving too slow.. stop it.  0.5 was chosen as lower
       values look rediculous. */
    if (op->speed < 0.5 && op->type == ARROW) {
        stop_projectile(op);
        return METHOD_OK;
    }

    return common_process_projectile(context, op);
}
