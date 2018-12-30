/*
 * Crossfire -- cooperative multi-player graphical RPG and adventure game
 *
 * Copyright (c) 1999-2013 Mark Wedel and the Crossfire Development Team
 * Copyright (c) 1992 Frank Tore Johansen
 *
 * Crossfire is free software and comes with ABSOLUTELY NO WARRANTY. You are
 * welcome to redistribute it under certain conditions. For details, see the
 * 'LICENSE' and 'COPYING' files.
 *
 * The authors can be reached via e-mail to crossfire-devel@real-time.com
 *
 * skills.c -- core skill handling
 */

#include "global.h"
#include "object.h"
#ifndef __CEXTRACT__
#include <sproto.h>
#endif
#include "living.h"
#include "skills.h"
#include "spells.h"
#include "book.h"

/**
 * Computes stealing chance.
 * Increased values indicate better attempts.
 *
 * @param op
 * who is stealing.
 * @param victim
 * who to steal from.
 * @param roll
 * dice roll.
 * @return
 * -1 if op can't steal, else adjusted roll value.
 * @todo rename roll to something more meaningful (check attempt_steal()).
 */
static int adj_stealchance(object *op, object *victim, int roll) {
    if (!op || !victim || !roll)
        return -1;

    /* Only prohibit stealing if the player does not have a free
     * hand available and in fact does have hands.
     */
    if (op->type == PLAYER
    && op->body_used[BODY_ARMS] <= 0
    && op->body_info[BODY_ARMS]) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                      "But you have no free hands to steal with!");
        return -1;
    }

    /* ADJUSTMENTS */

    /* Its harder to steal from hostile beings! */
    if (!QUERY_FLAG(victim, FLAG_UNAGGRESSIVE))
        roll = roll/2;

    /* Easier to steal from sleeping beings, or if the thief is
     * unseen */
    if (QUERY_FLAG(victim, FLAG_SLEEP))
        roll = roll*3;
    else if (op->invisible)
        roll = roll*2;

    /* check stealing 'encumberance'. Having this equipment applied makes
     * it quite a bit harder to steal.
     */
    FOR_INV_PREPARE(op, equip) {
        if (equip->type == WEAPON && QUERY_FLAG(equip, FLAG_APPLIED)) {
            roll -= equip->weight/10000;
        }
        if (equip->type == BOW && QUERY_FLAG(equip, FLAG_APPLIED))
            roll -= equip->weight/5000;
        if (equip->type == SHIELD && QUERY_FLAG(equip, FLAG_APPLIED)) {
            roll -= equip->weight/2000;
        }
        if (equip->type == ARMOUR && QUERY_FLAG(equip, FLAG_APPLIED))
            roll -= equip->weight/5000;
        if (equip->type == GLOVES && QUERY_FLAG(equip, FLAG_APPLIED))
            roll -= equip->weight/100;
    } FOR_INV_FINISH();
    if (roll < 0)
        roll = 0;
    return roll;
}

/**
 * Steal objects.
 * When stealing: dependent on the intelligence/wisdom of whom you're
 * stealing from (op in attempt_steal), offset by your dexterity and
 * skill at stealing. They may notice your attempt, whether successful
 * or not.
 *
 * @param op
 * target (person being pilfered).
 * @param who
 * person doing the stealing.
 * @param skill
 * stealing skill object.
 * @retval 0
 * nothing was stolen.
 * @retval 1
 * something was stolen.
 */
static int attempt_steal(object *op, object *who, object *skill) {
    object *success = NULL, *tmp = NULL;
    int roll = 0, chance = 0, stats_value;
    rv_vector rv;
    char name[MAX_BUF];

    stats_value = ((who->stats.Dex+who->stats.Int)*3)/2;

    /* if the victim is aware of a thief in the area (FLAG_NO_STEAL set on them)
     * they will try to prevent stealing if they can. Only unseen theives will
     * have much chance of success.
     */
    if (op->type != PLAYER && QUERY_FLAG(op, FLAG_NO_STEAL)) {
        if (monster_can_detect_enemy(op, who, &rv)) {
            monster_npc_call_help(op);
            CLEAR_FLAG(op, FLAG_UNAGGRESSIVE);
            draw_ext_info(NDI_UNIQUE, 0, who, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                          "Your attempt is prevented!");
            return 0;
        }

        /* help npc to detect thief next time by raising its wisdom
         * This probably isn't the right approach - we shouldn't be
         * changing the stats of the monsters - better approach
         * might be to use force objects for this - MSW 2009/02/24
         */
        op->stats.Wis += (op->stats.Int/5)+1;
        if (op->stats.Wis > settings.max_stat)
            op->stats.Wis = settings.max_stat;
    }
    if (op->type == PLAYER && QUERY_FLAG(op, FLAG_WIZ)) {
        draw_ext_info(NDI_UNIQUE, 0, who, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                      "You can't steal from the dungeon master!");
        return 0;
    }
    if (op->type == PLAYER && who->type == PLAYER && settings.no_player_stealing) {
        draw_ext_info(NDI_UNIQUE, 0, who, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                      "You can't steal from other players!");
        return 0;
    }


    /* Ok then, go thru their inventory, stealing */
    FOR_INV_PREPARE(op, inv) {
        /* you can't steal worn items, starting items, wiz stuff,
         * innate abilities, or items w/o a type. Generally
         * speaking, the invisibility flag prevents experience or
         * abilities from being stolen since these types are currently
         * always invisible objects. I was implicit here so as to prevent
         * future possible problems. -b.t.
         * Flesh items generated w/ fix_flesh_item should have FLAG_NO_STEAL
         * already  -b.t.
         */

        if (QUERY_FLAG(inv, FLAG_WAS_WIZ)
        || QUERY_FLAG(inv, FLAG_APPLIED)
        || !(inv->type)
        || inv->type == SPELL
        || QUERY_FLAG(inv, FLAG_STARTEQUIP)
        || QUERY_FLAG(inv, FLAG_NO_STEAL)
        || inv->invisible)
            continue;

        /* Okay, try stealing this item. Dependent on dexterity of thief,
         * skill level, see the adj_stealroll fctn for more detail.
         */

        roll = die_roll(2, 100, who, PREFER_LOW)/2; /* weighted 1-100 */

        chance = adj_stealchance(who, op, stats_value+skill->level*10-op->level*3);
        if (chance == -1)
            return 0;
        if (roll < chance) {
            tag_t inv_count = inv->count;

            pick_up(who, inv);
            /* need to see if the player actually stole this item -
             * if it is in the players inv, assume it is.  This prevents
             * abuses where the player can not carry the item, so just
             * keeps stealing it over and over.
             */
            if (object_was_destroyed(inv, inv_count) || inv->env != op) {
                /* for players, play_sound: steals item */
                success = inv;
                CLEAR_FLAG(inv, FLAG_INV_LOCKED);
            }
            tmp = inv;
            break;
        }
    } FOR_INV_FINISH(); /* for loop looking for an item */

    if (!tmp) {
        query_name(op, name, MAX_BUF);
        draw_ext_info_format(NDI_UNIQUE, 0, who, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                             "%s%s has nothing you can steal!",
                             op->type == PLAYER ? "" : "The ", name);
        return 0;
    }

    /* If you arent high enough level, you might get something BUT
     * the victim will notice your stealing attempt. Ditto if you
     * attempt to steal something heavy off them, they're bound to notice
     */

    if (roll >= skill->level
    || !chance
    || (tmp && tmp->weight > 250*random_roll(0, stats_value+skill->level*10-1, who, PREFER_LOW))) {
        /* victim figures out where the thief is! */
        if (who->hide)
            make_visible(who);

        if (op->type != PLAYER) {
            /* The unaggressives look after themselves 8) */
            if (who->type == PLAYER) {
                monster_npc_call_help(op);
                query_name(op, name, MAX_BUF);
                draw_ext_info_format(NDI_UNIQUE, 0, who, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                                     "%s notices your attempted pilfering!",
                                     name);
            }
            CLEAR_FLAG(op, FLAG_UNAGGRESSIVE);
            /* all remaining npc items are guarded now. Set flag NO_STEAL
             * on the victim.
             */
            SET_FLAG(op, FLAG_NO_STEAL);
        } else { /* stealing from another player */
            char buf[MAX_BUF];

            /* Notify the other player */
            if (success && who->stats.Int > random_roll(0, 19, op, PREFER_LOW)) {
                query_name(success, name, MAX_BUF);
                snprintf(buf, sizeof(buf), "Your %s is missing!", name);
            } else {
                snprintf(buf, sizeof(buf), "Your pack feels strangely lighter.");
            }
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ITEM, MSG_TYPE_ITEM_REMOVE,
                          buf);
            if (!success) {
                if (who->invisible) {
                    snprintf(buf, sizeof(buf), "you feel itchy fingers getting at your pack.");
                } else {
                    query_name(who, name, MAX_BUF);
                    snprintf(buf, sizeof(buf), "%s looks very shifty.", name);
                }
                draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_VICTIM, MSG_TYPE_VICTIM_STEAL,
                              buf);
            }
        } /* else stealing from another player */
        /* play_sound("stop! thief!"); kindofthing */
    } /* if you weren't 100% successful */
    return success ? 1 : 0;
}


/**
 * Main stealing function.
 *
 * @param op
 * thief.
 * @param dir
 * direction to steal from.
 * @param skill
 * stealing skill.
 * @return
 * experience gained for stealing, 0 if nothing was stolen.
 */
int steal(object *op, int dir, object *skill) {
    object *tmp;
    sint16 x, y;
    mapstruct *m;
    int mflags;

    x = op->x+freearr_x[dir];
    y = op->y+freearr_y[dir];

    if (dir == 0) {
        /* Can't steal from ourself! */
        return 0;
    }

    m = op->map;
    mflags = get_map_flags(m, &m, x, y, &x, &y);
    /* Out of map - can't do it.  If nothing alive on this space,
     * don't need to look any further.
     */
    if ((mflags&P_OUT_OF_MAP) || !(mflags&P_IS_ALIVE))
        return 0;

    /* If player can't move onto the space, can't steal from it. */
    if (OB_TYPE_MOVE_BLOCK(op, GET_MAP_MOVE_BLOCK(m, x, y)))
        return 0;

    /* Find the topmost object at this spot */
    tmp = GET_MAP_OB(m, x, y);
    FOR_OB_AND_ABOVE_PREPARE(tmp)
        if (tmp->above == NULL)
            break;
    FOR_OB_AND_ABOVE_FINISH();

    /* For all the stacked objects at this point, attempt a steal */
    FOR_OB_AND_BELOW_PREPARE(tmp) {
        /* Minor hack--for multi square beings - make sure we get
         * the 'head' coz 'tail' objects have no inventory! - b.t.
         */
        tmp = HEAD(tmp);
        if (tmp->type != PLAYER && !QUERY_FLAG(tmp, FLAG_MONSTER))
            continue;

        /* do not reveal hidden DMs */
        if (tmp->type == PLAYER && QUERY_FLAG(tmp, FLAG_WIZ) && tmp->contr->hidden)
            continue;
        if (attempt_steal(tmp, op, skill)) {
            if (tmp->type == PLAYER) /* no xp for stealing from another player */
                return 0;

            /* no xp for stealing from pets (of players) */
            if (QUERY_FLAG(tmp, FLAG_FRIENDLY) && tmp->attack_movement == PETMOVE) {
                object *owner = object_get_owner(tmp);
                if (owner != NULL && owner->type == PLAYER)
                    return 0;
            }

            return (calc_skill_exp(op, tmp, skill));
        }
    } FOR_OB_AND_BELOW_FINISH();
    return 0;
}

/**
 * Attempt to pick a lock. Handles traps.
 *
 * @param door
 * lock to pick.
 * @param pl
 * player picking.
 * @param skill
 * locking skill.
 * @retval 0
 * no lock was picked.
 * @retval 1
 * door was locked.
 */
static int attempt_pick_lock(object *door, object *pl, object *skill) {
    int difficulty = pl->map->difficulty ? pl->map->difficulty : 0;
    int success = 0, number;        /* did we get anything? */

    /* Try to pick the lock on this item (doors only for now).
     * Dependent on dexterity/skill SK_level of the player and
     * the map level difficulty.
     */
    number = (die_roll(2, 40, pl, PREFER_LOW)-2)/2;
    if (number < pl->stats.Dex + skill->level*2 - difficulty ) {
        remove_door(door);
        success = difficulty;
    } else if (door->inv && (door->inv->type == RUNE || door->inv->type == TRAP)) {  /* set off any traps? */
        spring_trap(door->inv, pl);
    }
    return success;
}


/**
 * Lock pick handling.
 *
 * Implementation by bt. (thomas@astro.psu.edu)
 * monster implementation 7-7-95 by bt.
 *
 * @param pl
 * player picking the lock.
 * @param dir
 * direction to pick.
 * @param skill
 * lock picking skill.
 * @return
 * experience for picking a lock, 0 if nothing was picked.
 */
int pick_lock(object *pl, int dir, object *skill) {
    object *tmp;
    int x = pl->x+freearr_x[dir];
    int y = pl->y+freearr_y[dir];
    int difficulty=0;

    if (!dir)
        dir = pl->facing;

    /* For all the stacked objects at this point find a door*/
    if (out_of_map(pl->map, x, y)) {
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                      "There is no lock there.");
        return 0;
    }

    for (tmp = GET_MAP_OB(pl->map, x, y); tmp; tmp = tmp->above)
        if (tmp->type == DOOR || tmp->type == LOCKED_DOOR)
            break;

    if (!tmp) {
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                      "There is no lock there.");
        return 0;
    }

    if (execute_event(tmp, EVENT_TRIGGER, pl, skill, NULL, SCRIPT_FIX_ALL) != 0)
        return 0;

    if (tmp->type == LOCKED_DOOR) {
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                      "You can't pick that lock!");
        return 0;
    }

    if (!tmp->move_block) {
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                      "The door has no lock!");
        return 0;
    }

    difficulty = attempt_pick_lock(tmp, pl, skill);
    /* Failure */
    if (!difficulty) {
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                      "You fail to pick the lock.");
        return 0;
    }

    draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_SUCCESS,
                  "You pick the lock.");
    return (calc_skill_exp(pl, NULL, skill) * isqrt(difficulty));
}


/**
 * Someone is trying to hide.
 * The user becomes undetectable (not just 'invisible') for
 * a short while (success and duration dependant on player SK_level,
 * dexterity, charisma, and map difficulty).
 * Players have a good chance of becoming 'unhidden' if they move
 * and like invisiblity will be come visible if they attack
 * Implemented by b.t. (thomas@astro.psu.edu)
 * July 7, 1995 - made hiding possible for monsters. -b.t.
 *
 * @param op
 * living trying to hide.
 * @param skill
 * hiding skill.
 * @retval 0
 * op couldn't hide.
 * @retval 1
 * op successfully hide.
 */
static int attempt_hide(object *op, object *skill) {
    int number, difficulty = op->map->difficulty;
    int terrain = hideability(op);

    if (terrain < -10) /* not enough cover here */
        return 0;

    /*  Hiding success and duration dependant on skill level,
     *  op->stats.Dex, map difficulty and terrain.
     */

    number = (die_roll(2, 25, op, PREFER_LOW)-2)/2;
    if (!stand_near_hostile(op) && number < op->stats.Dex+skill->level+terrain-difficulty) {
        op->invisible += 100;  /* set the level of 'hiddeness' */
        if (op->type == PLAYER)
            op->contr->tmp_invis = 1;
        op->hide = 1;
        return 1;
    }
    return 0;
}

/**
 * Main hide handling.
 * @param op
 * living trying to hide.
 * @param skill
 * hiding skill.
 * @return
 * experience gained for the skill use (can be 0).
 */
int hide(object *op, object *skill) {
    /* the preliminaries -- Can we really hide now? */
    /* this keeps monsters from using invisibilty spells and hiding */

    if (QUERY_FLAG(op, FLAG_MAKE_INVIS)) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                      "You don't need to hide while invisible!");
        return 0;
    }

    if (!op->hide && op->invisible > 0 && op->type == PLAYER) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_GOOD_EFFECT_END,
                      "Your attempt to hide breaks the invisibility spell!");
        make_visible(op);
    }

    if (op->invisible > 50*skill->level) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                      "You are as hidden as you can get.");
        return 0;
    }

    if (attempt_hide(op, skill)) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_SUCCESS,
                      "You hide in the shadows.");
        object_update(op, UP_OBJ_FACE);
        return calc_skill_exp(op, NULL, skill);
    }
    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                  "You fail to conceal yourself.");
    return 0;
}


/**
 * End of jump. Restore the map.
 * @param pl
 * player.
 * @todo Is fix_object() required?
 */
static void stop_jump(object *pl) {
    fix_object(pl);
    object_insert_in_map_at(pl, pl->map, pl, 0, pl->x, pl->y);
}

/**
 * Someone is trying to jump.
 * @param pl
 * living trying to jump.
 * @param dir
 * direction to jump in.
 * @param spaces
 * distance to jump.
 * @param skill
 * jumping skill.
 * @return
 * 1 if jump was successful, 0 else.
 */
static int attempt_jump(object *pl, int dir, int spaces, object *skill) {
    int i, dx = freearr_x[dir], dy = freearr_y[dir], mflags;
    sint16 x, y;
    mapstruct *m;

    /* Jump loop. Go through spaces opject wants to jump. Halt the
     * jump if a wall or creature is in the way. We set FLY_LOW
     * temporarily to allow player to aviod exits/archs that are not
     * move_on/off fly_low. This will also prevent pickup of objects
     * while jumping over them.
     */

    object_remove(pl);

    /*
     * I don't think this is actually needed - all the movement
     * code is handled in this function, and I don't see anyplace
     * that cares about the move_type being flying.
     */
    pl->move_type |= MOVE_FLY_LOW;

    for (i = 0; i <= spaces; i++) {
        x = pl->x+dx;
        y = pl->y+dy;
        m = pl->map;

        mflags = get_map_flags(m, &m, x, y, &x, &y);

        if (mflags&P_OUT_OF_MAP) {
            (void)stop_jump(pl);
            return 0;
        }
        if (OB_TYPE_MOVE_BLOCK(pl, GET_MAP_MOVE_BLOCK(m, x, y))) {
            draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                          "Your jump is blocked.");
            stop_jump(pl);
            return 0;
        }

        FOR_MAP_PREPARE(m, x, y, tmp) {
            tmp = HEAD(tmp);
            /* Jump into creature */
            if (QUERY_FLAG(tmp, FLAG_MONSTER)
                || (tmp->type == PLAYER && (!QUERY_FLAG(tmp, FLAG_WIZ) || !tmp->contr->hidden))) {
                draw_ext_info_format(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_SUCCESS,
                                     "You jump into %s%s.",
                                     tmp->type == PLAYER ? "" : "the ", tmp->name);

                stop_jump(pl);
                if (tmp->type != PLAYER
                || (pl->type == PLAYER && pl->contr->party == NULL)
                || (pl->type == PLAYER && tmp->type == PLAYER && pl->contr->party != tmp->contr->party))
                    skill_attack(tmp, pl, pl->facing, "kicked", skill); /* pl makes an attack */

                return 1;
            }
            /* If the space has fly on set (no matter what the space is),
             * we should get the effects - after all, the player is
             * effectively flying.
             */
            if (tmp->move_on&MOVE_FLY_LOW) {
                pl->x = x;
                pl->y = y;
                pl->map = m;
                if (pl->contr)
                    esrv_map_scroll(&pl->contr->socket, dx, dy);
                stop_jump(pl);
                return 1;
            }
        } FOR_MAP_FINISH();
        pl->x = x;
        pl->y = y;
        pl->map = m;
        if (pl->contr)
            esrv_map_scroll(&pl->contr->socket, dx, dy);
    }
    stop_jump(pl);
    return 1;
}

/**
 * Jump skill handling.
 * This is both a new type of movement for player/monsters and
 * an attack as well.
 * Perhaps we should allow more spaces based on level, eg, level 50
 * jumper can jump several spaces?
 *
 * @param pl
 * object jumping.
 * @param dir
 * direction to jump to.
 * @param skill
 * jumping skill.
 * @return
 * 1 if jump was successful, 0 else
 */
int jump(object *pl, int dir, object *skill) {
    int spaces = 0, stats;
    int str = pl->stats.Str;
    int dex = pl->stats.Dex;

    dex = dex ? dex : 15;
    str = str ? str : 10;

    stats = str*str*str*dex*skill->level;

    if (pl->carrying != 0) /* don't want div by zero !! */
        spaces = (int)(stats/pl->carrying);
    else
        spaces = 2; /* pl has no objects - gets the far jump */

    if (spaces > 2)
        spaces = 2;
    else if (spaces == 0) {
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                      "You are carrying too much weight to jump.");
        return 0;
    }
    return attempt_jump(pl, dir, spaces, skill);
}

/**
 * Runs a 'detect curse' check on a given item
 *
 * @param pl
 * player detecting a curse.
 * @param tmp
 * object to check for curse
 * @param skill
 * detect skill object.
 * @return
 * amount of experience gained (on successful detecting).
 */
int detect_curse_on_item(object *pl, object *tmp, object *skill) {
    if (!QUERY_FLAG(tmp, FLAG_IDENTIFIED) && !QUERY_FLAG(tmp, FLAG_KNOWN_CURSED)
            && (QUERY_FLAG(tmp, FLAG_CURSED) || QUERY_FLAG(tmp, FLAG_DAMNED))
            && tmp->item_power < skill->level) {
        SET_FLAG(tmp, FLAG_KNOWN_CURSED);
        esrv_update_item(UPD_FLAGS, pl, tmp);
        return calc_skill_exp(pl, tmp, skill);
    }
    return 0;
}
/**
 * Check for cursed object with the 'detect curse' skill.
 *
 * @param pl
 * player detecting.
 * @param skill
 * detect skill object.
 * @return
 * amount of experience gained (on successful detecting).
 */
static int do_skill_detect_curse(object *pl, object *skill) {
    int success = 0;

    FOR_INV_PREPARE(pl, tmp)
        if (!tmp->invisible) success += detect_curse_on_item(pl, tmp, skill);
    FOR_INV_FINISH();

    /* Check ground, too, but only objects the player could pick up. Cauldrons are exceptions,
     * you definitely want to know if they are cursed */
    FOR_MAP_PREPARE(pl->map, pl->x, pl->y, tmp)
        if (object_can_pick(pl, tmp) || QUERY_FLAG(tmp, FLAG_IS_CAULDRON))
            success += detect_curse_on_item(pl, tmp, skill);
    FOR_MAP_FINISH();

    return success;
}
/**
 * Runs a 'detect magic' check on a given item
 *
 * @param pl
 * player detecting magic.
 * @param tmp
 * object to check for magic
 * @param skill
 * detect skill object.
 * @return
 * amount of experience gained (on successful detecting).
 */

int detect_magic_on_item(object *pl, object *tmp, object *skill) {
    if (!QUERY_FLAG(tmp, FLAG_IDENTIFIED) && !QUERY_FLAG(tmp, FLAG_KNOWN_MAGICAL)
            && (is_magical(tmp)) && tmp->item_power < skill->level) {
        SET_FLAG(tmp, FLAG_KNOWN_MAGICAL);
        esrv_update_item(UPD_FLAGS, pl, tmp);
        return calc_skill_exp(pl, tmp, skill);
    }
    return 0;
}

/**
 * Check for magic object with the 'detect magic' skill.
 *
 * @param pl
 * player detecting.
 * @param skill
 * detect skill object.
 * @return
 * amount of experience gained (on successful detecting).
 */
static int do_skill_detect_magic(object *pl, object *skill) {
    int success = 0;

    FOR_INV_PREPARE(pl, tmp)
        if (!tmp->invisible)
            success += detect_magic_on_item(pl, tmp, skill);
    FOR_INV_FINISH();

    /* Check ground, too, but like above, only if the object can be picked up*/
    FOR_MAP_PREPARE(pl->map, pl->x, pl->y, tmp)
        if (object_can_pick(pl, tmp))
            success += detect_magic_on_item(pl, tmp, skill);
    FOR_MAP_FINISH();

    return success;
}

/**
 * Helper function for do_skill_ident, so that we can loop
 * over inventory AND objects on the ground conveniently.
 *
 * @param tmp
 * object to try to identify.
 * @param pl
 * object identifying.
 * @param skill
 * identification skill.
 * @param print_on_success
 * 1 to print a description if the object is identified, 0 to leave it to the calling function
 * @return
 * experience for successful identification.
 */
int identify_object_with_skill(object *tmp, object *pl, object *skill, int print_on_success) {
    int success = 0, chance, ip;
    int skill_value = skill->level*pl->stats.Int ? pl->stats.Int : 10;

    if (!QUERY_FLAG(tmp, FLAG_IDENTIFIED)
    && !QUERY_FLAG(tmp, FLAG_NO_SKILL_IDENT)
    && need_identify(tmp)
    && !tmp->invisible) {
        ip = tmp->magic;
        if (tmp->item_power > ip)
            ip = tmp->item_power;

        chance = die_roll(3, 10, pl, PREFER_LOW)-3+rndm(0, (tmp->magic ? tmp->magic*5 : 1)-1);
        if (skill_value >= chance) {
            tmp = identify(tmp);
            if (pl->type == PLAYER && print_on_success) {
                char desc[MAX_BUF];

                draw_ext_info_format(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_SUCCESS,
                                     "You identify %s.",
                                     ob_describe(tmp, pl, desc, sizeof(desc)));
                if (tmp->msg) {
                    draw_ext_info_format(NDI_UNIQUE, 0, pl, MSG_TYPE_ITEM, MSG_TYPE_ITEM_INFO,
                                         "The item has a story:\n%s",
                                         tmp->msg);
                }
            }
            success += calc_skill_exp(pl, tmp, skill);
        } else
            SET_FLAG(tmp, FLAG_NO_SKILL_IDENT);
    }
    return success;
}

/**
 * Workhorse for skill_ident() -b.t.
 *
 * @param pl
 * player identifying.
 * @param obj_class
 * type of objects to identify.
 * @param skill
 * skill to give experience to.
 * @return
 * experience gained by identifying items.
 */
static int do_skill_ident(object *pl, int obj_class, object *skill) {
    int success = 0, area, i;

    /* check the player */
    FOR_INV_PREPARE(pl, tmp)
        if (tmp->type == obj_class)
            success += identify_object_with_skill(tmp, pl, skill, 1);
    FOR_INV_FINISH();

    /*  check the ground */
    /* Altered to allow ident skills to increase in area with
     * experience. -- Aaron Baugher
     */

    if (skill->level > 64) {   /* Adjust these levels? */
        area = 49;
    } else if (skill->level > 16) {
        area = 25;
    } else if (skill->level > 4) {
        area = 9;
    } else {
        area = 1;
    }

    for (i = 0; i < area; i++) {
        sint16 x = pl->x+freearr_x[i];
        sint16 y = pl->y+freearr_y[i];
        mapstruct *m = pl->map;
        int mflags;

        mflags = get_map_flags(m, &m, x, y, &x, &y);
        if (mflags&P_OUT_OF_MAP)
            continue;

        if (can_see_monsterP(m, pl->x, pl->y, i)) {
            FOR_MAP_PREPARE(m, x, y, tmp)
                if (tmp->type == obj_class)
                    success += identify_object_with_skill(tmp, pl, skill, 1);
            FOR_MAP_FINISH();
        }
    }
    return success;
}

/**
 * Main identification skill handling.
 * @param pl
 * player identifying.
 * @param skill
 * identification skill.
 * @return
 * experience gained for identification.
 */
int skill_ident(object *pl, object *skill) {
    int success = 0;
    int i, identifiable_types=0;
    const typedata *tmptype;

    if (pl->type != PLAYER)
        return 0; /* only players will skill-identify */

    draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_SUCCESS,
                  "You look at the objects nearby...");

    switch (skill->subtype) {
    case SK_DET_CURSE:
        success = do_skill_detect_curse(pl, skill);
        if (success)
            draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_SUCCESS,
                          "...and discover cursed items!");
            break;

    case SK_DET_MAGIC:
        success = do_skill_detect_magic(pl, skill);
        if (success)
            draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_SUCCESS,
                          "...and discover items imbued with mystic forces!");
        break;

    default:
        /* we will try to identify items with this skill instead */
        for (i=0; i<=OBJECT_TYPE_MAX; i++) {
            tmptype = get_typedata(i);
            if (tmptype) {
                if (skill->subtype == tmptype->identifyskill || skill->subtype == tmptype->identifyskill2) {
                    success += do_skill_ident(pl, i, skill);
                    identifiable_types++;
                }
            }
        }
        if (identifiable_types == 0) {
            LOG(llevError, "Error: skill_ident() called with skill %d which can't identify any items\n", skill->subtype);
            return 0;
            break;
        }
        break;
    }
    if (!success) {
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_SUCCESS,
                      "...and learn nothing more.");
    }
    return success;
}


/**
 * Oratory skill handling.
 * Players using this skill can 'charm' a monster --
 * into working for them. It can only be used on
 * non-special (see below) 'neutral' creatures.
 * -b.t. (thomas@astro.psu.edu)
 *
 * @param pl
 * player trying to convince a monster.
 * @param dir
 * direction to orate in.
 * @param skill
 * oratory skill object.
 * @return
 * experience gained for oratoring.
 * @todo
 * check if can't be simplified, code looks duplicated.
 */
int use_oratory(object *pl, int dir, object *skill) {
    sint16 x = pl->x+freearr_x[dir], y = pl->y+freearr_y[dir];
    int mflags, chance;
    object *tmp;
    mapstruct *m;
    char name[MAX_BUF];

    if (pl->type != PLAYER)
        return 0; /* only players use this skill */
    m = pl->map;
    mflags = get_map_flags(m, &m, x, y, &x, &y);
    if (mflags&P_OUT_OF_MAP)
        return 0;

    /* Save some processing - we have the flag already anyways
     */
    if (!(mflags&P_IS_ALIVE)) {
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                      "There is nothing to orate to.");
        return 0;
    }

    tmp = NULL;
    FOR_MAP_PREPARE(m, x, y, tmp2) {
        tmp2 = HEAD(tmp2);
        /* can't persuade players - return because there is nothing else
         * on that space to charm.
         */
        if (tmp2->type == PLAYER)
            return 0;

        if (QUERY_FLAG(tmp2, FLAG_MONSTER)) {
            const char *value = object_get_value(tmp2, "no_mood_change");
            if (value && strcmp(value, "1") == 0)
                return 0;

            tmp = tmp2;
            break;
        }
    } FOR_MAP_FINISH();

    if (!tmp) {
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                      "There is nothing to orate to.");
        return 0;
    }

    query_name(tmp, name, MAX_BUF);
    draw_ext_info_format(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_SUCCESS,
                         "You orate to the %s.",
                         name);

    /* the following conditions limit who may be 'charmed' */

    /* it's hostile! */
    if (!QUERY_FLAG(tmp, FLAG_UNAGGRESSIVE) && !QUERY_FLAG(tmp, FLAG_FRIENDLY)) {
        query_name(tmp, name, MAX_BUF);
        draw_ext_info_format(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                             "Too bad the %s isn't listening!",
                             name);
        return 0;
    }

    /* it's already allied! */
    if (QUERY_FLAG(tmp, FLAG_FRIENDLY) && tmp->attack_movement == PETMOVE) {
        if (object_get_owner(tmp) == pl) {
            draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_SUCCESS,
                          "Your follower loves your speech.");
            return 0;
        }

        if (skill->level > tmp->level) {
            /* you steal the follower.  Perhaps we should really look at the
             * level of the owner above?
             */
            object_set_owner(tmp, pl);
            query_name(tmp, name, MAX_BUF);
            draw_ext_info_format(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_SUCCESS,
                                 "You convince the %s to follow you instead!",
                                 name);

            FREE_AND_COPY(tmp->skill, skill->skill);

            /* Abuse fix - don't give exp since this can otherwise
             * be used by a couple players to gets lots of exp.
             */
            return 0;
        }

        /* In this case, you can't steal it from the other player */
        return 0;
    } /* Creature was already a pet of someone */

    chance = skill->level*2+(pl->stats.Cha-2*tmp->stats.Int)/2;

    /* Ok, got a 'sucker' lets try to make them a follower */
    if (chance > 0 && tmp->level < random_roll(0, chance-1, pl, PREFER_HIGH)-1) {
        sint64 exp;
        query_name(tmp, name, MAX_BUF);
        draw_ext_info_format(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_SUCCESS,
                             "You convince the %s to become your follower.",
                             name);

        object_set_owner(tmp, pl);
        /* compute exp before setting to 0, else the monster's experience is not taken into account. */
        tmp->stats.exp /= 5; /* why 5? because. */
        exp = calc_skill_exp(pl, tmp, skill);
        tmp->stats.exp = 0;
        add_friendly_object(tmp);
        SET_FLAG(tmp, FLAG_FRIENDLY);
        tmp->attack_movement = PETMOVE;
        /* keep oratory skill, so exp goes where it should if the pet kills something */
        FREE_AND_COPY(tmp->skill, skill->skill);
        return exp;
    }

    /* Charm failed.  Creature may be angry now */
    if (skill->level+(pl->stats.Cha-10)/2 < random_roll(1, 2*tmp->level, pl, PREFER_LOW)) {
        query_name(tmp, name, MAX_BUF);
        draw_ext_info_format(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                             "Your speech angers the %s!",
                             name);

        if (QUERY_FLAG(tmp, FLAG_FRIENDLY)) {
            CLEAR_FLAG(tmp, FLAG_FRIENDLY);
            remove_friendly_object(tmp);
            tmp->attack_movement = 0;  /* needed? */
        }
        CLEAR_FLAG(tmp, FLAG_UNAGGRESSIVE);
    }

    return 0; /* Fall through - if we get here, we didn't charm anything */
}

/**
 * Singing skill handling.
 * This skill allows the player to pacify nearby creatures.
 * There are few limitations on who/what kind of
 * non-player creatures that may be pacified. Right now, a player
 * may pacify creatures which have Int == 0. In this routine, once
 * successfully pacified the creature gets Int=1. Thus, a player
 * may only pacify a creature once.
 * BTW, I appologize for the naming of the skill, I couldnt think
 * of anything better! -b.t.
 *
 * @param pl
 * player singing.
 * @param dir
 * direction to sing in.
 * @param skill
 * singing skill object.
 * @return
 * experience gained for singing.
 */
int singing(object *pl, int dir, object *skill) {
    int i, exp = 0, chance, mflags;
    object *tmp;
    mapstruct *m;
    sint16  x, y;
    char name[MAX_BUF];
    const char *value;

    if (pl->type != PLAYER)
        return 0;   /* only players use this skill */

    draw_ext_info_format(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_SUCCESS,
                         "You sing");
    for (i = dir; i < (dir+MIN(skill->level, SIZEOFFREE)); i++) {
        x = pl->x+freearr_x[i];
        y = pl->y+freearr_y[i];
        m = pl->map;

        mflags = get_map_flags(m, &m, x, y, &x, &y);
        if (mflags&P_OUT_OF_MAP)
            continue;
        if (!(mflags&P_IS_ALIVE))
            continue;

        tmp = NULL;
        FOR_MAP_PREPARE(m, x, y, tmp2) {
            tmp2 = HEAD(tmp2);
            if (QUERY_FLAG(tmp2, FLAG_MONSTER)) {
                tmp = tmp2;
                break;
            }
            /* can't affect players */
            if (tmp2->type == PLAYER) {
                tmp = tmp2;
                break;
            }
        } FOR_MAP_FINISH();

        /* Whole bunch of checks to see if this is a type of monster that would
         * listen to singing.
         */
        if (tmp
        && QUERY_FLAG(tmp, FLAG_MONSTER)
        && !QUERY_FLAG(tmp, FLAG_NO_STEAL)      /* Been charmed or abused before */
        && !QUERY_FLAG(tmp, FLAG_SPLITTING)     /* no ears */
        && !QUERY_FLAG(tmp, FLAG_HITBACK)       /* was here before */
        && (tmp->level <= skill->level)
        && !QUERY_FLAG(tmp, FLAG_UNDEAD)
        && !QUERY_FLAG(tmp, FLAG_UNAGGRESSIVE)   /* already calm */
        && !QUERY_FLAG(tmp, FLAG_FRIENDLY)) {    /* already calm */
            /* stealing isn't really related (although, maybe it should
             * be).  This is mainly to prevent singing to the same monster
             * over and over again and getting exp for it.
             */
            chance = skill->level*2+(pl->stats.Cha-5-tmp->stats.Int)/2;

            value = object_get_value(tmp, "no_mood_change");
            if (value && strcmp(value, "1") == 0)
                chance = 0;

            if (chance && tmp->level*2 < random_roll(0, chance-1, pl, PREFER_HIGH)) {
                SET_FLAG(tmp, FLAG_UNAGGRESSIVE);
                query_name(tmp, name, MAX_BUF);
                draw_ext_info_format(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_SUCCESS,
                                     "You calm down the %s",
                                     name);

                /* Give exp only if they are not aware */
                if (!QUERY_FLAG(tmp, FLAG_NO_STEAL))
                    exp += calc_skill_exp(pl, tmp, skill);
                SET_FLAG(tmp, FLAG_NO_STEAL);
            } else {
                query_name(tmp, name, MAX_BUF);
                draw_ext_info_format(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                                     "Too bad the %s isn't listening!",
                                     name);
                SET_FLAG(tmp, FLAG_NO_STEAL);
            }
        }
    }
    return exp;
}

/**
 * Checks for traps on the spaces around the player or in certain objects.
 *
 * @param pl
 * player searching.
 * @param skill
 * find trap skill object.
 * @return
 * experience gained for finding traps.
 */
int find_traps(object *pl, object *skill) {
    int i, expsum = 0, mflags;
    sint16 x, y;
    mapstruct *m;

    /* First we search all around us for runes and traps, which are
     * all type RUNE
     */

    for (i = 0; i < 9; i++) {
        x = pl->x+freearr_x[i];
        y = pl->y+freearr_y[i];
        m = pl->map;

        mflags = get_map_flags(m, &m, x, y, &x, &y);
        if (mflags&P_OUT_OF_MAP)
            continue;

        /*  Check everything in the square for trapness */
        FOR_MAP_PREPARE(m, x, y, tmp) {
            /* And now we'd better do an inventory traversal of each
             * of these objects' inventory
             * We can narrow this down a bit - no reason to search through
             * the players inventory or monsters for that matter.
             */
            if (tmp->type != PLAYER && !QUERY_FLAG(tmp, FLAG_MONSTER)) {
                FOR_INV_PREPARE(tmp, tmp2)
                    if (tmp2->type == RUNE || tmp2->type == TRAP)
                        if (trap_see(pl, tmp2)) {
                            trap_show(tmp2, tmp);
                            if (tmp2->stats.Cha > 1) {
                                object *owner;

                                owner = object_get_owner(tmp2);
                                if (owner == NULL || owner->type != PLAYER)
                                    expsum += calc_skill_exp(pl, tmp2, skill);

                                tmp2->stats.Cha = 1; /* unhide the trap */
                            }
                        }
                FOR_INV_FINISH();
            }
            if ((tmp->type == RUNE || tmp->type == TRAP) && trap_see(pl, tmp)) {
                trap_show(tmp, tmp);
                if (tmp->stats.Cha > 1) {
                    object *owner;

                    owner = object_get_owner(tmp);
                    if (owner == NULL || owner->type != PLAYER)
                        expsum += calc_skill_exp(pl, tmp, skill);
                    tmp->stats.Cha = 1; /* unhide the trap */
                }
            }
        } FOR_MAP_FINISH();
    }
    draw_ext_info(NDI_BLACK, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_SUCCESS,
                  "You search the area.");
    return expsum;
}

/**
 * This skill will disarm any previously discovered trap.
 * the algorithm is based (almost totally) on the old command_disarm() - b.t.
 *
 * @param op
 * player disarming. Must be on a map.
 * @param skill
 * disarming skill.
 * @return
 * experience gained to disarm.
 */
int remove_trap(object *op, object *skill) {
    int i, success = 0, mflags;
    mapstruct *m;
    sint16 x, y;

    for (i = 0; i < 9; i++) {
        x = op->x+freearr_x[i];
        y = op->y+freearr_y[i];
        m = op->map;

        mflags = get_map_flags(m, &m, x, y, &x, &y);
        if (mflags&P_OUT_OF_MAP)
            continue;

        /* Check everything in the square for trapness */
        FOR_MAP_PREPARE(m, x, y, tmp) {
            /* And now we'd better do an inventory traversal of each
             * of these objects inventory.  Like above, only
             * do this for interesting objects.
             */

            if (tmp->type != PLAYER && !QUERY_FLAG(tmp, FLAG_MONSTER)) {
                FOR_INV_PREPARE(tmp, tmp2)
                    if ((tmp2->type == RUNE || tmp2->type == TRAP) && tmp2->stats.Cha <= 1) {
                        object *owner;

                        trap_show(tmp2, tmp);
                        if (trap_disarm(op, tmp2, 1, skill) && ((owner = object_get_owner(tmp2)) == NULL || owner->type != PLAYER)) {
                            tmp2->stats.exp = tmp2->stats.Cha*tmp2->level;
                            success += calc_skill_exp(op, tmp2, skill);
                        } else {
                            /* Can't continue to disarm after failure */
                            return success;
                        }
                    }
                FOR_INV_FINISH();
            }
            if ((tmp->type == RUNE || tmp->type == TRAP) && tmp->stats.Cha <= 1) {
                object *owner;

                trap_show(tmp, tmp);
                if (trap_disarm(op, tmp, 1, skill) && ((owner = object_get_owner(tmp)) == NULL || owner->type != PLAYER)) {
                    tmp->stats.exp = tmp->stats.Cha*tmp->level;
                    success += calc_skill_exp(op, tmp, skill);
                } else {
                    /* Can't continue to disarm after failure */
                    return success;
                }
            }
        } FOR_MAP_FINISH();
    }
    return success;
}


/**
 * Praying skill handling.
 *
 * When this skill is called from do_skill(), it allows
 * the player to regain lost grace points at a faster rate. -b.t.
 *
 * This always returns 0 - return value is used by calling function
 * such that if it returns true, player gets exp in that skill.  This
 * the effect here can be done on demand, we probably don't want to
 * give infinite exp by returning true in any cases.
 *
 * @param pl
 * object praying, should be a player.
 * @param skill
 * praying skill.
 * @return
 * 0.
 */
int pray(object *pl, object *skill) {
    char buf[MAX_BUF];

    if (pl->type != PLAYER)
        return 0;

    snprintf(buf, sizeof(buf), "You pray.");

    /* Check all objects - we could stop at floor objects,
     * but if someone buries an altar, I don't see a problem with
     * going through all the objects, and it shouldn't be much slower
     * than extra checks on object attributes.
     */
    FOR_BELOW_PREPARE(pl, tmp)
        /* Only if the altar actually belongs to someone do you get special benefits */
        if (tmp->type == HOLY_ALTAR && tmp->other_arch) {
            snprintf(buf, sizeof(buf), "You pray over the %s.", tmp->name);
            pray_at_altar(pl, tmp, skill);
            break;  /* Only pray at one altar */
        }
    FOR_BELOW_FINISH();

    draw_ext_info(NDI_BLACK, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_SUCCESS,
                  buf);

    if (pl->stats.grace < pl->stats.maxgrace) {
        pl->stats.grace++;
        pl->last_grace = -1;
    }
    return 0;
}

/**
 * Meditation skill handling.
 *
 * This skill allows the player to regain a few sp or hp for a
 * brief period of concentration.
 * The amount of time needed to concentrate and the # of points regained is dependant on
 * the level of the user.
 *
 * Depending on the level, the player can wear armour or not.
 * @author b.t. thomas@astro.psu.edu
 *
 * @param pl
 * livng meditating, should be a player.
 * @param skill
 * meditation skill.
 */
void meditate(object *pl, object *skill) {
    if (pl->type != PLAYER)
        return; /* players only */

    /* check if pl has removed encumbering armour and weapons */
    if (QUERY_FLAG(pl, FLAG_READY_WEAPON) && skill->level < 6) {
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                      "You can't concentrate while wielding a weapon!");
        return;
    }

    FOR_INV_PREPARE(pl, tmp)
        if (((tmp->type == ARMOUR && skill->level < 12)
            || (tmp->type == HELMET && skill->level < 10)
            || (tmp->type == SHIELD && skill->level < 6)
            || (tmp->type == BOOTS && skill->level < 4)
            || (tmp->type == GLOVES && skill->level < 2))
        && QUERY_FLAG(tmp, FLAG_APPLIED)) {
            draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                          "You can't concentrate while wearing so much armour!");
            return;
        }
    FOR_INV_FINISH();

    /* ok let's meditate!  Spell points are regained first, then once
     * they are maxed we get back hp. Actual incrementing of values
     * is handled by the do_some_living() (in player.c). This way magical
     * bonuses for healing/sp regeneration are included properly
     * No matter what, we will eat up some playing time trying to
     * meditate. (see 'factor' variable for what sets the amount of time)
     */

    draw_ext_info(NDI_BLACK, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_SUCCESS,
                  "You meditate.");

    if (pl->stats.sp < pl->stats.maxsp) {
        pl->stats.sp++;
        pl->last_sp = -1;
    } else if (pl->stats.hp < pl->stats.maxhp)  {
        pl->stats.hp++;
        pl->last_heal = -1;
    }
}

/**
 * Called by write_on_item() to inscribe a message in an ordinary book.
 *
 * @param pl
 * Player attempting to write
 * @param item
 * Book to write the message in
 * @param msg
 * Message to write
 * @return
 * Experience gained from using the skill
 * @todo assert() instead of simple check.
 */
static int write_note(object *pl, object *item, const char *msg) {
    char buf[BOOK_BUF];
    object *newBook = NULL;

    // The item should exist and be a book, but check it just in case.
    if (!item || item->type != BOOK) {
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                "That was interesting...");
        // TODO: Print a scary log message.
        return 0;
    }

    // The message should never be NULL, but check it just in case.
    if (!msg) {
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                "Hmm... what was I going to write?");
        // TODO: Print a scary log message.
        return 0;
    }

    // Don't let the player write a reserved keyword.
    if (strcasestr_local(msg, "endmsg")) {
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                "Trying to cheat now are we?");
        return 0;
    }

    /* Lauwenmark: Handle for plugin book writing (trigger) event */
    if (execute_event(item, EVENT_TRIGGER, pl, NULL, msg, SCRIPT_FIX_ALL) != 0)
        return strlen(msg);

    buf[0] = 0;

    // Write the message in the book if it doesn't overflow the buffer.
    if (!book_overflow(item->msg, msg, BOOK_BUF)) {
        // TODO: Garble some characters depending on intelligence/skill.

        // If there was already text, append the new message on the end.
        if (item->msg) {
            snprintf(buf, sizeof(buf), "%s%s\n", item->msg, msg);
        } else {
            snprintf(buf, sizeof(buf), "%s\n", msg);
        }

        // If there were multiple items in a stack, unstack one and write.
        if (item->nrof > 1) {
            newBook = object_new();
            object_copy(item, newBook);
            object_decrease_nrof_by_one(item);
            newBook->nrof = 1;
            object_set_msg(newBook, buf);
            newBook = object_insert_in_ob(newBook, pl);
        } else {
            object_set_msg(item, buf);

            // This shouldn't be necessary; the object hasn't changed visibly.
            //esrv_send_item(pl, item);
        }

        // Tell the player that he/she wrote in the object.
        query_short_name(item, buf, BOOK_BUF);
        draw_ext_info_format(
                NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_SUCCESS,
                "You write in the %s.", buf);

        // Give the player experience for writing.
        return strlen(msg);
    } else {
        query_short_name(item, buf, BOOK_BUF);
        draw_ext_info_format(
                NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                "Your message won't fit in the %s!", buf);
        return 0;
    }
}

/**
 * Called by write_on_item() to inscribe spell scrolls that the player can
 * cast. Backfire effects are possible, varying in severity depending on the
 * difficulty of the spell being attempted.
 *
 * @param pl
 * player writing a scroll.
 * @param scroll
 * object to write into.
 * @param skill
 * writing skill.
 * @return
 * experience gained.
 */
static int write_scroll(object *pl, object *scroll, object *skill) {
    int success = 0, confused = 0, grace_cost = 0;
    object *newscroll, *chosen_spell, *tmp;

    // The item should exist and be a scroll, but check it just in case.
    if (!scroll || scroll->type != SCROLL) {
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                "A spell must be written on a magic scroll!");
        // TODO: Print a scary log message.
        return 0;
    }

    // The player must have a spell readied to inscribe.
    chosen_spell = pl->contr->ranges[range_magic];
    if (!chosen_spell) {
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                "You should ready the spell you wish to inscribe.");
        return 0;
    }

    // Make sure the player has enough SP or grace to write the spell.
    grace_cost = SP_level_spellpoint_cost(pl, chosen_spell, SPELL_GRACE);
    if (grace_cost > 0 && grace_cost > pl->stats.grace) {
        draw_ext_info_format(
                NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                "You don't have enough grace to write a scroll of %s.",
                chosen_spell->name);
        return 0;
    }

    if (SP_level_spellpoint_cost(pl, chosen_spell, SPELL_MANA) > pl->stats.sp) {
        draw_ext_info_format(
                NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                "You don't have enough mana to write a scroll of %s.",
                chosen_spell->name);
        return 0;
    }

    // Prevent players from writing spells that they are denied.
    if (chosen_spell->path_attuned & pl->path_denied
            && settings.allow_denied_spells_writing == 0) {
        char name[MAX_BUF];

        query_name(chosen_spell, name, MAX_BUF);
        draw_ext_info_format(
                NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                "Just the idea of writing a scroll of %s makes you sick!",
                name);
        return 0;
    }

    // If the scroll already had a spell written on it, the player could
    // accidentally read it while trying to write a new one. Give the player
    // a 50% chance to overwrite a spell at their own level.
    if ((scroll->stats.sp || scroll->inv) &&
            random_roll(0, scroll->level*2, pl, PREFER_LOW) > skill->level) {
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                "Oops! You accidently read it while trying to write on it.");
        apply_manual(pl, scroll, 0);
        return 0;
    }

    if (execute_event(scroll, EVENT_TRIGGER, pl, chosen_spell, NULL, 0) != 0) {
        return 0;
    }

    // Find out if the player is confused or not.
    if (QUERY_FLAG(pl, FLAG_CONFUSED)) {
        confused = 1;
    }

    // Mana or grace is lost no matter if the inscription is successful.
    pl->stats.grace -= SP_level_spellpoint_cost(pl, chosen_spell, SPELL_GRACE);
    pl->stats.sp -= SP_level_spellpoint_cost(pl, chosen_spell, SPELL_MANA);

    if (random_roll(0, chosen_spell->level * 4 - 1, pl, PREFER_LOW) <
            skill->level) {
        // If there were multiple items in a stack, unstack one.
        if (scroll->nrof > 1) {
            newscroll = object_new();
            object_copy(scroll, newscroll);
            object_decrease_nrof_by_one(scroll);
            newscroll->nrof = 1;
        } else {
            newscroll = scroll;
        }

        // Write spell if not confused; otherwise write random spell.
        newscroll->level = MAX(skill->level, chosen_spell->level);

        if (!confused) {
            draw_ext_info(
                    NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_SUCCESS,
                    "You succeed in writing a new scroll.");
        } else {
            chosen_spell = find_random_spell_in_ob(pl, NULL);
            if (!chosen_spell) {
                return 0;
            }

            draw_ext_info(
                    NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                    "In your confused state, you write down some odd spell.");
        }

        if (newscroll->inv) {
            object *ninv;
            ninv = newscroll->inv;
            object_remove(ninv);
            object_free2(ninv, FREE_OBJ_NO_DESTROY_CALLBACK);
        }

        tmp = object_new();
        object_copy(chosen_spell, tmp);
        object_insert_in_ob(tmp, newscroll);

        // This is needed so casting from the scroll works correctly with
        // moving_ball types, which checks attunements.
        newscroll->path_attuned = tmp->path_repelled;

        // Same code as from treasure.c - so they can better merge.
        // If players want to sell them, so be it.
        newscroll->value = newscroll->arch->clone.value *
            newscroll->inv->value * (newscroll->level + 50 ) /
            (newscroll->inv->level + 50);
        newscroll->stats.exp = newscroll->value / 5;

        // Finish manipulating the scroll before inserting it.
        if (newscroll == scroll) {
            // Remove object to stack correctly with other items.
            object_remove(newscroll);
        }

        newscroll = object_insert_in_ob(newscroll, pl);
        success = calc_skill_exp(pl, newscroll, skill);

        if (!confused) {
            success *= 2;
        }

        success = success * skill->level;
        return success;
    }

    // Inscription wasn't successful; do something bad to the player.
    if (chosen_spell->level > skill->level || confused) {
        draw_ext_info(
                NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                "Ouch! Your attempt to write a new scroll strains your mind!");

        // Either drain a stat or subtract experience.
        if (random_roll(0, 1, pl, PREFER_LOW) == 1) {
            drain_specific_stat(pl, 4);
        } else {
            confuse_living(pl, pl, 99);
            return -30 * chosen_spell->level;
        }
    } else if (random_roll(0, pl->stats.Int-1, pl, PREFER_HIGH) < 15) {
        draw_ext_info(
                NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                "Your attempt to write a new scroll rattles your mind!");
        confuse_living(pl, pl, 99);
    } else {
        draw_ext_info(
                NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                "You fail to write a new scroll.");
    }

    return 0;
}

/**
 * Implement the 'inscription' skill, which checks for the required skills and
 * marked items before running either write_note() or write_scroll().
 *
 * @param pl
 * Player attempting to write
 * @param params
 * Message to write, blank to write a spell
 * @param skill
 * Writing skill
 * @return
 * Experience gained from using the skill
 */
int write_on_item(object *pl, const char *params, object *skill) {
    object *item;
    const char *string = params;
    int msgtype;
    archetype *skat;

    // Only players can use the inscription skill.
    if (pl->type != PLAYER) {
        return 0;
    }

    // No message was given, so both strings are set to empty.
    if (!params) {
        params = "";
        string = params;
    }

    // You must be able to read before you can write.
    skat = get_archetype_by_type_subtype(SKILL, SK_LITERACY);

    if (!find_skill_by_name(pl, skat->clone.skill)) {
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_MISSING,
                "You must learn to read before you can write!");
        return 0;
    }

    // You must not be blind to write, unless you're a DM.
    if (QUERY_FLAG(pl, FLAG_BLIND) && !QUERY_FLAG(pl, FLAG_WIZ)) {
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                "You are unable to write while blind.");
        return 0;
    }

    // If a message was given, write a book. Otherwise, write a scroll.
    if (string[0] != '\0') {
        msgtype = BOOK;
    } else {
        msgtype = SCROLL;
    }

    // Find and attempt to write on the player's marked item.
    item = find_marked_object(pl);
    if (item == NULL) {
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                "You haven't marked any items to write on yet.");
        return 0;
    }

    // Don't let the player write on an unpaid item.
    if (QUERY_FLAG(item, FLAG_UNPAID)) {
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                "You had better pay for that before you write on it!");
        return 0;
    }

    // Check if the marked item is the type of item we're writing.
    if (msgtype != item->type) {
        draw_ext_info_format(
                NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                "You must mark a %s to write %s.",
                msgtype == BOOK ? "book" : "magic scroll",
                msgtype == BOOK ? "your message on" : "your spell down");
        return 0;
    }

    if (msgtype == BOOK) {
        return write_note(pl, item, string);
    } else if (msgtype == SCROLL) {
        return write_scroll(pl, item, skill);
    } else {
        // This case should never be reached.
        return 0;
    }
}

/**
 * Find an object to throw.
 * If there is a suitable marked item, use it.
 * Else find the first item that is of the correct race.
 *
 * @param op
 * object wishing to throw.
 * @param race
 * 'throwing' race.
 * @return
 * throwable object, NULL if none suitable found.
 */
static object *find_throw_ob(object *op, sstring race) {
    object *tmp;
    char name[MAX_BUF];

    if (!op) { /* safety */
        LOG(llevError, "find_throw_ob(): confused! have a NULL thrower!\n");
        return (object *)NULL;
    }

    /* prefer marked item */
    tmp = find_marked_object(op);
    if (tmp != NULL) {
        /* can't toss invisible or inv-locked items */
        if (tmp->invisible || QUERY_FLAG(tmp, FLAG_INV_LOCKED)) {
            tmp = NULL;
        }
    }

    /* look through the inventory if no marked found */
    if (tmp == NULL) {
        FOR_INV_PREPARE(op, tmp2) {
            /* can't toss invisible items */
            if (tmp2->invisible)
                continue;

            if (tmp2->type == CONTAINER && QUERY_FLAG(tmp2, FLAG_APPLIED) && tmp2->race == race) {
                tmp = find_throw_ob(tmp2, race);
                if (tmp != NULL) {
                    break;
                }
            }

            /* if not a container, then don't look if locked */
            if (tmp2->type == CONTAINER || QUERY_FLAG(tmp2, FLAG_INV_LOCKED))
                continue;

            if (tmp2->race == race) {
                tmp = tmp2;
                break;
            }

        } FOR_INV_FINISH();
    }

    /* this should prevent us from throwing away
     * cursed items, worn armour, etc. Only weapons
     * can be thrown from 'hand'.
     */
    if (!tmp)
        return NULL;

    if (QUERY_FLAG(tmp, FLAG_APPLIED)) {
        if (tmp->type != WEAPON) {
            query_name(tmp, name, MAX_BUF);
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                                 "You can't throw %s.",
                                 name);
            tmp = NULL;
        } else if (QUERY_FLAG(tmp, FLAG_CURSED) || QUERY_FLAG(tmp, FLAG_DAMNED)) {
            query_name(tmp, name, MAX_BUF);
            draw_ext_info_format(NDI_UNIQUE, 0, op,
                                 MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                                 "The %s sticks to your hand!",
                                 name);
            tmp = NULL;
        } else {
            if (apply_special(op, tmp, AP_UNAPPLY|AP_NO_MERGE)) {
                LOG(llevError, "BUG: find_throw_ob(): couldn't unapply\n");
                tmp = NULL;
            }
        }
    } else if (QUERY_FLAG(tmp, FLAG_UNPAID)) {
        query_name(tmp, name, MAX_BUF);
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                             "You should pay for the %s first.",
                             name);
        tmp = NULL;
    }

    if (tmp && QUERY_FLAG(tmp, FLAG_INV_LOCKED)) {
        LOG(llevError, "BUG: find_throw_ob(): object is locked\n");
        tmp = NULL;
    }
    return tmp;
}

/**
 * We construct the 'carrier' object in
 * which we will insert the object that is being thrown.
 * This combination  becomes the 'thrown object'. -b.t.
 *
 * @param orig
 * object to wrap.
 * @return
 * object to throw.
 */

static object *make_throw_ob(object *orig) {
    object *toss_item;

    if (!orig)
        return NULL;

    toss_item = object_new();
    if (QUERY_FLAG(orig, FLAG_APPLIED)) {
        LOG(llevError, "BUG: make_throw_ob(): ob is applied\n");
        /* insufficient workaround, but better than nothing */
        CLEAR_FLAG(orig, FLAG_APPLIED);
    }
    object_copy(orig, toss_item);
    toss_item->type = THROWN_OBJ;
    CLEAR_FLAG(toss_item, FLAG_CHANGING);
    toss_item->stats.dam = 0; /* default damage */
    object_insert_in_ob(orig, toss_item);
    return toss_item;
}


/**
 * Op throws any object toss_item. This code
 * was borrowed from fire_bow.
 *
 * @param op
 * living thing throwing something.
 * @param part
 * part of op throwing.
 * @param toss_item
 * item thrown.
 * @param dir
 * direction to throw.
 * @param skill
 * throwing skill.
 * @retval 0
 * skill use failed.
 * @retval 1
 * skill was successfully used.
 * @todo this messy function should probably be simplified.
 */
static int do_throw(object *op, object *part, object *toss_item, int dir, object *skill) {
    object *throw_ob = toss_item, *left = NULL;
    int eff_str = 0, str = op->stats.Str, dam = 0;
    int pause_f, weight_f = 0, mflags;
    float str_factor = 1.0, load_factor = 1.0, item_factor = 1.0;
    mapstruct *m;
    sint16  sx, sy;
    tag_t tag;
    char name[MAX_BUF];

    if (throw_ob == NULL) {
        if (op->type == PLAYER) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                          "You have nothing to throw.");
        }
        return 0;
    }
    if (QUERY_FLAG(throw_ob, FLAG_STARTEQUIP)) {
        if (op->type == PLAYER) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                          "The gods won't let you throw that.");
        }
        return 0;
    }

    /* Lauwenmark - Now we can call the associated script_throw event (if any) */
    tag = throw_ob->count;
    execute_event(throw_ob, EVENT_THROW, op, NULL, NULL, SCRIPT_FIX_ACTIVATOR);
    if (object_was_destroyed(throw_ob, tag)) {
        return 1;
    }

    /* Because throwing effectiveness must be reduced by the
     * encumbrance of the thrower and weight of the object. THus,
     * we use the concept of 'effective strength' as defined below.
     */

    /* if str exceeds settings.max_stat (30, eg giants), lets assign a str_factor > 1 */
    if (str > settings.max_stat) {
        str_factor = (float)str/(float)settings.max_stat;
        str = settings.max_stat;
    }

    /* the more we carry, the less we can throw. Limit only on players */
    /* This logic is basically grabbed right out of fix_object() */
    if (op->type == PLAYER
    && op->carrying > (get_weight_limit(op->stats.Str)*FREE_PLAYER_LOAD_PERCENT)
    && (FREE_PLAYER_LOAD_PERCENT < 1.0)) {
        int extra_weight = op->carrying-get_weight_limit(op->stats.Str)*FREE_PLAYER_LOAD_PERCENT;
        load_factor = (float)extra_weight/(float)(get_weight_limit(op->stats.Str)*(1.0-FREE_PLAYER_LOAD_PERCENT));
    }

    /* lighter items are thrown harder, farther, faster */
    if (throw_ob->weight > 0)
        item_factor = (float)(get_weight_limit(op->stats.Str)*FREE_PLAYER_LOAD_PERCENT)/(float)(3.0*throw_ob->weight);
    else { /* 0 or negative weight?!? Odd object, can't throw it */
        query_name(throw_ob, name, MAX_BUF);
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                             "You can't throw %s.",
                             name);
        return 0;
    }

    eff_str = str*MIN(load_factor, 1.0);
    eff_str = (float)eff_str*item_factor*str_factor;

    /* alas, arrays limit us to a value of settings.max_stat (30). Use str_factor to
     * account for super-strong throwers. */
    if (eff_str > settings.max_stat)
        eff_str = settings.max_stat;

#ifdef DEBUG_THROW
    LOG(llevDebug, "%s carries %d, eff_str=%d\n", op->name, op->carrying, eff_str);
    LOG(llevDebug, " max_c=%d, item_f=%f, load_f=%f, str=%d\n", (get_weight_limit(op->stats.Str)*FREE_PLAYER_LOAD_PERCENT), item_factor, load_factor, op->stats.Str);
    LOG(llevDebug, " str_factor=%f\n", str_factor);
    LOG(llevDebug, " item %s weight= %d\n", throw_ob->name, throw_ob->weight);
#endif

    /* 3 things here prevent a throw, you aimed at your feet, you
     * have no effective throwing strength, or you threw at something
     * that flying objects can't get through.
     */
    mflags = get_map_flags(part->map, &m, part->x+freearr_x[dir], part->y+freearr_y[dir], &sx, &sy);

    if (!dir
    || (eff_str <= 1)
    || (mflags&P_OUT_OF_MAP)
    || (GET_MAP_MOVE_BLOCK(m, sx, sy)&MOVE_FLY_LOW)) {
        /* bounces off 'wall', and drops to feet */
        object_remove(throw_ob);
        object_insert_in_map_at(throw_ob, part->map, op, 0, part->x, part->y);
        if (op->type == PLAYER) {
            if (eff_str <= 1) {
                query_name(throw_ob, name, MAX_BUF);
                draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                                     "Your load is so heavy you drop %s to the ground.",
                                     name);
            } else if (!dir) {
                query_name(throw_ob, name, MAX_BUF);
                draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                                     "You throw %s at the ground.",
                                     name);
            } else
                draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                              "Something is in the way.");
        }
        return 0;
    } /* if object can't be thrown */

    left = throw_ob; /* these are throwing objects left to the player */

    /* sometimes object_split() can't split an object (because op->nrof==0?)
     * and returns NULL. We must use 'left' then
     */
    throw_ob = object_split(throw_ob, 1, NULL, 0);
    if (throw_ob == NULL) {
        throw_ob = left;
        object_remove(left);
    }

    /* special case: throwing powdery substances like dust, dirt */
    if (throw_ob->type == POTION && throw_ob->subtype == POT_DUST) {
        cast_dust(op, throw_ob, dir);
        return 1;
    }

    /* Make a thrown object -- insert real object in a 'carrier' object.
     * If unsuccessfull at making the "thrown_obj", we just reinsert
     * the original object back into inventory and exit
     */
    toss_item = make_throw_ob(throw_ob);
    if (toss_item) {
        throw_ob = toss_item;
        if (throw_ob->skill)
            free_string(throw_ob->skill);
        throw_ob->skill = add_string(skill->skill);
    } else {
        object_insert_in_ob(throw_ob, op);
        return 0;
    }

    object_set_owner(throw_ob, op);
    /* At some point in the attack code, the actual real object (op->inv)
     * becomes the hitter.  As such, we need to make sure that has a proper
     * owner value so exp goes to the right place.
     */
    object_set_owner(throw_ob->inv, op);
    throw_ob->direction = dir;

    /* the damage bonus from the force of the throw */
    dam = str_factor*get_dam_bonus(eff_str);

    /* Now, lets adjust the properties of the thrown_ob. */

    /* how far to fly */
    throw_ob->last_sp = (eff_str*3)/5;

    /* speed */
    throw_ob->speed = (get_speed_bonus(eff_str)+1.0)/1.5;
    throw_ob->speed = MIN(1.0, throw_ob->speed); /* no faster than an arrow! */

    /* item damage. Eff_str and item weight influence damage done */
    weight_f = MIN(throw_ob->weight/2000, settings.max_stat);
    throw_ob->stats.dam += (dam/3)+get_dam_bonus(weight_f)+(throw_ob->weight/15000)-2;

    /* chance of breaking. Proportional to force used and weight of item */
    throw_ob->stats.food = (dam/2)+(throw_ob->weight/60000);

    /* replace 25 with a call to clone.arch wc? messes up w/ NPC */
    throw_ob->stats.wc = 25-get_dex_bonus(op->stats.Dex)-get_thaco_bonus(eff_str)-skill->level;


    /* the properties of objects which are meant to be thrown (ie dart,
     * throwing knife, etc) will differ from ordinary items. Lets tailor
     * this stuff in here.
     */

    if (QUERY_FLAG(throw_ob->inv, FLAG_IS_THROWN)) {
        throw_ob->last_sp += eff_str/3; /* fly a little further */
        throw_ob->stats.dam += throw_ob->inv->stats.dam+throw_ob->magic+2;
        throw_ob->stats.wc -= throw_ob->magic+throw_ob->inv->stats.wc;
        /* only throw objects get directional faces */
        if (GET_ANIM_ID(throw_ob) && NUM_ANIMATIONS(throw_ob))
            object_update_turn_face(throw_ob);
    } else {
        /* some materials will adjust properties.. */
        if (throw_ob->material&M_LEATHER) {
            throw_ob->stats.dam -= 1;
            throw_ob->stats.food -= 10;
        }
        if (throw_ob->material&M_GLASS)
            throw_ob->stats.food += 60;

        if (throw_ob->material&M_ORGANIC) {
            throw_ob->stats.dam -= 3;
            throw_ob->stats.food += 55;
        }
        if (throw_ob->material&M_PAPER || throw_ob->material&M_CLOTH) {
            throw_ob->stats.dam -= 5;
            throw_ob->speed *= 0.8;
            throw_ob->stats.wc += 3;
            throw_ob->stats.food -= 30;
        }
        /* light obj have more wind resistance, fly slower*/
        if (throw_ob->weight > 500)
            throw_ob->speed *= 0.8;
        if (throw_ob->weight > 50)
            throw_ob->speed *= 0.5;
    } /* else tailor thrown object */

    /* some limits, and safeties (needed?) */
    if (throw_ob->stats.dam < 0)
        throw_ob->stats.dam = 0;
    if (throw_ob->last_sp > eff_str)
        throw_ob->last_sp = eff_str;
    if (throw_ob->stats.food < 0)
        throw_ob->stats.food = 0;
    if (throw_ob->stats.food > 100)
        throw_ob->stats.food = 100;
    if (throw_ob->stats.wc > 30)
        throw_ob->stats.wc = 30;

    /* how long to pause the thrower. Higher values mean less pause */
    pause_f = ((2*eff_str)/3)+20+skill->level;

    /* Put a lower limit on this */
    if (pause_f < 10)
        pause_f = 10;
    if (pause_f > 100)
        pause_f = 100;

    /* Changed in 0.94.2 - the calculation before was really goofy.
     * In short summary, a throw can take anywhere between speed 5 and
     * speed 0.5
     */
    op->speed_left -= 50/pause_f;

    object_update_speed(throw_ob);
    throw_ob->speed_left = 0;

    throw_ob->move_type = MOVE_FLY_LOW;
    throw_ob->move_on = MOVE_FLY_LOW|MOVE_WALK;

#ifdef DEBUG_THROW
    LOG(llevDebug, " pause_f=%d \n", pause_f);
    LOG(llevDebug, " %s stats: wc=%d dam=%d dist=%d spd=%f break=%d\n", throw_ob->name, throw_ob->stats.wc, throw_ob->stats.dam, throw_ob->last_sp, throw_ob->speed, throw_ob->stats.food);
    LOG(llevDebug, "inserting tossitem (%d) into map\n", throw_ob->count);
#endif
    tag = throw_ob->count;
    object_insert_in_map_at(throw_ob, part->map, op, 0, part->x, part->y);
    if (!object_was_destroyed(throw_ob, tag))
        ob_process(throw_ob);
    return 1;
}

/**
 * Throwing skill handling.
 * @param op
 * object throwing.
 * @param part
 * actual part of op throwing.
 * @param dir
 * direction to throw into.
 * @param skill
 * throwing skill.
 * @retval 0
 * skill use failed.
 * @retval 1
 * skill was successfully used.
 */
int skill_throw(object *op, object *part, int dir, object *skill) {
    object *throw_ob;

    if (op->type == PLAYER)
        throw_ob = find_throw_ob(op, find_string("throwing"));
    else
        throw_ob = monster_find_throw_ob(op);

    return do_throw(op, part, throw_ob, dir, skill);
}
