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

/** @file poison.c
 * The implementation of the Poison class of objects.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static method_ret poison_type_apply(ob_methods *context, object *op, object *applier, int aflags);

/**
 * Initializer for the POISON object type.
 */
void init_type_poison(void) {
    register_apply(POISON, poison_type_apply);
}

/**
 * Attempts to apply some poison.
 * @param context The method context
 * @param op The poison to apply
 * @param applier The object attempting to apply the poison.
 * @param aflags Special flags (always apply/unapply)
 * @return The return value is always METHOD_OK
 */
static method_ret poison_type_apply(ob_methods *context, object *op, object *applier, int aflags) {
    /* If a player, let's tell them what happened */
    if (applier->type == PLAYER) {
        play_sound_player_only(applier->contr, SOUND_TYPE_ITEM, op, 0, "poison");
        draw_ext_info(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_CURSED,
            "Yech!  That tasted poisonous!");
        snprintf(applier->contr->killer, BIG_NAME, "poisonous %s", op->name);
    }
    /* If the 'hp' of the poison is greater than zero, use poison attacktype */
    if (op->stats.hp > 0) {
        LOG(llevDebug, "Trying to poison player/monster for %d hp\n", op->stats.hp);
        hit_player(applier, op->stats.hp, op, AT_POISON, 1);
    }
    /* Reduce the applier's food to one quarter of what it was */
    applier->stats.food -= applier->stats.food/4;
    apply_handle_yield(op);
    object_decrease_nrof_by_one(op);
    return METHOD_OK;
}
