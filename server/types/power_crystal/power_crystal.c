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
/** @file power_crystal.c
 * The implementation of a Power Crystal.
 * A Power Crystal can store mana from a player, and give it back when needed.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sproto.h>

static method_ret power_crystal_type_apply(ob_methods *context, object *op, object *applier, int aflags);

/**
 * Initializer for the POWER_CRYSTAL object type.
 */
void init_type_power_crystal(void) {
    register_apply(POWER_CRYSTAL, power_crystal_type_apply);
    register_describe(POWER_CRYSTAL, common_ob_describe);
}

/**
 * This function handles the application of power crystals.
 * Power crystals, when applied, either suck power from the applier,
 * if he's at full spellpoints, or gives him power, if it's got
 * spellpoins stored.
 * @param context The method context
 * @param op The power crystal to apply
 * @param applier The object applying the crystal
 * @param aflags Special flags (always apply/unapply)
 * @return The return value is always METHOD_OK
 */
static method_ret power_crystal_type_apply(ob_methods *context, object *op, object *applier, int aflags) {
    int available_power;
    int power_space;
    int power_grab;

    available_power = applier->stats.sp-applier->stats.maxsp;
    power_space = op->stats.maxsp-op->stats.sp;
    power_grab = 0;
    if (available_power >= 0 && power_space > 0)
        power_grab = MIN(power_space, 0.5*applier->stats.sp);
    if (available_power < 0 && op->stats.sp > 0)
        power_grab = -MIN(-available_power, op->stats.sp);

    applier->stats.sp -= power_grab;
    op->stats.sp += power_grab;
    op->speed = (float)op->stats.sp/(float)op->stats.maxsp;
    object_update_speed(op);
    if (applier->type == PLAYER)
        esrv_update_item(UPD_ANIMSPEED, applier, op);
    return METHOD_OK;
}
