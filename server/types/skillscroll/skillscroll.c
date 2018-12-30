/*
    CrossFire, A Multiplayer game for X-windows

    Copyright (C) 2007 Crossfire Development Team
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

/**
 * @file skillscroll.c
 * Implimentation of skill scrolls.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static method_ret skillscroll_type_apply(ob_methods *context, object *lighter, object *applier, int aflags);

/**
 * Initializer for the SKILLSCROLL object type.
 */
void init_type_skillscroll(void) {
    register_apply(SKILLSCROLL, skillscroll_type_apply);
}

/**
 * Applies a skillscroll.
 *
 * @param context
 * method context.
 * @param scroll
 * Skillscroll to apply.
 * @param applier
 * object attempting to apply the scroll. Should be a player.
 * @param aflags
 * special flags (always apply/unapply).
 * @return
 * METHOD_OK always.
 */
static method_ret skillscroll_type_apply(ob_methods *context, object *scroll, object *applier, int aflags) {
    char name[MAX_BUF];

    /* Must be applied by a player. */
    if (applier->type == PLAYER) {
        switch ((int)learn_skill(applier, scroll)) {
        case 0:
            query_name(scroll, name, MAX_BUF);
            draw_ext_info_format(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
                "You already possess the knowledge held within the %s.",
                name);
            return METHOD_OK;

        case 1:
            draw_ext_info_format(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
                "You succeed in learning %s",
                scroll->skill);
            draw_ext_info_format(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
                "Type 'bind ready_skill %s to store the skill in a key.",
                scroll->skill);
            object_decrease_nrof_by_one(scroll);
            return METHOD_OK;

        default:
            query_name(scroll, name, MAX_BUF);
            draw_ext_info_format(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_FAILURE,
                "You fail to learn the knowledge of the %s.\n",
                name);
            object_decrease_nrof_by_one(scroll);
            return METHOD_OK;
        }
    }
    return METHOD_OK;
}
