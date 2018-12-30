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

/** @file player_mover.c
 * The implementation of the Player-mover class of objects.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static method_ret player_mover_type_move_on(ob_methods *context, object *trap, object *victim, object *originator);

/**
 * Initializer for the PLAYERMOVER object type.
 */
void init_type_player_mover(void) {
    register_move_on(PLAYERMOVER, player_mover_type_move_on);
}
/**
 * Move on this Player Mover object.
 * @param context The method context
 * @param trap The Player Mover we're moving on
 * @param victim The object moving over this one
 * @param originator The object that caused the move_on event
 * @return METHOD_OK
 */
static method_ret player_mover_type_move_on(ob_methods *context, object *trap, object *victim, object *originator) {
    if (common_pre_ob_move_on(trap, victim, originator) == METHOD_ERROR)
        return METHOD_OK;
    if (trap->attacktype
    && (trap->level || victim->type != PLAYER)
    && !should_director_abort(trap, victim)) {
        if (!trap->stats.maxsp)
            trap->stats.maxsp = 2;

        /* Is this correct?  From the docs, it doesn't look like it
         * should be divided by trap->speed
         */
        victim->speed_left = -FABS(trap->stats.maxsp*victim->speed/trap->speed);

        /* Just put in some sanity check.  I think there is a bug in the
         * above with some objects have zero speed, and thus the player
         * getting permanently paralyzed.
         */
        if (victim->speed_left < -50.0)
            victim->speed_left = -50.0;
    }
    common_post_ob_move_on(trap, victim, originator);
    return METHOD_OK;
}
