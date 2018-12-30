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
/** @file projectile.c
 * This file contains code common to projectile objects. For now it is limited
 * to arrows and thrown objects.
 */
#include <global.h>
#include <sproto.h>
#include <ob_methods.h>
#include <ob_types.h>

/**
 * Handle an arrow or thrown object stopping.
 * @param op The arrow or thrown object that is stopping.
 */
void stop_projectile(object *op) {
    /* Lauwenmark: Handle for plugin stop event */
    object *event = op;
    tag_t tag;

    if (op->inv)
        event = op->inv;

    tag = event->count;
    execute_event(event, EVENT_STOP, NULL, NULL, NULL, SCRIPT_FIX_NOTHING);

    if (object_was_destroyed(event, tag)) {
        if (event != op) {
            object_remove(op);
            object_free2(op, FREE_OBJ_FREE_INVENTORY);
        }
        return;
    }

    if (op->inv) {
        object *payload = op->inv;

        object_remove(payload);
        object_clear_owner(payload);
        object_insert_in_map_at(payload, op->map, payload, 0, op->x, op->y);
        object_remove(op);
        object_free_drop_inventory(op);
    } else {
        op = fix_stopped_arrow(op);
        if (op)
            object_merge(op, NULL);
    }
}

/**
 * Move an arrow or thrown object along its course.
 * @param context The method context
 * @param op The arrow or thrown object being moved.
 * @todo Split this function up.
 * @return METHOD_ERROR if op is not in a map, otherwise METHOD_OK
 */

method_ret common_process_projectile(ob_methods *context, object *op) {
    object *tmp;
    sint16 new_x, new_y;
    int mflags;
    mapstruct *m;

    if (op->map == NULL) {
        LOG(llevError, "BUG: Projectile had no map.\n");
        object_remove(op);
        object_free_drop_inventory(op);
        return METHOD_ERROR;
    }

    /* Calculate target map square */
    new_x = op->x+DIRX(op);
    new_y = op->y+DIRY(op);

    m = op->map;
    mflags = get_map_flags(m, &m, new_x, new_y, &new_x, &new_y);

    if (mflags&P_OUT_OF_MAP) {
        stop_projectile(op);
        return METHOD_OK;
    }

    /* only need to look for living creatures if this flag is set */
    if (mflags&P_IS_ALIVE) {
        tmp = map_find_by_flag(m, new_x, new_y, FLAG_ALIVE);
        /* Not really fair, but don't let monsters hit themselves with
         * their own arrow - this can be because they fire it then
         * move into it.
         */
        if (tmp != NULL && tmp != object_get_owner(op)) {
            /* Found living object, but it is reflecting the missile.  Update
             * as below. (Note that for living creatures there is a small
             * chance that reflect_missile fails.)
             */

            if (QUERY_FLAG(tmp, FLAG_REFL_MISSILE)
            && (rndm(0, 99)) < (90-op->level/10)) {

                op->direction = absdir(op->direction+4);
                op->state = 0;
                if (GET_ANIM_ID(op)) {
                    object_update_turn_face(op);
                }
            } else {
                /* Attack the object. */
                op = hit_with_arrow(op, tmp);
                if (op == NULL)
                    return METHOD_OK;
             }
        } /* if this is not hitting its owner */
    } /* if there is something alive on this space */

    if (OB_TYPE_MOVE_BLOCK(op, GET_MAP_MOVE_BLOCK(m, new_x, new_y))) {
        int retry = 0;

        /* if the object doesn't reflect, stop the arrow from moving
         * note that this code will now catch cases where a monster is
         * on a wall but has reflecting - the arrow won't reflect.
         * Mapmakers shouldn't put monsters on top of wall in the first
         * place, so I don't consider that a problem.
         */
        if (!QUERY_FLAG(op, FLAG_REFLECTING) || !(rndm(0, 19))) {
            stop_projectile(op);
            return METHOD_OK;
        } else {
            /* If one of the major directions (n,s,e,w), just reverse it */
            if (op->direction&1) {
                op->direction = absdir(op->direction+4);
                retry = 1;
            }
            /* There were two blocks with identical code -
             * use this retry here to make this one block
             * that did the same thing.
             */
            while (retry < 2) {
                int left, right, mflags;
                mapstruct *m1;
                sint16  x1, y1;

                retry++;

                /* Need to check for P_OUT_OF_MAP: if the arrow is tavelling
                 * over a corner in a tiled map, it is possible that
                 * op->direction is within an adjacent map but either
                 * op->direction-1 or op->direction+1 does not exist.
                 */
                mflags = get_map_flags(op->map, &m1, op->x+freearr_x[absdir(op->direction-1)], op->y+freearr_y[absdir(op->direction-1)], &x1, &y1);
                left = (mflags&P_OUT_OF_MAP) ? 0 : OB_TYPE_MOVE_BLOCK(op, (GET_MAP_MOVE_BLOCK(m1, x1, y1)));

                mflags = get_map_flags(op->map, &m1, op->x+freearr_x[absdir(op->direction+1)], op->y+freearr_y[absdir(op->direction+1)], &x1, &y1);
                right = (mflags&P_OUT_OF_MAP) ? 0 : OB_TYPE_MOVE_BLOCK(op, (GET_MAP_MOVE_BLOCK(m1, x1, y1)));

                if (left == right)
                    op->direction = absdir(op->direction+4);
                else if (left)
                    op->direction = absdir(op->direction+2);
                else if (right)
                    op->direction = absdir(op->direction-2);

                mflags = get_map_flags(op->map, &m1, op->x+DIRX(op), op->y+DIRY(op), &x1, &y1);

                /* If this space is not out of the map and not blocked, valid space -
                 * don't need to retry again.
                 */
                if (!(mflags&P_OUT_OF_MAP)
                && !OB_TYPE_MOVE_BLOCK(op, GET_MAP_MOVE_BLOCK(m1, x1, y1)))
                    break;
            }
            /* Couldn't find a direction to move the arrow to - just
             * top it from moving.
             */
            if (retry == 2) {
                stop_projectile(op);
                return METHOD_OK;
            }
            /* update object image for new facing */
            /* many thrown objects *don't *have more than one face */
            if (GET_ANIM_ID(op))
                object_update_turn_face(op);
        } /* object is reflected */
    } /* object ran into a wall */

    /* decrease the speed as it flies. 0.05 means a standard bow will shoot
     * about 17 squares. Tune as needed.
     */
    op->speed -= 0.05;
    if (op->speed < 0.05) {
        stop_projectile(op);
        return METHOD_OK;
    }

    /* Move the arrow. */
    object_remove(op);
    object_insert_in_map_at(op, m, op, 0, new_x, new_y);
    return METHOD_OK;
}

/**
 * Move on this Thrown Object object.
 * @param context The method context
 * @param trap The thrown object or arrow we're moving on
 * @param victim The object moving over this one
 * @param originator The object that caused the move_on event
 * @return METHOD_OK
 */
method_ret common_projectile_move_on(ob_methods *context, object *trap, object *victim, object *originator) {
    if (common_pre_ob_move_on(trap, victim, originator) == METHOD_ERROR)
        return METHOD_OK;
    if (trap->inv == NULL) {
        common_post_ob_move_on(trap, victim, originator);
        return METHOD_OK;
    }

    /* bad bug: monster throw a object, make a step forwards, step on object ,
     * trigger this here and get hit by own missile - and will be own enemy.
     * Victim then is his own enemy and will start to kill herself (this is
     * removed) but we have not synced victim and his missile. To avoid senseless
     * action, we avoid hits here
     */
    if ((QUERY_FLAG(victim, FLAG_ALIVE) && trap->speed)
    && object_get_owner(trap) != victim)
        hit_with_arrow(trap, victim);
    common_post_ob_move_on(trap, victim, originator);
    return METHOD_OK;
}
