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

/** @file
 * The implementation of the Treasure class of objects.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static method_ret treasure_type_apply(ob_methods *context, object *op, object *applier, int aflags);

/**
 * Initializer for the TREASURE object type.
 */
void init_type_treasure(void) {
    register_apply(TREASURE, treasure_type_apply);
}

/**
 * Attempts to apply treasure.
 * @param context The method context
 * @param op The treasure to apply
 * @param applier The object attempting to apply the treasure. Ignored unless
 * a player
 * @param aflags Special flags (always apply/unapply)
 * @return The return value is always METHOD_OK
 */
static method_ret treasure_type_apply(ob_methods *context, object *op, object *applier, int aflags) {
    object *treas;
    tag_t op_tag = op->count, applier_tag = applier->count;
    char name[MAX_BUF];

    if (applier->type == PLAYER) {
         /* Nice side effect of new treasure creation method is that the
         * treasure for the chest is done when the chest is created,
         * and put into the chest inventory.  So that when the chest
         * burns up, the items still exist.  Also prevents people from
         * moving chests to more difficult maps to get better treasure
         */

        treas = op->inv;
        if (treas == NULL) {
            draw_ext_info(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_FAILURE,
                "The chest was empty.");
            object_decrease_nrof_by_one(op);
            return METHOD_OK;
        }
        while (op->inv) {
            treas = op->inv;

            object_remove(treas);
            /* don't tell the player about invisible objects */
            if (!treas->invisible) {
                query_name(treas, name, MAX_BUF);
                draw_ext_info_format(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
                        "You find %s in the chest.",
                        name);
            }

            treas = object_insert_in_map_at(treas, applier->map, applier, INS_BELOW_ORIGINATOR, applier->x, applier->y);

            if (treas
            && (treas->type == RUNE || treas->type == TRAP)
            && treas->level && QUERY_FLAG(applier, FLAG_ALIVE))
                spring_trap(treas, applier);
                /* If either player or container was destroyed, no need to do
                 * further processing.  I think this should be enclused with
                 * spring trap above, as I don't think there is otherwise
                 * any way for the treasure chest or player to get killed
                 */
            if (object_was_destroyed(applier, applier_tag) || object_was_destroyed(op, op_tag))
                break;
        }

        if (!object_was_destroyed(op, op_tag) && op->inv == NULL)
            object_decrease_nrof_by_one(op);
    }
    return METHOD_OK;
}
