/*
 * Crossfire -- cooperative multi-player graphical RPG and adventure game
 *
 * Copyright (c) 1999-2014 Mark Wedel and the Crossfire Development Team
 * Copyright (c) 1992 Frank Tore Johansen
 *
 * Crossfire is free software and comes with ABSOLUTELY NO WARRANTY. You are
 * welcome to redistribute it under certain conditions. For details, please
 * see COPYING and LICENSE.
 *
 * The authors can be reached via e-mail at <crossfire@metalforge.org>.
 */

/**
 * @file
 * This handles all attacks, magical or not.
 * @todo
 * clean functions. Are all parameters required? Seems quite a mess to send damage/wc etc in attack_ob_simple().
 */

#include <assert.h>
#include <global.h>
#include <living.h>
#include <material.h>
#include <skills.h>

#ifndef __CEXTRACT__
#include <sproto.h>
#endif

#include <sounds.h>

/*#define ATTACK_DEBUG*/

static void slow_living(object *op, object *hitter, int dam);
static void deathstrike_living(object *op, object *hitter, int *dam);
static int adj_attackroll(object *hitter, object *target);
static int is_aimed_missile(object *op);
static int did_make_save_item(object *op, int type, object *originator);
static void poison_living(object *op, object *hitter, int dam);

/**
 * Cancels object *op.  Cancellation basically means an object loses
 * its magical benefits.
 *
 * @param op
 * item to cancel. Its inventory will also be cancelled.
 */
static void cancellation(object *op) {
    if (op->invisible)
        return;

    if (QUERY_FLAG(op, FLAG_ALIVE) || op->type == CONTAINER  || op->type == THROWN_OBJ) {
        /* Recur through the inventory */
        FOR_INV_PREPARE(op, inv)
            if (!did_make_save_item(inv, AT_CANCELLATION, op))
                cancellation(inv);
        FOR_INV_FINISH();
    } else if (FABS(op->magic) <= rndm(0, 5)) {
        /* Nullify this object. This code could probably be more complete */
        /* in what abilities it should cancel */
        op->magic = 0;
        CLEAR_FLAG(op, FLAG_DAMNED);
        CLEAR_FLAG(op, FLAG_CURSED);
        CLEAR_FLAG(op, FLAG_KNOWN_MAGICAL);
        CLEAR_FLAG(op, FLAG_KNOWN_CURSED);
        if (op->env && op->env->type == PLAYER) {
            esrv_update_item(UPD_FLAGS, op->env, op);
        }
    }
}

/**
 * Checks to make sure the item actually
 * made its saving throw based on the tables.  It does not take
 * any further action (like destroying the item).
 * @param op
 * object to check.
 * @param type
 * attack type.
 * @param originator
 * what it attacking?
 * @return
 * TRUE if item saved, FALSE else.
 * @todo
 * check meaning of originator.
 */
static int did_make_save_item(object *op, int type, object *originator) {
    int i, roll, saves = 0, attacks = 0, number;
    materialtype_t *mt;

    if (op->materialname == NULL) {
        for (mt = materialt; mt != NULL && mt->next != NULL; mt = mt->next) {
            if (op->material&mt->material)
                break;
        }
    } else
        mt = name_to_material(op->materialname);
    if (mt == NULL)
        return TRUE;
    roll = rndm(1, 20);

    /* the attacktypes have no meaning for object saves
     * If the type is only magic, don't adjust type - basically, if
     * pure magic is hitting an object, it should save.  However, if it
     * is magic teamed with something else, then strip out the
     * magic type, and instead let the fire, cold, or whatever component
     * destroy the item.  Otherwise, you get the case of poisoncloud
     * destroying objects because it has magic attacktype.
     */
    if (type != AT_MAGIC)
        type &= ~(AT_CONFUSION|AT_DRAIN|AT_GHOSTHIT|AT_POISON|AT_SLOW|
                  AT_PARALYZE|AT_TURN_UNDEAD|AT_FEAR|AT_DEPLETE|AT_DEATH|
                  AT_COUNTERSPELL|AT_HOLYWORD|AT_BLIND|AT_LIFE_STEALING|
                  AT_MAGIC);

    if (type == 0)
        return TRUE;
    if (roll == 20)
        return TRUE;
    if (roll == 1)
        return FALSE;

    for (number = 0; number < NROFATTACKS; number++) {
        i = 1<<number;
        if (!(i&type))
            continue;
        attacks++;
        if (op->resist[number] == 100)
            saves++;
        else if (roll >= mt->save[number]-op->magic-op->resist[number]/100)
            saves++;
        else if ((20-mt->save[number])/3 > originator->stats.dam)
            saves++;
    }

    if (saves == attacks || attacks == 0)
        return TRUE;
    if (saves == 0 || rndm(1, attacks) > saves)
        return FALSE;
    return TRUE;
}

/**
 * Object is attacked with some attacktype (fire, ice, ...).
 * Calls did_make_save_item().  It then performs the
 * appropriate actions to the item (such as burning the item up,
 * calling cancellation(), etc.)
 * @param op
 * victim of the attack.
 * @param type
 * attacktype.
 * @param originator
 * what is attacking.
 */
void save_throw_object(object *op, uint32 type, object *originator) {
    if (!did_make_save_item(op, type, originator)) {
        object *env = op->env;
        int x = op->x, y = op->y;
        mapstruct *m = op->map;
        /* For use with burning off equipped items */
        int weight = op->weight;

        op = stop_item(op);
        if (op == NULL)
            return;

        /*
         * If this object is a transport and has players in it, make them disembark.
         */
        if (op->type == TRANSPORT && op->inv) {
            if (op->map == NULL) {
                LOG(llevError, "Transport %s not on a map but with an item %s in it?\n", op->name, op->inv->name);
            } else {
                char name[MAX_BUF];
                query_name(op, name, sizeof(name));
                FOR_INV_PREPARE(op, inv) {
                    if (inv->contr) {
                        draw_ext_info_format(NDI_UNIQUE, 0, inv, MSG_TYPE_APPLY, MSG_TYPE_APPLY_UNAPPLY,
                            "You are expelled from the %s during its destruction.",
                            name);
                        inv->contr->transport = NULL;
                    }
                }
                FOR_INV_FINISH();
            }
        }


        /* Set off runes in the inventory of the object being destroyed. */
        FOR_INV_PREPARE(op, inv)
            if (inv->type == RUNE)
                spring_trap(inv, originator);
        FOR_INV_FINISH();

        /* Hacked the following so that type LIGHTER will work.
         * Also, objects which are potenital "lights" that are hit by
         * flame/elect attacks will be set to glow. "lights" are any
         * object with +/- glow_radius and an "other_arch" to change to.
         * (and please note that we cant fail our save and reach this
         * function if the object doesnt contain a material that can burn.
         * So forget lighting magical swords on fire with this!) -b.t.
         */
        if (type&(AT_FIRE|AT_ELECTRICITY)
        && op->other_arch
        && QUERY_FLAG(op, FLAG_IS_LIGHTABLE)) {
            const char *arch = op->other_arch->name;

            op = object_decrease_nrof(op, 1);
            if (op)
                fix_stopped_item(op, m, originator);
            op = create_archetype(arch);
            if (op != NULL) {
                if (env) {
                    op->x = env->x,
                    op->y = env->y;
                    object_insert_in_ob(op, env);
                } else
                    object_insert_in_map_at(op, m, originator, 0, x, y);
            }
            return;
        }
        if (type&AT_CANCELLATION) {         /* Cancellation. */
            cancellation(op);
            fix_stopped_item(op, m, originator);
            return;
        }
        if (op->nrof > 1) {
            op = object_decrease_nrof(op, rndm(0, op->nrof-1));
            if (op)
                fix_stopped_item(op, m, originator);
        } else {
            if (!QUERY_FLAG(op, FLAG_REMOVED))
                object_remove(op);
            object_free_drop_inventory(op);
        }
        if (type&(AT_FIRE|AT_ELECTRICITY)) {
            if (env) {
                /* Check to see if the item being burnt is being worn */
                if (QUERY_FLAG(op, FLAG_APPLIED)) {
                    /* if the object is applied, it should be safe to assume env is a player or creature. */
                    if (env->resist[ATNR_FIRE] < 100)
                        /* Should the message type be something different? */
                        draw_ext_info_format(NDI_RED, 0, env, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
                            "OUCH! It burns!");
                    else
                        draw_ext_info_format(NDI_UNIQUE, 0, env, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
                            "Despite the flame, you feel nothing.");
                    /* burning off an item causes 1 point of fire damage for every kilogram of mass the item has */
                    int dam = weight / 1000 * (100 - env->resist[ATNR_FIRE]) / 100;
                    /* Double the damage on cursed items */
                    if (QUERY_FLAG(op, FLAG_CURSED))
                        dam *= 2;
                    /* Triple the damage on damned items. A cursed and damned item would thus inflict 6x damage. */
                    if (QUERY_FLAG(op, FLAG_DAMNED))
                        dam *= 3;
                    env->stats.hp -= dam;
                    /* You die at -1, not 0 */
                    if (env->stats.hp < 0)
                        kill_player(env, NULL);
                }
                op = create_archetype("burnout");
                op->x = env->x,
                op->y = env->y;
                object_insert_in_ob(op, env);
            } else {
                object_replace_insert_in_map("burnout", originator);
            }
        }
        return;
    }
    /* The value of 50 is arbitrary. */
    if (type&AT_COLD
    && (op->resist[ATNR_COLD] < 50)
    && !QUERY_FLAG(op, FLAG_NO_PICK)
    && (RANDOM()&2)) {
        object *tmp;
        archetype *at = find_archetype("icecube");

        if (at == NULL)
            return;
        op = stop_item(op);
        if (op == NULL)
            return;
        tmp = map_find_by_archetype(op->map, op->x, op->y, at);
        if (tmp == NULL) {
            tmp = arch_to_object(at);
            /* This was in the old (pre new movement code) -
             * icecubes have slow_move set to 1 - don't want
             * that for ones we create.
             */
            tmp->move_slow_penalty = 0;
            tmp->move_slow = 0;
            object_insert_in_map_at(tmp, op->map, originator, 0, op->x, op->y);
        }
        if (!QUERY_FLAG(op, FLAG_REMOVED))
            object_remove(op);
        (void)object_insert_in_ob(op, tmp);
        return;
    }
}

/**
 * Attack a spot on the map.
 *
 * @param op
 * object hitting the map.
 * @param dir
 * direction op is hitting/going.
 * @param type
 * attacktype.
 * @param full_hit
 * if set then monster area does not matter, it gets all damage. Else damage is proportional to
 * affected area vs full monster area.
 * @return
 * 1 if it hits something, 0 otherwise.
 */
int hit_map(object *op, int dir, uint32 type, int full_hit) {
    mapstruct *map;
    sint16 x, y;
    int retflag = 0;  /* added this flag..  will return 1 if it hits a monster */
    tag_t op_tag;

    if (QUERY_FLAG(op, FLAG_FREED)) {
        LOG(llevError, "BUG: hit_map(): free object\n");
        return 0;
    }

    if (QUERY_FLAG(op, FLAG_REMOVED) || op->env != NULL) {
        LOG(llevError, "BUG: hit_map(): hitter (arch %s, name %s) not on a map\n", op->arch->name, op->name);
        return 0;
    }

    if (!op->map) {
        LOG(llevError, "BUG: hit_map(): %s has no map\n", op->name);
        return 0;
    }

    op = HEAD(op);
    op_tag = op->count;

    map = op->map;
    x = op->x+freearr_x[dir];
    y = op->y+freearr_y[dir];
    if (get_map_flags(map, &map, x, y, &x, &y)&P_OUT_OF_MAP)
        return 0;

    /* peterm:  a few special cases for special attacktypes --counterspell
     * must be out here because it strikes things which are not alive
     */

    if (type&AT_COUNTERSPELL) {
        counterspell(op, dir);  /* see spell_effect.c */

        /* If the only attacktype is counterspell or magic, don't need
         * to do any further processing.
         */
        if (!(type&~(AT_COUNTERSPELL|AT_MAGIC))) {
            return 0;
        }
        type &= ~AT_COUNTERSPELL;
    }

    if (type&AT_CHAOS) {
        shuffle_attack(op, 1);  /*1 flag tells it to change the face */
        object_update(op, UP_OBJ_FACE);
        type &= ~AT_CHAOS;
    }

    FOR_MAP_PREPARE(map, x, y, tmp) {
        if (QUERY_FLAG(tmp, FLAG_FREED)) {
            LOG(llevError, "BUG: hit_map(): found freed object\n");
            break;
        }

        /* Something could have happened to 'tmp' while 'tmp->below' was processed.
         * For example, 'tmp' was put in an icecube.
         * This is one of the few cases where on_same_map should not be used.
         */
        if (tmp->map != map || tmp->x != x || tmp->y != y)
            continue;

        tmp = HEAD(tmp);

        /* Need to hit everyone in the transport with this spell */
        if (tmp->type == TRANSPORT) {
            FOR_INV_PREPARE(tmp, pl)
                if (pl->type == PLAYER)
                    hit_player(pl, op->stats.dam, op, type, full_hit);
            FOR_INV_FINISH();
        }

        if (QUERY_FLAG(tmp, FLAG_ALIVE)) {
            hit_player(tmp, op->stats.dam, op, type, full_hit);
            retflag |= 1;
            if (object_was_destroyed(op, op_tag))
                break;
        }
        /* Here we are potentially destroying an object.  If the object has
         * NO_PASS set, it is also immune - you can't destroy walls.  Note
         * that weak walls have is_alive set, which prevent objects from
         * passing over/through them.  We don't care what type of movement
         * the wall blocks - if it blocks any type of movement, can't be
         * destroyed right now.
         */
        else if ((tmp->material || tmp->materialname) && op->stats.dam > 0 && !tmp->move_block) {
            save_throw_object(tmp, type, op);
            if (object_was_destroyed(op, op_tag))
                break;
        }
    } FOR_MAP_FINISH();
    return 0;
}

/**
 * Send an attack message to someone.
 *
 * @param dam
 * amount of damage done.
 * @param type
 * attack type.
 * @param op
 * victim of the attack.
 * @param hitter
 * who is hitting.
 * @todo
 * move check for player at function start? this function seems called for everyone.
 * use string safe functions.
 */
static void attack_message(int dam, int type, object *op, object *hitter) {
    char buf[MAX_BUF], buf1[MAX_BUF], buf2[MAX_BUF];
    int i, found = 0;
    mapstruct *map;
    object *owner;

    /* put in a few special messages for some of the common attacktypes
     *  a player might have.  For example, fire, electric, cold, etc
     *  [garbled 20010919]
     */

    if (dam == 9998 && op->type == DOOR) {
        snprintf(buf1, sizeof(buf1), "unlock %s", op->name);
        snprintf(buf2, sizeof(buf2), " unlocks");
        found++;
    }
    if (dam < 0) {
        snprintf(buf1, sizeof(buf1), "hit %s", op->name);
        snprintf(buf2, sizeof(buf2), " hits");
        found++;
    } else if (dam == 0) {
        snprintf(buf1, sizeof(buf1), "missed %s", op->name);
        snprintf(buf2, sizeof(buf2), " misses");
        found++;
    } else if ((hitter->type == DISEASE
        || hitter->type == SYMPTOM
        || hitter->type == POISONING
        || (type&AT_POISON && IS_LIVE(op))) && !found) {
        for (i = 0; i < MAXATTACKMESS && attack_mess[ATM_SUFFER][i].level != -1; i++)
            if (dam < attack_mess[ATM_SUFFER][i].level
            || attack_mess[ATM_SUFFER][i+1].level == -1) {
                snprintf(buf1, sizeof(buf1), "%s %s%s", attack_mess[ATM_SUFFER][i].buf1, op->name, attack_mess[ATM_SUFFER][i].buf2);
                snprintf(buf2, sizeof(buf2), "%s", attack_mess[ATM_SUFFER][i].buf3);
                found++;
                break;
            }
    } else if (op->type == DOOR && !found) {
        for (i = 0; i < MAXATTACKMESS && attack_mess[ATM_DOOR][i].level != -1; i++)
            if (dam < attack_mess[ATM_DOOR][i].level
            || attack_mess[ATM_DOOR][i+1].level == -1) {
                snprintf(buf1, sizeof(buf1), "%s %s%s", attack_mess[ATM_DOOR][i].buf1, op->name, attack_mess[ATM_DOOR][i].buf2);
                snprintf(buf2, sizeof(buf2), "%s", attack_mess[ATM_DOOR][i].buf3);
                found++;
                break;
            }
    } else if (hitter->type == PLAYER && IS_LIVE(op)) {
        if (USING_SKILL(hitter, SK_KARATE)) {
            for (i = 0; i < MAXATTACKMESS && attack_mess[ATM_KARATE][i].level != -1; i++)
                if (dam < attack_mess[ATM_KARATE][i].level
                || attack_mess[ATM_KARATE][i+1].level == -1) {
                    snprintf(buf1, sizeof(buf1), "%s %s%s", attack_mess[ATM_KARATE][i].buf1, op->name, attack_mess[ATM_KARATE][i].buf2);
                    snprintf(buf2, sizeof(buf2), "%s", attack_mess[ATM_KARATE][i].buf3);
                    found++;
                    break;
                }
        } else if (USING_SKILL(hitter, SK_CLAWING)) {
            for (i = 0; i < MAXATTACKMESS && attack_mess[ATM_CLAW][i].level != -1; i++)
                if (dam < attack_mess[ATM_CLAW][i].level
                || attack_mess[ATM_CLAW][i+1].level == -1) {
                    snprintf(buf1, sizeof(buf1), "%s %s%s", attack_mess[ATM_CLAW][i].buf1, op->name, attack_mess[ATM_CLAW][i].buf2);
                    snprintf(buf2, sizeof(buf2), "%s", attack_mess[ATM_CLAW][i].buf3);
                    found++;
                    break;
                }
        } else if (USING_SKILL(hitter, SK_PUNCHING)) {
            for (i = 0; i < MAXATTACKMESS && attack_mess[ATM_PUNCH][i].level != -1; i++)
                if (dam < attack_mess[ATM_PUNCH][i].level
                || attack_mess[ATM_PUNCH][i+1].level == -1) {
                    snprintf(buf1, sizeof(buf1), "%s %s%s", attack_mess[ATM_PUNCH][i].buf1, op->name, attack_mess[ATM_PUNCH][i].buf2);
                    snprintf(buf2, sizeof(buf2), "%s", attack_mess[ATM_PUNCH][i].buf3);
                    found++;
                    break;
                }
        } else if (USING_SKILL(hitter, SK_WRAITH_FEED)) {
            for (i = 0; i < MAXATTACKMESS && attack_mess[ATM_WRAITH_FEED][i].level != -1; i++)
                if (dam < attack_mess[ATM_WRAITH_FEED][i].level) {
                    snprintf(buf1, sizeof(buf1), "%s %s%s", attack_mess[ATM_WRAITH_FEED][i].buf1, op->name, attack_mess[ATM_WRAITH_FEED][i].buf2);
                    snprintf(buf2, sizeof(buf2), "%s", attack_mess[ATM_WRAITH_FEED][i].buf3);
                    found++;
                    break;
                }
        }
    }
    if (found) {
        /* done */
    } else if (IS_ARROW(hitter) && (type == AT_PHYSICAL || type == AT_MAGIC)) {
        snprintf(buf1, sizeof(buf1), "hit"); /* just in case */
        for (i = 0; i < MAXATTACKMESS; i++)
            if (dam < attack_mess[ATM_ARROW][i].level
            || attack_mess[ATM_ARROW][i+1].level == -1) {
                snprintf(buf2, sizeof(buf2), "%s", attack_mess[ATM_ARROW][i].buf3);
                found++;
                break;
            }
    } else if (type&AT_DRAIN && IS_LIVE(op)) {
        /* drain is first, because some items have multiple attypes */
        for (i = 0; i < MAXATTACKMESS && attack_mess[ATM_DRAIN][i].level != -1; i++)
            if (dam < attack_mess[ATM_DRAIN][i].level
            || attack_mess[ATM_DRAIN][i+1].level == -1) {
                snprintf(buf1, sizeof(buf1), "%s %s%s", attack_mess[ATM_DRAIN][i].buf1, op->name, attack_mess[ATM_DRAIN][i].buf2);
                snprintf(buf2, sizeof(buf2), "%s", attack_mess[ATM_DRAIN][i].buf3);
                found++;
                break;
            }
    } else if (type&AT_ELECTRICITY && IS_LIVE(op)) {
        for (i = 0; i < MAXATTACKMESS && attack_mess[ATM_ELEC][i].level != -1; i++)
            if (dam < attack_mess[ATM_ELEC][i].level
            || attack_mess[ATM_ELEC][i+1].level == -1) {
                snprintf(buf1, sizeof(buf1), "%s %s%s", attack_mess[ATM_ELEC][i].buf1, op->name, attack_mess[ATM_ELEC][i].buf2);
                snprintf(buf2, sizeof(buf2), "%s", attack_mess[ATM_ELEC][i].buf3);
                found++;
                break;
            }
    } else if (type&AT_COLD && IS_LIVE(op)) {
        for (i = 0; i < MAXATTACKMESS && attack_mess[ATM_COLD][i].level != -1; i++)
            if (dam < attack_mess[ATM_COLD][i].level
            || attack_mess[ATM_COLD][i+1].level == -1) {
                snprintf(buf1, sizeof(buf1), "%s %s%s", attack_mess[ATM_COLD][i].buf1, op->name, attack_mess[ATM_COLD][i].buf2);
                snprintf(buf2, sizeof(buf2), "%s", attack_mess[ATM_COLD][i].buf3);
                found++;
                break;
            }
    } else if (type&AT_FIRE) {
        for (i = 0; i < MAXATTACKMESS && attack_mess[ATM_FIRE][i].level != -1; i++)
            if (dam < attack_mess[ATM_FIRE][i].level
            || attack_mess[ATM_FIRE][i+1].level == -1) {
                snprintf(buf1, sizeof(buf1), "%s %s%s", attack_mess[ATM_FIRE][i].buf1, op->name, attack_mess[ATM_FIRE][i].buf2);
                snprintf(buf2, sizeof(buf2), "%s", attack_mess[ATM_FIRE][i].buf3);
                found++;
                break;
            }
    } else if (hitter->current_weapon != NULL) {
        int mtype;

        switch (hitter->current_weapon->weapontype) {
        case WEAP_HIT: mtype = ATM_BASIC; break;
        case WEAP_SLASH: mtype = ATM_SLASH; break;
        case WEAP_PIERCE: mtype = ATM_PIERCE; break;
        case WEAP_CLEAVE: mtype = ATM_CLEAVE; break;
        case WEAP_SLICE: mtype = ATM_SLICE; break;
        case WEAP_STAB: mtype = ATM_STAB; break;
        case WEAP_WHIP: mtype = ATM_WHIP; break;
        case WEAP_CRUSH: mtype = ATM_CRUSH; break;
        case WEAP_BLUD: mtype = ATM_BLUD; break;
        default: mtype = ATM_BASIC; break;
        }
        for (i = 0; i < MAXATTACKMESS && attack_mess[mtype][i].level != -1; i++)
            if (dam < attack_mess[mtype][i].level
            || attack_mess[mtype][i+1].level == -1) {
                snprintf(buf1, sizeof(buf1), "%s %s%s", attack_mess[mtype][i].buf1, op->name, attack_mess[mtype][i].buf2);
                snprintf(buf2, sizeof(buf2), "%s", attack_mess[mtype][i].buf3);
                found++;
                break;
            }
    } else {
        for (i = 0; i < MAXATTACKMESS && attack_mess[ATM_BASIC][i].level != -1; i++)
            if (dam < attack_mess[ATM_BASIC][i].level
            || attack_mess[ATM_BASIC][i+1].level == -1) {
                snprintf(buf1, sizeof(buf1), "%s %s%s", attack_mess[ATM_BASIC][i].buf1, op->name, attack_mess[ATM_BASIC][i].buf2);
                snprintf(buf2, sizeof(buf2), "%s", attack_mess[ATM_BASIC][i].buf3);
                found++;
                break;
            }
    }

    if (!found) {
        snprintf(buf1, sizeof(buf1), "hit");
        snprintf(buf2, sizeof(buf2), "hits");
    }

    /* bail out if a monster is casting spells */
    owner = object_get_owner(hitter);
    if (hitter->type != PLAYER && (owner == NULL || owner->type != PLAYER))
        return;

    /* scale down magic considerably. */
    if (type&AT_MAGIC && rndm(0, 5))
        return;

    /* Did a player hurt another player?  Inform both! */
    /* only show half the player->player combat messages */
    if (op->type == PLAYER
    && rndm(0, 1)
    && ((owner != NULL ? owner : hitter)->type) == PLAYER) {
        if (owner != NULL)
            snprintf(buf, sizeof(buf), "%s's %s %s you.", owner->name, hitter->name, buf2);
        else {
            snprintf(buf, sizeof(buf), "%s%s you.", hitter->name, buf2);
            if (dam != 0) {
                if (hitter->chosen_skill)
                    play_sound_player_only(op->contr, SOUND_TYPE_HIT_BY, op, 0, hitter->chosen_skill->name);
                else if (dam < 10)
                    play_sound_player_only(op->contr, SOUND_TYPE_HIT_BY, op, 0, "low");
                else if (dam < 20)
                    play_sound_player_only(op->contr, SOUND_TYPE_HIT_BY, op, 0, "medium");
                else
                    play_sound_player_only(op->contr, SOUND_TYPE_HIT_BY, op, 0, "high");
            }
        }
        draw_ext_info(NDI_BLACK, 0, op, MSG_TYPE_VICTIM, MSG_TYPE_VICTIM_WAS_HIT, buf);
    } /* end of player hitting player */

    /* scale down these messages too */
    /*if(hitter->type == PLAYER && rndm(0, 2) == 0) {*/
    if (hitter->type == PLAYER) {
        snprintf(buf, sizeof(buf), "You %s.", buf1);
        if (dam != 0) {
            if (hitter->chosen_skill)
                play_sound_player_only(hitter->contr, SOUND_TYPE_HIT, hitter, 0, hitter->chosen_skill->name);
            else if (dam < 10)
                play_sound_player_only(hitter->contr, SOUND_TYPE_HIT, hitter, 0, "low");
            else if (dam < 20)
                play_sound_player_only(hitter->contr, SOUND_TYPE_HIT, hitter, 0, "medium");
            else
                play_sound_player_only(hitter->contr, SOUND_TYPE_HIT, hitter, 0, "high");
        }
        draw_ext_info(NDI_BLACK, 0, hitter, MSG_TYPE_ATTACK, MSG_TYPE_ATTACK_DID_HIT,
                      buf);
    } else if (owner != NULL && owner->type == PLAYER) {
        /* look for stacked spells and start reducing the message chances */
        if (hitter->type == SPELL_EFFECT
        && (hitter->subtype == SP_EXPLOSION || hitter->subtype == SP_BULLET || hitter->subtype == SP_CONE)) {
            i = 4;
            map = hitter->map;
            if (out_of_map(map, hitter->x, hitter->y))
                return;
            FOR_MAP_PREPARE(map, hitter->x, hitter->y, next)
                if (next->type == SPELL_EFFECT
                && (next->subtype == SP_EXPLOSION || next->subtype == SP_BULLET || next->subtype == SP_CONE)) {
                    i *= 3;
                    if (i > 10000)
                        /* no need to test more, and avoid overflows */
                        break;
                }
            FOR_MAP_FINISH();
            if (i < 0)
                return;
            if (rndm(0, i) != 0)
                return;
        } else if (rndm(0, 5) != 0)
            return;
        play_sound_map(SOUND_TYPE_HIT, owner, 0, "hit");
        draw_ext_info_format(NDI_BLACK, 0, owner, MSG_TYPE_ATTACK, MSG_TYPE_ATTACK_PET_HIT,
                             "Your %s%s %s.",
                             hitter->name, buf2, op->name);
    }
}

/**
 * Find correct parameters for attack, do some sanity checks.
 * @param target
 * will point to victim's head.
 * @param hitter
 * will point to hitter's head.
 * @param simple_attack
 * will be 1 if one of victim or target isn't on a map, 0 else.
 * @return
 * 0 if hitter can attack target, 1 else.
 */
static int get_attack_mode(object **target, object **hitter,
                           int *simple_attack) {
    if (QUERY_FLAG(*target, FLAG_FREED) || QUERY_FLAG(*hitter, FLAG_FREED)) {
        LOG(llevError, "BUG: get_attack_mode(): freed object\n");
        return 1;
    }
    *target = HEAD(*target);
    *hitter = HEAD(*hitter);
    if ((*hitter)->env != NULL || (*target)->env != NULL) {
        *simple_attack = 1;
        return 0;
    }
    if (QUERY_FLAG(*target, FLAG_REMOVED)
    || QUERY_FLAG(*hitter, FLAG_REMOVED)
    || (*hitter)->map == NULL
    || !on_same_map((*hitter), (*target))) {
        LOG(llevError, "BUG: hitter (arch %s, name %s) with no relation to target\n", (*hitter)->arch->name, (*hitter)->name);
        return 1;
    }
    *simple_attack = 0;
    return 0;
}

/**
 * Check if target and hitter are still in a relation similar to the one
 * determined by get_attack_mode().
 *
 * @param target
 * who is attacked.
 * @param hitter
 * who is attacking.
 * @param simple_attack
 * previous mode as returned by get_attack_mode().
 * @return
 * 1 if the relation has changed, 0 else.
 */
static int abort_attack(object *target, object *hitter, int simple_attack) {
    int new_mode;

    if (hitter->env == target || target->env == hitter)
        new_mode = 1;
    else if (QUERY_FLAG(hitter, FLAG_REMOVED)
    || QUERY_FLAG(target, FLAG_REMOVED)
    || hitter->map == NULL || !on_same_map(hitter, target))
        return 1;
    else
        new_mode = 0;
    return new_mode != simple_attack;
}

static void thrown_item_effect(object *, object *);

/**
 * Handles simple attack cases.
 * @param op
 * victim. Should be the head part.
 * @param hitter
 * attacked. Should be the head part.
 * @param base_dam
 * damage to do.
 * @param base_wc
 * wc to hit with.
 * @return
 * dealt damage.
 * @todo
 * fix void return values. Try to remove gotos. Better document when it's called.
 */
static int attack_ob_simple(object *op, object *hitter, int base_dam, int base_wc) {
    int simple_attack, roll, dam;
    uint32 type;
    tag_t op_tag, hitter_tag;

    if (get_attack_mode(&op, &hitter, &simple_attack))
        return 1;

    /* Lauwenmark: This is used to handle script_weapons with weapons.
     * Only used for players.
     */
    if (hitter->type == PLAYER) {
        if (hitter->current_weapon != NULL) {
            /* Lauwenmark: Handle for plugin attack event */
            if (execute_event(hitter->current_weapon, EVENT_ATTACKS,
                              hitter, op, NULL, SCRIPT_FIX_ALL) != 0)
                return 0;
            if (hitter->current_weapon->anim_suffix)
                apply_anim_suffix(hitter, hitter->current_weapon->anim_suffix);
        } else if (hitter->chosen_skill &&  hitter->chosen_skill->anim_suffix)
            /* if no weapon, then skill (karate, wraith feed) attack */
            apply_anim_suffix(hitter, hitter->chosen_skill->anim_suffix);
    }
    op_tag = op->count;
    hitter_tag = hitter->count;
    /*
     * A little check to make it more difficult to dance forward and back
     * to avoid ever being hit by monsters.
     */
    if (!simple_attack && QUERY_FLAG(op, FLAG_MONSTER)
        && op->speed_left > -(FABS(op->speed))*0.3) {
        /* Decrease speed BEFORE calling process_object.  Otherwise, an
         * infinite loop occurs, with process_object calling monster_move(),
         * which then gets here again.  By decreasing the speed before
         * we call process_object, the 'if' statement above will fail.
         */
        op->speed_left--;
        process_object(op);
        if (object_was_destroyed(op, op_tag)
        || object_was_destroyed(hitter, hitter_tag)
        || abort_attack(op, hitter, simple_attack))
            return 1;
    }

    roll = random_roll(1, 20, hitter, PREFER_HIGH);

    /* Adjust roll for various situations. */
    if (!simple_attack)
        roll += adj_attackroll(hitter, op);

    /* See if we hit the creature */
    if (roll >= 20 || op->stats.ac >= base_wc-roll) {
        if (settings.casting_time == TRUE) {
            if (hitter->type == PLAYER && hitter->casting_time > -1) {
                hitter->casting_time = -1;
                draw_ext_info(NDI_UNIQUE, 0, hitter, MSG_TYPE_ATTACK, MSG_TYPE_ATTACK_FUMBLE,
                              "You attacked and lost your spell!");
            }
            if (op->casting_time > -1 && base_dam > 0) {
                op->casting_time = -1;
                if (op->type == PLAYER)  {
                    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ATTACK, MSG_TYPE_ATTACK_FUMBLE,
                                  "You were hit and lost your spell!");
                    draw_ext_info_format(NDI_ALL|NDI_UNIQUE, 5, NULL,
                                         MSG_TYPE_ATTACK, MSG_TYPE_ATTACK_FUMBLE,
                                         "%s was hit by %s and lost a spell.",
                                         op->name, hitter->name);
                }
            }
        }
        if (!simple_attack) {
            /* If you hit something, the victim should *always *wake up.
             * Before, invisible hitters could avoid doing this.
             * -b.t. */
            if (QUERY_FLAG(op, FLAG_SLEEP))
                CLEAR_FLAG(op, FLAG_SLEEP);

            /* If the victim can't see the attacker, it may alert others
             * for help. */
            if (op->type != PLAYER && !monster_can_see_enemy(op, hitter)
                && !object_get_owner(op) && rndm(0, op->stats.Int))
                monster_npc_call_help(op);

            /* if you were hidden and hit by a creature, you are discovered*/
            if (op->hide && QUERY_FLAG(hitter, FLAG_ALIVE)) {
                make_visible(op);
                if (op->type == PLAYER)
                    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_VICTIM, MSG_TYPE_VICTIM_WAS_HIT,
                                  "You were hit by a wild attack. You are no longer hidden!");
            }

            /* thrown items (hitter) will have various effects
             * when they hit the victim.  For things like thrown daggers,
             * this sets 'hitter' to the actual dagger, and not the
             * wrapper object.
             */
            thrown_item_effect(hitter, op);
            if (object_was_destroyed(hitter, hitter_tag)
                || object_was_destroyed(op, op_tag)
                || abort_attack(op, hitter, simple_attack)) {
                return 0;
            }
        }

        /* Need to do at least 1 damage, otherwise there is no point
         * to go further and it will cause FPE's below.
         */
        if (base_dam <= 0)
            base_dam = 1;

        type = hitter->attacktype;
        if (!type)
            type = AT_PHYSICAL;
        /* Handle monsters that hit back */
        if (!simple_attack && QUERY_FLAG(op, FLAG_HITBACK)
            && QUERY_FLAG(hitter, FLAG_ALIVE)) {
            if (op->attacktype&AT_ACID && hitter->type == PLAYER)
                draw_ext_info(NDI_UNIQUE, 0, hitter, MSG_TYPE_VICTIM, MSG_TYPE_VICTIM_WAS_HIT,
                              "You are splashed by acid!\n");
            hit_player(hitter, random_roll(0, (op->stats.dam), hitter, PREFER_LOW), op, op->attacktype, 1);
            if (object_was_destroyed(op, op_tag)
            || object_was_destroyed(hitter, hitter_tag)
            || abort_attack(op, hitter, simple_attack)) {
                return 0;
            }
        }

        /* In the new attack code, it should handle multiple attack
         * types in its area, so remove it from here.
         */
        dam = hit_player(op, random_roll(1, base_dam, hitter, PREFER_HIGH), hitter, type, 1);
        if (object_was_destroyed(op, op_tag)
        || object_was_destroyed(hitter, hitter_tag)
        || abort_attack(op, hitter, simple_attack)) {
            return 0;
        }
    } else {/* end of if hitter hit op */
        dam = 0; /* if we missed, dam=0 */
    }

    /*attack_message(dam, type, op, hitter);*/

    return dam;
}

/**
 * Simple wrapper for attack_ob_simple(), will use hitter's values.
 * @param op
 * victim.
 * @param hitter
 * attacker.
 * @return
 * dealt damage.
 */
int attack_ob(object *op, object *hitter) {
    hitter = HEAD(hitter);
    return attack_ob_simple(op, hitter, hitter->stats.dam, hitter->stats.wc);
}

/**
 * Try to put an arrow in inventory.
 *
 * @param op
 * arrow to try to insert.
 * @param tmp
 * what is stopping the arrow.
 * @return
 * 1 if op was inserted into tmp's inventory, 0 otherwise.
 */
static int stick_arrow(object *op, object *tmp) {
    /* If the missile hit a player, we insert it in their inventory.
     * However, if the missile is heavy, we don't do so (assume it falls
     * to the ground after a hit).  What a good value for this is up to
     * debate - 5000 is 5 kg, so arrows, knives, and other light weapons
     * stick around.
     */
    if (op->weight <= 5000 && tmp->stats.hp >= 0) {
        object_remove(op);
        op = object_insert_in_ob(op, HEAD(tmp));
        return 1;
    } else
        return 0;
}

/**
 * hit_with_arrow() disassembles the missile, attacks the victim and
 * reassembles the missile.
 *
 * @param op
 * missile hitting.
 * @param victim
 * who is hit by op.
 * @return
 * pointer to the reassembled missile, or NULL if the missile
 * isn't available anymore.
 */
object *hit_with_arrow(object *op, object *victim) {
    object *container, *hitter;
    int hit_something = 0;
    tag_t victim_tag, hitter_tag, container_tag;
    sint16 victim_x, victim_y;
    mapstruct *victim_map;
    const char *old_skill = NULL;

    /* Disassemble missile */
    hitter = op->inv;
    FOR_OB_AND_BELOW_PREPARE(hitter)
        if (hitter->type != EVENT_CONNECTOR) {
            break;
        }
    FOR_OB_AND_BELOW_FINISH();
    if (!hitter) {
        container = NULL;
        hitter = op;
        if (free_no_drop(hitter))
            return NULL;
    } else {
        container = op;
        object_remove(hitter);
        if (free_no_drop(hitter))
            return NULL;

        object_insert_in_map_at(hitter, container->map, hitter, INS_NO_MERGE|INS_NO_WALK_ON, container->x, container->y);
        /* Note that we now have an empty THROWN_OBJ on the map.  Code that
         * might be called until this THROWN_OBJ is either reassembled or
         * removed at the end of this function must be able to deal with empty
         * THROWN_OBJs. */
	container_tag = container->count;
    }

    /* Try to hit victim */
    victim_x = victim->x;
    victim_y = victim->y;
    victim_map = victim->map;
    victim_tag = victim->count;
    hitter_tag = hitter->count;
    /* Lauwenmark: Handling plugin attack event for thrown items */
    /* FIXME provide also to script the skill? hitter is the throwed
       items, but there is no information about the fact it was
       thrown
    */
    if (execute_event(op, EVENT_ATTACKS, hitter, victim, NULL, SCRIPT_FIX_ALL) == 0) {
        /*
         * temporary set the hitter's skill to the one associated with the
         * throw wrapper. This is needed to that thrower gets it's xp at the
         * correct level. This might proves an awfull hack :/ We should really
         * provide attack_ob_simple with the skill to use...
         */
        if (container != NULL) {
            old_skill = hitter->skill;
            hitter->skill = add_refcount(container->skill);
        }
        hit_something = attack_ob_simple(victim, hitter, op->stats.dam, op->stats.wc);
    }
    /* Arrow attacks door, rune of summoning is triggered, demon is put on
     * arrow, move_apply() calls this function, arrow sticks in demon,
     * attack_ob_simple() returns, and we've got an arrow that still exists
     * but is no longer on the map. Ugh. (Beware: Such things can happen at
     * other places as well!)
     */
    if (object_was_destroyed(hitter, hitter_tag) || hitter->env != NULL) {
        if (container) {
            object_remove(container);
            object_free_drop_inventory(container);
        }
        return NULL;
    }
    if (container != NULL) {
        free_string(hitter->skill);
        hitter->skill = old_skill;
    }
    /* Missile hit victim */
    /* if the speed is > 10, then this is a fast moving arrow, we go straight
     * through the target
     */
    if (hit_something && op->speed <= 10.0) {
        /* Stop arrow */
        if (container == NULL) {
            hitter = fix_stopped_arrow(hitter);
            if (hitter == NULL)
                return NULL;
        } else {
            if(!object_was_destroyed(container, container_tag)) {
                object_remove(container);
                object_free_drop_inventory(container);
            }
        }

        /* Try to stick arrow into victim */
        if (!object_was_destroyed(victim, victim_tag)
	        && stick_arrow(hitter, victim)) {
	    object_set_owner(hitter, NULL);
            return NULL;
	}

        /* Else try to put arrow on victim's map square
        * remove check for P_WALL here.  If the arrow got to this
        * space, that is good enough - with the new movement code,
        * there is now the potential for lots of spaces where something
        * can fly over but not otherwise move over.  What is the correct
        * way to handle those otherwise?
        */
        if (victim_x != hitter->x || victim_y != hitter->y) {
            object_remove(hitter);
	    object_set_owner(hitter, NULL);
            object_insert_in_map_at(hitter, victim_map, hitter, 0, victim_x, victim_y);
        } else {
            /* Else leave arrow where it is */
            object_merge(hitter, NULL);
	    object_set_owner(hitter, NULL);
        }
        return NULL;
    }

    if (hit_something && op->speed >= 10.0)
        op->speed -= 1.0;

    /* Missile missed victim - reassemble missile */
    if (container) {
        object_remove(hitter);
        object_insert_in_ob(hitter, container);
    }
    return op;
}
/**
 * Handles wall tearing animation. Will change the face according to the hp/maxhp ration.
 *
 * If the wall reaches its last animation, either free it or set it non living so it doesn't block anymore.
 * @param op
 * wall to update.
 */
static void tear_down_wall(object *op) {
    int perc = 0;

    if (!op->stats.maxhp) {
        LOG(llevError, "TEAR_DOWN wall %s had no maxhp.\n", op->name);
        perc = 1;
    } else if (!GET_ANIM_ID(op)) {
        /* Object has been called - no animations, so remove it */
        if (op->stats.hp < 0) {
            object_remove(op); /* Should update LOS */
            object_free_drop_inventory(op);
            /* Don't know why this is here - object_remove() should do it for us */
            /*update_position(m, x, y);*/
        }
        return; /* no animations, so nothing more to do */
    }
    assert(op->stats.maxhp > 0);
    perc = NUM_ANIMATIONS(op)-((int)NUM_ANIMATIONS(op)*op->stats.hp)/op->stats.maxhp;

    if (perc >= (int)NUM_ANIMATIONS(op))
        perc = NUM_ANIMATIONS(op)-1;
    else if (perc < 1)
        perc = 1;
    SET_ANIMATION(op, perc);
    object_update(op, UP_OBJ_FACE);
    if (perc == NUM_ANIMATIONS(op)-1) { /* Reached the last animation */
        if (op->face == blank_face) {
            /* If the last face is blank, remove the ob */
            object_remove(op); /* Should update LOS */
            object_free_drop_inventory(op);

            /* object_remove() should call update_position for us */
            /*update_position(m, x, y);*/
        } else { /* The last face was not blank, leave an image */
            CLEAR_FLAG(op, FLAG_BLOCKSVIEW);
            update_all_los(op->map, op->x, op->y);
            op->move_block = 0;
            CLEAR_FLAG(op, FLAG_ALIVE);
        }
    }
}

/**
 * Creature is scared, update its values.
 * @param target
 * scared creature.
 * @param hitter
 * who scared target.
 */
static void scare_creature(object *target, object *hitter) {
    object *owner = object_get_owner(hitter);

    if (!owner)
        owner = hitter;

    SET_FLAG(target, FLAG_SCARED);
    if (!target->enemy)
        object_set_enemy(target, owner);
}

/**
 * Handles one attacktype's damage.
 * This doesn't damage the creature, but returns how much it should
 * take. However, it will do other effects (paralyzation, slow, etc.).
 * @param op
 * victim of the attack.
 * @param hitter
 * attacker.
 * @param dam
 * maximum dealt damage.
 * @param attacknum
 * number of the attacktype of the attack. See @ref Attacktypes "the ATNR_xxx"
 * values.
 * @return
 * damage to actually do.
 */
static int hit_with_one_attacktype(object *op, object *hitter, int dam, uint32 attacknum) {
    int doesnt_slay = 1;
    char name_hitter[MAX_BUF], name_op[MAX_BUF];

    /* Catch anyone that may be trying to send us a bitmask instead of the number */
    if (attacknum >= NROFATTACKS) {
        LOG(llevError, "hit_with_one_attacktype: Invalid attacknumber passed: %u\n", attacknum);
        return 0;
    }

    if (dam < 0) {
        LOG(llevError, "hit_with_one_attacktype called with negative damage: %d\n", dam);
        return 0;
    }

    if (hitter->current_weapon && hitter->current_weapon->discrete_damage != NULL)
        dam = hitter->current_weapon->discrete_damage[attacknum];
    else if (hitter->discrete_damage != NULL)
        dam = hitter->discrete_damage[attacknum];

    /* AT_INTERNAL is supposed to do exactly dam.  Put a case here so
     * people can't mess with that or it otherwise get confused.  */
    if (attacknum == ATNR_INTERNAL)
        return dam;

    if (hitter->slaying) {
        if ((op->race != NULL && strstr(hitter->slaying, op->race))
        || (op->arch && op->arch->name != NULL && strstr(op->arch->name, hitter->slaying))) {
            doesnt_slay = 0;
            dam *= 3;
        }
    }

    /* Adjust the damage for resistance. Note that neg. values increase damage. */
    if (op->resist[attacknum]) {
        /* basically:  dam = dam*(100-op->resist[attacknum])/100;
         * in case 0>dam>1, we try to "simulate" a float value-effect */
        dam *= (100-op->resist[attacknum]);
        if (dam >= 100)
            dam /= 100;
        else
            dam = (dam > (random_roll(0, 99, op, PREFER_LOW))) ? 1 : 0;
    }

    /* Special hack.  By default, if immune to something, you
     * shouldn't need to worry.  However, acid is an exception, since
     * it can still damage your items.  Only include attacktypes if
     * special processing is needed */

    if (op->resist[attacknum] >= 100
    && doesnt_slay
    && attacknum != ATNR_ACID)
        return 0;

    /* Keep this in order - makes things easier to find */

    switch (attacknum) {
    case ATNR_PHYSICAL:
        /*  here also check for diseases */
        check_physically_infect(op, hitter);
        break;

        /* Don't need to do anything for:
           magic,
           fire,
           electricity,
           cold */

    case ATNR_CONFUSION:
    case ATNR_POISON:
    case ATNR_SLOW:
    case ATNR_PARALYZE:
    case ATNR_FEAR:
    case ATNR_CANCELLATION:
    case ATNR_DEPLETE:
    case ATNR_BLIND: {
            /* chance for inflicting a special attack depends on the
             * difference between attacker's and defender's level
             */
            int level_diff = MIN(110, MAX(0, op->level-hitter->level));

            /* First, only creatures/players with speed can be affected.
             * Second, just getting hit doesn't mean it always affects
             * you.  Third, you still get a saving through against the
             * effect.
             */
            if (op->speed
            && (QUERY_FLAG(op, FLAG_MONSTER) || op->type == PLAYER)
            && !(rndm(0, (attacknum == ATNR_SLOW ? 6 : 3)-1))
            && !did_make_save(op, level_diff, op->resist[attacknum]/10)) {
                /* Player has been hit by something */
                if (attacknum == ATNR_CONFUSION)
                    confuse_living(op, hitter, dam);
                else if (attacknum == ATNR_POISON)
                    poison_living(op, hitter, dam);
                else if (attacknum == ATNR_SLOW)
                    slow_living(op, hitter, dam);
                else if (attacknum == ATNR_PARALYZE)
                    paralyze_living(op, dam);
                else if (attacknum == ATNR_FEAR)
                    scare_creature(op, hitter);
                else if (attacknum == ATNR_CANCELLATION)
                    cancellation(op);
                else if (attacknum == ATNR_DEPLETE)
                    drain_stat(op);
                else if (attacknum == ATNR_BLIND
                && !QUERY_FLAG(op, FLAG_UNDEAD)
                && !QUERY_FLAG(op, FLAG_GENERATOR))
                    blind_living(op, hitter, dam);
            }
            dam = 0; /* These are all effects and don't do real damage */
        }
        break;

    case ATNR_ACID: {
            int flag = 0;

            /* Items only get corroded if you're not on a battleground and
             * if your acid resistance is below 50%. */
            if (!op_on_battleground(op, NULL, NULL, NULL)
            && (op->resist[ATNR_ACID] < 50)) {
                FOR_INV_PREPARE(op, tmp) {
                    if (tmp->invisible)
                        continue;
                    if (!QUERY_FLAG(tmp, FLAG_APPLIED)
                    || (tmp->resist[ATNR_ACID] >= 10))
                        /* >= 10% acid res. on itmes will protect these */
                        continue;
                    if (!(tmp->material&M_IRON))
                        continue;
                    if (tmp->magic < -4) /* Let's stop at -5 */
                        continue;
                    if (tmp->type == RING
                    /* removed boots and gloves from exclusion list in PR */
                    || tmp->type == GIRDLE
                    || tmp->type == AMULET
                    || tmp->type == WAND
                    || tmp->type == ROD)
                        continue; /* To avoid some strange effects */

                    /* High damage acid has better chance of corroding objects */
                    if (rndm(0, dam+4) > random_roll(0, 39, op, PREFER_HIGH)+2*tmp->magic) {
                        if (op->type == PLAYER) {
                            /* Make this more visible */
                            query_name(hitter, name_hitter, MAX_BUF);
                            query_name(tmp, name_op, MAX_BUF);
                            draw_ext_info_format(NDI_UNIQUE|NDI_RED, 0, op,
                                                 MSG_TYPE_VICTIM, MSG_TYPE_VICTIM_WAS_HIT,
                                                 "The %s's acid corrodes your %s!",
                                                 name_hitter, name_op);
                        }
                        flag = 1;
                        tmp->magic--;
                        if (op->type == PLAYER)
                            esrv_update_item(UPD_NAME, op, tmp);
                    }
                } FOR_INV_FINISH();
                if (flag)
                    fix_object(op); /* Something was corroded */
            }
        }
        break;

    case ATNR_DRAIN: {
            /* rate is the proportion of exp drained.  High rate means
             * not much is drained, low rate means a lot is drained.
             */
            int rate;

            if (op->resist[ATNR_DRAIN] >= 0)
                rate = 50+op->resist[ATNR_DRAIN]/2;
            else
                rate = 5000/(100-op->resist[ATNR_DRAIN]);

            /* full protection has no effect.  Nothing else in this
             * function needs to get done, so just return.  */
            if (!rate)
                return 0;

            if (op->stats.exp <= rate) {
                if (op->type == GOLEM)
                    dam = 999; /* Its force is "sucked" away. 8) */
                else
                    /* If we can't drain, lets try to do physical damage */
                    dam = hit_with_one_attacktype(op, hitter, dam, ATNR_PHYSICAL);
            } else {
                /* Randomly give the hitter some hp */
                if (hitter->stats.hp < hitter->stats.maxhp
                && (op->level > hitter->level)
                && random_roll(0, (op->level-hitter->level+2), hitter, PREFER_HIGH) > 3)
                    hitter->stats.hp++;

                /* Can't do drains on battleground spaces.
                 * Move the wiz check up here - before, the hitter wouldn't gain exp
                 * exp, but the wiz would still lose exp! If drainee is a wiz,
                 * nothing happens.
                 * Try to credit the owner.  We try to display player -> player drain
                 * attacks, hence all the != PLAYER checks.
                 */
                if (!op_on_battleground(hitter, NULL, NULL, NULL) && !QUERY_FLAG(op, FLAG_WAS_WIZ)) {
                    object *owner = object_get_owner(hitter);
                    sint64 orig_exp = op->stats.exp;

                    change_exp(op, -op->stats.exp/rate, NULL, 0);

                    if (owner && owner != hitter) {
                        if (op->type != PLAYER || owner->type != PLAYER)
                            change_exp(owner, MIN(op->stats.exp/(rate*2), orig_exp - op->stats.exp),
                                       hitter->chosen_skill ? hitter->chosen_skill->skill : NULL, SK_EXP_TOTAL);
                    } else if (op->type != PLAYER || hitter->type != PLAYER) {
                        change_exp(hitter, MIN(op->stats.exp/(rate*2), orig_exp - op->stats.exp),
                                   hitter->chosen_skill ? hitter->chosen_skill->skill : NULL, 0);
                    }
                }
                dam = 1; /* Drain is an effect.  Still return 1 - otherwise, if you have pure
                          * drain attack, you won't know that you are actually sucking out EXP,
                          * as the messages will say you missed
                          */
            }
        }
        break;

    case ATNR_TURN_UNDEAD: {
            if (QUERY_FLAG(op, FLAG_UNDEAD)) {
                object *owner = object_get_owner(hitter) == NULL ? hitter : object_get_owner(hitter);
                const object *god = find_god(determine_god(owner));
                int div = 1;

                /* if undead are not an enemy of your god, you turn them
                        * at half strength */
                if (!god
                || !god->slaying
                || strstr(god->slaying, undead_name) == NULL)
                    div = 2;

                /* The previous code was highly suspect - resist turn undead/100 would
                 * at best give a bonus of 1 - increase that to resist turn undead/20 -
                 * this gives a bit higher bonus.  Also the bonus was added to the wrong
                 * side of the equation, actually making it easier to turn creatures
                 * if they had that resistance.
                 */
                if ((op->level*div + (op->resist[ATNR_TURN_UNDEAD] / 20)) < (get_turn_bonus(owner->stats.Wis)+owner->level))
                    scare_creature(op, owner);
            } else
                dam = 0; /* don't damage non undead - should we damage undead? */
        }
        break;

    case ATNR_DEATH:
        deathstrike_living(op, hitter, &dam);
        break;

    case ATNR_CHAOS:
        query_name(op, name_op, MAX_BUF);
        query_name(hitter, name_hitter, MAX_BUF);
        LOG(llevError, "%s was hit by %s with non-specific chaos.\n", name_op, name_hitter);
        dam = 0;
        break;

    case ATNR_COUNTERSPELL:
        query_name(op, name_op, MAX_BUF);
        query_name(hitter, name_hitter, MAX_BUF);
        LOG(llevError, "%s was hit by %s with counterspell attack.\n", name_op, name_hitter);
        dam = 0;
        /* This should never happen.  Counterspell is handled
         * seperately and filtered out.  If this does happen,
         * Counterspell has no effect on anything but spells, so it
         * does no damage. */
        break;

    case ATNR_HOLYWORD: {
            /* This has already been handled by hit_player,
             *  no need to check twice  -- DAMN */

            object *owner = object_get_owner(hitter) == NULL ? hitter : object_get_owner(hitter);

            /* As with turn undead above, give a bonus on the saving throw */
            if (op->level+(op->resist[ATNR_HOLYWORD]/100) < owner->level+get_turn_bonus(owner->stats.Wis))
                scare_creature(op, owner);
        }
        break;

    case ATNR_LIFE_STEALING: {
            int new_hp;
            /* this is replacement to drain for players, instead of taking
             * exp it takes hp. It is geared for players, probably not
             * much use giving it to monsters
             *
             * life stealing doesn't do a lot of damage, but it gives the
             * damage it does do to the player.  Given that,
             * it only does 1/30'th normal damage (hence the divide by
             * 3000). Wraith get 1/2 of the damage, and therefore divide
             * by 200. This number may need tweaking for game balance.
             */

            int dam_modifier = is_wraith_pl(hitter) ? 200 : 3000;

            /* You can't steal life from something undead or not alive. */
            if (op->type == GOLEM
            || (QUERY_FLAG(op, FLAG_UNDEAD))
            || !(QUERY_FLAG(op, FLAG_ALIVE))
            || (op->type == DOOR))
                return 0;
            /* If drain protection is higher than life stealing, use that */
            if (op->resist[ATNR_DRAIN] >= op->resist[ATNR_LIFE_STEALING])
                dam = (dam*(100-op->resist[ATNR_DRAIN]))/dam_modifier;
            else
                dam = (dam*(100-op->resist[ATNR_LIFE_STEALING]))/dam_modifier;
            /* You die at -1 hp, not zero. */
            if (dam > op->stats.hp+1)
                dam = op->stats.hp+1;
            new_hp = hitter->stats.hp+dam;
            if (new_hp > hitter->stats.maxhp)
                new_hp = hitter->stats.maxhp;
            if (new_hp > hitter->stats.hp)
                hitter->stats.hp = new_hp;

            /* Wraith also get food through life stealing */
            if (is_wraith_pl(hitter)) {
                if (hitter->stats.food+dam >= 999)
                    hitter->stats.food = 999;
                else
                    hitter->stats.food += dam;
                fix_object(hitter);
            }
        }
    }
    return dam;
}

/*
 * This function is defined in party.c, but conditionally, something "make proto"
 * doesn't handle. So define it locally.
 */
#ifdef PARTY_KILL_LOG
void party_add_kill(partylist *party, const char *killer, const char *dead, long exp);
#endif

/**
 * An object was killed, handle various things (logging, messages, ...).
 *
 * GROS: This code comes from hit_player. It has been made external to
 * allow script procedures to "kill" objects in a combat-like fashion.
 * It was initially used by (kill-object) developed for the Collector's
 * Sword. Note that nothing has been changed from the original version
 * of the following code.
 *
 * Will LOG pk, handles battleground, and so on.
 *
 * This function was a bit of a mess with hitter getting changed,
 * values being stored away but not used, etc.  I've cleaned it up
 * a bit - I think it should be functionally equivalant.
 * MSW 2002-07-17
 *
 * @param op
 * what is being killed.
 * @param dam
 * damage done to it.
 * @param hitter
 * what is hitting it.
 * @return
 * dealt damage.
 * @todo
 * finish commenting what it does exactly.
 */
static int kill_object(object *op, int dam, object *hitter) {
    char kill_message[MAX_BUF];
    const char *skill;
    int maxdam = 0;
    int battleg = 0;  /* true if op standing on battleground */
    int pk = 0;       /* true if op and what controls hitter are both players*/
    object *owner = NULL;
    const object *skop = NULL;
    sstring death_animation;

    if (op->stats.hp >= 0)
        return -1;

    /* Lauwenmark: Handle for plugin death event */
    if (execute_event(op, EVENT_DEATH, hitter, NULL, NULL, SCRIPT_FIX_ALL) != 0)
        return 0;
    /* Lauwenmark: Handle for the global kill event */
    execute_global_event(EVENT_GKILL, op, hitter);

    if (op->map) {
        death_animation = object_get_value(op, "death_animation");
        if (death_animation != NULL) {
            object *death = create_archetype(death_animation);

            if (death)
                object_insert_in_map_at(death, op->map, op, 0, op->x, op->y);
        }
    }

    /* maxdam needs to be the amount of damage it took to kill
     * this creature.  The function(s) that call us have already
     * adjusted the creatures HP total, so that is negative.
     */
    maxdam = dam+op->stats.hp+1;

    if (QUERY_FLAG(op, FLAG_BLOCKSVIEW))
        update_all_los(op->map, op->x, op->y); /* makes sure los will be recalculated */

    if (op->type == DOOR) {
        op->speed = 0.1;
        object_update_speed(op);
        op->speed_left = -0.05;
        return maxdam;
    }
    if (QUERY_FLAG(op, FLAG_FRIENDLY) && op->type != PLAYER) {
        object *owner;

        remove_friendly_object(op);
        owner = object_get_owner(op);
        if (owner != NULL
        && owner->type == PLAYER) {
            if (owner->contr->ranges[range_golem] == op) {
                owner->contr->ranges[range_golem] = NULL;
                owner->contr->golem_count = 0;
            }

            /*play_sound_player_only(owner1->contr, SOUND_PET_IS_KILLED, 0, 0);*/
            /* Maybe we should include the owner that killed this, maybe not */
            draw_ext_info_format(NDI_UNIQUE, 0, owner, MSG_TYPE_ATTACK, MSG_TYPE_ATTACK_PET_DIED,
                                 "Your pet, the %s, is killed by %s.",
                                 op->name, hitter->name);
        }
        /*
         * there can be items both friendly and without any owner, for instance
         * in various maps, so this isn't an error.
        else
            LOG(llevError, "BUG: hit_player(): Encountered golem without owner.\n");
         */

        object_remove(op);
        object_free_drop_inventory(op);
        return maxdam;
    }

    /* Now lets start dealing with experience we get for killing something */

    owner = object_get_owner(hitter);
    if (owner == NULL)
        owner = hitter;

    /* is the victim (op) standing on battleground? */
    if (op_on_battleground(op, NULL, NULL, NULL))
        battleg = 1;

    /* is this player killing?*/
    if (op->type == PLAYER && owner->type == PLAYER)
        pk = 1;

    /* Player killed something */
    if (owner->type == PLAYER) {

        /* Log players killing other players - makes it easier to detect
         * and filter out malicious player killers - that is why the
         * ip address is included.
         */
        if (op->type == PLAYER && !battleg)  {
            time_t t = time(NULL);
            struct tm *tmv;
            char buf[256];
            char name[MAX_BUF];

            tmv = localtime(&t);
            strftime(buf, sizeof(buf), "%a %b %d %H:%M:%S %Y", tmv);
            query_name(op, name, MAX_BUF);

            LOG(llevInfo, "%s PLAYER_KILL_PLAYER: %s (%s) killed %s\n", buf, owner->name, owner->contr->socket.host, name);
        }

        /* try to filter some things out - basically, if you are
         * killing a level 1 creature and your level 20, you
         * probably don't want to see that.
         */
        if (owner->level < op->level*2 ||  op->stats.exp > 1000) {
            if (owner != hitter) {
                char killed[MAX_BUF], with[MAX_BUF];

                query_name(op, killed, MAX_BUF);
                query_name(hitter, with, MAX_BUF);
                draw_ext_info_format(NDI_BLACK, 0, owner, MSG_TYPE_ATTACK, MSG_TYPE_ATTACK_DID_KILL,
                                     "You killed %s with %s.",
                                     killed, with);
            } else {
                char killed[MAX_BUF];

                query_name(op, killed, MAX_BUF);
                draw_ext_info_format(NDI_BLACK, 0, owner, MSG_TYPE_ATTACK, MSG_TYPE_ATTACK_DID_KILL,
                                     "You killed %s.",
                                     killed);
            }
            /* Only play sounds for melee kills */
            if (hitter->type == PLAYER)
                play_sound_map(SOUND_TYPE_HIT, owner, 0, "kill");
        }

        /* If a player kills another player, not on
         * battleground, the "killer" looses 1 luck. Since this is
         * not reversible, it's actually quite a pain IMHO. -AV
         * Fix bug in that we were changing the luck of the hitter, not
         * player that the object belonged to - so if you killed another player
         * with spells, pets, whatever, there was no penalty.
         * Changed to make luck penalty configurable in settings.
         */
        if (op->type == PLAYER && owner != op && !battleg)
            change_luck(owner, -settings.pk_luck_penalty);

        /* This code below deals with finding the appropriate skill
         * to credit exp to.  This is a bit problematic - we should
                * probably never really have to look at current_weapon->skill
         */
        skill = NULL;
        if (hitter->skill && hitter->type != PLAYER)
            skill = hitter->skill;
        else if (owner->chosen_skill) {
            skill = owner->chosen_skill->skill;
            skop = owner->chosen_skill;
        } else if (QUERY_FLAG(owner, FLAG_READY_WEAPON))
            skill = owner->current_weapon->skill;
        else
            LOG(llevError, "kill_object - unable to find skill that killed monster\n");

        /* We have the skill we want to credit to - now find the object this goes
         * to.  Make sure skop is an actual skill, and not a skill tool!
         */
        if ((!skop || skop->type != SKILL) && skill) {
            int i;

            for (i = 0; i < NUM_SKILLS; i++)
                if (owner->contr->last_skill_ob[i]
                && !strcmp(owner->contr->last_skill_ob[i]->skill, skill)) {
                    skop = owner->contr->last_skill_ob[i];
                    break;
                }
        }
    } /* Was it a player that hit somethign */
    else {
        skill = NULL;
    }

    /* Pet (or spell) killed something. */
    if (owner != hitter) {
        char name_op[MAX_BUF], name_hitter[MAX_BUF];
        const char *owner_prefix;
        const char *op_prefix;

        owner_prefix = !battleg && pk && owner->contr != NULL && !owner->contr->peaceful ? "hostile " : "";
        op_prefix = !battleg && pk && op->contr != NULL && !op->contr->peaceful ? "hostile " : "";

        query_name(op, name_op, MAX_BUF);
        query_name(hitter, name_hitter, MAX_BUF);
        snprintf(kill_message, sizeof(kill_message), "%s%s killed %s%s with %s%s.", owner_prefix, owner->name, op_prefix, name_op, name_hitter, battleg ? " (duel)" : (pk ? " (pk)" : ""));
    } else {
        const char *hitter_prefix;
        const char *op_prefix;

        hitter_prefix = !battleg && pk && hitter->contr != NULL && !hitter->contr->peaceful ? "hostile " : "";
        op_prefix = !battleg && pk && op->contr != NULL && !op->contr->peaceful ? "hostile " : "";

        snprintf(kill_message, sizeof(kill_message), "%s%s killed %s%s%s%s.", hitter_prefix, hitter->name, op_prefix, op->name,
                      (QUERY_FLAG(hitter, FLAG_MONSTER)) || hitter->type == PLAYER ?
                      " in hand to hand combat" : "", battleg ? " (duel)" : (pk ? " (pk)" : ""));
    }
    /* These may have been set in the player code section above */
    if (!skop)
        skop = hitter->chosen_skill;
    if (!skill && skop)
        skill = skop->skill;

    draw_ext_info(NDI_ALL, op->type == PLAYER ? 1 : 10, NULL, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_PLAYER,
                  kill_message);


    /* If you didn't kill yourself, and your not the wizard */
    if (owner != op && !QUERY_FLAG(op, FLAG_WAS_WIZ)) {
        sint64 exp;

        exp = calc_skill_exp(owner, op, skop);

        /* Really don't give much experience for killing other players */
        if (op->type == PLAYER) {
            if (battleg) {
                draw_ext_info(NDI_UNIQUE, 0, owner, MSG_TYPE_ATTACK, MSG_TYPE_ATTACK_DID_KILL,
                              "Your foe has fallen!\nVICTORY!!!");
            } else {
                exp = settings.pk_max_experience_percent*exp/100;
                if (settings.pk_max_experience >= 0)
                    exp = MIN(settings.pk_max_experience, exp);
                /* Never exceed what victim can lose considering permanent exp. */
                exp = check_exp_loss(op, exp);
            }
        }

        /* Don't know why this is set this way - doesn't make
         * sense to just divide everything by two for no reason.
         */

        if (!settings.simple_exp)
            exp = exp/2;

        /* if op is standing on "battleground" (arena), no way to gain
         * exp by killing him
         */
        if (battleg)
            exp = 0;

#ifdef PARTY_KILL_LOG
        if (owner->type == PLAYER && owner->contr->party != NULL) {
            char name[MAX_BUF];
            char op_name[MAX_BUF];

            query_name(owner, name, MAX_BUF);
            query_name(op, op_name, sizeof(op_name));
            party_add_kill(owner->contr->party, name, op_name, exp);
        }
#endif
        share_exp(owner, exp, skill, SK_EXP_TOTAL);
    } /* end if person didn't kill himself */

    if (op->type != PLAYER) {
        object_remove(op);
        object_free_drop_inventory(op);
    /* Player has been killed! */
    } else {
        if (owner->type == PLAYER) {
            snprintf(op->contr->killer, BIG_NAME, "%s the %s", owner->name, owner->contr->title);
        } else {
            strncpy(op->contr->killer, hitter->name, BIG_NAME);
            op->contr->killer[BIG_NAME-1] = '\0';
        }
        /* Need to run kill_player (just in case, make sure is not wiz) */
        if (!QUERY_FLAG(op, FLAG_WIZ))
            kill_player(op, owner->type == PLAYER ? owner : hitter);
    }
    /* This was return -1 - that doesn't seem correct - if we return -1, process
     * continues in the calling function.
     */
    return maxdam;
}

/**
 * Find out if this is friendly fire (PVP and attacker is peaceful) or not
 *
 * @param op
 * victim.
 * @param hitter
 * attacker.
 * @return
 * 0 this is not friendly fire, 1 if hitter is a peaceful player, 2 if hitter is a pet of a peaceful player.
 */
int friendly_fire(object *op, object *hitter) {
    object *owner;
    int friendlyfire;

    hitter = HEAD(hitter);
    friendlyfire = 0;

    if (op->type == PLAYER) {
        if (hitter->type == PLAYER && hitter->contr->peaceful == 1)
            return 1;

        owner = object_get_owner(hitter);
        if (owner != NULL) {
            if (owner->type == PLAYER && owner->contr->peaceful == 1)
                friendlyfire = 2;
        }

        if (hitter->type == SPELL
        || hitter->type == POISONING
        || hitter->type == DISEASE
        || hitter->type == RUNE)
            friendlyfire = 0;
    }
    return friendlyfire;
}

/**
 * Object is attacked by something.
 *
 * This isn't used just for players, but in fact most objects.
 *
 * Oct 95 - altered the following slightly for MULTIPLE_GODS hack
 * which needs new attacktype AT_HOLYWORD to work . b.t.
 *
 * @param op
 * object to be hit
 * @param dam
 * base damage - protections/vulnerabilities/slaying matches can modify it.
 * @param hitter
 * what is hitting the object
 * @param type
 * attacktype
 * @param full_hit
 * set if monster area does not matter.
 * @return
 * dealt damage.
 * @todo
 * rename to something more meaningful.
 */
int hit_player(object *op, int dam, object *hitter, uint32 type, int full_hit) {
    int maxdam = 0, ndam = 0, magic = (type&AT_MAGIC);
    int maxattacktype, attacknum;
    int body_attack = op->head != NULL;   /* Did we hit op's head? */
    int simple_attack;
    tag_t op_tag, hitter_tag;
    int rtn_kill = 0;
    int friendlyfire;
    object *owner;

    /* Lauwenmark: Handle for plugin attack event */
    if (execute_event(op, EVENT_ATTACKED, hitter, hitter->current_weapon ? hitter->current_weapon : hitter, NULL, SCRIPT_FIX_ALL) != 0)
        return 0;
    FOR_INV_PREPARE(op, inv)
        if (execute_event(inv, EVENT_ATTACKED, hitter, hitter->current_weapon ? hitter->current_weapon : hitter, NULL, SCRIPT_FIX_ALL) != 0)
            return 0;
    FOR_INV_FINISH();

    if (get_attack_mode(&op, &hitter, &simple_attack))
        return 0;

    /* very simple: if our target has no_damage 1 set or is wiz, we can't hurt him */
    if (QUERY_FLAG(op, FLAG_WIZ) || QUERY_FLAG(op, FLAG_NO_DAMAGE))
        return 0;

    op_tag = op->count;
    hitter_tag = hitter->count;

    if (body_attack) {
        /* slow and paralyze must hit the head.  But we don't want to just
         * return - we still need to process other attacks the spell still
         * might have.  So just remove the paralyze and slow attacktypes,
         * and keep on processing if we have other attacktypes.
         * return if only magic or nothing is left - under normal code
         * we don't attack with pure magic if there is another attacktype.
         * Only do processing if the initial attacktype includes one of those
         * attack so we don't cancel out things like magic bullet.
         */
        if (type&(AT_PARALYZE|AT_SLOW)) {
            type &= ~(AT_PARALYZE|AT_SLOW);
            if (!type || type == AT_MAGIC)
                return 0;
        }
    }

    if (!simple_attack && op->type == DOOR) {
        object *tmp;

        tmp = object_find_by_type2(op, RUNE, TRAP);
        if (tmp != NULL) {
            spring_trap(tmp, hitter);
            if (object_was_destroyed(hitter, hitter_tag)
            || object_was_destroyed(op, op_tag)
            || abort_attack(op, hitter, simple_attack))
                return 0;
        }
    }

    if (!QUERY_FLAG(op, FLAG_ALIVE) || op->stats.hp < 0) {
        /* FIXME: If a player is killed by a rune in a door, the
         * object_was_destroyed() check above doesn't return, and might get here.
         */
        LOG(llevDebug, "victim (arch %s, name %s) already dead in hit_player()\n", op->arch->name, op->name);
        return 0;
    }

#ifdef ATTACK_DEBUG
    LOG(llevDebug, "hit player: attacktype %d, dam %d\n", type, dam);
#endif

    if (magic) {
        /* basically:  dam = dam*(100-op->resist[attacknum])/100;
         * in case 0>dam>1, we try to "simulate" a float value-effect */
        dam = dam*(100-op->resist[ATNR_MAGIC]);
        if (dam >= 100)
            dam /= 100;
        else
            dam = (dam > (rndm(0, 99))) ? 1 : 0;
    }

    /* AT_CHAOS here is a weapon or monster.  Spells are handled by hit_map
     * We don't use shuffle_attack(), because that changes the it in the
     * creature structure, and thus is permanent until fix_object() is
     * called again.  Chaos should change on each attack.
     */
    if (type&AT_CHAOS) {
        type = ATTACKS[RANDOM()%(sizeof(ATTACKS)/sizeof(*ATTACKS))].attacktype|AT_MAGIC;
    }

    /* Holyword is really an attacktype modifier (like magic is).  If
     * holyword is part of an attacktype, then make sure the creature is
     * a proper match, otherwise no damage.
     */
    if (type&AT_HOLYWORD) {
        const object *god;

        if ((!hitter->slaying || (!(op->race && strstr(hitter->slaying, op->race))
                    && !(op->name && strstr(hitter->slaying, op->name))))
        && (!QUERY_FLAG(op, FLAG_UNDEAD) || (hitter->title != NULL
                    && (god = find_god(determine_god(hitter))) != NULL
                    && god->race != NULL
                    && strstr(god->race, undead_name) != NULL)))
            return 0;
    }

    maxattacktype = type; /* initialize this to something */
    for (attacknum = 0; attacknum < NROFATTACKS; attacknum++) {
        int attacktype;

        attacktype = 1<<attacknum;

        /* Magic isn't really a true attack type - it gets combined with other
         * attack types.  As such, skip it over.  However, if magic is
         * the only attacktype in the group, then still attack with it
         */
        if (attacktype == AT_MAGIC && (type&~AT_MAGIC))
            continue;

        /* Go through and hit the player with each attacktype, one by one.
         * hit_with_one_attacktype only figures out the damage, doesn't inflict
         * it.  It will do the appropriate action for attacktypes with
         * effects (slow, paralization, etc.
                */
        if (type&attacktype) {
            ndam = hit_with_one_attacktype(op, hitter, dam, attacknum);
            /* the >= causes us to prefer messages from special attacks, if
             * the damage is equal.
             */
            if (ndam >= maxdam) {
                maxdam = ndam;
                maxattacktype = 1<<attacknum;
            }
            /* Special case: death attack always deals all damage, as it should kill the monster
             * right away. */
            if (attacktype == AT_DEATH && ndam > 0)
                full_hit = 1;
        }
    }


    /* if this is friendly fire then do a set % of damage only
     * Note - put a check in to make sure this attack is actually
     * doing damage - otherwise, the +1 in the coe below will make
     * an attack do damage before when it otherwise didn't
     * Only reduce damage if not on battlground - if in arena, do
     * full damage.  Note that it is intentional that the check for
     * battleground is inside the friendlyfire if statement - op_on_battleground()
     * is a fairly costly function to call, and we don't want to call it for
     * every attack - by doing it only for friendlyfire, it shouldn't get called
     * that often
     */
    friendlyfire = friendly_fire(op, hitter);
    if (friendlyfire && maxdam) {
        if (!op_on_battleground(op, NULL, NULL, NULL)) {
            maxdam = ((dam*settings.set_friendly_fire)/100)+1;
#ifdef ATTACK_DEBUG
            LOG(llevDebug, "Friendly fire (type:%d setting: %d%) did %d damage dropped to %d\n", friendlyfire, settings.set_friendly_fire, dam, maxdam);
#endif
        }
    }

    if (!full_hit) {
        archetype *at;
        int area;
        int remainder;

        area = 0;
        for (at = op->arch; at != NULL; at = at->more)
            area++;
        assert(area > 0);

        /* basically: maxdam /= area; we try to "simulate" a float
           value-effect */
        remainder = 100*(maxdam%area)/area;
        maxdam /= area;
        if (RANDOM()%100 < remainder)
            maxdam++;
    }

#ifdef ATTACK_DEBUG
    LOG(llevDebug, "Attacktype %d did %d damage\n", type, maxdam);
#endif

    owner = object_get_owner(hitter);
    if (owner != NULL) {
        if (op->enemy != hitter)
            object_set_enemy(op, owner);
    } else if (QUERY_FLAG(hitter, FLAG_ALIVE))
        if (op->enemy == NULL || rndm(1, 20) == 0)
            object_set_enemy(op, hitter);

    if (QUERY_FLAG(op, FLAG_UNAGGRESSIVE) && op->type != PLAYER) {
        /* The unaggressives look after themselves 8) */
        CLEAR_FLAG(op, FLAG_UNAGGRESSIVE);
        monster_npc_call_help(op);
    }

    if (magic && did_make_save(op, op->level, 0))
        maxdam = maxdam/2;

    attack_message(maxdam, maxattacktype, op, hitter);

    op->stats.hp -= maxdam;

    /* Eneq(@csd.uu.se): Check to see if monster runs away. */
    if (op->stats.hp >= 0
    && (QUERY_FLAG(op, FLAG_MONSTER) || op->type == PLAYER)
    && op->stats.hp < (signed short)(((float)op->run_away/(float)100)*(float)op->stats.maxhp)) {
        if (QUERY_FLAG(op, FLAG_MONSTER))
            SET_FLAG(op, FLAG_RUN_AWAY);
        else
            scare_creature(op, hitter);
    }

    if (QUERY_FLAG(op, FLAG_TEAR_DOWN)) {
        if (maxdam)
            tear_down_wall(op);
        return maxdam; /* nothing more to do for wall */
    }

    /* See if the creature has been killed */
    rtn_kill = kill_object(op, maxdam, hitter);
    if (rtn_kill != -1)
        return rtn_kill;

    /* Used to be ghosthit removal - we now use the ONE_HIT flag.  Note
     * that before if the player was immune to ghosthit, the monster
     * remained - that is no longer the case.
     */
    if (QUERY_FLAG(hitter, FLAG_ONE_HIT)) {
        if (QUERY_FLAG(hitter, FLAG_FRIENDLY))
            remove_friendly_object(hitter);
        object_remove(hitter);
        object_free_drop_inventory(hitter);
    /* Lets handle creatures that are splitting now */
    } else if (type&AT_PHYSICAL && !QUERY_FLAG(op, FLAG_FREED) && QUERY_FLAG(op, FLAG_SPLITTING)) {
        change_object(op);
    } else if (type&AT_DRAIN && hitter->type == GRIMREAPER && hitter->value++ > 10) {
        object_remove(hitter);
        object_free_drop_inventory(hitter);
    }
    return maxdam;
}

/**
 * Poison a living thing.
 *
 * @param op
 * victim.
 * @param hitter
 * who is attacking.
 * @param dam
 * damage to deal.
 */
static void poison_living(object *op, object *hitter, int dam) {
    archetype *at = find_archetype("poisoning");
    object *tmp = arch_present_in_ob(at, op);
    const char *skill;

    if (tmp == NULL) {
        tmp = arch_to_object(at);
        if (tmp == NULL)
            LOG(llevError, "Failed to clone arch poisoning.\n");
        else {
            tmp = object_insert_in_ob(tmp, op);
            /*  peterm:  give poisoning some teeth.  It should
             * be able to kill things better than it does:
             * damage should be dependent something--I choose to
             * do this:  if it's a monster, the damage from the
             * poisoning goes as the level of the monster/2.
             * If anything else, goes as damage.
             */

            if (QUERY_FLAG(hitter, FLAG_ALIVE))
                tmp->stats.dam += hitter->level/2;
            else
                tmp->stats.dam = dam;

            object_copy_owner(tmp, hitter);   /*  so we get credit for poisoning kills */
            skill = hitter->skill;
            if (!skill && hitter->chosen_skill)
                skill = hitter->chosen_skill->name;

            if (skill && skill != tmp->skill) {
                if (tmp->skill)
                    free_string(tmp->skill);
                tmp->skill = add_refcount(skill);
            }

            tmp->stats.food += dam;  /*  more damage, longer poisoning */

            if (op->type == PLAYER) {
                /* player looses stats, maximum is -10 of each */
                tmp->stats.Con = MAX(-(dam/4+1), -10);
                tmp->stats.Str = MAX(-(dam/3+2), -10);
                tmp->stats.Dex = MAX(-(dam/6+1), -10);
                tmp->stats.Int = MAX(-dam/7, -10);
                SET_FLAG(tmp, FLAG_APPLIED);
                fix_object(op);
                draw_ext_info(NDI_UNIQUE, 0, op,
                              MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_BAD_EFFECT_START,
                              "You suddenly feel very ill.");
            }
            if (hitter->type == PLAYER)
                draw_ext_info_format(NDI_UNIQUE, 0, hitter,
                                     MSG_TYPE_ATTACK, MSG_TYPE_ATTACK_DID_HIT,
                                     "You poison %s.",
                                     op->name);
            else {
                object *owner;

                owner = object_get_owner(hitter);
                if (owner != NULL && owner->type == PLAYER)
                    draw_ext_info_format(NDI_UNIQUE, 0, owner,
                        MSG_TYPE_ATTACK, MSG_TYPE_ATTACK_PET_HIT,
                        "Your %s poisons %s.",
                        hitter->name, op->name);
            }
            tmp->speed_left = 0;
        }
    } else
        tmp->stats.food++;
}

/**
 * Slow a living thing.
 *
 * @param op
 * victim.
 * @param hitter
 * who is attacking.
 * @param dam
 * damage to deal.
 */
static void slow_living(object *op, object *hitter, int dam) {
    archetype *at = find_archetype("slowness");
    object *tmp;

    if (at == NULL) {
        LOG(llevError, "Can't find slowness archetype.\n");
    }
    tmp = arch_present_in_ob(at, op);
    if (tmp == NULL) {
        tmp = arch_to_object(at);
        tmp = object_insert_in_ob(tmp, op);
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_BAD_EFFECT_START,
                      "The world suddenly moves very fast!");
    } else
        tmp->stats.food++;
    SET_FLAG(tmp, FLAG_APPLIED);
    tmp->speed_left = 0;
    fix_object(op);
}

/**
 * Confuse a living thing.
 *
 * @param op
 * victim.
 * @param hitter
 * who is attacking.
 * @param dam
 * damage to deal.
 */
void confuse_living(object *op, object *hitter, int dam) {
    object *tmp;
    int maxduration;

    tmp = object_present_in_ob_by_name(FORCE, "confusion", op);
    if (!tmp) {
        tmp = create_archetype(FORCE_NAME);
        tmp = object_insert_in_ob(tmp, op);
    }

    /* Duration added per hit and max. duration of confusion both depend
     *  on the player's resistance
     */
    tmp->speed = 0.05;
    tmp->subtype = FORCE_CONFUSION;
    tmp->duration = 8+MAX(1, 5*(100-op->resist[ATNR_CONFUSION])/100);
    if (tmp->name)
        free_string(tmp->name);
    tmp->name = add_string("confusion");
    maxduration = MAX(2, 30*(100-op->resist[ATNR_CONFUSION])/100);
    if (tmp->duration > maxduration)
        tmp->duration = maxduration;

    if (op->type == PLAYER && !QUERY_FLAG(op, FLAG_CONFUSED))
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_BAD_EFFECT_START,
                      "You suddenly feel very confused!");
    SET_FLAG(op, FLAG_CONFUSED);
}

/**
 * Blind a living thing.
 *
 * @param op
 * victim.
 * @param hitter
 * who is attacking.
 * @param dam
 * damage to deal.
 */
void blind_living(object *op, object *hitter, int dam) {
    object *tmp, *owner;
    char victim[MAX_BUF];

    /* Save some work if we know it isn't going to affect the player */
    if (op->resist[ATNR_BLIND] == 100)
        return;

    tmp = object_present_in_ob(BLINDNESS, op);
    if (!tmp) {
        tmp = create_archetype("blindness");
        SET_FLAG(tmp, FLAG_BLIND);
        SET_FLAG(tmp, FLAG_APPLIED);
        /* use floats so we don't lose too much precision due to rounding errors.
         * speed is a float anyways.
         */
        tmp->speed = tmp->speed*(100.0-(float)op->resist[ATNR_BLIND])/100;

        tmp = object_insert_in_ob(tmp, op);
        change_abil(op, tmp);  /* Mostly to display any messages */
        fix_object(op);        /* This takes care of some other stuff */

        owner = object_get_owner(hitter);
        if (owner == NULL)
            owner = hitter;

        query_name(op, victim, MAX_BUF);
        draw_ext_info_format(NDI_UNIQUE, 0, owner, MSG_TYPE_ATTACK, MSG_TYPE_ATTACK_DID_HIT,
                             "Your attack blinds %s!",
                             victim);
    }
    tmp->stats.food += dam;
    if (tmp->stats.food > 10)
        tmp->stats.food = 10;
}

/**
 * Paralyze a living thing.
 *
 * @param op
 * victim.
 * @param dam
 * damage to deal.
 */
void paralyze_living(object *op, int dam) {
    float effect, max;

    /* Do this as a float - otherwise, rounding might very well reduce this to 0 */
    effect = (float)dam*3.0*(100.0-op->resist[ATNR_PARALYZE])/100;

    if (effect == 0)
        return;

    op->speed_left -= FABS(op->speed)*effect;

    /* max number of ticks to be affected for. */
    max = (100-op->resist[ATNR_PARALYZE])/2;
    if (op->speed_left < -(FABS(op->speed)*max))
        op->speed_left = (float)-(FABS(op->speed)*max);
}

/**
 * Attempts to kill 'op'.
 *
 * The intention of a death attack is to kill outright things
 * that are a lot weaker than the attacker, have a chance of killing
 * things somewhat weaker than the caster, and no chance of
 * killing something equal or stronger than the attacker.
 *
 * If a deathstrike attack has a slaying, only a monster
 * whose name or race matches a comma-delimited list in the slaying
 * field of the deathstriking object is affected (this includes undead).
 * If no slaying set, only undead are unaffected.
 *
 * @param op
 * victim.
 * @param hitter
 * attacker.
 * @param[out] dam
 * damage to deal, will contain computed damage or 0 if strike failed.
 */
static void deathstrike_living(object *op, object *hitter, int *dam) {
    int atk_lev, def_lev, kill_lev;

    if (hitter->slaying) {
        if (!((QUERY_FLAG(op, FLAG_UNDEAD) && strstr(hitter->slaying, undead_name))
        || (op->race && strstr(hitter->slaying, op->race))))
            return;
    } else
        if (QUERY_FLAG(op, FLAG_UNDEAD))
            return;

    def_lev = op->level;
    if (def_lev < 1) {
        LOG(llevError, "BUG: arch %s, name %s with level < 1\n", op->arch->name, op->name);
        def_lev = 1;
    }
    atk_lev = (hitter->chosen_skill ? hitter->chosen_skill->level : hitter->level)/2;
    /* LOG(llevDebug, "Deathstrike - attack level %d, defender level %d\n", atk_lev, def_lev); */

    if (atk_lev >= def_lev) {
        kill_lev = random_roll(0, atk_lev-1, hitter, PREFER_HIGH);

        /* Note that the below effectively means the ratio of the atk vs
         * defener level is important - if level 52 character has very little
         * chance of killing a level 50 monster.  This should probably be
         * redone.
         */
        if (kill_lev >= def_lev) {
            *dam = op->stats.hp+10; /* take all hp. they can still save for 1/2 */
            /* I think this doesn't really do much.  Because of
             * integer rounding, this only makes any difference if the
             * attack level is double the defender level.
             */
            *dam *= kill_lev/def_lev;
        }
    } else {
        *dam = 0;  /* no harm done */
    }
}

/**
 * Handles any special effects of thrown
 * items (like attacking living creatures--a potion thrown at a
 * monster).
 *
 * @param hitter
 * thrown item.
 * @param victim
 * object that is hit by hitter.
 * @todo
 * invert parameters for coherence with other functions?
 */
static void thrown_item_effect(object *hitter, object *victim) {
    if (!QUERY_FLAG(hitter, FLAG_ALIVE)) {
        /* May not need a switch for just 2 types, but this makes it
         * easier for expansion.
         */
        switch (hitter->type) {
        case POTION:
            /* should player get a save throw instead of checking magic protection? */
            if (QUERY_FLAG(victim, FLAG_ALIVE)
            && !QUERY_FLAG(victim, FLAG_UNDEAD)
            && (victim->resist[ATNR_MAGIC] < 60))
                (void)ob_apply(hitter, victim, 0);
            break;

        case POISON: /* poison drinks */
            /* As with potions, should monster get a save? */
            if (QUERY_FLAG(victim, FLAG_ALIVE)
            && !QUERY_FLAG(victim, FLAG_UNDEAD)
            && (victim->resist[ATNR_POISON] < 60))
                (void)ob_apply(victim, hitter, 0);
            break;

            /* Removed case statements that did nothing.
             * food may be poisonous, but monster must be willing to eat it,
             * so we don't handle it here.
             * Containers should perhaps break open, but that code was disabled.
             */
        }
    }
}

/**
 * Adjustments to attack rolls by various conditions
 * @param hitter
 * who is hitting.
 * @param target
 * victim of the attack.
 * @return
 * adjustment to attack roll.
 */
static int adj_attackroll(object *hitter, object *target) {
    object *attacker = hitter;
    int adjust = 0;

    /* safety */
    if (!target || !hitter || !hitter->map || !target->map || !on_same_map(hitter, target)) {
        LOG(llevError, "BUG: adj_attackroll(): hitter and target not on same map\n");
        return 0;
    }

    /* aimed missiles use the owning object's sight */
    if (is_aimed_missile(hitter)) {
        attacker = object_get_owner(hitter);
        if (attacker == NULL)
            attacker = hitter;
        /* A player who saves but hasn't quit still could have objects
         * owned by him - need to handle that case to avoid crashes.
         */
        if (QUERY_FLAG(attacker, FLAG_REMOVED))
            attacker = hitter;
    } else if (!QUERY_FLAG(hitter, FLAG_ALIVE))
        return 0;

    /* determine the condtions under which we make an attack.
     * Add more cases, as the need occurs. */

    if (!monster_can_see_enemy(attacker, target)) {
        /* target is unseen */
        if (target->invisible || QUERY_FLAG(attacker, FLAG_BLIND))
            adjust -= 10;
        /* dark map penalty for the hitter, though xray can help for a player */
        else if (target->map && target->map->darkness > 0 && !monster_stand_in_light(target) && (hitter->type != PLAYER || !player_can_view(hitter, target)))
            adjust -= target->map->darkness;
    }

    if (QUERY_FLAG(attacker, FLAG_SCARED))
        adjust -= 3;

    if (QUERY_FLAG(target, FLAG_UNAGGRESSIVE))
        adjust += 1;

    if (QUERY_FLAG(target, FLAG_SCARED))
        adjust += 1;

    if (QUERY_FLAG(attacker, FLAG_CONFUSED))
        adjust -= 3;

    /* if we attack at a different 'altitude' its harder
     * Note - only make this adjustment if the target actually
     * has a move type.  Doors don't (they don't move), and
     * this would evaluate as true.  If anything, something without
     * a move type should be easier to hit.
     */
    if (target->move_type && (attacker->move_type&target->move_type) == 0)
        adjust -= 2;

    return adjust;
}

/**
 * Determine if the object is an 'aimed' missile.
 *
 * @param op
 * object to check.
 * @return
 * 1 if aimed missile, 0 else.
 */
static int is_aimed_missile(object *op) {
    /* I broke what used to be one big if into a few nested
     * ones so that figuring out the logic is at least possible.
     */
    if (op && (op->move_type&MOVE_FLYING)) {
        if (op->type == ARROW || op->type == THROWN_OBJ)
            return 1;
        else if (op->type == SPELL_EFFECT
        && (op->subtype == SP_BULLET || op->subtype == SP_EXPLOSION))
            return 1;
    }
    return 0;
}
