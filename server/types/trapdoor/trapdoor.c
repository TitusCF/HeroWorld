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

/** @file trapdoor.c
 * The implementation of the Trapdoor class of objects.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static method_ret trapdoor_type_move_on(ob_methods *context, object *trap, object *victim, object *originator);

/**
 * Initializer for the TRAPDOOR object type.
 */
void init_type_trapdoor(void) {
    register_move_on(TRAPDOOR, trapdoor_type_move_on);
}

/**
 * Move on this Trapdoor object.
 * @param context The method context
 * @param trap The Trapdoor we're moving on
 * @param victim The object moving over this one
 * @param originator The object that caused the move_on event
 * @return METHOD_OK
 */
static method_ret trapdoor_type_move_on(ob_methods *context, object *trap, object *victim, object *originator) {
    int max, sound_was_played;
    object *ab, *ab_next;
    if (common_pre_ob_move_on(trap, victim, originator) == METHOD_ERROR)
        return METHOD_OK;
    if (!trap->value) {
        int tot;

        for (ab = trap->above, tot = 0; ab != NULL; ab = ab->above)
            if ((ab->move_type && trap->move_on) || ab->move_type == 0)
                tot += (ab->nrof ? ab->nrof : 1)*ab->weight+ab->carrying;

        if (!(trap->value = (tot > trap->weight) ? 1 : 0)) {
            common_post_ob_move_on(trap, victim, originator);
            return METHOD_OK;
        }
        SET_ANIMATION(trap, trap->value);
        object_update(trap, UP_OBJ_FACE);
    }

    for (ab = trap->above, max = 100, sound_was_played = 0; --max && ab; ab = ab_next) {
        /* need to set this up, since if we do transfer the object,
         * ab->above would be bogus
         */
        ab_next = ab->above;

        if ((ab->move_type && trap->move_on) || ab->move_type == 0) {
            if (!sound_was_played) {
                play_sound_map(SOUND_TYPE_GROUND, trap, 0, "fall hole");
                sound_was_played = 1;
            }
            draw_ext_info(NDI_UNIQUE, 0, ab, MSG_TYPE_APPLY, MSG_TYPE_APPLY_TRAP,
                  "You fall into a trapdoor!");
            transfer_ob(ab, (int)EXIT_X(trap), (int)EXIT_Y(trap), 0, ab);
        }
    }
    common_post_ob_move_on(trap, victim, originator);
    return METHOD_OK;
}
