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

/** @file altar.c
 * The implementation of the Altar class of objects.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static method_ret altar_type_move_on(ob_methods *context, object *trap, object *victim, object *originator);

/**
 * Initializer for the ALTAR object type.
 */
void init_type_altar(void) {
    register_move_on(ALTAR, altar_type_move_on);
}

/**
 * Move on this Altar object.
 * @param context The method context
 * @param trap The Altar we're moving on
 * @param victim The object moving over this one
 * @param originator The object that caused the move_on event
 * @return METHOD_OK
 */
static method_ret altar_type_move_on(ob_methods *context, object *trap, object *victim, object *originator) {
    trap = HEAD(trap);

    if (common_pre_ob_move_on(trap, victim, originator) == METHOD_ERROR)
        return METHOD_OK;

    /* sacrifice victim on trap */
    /* Only players can make sacrifices on spell casting altars. */
    if (trap->inv && (!originator || originator->type != PLAYER)) {
        common_post_ob_move_on(trap, victim, originator);
        return METHOD_OK;
    }
    if (operate_altar(trap, &victim)) {
        /* Simple check.  Unfortunately, it means you can't cast magic bullet
         * with an altar.  We call it a Potion - altars are stationary - it
         * is up to map designers to use them properly.
         */
        if (trap->inv && trap->inv->type == SPELL) {
            draw_ext_info_format(NDI_BLACK, 0, originator, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
                "The altar casts %s.", trap->inv->name);
            cast_spell(originator, trap, 0, trap->inv, NULL);
        } else {
            trap->value = 1;  /* works only once */
            push_button(trap);
        }
    }
    common_post_ob_move_on(trap, victim, originator);
    return METHOD_OK;
}
