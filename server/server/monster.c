/*
 * Crossfire -- cooperative multi-player graphical RPG and adventure game
 *
 * Copyright (c) 1999-2013 Mark Wedel and the Crossfire Development Team
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
 * This deals with monster moving, attacking, using items and such.
 * The core function is monster_move().
 */

#include <assert.h>
#include <global.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#include <spells.h>
#include <skills.h>
#endif

static int monster_can_hit(object *ob1, object *ob2, rv_vector *rv);
static int monster_cast_spell(object *head, object *part, object *pl, int dir);
static int monster_use_scroll(object *head, object *part, object *pl, int dir);
static int monster_use_skill(object *head, object *part, object *pl, int dir);
static int monster_use_range(object *head, object *part, object *pl, int dir);
static int monster_use_bow(object *head, object *part, object *pl, int dir);
static void monster_check_pickup(object *monster);
static int monster_can_pick(object *monster, object *item);
static void monster_apply_below(object *monster);
static int monster_dist_att(int dir, object *enemy, object *part, rv_vector *rv);
static int monster_run_att(int dir, object *ob, object *enemy, object *part, rv_vector *rv);
static int monster_hitrun_att(int dir, object *ob, object *enemy);
static int monster_wait_att(int dir, object *ob, object *enemy, object *part, rv_vector *rv);
static int monster_disthit_att(int dir, object *ob, object *enemy, object *part, rv_vector *rv);
static int monster_wait_att2(int dir, rv_vector *rv);
static void monster_circ1_move(object *ob);
static void monster_circ2_move(object *ob);
static void monster_pace_movev(object *ob);
static void monster_pace_moveh(object *ob);
static void monster_pace2_movev(object *ob);
static void monster_pace2_moveh(object *ob);
static void monster_rand_move(object *ob);
static int monster_talk_to_npc(object *npc, talk_info *info);

/** Minimum monster detection radius. */
#define MIN_MON_RADIUS 3

/**
 * Checks npc->enemy and returns that enemy if still valid,
 * NULL otherwise.
 * This is map tile aware.
 * If this returns an enemy, the range vector rv should also be
 * set to sane values.
 *
 * @param npc
 * monster we're considering
 * @param[out] rv
 * will contain vector to go to enemy if function returns not NULL.
 * @return
 * valid enemy for npc.
 */
object *monster_check_enemy(object *npc, rv_vector *rv) {
    object *owner;

    /* if this is pet, let him attack the same enemy as his owner
     * TODO: when there is no ower enemy, try to find a target,
     * which CAN attack the owner. */
    owner = object_get_owner(npc);
    if ((npc->attack_movement&HI4) == PETMOVE) {
        if (owner == NULL)
            object_set_enemy(npc, NULL);
        else if (npc->enemy == NULL)
            object_set_enemy(npc, owner->enemy);
    }

    /* periodically, a monster may change its target.  Also, if the object
     * has been destroyed, etc, clear the enemy.
     * TODO: this should be changed, because it invokes to attack forced or
     * attacked monsters to leave the attacker alone, before it is destroyed
     */
    /* i had removed the random target leave, this invokes problems with friendly
     * objects, getting attacked and defending herself - they don't try to attack
     * again then but perhaps get attack on and on
     * If we include a aggravated flag in , we can handle evil vs evil and good vs good
     * too. */

    if (npc->enemy) {
        /* I broke these if's apart to better be able to see what
         * the grouping checks are.  Code is the same.
         */
        if (QUERY_FLAG(npc->enemy, FLAG_REMOVED)
        || QUERY_FLAG(npc->enemy, FLAG_FREED)
        || !on_same_map(npc, npc->enemy)
        || npc == npc->enemy
        || QUERY_FLAG(npc, FLAG_NEUTRAL)
        || QUERY_FLAG(npc->enemy, FLAG_NEUTRAL))
            object_set_enemy(npc, NULL);

        else if (QUERY_FLAG(npc, FLAG_FRIENDLY) && (
                (QUERY_FLAG(npc->enemy, FLAG_FRIENDLY) && !pets_should_arena_attack(npc, owner, npc->enemy))
                || (npc->enemy->type == PLAYER && !pets_should_arena_attack(npc, owner, npc->enemy))
                || npc->enemy == owner))
            object_set_enemy(npc, NULL);
        else if (!QUERY_FLAG(npc, FLAG_FRIENDLY)
        && (!QUERY_FLAG(npc->enemy, FLAG_FRIENDLY) && npc->enemy->type != PLAYER))
            object_set_enemy(npc, NULL);

        /* I've noticed that pets could sometimes get an arrow as the
         * target enemy - this code below makes sure the enemy is something
         * that should be attacked.  My guess is that the arrow hits
         * the creature/owner, and so the creature then takes that
         * as the enemy to attack.
         */
        else if (!QUERY_FLAG(npc->enemy, FLAG_MONSTER)
        && !QUERY_FLAG(npc->enemy, FLAG_GENERATOR)
        && npc->enemy->type != PLAYER
        && npc->enemy->type != GOLEM)
            object_set_enemy(npc, NULL);
    }
    return monster_can_detect_enemy(npc, npc->enemy, rv) ? npc->enemy : NULL;
}

/**
 * Returns the nearest living creature (monster or generator).
 * Modified to deal with tiled maps properly.
 * Also fixed logic so that monsters in the lower directions were more
 * likely to be skipped - instead of just skipping the 'start' number
 * of direction, revisit them after looking at all the other spaces.
 *
 * Note that being this may skip some number of spaces, it will
 * not necessarily find the nearest living creature - it basically
 * chooses one from within a 3 space radius, and since it skips
 * the first few directions, it could very well choose something
 * 3 spaces away even though something directly north is closer.
 *
 * This function is map tile aware.
 *
 * @param npc
 * monster to consider
 * @return
 * living creature, or NULL if none found.
 */
object *monster_find_nearest_living_creature(object *npc) {
    int i, mflags;
    sint16 nx, ny;
    mapstruct *m;
    int search_arr[SIZEOFFREE];

    get_search_arr(search_arr);
    for (i = 0; i < SIZEOFFREE; i++) {
        /* modified to implement smart searching using search_arr
         * guidance array to determine direction of search order
         */
        nx = npc->x+freearr_x[search_arr[i]];
        ny = npc->y+freearr_y[search_arr[i]];
        m = npc->map;

        mflags = get_map_flags(m, &m, nx, ny, &nx, &ny);
        if (mflags&P_OUT_OF_MAP)
            continue;

        if (mflags&P_IS_ALIVE) {
            object *creature;

            creature = NULL;
            FOR_MAP_PREPARE(m, nx, ny, tmp)
                if (QUERY_FLAG(tmp, FLAG_MONSTER)
                || QUERY_FLAG(tmp, FLAG_GENERATOR)
                || tmp->type == PLAYER) {
                    creature = tmp;
                    break;
                }
            FOR_MAP_FINISH();
            if (!creature) {
                LOG(llevDebug, "monster_find_nearest_living_creature: map %s (%d,%d) has is_alive set but did not find a monster?\n", m->path, nx, ny);
            } else {
                if (can_see_monsterP(m, nx, ny, i))
                    return creature;
            }
        } /* is something living on this space */
    }
    return NULL;  /* nothing found */
}

/**
 * Tries to find an enmy for npc.  We pass the range vector since
 * our caller will find the information useful.
 * Currently, only monster_move() calls this function.
 * Fix function so that we always make calls to get_rangevector
 * if we have a valid target - function as not doing so in
 * many cases.
 *
 * @param npc
 * monster we're considering.
 * @param[out] rv
 * vector that will contain how to reach the target. Must not be NULL.
 * @return
 * enemy npc wants to attack, or NULL if nont found.
 */
static object *monster_find_enemy(object *npc, rv_vector *rv) {
    object *attacker, *tmp = NULL;

    attacker = npc->attacked_by; /* save this for later use. This can be a attacker. */
    npc->attacked_by = NULL;     /* always clear the attacker entry */

    /* if we berserk, we don't care about others - we attack all we can find */
    if (QUERY_FLAG(npc, FLAG_BERSERK)) {
        tmp = monster_find_nearest_living_creature(npc);
        if (tmp == NULL)
            return NULL;
        if (!get_rangevector(npc, tmp, rv, 0))
            return NULL;
        return tmp;
    }

    /* Here is the main enemy selection.
     * We want this: if there is an enemy, attack him until its not possible or
     * one of both is dead.
     * If we have no enemy and we are...
     * a monster: try to find a player, a pet or a friendly monster
     * a friendly: only target a monster which is targeting you first or targeting a player
     * a neutral: fight a attacker (but there should be none), then do nothing
     * a pet: attack player enemy or a monster
     */

    /* pet move */
    if ((npc->attack_movement&HI4) == PETMOVE) {
        tmp = pets_get_enemy(npc, rv);
        if (tmp == NULL)
            return NULL;
        if (!get_rangevector(npc, tmp, rv, 0))
            return NULL;
        return tmp;
    }

    /* we check our old enemy. */
    tmp = monster_check_enemy(npc, rv);
    if (tmp == NULL) {
        if (attacker) { /* if we have an attacker, check him */
            /* we want be sure this is the right one! */
            if (attacker->count == npc->attacked_by_count) {
                /* TODO: thats not finished */
                /* we don't want a fight evil vs evil or good against non evil */
                if (QUERY_FLAG(npc, FLAG_NEUTRAL)
                || QUERY_FLAG(attacker, FLAG_NEUTRAL) /* neutral */
                || (QUERY_FLAG(npc, FLAG_FRIENDLY) && QUERY_FLAG(attacker, FLAG_FRIENDLY))
                || (!QUERY_FLAG(npc, FLAG_FRIENDLY) && (!QUERY_FLAG(attacker, FLAG_FRIENDLY) && attacker->type != PLAYER)))
                    CLEAR_FLAG(npc, FLAG_SLEEP); /* skip it, but lets wakeup */
                else if (on_same_map(npc, attacker)) { /* thats the only thing we must know... */
                    CLEAR_FLAG(npc, FLAG_SLEEP); /* well, NOW we really should wake up! */
                    object_set_enemy(npc, attacker);
                    if (!get_rangevector(npc, attacker, rv, 0))
                        return NULL;
                    return attacker; /* yes, we face our attacker! */
                }
            }
        }

        /* we have no legal enemy or attacker, so we try to target a new one */
        if (!QUERY_FLAG(npc, FLAG_UNAGGRESSIVE)
        && !QUERY_FLAG(npc, FLAG_FRIENDLY)
        && !QUERY_FLAG(npc, FLAG_NEUTRAL)) {
            object_set_enemy(npc, get_nearest_player(npc));
            if (npc->enemy)
                tmp = monster_check_enemy(npc, rv);
        }
    }

    return tmp;
}

/**
 * Sees if this monster should wake up.
 * Currently, this is only called from monster_move(), and
 * if enemy is set, then so should be rv.
 * @param op
 * monster to check.
 * @param enemy
 * enemy that can cause to wake up.
 * @param[out] rv
 * vector pointing to enemy.
 * @return
 * 1 if the monster should wake up, 0 otherwise.
 * @note
 * will return 0 if enemy is NULL.
 */
static int monster_check_wakeup(object *op, object *enemy, rv_vector *rv) {
    int radius = MAX(op->stats.Wis, MIN_MON_RADIUS);

    /* Trim work - if no enemy, no need to do anything below */
    if (!enemy)
        return 0;

    /* blinded monsters can only find nearby objects to attack */
    if (QUERY_FLAG(op, FLAG_BLIND))
        radius = MIN_MON_RADIUS;

    /* This covers the situation where the monster is in the dark
     * and has an enemy. If the enemy has no carried light (or isnt
     * glowing!) then the monster has trouble finding the enemy.
     * Remember we already checked to see if the monster can see in
     * the dark. */
    else if (op->map
    && op->map->darkness > 0
    && enemy
    && !enemy->invisible
    && !monster_stand_in_light(enemy)
    && (!QUERY_FLAG(op, FLAG_SEE_IN_DARK) || !QUERY_FLAG(op, FLAG_SEE_INVISIBLE))) {
        int dark = radius/(op->map->darkness);

        radius = (dark > MIN_MON_RADIUS) ? (dark+1) : MIN_MON_RADIUS;
    } else if (!QUERY_FLAG(op, FLAG_SLEEP))
        return 1;

    /* enemy should already be on this map, so don't really need to check
     * for that.
     */
    if (rv->distance < (unsigned)(QUERY_FLAG(enemy, FLAG_STEALTH) ? radius/2+1 : radius)) {
        CLEAR_FLAG(op, FLAG_SLEEP);
        return 1;
    }
    return 0;
}

/**
 * Handles random object movement. If the monster was talked to lately, then don't move and reduce wait time.
 *
 * @param op
 * object to move.
 * @return
 * 1 if moved or should wait (talked to), 0 else.
 */
static int monster_move_randomly(object *op) {
    int i;
    sstring talked;
    char value[2];

    /* timeout before moving */
    if (QUERY_FLAG(op, FLAG_UNAGGRESSIVE)) {
        talked = object_get_value(op, "talked_to");
        if (talked && strlen(talked) > 0) {
            i = atoi(talked);
            i--;

            if (i != 0) {
                value[1] = '\0';
                value[0] = '0' + i;
                object_set_value(op, "talked_to", value, 1);
                    return 1;
            }

            /* finished timeout talked to */
            object_set_value(op, "talked_to", "", 1);
        }
    }

    /* Give up to 15 chances for a monster to move randomly */
    for (i = 0; i < 15; i++) {
        if (move_object(op, RANDOM()%8+1))
            return 1;
    }
    return 0;
}

/** Maximum map size to consider when finding a path in monster_compute_path(). */
#define MAX_EXPLORE 5000

/**
 * Computes a path from source to target. Takes into account walls, other living things, and such.
 * Only works if both items are on same map.
 *
 * @param source
 * what wants to move.
 * @param target
 * target to go to.
 * @param default_dir
 * general direction from source to target.
 * @return
 * direction to go into. Will be default_dir if no path found.
 * @todo cache path, smart ajustment and such things to not compute all the time ; try directions randomly.
 */
int monster_compute_path(object *source, object *target, int default_dir) {
    unsigned short *distance;
    int explore_x[MAX_EXPLORE], explore_y[MAX_EXPLORE], dirs[8];
    int current = 0, dir, max = 1, size, x, y, check_dir, i;

    if (target->map != source->map)
        return default_dir;

    /*printf("monster_compute_path (%d, %d) => (%d, %d)\n", source->x, source->y, target->x, target->y);*/

    size = source->map->width*source->map->height;
    distance = calloc(size, sizeof(*distance));
    if (distance == NULL) {
        fatal(OUT_OF_MEMORY);
    }
    for (dir = 0; dir < size; dir++) {
        distance[dir] = 999;
    }
    distance[source->map->height * target->x + target->y] = 0;
    explore_x[0] = target->x;
    explore_y[0] = target->y;

    while (current < max) {
        /* Fisher–Yates shuffle the directions, "inside-out" algorithm
         * from http://en.wikipedia.org/wiki/Fisher-Yates_shuffle */
        dirs[0] = 1;
        for (i = 1; i < 8; i++) {
            x = RANDOM() % (i+1);
            dirs[i] = dirs[x];
            dirs[x] = i+1;
        }

        for (i = 0; i < 8; i++) {
            int diagonal;
            unsigned short new_distance;
            unsigned short *this_distance;

            check_dir = dirs[i];
            dir = absdir(default_dir+4+check_dir);
            x = explore_x[current]+freearr_x[dir];
            y = explore_y[current]+freearr_y[dir];

            if (x == source->x && y == source->y) {
                free(distance);
                return absdir(dir+4);
            }

            if (OUT_OF_REAL_MAP(source->map, x, y))
                continue;
            if (ob_blocked(source, source->map, x, y))
                continue;

            assert(source->map->height*x+y >= 0);
            assert(source->map->height*x+y < size);

            this_distance = &distance[source->map->height*explore_x[current]+explore_y[current]];
            diagonal = dir%2 == 0;
            new_distance = *this_distance+(diagonal ? 3 : 2);

            /*LOG(llevDebug, "check %d, %d dist = %d, nd = %d\n", x, y, distance[source->map->height*x+y], new_distance);*/

            if (distance[source->map->height*x+y] > new_distance) {
                assert(max < MAX_EXPLORE);
                explore_x[max] = x;
                explore_y[max] = y;

                distance[source->map->height*x+y] = new_distance;
                /*                printf("explore[%d] => (%d, %d) %u\n", max, x, y, new_distance);*/
                max++;
                if (max == MAX_EXPLORE) {
                    free(distance);
                    return default_dir;
                }
            }
        }
        current++;
    }

    /*LOG(llevDebug, "no path\n");*/
    free(distance);
    return default_dir;
}

/**
 * For a monster, regenerate hp and sp, potentially clear scared status.
 *
 * @param op
 * monster. Must have FLAG_MONSTER set.
 */
void monster_do_living(object *op) {
    assert(op);
    assert(QUERY_FLAG(op, FLAG_MONSTER));

    /*  generate hp, if applicable */
    if (op->stats.Con > 0 && op->stats.hp < op->stats.maxhp) {
        /* last heal is in funny units.  Dividing by speed puts
         * the regeneration rate on a basis of time instead of
         * #moves the monster makes.  The scaling by 8 is
         * to capture 8th's of a hp fraction regens
         *
         * Cast to sint32 before comparing to maxhp since otherwise an (sint16)
         * overflow might produce monsters with negative hp.
         */

        op->last_heal += (int)((float)(8*op->stats.Con)/FABS(op->speed));
        op->stats.hp = MIN((sint32)op->stats.hp+op->last_heal/32, op->stats.maxhp); /* causes Con/4 hp/tick */
        op->last_heal %= 32;

        /* So if the monster has gained enough HP that they are no longer afraid */
        if (QUERY_FLAG(op, FLAG_RUN_AWAY)
        && op->stats.hp >= (signed short)(((float)op->run_away/(float)100)*(float)op->stats.maxhp))
            CLEAR_FLAG(op, FLAG_RUN_AWAY);

        if (op->stats.hp > op->stats.maxhp)
            op->stats.hp = op->stats.maxhp;
    }

    /* generate sp, if applicable */
    if (op->stats.Pow > 0 && op->stats.sp < op->stats.maxsp) {
        /*  last_sp is in funny units.  Dividing by speed puts
         * the regeneration rate on a basis of time instead of
         * #moves the monster makes.  The scaling by 8 is
         * to capture 8th's of a sp fraction regens
         *
         * Cast to sint32 before comparing to maxhp since otherwise an (sint16)
         * overflow might produce monsters with negative sp.
         */

        op->last_sp += (int)((float)(8*op->stats.Pow)/FABS(op->speed));
        op->stats.sp = MIN(op->stats.sp+op->last_sp/128, op->stats.maxsp);  /* causes Pow/16 sp/tick */
        op->last_sp %= 128;
    }

    /* this should probably get modified by many more values.
    * (eg, creatures resistance to fear, level, etc. )
    */
    if (QUERY_FLAG(op, FLAG_SCARED) && !(RANDOM()%20)) {
        CLEAR_FLAG(op, FLAG_SCARED); /* Time to regain some "guts"... */
    }
}

/**
 * Makes a monster without any enemy move.
 *
 * @param op
 * monster, must have FLAG_MONSTER set.
 * @return
 * 1 if monster was removed, 0 else.
 */
static int monster_move_no_enemy(object *op) {
    assert(QUERY_FLAG(op, FLAG_MONSTER));

    if (QUERY_FLAG(op, FLAG_ONLY_ATTACK))  {
        object_remove(op);
        object_free_drop_inventory(op);
        return 1;
    }

    /* Probably really a bug for a creature to have both
       * stand still and a movement type set.
           */
    if (!QUERY_FLAG(op, FLAG_STAND_STILL))  {
        if (op->attack_movement&HI4) {
            switch (op->attack_movement&HI4) {
            case PETMOVE:
                pets_move(op);
                break;

            case CIRCLE1:
                monster_circ1_move(op);
                break;

            case CIRCLE2:
                monster_circ2_move(op);
                break;

            case PACEV:
                monster_pace_movev(op);
                break;

            case PACEH:
                monster_pace_moveh(op);
                break;

            case PACEV2:
                monster_pace2_movev(op);
                break;

            case PACEH2:
                monster_pace2_moveh(op);
                break;

            case RANDO:
                monster_rand_move(op);
                break;

            case RANDO2:
                monster_move_randomly(op);
                break;
            }
            return 0;
        }

        if (QUERY_FLAG(op, FLAG_RANDOM_MOVE))
            monster_move_randomly(op);
    } /* stand still */

    return 0;
}

/**
 * Main monster processing routine.
 *
 * Will regenerate spell points, hit points.
 * Moves the monster, handle attack, item applying, pickup, ...
 *
 * @param op
 * monster to process.
 * @return
 * 1 if the object has been freed, otherwise 0.
 */
int monster_move(object *op) {
    int dir, diff;
    object *owner, *enemy, *part;
    rv_vector rv;

    /* Monsters not on maps don't do anything.  These monsters are things
     * Like royal guards in city dwellers inventories.
     */
    if (!op->map)
        return 0;

    if (QUERY_FLAG(op, FLAG_NO_ATTACK)) { /* we never ever attack */
        object_set_enemy(op, NULL);
        enemy = NULL;
    } else {
        enemy = monster_find_enemy(op, &rv);
        if (enemy != NULL) {
            /* we have an enemy, just tell him we want him dead */
            enemy->attacked_by = op;       /* our ptr */
            enemy->attacked_by_count = op->count; /* our tag */
        }
    }

    monster_do_living(op);

    if (QUERY_FLAG(op, FLAG_SLEEP)
    || QUERY_FLAG(op, FLAG_BLIND)
    || (op->map->darkness > 0 && !QUERY_FLAG(op, FLAG_SEE_IN_DARK) && !QUERY_FLAG(op, FLAG_SEE_INVISIBLE))) {
        if (!monster_check_wakeup(op, enemy, &rv))
            return 0;
    }

    /* check if monster pops out of hidden spot */
    if (op->hide)
        do_hidden_move(op);

    if (op->pick_up)
        monster_check_pickup(op);

    if (op->will_apply)
        monster_apply_below(op); /* Check for items to apply below */

    /* If we don't have an enemy, do special movement or the like */
    if (!enemy) {
        return monster_move_no_enemy(op);
    } /* no enemy */

    /* We have an enemy.  Block immediately below is for pets */
    if ((op->attack_movement&HI4) == PETMOVE) {
        owner = object_get_owner(op);
        if (owner != NULL && !on_same_map(op, owner)) {
            pets_follow_owner(op, owner);
            /* If the pet was unable to follow the owner, free it */
            if (QUERY_FLAG(op, FLAG_REMOVED) && FABS(op->speed) > MIN_ACTIVE_SPEED) {
                remove_friendly_object(op);
                object_free_drop_inventory(op);
                return 1;
            }
            return 0;
        }
    }

    /* doppleganger code to change monster facing to that of the nearest
     * player.  Hmm.  The code is here, but no monster in the current
     * arch set uses it.
     */
    if (op->race != NULL && strcmp(op->race, "doppleganger") == 0) {
        op->face = enemy->face;
        if (op->name)
            free_string(op->name);
        add_refcount(op->name = enemy->name);
    }

    /* Calculate range information for closest body part - this
     * is used for the 'skill' code, which isn't that smart when
     * it comes to figuring it out - otherwise, giants throw boulders
     * into themselves.
     */
    if (!get_rangevector(op, enemy, &rv, 0))
        return 0;
    if (op->direction != rv.direction) {
        op->direction = rv.direction;
        op->facing = op->direction;
        if (op->animation_id)
            animate_object(op, op->direction);
    }

    /* Move the check for scared up here - if the monster was scared,
     * we were not doing any of the logic below, so might as well save
     * a few cpu cycles.
     */
    if (!QUERY_FLAG(op, FLAG_SCARED)) {
        dir = rv.direction;

        if (QUERY_FLAG(op, FLAG_RUN_AWAY))
            dir = absdir(dir+4);
        if (QUERY_FLAG(op, FLAG_CONFUSED))
            dir = get_randomized_dir(dir);

        if (QUERY_FLAG(op, FLAG_CAST_SPELL) && !(RANDOM()%3)) {
            if (monster_cast_spell(op, rv.part, enemy, dir))
                return 0;
        }

        if (QUERY_FLAG(op, FLAG_READY_SCROLL) && !(RANDOM()%3)) {
            if (monster_use_scroll(op, rv.part, enemy, dir))
                return 0;
        }

        if (QUERY_FLAG(op, FLAG_READY_RANGE) && !(RANDOM()%3)) {
            if (monster_use_range(op, rv.part, enemy, dir))
                return 0;
        }
        if (QUERY_FLAG(op, FLAG_READY_SKILL) && !(RANDOM()%3)) {
            if (monster_use_skill(op, rv.part, enemy, rv.direction))
                return 0;
        }
        if (QUERY_FLAG(op, FLAG_READY_BOW) && !(RANDOM()%2)) {
            if (monster_use_bow(op, rv.part, enemy, dir))
                return 0;
        }
    } /* If not scared */


    /* code below is for when we didn't use a range attack or a skill, so
     * either move or hit with hth attack. */

    part = rv.part;
    dir = rv.direction;

    if (QUERY_FLAG(op, FLAG_SCARED) || QUERY_FLAG(op, FLAG_RUN_AWAY))
        dir = absdir(dir+4);
    else if (!monster_can_hit(part, enemy, &rv))
        dir = monster_compute_path(op, enemy, rv.direction);

    if (QUERY_FLAG(op, FLAG_CONFUSED))
        dir = get_randomized_dir(dir);

    if ((op->attack_movement&LO4) && !QUERY_FLAG(op, FLAG_SCARED)) {
        switch (op->attack_movement&LO4) {
        case DISTATT:
            dir = monster_dist_att(dir, enemy, part, &rv);
            break;

        case RUNATT:
            dir = monster_run_att(dir, op, enemy, part, &rv);
            break;

        case HITRUN:
            dir = monster_hitrun_att(dir, op, enemy);
            break;

        case WAITATT:
            dir = monster_wait_att(dir, op, enemy, part, &rv);
            break;

        case RUSH: /* default - monster normally moves towards player */
        case ALLRUN:
            break;

        case DISTHIT:
            dir = monster_disthit_att(dir, op, enemy, part, &rv);
            break;

        case WAIT2:
            dir = monster_wait_att2(dir, &rv);
            break;

        default:
            LOG(llevDebug, "Illegal low mon-move: %d\n", op->attack_movement&LO4);
        }
    }

    if (!dir)
        return 0;

    if (!QUERY_FLAG(op, FLAG_STAND_STILL)) {
        if (move_object(op, dir)) /* Can the monster move directly toward player? */
            return 0;

        if (QUERY_FLAG(op, FLAG_SCARED)
        || !monster_can_hit(part, enemy, &rv)
        || QUERY_FLAG(op, FLAG_RUN_AWAY)) {
            /* Try move around corners if !close */
            int maxdiff = (QUERY_FLAG(op, FLAG_ONLY_ATTACK) || RANDOM()&1) ? 1 : 2;
            for (diff = 1; diff <= maxdiff; diff++) {
                /* try different detours */
                int m = 1-(RANDOM()&2);          /* Try left or right first? */
                if (move_object(op, absdir(dir+diff*m))
                || move_object(op, absdir(dir-diff*m)))
                    return 0;
            }
        }
    } /* if monster is not standing still */

    /*
     * Eneq(@csd.uu.se): Patch to make RUN_AWAY or SCARED monsters move a random
     * direction if they can't move away.
     */
    if (!QUERY_FLAG(op, FLAG_ONLY_ATTACK)
    && (QUERY_FLAG(op, FLAG_RUN_AWAY) || QUERY_FLAG(op, FLAG_SCARED)))
        if (monster_move_randomly(op))
            return 0;

    /*
     * Try giving the monster a new enemy - the player that is closest
     * to it.  In this way, it won't just keep trying to get to a target
     * that is inaccessible.
     * This could be more clever - it should go through a list of several
     * enemies, as it is now, you could perhaps get situations where there
     * are two players flanking the monster at close distance, but which
     * the monster can't get to, and a third one at a far distance that
     * the monster could get to - as it is, the monster won't look at that
     * third one.
     */
    if (!QUERY_FLAG(op, FLAG_FRIENDLY) && enemy == op->enemy) {
        object *nearest_player = get_nearest_player(op);

        if (nearest_player && nearest_player != enemy && !monster_can_hit(part, enemy, &rv)) {
            object_set_enemy(op, NULL);
            enemy = nearest_player;
        }
    }

    if (!QUERY_FLAG(op, FLAG_SCARED) && monster_can_hit(part, enemy, &rv)) {
        /* The adjustement to wc that was here before looked totally bogus -
         * since wc can in fact get negative, that would mean by adding
         * the current wc, the creature gets better?  Instead, just
         * add a fixed amount - nasty creatures that are runny away should
         * still be pretty nasty.
         */
        if (QUERY_FLAG(op, FLAG_RUN_AWAY)) {
            part->stats.wc += 10;
            skill_attack(enemy, part, 0, NULL, NULL);
            part->stats.wc -= 10;
        } else
            skill_attack(enemy, part, 0, NULL, NULL);
    } /* if monster is in attack range */

    if (QUERY_FLAG(part, FLAG_FREED))   /* Might be freed by ghost-attack or hit-back */
        return 1;

    if (QUERY_FLAG(op, FLAG_ONLY_ATTACK)) {
        object_remove(op);
        object_free_drop_inventory(op);
        return 1;
    }
    return 0;
}

/**
 * Checks if monster can hit in hand-to-hand combat. Multitile aware.
 *
 * @param ob1
 * monster trying to hit.
 * @param ob2
 * target to hit.
 * @param rv
 * vector from ob1 to ob2.
 * @return
 * 1 if ob1 is adjacent to ob2, 0 else.
 * @todo
 * rename to something more clear (is_adjacent?).
 */
static int monster_can_hit(object *ob1, object *ob2, rv_vector *rv) {
    object *more;
    rv_vector rv1;

    if (QUERY_FLAG(ob1, FLAG_CONFUSED) && !(RANDOM()%3))
        return 0;

    if (abs(rv->distance_x) < 2 && abs(rv->distance_y) < 2)
        return 1;

    /* check all the parts of ob2 - just because we can't get to
     * its head doesn't mean we don't want to pound its feet
     */
    for (more = ob2->more; more != NULL; more = more->more) {
        if (get_rangevector(ob1, more, &rv1, 0)
        && abs(rv1.distance_x) < 2 && abs(rv1.distance_y) < 2)
            return 1;
    }
    return 0;
}

/**
 * Checks if a monster should cast a spell.
 *
 * Note that this function does not check to see if the monster can
 * in fact cast the spell (sp dependencies and what not.)  That is because
 * this function is also sued to see if the monster should use spell items
 * (rod/horn/wand/scroll).
 *
 * Note that there are certainly other offensive spells that could be
 * included, but I decided to leave out the spells that may kill more
 * monsters than players (eg, disease).
 *
 * This could be a lot smarter - if there are few monsters around,
 * then disease might not be as bad. Likewise, if the monster is damaged,
 * the right type of healing spell could be useful.
 *
 * @param spell_ob
 * spell considered.
 * @return
 * 1 is monster should cast spell sp, 0 else.
 * @todo improve logic, take enemy into consideration.
 */
static int monster_should_cast_spell(object *spell_ob) {
    /* The caller is responsible for making sure that *spell_ob is defined. */
    assert(spell_ob != NULL);

    if (spell_ob->subtype == SP_BOLT
    || spell_ob->subtype == SP_BULLET
    || spell_ob->subtype == SP_EXPLOSION
    || spell_ob->subtype == SP_CONE
    || spell_ob->subtype == SP_BOMB
    || spell_ob->subtype == SP_SMITE
    || spell_ob->subtype == SP_MAGIC_MISSILE
    || spell_ob->subtype == SP_SUMMON_GOLEM
    || spell_ob->subtype == SP_MAGIC_WALL
    || spell_ob->subtype == SP_SUMMON_MONSTER
    || spell_ob->subtype == SP_MOVING_BALL
    || spell_ob->subtype == SP_SWARM
    || spell_ob->subtype == SP_INVISIBLE)
        return 1;

    return 0;
}

/** Maximum number of spells to consider when choosing a spell for a monster. */
#define MAX_KNOWN_SPELLS 20

/**
 * Selects a spell to cast for a monster.
 *
 * Returns a randomly selected spell.    This logic is still
 * less than ideal.  This code also only seems to deal with
 * wizard spells, as the check is against sp, and not grace.
 * can monsters know cleric spells?
 *
 * @param monster
 * monster trying to cast a spell.
 * @return
 * spell to cast, NULL if none suitable found.
 * @note
 * Will only consider the first MAX_KNOWN_SPELLS spells found.
 */
static object *monster_choose_random_spell(object *monster) {
    object *altern[MAX_KNOWN_SPELLS];
    int i = 0;

    FOR_INV_PREPARE(monster, tmp)
        if (tmp->type == SPELLBOOK || tmp->type == SPELL) {
            /* Check and see if it's actually a useful spell.
             * If its a spellbook, the spell is actually the inventory item.
             * if it is a spell, then it is just the object itself.
             */
            if (monster_should_cast_spell(tmp->type == SPELLBOOK ? tmp->inv : tmp)) {
                altern[i++] = tmp;
                if (i == MAX_KNOWN_SPELLS)
                    break;
            }
        }
    FOR_INV_FINISH();
    if (!i)
        return NULL;
    return altern[RANDOM()%i];
}

/**
 * Tries to make a (part of a) monster cast a spell.
 *
 * Handles sp/gr limits, and confusion.
 *
 * @param head
 * head of the monster.
 * @param part
 * part of the monster that we use to cast.
 * @param pl
 * target.
 * @param dir
 * direction to cast.
 * @return
 * 1 if monster casted a spell, 0 else.
 */
static int monster_cast_spell(object *head, object *part, object *pl, int dir) {
    object *spell_item;
    object *owner;
    rv_vector rv1;

    /* If you want monsters to cast spells over friends, this spell should
     * be removed.  It probably should be in most cases, since monsters still
     * don't care about residual effects (ie, casting a cone which may have a
     * clear path to the player, the side aspects of the code will still hit
     * other monsters)
     */
    dir = path_to_player(part, pl, 0);
    if (dir == 0)
        return 0;

    if (QUERY_FLAG(head, FLAG_FRIENDLY)) {
        owner = object_get_owner(head);
        if (owner != NULL) {
            if (get_rangevector(head, owner, &rv1, 0x1)
            && dirdiff(dir, rv1.direction) < 2) {
                return 0; /* Might hit owner with spell */
            }
        }
    }

    if (QUERY_FLAG(head, FLAG_CONFUSED))
        dir = get_randomized_dir(dir);

    /* If the monster hasn't already chosen a spell, choose one
     * I'm not sure if it really make sense to pre-select spells (events
     * could be different by the time the monster goes again).
     */
    if (head->spellitem == NULL) {
        spell_item = monster_choose_random_spell(head);
        if (spell_item == NULL) {
            LOG(llevMonster, "Turned off spells in %s\n", head->name);
            CLEAR_FLAG(head, FLAG_CAST_SPELL); /* Will be turned on when picking up book */
            return 0;
        }
        if (spell_item->type == SPELLBOOK) {
            if (!spell_item->inv) {
                LOG(llevError, "spellbook %s does not contain a spell?\n", spell_item->name);
                return 0;
            }
            spell_item = spell_item->inv;
        }
    } else
        spell_item = head->spellitem;

    if (!spell_item)
        return 0;

    /* Best guess this is a defensive/healing spell */
    if (spell_item->range <= 1 || spell_item->stats.dam < 0)
        dir = 0;

    /* Monster doesn't have enough spell-points */
    if (head->stats.sp < SP_level_spellpoint_cost(head, spell_item, SPELL_MANA))
        return 0;

    if (head->stats.grace < SP_level_spellpoint_cost(head, spell_item, SPELL_GRACE))
        return 0;

    head->stats.sp -= SP_level_spellpoint_cost(head, spell_item, SPELL_MANA);
    head->stats.grace -= SP_level_spellpoint_cost(head, spell_item, SPELL_GRACE);

    /* set this to null, so next time monster will choose something different */
    head->spellitem = NULL;

    return cast_spell(part, part, dir, spell_item, NULL);
}

/**
 * Tries to make a (part of a) monster apply a spell.
 *
 * @param head
 * head of the monster.
 * @param part
 * part of the monster that we use to cast.
 * @param pl
 * target.
 * @param dir
 * direction to cast.
 * @return
 * 1 if monster applied a scroll, 0 else.
 */
static int monster_use_scroll(object *head, object *part, object *pl, int dir) {
    object *scroll;
    object *owner;
    rv_vector rv1;

    /* If you want monsters to cast spells over friends, this spell should
     * be removed.  It probably should be in most cases, since monsters still
     * don't care about residual effects (ie, casting a cone which may have a
     * clear path to the player, the side aspects of the code will still hit
     * other monsters)
     */
    dir = path_to_player(part, pl, 0);
    if (dir == 0)
        return 0;

    if (QUERY_FLAG(head, FLAG_FRIENDLY)) {
        owner = object_get_owner(head);
        if (owner != NULL) {
            if (get_rangevector(head, owner, &rv1, 0x1)
            && dirdiff(dir, rv1.direction) < 2) {
                return 0; /* Might hit owner with spell */
            }
        }
    }

    if (QUERY_FLAG(head, FLAG_CONFUSED))
        dir = get_randomized_dir(dir);

    scroll = NULL;
    FOR_INV_PREPARE(head, tmp)
        if (tmp->type == SCROLL && monster_should_cast_spell(tmp->inv)) {
            scroll = tmp;
            break;
        }
    FOR_INV_FINISH();

    /* Used up all his scrolls, so nothing do to */
    if (!scroll) {
        CLEAR_FLAG(head, FLAG_READY_SCROLL);
        return 0;
    }

    /* Spell should be cast on caster (ie, heal, strength) */
    if (scroll->inv->range == 0)
        dir = 0;

    /* Face the direction that we want to cast. */
    head->direction = dir;
    head->facing = head->direction;
    if (head->animation_id)
        animate_object(head, head->direction);

    ob_apply(scroll, part, 0);
    return 1;
}

/**
 * A monster attempts using a skill.
 * Implemented 95-04-28 to allow monster skill use.
 * Note that monsters do not need the skills SK_MELEE_WEAPON and
 * SK_MISSILE_WEAPON to make those respective attacks, if we
 * required that we would drastically increase the memory
 * requirements of CF!!
 *
 * The skills we are treating here are all but those. -b.t.
 *
 * At the moment this is only useful for throwing, perhaps for
 * stealing. TODO: This should be more integrated in the game. -MT, 25.11.01
 *
 * Will switch between at most 2 skills.
 *
 * @param head
 * head of the monster.
 * @param part
 * part of the monster that may use a skill.
 * @param pl
 * target.
 * @param dir
 * direction to cast.
 * @return
 * 1 if monster used a skill, 0 else.
 * @todo
 * improve skill logic? Fix comments.
 */
static int monster_use_skill(object *head, object *part, object *pl, int dir) {
    object *owner;
    int found;

    dir = path_to_player(part, pl, 0);
    if (dir == 0)
        return 0;

    if (QUERY_FLAG(head, FLAG_FRIENDLY)) {
        owner = object_get_owner(head);
        if (owner != NULL) {
            rv_vector rv;

            if (get_rangevector(head, owner, &rv, 0) && dirdiff(dir, rv.direction) < 1)
                return 0; /* Might hit owner with skill -thrown rocks for example ?*/
        }
    }
    if (QUERY_FLAG(head, FLAG_CONFUSED))
        dir = get_randomized_dir(dir);

    /* skill selection - monster will use the next unused skill.
     * well...the following scenario will allow the monster to
     * toggle between 2 skills. One day it would be nice to make
     * more skills available to monsters.
     */
    found = 0;
    FOR_INV_PREPARE(head, skill)
        if (skill->type == SKILL && skill != head->chosen_skill) {
            head->chosen_skill = skill;
            found = 1;
            break;
        }
    FOR_INV_FINISH();

    if (!found && !head->chosen_skill) {
        LOG(llevDebug, "Error: Monster %s (%d) has FLAG_READY_SKILL without skill.\n", head->name, head->count);
        CLEAR_FLAG(head, FLAG_READY_SKILL);
        return 0;
    }
    /* use skill */
    return do_skill(head, part, head->chosen_skill, dir, NULL);
}

/**
 * Monster will use a ranged attack (ROD, WAND, ...).
 *
 * @param head
 * head of the monster.
 * @param part
 * part of the monster that can do a range attack.
 * @param pl
 * target.
 * @param dir
 * direction to fire.
 * @return
 * 1 if monster casted a spell, 0 else.
 */
static int monster_use_range(object *head, object *part, object *pl, int dir) {
    object *owner;
    int at_least_one = 0;

    dir = path_to_player(part, pl, 0);
    if (dir == 0)
        return 0;

    if (QUERY_FLAG(head, FLAG_FRIENDLY)) {
        owner = object_get_owner(head);
        if (owner != NULL) {
            rv_vector rv;

            if (get_rangevector(head, owner, &rv, 0) && dirdiff(dir, rv.direction) < 2)
                return 0; /* Might hit owner with spell */
        }
    }
    if (QUERY_FLAG(head, FLAG_CONFUSED))
        dir = get_randomized_dir(dir);

    FOR_INV_PREPARE(head, wand) {
        if (wand->type == WAND) {
            /* Found a wand, let's see if it has charges left */
            at_least_one = 1;
            if (wand->stats.food <= 0)
                continue;

            cast_spell(head, wand, dir, wand->inv, NULL);
            drain_wand_charge(wand);

            /* Success */
            return 1;
        }

        if (wand->type == ROD) {
            /* Found rod/horn, let's use it if possible */
            at_least_one = 1;
            if (wand->stats.hp < MAX(wand->inv->stats.sp, wand->inv->stats.grace))
                continue;

            /* drain charge before casting spell - can be a case where the
             * spell destroys the monster, and rod, so if done after, results
             * in crash.
             */
            drain_rod_charge(wand);
            cast_spell(head, wand, dir, wand->inv, NULL);

            /* Success */
            return 1;
        }
    } FOR_INV_FINISH();

    if (at_least_one)
        return 0;

    LOG(llevError, "Error: Monster %s (%d) HAS_READY_RANG() without wand/horn/rod.\n", head->name, head->count);
    CLEAR_FLAG(head, FLAG_READY_RANGE);
    return 0;
}

/**
 * Tries to make a (part of a) monster fire a bow.
 *
 * Handles confusion effect.
 *
 * @param head
 * head of the monster.
 * @param part
 * part of the monster that we use to cast.
 * @param pl
 * target.
 * @param dir
 * direction to cast.
 * @return
 * 1 if monster fired something, 0 else.
 */
static int monster_use_bow(object *head, object *part, object *pl, int dir) {
    object *owner;
    rv_vector rv;
    sint16 x, y;
    mapstruct *map;

    if (!get_rangevector(part, pl, &rv, 1))
        return 0;
    if (rv.distance > 100)
        /* Too far */
        return 0;
    if (rv.distance_x != 0 && rv.distance_y != 0 && abs(rv.distance_x) != abs(rv.distance_y))
        /* Player must be on same horizontal, vertical or diagonal line. */
        return 0;
    dir = rv.direction;

    if (QUERY_FLAG(head, FLAG_FRIENDLY))
        owner = object_get_owner(head);
    else
        owner = NULL;

    /* The monster can possibly fire, let's see if the path is ok for an arrow. */
    x = part->x;
    y = part->y;
    map = part->map;
    while (x != pl->x || y != pl->y || map != pl->map) {
        x += freearr_x[dir];
        y += freearr_y[dir];
        map = get_map_from_coord(map, &x, &y);
        if (!map) {
            LOG(llevError, "monster_use_bow: no map but still path exists??\n");
            return 0;
        }
        if ((GET_MAP_MOVE_BLOCK(map, x, y)&MOVE_FLY_LOW) == MOVE_FLY_LOW)
            return 0;
        if (owner && owner->x == x && owner->y == y && owner->map == map)
            /* Don't hit owner! */
            return 0;
    }

    /* Finally, path is clear, can fire. */

    if (QUERY_FLAG(head, FLAG_CONFUSED))
        dir = get_randomized_dir(dir);

    /* in server/player.c */
    return fire_bow(head, NULL, dir, 0, part->x, part->y);
}

/**
 * Returns the "quality" value of a weapon of a monster. Higher quality values
 * are considered better.
 *
 * @param item
 * the item to check
 * @return the quality value
 */
static int monster_get_weapon_quality(const object *item) {
    int val;
    int i;

    val = item->stats.dam;
    val += item->magic*3;
    /* Monsters don't really get benefits from things like regen rates
     * from items.  But the bonus for their stats are very important.
     */
    for (i = 0; i < NUM_STATS; i++)
        val += get_attr_value(&item->stats, i)*2;
    return val;
}

/**
 * Checks if using weapon 'item' would be better for 'who'.
 * This is a very simplistic check - also checking things
 * like speed and ac are also relevant.
 *
 * @param who
 * creature considering to apply item.
 * @param item
 * item to check.
 * @return
 * 1 if item is a better object, 0 else.
 */
static int monster_check_good_weapon(object *who, object *item) {
    object *other_weap;
    int val;

    other_weap = object_find_by_type_applied(who, item->type);
    if (other_weap == NULL) /* No other weapons */
        return 1;

    /* Rather than go through and apply the new one, and see if it is
     * better, just do some simple checks
     * Put some multipliers for things that hvae several effects,
     * eg, magic affects both damage and wc, so it has more weight
     */

    val = monster_get_weapon_quality(item)-monster_get_weapon_quality(other_weap);
    return val > 0;
}

/**
 * Returns the "quality" value of an armour of a monster. Higher quality values
 * are considered better.
 *
 * @param item
 * the item to check
 * @return the quality value
 */
static int monster_get_armour_quality(const object *item) {
    int val;

    val = item->stats.ac;
    val += item->resist[ATNR_PHYSICAL]/5;
    val += item->magic*3;
    return val;
}

/**
 * Checks if using armor 'item' would be better for 'who'.
 * This is a very simplistic check - also checking things
 * like speed and ac are also relevant.
 *
 * @param who
 * creature considering to apply item.
 * @param item
 * item to check.
 * @return
 * 1 if item is a better object, 0 else.
 */
static int monster_check_good_armour(object *who, object *item) {
    object *other_armour;
    int val, i;

    other_armour = object_find_by_type_applied(who, item->type);
    if (other_armour == NULL) /* No other armour, use the new */
        return 1;

    val = monster_get_armour_quality(item)-monster_get_armour_quality(other_armour);

    /* for the other protections, do weigh them very much in the equation -
     * it is the armor protection which is most important, because there is
     * no good way to know what the player may attack the monster with.
     * So if the new item has better protection than the old, give that higher
     * value.  If the reverse, then decrease the value of this item some.
     */
    for (i = 1; i < NROFATTACKS; i++) {
        if (item->resist[i] > other_armour->resist[i])
            val++;
        else if (item->resist[i] < other_armour->resist[i])
            val--;
    }

    /* Very few armours have stats, so not much need to worry about those. */

    return val > 0;
}

/**
 * Checks for items that monster can pick up.
 *
 * Vick's (vick(at)bern.docs.uu.se) fix 921030 for the sweeper blob.
 * Each time the blob passes over some treasure, it will
 * grab it a.s.a.p.
 *
 * Eneq((at)csd.uu.se): This can now be defined in the archetypes, added code
 * to handle this.
 *
 * This function was seen be continueing looping at one point (tmp->below
 * became a recursive loop.  It may be better to call monster_check_apply
 * after we pick everything up, since that function may call others which
 * affect stacking on this space.
 *
 * @param monster
 * monster that can pick up items.
 */

static void monster_check_pickup(object *monster) {
    object *part;

    for (part = monster; part != NULL; part = part->more)
        FOR_BELOW_PREPARE(part, tmp) {
            if (monster_can_pick(monster, tmp)) {
                uint32 nrof;

                if (tmp->weight > 0) {
                    sint32 weight_limit;

                    weight_limit = get_weight_limit(monster->stats.Str);
                    if (weight_limit >= monster->weight-monster->carrying)
                        nrof = (weight_limit-monster->weight-monster->carrying)/tmp->weight;
                    else
                        nrof = 0;
                } else
                    nrof = MAX(1, tmp->nrof);
                if (nrof >= 1) {
                    object *tmp2;

                    tmp2 = object_split(tmp, MIN(nrof, MAX(1, tmp->nrof)), NULL, 0);
                    tmp2 = object_insert_in_ob(tmp2, monster);
                    (void)monster_check_apply(monster, tmp2);
                }
            }
        } FOR_BELOW_FINISH();
}

/**
 * Check if the monster can and is interested in picking up
 * an item.
 * @param monster monster to check.
 * @param item what item to check against
 * @return
 * - 0 if item can or should not be picked by monster.
 * - 1 if monster picks item.
 */
static int monster_can_pick(object *monster, object *item) {
    int flag = 0;
    int i;

    if (!object_can_pick(monster, item))
        return 0;

    if (QUERY_FLAG(item, FLAG_UNPAID))
        return 0;

    if (monster->pick_up&64)           /* All */
        flag = 1;

    else {
        if (IS_WEAPON(item))
            flag = (monster->pick_up&8) || QUERY_FLAG(monster, FLAG_USE_WEAPON);
        else if (IS_ARMOR(item))
            flag = (monster->pick_up&16) || QUERY_FLAG(monster, FLAG_USE_ARMOUR);
        else if (IS_SHIELD(item))
            flag = (monster->pick_up&16) || QUERY_FLAG(monster, FLAG_USE_SHIELD);
        else switch (item->type) {
            case MONEY:
            case GEM:
                flag = monster->pick_up&2;
                break;

            case FOOD:
                flag = monster->pick_up&4;
                break;

            case SKILL:
                flag = QUERY_FLAG(monster, FLAG_CAN_USE_SKILL);
                break;

            case RING:
                flag = QUERY_FLAG(monster, FLAG_USE_RING);
                break;

            case WAND:
            case ROD:
                flag = QUERY_FLAG(monster, FLAG_USE_RANGE);
                break;

            case SPELLBOOK:
                flag = (monster->arch != NULL && QUERY_FLAG(&monster->arch->clone, FLAG_CAST_SPELL));
                break;

            case SCROLL:
                flag = QUERY_FLAG(monster, FLAG_USE_SCROLL);
                break;

            case BOW:
            case ARROW:
                flag = QUERY_FLAG(monster, FLAG_USE_BOW);
                break;
            }
        if (!flag && QUERY_FLAG(item, FLAG_IS_THROWN) && object_find_by_type_subtype(monster, SKILL, SK_THROWING) != NULL)
            flag = (monster->pick_up&8) || QUERY_FLAG(monster, FLAG_USE_WEAPON);
        /* Simplistic check - if the monster has a location to equip it, he will
         * pick it up.  Note that this doesn't handle cases where an item may
         * use several locations.
         */
        for (i = 0; i < NUM_BODY_LOCATIONS; i++) {
            if (monster->body_info[i] && item->body_info[i]) {
                flag = 1;
                break;
            }
        }
    }

    if (((!(monster->pick_up&32)) && flag) || ((monster->pick_up&32) && (!flag)))
        return 1;
    return 0;
}

/**
 * If a monster who's eager to apply things encounters something apply-able,
 * then make him apply it.
 * @author Vick's (vick@bern.docs.uu.se)
 * @date 921107
 */
static void monster_apply_below(object *monster) {
    FOR_BELOW_PREPARE(monster, tmp) {
        switch (tmp->type) {
        case CF_HANDLE:
        case TRIGGER:
            if (monster->will_apply&WILL_APPLY_HANDLE)
                apply_manual(monster, tmp, 0);
            break;

        case TREASURE:
            if (monster->will_apply&WILL_APPLY_TREASURE)
                apply_manual(monster, tmp, 0);
            break;
        }
        if (QUERY_FLAG(tmp, FLAG_IS_FLOOR))
            break;
    } FOR_BELOW_FINISH();
}

/**
 * Calls monster_check_apply() for all inventory objects.
 * @param monster the monster to operate on
 */
void monster_check_apply_all(object *monster) {
    FOR_INV_PREPARE(monster, inv)
        monster_check_apply(monster, inv);
    FOR_INV_FINISH();
}

/**
 * Called after an item is inserted in a monster.
 * Will look if item should be applied to replace another item.
 * @param mon monster who picked an item.
 * @param item what was picked up.
 * @note Sept 96, fixed this so skills will be readied -b.t.
 */
void monster_check_apply(object *mon, object *item) {
    int flag = 0;

    if (item->type == SPELLBOOK
    && mon->arch != NULL
    && (QUERY_FLAG(&mon->arch->clone, FLAG_CAST_SPELL))) {
        SET_FLAG(mon, FLAG_CAST_SPELL);
        return;
    }

    /* If for some reason, this item is already applied, no more work to do */
    if (QUERY_FLAG(item, FLAG_APPLIED))
        return;

    /* Might be better not to do this - if the monster can fire a bow,
     * it is possible in his wanderings, he will find one to use.  In
     * which case, it would be nice to have ammo for it.
     */
    if (QUERY_FLAG(mon, FLAG_USE_BOW) && item->type == ARROW) {
        /* Check for the right kind of bow */
        object *bow;

        bow = object_find_by_type_and_race(mon, BOW, item->race);
        if (bow != NULL) {
            SET_FLAG(mon, FLAG_READY_BOW);
            LOG(llevMonster, "Found correct bow for arrows.\n");
            return;     /* nothing more to do for arrows */
        }
    }

    if (item->type == TREASURE && mon->will_apply&WILL_APPLY_TREASURE)
        flag = 1;
    /* Eating food gets hp back */
    else if (item->type == FOOD && mon->will_apply&WILL_APPLY_FOOD)
        flag = 1;
    else if (item->type == SCROLL && QUERY_FLAG(mon, FLAG_USE_SCROLL)) {
        if (!item->inv)
            LOG(llevDebug, "Monster %d having scroll %d with empty inventory!\n", mon->count, item->count);
        else if (monster_should_cast_spell(item->inv))
            SET_FLAG(mon, FLAG_READY_SCROLL);
        /* Don't use it right now */
        return;
    } else if (item->type == WEAPON)
        flag = monster_check_good_weapon(mon, item);
    else if (IS_ARMOR(item) || IS_SHIELD(item))
        flag = monster_check_good_armour(mon, item);
    /* Should do something more, like make sure this is a better item */
    else if (item->type == RING)
        flag = 1;
    else if (item->type == WAND || item->type == ROD) {
        /* We never really 'ready' the wand/rod/horn, because that would mean the
        * weapon would get undone.
        */
        if (!(apply_can_apply_object(mon, item)&CAN_APPLY_NOT_MASK)) {
            SET_FLAG(mon, FLAG_READY_RANGE);
            SET_FLAG(item, FLAG_APPLIED);
        }
        return;
    } else if (item->type == BOW) {
        /* We never really 'ready' the bow, because that would mean the
        * weapon would get undone.
        */
        if (!(apply_can_apply_object(mon, item)&CAN_APPLY_NOT_MASK))
            SET_FLAG(mon, FLAG_READY_BOW);
        return;
    } else if (item->type == SKILL) {
        /*
         * skills are specials: monsters must have the 'FLAG_READY_SKILL' flag set,
         * else they can't use the skill...
         * Skills also don't need to get applied, so return now.
         */
        SET_FLAG(mon, FLAG_READY_SKILL);
        return;
    }

    /* if we don't match one of the above types, return now.
     * apply_can_apply_object() will say that we can apply things like flesh,
     * bolts, and whatever else, because it only checks against the
     * body_info locations.
     */
    if (!flag)
        return;

    /* Check to see if the monster can use this item.  If not, no need
     * to do further processing.  Note that apply_can_apply_object() already checks
     * for the CAN_USE flags.
     */
    if (apply_can_apply_object(mon, item)&CAN_APPLY_NOT_MASK)
        return;

    /* should only be applying this item, not unapplying it.
     * also, ignore status of curse so they can take off old armour.
     * monsters have some advantages after all.
     */
    apply_manual(mon, item, AP_APPLY|AP_IGNORE_CURSE|AP_NOPRINT);
    return;
}

/**
 * A monster calls for help against its enemy.
 * @param op monster calling for help.
 */
void monster_npc_call_help(object *op) {
    int x, y, mflags;
    sint16 sx, sy;
    mapstruct *m;

    for (x = -3; x < 4; x++)
        for (y = -3; y < 4; y++) {
            m = op->map;
            sx = op->x+x;
            sy = op->y+y;
            mflags = get_map_flags(m, &m, sx, sy, &sx, &sy);
            /* If nothing alive on this space, no need to search the space. */
            if ((mflags&P_OUT_OF_MAP) || !(mflags&P_IS_ALIVE))
                continue;

            FOR_MAP_PREPARE(m, sx, sy, npc)
                if (QUERY_FLAG(npc, FLAG_ALIVE) && QUERY_FLAG(npc, FLAG_UNAGGRESSIVE))
                    object_set_enemy(npc, op->enemy);
            FOR_MAP_FINISH();
        }
}

/**
 * Return the direction the monster should move or look to attack an enemy.
 * @param dir direction the monster is currently facing.
 * @param enemy target of the monster.
 * @param part monster's part we're considering.
 * @param rv vector to enemy.
 * @return direction to go into.
 */
static int monster_dist_att(int dir, object *enemy, object *part, rv_vector *rv) {
    if (monster_can_hit(part, enemy, rv))
        return dir;
    if (rv->distance < 10)
        return absdir(dir+4);
    else if (rv->distance > 18)
        return dir;
    return 0;
}

/**
 * Return the next direction the monster should move for a specific movement type.
 * @param dir direction the monster is currently facing.
 * @param ob unused.
 * @param enemy target of the monster.
 * @param part monster's part we're considering.
 * @param rv vector to enemy.
 * @return direction to go into.
 */
static int monster_run_att(int dir, object *ob, object *enemy, object *part, rv_vector *rv) {
    if ((monster_can_hit(part, enemy, rv) && ob->move_status < 20) || ob->move_status < 20) {
        ob->move_status++;
        return (dir);
    } else if (ob->move_status > 20)
        ob->move_status = 0;
    return absdir(dir+4);
}

/**
 * Return the next direction the monster should move for a specific movement type.
 * @param dir direction the monster is currently facing.
 * @param ob unused.
 * @param enemy target of the monster.
 * @return direction to go into.
 */
static int monster_hitrun_att(int dir, object *ob, object *enemy) {
    if (ob->move_status++ < 25)
        return dir;
    else if (ob->move_status < 50)
        return absdir(dir+4);
    ob->move_status = 0;
    return absdir(dir+4);
}

/**
 * Return the next direction the monster should move for a specific movement type.
 * @param dir direction the monster is currently facing.
 * @param ob unused.
 * @param enemy target of the monster.
 * @param part monster's part we're considering.
 * @param rv vector to enemy.
 * @return direction to go into.
 */
static int monster_wait_att(int dir, object *ob, object *enemy, object *part, rv_vector *rv) {
    int inrange = monster_can_hit(part, enemy, rv);

    if (ob->move_status || inrange)
        ob->move_status++;

    if (ob->move_status == 0)
        return 0;
    else if (ob->move_status < 10)
        return dir;
    else if (ob->move_status < 15)
        return absdir(dir+4);
    ob->move_status = 0;
    return 0;
}

/**
 * Return the next direction the monster should move for a specific movement type.
 * @param dir direction the monster is currently facing.
 * @param ob unused.
 * @param enemy target of the monster.
 * @param part monster's part we're considering.
 * @param rv vector to enemy.
 * @return direction to go into.
 */
static int monster_disthit_att(int dir, object *ob, object *enemy, object *part, rv_vector *rv) {
    /* The logic below here looked plain wrong before.  Basically, what should
     * happen is that if the creatures hp percentage falls below run_away,
     * the creature should run away (dir+4)
     * I think its wrong for a creature to have a zero maxhp value, but
     * at least one map has this set, and whatever the map contains, the
     * server should try to be resilant enough to avoid the problem
     */
    if (ob->stats.maxhp && (ob->stats.hp*100)/ob->stats.maxhp < ob->run_away)
        return absdir(dir+4);
    return monster_dist_att(dir, enemy, part, rv);
}

/**
 * Return the next direction the monster should move for a specific movement type.
 * @param dir direction the monster is currently facing.
 * @param rv vector to enemy.
 * @return direction to go into.
 */
static int monster_wait_att2(int dir, rv_vector *rv) {
    if (rv->distance < 9)
        return absdir(dir+4);
    return 0;
}

/**
 * Move the monster in a specified movement pattern.
 * @param ob monster.
 */
static void monster_circ1_move(object *ob) {
    static const int circle [12] = { 3, 3, 4, 5, 5, 6, 7, 7, 8, 1, 1, 2 };

    if (++ob->move_status > 11)
        ob->move_status = 0;
    if (!(move_object(ob, circle[ob->move_status])))
        (void)move_object(ob, RANDOM()%8+1);
}

/**
 * Move the monster in a specified movement pattern.
 * @param ob monster.
 */
static void monster_circ2_move(object *ob) {
    static const int circle[20] = { 3, 3, 3, 4, 4, 5, 5, 5, 6, 6, 7, 7, 7, 8, 8, 1, 1, 1, 2, 2 };

    if (++ob->move_status > 19)
        ob->move_status = 0;
    if (!(move_object(ob, circle[ob->move_status])))
        (void)move_object(ob, RANDOM()%8+1);
}

/**
 * Move the monster in a specified movement pattern.
 * @param ob monster.
 */
static void monster_pace_movev(object *ob) {
    if (ob->move_status++ > 6)
        ob->move_status = 0;
    if (ob->move_status < 4)
        (void)move_object(ob, 5);
    else
        (void)move_object(ob, 1);
}

/**
 * Move the monster in a specified movement pattern.
 * @param ob monster.
 */
static void monster_pace_moveh(object *ob) {
    if (ob->move_status++ > 6)
        ob->move_status = 0;
    if (ob->move_status < 4)
        (void)move_object(ob, 3);
    else
        (void)move_object(ob, 7);
}

/**
 * Move the monster in a specified movement pattern.
 * @param ob monster.
 */
static void monster_pace2_movev(object *ob) {
    if (ob->move_status++ > 16)
        ob->move_status = 0;
    if (ob->move_status < 6)
        (void)move_object(ob, 5);
    else if (ob->move_status < 8)
        return;
    else if (ob->move_status < 13)
        (void)move_object(ob, 1);
}

/**
 * Move the monster in a specified movement pattern.
 * @param ob monster.
 */
static void monster_pace2_moveh(object *ob) {
    if (ob->move_status++ > 16)
        ob->move_status = 0;
    if (ob->move_status < 6)
        (void)move_object(ob, 3);
    else if (ob->move_status < 8)
        return;
    else if (ob->move_status < 13)
        (void)move_object(ob, 7);
}

/**
 * Move the monster in a specified movement pattern.
 * @param ob monster.
 */
static void monster_rand_move(object *ob) {
    int i;

    if (ob->move_status < 1
    || ob->move_status > 8
    || !(move_object(ob, ob->move_status || !(RANDOM()%9))))
        for (i = 0; i < 5; i++) {
            ob->move_status = RANDOM()%8+1;
            if (move_object(ob, ob->move_status))
                return;
        }
}

/**
 * Living creature attempts to hit an earthwall.
 * @param op creature to consider.
 * @param m map to consider.
 * @param x coordinate.
 * @param y coordinate.
 */
void monster_check_earthwalls(object *op, mapstruct *m, int x, int y) {
    FOR_MAP_PREPARE(m, x, y, tmp)
        if (tmp->type == EARTHWALL) {
            hit_player(tmp, op->stats.dam, op, AT_PHYSICAL, 1);
            return;
        }
    FOR_MAP_FINISH();
}

/**
 * Living creature attempts to open a door.
 * @param op creature to consider.
 * @param m map to consider.
 * @param x coordinate.
 * @param y coordinate.
 */
void monster_check_doors(object *op, mapstruct *m, int x, int y) {
    FOR_MAP_PREPARE(m, x, y, tmp)
        if (tmp->type == DOOR) {
            hit_player(tmp, op->stats.dam, op, AT_PHYSICAL, 1);
            return;
        }
    FOR_MAP_FINISH();
}

/**
 * Output a NPC message on a map.
 * @param map where to talk to.
 * @param message what to say.
 */
void monster_do_say(const mapstruct *map, const char *message) {
    ext_info_map(NDI_NAVY|NDI_UNIQUE, map, MSG_TYPE_DIALOG, MSG_TYPE_DIALOG_NPC,
                 message);
}

/**
 * Format an NPC message.
 * @param npc who is talking.
 * @param message what is being said.
 * @return new StringBuffer containing the text.
 */
static StringBuffer *monster_format_say(const object* npc, const char *message) {
    char name[MAX_BUF];
    StringBuffer *buf;

    query_name(npc, name, sizeof(name));
    buf = stringbuffer_new();
    stringbuffer_append_printf(buf, "%s says: %s", name, message);
    return buf;
}

/**
 * Return the verb for the player's dialog type.
 * @param rt dialog type.
 * @return verb.
 */
static const char *get_reply_text_own(reply_type rt) {
    switch (rt) {
        case rt_say:
            return "say";
        case rt_reply:
            return "reply";
        case rt_question:
            return "ask";
    }
    assert(0);
    return NULL;
}

/**
 * Return the verb for the player's dialog type seen from others (third person).
 * @param rt dialog type.
 * @return verb.
 */
static const char *get_reply_text_other(reply_type rt) {
    switch (rt) {
        case rt_say:
            return "says";
        case rt_reply:
            return "replies";
        case rt_question:
            return "asks";
    }
    assert(0);
    return NULL;
}

/**
 * This function looks for an object or creature that is listening to said text.
 *
 * The process is such:
 * - first, build up information on NPCs reacting to what is said, replies
 * - second, figure what the player will be displayed as said
 * - third, have the player actually talk
 * - fourth, the NPCs talk too
 * - fifth: show the player available replies
 *
 * There is a rare event that the orig_map is used for - basically, if
 * a player says the magic word that gets him teleported off the map,
 * it can result in the new map putting the object count too high,
 * which forces the swap out of some other map.  In some cases, the
 * map the player was just on now gets swapped out - thus, the
 * object on that map are no longer in memory.  So check to see if the
 * players map changes, and if so, don't process any further.
 * If it does change, most likely we don't care about the results
 * of further conversation.  Also, depending on the value of i,
 * the conversation would continue on the new map, which probably isn't
 * what is really wanted either.
 *
 * @param op who is saying something.
 * @param txt what is said.
 */
void monster_communicate(object *op, const char *txt) {
    int i, mflags;
    sint16 x, y;
    mapstruct *mp, *orig_map = op->map;
    char own[MAX_BUF], others[MAX_BUF];
    talk_info info;

    info.text = txt;
    info.message = NULL;
    info.replies_count = 0;
    info.who = op;
    info.npc_msg_count = 0;

    /* Note that this loop looks pretty inefficient to me - we look and try to talk
     * to every object within 2 spaces.  It would seem that if we trim this down to
     * only try to talk to objects with npc->msg set, things would be a lot more efficient,
     * but I'm not sure if there are any objects out there that don't have a message and instead
     * rely sorely on events - MSW 2009-04-14
     */
    for (i = 0; i <= SIZEOFFREE2; i++) {
        mp = op->map;
        x = op->x+freearr_x[i];
        y = op->y+freearr_y[i];

        mflags = get_map_flags(mp, &mp, x, y, &x, &y);
        if (mflags&P_OUT_OF_MAP)
            continue;

        FOR_MAP_PREPARE(mp, x, y, npc) {
            monster_talk_to_npc(npc, &info);
            if (orig_map != op->map) {
                LOG(llevDebug, "Warning: Forced to swap out very recent map - MAX_OBJECTS should probably be increased\n");
                return;
            }
        } FOR_MAP_FINISH();
    }

    /* First, what the player says. */
    if (info.message != NULL) {
        snprintf(own, sizeof(own), "You %s: %s", get_reply_text_own(info.message_type), info.message);
        snprintf(others, sizeof(others), "%s %s: %s", op->name, get_reply_text_other(info.message_type), info.message);
        free_string(info.message);
    } else {
        snprintf(own, sizeof(own), "You say: %s", txt);
        snprintf(others, sizeof(others), "%s says: %s", op->name, txt);
    }
    draw_ext_info(NDI_WHITE, 0, op, MSG_TYPE_COMMUNICATION, MSG_TYPE_COMMUNICATION_SAY, own);
    ext_info_map_except(NDI_WHITE, op->map, op, MSG_TYPE_COMMUNICATION, MSG_TYPE_COMMUNICATION_SAY, others);

    /* Then NPCs can actually talk. */
    for (i = 0; i < info.npc_msg_count; i++) {
        monster_do_say(orig_map, info.npc_msgs[i]);
        free_string(info.npc_msgs[i]);
    }

    /* Finally, the replies the player can use. */
    if (info.replies_count > 0) {
        draw_ext_info(NDI_WHITE, 0, op, MSG_TYPE_COMMUNICATION, MSG_TYPE_COMMUNICATION_SAY, "Replies:");
        for (i = 0; i < info.replies_count; i++) {
            draw_ext_info_format(NDI_WHITE, 0, op, MSG_TYPE_COMMUNICATION, MSG_TYPE_COMMUNICATION_SAY, " - %s: %s", info.replies_words[i], info.replies[i]);
            free_string(info.replies_words[i]);
            free_string(info.replies[i]);
        }
    }
}

/**
 * Checks the messages of a NPC for a matching text. Will not call
 * plugin events. Called by monster_talk_to_npc().
 *
 * @param npc object that gets a chance to reply.
 * @param info message information.
 * @return 1 if npc talked, 0 else.
 */
static int monster_do_talk_npc(object *npc, talk_info *info) {
    struct_dialog_reply *reply;
    struct_dialog_message *message;

    if (!get_dialog_message(npc, info->text, &message, &reply))
        return 0;

    if (reply) {
        info->message = add_string(reply->message);
        info->message_type = reply->type;
    }

    if (npc->type == MAGIC_EAR) {
        ext_info_map(NDI_NAVY|NDI_UNIQUE, npc->map, MSG_TYPE_DIALOG, MSG_TYPE_DIALOG_MAGIC_EAR, message->message);
        use_trigger(npc);
    } else {
        char value[2];

        if (info->npc_msg_count < MAX_NPC) {
            info->npc_msgs[info->npc_msg_count] = stringbuffer_finish_shared(monster_format_say(npc, message->message));
            info->npc_msg_count++;
        }

        /* mark that the npc was talked to, so it won't move randomly later on */
        value[0] = '3' + rand() % 6;
        value[1] = '\0';
        object_set_value(npc, "talked_to", value, 1);

        reply = message->replies;
        while (reply && info->replies_count < MAX_REPLIES) {
            info->replies[info->replies_count] = add_string(reply->message);
            info->replies_words[info->replies_count] = add_string(reply->reply);
            info->replies_count++;
            reply = reply->next;
        }
    }

    return 1;
}

/**
 * Simple function to have some NPC say something.
 * @param npc who should say something.
 * @param cp what is being said.
 */
void monster_npc_say(object *npc, const char *cp) {
    char *message;
    StringBuffer *buf = monster_format_say(npc, cp);

    message = stringbuffer_finish(buf);
    monster_do_say(npc->map, message);
    free(message);
}

/**
 * Give an object the chance to handle something being said.
 * Plugin hooks will be called, including in the NPC's inventory.
 *
 * @param npc object to try to talk to. Can be an NPC or a MAGIC_EAR.
 * @param info message information.
 * @return 0 if text was handled by a plugin or not handled, 1 if handled internally by the server.
 */
static int monster_talk_to_npc(object *npc, talk_info *info) {
    /* Move this commone area up here - shouldn't cost much extra cpu
     * time, and makes the function more readable */
    /* Lauwenmark: Handle for plugin say event */
    if (plugin_event_say(npc, info) != 0)
        return 0;
    /* Lauwenmark - Here we let the objects inside inventories hear and answer, too. */
    /* This allows the existence of "intelligent" weapons you can discuss with */
    FOR_INV_PREPARE(npc, cobj)
        if (plugin_event_say(cobj, info) != 0)
            return 0;
    FOR_INV_FINISH();
    if (info->who == npc)
        return 0;
    return monster_do_talk_npc(npc, info);
}

/**
 * Find an item for the monster to throw.
 * Modeled on find_throw_ob().
 * This is probably overly simplistic as it is now - We want
 * monsters to throw things like chairs and other pieces of
 * furniture, even if they are not good throwable objects.
 * Probably better to have the monster throw a throwable object
 * first, then throw any non equipped weapon.
 *
 * @param op monster to find an item to throw for.
 * @return item, NULL if none suitable.
 */
object *monster_find_throw_ob(object *op) {
    /* New throw code: look through the inventory. Grap the first legal is_thrown
     * marked item and throw it to the enemy.
     */

    FOR_INV_PREPARE(op, tmp) {
        /* Can't throw invisible objects or items that are applied */
        if (!tmp->invisible && !QUERY_FLAG(tmp, FLAG_APPLIED) && QUERY_FLAG(tmp, FLAG_IS_THROWN)) {
#ifdef DEBUG_THROW
            char what[MAX_BUF];

            query_name(tmp, what, MAX_BUF);
            LOG(llevDebug, "%s chooses to throw: %s (%d)\n", op->name, what, tmp->count);
#endif
            return tmp;
        }
    } FOR_INV_FINISH();

#ifdef DEBUG_THROW
    LOG(llevDebug, "%s chooses to throw nothing\n", op->name);
#endif
    return NULL;
}

/**
 * Determine if we can 'detect' the enemy. Check for walls blocking the
 * los. Also, just because its hidden/invisible, we may be sensitive/smart
 * enough (based on Wis & Int) to figure out where the enemy is. -b.t.
 *
 * Modified by MSW to use the get_rangevector so that map tiling works
 * properly.  I also so odd code in place that checked for x distance
 * OR y distance being within some range - that seemed wrong - both should
 * be within the valid range. MSW 2001-08-05
 *
 * @param op who should detect.
 * @param enemy what to detect.
 * @param rv if the function returns 1, contains the range vector towards enemy.
 * @return 0 if enemy can not be detected, 1 if it is detected
 */
int monster_can_detect_enemy(object *op, object *enemy, rv_vector *rv) {
    int radius = MIN_MON_RADIUS, hide_discovery;

    /* null detection for any of these condtions always */
    if (!op || !enemy || !op->map || !enemy->map)
        return 0;

    /* If the monster (op) has no way to get to the enemy, do nothing */
    if (!get_rangevector(op, enemy, rv, 0))
        return 0;

    /* Monsters always ignore the DM */
    if (op->type != PLAYER && QUERY_FLAG(enemy, FLAG_WIZ))
        return 0;

    /* simple check.  Should probably put some range checks in here. */
    if (monster_can_see_enemy(op, enemy))
        return 1;

    /* The rest of this is for monsters. Players are on their own for
     * finding enemies!
     */
    if (op->type == PLAYER)
        return 0;

    /* Quality invisible? Bah, we wont see them w/o SEE_INVISIBLE
     * flag (which was already checked) in can_see_enmy (). Lets get out of here
     */
    if (enemy->invisible && (!enemy->contr || (!enemy->contr->tmp_invis && !enemy->contr->hidden)))
        return 0;

    /* use this for invis also */
    hide_discovery = op->stats.Int/5;

    /* Determine Detection radii */
    if (!enemy->hide) /* to detect non-hidden (eg dark/invis enemy) */
        radius = MAX(op->stats.Wis/5+1, MIN_MON_RADIUS);
    else { /* a level/INT/Dex adjustment for hiding */
        object *sk_hide;
        int bonus = (op->level/2)+(op->stats.Int/5);

        if (enemy->type == PLAYER) {
            sk_hide = find_skill_by_number(enemy, SK_HIDING);
            if (sk_hide != NULL)
                bonus -= sk_hide->level;
            else {
                LOG(llevError, "monster_can_detect_enemy() got hidden player w/o hiding skill!\n");
                make_visible(enemy);
                radius = MAX(radius, MIN_MON_RADIUS);
            }
        } else /* enemy is not a player */
            bonus -= enemy->level;

        radius += bonus/5;
        hide_discovery += bonus*5;
    } /* else creature has modifiers for hiding */

    /* Radii stealth adjustment. Only if you are stealthy
    * will you be able to sneak up closer to creatures */
    if (QUERY_FLAG(enemy, FLAG_STEALTH))
        radius = radius/2, hide_discovery = hide_discovery/3;

    /* Radii adjustment for enemy standing in the dark */
    if (op->map->darkness > 0 && !monster_stand_in_light(enemy)) {
        /* on dark maps body heat can help indicate location with infravision
         * undead don't have body heat, so no benefit detecting them.
         */
        if (QUERY_FLAG(op, FLAG_SEE_IN_DARK) && !is_true_undead(enemy))
            radius += op->map->darkness/2;
        else
            radius -= op->map->darkness/2;

        /* op next to a monster (and not in complete darkness)
         * the monster should have a chance to see you.
         */
        if (radius < MIN_MON_RADIUS && op->map->darkness < 5 && rv->distance <= 1)
            radius = MIN_MON_RADIUS;
    } /* if on dark map */

    /* Lets not worry about monsters that have incredible detection
     * radii, we only need to worry here about things the player can
     * (potentially) see.  This is 13, as that is the maximum size the player
     * may have for their map - in that way, creatures at the edge will
     * do something.  Note that the distance field in the
     * vector is real distance, so in theory this should be 18 to
     * find that.
     */
    if (radius > 13)
        radius = 13;

    /* Enemy in range! Now test for detection */
    if ((int)rv->distance <= radius) {
        /* ah, we are within range, detected? take cases */
        if (!enemy->invisible) /* enemy in dark squares... are seen! */
            return 1;

        /* hidden or low-quality invisible */
        if (enemy->hide && rv->distance <= 1 && RANDOM()%100 <= hide_discovery) {
            make_visible(enemy);
            /* inform players of new status */
            if (enemy->type == PLAYER && player_can_view(enemy, op))
                draw_ext_info_format(NDI_UNIQUE, 0, enemy, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                                     "You are discovered by %s!",
                                     op->name);
            return 1; /* detected enemy */
        } else if (enemy->invisible) {
            /* Change this around - instead of negating the invisible, just
             * return true so that the mosnter that managed to detect you can
             * do something to you.  Decreasing the duration of invisible
             * doesn't make a lot of sense IMO, as a bunch of stupid creatures
             * can then basically negate the spell.  The spell isn't negated -
             * they just know where you are!
             */
            if (RANDOM()%50 <= hide_discovery) {
                if (enemy->type == PLAYER) {
                    char name[MAX_BUF];

                    query_name(op, name, MAX_BUF);
                    draw_ext_info_format(NDI_UNIQUE, 0, enemy, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                                         "You see %s noticing your position.",
                                         name);
                }
                return 1;
            }
        }
    } /* within range */

    /* Wasn't detected above, so still hidden */
    return 0;
}

/**
 * Determine if op stands in a lighted square. This is not a very
 * intellegent algorithm. For one thing, we ignore los here, SO it
 * is possible for a bright light to illuminate a player on the
 * other side of a wall (!).
 *
 * @param op who to check.
 * @return 1 if op is in lighe, 0 else.
 */
int monster_stand_in_light(object *op) {
    sint16 nx, ny;
    mapstruct *m;

    if (!op)
        return 0;
    if (op->glow_radius > 0)
        return 1;

    if (op->map) {
        int x, y, x1, y1;

        /* Check the spaces with the max light radius to see if any of them
         * have lights, and if any of them light the player enough, then return 1.
         */
        for (x = op->x-MAX_LIGHT_RADII; x <= op->x+MAX_LIGHT_RADII; x++) {
            for (y = op->y-MAX_LIGHT_RADII; y <= op->y+MAX_LIGHT_RADII; y++) {
                m = op->map;
                nx = x;
                ny = y;

                if (get_map_flags(m, &m, nx, ny, &nx, &ny)&P_OUT_OF_MAP)
                    continue;

                x1 = abs(x-op->x)*abs(x-op->x);
                y1 = abs(y-op->y)*abs(y-op->y);
                if (isqrt(x1+y1) < GET_MAP_LIGHT(m, nx, ny))
                    return 1;
            }
        }
    }
    return 0;
}

/**
 * Assuming no walls/barriers, lets check to see if its *possible*
 * to see an enemy. Note, "detection" is different from "seeing".
 * See monster_can_detect_enemy() for more details. -b.t.
 *
 * @param op who is trying to see enemy.
 * @param enemy victim op is trying to see.
 * @return 0 if can't be seen, 1 if can be
 */
int monster_can_see_enemy(object *op, object *enemy) {
    object *looker = HEAD(op);

    /* safety */
    if (!looker || !enemy || !QUERY_FLAG(looker, FLAG_ALIVE))
        return 0;

    /* we dont give a full treatment of xrays here (shorter range than normal,
     * see through walls). Should we change the code elsewhere to make you
     * blind even if you can xray?
     */
    if (QUERY_FLAG(looker, FLAG_BLIND) && !QUERY_FLAG(looker, FLAG_XRAYS))
        return 0;

    /* checking for invisible things */
    if (enemy->invisible) {
        /* HIDDEN ENEMY. by definition, you can't see hidden stuff!
         * However, if you carry any source of light, then the hidden
         * creature is seeable (and stupid) */
        if (has_carried_lights(enemy)) {
            if (enemy->hide) {
                make_visible(enemy);
                draw_ext_info(NDI_UNIQUE, 0, enemy, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                              "Your light reveals your hiding spot!");
            }
            return 1;
        } else if (enemy->hide)
            return 0;

        /* Invisible enemy.  Break apart the check for invis undead/invis looker
         * into more simple checks - the QUERY_FLAG doesn't return 1/0 values,
         * and making it a conditional makes the code pretty ugly.
         */
        if (!QUERY_FLAG(looker, FLAG_SEE_INVISIBLE)) {
            if (makes_invisible_to(enemy, looker))
                return 0;
        }
    } else if (looker->type == PLAYER) /* for players, a (possible) shortcut */
        if (player_can_view(looker, enemy))
            return 1;

    /* ENEMY IN DARK MAP. Without infravision, the enemy is not seen
     * unless they carry a light or stand in light. Darkness doesnt
     * inhibit the undead per se (but we should give their archs
     * CAN_SEE_IN_DARK, this is just a safety
     * we care about the enemy maps status, not the looker.
     * only relevant for tiled maps, but it is possible that the
     * enemy is on a bright map and the looker on a dark - in that
     * case, the looker can still see the enemy
     */
    if (enemy->map->darkness > 0
    && !monster_stand_in_light(enemy)
    && (!QUERY_FLAG(looker, FLAG_SEE_IN_DARK) || !is_true_undead(looker) || !QUERY_FLAG(looker, FLAG_XRAYS)))
        return 0;

    return 1;
}
