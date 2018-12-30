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

/** @file shop_mat.c
 * The implementation of the Shop Mat class of objects.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static method_ret shop_mat_type_move_on(ob_methods *context, object *trap, object *victim, object *originator);

/**
 * Initializer for the SHOP_MAT object type.
 */
void init_type_shop_mat(void) {
    register_move_on(SHOP_MAT, shop_mat_type_move_on);
}

/**
 * Move on this Shop Mat object.
 * @param context The method context
 * @param trap The Shop Mat we're moving on
 * @param victim The object moving over this one
 * @param originator The object that caused the move_on event
 * @return METHOD_OK
 */
static method_ret shop_mat_type_move_on(ob_methods *context, object *trap, object *victim, object *originator) {
    int rv = 0;
    double opinion;

    if (common_pre_ob_move_on(trap, victim, originator) == METHOD_ERROR)
        return METHOD_OK;

    SET_FLAG(victim, FLAG_NO_APPLY);   /* prevent loops */

    if (victim->type != PLAYER) {
        /* Remove all the unpaid objects that may be carried here.
         * This could be pets or monsters that are somehow in
         * the shop.
         */
        FOR_INV_PREPARE(victim, tmp) {
            if (QUERY_FLAG(tmp, FLAG_UNPAID)) {
                int i = object_find_free_spot(tmp, victim->map, victim->x, victim->y, 1, 9);
                object_remove(tmp);
                if (i == -1)
                    i = 0;
                object_insert_in_map_at(tmp, victim->map, victim, 0, victim->x+freearr_x[i], victim->y+freearr_y[i]);
            }
        } FOR_INV_FINISH();

        /* Don't teleport things like spell effects */
        if (QUERY_FLAG(victim, FLAG_NO_PICK))
            goto leave;

        /* unpaid objects, or non living objects, can't transfer by
         * shop mats.  Instead, put it on a nearby space.
         */
        if (QUERY_FLAG(victim, FLAG_UNPAID) || !QUERY_FLAG(victim, FLAG_ALIVE)) {
            /* Somebody dropped an unpaid item, just move to an adjacent place. */
            int i = object_find_free_spot(victim, victim->map, victim->x, victim->y, 1, 9);
            if (i != -1) {
                rv = transfer_ob(victim, victim->x+freearr_x[i], victim->y+freearr_y[i], 0, trap);
            }
            goto leave;
        }
        rv = teleport(trap, SHOP_MAT, victim);
    /* immediate block below is only used for players */
    } else if (can_pay(victim)) {
        get_payment(victim, victim->inv);
        rv = teleport(trap, SHOP_MAT, victim);
        if (trap->msg) {
            draw_ext_info(NDI_UNIQUE, 0, victim, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
                trap->msg);
        }
        /* This check below is a bit simplistic - generally it should be correct,
         * but there is never a guarantee that the bottom space on the map is
         * actually the shop floor.
         */
        else if (!rv && !is_in_shop(victim)) {
            opinion = shopkeeper_approval(victim->map, victim);
            if (opinion > 0.9)
                draw_ext_info(NDI_UNIQUE, 0, victim, MSG_TYPE_SHOP, MSG_TYPE_SHOP_MISC,
                    "The shopkeeper gives you a friendly wave.");
            else if (opinion > 0.75)
                draw_ext_info(NDI_UNIQUE, 0, victim, MSG_TYPE_SHOP, MSG_TYPE_SHOP_MISC,
                    "The shopkeeper waves to you.");
            else if (opinion > 0.5)
                draw_ext_info(NDI_UNIQUE, 0, victim, MSG_TYPE_SHOP, MSG_TYPE_SHOP_MISC,
                    "The shopkeeper ignores you.");
            else
                draw_ext_info(NDI_UNIQUE, 0, victim, MSG_TYPE_SHOP, MSG_TYPE_SHOP_MISC,
                    "The shopkeeper glares at you with contempt.");
        }
    } else {
        /* if we get here, a player tried to leave a shop but was not able
         * to afford the items he has.  We try to move the player so that
         * they are not on the mat anymore
         */
        int i = object_find_free_spot(victim, victim->map, victim->x, victim->y, 1, 9);
        if (i == -1)
            LOG(llevError, "Internal shop-mat problem.\n");
        else {
            object_remove(victim);
            rv = object_insert_in_map_at(victim, victim->map, trap, 0, victim->x+freearr_x[i], victim->y+freearr_y[i]) == NULL;
            esrv_map_scroll(&victim->contr->socket, freearr_x[i], freearr_y[i]);
            victim->contr->socket.update_look = 1;
            victim->contr->socket.look_position = 0;
        }
    }
    CLEAR_FLAG(victim, FLAG_NO_APPLY);
leave:
    common_post_ob_move_on(trap, victim, originator);
    return METHOD_OK;
}
