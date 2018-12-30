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
 * Those functions deal with pets.
 */

#include <global.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#endif

/**
 * Mark all inventory items as FLAG_NO_DROP.
 *
 * @param ob
 * the object to modify.
 */
static void mark_inventory_as_no_drop(object *ob) {
    FOR_INV_PREPARE(ob, tmp)
        SET_FLAG(tmp, FLAG_NO_DROP);
    FOR_INV_FINISH();
}

/**
 * Given that 'pet' is a friendly object, this function returns a
 * monster the pet should attack, NULL if nothing appropriate is
 * found.  it basically looks for nasty things around the owner
 * of the pet to attack.
 * This is now tilemap aware.
 *
 * @param pet
 * who is seeking an enemy.
 * @param[out] rv
 * will contain the path to the enemy.
 * @return
 * enemy, or NULL if nothing suitable.
 *
 * @note
 * pet_get_enemy() has been renamed to pets_get_enemy()
 */
object *pets_get_enemy(object *pet, rv_vector *rv) {
    object *owner, *tmp, *attacker, *tmp3;
    int i;
    sint16 x, y;
    mapstruct *nm;
    int search_arr[SIZEOFFREE];
    int mflags;

    attacker = pet->attacked_by; /*pointer to attacking enemy*/
    pet->attacked_by = NULL;     /*clear this, since we are dealing with it*/

    owner = object_get_owner(pet);
    if (owner != NULL) {
        /* If the owner has turned on the pet, make the pet
         * unfriendly.
         */
        if (monster_check_enemy(owner, rv) == pet) {
            CLEAR_FLAG(pet, FLAG_FRIENDLY);
            remove_friendly_object(pet);
            pet->attack_movement &= ~PETMOVE;
            return owner;
        }
    } else {
        /* else the owner is no longer around, so the
         * pet no longer needs to be friendly.
         */
        CLEAR_FLAG(pet, FLAG_FRIENDLY);
        remove_friendly_object(pet);
        pet->attack_movement &= ~PETMOVE;
        return NULL;
    }
    /* If they are not on the same map, the pet won't be agressive */
    if (!on_same_map(pet, owner))
        return NULL;

    /* See if the pet has an existing enemy. If so, don't start a new one*/
    tmp = monster_check_enemy(pet, rv);
    if (tmp != NULL) {
        if (tmp != owner || QUERY_FLAG(pet, FLAG_CONFUSED) || !QUERY_FLAG(pet, FLAG_FRIENDLY))
            return tmp;

        /* without this check, you can actually get pets with
         * enemy set to owner!
         */
        object_set_enemy(pet, NULL);
    }
    get_search_arr(search_arr);

    if (owner->type == PLAYER && owner->contr->petmode == pet_sad) {
        tmp = monster_find_nearest_living_creature(pet);
        if (tmp != NULL && get_rangevector(pet, tmp, rv, 0) && monster_check_enemy(pet, rv) != NULL)
            return tmp;
        /* if we got here we have no enemy */
        /* we return NULL to avoid heading back to the owner */
        object_set_enemy(pet, NULL);
        return NULL;
    }

    /* Since the pet has no existing enemy, look for anything nasty
     * around the owner that it should go and attack.
     */
    tmp3 = NULL;
    for (i = 0; i < SIZEOFFREE; i++) {
        x = owner->x+freearr_x[search_arr[i]];
        y = owner->y+freearr_y[search_arr[i]];
        nm = owner->map;
        /* Only look on the space if there is something alive there. */
        mflags = get_map_flags(nm, &nm, x, y, &x, &y);
        if (!(mflags&P_OUT_OF_MAP) && mflags&P_IS_ALIVE) {
            FOR_MAP_PREPARE(nm, x, y, tmp) {
                object *tmp2 = HEAD(tmp);

                if (QUERY_FLAG(tmp2, FLAG_ALIVE)
                && ((!QUERY_FLAG(tmp2, FLAG_FRIENDLY) && tmp2->type != PLAYER) || pets_should_arena_attack(pet, owner, tmp2))
                && !QUERY_FLAG(tmp2, FLAG_UNAGGRESSIVE)
                && tmp2 != pet
                && tmp2 != owner
                && monster_can_detect_enemy(pet, tmp2, rv)) {
                    if (!monster_can_see_enemy(pet, tmp2)) {
                        if (tmp3 != NULL)
                            tmp3 = tmp2;
                    } else {
                        object_set_enemy(pet, tmp2);
                        if (monster_check_enemy(pet, rv) != NULL)
                            return tmp2;
                        object_set_enemy(pet, NULL);
                    }
                }/* if this is a valid enemy */
            } FOR_MAP_FINISH();/* for objects on this space */
        }/* if there is something living on this space */
    } /* for loop of spaces around the owner */

    /* fine, we went through the whole loop and didn't find one we could
       see, take what we have */
    if (tmp3 != NULL) {
        object_set_enemy(pet, tmp3);
        if (monster_check_enemy(pet, rv) != NULL)
            return tmp3;
        object_set_enemy(pet, NULL);
    }

    /* No threat to owner, check to see if the pet has an attacker*/
    if (attacker != NULL) {
        /* need to be sure this is the right one! */
        if (attacker->count == pet->attacked_by_count) {
            /* also need to check to make sure it is not freindly */
            /* or otherwise non-hostile, and is an appropriate target */
            if (!QUERY_FLAG(attacker, FLAG_FRIENDLY) && on_same_map(pet, attacker)) {
                object_set_enemy(pet, attacker);
                if (monster_check_enemy(pet, rv) != NULL)
                    return attacker;
                object_set_enemy(pet, NULL);
            }
        }
    }

    /* Don't have an attacker or legal enemy, so look for a new one!.
     * This looks for one around where the pet is.  Thus, you could lead
     * a pet to danger, then take a few steps back.  This code is basically
     * the same as the code that looks around the owner.
     */
    if (owner->type == PLAYER && owner->contr->petmode != pet_defend) {
        tmp3 = NULL;
        for (i = 0; i < SIZEOFFREE; i++) {
            x = pet->x+freearr_x[search_arr[i]];
            y = pet->y+freearr_y[search_arr[i]];
            nm = pet->map;
            /* Only look on the space if there is something alive there. */
            mflags = get_map_flags(nm, &nm, x, y, &x, &y);
            if (!(mflags&P_OUT_OF_MAP) && mflags&P_IS_ALIVE) {
                FOR_MAP_PREPARE(nm, x, y, tmp) {
                    object *tmp2 = HEAD(tmp);
                    if (QUERY_FLAG(tmp2, FLAG_ALIVE)
                    && ((!QUERY_FLAG(tmp2, FLAG_FRIENDLY) && tmp2->type != PLAYER) || pets_should_arena_attack(pet, owner, tmp2))
                    && !QUERY_FLAG(tmp2, FLAG_UNAGGRESSIVE)
                    && tmp2 != pet
                    && tmp2 != owner
                    && monster_can_detect_enemy(pet, tmp2, rv)) {
                        if (!monster_can_see_enemy(pet, tmp2)) {
                            if (tmp3 != NULL)
                                tmp3 = tmp2;
                        } else {
                            object_set_enemy(pet, tmp2);
                            if (monster_check_enemy(pet, rv) != NULL)
                                return tmp2;
                            object_set_enemy(pet, NULL);
                        }
                    } /* make sure we can get to the bugger */
                } FOR_MAP_FINISH();/* for objects on this space */
            } /* if there is something living on this space */
        } /* for loop of spaces around the pet */
    } /* pet in defence mode */

    /* fine, we went through the whole loop and didn't find one we could
       see, take what we have */
    if (tmp3 != NULL) {
        object_set_enemy(pet, tmp3);
        if (monster_check_enemy(pet, rv) != NULL)
            return tmp3;
        object_set_enemy(pet, NULL);
    }

    /* Didn't find anything - return the owner's enemy or NULL */
    return monster_check_enemy(pet, rv);
}

/**
 * Removes all pets someone owns.
 *
 * @param owner
 * player we wish to remove all pets of.
 *
 * @note
 * terminate_all_pets() has been renamed to pets_terminate_all()
 */
void pets_terminate_all(object *owner) {
    objectlink *obl, *next;

    for (obl = first_friendly_object; obl != NULL; obl = next) {
        object *ob = obl->ob;
        next = obl->next;
        if (object_get_owner(ob) == owner) {
            if (!QUERY_FLAG(ob, FLAG_REMOVED))
                object_remove(ob);
            remove_friendly_object(ob);
            object_free_drop_inventory(ob);
        }
    }
}

/**
 * This function checks all pets so they try to follow their master around the world.
 *
 * Unfortunately, sometimes, the owner of a pet is in the
 * process of entering a new map when this is called.
 * Thus the map isn't loaded yet, and we have to remove
 * the pet...
 *
 * @note
 * remove_all_pets() has been renamed to pets_remove_all()
 */
void pets_remove_all(void) {
    objectlink *obl, *next;
    object *owner;

    for (obl = first_friendly_object; obl != NULL; obl = next) {
        next = obl->next;
        if (obl->ob->type != PLAYER
        && QUERY_FLAG(obl->ob, FLAG_FRIENDLY)
        && (owner = object_get_owner(obl->ob)) != NULL
        && !on_same_map(owner, obl->ob)) {
            /* follow owner checks map status for us.  Note that pet can
             * die in pets_follow_owner(), so check for obl->ob existence
             */
            pets_follow_owner(obl->ob, owner);
            if (obl->ob && QUERY_FLAG(obl->ob, FLAG_REMOVED) && FABS(obl->ob->speed) > MIN_ACTIVE_SPEED) {
                object *ob = obl->ob;

                LOG(llevMonster, "(pet failed to follow)\n");
                remove_friendly_object(ob);
                object_free_drop_inventory(ob);
            }
        }
    }
}

/**
 * A pet is trying to follow its owner.
 *
 * @param ob
 * pet trying to follow. Will be object_remove()'d if can't follow.
 * @param owner
 * owner of ob.
 *
 * @note
 * follow_owner() has been renamed to pets_follow_owner()
 */
void pets_follow_owner(object *ob, object *owner) {
    int dir;

    if (!QUERY_FLAG(ob, FLAG_REMOVED))
        object_remove(ob);

    if (owner->map == NULL) {
        LOG(llevError, "Can't follow owner: no map.\n");
        return;
    }
    if (owner->map->in_memory != MAP_IN_MEMORY) {
        LOG(llevError, "Owner of the pet not on a map in memory!?\n");
        return;
    }
    dir = object_find_free_spot(ob, owner->map, owner->x, owner->y, 1, SIZEOFFREE);

    if (dir == -1) {
        LOG(llevMonster, "No space for pet to follow, freeing %s.\n", ob->name);
        return; /* Will be freed since it's removed */
    }
    object_insert_in_map_at(ob, owner->map, NULL, 0, owner->x+freearr_x[dir], owner->y+freearr_y[dir]);
    if (owner->type == PLAYER) /* Uh, I hope this is always true... */
        draw_ext_info(NDI_UNIQUE, 0, owner, MSG_TYPE_SPELL, MSG_TYPE_SPELL_PET,
                      "Your pet magically appears next to you");
    return;
}

/**
 * Handles a pet's movement.
 *
 * @param ob
 * pet to move.
 *
 * @note
 * pet_move() has been renamed to pets_move()
 */
void pets_move(object *ob) {
    int dir, i;
    tag_t tag;
    sint16 dx, dy;
    object *owner;
    mapstruct *m;

    /* Check to see if player pulled out */
    owner = object_get_owner(ob);
    if (owner == NULL) {
        object_remove(ob); /* Will be freed when returning */
        remove_friendly_object(ob);
        object_free_drop_inventory(ob);
        LOG(llevMonster, "Pet: no owner, leaving.\n");
        return;
    }

    /* move monster into the owners map if not in the same map */
    if (!on_same_map(ob, owner)) {
        pets_follow_owner(ob, owner);
        return;
    }
    /* Calculate Direction */
    if (owner->type == PLAYER && owner->contr->petmode == pet_sad) {
        /* in S&D mode, if we have no enemy, run randomly about. */
        for (i = 0; i < 15; i++) {
            dir = get_random_dir();
            dx = ob->x+freearr_x[dir];
            dy = ob->y+freearr_y[dir];
            m = ob->map;
            if (!(get_map_flags(ob->map, &m, dx, dy, &dx, &dy)&P_OUT_OF_MAP)
            && !OB_TYPE_MOVE_BLOCK(ob, GET_MAP_MOVE_BLOCK(m, dx, dy)))
                break;
        }
    } else {
        rv_vector rv;

        if (get_rangevector(ob, owner, &rv, 0))
            dir = rv.direction;
        else
            dir = get_random_dir();
    }
    ob->direction = dir;

    tag = ob->count;
    /* move_ob returns 0 if the object couldn't move.  If that is the
     * case, lets do some other work.
     */
    if (!move_ob(ob, dir, ob)) {
        object *part;

        /* the failed move_ob above may destroy the pet, so check here */
        if (object_was_destroyed(ob, tag))
            return;

        for (part = ob; part != NULL; part = part->more) {
            dx = part->x+freearr_x[dir];
            dy = part->y+freearr_y[dir];
            m = get_map_from_coord(part->map, &dx, &dy);
            if (m == NULL)
                continue;

            FOR_MAP_PREPARE(m, dx, dy, ob2) {
                object *new_ob;

                new_ob = HEAD(ob2);
                if (new_ob == ob)
                    break;
                if (new_ob == owner)
                    return;
                if (object_get_owner(new_ob) == owner)
                    break;

                /* Hmm.  Did we try to move into an enemy monster?  If so,
                 * make it our enemy.
                 */
                if (QUERY_FLAG(new_ob, FLAG_ALIVE)
                && !QUERY_FLAG(ob, FLAG_UNAGGRESSIVE)
                && !QUERY_FLAG(new_ob, FLAG_UNAGGRESSIVE)
                && !QUERY_FLAG(new_ob, FLAG_FRIENDLY)) {
                    object_set_enemy(ob, new_ob);
                    if (new_ob->enemy == NULL)
                        object_set_enemy(new_ob, ob);
                    return;
                } else if (new_ob->type == PLAYER) {
                    draw_ext_info(NDI_UNIQUE, 0, new_ob,
                                  MSG_TYPE_MISC, MSG_SUBTYPE_NONE,
                                  "You stand in the way of someones pet.");
                    return;
                }
            } FOR_MAP_FINISH();
        }
        /* Try a different course */
        dir = absdir(dir+4-(RANDOM()%5)-(RANDOM()%5));
        (void)move_ob(ob, dir, ob);
    }
    return;
}

/****************************************************************************
 *
 * GOLEM SPELL CODE
 *
 ****************************************************************************/

/**
 * This makes multisquare/single square monsters
 * proper for map insertion.
 * @param at
 * archetype to prepare.
 * @param op
 * caster of the spell
 * @param dir
 * direction the monster should be placed in.
 * @return
 * suitable golem.
 */
static object *fix_summon_pet(archetype *at, object *op, int dir) {
    archetype *atmp;
    object *tmp = NULL, *prev = NULL, *head = NULL;

    for (atmp = at; atmp != NULL; atmp = atmp->more) {
        tmp = arch_to_object(atmp);
        if (atmp == at) {

            /* Ensure the golem can actually move if no move_type defined.
             * This check is redundant since this is checked at server startup. */
            if (tmp->move_type == 0) {
                LOG(llevError, "summoned %s [%s] is without move_type!\n", tmp->name, atmp->name);
                tmp->move_type = MOVE_WALK;
            }

            object_set_owner(tmp, op);
            if (op->type == PLAYER) {
                tmp->stats.exp = 0;
                add_friendly_object(tmp);
                SET_FLAG(tmp, FLAG_FRIENDLY);
                CLEAR_FLAG(tmp, FLAG_MONSTER);
            } else if (QUERY_FLAG(op, FLAG_FRIENDLY)) {
                object *owner = object_get_owner(op);

                if (owner != NULL) {/* For now, we transfer ownership */
                    object_set_owner(tmp, owner);
                    tmp->attack_movement = PETMOVE;
                    add_friendly_object(tmp);
                    SET_FLAG(tmp, FLAG_FRIENDLY);
                }
            }
            if (op->type != PLAYER) {
                tmp->attack_movement = PETMOVE;
                tmp->speed_left = -1;
                tmp->type = 0;
                object_set_enemy(tmp, op->enemy);
            } else
                tmp->type = GOLEM;
        }
        if (head == NULL)
            head = tmp;
        tmp->x = op->x+freearr_x[dir]+tmp->arch->clone.x;
        tmp->y = op->y+freearr_y[dir]+tmp->arch->clone.y;
        tmp->map = op->map;
        if (tmp->invisible)
            tmp->invisible = 0;
        if (head != tmp)
            tmp->head = head,
            prev->more = tmp;
        prev = tmp;
    }
    head->direction = dir;

    if (head->randomitems) {
        create_treasure(head->randomitems, head, GT_STARTEQUIP, 6, 0);
        if (QUERY_FLAG(head, FLAG_MONSTER)) {
            monster_check_apply_all(head);
        }
    }
    mark_inventory_as_no_drop(head);

    /* need to change some monster attr to prevent problems/crashing */
    head->last_heal = 0;
    head->last_eat = 0;
    head->last_grace = 0;
    head->last_sp = 0;
    head->other_arch = NULL;
    head->stats.exp = 0;
    CLEAR_FLAG(head, FLAG_CHANGING);
    CLEAR_FLAG(head, FLAG_STAND_STILL);
    CLEAR_FLAG(head, FLAG_GENERATOR);
    CLEAR_FLAG(head, FLAG_SPLITTING);
    if (head->attacktype&AT_GHOSTHIT)
        head->attacktype = (AT_PHYSICAL|AT_DRAIN);

    return head;
}

/**
 * Handles a golem's movement.
 *
 * Updated this to allow more than the golem 'head' to attack.
 * @param op
 * golem to be moved.
 *
 * @note
 * move_golem() has been renamed to pets_move_golem()
 */
void pets_move_golem(object *op) {
    int made_attack = 0;
    object *tmp;
    tag_t tag;
    object *owner;

    if (QUERY_FLAG(op, FLAG_MONSTER))
        return; /* Has already been moved */

    owner = object_get_owner(op);
    if (owner == NULL) {
        LOG(llevDebug, "Golem without owner destructed.\n");
        object_remove(op);
        object_free_drop_inventory(op);
        return;
    }
    /* It would be nice to have a cleaner way of what message to print
     * when the golem expires than these hard coded entries.
     * Note it is intentional that a golems duration is based on its
     * hp, and not duration
     */
    if (--op->stats.hp < 0) {
        if (op->msg != NULL)
            draw_ext_info(NDI_UNIQUE, 0, owner, MSG_TYPE_SPELL, MSG_TYPE_SPELL_PET,
                          op->msg);
        owner->contr->ranges[range_golem] = NULL;
        owner->contr->golem_count = 0;
        remove_friendly_object(op);
        object_remove(op);
        object_free_drop_inventory(op);
        return;
    }

    /* Do golem attacks/movement for single & multisq golems.
     * Assuming here that op is the 'head' object. Pass only op to
     * move_ob (makes recursive calls to other parts)
     * move_ob returns 0 if the creature was not able to move.
     */
    tag = op->count;
    if (move_ob(op, op->direction, op))
        return;
    if (object_was_destroyed(op, tag))
        return;

    for (tmp = op; tmp; tmp = tmp->more) {
        sint16 x = tmp->x+DIRX(op), y = tmp->y+DIRY(op);
        object *victim;
        mapstruct *m;
        int mflags;

        m = op->map;
        mflags = get_map_flags(m, &m, x, y, &x, &y);

        if (mflags&P_OUT_OF_MAP)
            continue;

        victim = NULL;
        FOR_MAP_PREPARE(op->map, x, y, tmp)
            if (QUERY_FLAG(tmp, FLAG_ALIVE)) {
                victim = tmp;
                break;
            }
        FOR_MAP_FINISH();

        /* We used to call will_hit_self to make sure we don't
         * hit ourselves, but that didn't work, and I don't really
         * know if that was more efficient anyways than this.
         * This at least works.  Note that victim->head can be NULL,
         * but since we are not trying to dereferance that pointer,
         * that isn't a problem.
         */
        if (victim != NULL && victim != op && victim->head != op) {
            /* for golems with race fields, we don't attack
             * aligned races
             */

            if (victim->race != NULL && op->race != NULL && strstr(op->race, victim->race)) {
                if (owner != NULL)
                    draw_ext_info_format(NDI_UNIQUE, 0, owner,
                                         MSG_TYPE_SPELL, MSG_TYPE_SPELL_PET,
                                         "%s avoids damaging %s.",
                                         op->name, victim->name);
            } else if (victim == owner) {
                if (owner != NULL)
                    draw_ext_info_format(NDI_UNIQUE, 0, owner,
                                         MSG_TYPE_SPELL, MSG_TYPE_SPELL_PET,
                                         "%s avoids damaging you.",
                                         op->name);
            } else {
                attack_ob(victim, op);
                made_attack = 1;
            }
        } /* If victim */
    }
    if (made_attack)
        object_update(op, UP_OBJ_FACE);
}

/**
 * Makes the golem go in specified direction.
 * This is a really stupid function when you get down and
 * look at it.  Keep it here for the time being - makes life
 * easier if we ever decide to do more interesting thing with
 * controlled golems.
 *
 * @param op
 * golem.
 * @param dir
 * desired direction.
 * @todo trash.
 *
 * @note
 * control_golem() has been renamed to pets_control_golem()
 */
void pets_control_golem(object *op, int dir) {
    op->direction = dir;
}

/**
 * Summons a monster.
 *
 * @param op
 * who is summoning.
 * @param caster
 * object casting the spell.
 * @param dir
 * direction to place the monster.
 * @param spob
 * spell object casting. At this stage, all spob is really used for is to
 * adjust some values in the monster.
 * @retval 0
 * failed to summon something.
 * @retval 1
 * summoned correctly something.
 *
 * @note
 * summon_golem() has been renamed to pets_summon_golem()
 */
int pets_summon_golem(object *op, object *caster, int dir, object *spob) {
    object *tmp;
    const object *god = NULL;
    archetype *at;
    char buf[MAX_BUF];

    /* Because there can be different golem spells, player may want to
     * 'lose' their old golem.
     */
    if (op->type == PLAYER
    && op->contr->ranges[range_golem] != NULL
    && op->contr->golem_count == op->contr->ranges[range_golem]->count) {
        draw_ext_info(NDI_UNIQUE, 0, op,
                      MSG_TYPE_SPELL, MSG_TYPE_SPELL_PET,
                      "You dismiss your existing golem.");
        object_remove(op->contr->ranges[range_golem]);
        object_free_drop_inventory(op->contr->ranges[range_golem]);
        op->contr->ranges[range_golem] = NULL;
        op->contr->golem_count = -1;
    }

    if (spob->other_arch != NULL)
        at = spob->other_arch;
    else if (spob->race != NULL) {
        god = find_god(determine_god(caster));
        if (god == NULL) {
            draw_ext_info_format(NDI_UNIQUE, 0, op,
                                 MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                                 "You must worship a god to cast %s.",
                                 spob->name);
            return 0;
        }

        at = determine_holy_arch(god, spob->race);
        if (at == NULL) {
            draw_ext_info_format(NDI_UNIQUE, 0, op,
                                 MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                                 "%s has no %s for you to call.",
                                 god->name, spob->race);
            return 0;
        }
    } else {
        LOG(llevError, "Spell %s lacks other_arch\n", spob->name);
        return 0;
    }

    if (!dir)
        dir = object_find_free_spot(NULL, op->map, op->x, op->y, 1, SIZEOFFREE1+1);

    if (dir == -1
    || ob_blocked(&at->clone, op->map, op->x+freearr_x[dir], op->y+freearr_y[dir])) {
        draw_ext_info(NDI_UNIQUE, 0, op,
                      MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                      "There is something in the way.");
        return 0;
    }
    /* basically want to get proper map/coordinates for this object */
    tmp = fix_summon_pet(at, op, dir);
    if (tmp == NULL) {
        draw_ext_info(NDI_UNIQUE, 0, op,
                      MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                      "Your spell fails.");
        return 0;
    }

    if (op->type == PLAYER) {
        tmp->type = GOLEM;
        object_set_owner(tmp, op);
        set_spell_skill(op, caster, spob, tmp);
        op->contr->ranges[range_golem] = tmp;
        op->contr->golem_count = tmp->count;
        /* give the player control of the golem */
        op->contr->shoottype = range_golem;
    } else {
        if (QUERY_FLAG(op, FLAG_FRIENDLY)) {
            object *owner = object_get_owner(op);

            if (owner != NULL) { /* For now, we transfer ownership */
                object_set_owner(tmp, owner);
                tmp->attack_movement = PETMOVE;
                add_friendly_object(tmp);
                set_spell_skill(op, caster, spob, tmp);
                SET_FLAG(tmp, FLAG_FRIENDLY);
            }
        }
        SET_FLAG(tmp, FLAG_MONSTER);
    }

    /* make the speed positive.*/
    tmp->speed = FABS(tmp->speed);

    /*  This sets the level dependencies on dam and hp for monsters */
    /* players can't cope with too strong summonings. */
    /* but monsters can.  reserve these for players. */
    if (op->type == PLAYER) {
        tmp->stats.hp += spob->duration+SP_level_duration_adjust(caster, spob);
        tmp->stats.maxhp = tmp->stats.hp;
        if (!spob->stats.dam)
            tmp->stats.dam += SP_level_dam_adjust(caster, spob);
        else
            tmp->stats.dam = spob->stats.dam+SP_level_dam_adjust(caster, spob);
        tmp->speed += .02*SP_level_range_adjust(caster, spob);
        tmp->speed = MIN(tmp->speed, 1.0);
        if (spob->attacktype)
            tmp->attacktype = spob->attacktype;
    }
    tmp->stats.wc -= SP_level_wc_adjust(caster, spob);

    /* limit the speed to 0.3 for non-players, 1 for players. */

    /* make experience increase in proportion to the strength.
     * this is a bit simplistic - we are basically just looking at how
     * often the sp doubles and use that as the ratio.
     */
    tmp->stats.exp *= 1+(MAX(spob->stats.maxgrace, spob->stats.sp)/caster_level(caster, spob));
    tmp->speed_left = 0;
    tmp->direction = dir;

    /* Holy spell - some additional tailoring */
    if (god != NULL) {
        object *tmp2;

        snprintf(buf, sizeof(buf), "%s of %s", spob->name, god->name);
        buf[0] = toupper(buf[0]);
        for (tmp2 = tmp; tmp2; tmp2 = tmp2->more) {
            if (tmp2->name != NULL)
                free_string(tmp2->name);
            tmp2->name = add_string(buf);
        }
        tmp->attacktype |= god->attacktype;
        memcpy(tmp->resist, god->resist, sizeof(tmp->resist));
        if (tmp->race != NULL)
            FREE_AND_CLEAR_STR(tmp->race);
        if (god->race != NULL)
            tmp->race = add_string(god->race);
        if (tmp->slaying != NULL)
            FREE_AND_CLEAR_STR(tmp->slaying);
        if (god->slaying != NULL)
            tmp->slaying = add_string(god->slaying);
        /* safety, we must allow a god's servants some reasonable attack */
        if (!(tmp->attacktype&AT_PHYSICAL))
            tmp->attacktype |= AT_PHYSICAL;
    }

    object_insert_in_map_at(tmp, tmp->map, op, 0, tmp->x, tmp->y);
    return 1;
}

/***************************************************************************
 *
 * Summon monster/pet/other object code
 *
 ***************************************************************************/

/**
 * Returns a monster (chosen at random) that this particular player (and his
 * god) find acceptable. This checks level, races allowed by god, etc
 * to determine what is acceptable.
 *
 * @param pl
 * player summoning.
 * @param god
 * god the player worships.
 * @param summon_level
 * summoning level.
 * @return
 * suitable monster, or NULL if no match found.
 */
static object *choose_cult_monster(object *pl, const object *god, int summon_level) {
    char buf[MAX_BUF];
    const char *race;
    int racenr, mon_nr, i;
    racelink *list;
    objectlink *tobl;
    object *otmp;

    /* Determine the number of races available */
    racenr = 0;
    strcpy(buf, god->race);
    race = strtok(buf, ",");
    while (race) {
        racenr++;
        race = strtok(NULL, ",");
    }

    /* next, randomly select a race from the aligned_races string */
    if (racenr > 1) {
        racenr = rndm(0, racenr-1);
        strcpy(buf, god->race);
        race = strtok(buf, ",");
        for (i = 0; i < racenr; i++)
            race = strtok(NULL, ",");
    } else
        race = god->race;


    /* see if a we can match a race list of monsters.  This should not
     * happen, so graceful recovery isn't really needed, but this sanity
     * checking is good for cases where the god archetypes mismatch the
     * race file
     */
    list = find_racelink(race);
    if (list == NULL) {
        draw_ext_info_format(NDI_UNIQUE, 0, pl,
                             MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                             "The spell fails! %s's creatures are beyond the range of your summons",
                             god->name);
        LOG(llevDebug, "choose_cult_monster() requested non-existent aligned race!\n");
        return NULL;
    }

    /* search for an apprplritate monster on this race list */
    mon_nr = 0;
    for (tobl = list->member; tobl; tobl = tobl->next) {
        otmp = tobl->ob;
        if (otmp == NULL || !QUERY_FLAG(otmp, FLAG_MONSTER))
            continue;
        if (otmp->level <= summon_level)
            mon_nr++;
    }

    /* If this god has multiple race entries, we should really choose another.
     * But then we either need to track which ones we have tried, or just
     * make so many calls to this function, and if we get so many without
     * a valid entry, assuming nothing is available and quit.
     */
    if (!mon_nr)
        return NULL;

    mon_nr = rndm(0, mon_nr-1);
    for (tobl = list->member; tobl; tobl = tobl->next) {
        otmp = tobl->ob;
        if (otmp == NULL || !QUERY_FLAG(otmp, FLAG_MONSTER))
            continue;
        if (otmp->level <= summon_level && !mon_nr--)
            return otmp;
    }
    /* This should not happen */
    LOG(llevDebug, "choose_cult_monster() mon_nr was set, but did not find a monster\n");
    return NULL;
}

/**
 * General purpose summoning function.
 *
 * @param op
 * who is summoning.
 * @param caster
 * what object did cast the summoning spell.
 * @param spell_ob
 * actual spell object for summoning.
 * @param dir
 * direction to summon in.
 * @param stringarg
 * additional parameters.
 * @retval 0
 * nothing was summoned.
 * @retval 1
 * something was summoned.
 *
 * @note
 * summon_object() has been renamed to pets_summon_object()
 */
int pets_summon_object(object *op, object *caster, object *spell_ob, int dir, const char *stringarg) {
    sint16 x, y, nrof = 1, i;
    archetype *summon_arch;
    int ndir, mult;

    if (spell_ob->other_arch != NULL) {
        summon_arch = spell_ob->other_arch;
    } else if (spell_ob->randomitems != NULL) {
        int level = caster_level(caster, spell_ob);
        treasure *tr, *lasttr = NULL;

        /* In old code, this was a very convuluted for statement,
         * with all the checks in the 'for' portion itself.  Much
         * more readable to break some of the conditions out.
         */
        for (tr = spell_ob->randomitems->items; tr; tr = tr->next) {
            if (level < tr->magic)
                break;
            lasttr = tr;
            if (stringarg != NULL && !strcmp(tr->item->name, stringarg))
                break;
            if (tr->next == NULL || tr->next->item == NULL)
                break;
        }
        if (lasttr == NULL) {
            LOG(llevError, "Treasurelist %s did not generate a valid entry in pets_summon_object\n", spell_ob->randomitems->name);
            draw_ext_info(NDI_UNIQUE, 0, op,
                          MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                          "The spell fails to summon any monsters.");
            return 0;
        }
        summon_arch = lasttr->item;
        nrof = lasttr->nrof;
    } else if (spell_ob->race != NULL && !strcmp(spell_ob->race, "GODCULTMON")) {
        const object *god = find_god(determine_god(op));
        object *mon, *owner;
        int summon_level, tries;

        if (god == NULL) {
            owner = object_get_owner(op);
            if (owner != NULL) {
                god = find_god(determine_god(owner));
            }
        }
        /* If we can't find a god, can't get what monster to summon */
        if (god == NULL)
            return 0;

        if (god->race == NULL) {
            draw_ext_info_format(NDI_UNIQUE, 0, op,
                                 MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                                 "%s has no creatures that you may summon!",
                                 god->name);
            return 0;
        }
        /* the summon level */
        summon_level = caster_level(caster, spell_ob);
        if (summon_level == 0)
            summon_level = 1;
        tries = 0;
        do {
            mon = choose_cult_monster(op, god, summon_level);
            if (mon == NULL) {
                draw_ext_info_format(NDI_UNIQUE, 0, op,
                                     MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                                     "%s fails to send anything.",
                                     god->name);
                return 0;
            }
            ndir = dir;
            if (!ndir)
                ndir = object_find_free_spot(mon, op->map, op->x, op->y, 1, SIZEOFFREE);
            if (ndir == -1
            || ob_blocked(mon, op->map, op->x+freearr_x[ndir], op->y+freearr_y[ndir])) {
                ndir = -1;
                if (++tries == 5) {
                    draw_ext_info(NDI_UNIQUE, 0, op,
                                  MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                                  "There is something in the way.");
                    return 0;
                }
            }
        } while (ndir == -1);
        if (mon->level > summon_level/2)
            nrof = random_roll(1, 2, op, PREFER_HIGH);
        else
            nrof = die_roll(2, 2, op, PREFER_HIGH);
        summon_arch = mon->arch;
    } else {
        summon_arch = NULL;
    }

    if (spell_ob->stats.dam)
        nrof += spell_ob->stats.dam+SP_level_dam_adjust(caster, spell_ob);

    if (summon_arch == NULL) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                      "There is no monsters available for summoning.");
        return 0;
    }

    if (dir) {
        /* Only fail if caster specified a blocked direction. */
        x = freearr_x[dir];
        y = freearr_y[dir];
        if (ob_blocked(&summon_arch->clone, op->map, op->x+x, op->y+y)) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                          "There is something in the way.");
            return 0;
        }
    }

    mult = (RANDOM()%2 ? -1 : 1);

    for (i = 1; i <= nrof; i++) {
        archetype *atmp;
        object *prev = NULL, *head = NULL, *tmp;

        if (dir) {
            ndir = absdir(dir+(i/2)*mult);
            mult = -mult;
        } else
            ndir = object_find_free_spot(&summon_arch->clone, op->map, op->x, op->y, 1, SIZEOFFREE);

        x = ndir > 0 ? freearr_x[ndir] : 0;
        y = ndir > 0 ? freearr_y[ndir] : 0;
        if (ndir == -1 || ob_blocked(&summon_arch->clone, op->map, op->x+x, op->y+y))
            continue;

        for (atmp = summon_arch; atmp != NULL; atmp = atmp->more) {
            tmp = arch_to_object(atmp);
            if (atmp == summon_arch) {
                if (QUERY_FLAG(tmp, FLAG_MONSTER)) {
                    object_set_owner(tmp, op);
                    set_spell_skill(op, caster, spell_ob, tmp);
                    object_set_enemy(tmp, op->enemy);
                    tmp->type = 0;
                    CLEAR_FLAG(tmp, FLAG_SLEEP);
                    if (op->type == PLAYER || QUERY_FLAG(op, FLAG_FRIENDLY)) {
                        /* If this is not set, we make it friendly */
                        if (!QUERY_FLAG(spell_ob, FLAG_MONSTER)) {
                            SET_FLAG(tmp, FLAG_FRIENDLY);
                            add_friendly_object(tmp);
                            tmp->stats.exp = 0;
                            if (spell_ob->attack_movement)
                                tmp->attack_movement = spell_ob->attack_movement;
                            if (object_get_owner(op) != NULL)
                                object_set_owner(tmp, object_get_owner(op));
                        }
                    }
                }
                if (tmp->speed > MIN_ACTIVE_SPEED)
                    tmp->speed_left = -1;
            }
            if (head == NULL)
                head = tmp;
            else {
                tmp->head = head;
                prev->more = tmp;
            }
            prev = tmp;
        }
        head->direction = freedir[ndir];
        head->stats.exp = 0;
        head = object_insert_in_map_at(head, op->map, op, 0, op->x+x, op->y+y);
        if (head != NULL && head->randomitems) {
            create_treasure(head->randomitems, head, GT_STARTEQUIP, 6, 0);
            if (QUERY_FLAG(head, FLAG_MONSTER)) {
                monster_check_apply_all(head);
            }
        }
        if (head != NULL) {
            mark_inventory_as_no_drop(head);
        }
    } /* for i < nrof */
    return 1;
}

/**
 * Recursively look through the owner property of objects until the real owner
 * is found
 *
 * @param ob
 * item we're searching the owner of.
 * @return
 * owner, NULL if nothing found.
 */
static object *get_real_owner(object *ob) {
    object *realowner = ob;

    if (realowner == NULL)
        return NULL;

    while (object_get_owner(realowner) != NULL) {
        realowner = object_get_owner(realowner);
    }
    return realowner;
}

/**
 * Determines if checks so pets don't attack players or other pets should be
 * overruled by the arena petmode.
 *
 * @param pet
 * pet considered.
 * @param owner
 * pet's owner.
 * @param target
 * potential pet target.
 * @retval 0
 * pet shouldn't attack target.
 * @retval 1
 * target is a suitable victim for the pet.
 *
 * @note
 * should_arena_attack() has been renamed to pets_should_arena_attack()
 */
int pets_should_arena_attack(object *pet, object *owner, object *target) {
    object *rowner, *towner;

    /* exit if the target, pet, or owner is null. */
    if (target == NULL || pet == NULL || owner == NULL)
        return 0;

    /* get the owners of itself and the target, this is to deal with pets of
    pets */
    rowner = get_real_owner(owner);
    if (target->type != PLAYER) {
        towner = get_real_owner(target);
    } else {
        towner = NULL;
    }

    /* if the pet has no owner, exit with error */
    if (rowner == NULL) {
        LOG(llevError, "Pet has no owner.\n");
        return 0;
    }

    /* if the target is not a player, and has no owner, we shouldn't be here
    */
    if (towner == NULL && target->type != PLAYER) {
        LOG(llevError, "Target is not a player but has no owner. We should not be here.\n");
        return 0;
    }

    /* make sure that the owner is a player */
    if (rowner->type != PLAYER)
        return 0;

    /* abort if the petmode is not arena */
    if (rowner->contr->petmode != pet_arena)
        return 0;

    /* abort if the pet, it's owner, or the target is not on battleground*/
    if (!(op_on_battleground(pet, NULL, NULL, NULL)
        && op_on_battleground(owner, NULL, NULL, NULL)
        && op_on_battleground(target, NULL, NULL, NULL)))
        return 0;

    /* if the target is a monster, make sure it's owner is not the same */
    if (target->type != PLAYER && rowner == towner)
        return 0;

    /* check if the target is a player which affects how it will handle
    parties */
    if (target->type != PLAYER) {
        /* if the target is owned by a player make sure than make sure
        it's not in the same party */
        if (towner->type == PLAYER && rowner->contr->party != NULL) {
            if (rowner->contr->party == towner->contr->party)
                return 0;
        }
    } else {
        /* if the target is a player make sure than make sure it's not
        in the same party */
        if (rowner->contr->party != NULL) {
            if (rowner->contr->party == target->contr->party)
                return 0;
        }
    }

    return 1;
}
