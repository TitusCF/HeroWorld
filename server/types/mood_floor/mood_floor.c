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
 * The implementation of @ref page_type_65 "mood floor" objects.
 */

#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static method_ret mood_floor_type_process(ob_methods *context, object *op);
static method_ret mood_floor_type_trigger(ob_methods *context, object *op, object *cause, int state);

/**
 * Initializer for the @ref page_type_65 "mood floor" object type.
 */
void init_type_mood_floor(void) {
    register_process(MOOD_FLOOR, mood_floor_type_process);
    register_trigger(MOOD_FLOOR, mood_floor_type_trigger);
}

/**
 * Main function for @ref page_type_65 "mood floor" objects.
 *
 * This routine makes monsters who are
 * standing on the 'mood floor' change their
 * disposition if it is different.
 * If floor is to be triggered must have
 * a speed of zero (default is 1 for all
 * but the charm floor type).
 * by b.t. thomas@nomad.astro.psu.edu
 * @param op
 * floor that activates
 * @param op2
 * object that caused op to activate. Should be either op (for floors with speed), or
 * a connected button a player pushed. Must not be NULL. Will be used for charming floors
 * to locate the player the monster will become a pet of.
 */
static void do_mood_floor(object *op, object *op2) {
    object *tmp;
    object *tmp2;

    if (op->map == NULL) {
        LOG(llevError, "mood floor not in a map but in %s\n", op->env ? op->env->name : "null??");
        op->speed = 0;
        object_update_speed(op);
        return;
    }

    tmp = map_find_by_flag(op->map, op->x, op->y, FLAG_MONSTER);
    /* doesn't effect players, and if there is a player on this space, won't also
    * be a monster here.
    */
    if (!tmp || tmp->type == PLAYER)
        return;

    switch (op->last_sp) {
    case 0:                     /* furious--make all monsters mad */
        if (QUERY_FLAG(tmp, FLAG_UNAGGRESSIVE))
            CLEAR_FLAG(tmp, FLAG_UNAGGRESSIVE);
        if (QUERY_FLAG(tmp, FLAG_FRIENDLY)) {
            CLEAR_FLAG(tmp, FLAG_FRIENDLY);
            remove_friendly_object(tmp);
            tmp->attack_movement = 0;
            /* lots of checks here, but want to make sure we don't
             * dereference a null value
             */
            if (tmp->type == GOLEM) {
                object *owner;

                owner = object_get_owner(tmp);
                if (owner != NULL
                && owner->type == PLAYER
                && owner->contr->ranges[range_golem] == tmp) {
                    owner->contr->ranges[range_golem] = NULL;
                    owner->contr->golem_count = 0;
                }
            }
            object_clear_owner(tmp);
        }
        break;

    case 1:                     /* angry -- get neutral monsters mad */
        if (QUERY_FLAG(tmp, FLAG_UNAGGRESSIVE)
        && !QUERY_FLAG(tmp, FLAG_FRIENDLY))
            CLEAR_FLAG(tmp, FLAG_UNAGGRESSIVE);
        break;

    case 2:                     /* calm -- pacify unfriendly monsters */
        if (!QUERY_FLAG(tmp, FLAG_UNAGGRESSIVE)) {
            SET_FLAG(tmp, FLAG_UNAGGRESSIVE);
            object_set_enemy(tmp, NULL);
        }
        break;

    case 3:                     /* make all monsters fall asleep */
        if (!QUERY_FLAG(tmp, FLAG_SLEEP))
            SET_FLAG(tmp, FLAG_SLEEP);
        break;

    case 4:                     /* charm all monsters */
        if (op == op2)
            break;           /* only if 'connected' */

        for (tmp2 = GET_MAP_OB(op2->map, op2->x, op2->y); tmp2->type != PLAYER; tmp2 = tmp2->above)
            if (tmp2->above == NULL)
                break;
        if (tmp2->type != PLAYER)
            break;
        object_set_owner(tmp, tmp2);
        SET_FLAG(tmp, FLAG_MONSTER);
        tmp->stats.exp = 0;
        SET_FLAG(tmp, FLAG_FRIENDLY);
        add_friendly_object(tmp);
        tmp->attack_movement = PETMOVE;
        break;

    default:
        break;
    }
}

/**
 * Processes a @ref page_type_65 "mood floor".
 * @param context The method context
 * @param op The mood_floor to process
 * @retval METHOD_OK
 */
static method_ret mood_floor_type_process(ob_methods *context, object *op) {
    do_mood_floor(op, op);
    return METHOD_OK;
}

/**
 * A @ref page_type_65 "mood floor" is triggered.
 * @param context Ignored.
 * @param op The object being triggered
 * @param cause Ignored.
 * @param state Ignored.
 * @retval METHOD_OK
 */
static method_ret mood_floor_type_trigger(ob_methods *context, object *op, object *cause, int state) {
    do_mood_floor(op, cause);
    return METHOD_OK;
}
