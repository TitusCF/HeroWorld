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

/** @file savebed.c
 * The implementation of the Savebed class of objects.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static method_ret savebed_type_apply(ob_methods *context, object *op, object *applier, int aflags);
static void apply_savebed(object *pl);

/**
 * Initializer for the SAVEBED object type.
 */
void init_type_savebed(void) {
    register_apply(SAVEBED, savebed_type_apply);
}

/**
 * Attempts to apply a savebed.
 * @param context The method context
 * @param op The savebed to apply
 * @param applier The object attempting to apply the savebed. Ignore if not
 *   a player
 * @param aflags Special flags (always apply/unapply)
 * @return The return value is always METHOD_OK
 */
static method_ret savebed_type_apply(ob_methods *context, object *op, object *applier, int aflags) {
    if (applier->type == PLAYER)
        apply_savebed(applier);
    return METHOD_OK;
}

/**
 * Handle savebed.
 *
 * @param pl
 * player who is applying the bed.
 */
static void apply_savebed(object *pl) {

    /* What is otherwise happening is a brand new character goes to save, it seems to work,
     * but the character isn't actually saved as save_player() won't save characters
     * with 0 exp. Warn the player
     */
    if (!pl->stats.exp) {
        draw_ext_info_format(NDI_UNIQUE | NDI_RED, 5, pl, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_LOADSAVE,
                             "You need to earn some experience before you can save the character");
        return;
    }

    /* Lauwenmark : Here we handle the LOGOUT global event */
    execute_global_event(EVENT_LOGOUT, pl->contr, pl->contr->socket.host);

    /* Need to call pets_terminate_all()  before we remove the player ob */
    pets_terminate_all(pl);
    object_remove(pl);
    pl->direction = 0;
    draw_ext_info_format(NDI_UNIQUE|NDI_ALL|NDI_DK_ORANGE, 5, pl, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_PLAYER,
        "%s leaves the game.",
        pl->name);

    /* update respawn position */
    strcpy(pl->contr->savebed_map, pl->map->path);
    pl->contr->bed_x = pl->x;
    pl->contr->bed_y = pl->y;

    strcpy(pl->contr->killer, "left");
    hiscore_check(pl, 0); /* Always check score */
    (void)save_player(pl, 0);
    party_leave(pl);
#if MAP_MAXTIMEOUT
    MAP_SWAP_TIME(pl->map) = MAP_TIMEOUT(pl->map);
#endif
    play_again(pl);
    pl->speed = 0;
    object_update_speed(pl);
}
