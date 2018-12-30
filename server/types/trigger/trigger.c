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

/** @file trigger.c
 * The implementation of the Trigger class of objects.
 * Triggers are things like handles in the game.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static method_ret trigger_type_apply(ob_methods *context, object *op, object *applier, int aflags);
static method_ret trigger_type_process(ob_methods *context, object *op);

/**
 * Initializer for the TRIGGER object type.
 */
void init_type_trigger(void) {
    register_apply(TRIGGER, trigger_type_apply);
    register_process(TRIGGER, trigger_type_process);
}

/**
 * Attempts to apply a trigger.
 * @param context The method context
 * @param op The Trigger to apply
 * @param applier The object attempting to apply the Trigger
 * @param aflags Special flags (always apply/unapply)
 * @return The return value is always 1
 */
static method_ret trigger_type_apply(ob_methods *context, object *op, object *applier, int aflags) {
    if (check_trigger(op, applier)) {
        draw_ext_info(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
            "You turn the handle.");
        play_sound_map(SOUND_TYPE_GROUND, op, 0, "turn handle");
    } else {
        draw_ext_info(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_FAILURE,
            "The handle doesn't move.");
    }
    return 1;
}

/**
 * Processes a Trigger.
 * @param context The method context
 * @param op The Trigger to process
 * @return Always METHOD_OK
 */
static method_ret trigger_type_process(ob_methods *context, object *op) {
    if ((unsigned char)++op->stats.wc >= NUM_ANIMATIONS(op)) {
        op->stats.wc = 0;
        check_trigger(op, NULL);
    } else {
        SET_ANIMATION(op, op->stats.wc);
        object_update(op, UP_OBJ_FACE);
    }
    return METHOD_OK;
}
