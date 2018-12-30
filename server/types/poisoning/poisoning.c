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

/** @file poisoning/poisoning.c
 * The implementation of the Poisoning class of objects.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static method_ret poisoning_type_process(ob_methods *context, object *op);

/**
 * Initializer for the poisoning object type.
 */
void init_type_poisoning(void) {
    register_process(POISONING, poisoning_type_process);
}

/**
 * Handle ob_process for all poisoning objects.
 * @param context The method context
 * @param op The poisoning that's being processed.
 * @return METHOD_OK normally. METHOD_ERROR if POISONING is in an invalid env.
 */
static method_ret poisoning_type_process(ob_methods *context, object *op) {
    if (op->env == NULL
    || !QUERY_FLAG(op->env, FLAG_ALIVE)
    || op->env->stats.hp < 0) {
        object_remove(op);
        object_free_drop_inventory(op);
        LOG(llevDebug, "Found POISONING with invalid env. Removing...\n");
        return METHOD_ERROR;
    }

    if (op->stats.food == 1) {
        /* need to remove the object before fix_player is called, else fix_object
         * will not do anything.
         */
        if (op->env->type == PLAYER) {
            CLEAR_FLAG(op, FLAG_APPLIED);
            fix_object(op->env);
            draw_ext_info(NDI_UNIQUE, 0, op->env, MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_BAD_EFFECT_END,
                "You feel much better now.");
        }
        object_remove(op);
        object_free_drop_inventory(op);
        return METHOD_OK;
    }

    if (op->env->type == PLAYER) {
        op->env->stats.food--;
        /* Not really the start of a bad effect, more the continuing effect */
        draw_ext_info(NDI_UNIQUE, 0, op->env, MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_BAD_EFFECT_START,
            "You feel very sick...");
    }
    (void)hit_player(op->env, op->stats.dam, op, AT_INTERNAL, 1);
    return METHOD_OK;
}
