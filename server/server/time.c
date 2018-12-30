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
 * Routines that is executed from objects based on their speed have been
 * collected in this file.
 */

#include <global.h>
#include <spells.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#endif

/**
 * Remove non locked doors. The functions check to see if similar
 * doors are next to the one that is being removed, and if so, set it
 * so those will be removed shortly (in a cascade like fashion.)
 *
 * @sa remove_locked_door().
 *
 * @param op
 * door to remove.
 */
void remove_door(object *op) {
    int i;
    object *tmp;

    for (i = 1; i < 9; i += 2)
        if ((tmp = map_find_by_type(op->map, op->x+freearr_x[i], op->y+freearr_y[i], DOOR)) != NULL) {
            tmp->speed = 0.1;
            object_update_speed(tmp);
            tmp->speed_left = -0.2;
        }

    if (op->other_arch) {
        tmp = arch_to_object(op->other_arch);
        tmp->level = op->level;
        object_insert_in_map_at(tmp, op->map, op, 0, op->x, op->y);
    }
    object_remove(op);
    object_free_drop_inventory(op);
}

/**
 * Same as remove_door() but for locked doors.
 *
 * @param op
 * door to remove.
 */
void remove_locked_door(object *op) {
    int i;
    object *tmp;

    for (i = 1; i < 9; i += 2) {
        tmp = map_find_by_type(op->map, op->x+freearr_x[i], op->y+freearr_y[i], LOCKED_DOOR);
        if (tmp && tmp->slaying == op->slaying) {/* same key both doors */
            tmp->speed = 0.1;
            object_update_speed(tmp);
            tmp->speed_left = -0.2;
        }
    }
    if (op->other_arch) {
        tmp = arch_to_object(op->other_arch);
        tmp->level = op->level;
        object_insert_in_map_at(tmp, op->map, op, 0, op->x, op->y);
    }
    object_remove(op);
    object_free_drop_inventory(op);
}

/**
 * Will generate a monster according to parameters of generator.
 *
 * What is generated should be in the generator's inventory.
 *
 * See generate_monster() for the main generator function.
 *
 * @param gen
 * generator.
 * @return
 * TRUE if put a monster on a map, FALSE if did not
 */
static int generate_monster_inv(object *gen) {
    int i;
    int nx, ny;
    object *op, *head = NULL;
    const char *code;
    int qty = 0;

    /* Code below assumes the generator is on a map, as it tries
     * to place the monster on the map.  So if the generator
     * isn't on a map, complain and exit.
     */
    if (gen->map == NULL) {
        LOG(llevError, "Generator (%s) not on a map?\n", gen->name);
        return FALSE;
    }

    /*First count number of objects in inv*/
    FOR_INV_PREPARE(gen, op)
        qty++;
    FOR_INV_FINISH();
    if (!qty) {
        LOG(llevError, "Generator (%s) has no inventory in generate_monster_inv?\n", gen->name);
        return FALSE;/*No inventory*/
    }
    qty = rndm(0, qty-1);
    op = NULL;
    FOR_INV_PREPARE(gen, tmp) {
        op = tmp;
        if (qty == 0)
            break;
        qty--;
    } FOR_INV_FINISH();
    i = object_find_multi_free_spot_within_radius(op, gen, &nx, &ny);
    if (i == -1)
        return FALSE;
    head = object_create_clone(op);
    CLEAR_FLAG(head, FLAG_IS_A_TEMPLATE);
    object_unset_flag_inv(head, FLAG_IS_A_TEMPLATE);
    if (rndm(0, 9))
        generate_artifact(head, gen->map->difficulty);
    code = object_get_value(gen, "generator_code");
    if (code) {
        object_set_value(head, "generator_code", code, 1);
    }
    object_insert_in_map_at(head, gen->map, gen, 0, nx, ny);
    if (QUERY_FLAG(head, FLAG_FREED))
            return TRUE;
    object_fix_multipart(head);
    if (HAS_RANDOM_ITEMS(head)) {
        create_treasure(head->randomitems, head, 0, gen->map->difficulty, 0);
        if (QUERY_FLAG(head, FLAG_MONSTER)) {
            monster_check_apply_all(head);
        }
    }
    return TRUE;
}

/**
 * Generate a monster from the other_arch field.
 *
 * See generate_monster() for the main generator function.
 *
 * @param gen
 * generator.
 * @return
 * TRUE if monster was put on map, FALSE if not
 */
static int generate_monster_arch(object *gen) {
    int i;
    int nx, ny;
    object *op, *head = NULL, *prev = NULL;
    archetype *at = gen->other_arch;
    const char *code;

    if (gen->other_arch == NULL) {
        LOG(llevError, "Generator without other_arch: %s\n", gen->name);
        return FALSE;
    }
    /* Code below assumes the generator is on a map, as it tries
     * to place the monster on the map.  So if the generator
     * isn't on a map, complain and exit.
     */
    if (gen->map == NULL) {
        LOG(llevError, "Generator (%s) not on a map?\n", gen->name);
        return FALSE;
    }
    i = object_find_multi_free_spot_within_radius(&at->clone, gen, &nx, &ny);
    if (i == -1)
        return FALSE;
    while (at != NULL) {
        op = arch_to_object(at);
        op->x = nx+at->clone.x;
        op->y = ny+at->clone.y;

        if (head != NULL) {
            op->head = head;
            prev->more = op;
        }

        if (rndm(0, 9))
            generate_artifact(op, gen->map->difficulty);

        code = object_get_value(gen, "generator_code");

        object_insert_in_map(op, gen->map, gen, 0);
        /* Did generate a monster, just didn't live very long */
        if (QUERY_FLAG(op, FLAG_FREED))
            return TRUE;
        if (HAS_RANDOM_ITEMS(op)) {
            create_treasure(op->randomitems, op, 0, gen->map->difficulty, 0);
            if (QUERY_FLAG(op, FLAG_MONSTER)) {
                monster_check_apply_all(op);
            }
        }
        if (head == NULL) {
            head = op;
            if (code) {
                object_set_value(head, "generator_code", code, 1);
            }
        }
        prev = op;
        at = at->more;
    }
    return TRUE;
}

/**
 * Main generator function. Will generate a monster based on the parameters.
 *
 * @param gen
 * generator.
 */
static void generate_monster(object *gen) {
    sint8 children, max_children;
    sint8 x, y;
    const char *code, *value;
    int did_gen = 0;

    if (GENERATE_SPEED(gen) && rndm(0, GENERATE_SPEED(gen)-1))
        return;

    value = object_get_value(gen, "generator_max_map");
    if (value) {
        max_children = (sint8)strtol(value, NULL, 10);
        if (max_children < 1)
            return;
        code = object_get_value(gen, "generator_code");
        if (code) {
            /* Generator has a limit and has created some,
             * so count how many already exist
             */
            children = 0;
            for (x = 0; x < MAP_WIDTH(gen->map); x++) {
                for (y = 0; y < MAP_HEIGHT(gen->map); y++) {
                    FOR_MAP_PREPARE(gen->map, x, y, tmp) {
                        value = object_get_value(tmp, "generator_code");
                        if (value && value == code) {
                            children++;
                        }
                    } FOR_MAP_FINISH();
                }
            }
            /* and return without generating if there are already enough */
            if (children >= max_children+1)
                return;
        }  else {
            /* Generator has a limit, but hasn't created anything yet,
             * so no need to count, just set code and go
             */
            value = object_get_value(gen, "generator_name");
            if (value) {
                object_set_value(gen, "generator_code", value, 1);
            } else if (gen->name) {
                object_set_value(gen, "generator_code", gen->name, 1);
            } else {
                object_set_value(gen, "generator_code", "generator", 1);
            }
        }
    } /* If this has a max map generator limit */

    if (QUERY_FLAG(gen, FLAG_CONTENT_ON_GEN))
        did_gen = generate_monster_inv(gen);
    else
        did_gen = generate_monster_arch(gen);

    /* See if generator has a generator_limit limit set */
    value = object_get_value(gen, "generator_limit");

    /* Only do this if we actually made a monster.  If the generator
     * was unable to create a monster (no space for example),
     * we don't want to prematurely remove the generator.
     */
    if (value && did_gen) {
        int limit = atoi(value), num_generated = 0;

        value = object_get_value(gen, "generator_generated");
        if (value)
            num_generated = atoi(value);

        if (num_generated++ >= limit) {
            object_remove(gen);
            object_free_drop_inventory(gen);
        } else {
            char buf[50];

            snprintf(buf, sizeof(buf), "%d", num_generated);
            object_set_value(gen, "generator_generated", buf, 1);
        }
    }
}

/**
 * Move for ::FORCE objects.
 *
 * @param op
 * force to test.
 * @todo rename to move_force?
 */
static void remove_force(object *op) {
    if (--op->duration > 0) {
        check_spell_expiry(op);
        return;
    }

    switch (op->subtype) {
    case FORCE_CONFUSION:
        if (op->env != NULL) {
            CLEAR_FLAG(op->env, FLAG_CONFUSED);
            draw_ext_info(NDI_UNIQUE, 0, op->env,
                          MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_BAD_EFFECT_END,
                          "You regain your senses.");
        }
        break;

    case FORCE_TRANSFORMED_ITEM:
        /* The force is into the item that was created */
        if (op->env != NULL && op->inv != NULL) {
            object *inv = op->inv;
            object *pl = object_get_player_container(op);

            object_remove(inv);
            inv->weight = (inv->nrof ? (sint32)(op->env->weight/inv->nrof) : op->env->weight);
            if (op->env->env) {
                object_insert_in_ob(inv, op->env->env);
                if (pl) {
                    char name[HUGE_BUF];

                    query_short_name(inv, name, HUGE_BUF);
                    draw_ext_info_format(NDI_UNIQUE, 0, pl, MSG_TYPE_ITEM, MSG_TYPE_ITEM_CHANGE,
                                         "Your %s recovers its original form.",
                                         name);
                }
            } else {
                /* Object on map */
                object_insert_in_map_at(inv, op->env->map, NULL, 0, op->env->x, op->env->y);
            }
            inv = op->env;
            object_remove(op);
            object_free_drop_inventory(op);
            object_remove(inv);
        }
        return;

    default:
        break;
    }

    if (op->env != NULL) {
        CLEAR_FLAG(op, FLAG_APPLIED);
        change_abil(op->env, op);
        fix_object(op->env);
    }
    object_remove(op);
    object_free_drop_inventory(op);
}

/**
 * Animate a ::TRIGGER.
 *
 * @param op
 * trigger.
 */
static void animate_trigger(object *op) {
    if ((unsigned char)++op->stats.wc >= NUM_ANIMATIONS(op)) {
        op->stats.wc = 0;
        check_trigger(op, NULL);
    } else {
        SET_ANIMATION(op, op->stats.wc);
        object_update(op, UP_OBJ_FACE);
    }
}

/**
 * Move a ::HOLE.
 *
 * @param op
 * hole to move.
 */
static void move_hole(object *op) { /* 1 = opening, 0 = closing */
    if (op->value) { /* We're opening */
        if (--op->stats.wc <= 0) { /* Opened, let's stop */
            op->stats.wc = 0;
            op->speed = 0;
            object_update_speed(op);

            /* Hard coding this makes sense for holes I suppose */
            op->move_on = MOVE_WALK;
            FOR_ABOVE_PREPARE(op, tmp)
                ob_move_on(op, tmp, tmp);
            FOR_ABOVE_FINISH();
        }

        op->state = op->stats.wc;
        animate_object(op, 0);
        object_update(op, UP_OBJ_FACE);
        return;
    }
    /* We're closing */
    op->move_on = 0;

    op->stats.wc++;
    if ((int)op->stats.wc >= NUM_ANIMATIONS(op))
        op->stats.wc = NUM_ANIMATIONS(op)-1;

    op->state = op->stats.wc;
    animate_object(op, 0);
    object_update(op, UP_OBJ_FACE);
    if ((unsigned char)op->stats.wc == (NUM_ANIMATIONS(op)-1)) {
        op->speed = 0;
        object_update_speed(op); /* closed, let's stop */
        return;
    }
}

/**
 * An item (::ARROW or such) stops moving.
 *
 * stop_item() returns a pointer to the stopped object.  The stopped object
 * may or may not have been removed from maps or inventories.  It will not
 * have been merged with other items.
 *
 * This function assumes that only items on maps need special treatment.
 *
 * If the object can't be stopped, or it was destroyed while trying to stop
 * it, NULL is returned.
 *
 * fix_stopped_item() should be used if the stopped item should be put on
 * the map.
 *
 * @param op
 * object to check.
 * @return
 * pointer to stopped object, NULL if destroyed or can't be stopped.
 */
object *stop_item(object *op) {
    if (free_no_drop(op))
        return NULL;

    if (op->map == NULL)
        return op;

    switch (op->type) {
    case THROWN_OBJ: {
            object *payload = op->inv;

            if (payload == NULL)
                return NULL;
            object_remove(payload);
            object_remove(op);
            object_free_drop_inventory(op);
            return payload;
        }

    case ARROW:
        if (op->speed >= MIN_ACTIVE_SPEED)
            op = fix_stopped_arrow(op);
        return op;

    default:
        return op;
    }
}

/**
 * Put stopped item where stop_item() had found it.
 * Inserts item into the old map, or merges it if it already is on the map.
 *
 * @param op
 * object to stop.
 * @param map
 * must be the value of op->map before stop_item() was called.
 * @param originator
 * what caused op to be stopped.
 */
void fix_stopped_item(object *op, mapstruct *map, object *originator) {
    if (map == NULL)
        return;
    if (QUERY_FLAG(op, FLAG_REMOVED))
        object_insert_in_map(op, map, originator, 0);
    else if (op->type == ARROW)
        object_merge(op, NULL);    /* only some arrows actually need this */
}

/**
 * An ::ARROW stops moving.
 *
 * @param op
 * arrow stopping.
 * @return
 * arrow, or NULL if it was broken.
 */
object *fix_stopped_arrow(object *op) {
    if (free_no_drop(op))
        return NULL;

    if (rndm(0, 99) < op->stats.food) {
        /* Small chance of breaking */
        object_remove(op);
        object_free_drop_inventory(op);
        return NULL;
    }

    op->direction = 0;
    op->move_on = 0;
    op->move_type = 0;
    op->speed = 0;
    object_update_speed(op);
    op->stats.wc = op->stats.sp;
    op->stats.dam = op->stats.hp;
    op->attacktype = op->stats.grace;
    if (op->slaying != NULL)
        FREE_AND_CLEAR_STR(op->slaying);

    if (op->skill != NULL)
        FREE_AND_CLEAR_STR(op->skill);

    if (op->spellarg != NULL) {
        op->slaying = add_string(op->spellarg);
        free(op->spellarg);
        op->spellarg = NULL;
    } else
        op->slaying = NULL;

    /* Reset these to zero, so that object_can_merge will work properly */
    op->spellarg = NULL;
    op->stats.sp = 0;
    op->stats.hp = 0;
    op->stats.grace = 0;
    op->level = 0;
    animate_object(op, 0);
    object_clear_owner(op); /* So that stopped arrows will be saved */
    object_update(op, UP_OBJ_FACE);
    return op;
}

/**
 * Check whether the given object is FLAG_NO_DROP. If so, (optionally) remove
 * and free it.
 *
 * @param op
 * the object to check
 * @return
 * whether the object was freed
 */
int free_no_drop(object *op) {
    if (!QUERY_FLAG(op, FLAG_NO_DROP)) {
        return 0;
    }

    if (!QUERY_FLAG(op, FLAG_REMOVED)) {
        object_remove(op);
    }

    object_free2(op, FREE_OBJ_FREE_INVENTORY);
    return 1;
}

/**
 * Replaces op with its other_arch if it has reached its end of life.
 *
 * This routine doesnt seem to work for "inanimate" objects that
 * are being carried, ie a held torch leaps from your hands!.
 * Modified this routine to allow held objects. b.t.
 *
 * @param op
 * object to change. Will be removed and replaced.
 */
void change_object(object *op) { /* Doesn`t handle linked objs yet */
    object *env;
    int i;
    int friendly;
    int unaggressive;
    object *owner;

    if (op->other_arch == NULL) {
        LOG(llevError, "Change object (%s) without other_arch error.\n", op->name);
        return;
    }

    /* In non-living items only change when food value is 0 */
    if (!QUERY_FLAG(op, FLAG_ALIVE)) {
        if (op->stats.food-- > 0)
            return;
        else
            op->stats.food = 1; /* so 1 other_arch is made */
    }

    env = op->env;
    object_remove(op);
    friendly = QUERY_FLAG(op, FLAG_FRIENDLY);
    unaggressive = QUERY_FLAG(op, FLAG_UNAGGRESSIVE);
    owner = object_get_owner(op);
    for (i = 0; i < op->stats.food; i++) { /* This doesn't handle op->more yet */
        object *tmp;

        tmp = arch_to_object(op->other_arch);
        if (op->type == LAMP)
            tmp->stats.food = op->stats.food-1;
        tmp->stats.hp = op->stats.hp;
        if (friendly) {
            SET_FLAG(tmp, FLAG_FRIENDLY);
            add_friendly_object(tmp);
            tmp->attack_movement = PETMOVE;
            if (owner != NULL)
                object_set_owner(tmp, owner);
        }
        if (unaggressive)
            SET_FLAG(tmp, FLAG_UNAGGRESSIVE);
        if (env) {
            tmp->x = env->x,
            tmp->y = env->y;
            tmp = object_insert_in_ob(tmp, env);
        } else
            object_insert_to_free_spot_or_free(tmp, op->map, op->x, op->y, 1, SIZEOFFREE1+1, op);
    }
    if (friendly)
        remove_friendly_object(op);
    object_free_drop_inventory(op);
}

/**
 * Move for ::FIREWALL.
 *
 * firewalls fire other spells.
 * The direction of the wall is stored in op->stats.sp.
 * walls can have hp, so they can be torn down.
 *
 * @param op
 * firewall.
 */
void move_firewall(object *op) {
    object *spell;

    if (!op->map)
        return;   /* dm has created a firewall in his inventory */

    spell = op->inv;
    if (!spell) {
        LOG(llevError, "move_firewall: no spell specified (%s, %s, %d, %d)\n", op->name, op->map->name, op->x, op->y);
        return;
    }

    cast_spell(op, op, op->stats.sp ? op->stats.sp : get_random_dir(), spell, NULL);
}


/**
 * This function takes a ::PLAYERMOVER as an
 * argument, and performs the function of a player mover, which is:
 *
 * a player mover finds any players that are sitting on it.  It
 * moves them in the op->stats.sp direction.  speed is how often it'll move.
 * - If attacktype is nonzero it will paralyze the player.  If lifesave is set,
 * - it'll dissapear after hp+1 moves.  If hp is set and attacktype is set,
 * - it'll paralyze the victim for hp*his speed/op->speed
 *
 * @param op
 * mover.
 */
void move_player_mover(object *op) {
    int dir = op->stats.sp;
    sint16 nx, ny;
    mapstruct *m;

    if (!op->map) {
        if (op->env && op->env->map)
            LOG(llevError, "move_player_mover: mover not in a map at %s %d %d!\n", op->env->map->path, op->env->x, op->env->y);
        else
            LOG(llevError, "move_player_mover: mover not in a map at undefinite location!");
        op->speed = 0;
        object_update_speed(op);
        return;
    }

    /* Determine direction now for random movers so we do the right thing */
    if (!dir)
        dir = get_random_dir();

    FOR_MAP_PREPARE(op->map, op->x, op->y, victim) {
        if (QUERY_FLAG(victim, FLAG_ALIVE)
        && !QUERY_FLAG(victim, FLAG_WIZPASS)
        && (victim->move_type&op->move_type || !victim->move_type)) {
            victim = HEAD(victim);

            if (QUERY_FLAG(op, FLAG_LIFESAVE) && op->stats.hp-- < 0) {
                object_remove(op);
                object_free_drop_inventory(op);
                return;
            }
            nx = op->x+freearr_x[dir];
            ny = op->y+freearr_y[dir];
            m = op->map;
            if (get_map_flags(m, &m, nx, ny, &nx, &ny)&P_OUT_OF_MAP) {
                LOG(llevError, "move_player_mover: Trying to push player off the map! map=%s (%d, %d)\n", m->path, op->x, op->y);
                return;
            }

            if (should_director_abort(op, victim))
                return;

            FOR_MAP_PREPARE(m, nx, ny, nextmover) {
                if (nextmover->type == PLAYERMOVER)
                    nextmover->speed_left = -.99;
                if (QUERY_FLAG(nextmover, FLAG_ALIVE)) {
                    op->speed_left = -1.1;  /* wait until the next thing gets out of the way */
                }
            } FOR_MAP_FINISH();

            if (victim->type == PLAYER) {
                /*  only level >= 1 movers move people */
                if (op->level) {
                    /* Following is a bit of hack.  We need to make sure it
                     * is cleared, otherwise the player will get stuck in
                     * place.  This can happen if the player used a spell to
                     * get to this space.
                     */
                    victim->contr->fire_on = 0;
                    victim->speed_left = -FABS(victim->speed);
                    move_player(victim, dir);
                } else
                    return;
            } else
                move_object(victim, dir);

            if (!op->stats.maxsp && op->attacktype)
                op->stats.maxsp = 2.0;

            if (op->attacktype)  { /* flag to paralyze the player */
                victim->speed_left = -FABS(op->stats.maxsp*victim->speed/op->speed);
                /* Not sure why, but for some chars on metalforge, they
                 * would sometimes get -inf speed_left, and from the
                 * description, it could only happen here, so just put
                 * a lower sanity limit.  My only guess is that the
                 * mover has 0 speed.
                 */
                if (victim->speed_left < -5.0)
                    victim->speed_left = -5.0;
            }
        }
    } FOR_MAP_FINISH();
}

/**
 * Main object move function.
 *
 * @param op
 * object to move.
 * @return
 * 0 if object didn't move, 1 else?
 * @todo remove unused return value?
 */
int process_object(object *op) {
    if (QUERY_FLAG(op, FLAG_IS_A_TEMPLATE))
        return 0;

    /* Lauwenmark: Handle for plugin time event */
    if (execute_event(op, EVENT_TIME, NULL, NULL, NULL, SCRIPT_FIX_NOTHING) != 0)
        return 0;

    if (QUERY_FLAG(op, FLAG_MONSTER))
        if (monster_move(op) || QUERY_FLAG(op, FLAG_FREED))
            return 1;

    if ((QUERY_FLAG(op, FLAG_ANIMATE) && op->anim_speed == 0)
    || (op->temp_animation_id && op->temp_anim_speed == 0)) {
        op->state++;
        if (op->type == PLAYER)
            animate_object(op, op->facing);
        else
            animate_object(op, op->direction);

        if (QUERY_FLAG(op, FLAG_SEE_ANYWHERE))
            make_sure_seen(op);
    }
    if (QUERY_FLAG(op, FLAG_CHANGING) && !op->state) {
        change_object(op);
        return 1;
    }
    if (QUERY_FLAG(op, FLAG_GENERATOR) && !QUERY_FLAG(op, FLAG_FRIENDLY))
        generate_monster(op);

    /* If object can be used up, decrement 'food' and eventually remove it. */
    if (QUERY_FLAG(op, FLAG_IS_USED_UP) && --op->stats.food <= 0) {
        if (QUERY_FLAG(op, FLAG_APPLIED)) {
            remove_force(op);
        } else {
            if (op->env != NULL && op->env->type == PLAYER) {
                sstring key;
                key_value *used_up_message;

                key = add_string("used_up_message");
                used_up_message = object_get_key_value(op, key);
                free_string(key);

                if (used_up_message != NULL) {
                    draw_ext_info_format(
                        NDI_BLACK, 0, op->env, MSG_TYPE_ITEM, MSG_TYPE_ITEM_REMOVE,
                        "The %s %s.", op->name, used_up_message->value);
                }
            }

            object_remove(op);
            if (QUERY_FLAG(op, FLAG_SEE_ANYWHERE))
                make_sure_not_seen(op);
            object_free2(op, FREE_OBJ_DROP_ABOVE_FLOOR);
        }
        return 1;
    }
    return (ob_process(op) == METHOD_OK ? 1 : 0);
}

void legacy_remove_force(object *op) {
    remove_force(op);
}

void legacy_animate_trigger(object *op) {
    animate_trigger(op);
}

void legacy_move_hole(object *op) {
    move_hole(op);
}
