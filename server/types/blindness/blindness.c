/*
    CrossFire, A Multiplayer game for X-windows

    Copyright (C) 2008 Crossfire Development Team
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

/**
 * @file
 * The implementation of the BLINDNESS class of objects.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static method_ret blindness_type_process(ob_methods *context, object *op);

/**
 * Initializer for the blindness object type.
 */
void init_type_blindness(void) {
    register_process(BLINDNESS, blindness_type_process);
}

/**
 * Move for ::BLINDNESS.
 * If blindness is finished, remove it from the victim's inventory.
 *
 * @param context The method context.
 * @param op Blindness being processed.
 * @return METHOD_OK
 */
static method_ret blindness_type_process(ob_methods *context, object *op) {
    if (--op->stats.food > 0)
        return METHOD_OK;
    CLEAR_FLAG(op, FLAG_APPLIED);
    if (op->env != NULL) {
        change_abil(op->env, op);
        fix_object(op->env);
    }
    object_remove(op);
    object_free_drop_inventory(op);
    return METHOD_OK;
}
