/*
    CrossFire, A Multiplayer game for X-windows

    Copyright (C) 2008 Crossfire Development Team
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
 * @file
 * The implementation of the PLAYER_CHANGER class of objects.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static method_ret player_changer_type_process(ob_methods *context, object *op);

/**
 * Initializer for the player changer object type.
 */
void init_type_player_changer(void) {
    register_process(PLAYER_CHANGER, player_changer_type_process);
}

/**
 * Move for ::PLAYER_CHANGER.
 *
 * This object will teleport someone to a different map
 * and will also apply changes to the player from its inventory.
 *
 * This was invented for giving classes, but there's no reason it
 *  can't be generalized.
 *
 * @param context The method context
 * @param op The player changer that's being processed.
 * @return METHOD_OK
 */
static method_ret player_changer_type_process(ob_methods *context, object *op) {
    object *player;
    char c;

    if (!op->above || !EXIT_PATH(op))
        return METHOD_OK;

    /* This isn't all that great - means that the player_mover
    * needs to be on top.
    */
    if (op->above->type == PLAYER) {
        /* Lauwenmark: Handle for plugin TRIGGER event */
        if (execute_event(op, EVENT_TRIGGER, op->above, NULL, NULL, SCRIPT_FIX_NOTHING) != 0)
            return METHOD_OK;
        player = op->above;
        FOR_INV_PREPARE(op, walk)
            apply_changes_to_player(player, walk, AC_PLAYER_STAT_LIMIT);
        FOR_INV_FINISH();

        fix_object(player);
        esrv_send_inventory(op->above, op->above);
        esrv_update_item(UPD_FACE, op->above, op->above);

        /* update players death & WoR home-position */
        sscanf(EXIT_PATH(op), "%c", &c);
        if (c == '/') {
            strcpy(player->contr->savebed_map, EXIT_PATH(op));
            player->contr->bed_x = EXIT_X(op);
            player->contr->bed_y = EXIT_Y(op);
        } else
            LOG(llevDebug, "WARNING: destination '%s' in player_changer must be an absolute path!\n", EXIT_PATH(op));

        enter_exit(op->above, op);
        save_player(player, 1);
    }

    return METHOD_OK;
}
