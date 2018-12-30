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

/** @file weapon_improver.c
 * The implementation of the Weapon Improver class of objects.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static method_ret weapon_improver_type_apply(ob_methods *context, object *op, object *applier, int aflags);
static int check_item(object *op, const char *item);
static void eat_item(object *op, const char *item, uint32 nrof);
static int check_sacrifice(object *op, const object *improver);
static int improve_weapon_stat(object *op, object *improver, object *weapon, signed char *stat, int sacrifice_count, const char *statname);
static int prepare_weapon(object *op, object *improver, object *weapon);
static int improve_weapon(object *op, object *improver, object *weapon);

/**
 * Initializer for the WEAPON_IMPROVER object type.
 */
void init_type_weapon_improver(void) {
    register_apply(WEAPON_IMPROVER, weapon_improver_type_apply);
}

/**
 * Attempts to apply weapon_improver.
 * @param context The method context
 * @param op The weapon_improver to apply
 * @param applier The object attempting to apply the weapon_improver. Ignored unless
 * a player
 * @param aflags Special flags (always apply/unapply)
 * @return The return value is METHOD_OK unless it fails to apply.
 */
static method_ret weapon_improver_type_apply(ob_methods *context, object *op, object *applier, int aflags) {
    object *oop;

    if (applier->type != PLAYER)
        return METHOD_ERROR;
    if (!QUERY_FLAG(applier, FLAG_WIZCAST)
    && (get_map_flags(applier->map, NULL, applier->x, applier->y, NULL, NULL)&P_NO_MAGIC)) {
        draw_ext_info(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
            "Something blocks the magic of the scroll.");
        return METHOD_ERROR;
    }

    oop = find_marked_object(applier);
    if (!oop) {
        draw_ext_info(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
            "You need to mark a weapon object.");
        return METHOD_ERROR;
    }
    if (oop->type != WEAPON && oop->type != BOW) {
        draw_ext_info(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
            "Marked item is not a weapon or bow");
        return METHOD_ERROR;
    }
    draw_ext_info(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
        "Applied weapon builder.");
    improve_weapon(applier, op, oop);
    esrv_update_item(UPD_NAME|UPD_NROF|UPD_FLAGS, applier, oop);
    return METHOD_OK;
}

/**
 * Counts suitable items with specified archetype name. Will not consider unpaid/cursed items.
 * @param op
 * object just before the bottom of the pile, others will be checked through object->below.
 * @param item
 * what archetype to check for.
 * @return
 * count of matching items.
 * @todo
 * couldn't item be a shared string, and == be used instead of strcmp?
 * The op = op->below is weird - what is it's NULL?
 */
static int check_item(object *op, const char *item) {
    int count = 0;

    if (item == NULL)
        return 0;

    FOR_BELOW_PREPARE(op, tmp) {
        if (strcmp(tmp->arch->name, item) == 0) {
            if (!QUERY_FLAG(tmp, FLAG_CURSED)
            && !QUERY_FLAG(tmp, FLAG_DAMNED)
            /* Loophole bug? -FD- */ && !QUERY_FLAG(tmp, FLAG_UNPAID)) {
                if (tmp->nrof == 0)/* this is necessary for artifact sacrifices --FD-- */
                    count++;
                else
                    count += tmp->nrof;
            }
        }
    } FOR_BELOW_FINISH();
    return count;
}

/**
 * This removes 'nrof' items with specified archetype.
 * op is typically the player, which is only
 * really used to determine what space to look at.
 * Modified to only eat 'nrof' of objects.
 *
 * @param op
 * item at the bottom to check.
 * @param item
 * archetype to look for.
 * @param nrof
 * count to remove.
 * @todo
 * couldn't item be a shared string, and use == instead of strcmp?
 * also, the remove logic is wrong - op->nrof will be 0 after decreat_ob_nr in the 2nd case.
 */
static void eat_item(object *op, const char *item, uint32 nrof) {
    object *prev;

    prev = op;
    op = op->below;

    while (op != NULL) {
        if (strcmp(op->arch->name, item) == 0) {
            if (op->nrof >= nrof) {
                object_decrease_nrof(op, nrof);
                return;
            } else {
                object_decrease_nrof(op, op->nrof);
                nrof -= op->nrof;
            }
            op = prev;
        }
        prev = op;
        op = op->below;
    }
}

/**
 * Returns how many items of type improver->slaying there are under op.
 * Will display a message if none found, and 1 if improver->slaying is NULL.
 *
 * @param op
 * item just below the bottom of the pile.
 * @param improver
 * sacrifice object.
 * @return
 * count of matching items.
 * @todo
 * weird logic? use shared string directly, improver isn't really useful.
 */
static int check_sacrifice(object *op, const object *improver) {
    int count = 0;

    if (improver->slaying != NULL) {
        count = check_item(op, improver->slaying);
        if (count < 1) {
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
                "The gods want more %ss",
                improver->slaying);
            return 0;
        }
    } else
        count = 1;

    return count;
}

/**
 * Actually improves the weapon, and tells user. Won't test anything.
 *
 * @param op
 * player improving.
 * @param improver
 * scroll used to improve.
 * @param weapon
 * improved weapon.
 * @param stat
 * what statistic to improve.
 * @param sacrifice_count
 * how much to improve stat by.
 * @param statname
 * name of stat to display to player.
 * @return
 * 1.
 */
static int improve_weapon_stat(object *op, object *improver, object *weapon, signed char *stat, int sacrifice_count, const char *statname) {
    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
        "Your sacrifice was accepted.");

    *stat += sacrifice_count;
    weapon->last_eat++;

    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
        "Weapon's bonus to %s improved by %d",
        statname, sacrifice_count);

    object_decrease_nrof_by_one(improver);

        /* So it updates the players stats and the window */
    fix_object(op);
    return 1;
}

/* Types of improvements, hidden in the sp field. */
#define IMPROVE_PREPARE 1   /**< Prepare the weapon. */
#define IMPROVE_DAMAGE 2    /**< Increase damage. */
#define IMPROVE_WEIGHT 3    /**< Decrease weight. */
#define IMPROVE_ENCHANT 4   /**< Increase magic. */
#define IMPROVE_STR 5       /**< Increase strength bonus. */
#define IMPROVE_DEX 6       /**< Increase dexterity bonus. */
#define IMPROVE_CON 7       /**< Increase constitution bonus. */
#define IMPROVE_WIS 8       /**< Increase wisdom bonus. */
#define IMPROVE_CHA 9       /**< Increase charisma bonus. */
#define IMPROVE_INT 10      /**< Increase intelligence bonus. */
#define IMPROVE_POW 11      /**< Increase power bonus. */

/**
 * This does the prepare weapon scroll.
 *
 * Checks for sacrifice, and so on. Will inform the player of failures or success.
 *
 * @param op
 * player using the scroll.
 * @param improver
 * improvement scroll.
 * @param weapon
 * weapon to improve.
 * @return
 * 1 if weapon was prepared, 0 else.
 */
static int prepare_weapon(object *op, object *improver, object *weapon) {
    int sacrifice_count, i;
    char buf[MAX_BUF];

    if (weapon->level != 0) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
            "Weapon already prepared.");
        return 0;
    }
    for (i = 0; i < NROFATTACKS; i++)
        if (weapon->resist[i])
            break;

        /* If we break out, i will be less than nrofattacks, preventing
         * improvement of items that already have protections.
         */
    if (i < NROFATTACKS
    || weapon->stats.hp        /* regeneration */
    || (weapon->stats.sp && weapon->type == WEAPON) /* sp regeneration */
    || weapon->stats.exp       /* speed */
    || weapon->stats.ac) {     /* AC - only taifu's I think */
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
            "Cannot prepare magic weapons.");
        return 0;
    }

    sacrifice_count = check_sacrifice(op, improver);
    if (sacrifice_count <= 0)
        return 0;

    /* We do not allow improving stacks, so split this off from
     * stack.  Only need to do this if weapon is part of a stack.
     * We set nrof of weapon to zero so it can not merge with other
     * items, so one can not do further improvements on a stack.
     * side effect of doing it before the object_insert_in_ob() is that
     * it won't merge back in.  We know from the code that marked
     * objects must be in the players inventory, so we know where
     * to put this.
     */
    if (weapon->nrof >1) {
        weapon = object_split(weapon,1, NULL, 0);
        weapon->nrof = 0;
        object_insert_in_ob(weapon, op);
    } else {
        weapon->nrof = 0;
    }


    weapon->level = isqrt(sacrifice_count);
    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
        "Your sacrifice was accepted.");
    eat_item(op, improver->slaying, sacrifice_count);


    snprintf(buf, sizeof(buf), "%s's %s", op->name, weapon->name);
    FREE_AND_COPY(weapon->name, buf);
    FREE_AND_COPY(weapon->name_pl, buf);

    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
        "Your %s may be improved %d times.",
        weapon->name, weapon->level);

    object_decrease_nrof_by_one(improver);
    weapon->last_eat = 0;
    esrv_update_item(UPD_NAME | UPD_NROF, op, weapon);
    return 1;
}

/**
 * Does the dirty job for 'improve weapon' scroll, prepare or add something.
 * This is the new improve weapon code.
 *
 * Checks if weapon was prepared, if enough potions on the floor, ...
 *
 * We are hiding extra information about the weapon in the level and
 * last_eat numbers for an object.  Hopefully this won't break anything ??
 * level == max improve last_eat == current improve
 *
 * @param op
 * player improving.
 * @param improver
 * the scroll that was read.
 * @param weapon
 * wepaon to improve.
 * @return
 * 1 if weapon was improved, 0 if not enough sacrifice, weapon not prepared, ...
 */
static int improve_weapon(object *op, object *improver, object *weapon) {
    int sacrifice_count, sacrifice_needed = 0;

    if (improver->stats.sp == IMPROVE_PREPARE) {
        return prepare_weapon(op, improver, weapon);
    }

    if (weapon->level == 0) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
            "This weapon has not been prepared.");
        return 0;
    }

    if (weapon->level == weapon->last_eat && weapon->item_power >= MAX_WEAPON_ITEM_POWER) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
            "This weapon cannot be improved any more.");
        return 0;
    }

    if (QUERY_FLAG(weapon, FLAG_APPLIED)
    && !apply_check_weapon_power(op, weapon->last_eat+1)) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
            "Improving the weapon will make it too powerful for you to use.  Unready it if you really want to improve it.");
        return 0;
    }

    /* All improvements add to item power, so check if player can
     * still wear the weapon after improvement.
     */
    if (QUERY_FLAG(weapon, FLAG_APPLIED)
    && op->type == PLAYER
    && (op->contr->item_power+1) > (settings.item_power_factor*op->level)) {
        apply_special(op, weapon, AP_UNAPPLY);
        if (QUERY_FLAG(weapon, FLAG_APPLIED)) {
            /* Weapon is cursed, too bad */
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
                "You can't enchant this weapon without unapplying it because it would consume your soul!");
            return 0;
        }
    }

    /* This just increases damage by 5 points, no matter what.  No
     * sacrifice is needed.  Since stats.dam is now a 16 bit value and
     * not 8 bit, don't put any maximum value on damage - the limit is
     * how much the weapon  can be improved.
     */
    if (improver->stats.sp == IMPROVE_DAMAGE) {
        weapon->stats.dam += 5;
        weapon->weight += 5000;         /* 5 KG's */
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
            "Damage has been increased by 5 to %d",
            weapon->stats.dam);
        weapon->last_eat++;

        weapon->item_power++;
        object_decrease_nrof_by_one(improver);
        esrv_update_item(UPD_WEIGHT, op, weapon);
        return 1;
    }

    if (improver->stats.sp == IMPROVE_WEIGHT) {
        /* Reduce weight by 20% */
        weapon->weight = (weapon->weight*8)/10;
        if (weapon->weight < 1)
            weapon->weight = 1;
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
            "Weapon weight reduced to %6.1f kg",
            (float)weapon->weight/1000.0);
        weapon->last_eat++;
        weapon->item_power++;
        object_decrease_nrof_by_one(improver);
        esrv_update_item(UPD_WEIGHT, op, weapon);
        return 1;
    }

    if (improver->stats.sp == IMPROVE_ENCHANT) {
        weapon->magic++;
        weapon->last_eat++;
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
            "Weapon magic increased to %d",
            weapon->magic);
        object_decrease_nrof_by_one(improver);
        weapon->item_power++;
        return 1;
    }

    sacrifice_needed = weapon->stats.Str
        +weapon->stats.Int
        +weapon->stats.Dex
        +weapon->stats.Pow
        +weapon->stats.Con
        +weapon->stats.Cha
        +weapon->stats.Wis;

    if (sacrifice_needed < 1)
        sacrifice_needed = 1;
    sacrifice_needed *= 2;

    sacrifice_count = check_sacrifice(op, improver);
    if (sacrifice_count < sacrifice_needed) {
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
            "You need at least %d %s",
            sacrifice_needed, improver->slaying);
        return 0;
    }
    eat_item(op, improver->slaying, sacrifice_needed);
    weapon->item_power++;

    switch (improver->stats.sp) {
    case IMPROVE_STR:
        return improve_weapon_stat(op, improver, weapon, (signed char *)&(weapon->stats.Str), 1, "strength");

    case IMPROVE_DEX:
        return improve_weapon_stat(op, improver, weapon, (signed char *)&(weapon->stats.Dex), 1, "dexterity");

    case IMPROVE_CON:
        return improve_weapon_stat(op, improver, weapon, (signed char *)&(weapon->stats.Con), 1, "constitution");

    case IMPROVE_WIS:
        return improve_weapon_stat(op, improver, weapon, (signed char *)&(weapon->stats.Wis), 1, "wisdom");

    case IMPROVE_CHA:
        return improve_weapon_stat(op, improver, weapon, (signed char *)&(weapon->stats.Cha), 1, "charisma");

    case IMPROVE_INT:
        return improve_weapon_stat(op, improver, weapon, (signed char *)&(weapon->stats.Int), 1, "intelligence");

    case IMPROVE_POW:
        return improve_weapon_stat(op, improver, weapon, (signed char *)&(weapon->stats.Pow), 1, "power");

    default:
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
            "Unknown improvement type.");
    }

    LOG(llevError, "improve_weapon: Got to end of function\n");
    return 0;
}
