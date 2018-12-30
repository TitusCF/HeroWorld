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

/** @file sign.c
 * The implementation of the Sign class of objects.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static void apply_sign(object *sign, object *op, int autoapply);
static method_ret sign_type_apply(ob_methods *context, object *op, object *applier, int aflags);
static method_ret sign_type_move_on(ob_methods *context, object *trap, object *victim, object *originator);

/**
 * Initializer for the SIGN object type.
 */
void init_type_sign(void) {
    register_move_on(SIGN, sign_type_move_on);
    register_apply(SIGN, sign_type_apply);
}

/**
 * Handles applying a sign.
 * @param sign The sign applied
 * @param op The object applying the sign
 * @param autoapply Set this to 1 to automatically apply the sign
 */
static void apply_sign(object *sign, object *op, int autoapply) {
    const readable_message_type *msgType;

    if (sign->msg == NULL) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_FAILURE,
            "Nothing is written on it.");
        return;
    }

    if (sign->stats.food) {
        if (sign->last_eat >= sign->stats.food) {
            if (!sign->move_on)
                draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_FAILURE,
                    "You cannot read it anymore.");
            return;
        }

        if (!QUERY_FLAG(op, FLAG_WIZPASS))
            sign->last_eat++;
    }

    /* Sign or magic mouth?  Do we need to see it, or does it talk to us?
     * No way to know for sure.  The presumption is basically that if
     * move_on is zero, it needs to be manually applied (doesn't talk
     * to us).
     */
    if (QUERY_FLAG(op, FLAG_BLIND)
    && !QUERY_FLAG(op, FLAG_WIZ)
    && !sign->move_on) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
            "You are unable to read while blind.");
        return;
    }
    msgType = get_readable_message_type(sign);
    draw_ext_info(NDI_UNIQUE|NDI_NAVY, 0, op, msgType->message_type, msgType->message_subtype,
        sign->msg);
}

/**
 * Attempts to apply a sign.
 * @param context The method context
 * @param op The Sign to apply
 * @param applier The object attempting to apply the Sign
 * @param aflags Special flags (always apply/unapply)
 * @return The return value is always METHOD_OK
 */
static method_ret sign_type_apply(ob_methods *context, object *op, object *applier, int aflags) {
    apply_sign(op, applier, 0);
    return METHOD_OK;
}

/**
 * Move on this Sign object.
 * @param context The method context
 * @param trap The Sign we're moving on
 * @param victim The object moving over this one
 * @param originator The object that caused the move_on event
 * @return METHOD_OK
 */
static method_ret sign_type_move_on(ob_methods *context, object *trap, object *victim, object *originator) {
    if (common_pre_ob_move_on(trap, victim, originator) == METHOD_ERROR)
        return METHOD_OK;
    if (victim->type != PLAYER && trap->stats.food > 0) {
        common_post_ob_move_on(trap, victim, originator);
        return METHOD_OK; /* monsters musn't apply magic_mouths with counters */
    }
    apply_sign(trap, victim, 1);
    common_post_ob_move_on(trap, victim, originator);
    return METHOD_OK;
}
