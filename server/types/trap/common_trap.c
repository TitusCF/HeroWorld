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

/** @file common_trap.c
 * Common code for traps and runes.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

/**
 * Move on this Rune or Trap object.
 * @param context The method context
 * @param trap The rune or trap we're moving on
 * @param victim The object moving over this one
 * @param originator The object that caused the move_on event
 * @return METHOD_OK
 */
method_ret common_trap_type_move_on(ob_methods *context, object *trap, object *victim, object *originator) {
    if (common_pre_ob_move_on(trap, victim, originator) == METHOD_ERROR)
        return METHOD_OK;
    if (trap->level && QUERY_FLAG(victim, FLAG_ALIVE))
        spring_trap(trap, victim);
    common_post_ob_move_on(trap, victim, originator);
    return METHOD_OK;
}

/**
 * Processes a Rune or Trap.
 * Comments on runes (and traps):
 *    rune->level     :     level at which rune will cast its spell.
 *    rune->hp        :     number of detonations before rune goes away
 *    rune->msg       :     message the rune displays when it goes off
 *    rune->direction :     direction it will cast a spell in
 *    rune->dam       :     damage the rune will do if it doesn't cast spells
 *    rune->attacktype:     type of damage it does, if not casting spells
 *    rune->other_arch:       spell in the rune
 *    rune->Cha       :       how hidden the rune is
 *    rune->maxhp     :       number of spells the rune casts
 * @param context The method context
 * @param op The rune or trap to process
 * @return Always METHOD_OK
 */
method_ret common_trap_type_process(ob_methods *context, object *op) {
    int det = 0;

    if (!op->level) {
        return METHOD_OK;  /* runes of level zero cannot detonate. */
    }

    det = op->invisible;
    if (!(rndm(0, MAX(1, (op->stats.Cha))-1))) {
        op->invisible = 0;
        op->speed_left -= 1;
    } else
        op->invisible = 1;
    if (op->invisible != det)
        object_update(op, UP_OBJ_FACE);
    return METHOD_OK;
}
