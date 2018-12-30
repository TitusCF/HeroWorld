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

/** @file gate/gate.c
 * The implementation of the Gate and Timed Gate classes of objects.
 * @todo merge GATE and TIMED_GATE object types.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static method_ret gate_type_process(ob_methods *context, object *op);
static method_ret timed_gate_type_process(ob_methods *context, object *op);

/**
 * Initializer for the gate object type.
 */
void init_type_gate(void) {
    register_process(GATE, gate_type_process);
    register_process(TIMED_GATE, timed_gate_type_process);
}

/**
 * Handle ob_process for all gate objects.
 * @param context The method context
 * @param op The gate that's being processed.
 * @return METHOD_OK
 */
static method_ret gate_type_process(ob_methods *context, object *op) {
    object *tmp;

    if (op->stats.wc < 0 || (int)op->stats.wc >= NUM_ANIMATIONS(op)) {
        StringBuffer *sb;
        char *diff;

        LOG(llevError, "Gate error: animation was %d, max=%d, on %s (%d, %d)\n", op->stats.wc, NUM_ANIMATIONS(op), map_get_path(op), op->x, op->y);
        sb = stringbuffer_new();
        object_dump(op, sb);
        diff = stringbuffer_finish(sb);
        LOG(llevError, "%s\n", diff);
        free(diff);
        op->stats.wc = 0;
    }

    /* We're going down */
    if (op->value) {
        if (--op->stats.wc <= 0) { /* Reached bottom, let's stop */
            op->stats.wc = 0;
            if (op->arch->clone.speed)
                op->value = 0;
            else {
                op->speed = 0;
                object_update_speed(op);
            }
        }
        if ((int)op->stats.wc < (NUM_ANIMATIONS(op)/2+1)) {
            op->move_block = 0;
            CLEAR_FLAG(op, FLAG_BLOCKSVIEW);
            update_all_los(op->map, op->x, op->y);
        }
        SET_ANIMATION(op, op->stats.wc);
        object_update(op, UP_OBJ_CHANGE);
        return METHOD_OK;
    }

    /* We're going up */

    /* First, lets see if we are already at the top */
    if ((unsigned char)op->stats.wc == (NUM_ANIMATIONS(op)-1)) {
        /* Check to make sure that only non pickable and non rollable
         * objects are above the gate.  If so, we finish closing the gate,
         * otherwise, we fall through to the code below which should lower
         * the gate slightly.
         */

        for (tmp = op->above; tmp != NULL; tmp = tmp->above)
            if (!QUERY_FLAG(tmp, FLAG_NO_PICK)
            || QUERY_FLAG(tmp, FLAG_CAN_ROLL)
            || QUERY_FLAG(tmp, FLAG_ALIVE))
                break;

        if (tmp == NULL) {
            if (op->arch->clone.speed)
                op->value = 1;
            else {
                op->speed = 0;
                object_update_speed(op); /* Reached top, let's stop */
            }
            return METHOD_OK;
        }
    }

    if (op->stats.food) {    /* The gate is going temporarily down */
        if (--op->stats.wc <= 0) { /* Gone all the way down? */
            op->stats.food = 0;     /* Then let's try again */
            op->stats.wc = 0;
        }
    } else {                /* The gate is still going up */
        op->stats.wc++;

        if ((int)op->stats.wc >= (NUM_ANIMATIONS(op)))
            op->stats.wc = (signed char)NUM_ANIMATIONS(op)-1;

        /* If there is something on top of the gate, we try to roll it off.
         * If a player/monster, we don't roll, we just hit them with damage
         */
        if ((int)op->stats.wc >= NUM_ANIMATIONS(op)/2) {
            /* Halfway or further, check blocks */
            /* First, get the top object on the square. */
            for (tmp = op->above; tmp != NULL && tmp->above != NULL; tmp = tmp->above)
                ;

            if (tmp != NULL) {
                if (QUERY_FLAG(tmp, FLAG_ALIVE)) {
                    hit_player(tmp, random_roll(1, op->stats.dam, tmp, PREFER_LOW), op, AT_PHYSICAL, 1);
                    if (tmp->type == PLAYER)
                        draw_ext_info_format(NDI_UNIQUE, 0, tmp, MSG_TYPE_VICTIM, MSG_TYPE_VICTIM_WAS_HIT,
                                             "You are crushed by the %s!",
                                             op->name);
                } else
                    /* If the object is not alive, and the object either can
                     * be picked up or the object rolls, move the object
                     * off the gate.
                     */
                    if (!QUERY_FLAG(tmp, FLAG_ALIVE)
                    && (!QUERY_FLAG(tmp, FLAG_NO_PICK) || QUERY_FLAG(tmp, FLAG_CAN_ROLL))) {
                    /* If it has speed, it should move itself, otherwise: */
                        int i = object_find_free_spot(tmp, op->map, op->x, op->y, 1, 9);

                        /* If there is a free spot, move the object someplace */
                        if (i != -1) {
                            object_remove(tmp);
                            object_insert_in_map_at(tmp, op->map, op, 0, op->x+freearr_x[i], op->y+freearr_y[i]);
                        }
                    }
            }

            /* See if there is still anything blocking the gate */
            for (tmp = op->above; tmp != NULL; tmp = tmp->above)
                if (!QUERY_FLAG(tmp, FLAG_NO_PICK)
                || QUERY_FLAG(tmp, FLAG_CAN_ROLL)
                || QUERY_FLAG(tmp, FLAG_ALIVE))
                    break;

            /* IF there is, start putting the gate down  */
            if (tmp) {
                op->stats.food = 1;
            } else {
                op->move_block = MOVE_ALL;
                if (!op->arch->clone.stats.ac)
                    SET_FLAG(op, FLAG_BLOCKSVIEW);
                update_all_los(op->map, op->x, op->y);
            }
        } /* gate is halfway up */

        SET_ANIMATION(op, op->stats.wc);
        object_update(op, UP_OBJ_CHANGE);
    } /* gate is going up */

    return METHOD_OK;
}

 /**
 * Handle ob_process for all timed gate objects.
 * - hp      : how long door is open/closed
 * - maxhp   : initial value for hp
 * - sp      : 1 = open, 0 = close
 * @param context The method context
 * @param op The timed gate that's being processed.
 * @return METHOD_OK
 * @todo Split function into more managable functions.
 */
static method_ret timed_gate_type_process(ob_methods *context, object *op) {
    int v = op->value;

    if (op->stats.sp) {
        gate_type_process(context, op);
        if (op->value != v)   /* change direction ? */
            op->stats.sp = 0;
        return METHOD_OK;
    }
    if (--op->stats.hp <= 0) { /* keep gate down */
        gate_type_process(context, op);
        if (op->value != v) {  /* ready ? */
            op->speed = 0;
            object_update_speed(op);
        }
    }
    return METHOD_OK;
}
