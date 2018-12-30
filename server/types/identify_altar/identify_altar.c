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

/** @file identify_altar.c
 * The implementation of the Identify Altar class of objects.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static method_ret identify_altar_type_move_on(ob_methods *context, object *altar, object *money, object *originator);

/**
 * Initializer for the IDENTIFY_ALTAR object type.
 */
void init_type_identify_altar(void) {
    register_move_on(IDENTIFY_ALTAR, identify_altar_type_move_on);
}

/**
 * Move on this Altar Identifier object.
 * @param context The method context
 * @param altar The Altar Identifier we're moving on
 * @param money The object moving over this one
 * @param originator The object that caused the move_on event
 * @return METHOD_OK
 */
static method_ret identify_altar_type_move_on(ob_methods *context, object *altar, object *money, object *originator) {
    object *marked;
    int success = 0;
    char desc[MAX_BUF];

    if (common_pre_ob_move_on(altar, money, originator) == METHOD_ERROR)
        return METHOD_OK;

    if (originator == NULL || originator->type != PLAYER) {
        common_post_ob_move_on(altar, money, originator);
        return METHOD_OK;
    }
    /* Check for MONEY type is a special hack - it prevents 'nothing needs
     * identifying' from being printed out more than it needs to be.
     */
    if (money->type != MONEY || !check_altar_sacrifice(altar, money, 0, NULL)) {
        common_post_ob_move_on(altar, money, originator);
        return METHOD_OK;
    }
    marked = find_marked_object(originator);
    /* if the player has a marked item, identify that if it needs to be
     * identified.  IF it doesn't, then go through the player inventory.
     */
    if (marked
    && !QUERY_FLAG(marked, FLAG_IDENTIFIED)
    && need_identify(marked)) {
        if (operate_altar(altar, &money)) {
            marked = identify(marked);
            draw_ext_info_format(NDI_UNIQUE, 0, originator, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
                "You have %s.",
                ob_describe(marked, originator, desc, sizeof(desc)));

            if (marked->msg) {
                draw_ext_info(NDI_UNIQUE, 0, originator, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
                    "The item has a story:");
                draw_ext_info(NDI_UNIQUE, 0, originator, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
                    marked->msg);
            }
            common_post_ob_move_on(altar, money, originator);
            return METHOD_OK;
        }
    }

    FOR_INV_PREPARE(originator, id) {
        if (!QUERY_FLAG(id, FLAG_IDENTIFIED)
        && !id->invisible
        && need_identify(id)) {
            if (operate_altar(altar, &money)) {
                id = identify(id);
                draw_ext_info_format(NDI_UNIQUE, 0, originator, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
                    "You have %s.", ob_describe(id, originator, desc, sizeof(desc)));
                if (id->msg) {
                    draw_ext_info(NDI_UNIQUE, 0, originator, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
                        "The item has a story:");
                    draw_ext_info(NDI_UNIQUE, 0, originator, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
                        id->msg);
                }
                success = 1;
                /* If no more money, might as well quit now */
                if (money == NULL || !check_altar_sacrifice(altar, money, 0, NULL))
                    break;
            } else {
                LOG(llevError, "check_id_altar:  Couldn't do sacrifice when we should have been able to\n");
                break;
            }
        }
    } FOR_INV_FINISH();
    if (!success)
        draw_ext_info(NDI_UNIQUE, 0, originator, MSG_TYPE_APPLY, MSG_TYPE_APPLY_FAILURE,
            "You have nothing that needs identifying");
    common_post_ob_move_on(altar, money, originator);
    return METHOD_OK;
}
