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

/** @file armour_improver.c
 * Armour improvement scrolls.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static method_ret armour_improver_type_apply(ob_methods *context, object *lighter, object *applier, int aflags);

/**
 * Initializer for the ARMOUR_IMPROVER object type.
 */
void init_type_armour_improver(void) {
    register_apply(ARMOUR_IMPROVER, armour_improver_type_apply);
}

/**
 * This code deals with the armour improvment scrolls.
 * Change limits on improvement - let players go up to
 * +5 no matter what level, but they are limited by item
 * power.
 * Try to use same improvement code as in the common/treasure.c
 * file, so that if you make a +2 full helm, it will be just
 * the same as one you find in a shop.
 *
 * @param op
 * player improving the armor.
 * @param improver
 * improvement scroll.
 * @param armour
 * armour to improve.
 */
static void improve_armour(object *op, object *improver, object *armour) {
    object *tmp;
    int oldmagic = armour->magic;

    if (armour->magic >= settings.armor_max_enchant) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
            "This armour can not be enchanted any further.");
        return;
    }

    /* Dealing with random artifact armor is a lot trickier
     * (in terms of value, weight, etc), so take the easy way out
     * and don't worry about it. Note - maybe add scrolls which
     * make the random artifact versions (eg, armour of gnarg and
     * what not?) */
    if (armour->title) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
            "This armour will not accept further enchantment.");
        return;
    }

    /* Check item power: if player has it equipped, unequip if armor
     * would become unwearable. */
    if (QUERY_FLAG(armour, FLAG_APPLIED)
    && op->type == PLAYER
    && (op->contr->item_power+1) > (settings.item_power_factor*op->level)) {
        apply_special(op, armour, AP_UNAPPLY);
        if (QUERY_FLAG(armour, FLAG_APPLIED)) {
                /* Armour is cursed, too bad */
            draw_ext_info(NDI_UNIQUE, 0, op,
                MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
                "You can't enchant this armour without unapplying it because it would consume your soul!");
            return;
        }
    }

    /* Split objects if needed.  Can't insert tmp until the
     * end of this function - otherwise it will just re-merge. */
    if (armour->nrof > 1)
        tmp = object_split(armour, armour->nrof-1, NULL, 0);
    else
        tmp = NULL;

    armour->magic++;

    if (!settings.armor_speed_linear) {
        int base = 100;
        int pow = 0;

        while (pow < armour->magic) {
            base = base-(base*settings.armor_speed_improvement)/100;
            pow++;
        }

        ARMOUR_SPEED(armour) = (ARMOUR_SPEED(&armour->arch->clone)*base)/100;
    } else
        ARMOUR_SPEED(armour) = (ARMOUR_SPEED(&armour->arch->clone)*(100+armour->magic*settings.armor_speed_improvement))/100;

    if (!settings.armor_weight_linear) {
        int base = 100;
        int pow = 0;

        while (pow < armour->magic) {
            base = base-(base*settings.armor_weight_reduction)/100;
            pow++;
        }
        armour->weight = (armour->arch->clone.weight*base)/100;
    } else
        armour->weight = (armour->arch->clone.weight*(100-armour->magic*settings.armor_weight_reduction))/100;

    if (armour->weight <= 0) {
        LOG(llevInfo, "Warning: enchanted armours can have negative weight\n.");
        armour->weight = 1;
    }

    armour->item_power += (armour->magic-oldmagic)*3;
    if (armour->item_power < 0)
        armour->item_power = 0;

    if (op->type == PLAYER) {
        esrv_update_item(UPD_WEIGHT|UPD_NAME|UPD_NROF, op, armour);
        if (QUERY_FLAG(armour, FLAG_APPLIED))
            fix_object(op);
    }
    object_decrease_nrof_by_one(improver);
    if (tmp) {
        object_insert_in_ob(tmp, op);
    }
    return;
}

/**
 * Applies a scroll of Enchant Armour.
 *
 * @param context
 * method context.
 * @param scroll
 * scroll to apply.
 * @param applier
 * object attempting to apply the scroll. Should be a player.
 * @param aflags
 * special flags (always apply/unapply).
 * @return
 * METHOD_OK if applier is a player, METHOD_UNHANDLED else.
 */
static method_ret armour_improver_type_apply(ob_methods *context, object *scroll, object *applier, int aflags) {
    object *armor;

    if (applier->type != PLAYER)
        return METHOD_UNHANDLED;

    if (!QUERY_FLAG(applier, FLAG_WIZCAST)
    && (get_map_flags(applier->map, NULL, applier->x, applier->y, NULL, NULL)&P_NO_MAGIC)) {
        draw_ext_info(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
            "Something blocks the magic of the scroll.");
        return METHOD_OK;
    }
    armor = find_marked_object(applier);
    if (!armor) {
        draw_ext_info(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
                      "You need to mark an armor object.");
        return METHOD_OK;
    }
    if (armor->type != ARMOUR
    && armor->type != CLOAK
    && armor->type != BOOTS
    && armor->type != GLOVES
    && armor->type != BRACERS
    && armor->type != SHIELD
    && armor->type != HELMET) {
        draw_ext_info(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
            "Your marked item is not armour!");
        return METHOD_OK;
    }

    draw_ext_info(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
        "Applying armour enchantment.");
    improve_armour(applier, scroll, armor);
    return METHOD_OK;
}
