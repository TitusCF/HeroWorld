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

/** @file check_inv.c
 * The implementation of the Inventory Checker class of objects.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static method_ret check_inv_type_move_on(ob_methods *context, object *trap, object *victim, object *originator);

/**
 * Initializer for the CHECK_INV object type.
 */
void init_type_check_inv(void) {
    register_move_on(CHECK_INV, check_inv_type_move_on);
}
/**
 * Move on this Inventory Checker object.
 * @param context The method context
 * @param trap The Inventory Checker we're moving on
 * @param victim The object moving over this one
 * @param originator The object that caused the move_on event
 * @return METHOD_OK
 */
static method_ret check_inv_type_move_on(ob_methods *context, object *trap, object *victim, object *originator) {
    if (common_pre_ob_move_on(trap, victim, originator) == METHOD_ERROR)
        return METHOD_OK;
    check_inv(victim, trap);
    common_post_ob_move_on(trap, victim, originator);
    return METHOD_OK;
}
