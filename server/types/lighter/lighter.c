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

/** @file lighter.c
 * Implementation of the Lighter class of objects.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static method_ret lighter_type_apply(ob_methods *context, object *lighter, object *applier, int aflags);

/**
 * Initializer for the LIGHTER object type.
 */
void init_type_lighter(void) {
    register_apply(LIGHTER, lighter_type_apply);
}

/**
 * Attempts to apply a lighter.
 *
 * Designed primarily to light torches/lanterns/etc.
 * Also burns up burnable material too. First object in the inventory is
 * the selected object to "burn". -b.t.
 *
 * @param context
 * method context.
 * @param lighter
 * lighter to apply.
 * @param applier
 * object attempting to apply the lighter.
 * @param aflags
 * special flags (always apply/unapply).
 * @return
 * METHOD_UNHANDLED if the applier is not a player, METHOD_OK else..
 */
static method_ret lighter_type_apply(ob_methods *context, object *lighter, object *applier, int aflags) {
    object *item;
    int is_player_env = 0;
    uint32 nrof;
    tag_t count;
    char item_name[MAX_BUF];

    if (applier->type != PLAYER)
        return METHOD_UNHANDLED;

    item = find_marked_object(applier);
    if (!item) {
        draw_ext_info(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
            "You need to mark a lightable object.");
        return METHOD_OK;
    }

    if (lighter->last_eat && lighter->stats.food) {
        /* Split multiple lighters if they're being used up.  Otherwise
         * one charge from each would be used up.  --DAMN
         */
        if (lighter->nrof > 1) {
            object *oneLighter = object_split(lighter, 1, NULL, 0);

            oneLighter->stats.food--;
            oneLighter = object_insert_in_ob(oneLighter, applier);
        } else {
            lighter->stats.food--;
        }
    } else if (lighter->last_eat) { /* no charges left in lighter */
        draw_ext_info_format(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_FAILURE,
            "You fail to light the %s with a used up %s.",
            item->name, lighter->name);
        return METHOD_OK;
    }

    /* Perhaps we should split what we are trying to light on fire?
     * I can't see many times when you would want to light multiple
     * objects at once. */
    nrof = item->nrof;
    count = item->count;
    /* If the item is destroyed, we don't have a valid pointer to the
     * name object, so make a copy so the message we print out makes
     * some sense. */
    strncpy(item_name, item->name, sizeof(item_name));
    if (applier == object_get_player_container(item))
        is_player_env = 1;

    save_throw_object(item, AT_FIRE, applier);

    /* Change to check count and not freed, since the object pointer
        * may have gotten recycled */
    if ((nrof != item->nrof) || (count != item->count)) {
        draw_ext_info_format(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
            "You light the %s with the %s.",
            item_name, lighter->name);

        /* Need to update the player so that the players glow radius
         * gets changed. */
        if (is_player_env)
            fix_object(applier);
    } else {
        draw_ext_info_format(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_FAILURE,
            "You attempt to light the %s with the %s and fail.",
            item->name, lighter->name);
    }

    return METHOD_OK;
}
