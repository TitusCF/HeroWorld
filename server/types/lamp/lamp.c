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
 * @file lamp.c
 * Lamps.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static method_ret lamp_type_apply(ob_methods *context, object *lighter, object *applier, int aflags);

/**
 * Initializer for the LAMP object type.
 */
void init_type_lamp(void) {
    register_apply(LAMP, lamp_type_apply);
}

/**
 * Turn on/off the lamp, based on op's APPLIED status.
 * @param op
 * lamp.
 * @param who
 * player or monster.
 * @param aflags
 * flags.
 * @param onoff
 * string that will be appended to the 'turn' verb in the message.
 */
static void do_turn(object *op, object *who, int aflags, const char *onoff) {
    object *tmp2;

    if (!(aflags&AP_NOPRINT))
        draw_ext_info_format(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
            "You turn %s your %s.",
            onoff, op->name);

    tmp2 = arch_to_object(op->other_arch);
    tmp2->stats.food = op->stats.food;
    if (QUERY_FLAG(op, FLAG_APPLIED))
        CLEAR_FLAG(tmp2, FLAG_APPLIED);
    else
        SET_FLAG(tmp2, FLAG_APPLIED);
    if (QUERY_FLAG(op, FLAG_INV_LOCKED))
        SET_FLAG(tmp2, FLAG_INV_LOCKED);
    object_insert_in_ob(tmp2, who);

    object_remove(op);
    object_free_drop_inventory(op);

    fix_object(who);

    if (QUERY_FLAG(op, FLAG_CURSED) || QUERY_FLAG(op, FLAG_DAMNED)) {
        if (who->type == PLAYER) {
            if (!(aflags&AP_NOPRINT))
                draw_ext_info(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_CURSED,
                    "Oops, it feels deadly cold!");
            SET_FLAG(tmp2, FLAG_KNOWN_CURSED);
        }
    }

    if (who->map) {
        SET_MAP_FLAGS(who->map, who->x, who->y, P_NEED_UPDATE);
        update_position(who->map, who->x, who->y);
        update_all_los(who->map, who->x, who->y);
    }
}

/**
 * Applies a lamp.
 *
 * @param context
 * method context.
 * @param lamp
 * lamp to turn on/off.
 * @param applier
 * object attempting to apply the scroll. Should be a player.
 * @param aflags
 * special flags (always apply/unapply).
 * @return
 * METHOD_ERROR if lamp can't be applied, METHOD_OK else.
 */
static method_ret lamp_type_apply(ob_methods *context, object *lamp, object *applier, int aflags) {
    object *tmp;

    if (object_get_player_container(lamp) != applier) {
        draw_ext_info_format(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
            "You must get it first!\n");
        return METHOD_ERROR;
    }

    if (lamp->nrof > 1)
        tmp = object_split(lamp, lamp->nrof-1, NULL, 0);
    else
        tmp = NULL;

    if (QUERY_FLAG(lamp, FLAG_APPLIED))
        do_turn(lamp, applier, aflags, "off");
    else {
        if (lamp->stats.food < 1) {
            if (!(aflags&AP_NOPRINT))
                draw_ext_info_format(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_FAILURE,
                    "Your %s is out of fuel!",
                    lamp->name);
            return METHOD_OK;
        }
        do_turn(lamp, applier, aflags, "on");
    }

    /* insert the portion that was split off. */
    if (tmp != NULL) {
        object_insert_in_ob(tmp, applier);
    }

    return METHOD_OK;
}
