/*
    CrossFire, A Multiplayer game for X-windows

    Copyright (C) 2006 Mark Wedel & Crossfire Development Team
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

    The authors can be reached via e-mail to crossfire-devel@real-time.com
*/
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sproto.h>

/** @file process.c
 * Legacy implementation of the process method.
 */
method_ret legacy_ob_process(ob_methods *context, object *op) {
    switch (op->type) {
    case ROD:
        regenerate_rod(op);
        return METHOD_OK;

    case FORCE:
    case POTION_RESIST_EFFECT:
        legacy_remove_force(op);
        return METHOD_OK;

    case DISEASE:
        move_disease(op);
        return METHOD_OK;

    case SYMPTOM:
        move_symptom(op);
        return METHOD_OK;

    case DOOR:
        remove_door(op);
        return METHOD_OK;

    case LOCKED_DOOR:
        remove_locked_door(op);
        return METHOD_OK;

    case GOLEM:
        pets_move_golem(op);
        return METHOD_OK;

    case EARTHWALL:
        hit_player(op, 2, op, AT_PHYSICAL, 1);
        return METHOD_OK;

    case FIREWALL:
        move_firewall(op);
        if (op->stats.maxsp)
            animate_turning(op);
        return METHOD_OK;

    case TRIGGER_BUTTON:
    case TRIGGER_PEDESTAL:
    case TRIGGER_ALTAR:
        legacy_animate_trigger(op);
        return METHOD_OK;

    case DIRECTOR:
        if (op->stats.maxsp)
            animate_turning(op);
        return METHOD_OK;

    case HOLE:
        legacy_move_hole(op);
        return METHOD_OK;

    case PLAYERMOVER:
        move_player_mover(op);
        return METHOD_OK;
    }
    return METHOD_UNHANDLED;
}
