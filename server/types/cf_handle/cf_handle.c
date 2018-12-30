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

/** @file cf_handle.c
 * The implementation of the CF_Handle class of objects.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static method_ret cf_handle_type_apply(ob_methods *context, object *op, object *applier, int aflags);

/**
 * Initializer for the CF_HANDLE object type.
 */
void init_type_cf_handle(void) {
    register_apply(CF_HANDLE, cf_handle_type_apply);
}

/**
 * Attempts to apply a handle.
 * @param context The method context
 * @param op The handle to apply
 * @param applier The object attempting to apply the handle.
 * @param aflags Special flags (always apply/unapply)
 * @return The return value is always METHOD_OK
 */
static method_ret cf_handle_type_apply(ob_methods *context, object *op, object *applier, int aflags) {
    draw_ext_info(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
        "You turn the handle.");
    play_sound_map(SOUND_TYPE_ITEM, op, 0, "turn handle");
    op->value = op->value ? 0 : 1;
    SET_ANIMATION(op, op->value);
    object_update(op, UP_OBJ_FACE);
    push_button(op);
    return METHOD_OK;
}
