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

/** @file deep_swamp.c
 * The implementation of the Deep Swamp class of objects.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static method_ret deep_swamp_type_process(ob_methods *context, object *op);
static method_ret deep_swamp_type_move_on(ob_methods *context, object *trap, object *victim, object *originator);

/**
 * Initializer for the DEEP_SWAMP object type.
 */
void init_type_deep_swamp(void) {
    register_move_on(DEEP_SWAMP, deep_swamp_type_move_on);
    register_process(DEEP_SWAMP, deep_swamp_type_process);
}

/**
 * Processes a Deep Swamp.
 * @param context The method context
 * @param op The swamp to process
 * @return Always METHOD_OK
 */
static method_ret deep_swamp_type_process(ob_methods *context, object *op) {
    object *above = op->above;
    object *nabove;
    int got_player = 0;

    while (above) {
        nabove = above->above;
        if (above->type == PLAYER
        && !(above->move_type&MOVE_FLYING)
        && above->stats.hp >= 0
        && !QUERY_FLAG(above, FLAG_WIZ)) {
            object *woodsman = object_find_by_type_subtype(above, SKILL, SK_WOODSMAN);
            got_player = 1;
            if (op->stats.food < 1) {
                LOG(llevDebug, "move_deep_swamp(): player is here, but state is %d\n", op->stats.food);
                op->stats.food = 1;
            }
            if (op->stats.food < 10) {
                if (rndm(0, 2) == 0) {
                    draw_ext_info_format(NDI_UNIQUE, 0, above, MSG_TYPE_VICTIM, MSG_TYPE_VICTIM_SWAMP,
                        "You are down to your waist in the wet %s.",
                        op->name);
                        op->stats.food = woodsman ? op->stats.food+1 : 10;
                        above->speed_left -= op->move_slow_penalty;
                }
            } else if (op->stats.food < 20) {
                if (rndm(0, 2) == 0) {
                    draw_ext_info_format(NDI_UNIQUE|NDI_RED, 0, above, MSG_TYPE_VICTIM, MSG_TYPE_VICTIM_SWAMP,
                        "You are down to your NECK in the dangerous %s.",
                        op->name);
                    op->stats.food = woodsman ? op->stats.food+1 : 20;
                    snprintf(above->contr->killer, sizeof(above->contr->killer), "drowning in a %s", op->name);
                    above->stats.hp--;
                    above->speed_left -= op->move_slow_penalty;
                }
            } else if (rndm(0, 4) == 0) {
                op->stats.food = 0;
                draw_ext_info_format(NDI_UNIQUE|NDI_ALL, 1, NULL, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_PLAYER,
                    "%s disappeared into a %s.", above->name, op->name);
                snprintf(above->contr->killer, sizeof(above->contr->killer), "drowning in a %s", op->name);
                above->stats.hp = -1;
                kill_player(above, op); /* player dies in the swamp */
            }
        } else if (!QUERY_FLAG(above, FLAG_ALIVE)
        && !(above->move_type&MOVE_FLYING)
        && !(QUERY_FLAG(above, FLAG_IS_FLOOR))
        && !(QUERY_FLAG(above, FLAG_OVERLAY_FLOOR))
        && !(QUERY_FLAG(above, FLAG_NO_PICK))) {
            if (rndm(0, 2) == 0)
                object_decrease_nrof_by_one(above);
        }
        above = nabove;
    }
    if (!got_player)
        op->stats.food = 1;
    return METHOD_OK;
}

/**
 * Move on this Deep Swamp object.
 * @param context The method context
 * @param trap The Deep Swamp we're moving on
 * @param victim The object moving over this one
 * @param originator The object that caused the move_on event
 * @return METHOD_OK
 */
static method_ret deep_swamp_type_move_on(ob_methods *context, object *trap, object *victim, object *originator) {
    if (common_pre_ob_move_on(trap, victim, originator) == METHOD_ERROR)
        return METHOD_OK;
    if (victim->type == PLAYER
    && victim->stats.hp >= 0
    && !(victim->move_type&MOVE_FLYING)) {
        draw_ext_info_format(NDI_UNIQUE, 0, victim, MSG_TYPE_VICTIM, MSG_TYPE_VICTIM_SWAMP,
            "You are down to your knees in the %s.", trap->name);
        trap->stats.food = 1;
        victim->speed_left -= trap->move_slow_penalty;
    }
    common_post_ob_move_on(trap, victim, originator);
    return METHOD_OK;
}
