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
 * Those functions handle object moving and pushing.
 */

#include <global.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#endif

static int roll_ob(object *op, int dir, object *pusher);

/**
 * Try to move op in the direction "dir".
 * @param op
 * what to move.
 * @param dir
 * moving direction.
 * @retval 0
 * something blocks the passage.
 * @retval 1
 * op was moved..
 */

int move_object(object *op, int dir) {
    return move_ob(op, dir, op);
}

/**
 * Op is trying to move in direction dir.
 *
 * @param op
 * what is moving.
 * @param dir
 * what direction op wants to move.
 * @param originator
 * typically the same as op, but can be different if originator is causing op to
 * move (originator is pushing op).
 * @retval 0
 * op is not able to move to the desired space.
 * @retval 1
 * op was moved.
  */
int move_ob(object *op, int dir, object *originator) {
    object *part;
    mapstruct *m;

    op->direction = dir;

    if (op->will_apply&(WILL_APPLY_EARTHWALL|WILL_APPLY_DOOR)) {
        tag_t op_tag;

        op_tag = op->count;
        for (part = op; part != NULL; part = part->more) {
            sint16 x, y;

            if (get_map_flags(part->map, &m, part->x+freearr_x[dir], part->y+freearr_y[dir], &x, &y)&P_OUT_OF_MAP)
                continue;

            if (op->will_apply&WILL_APPLY_EARTHWALL) {
                monster_check_earthwalls(op, m, x, y);
                if (object_was_destroyed(op, op_tag))
                    return 1;
            }

            if (op->will_apply&WILL_APPLY_DOOR) {
                monster_check_doors(op, m, x, y);
                if (object_was_destroyed(op, op_tag))
                    return 1;
            }
        }
    }

    for (part = op; part != NULL; part = part->more) {
        sint16 x, y;

        if (get_map_flags(part->map, &m, part->x+freearr_x[dir], part->y+freearr_y[dir], &x, &y)&P_OUT_OF_MAP)
            return 0;

        if (!QUERY_FLAG(op, FLAG_WIZPASS) && blocked_link(op, m, x, y))
            return 0;
    }

    object_remove(op);
    object_insert_in_map_at(op, op->map, originator, 0, op->x+freearr_x[dir], op->y+freearr_y[dir]);

    /* Hmmm.  Should be possible for multispace players now */
    if (op->type == PLAYER) {
        esrv_map_scroll(&op->contr->socket, freearr_x[dir], freearr_y[dir]);
        op->contr->socket.update_look = 1;
        op->contr->socket.look_position = 0;
    } else if (op->type == TRANSPORT) {
        FOR_INV_PREPARE(op, pl)
            if (pl->type == PLAYER) {
                pl->contr->do_los = 1;
                pl->map = op->map;
                pl->x = op->x;
                pl->y = op->y;
                esrv_map_scroll(&pl->contr->socket, freearr_x[dir], freearr_y[dir]);
                pl->contr->socket.update_look = 1;
                pl->contr->socket.look_position = 0;
            }
        FOR_INV_FINISH();
    }

    return 1; /* this shouldn't be reached */
}

/**
 * Move an object (even linked objects) to another spot
 * on the same map.
 *
 * Does nothing if there is no free spot.
 *
 * @param op
 * what to move.
 * @param x
 * @param y
 * new coordinates.
 * @param randomly
 * if true, use object_find_free_spot() to find the destination, otherwise
 * use object_find_first_free_spot().
 * @param originator
 * what is causing op to move.
 * @retval 1
 * op was destroyed.
 * @retval 0
 * op was moved.
 */
int transfer_ob(object *op, int x, int y, int randomly, object *originator) {
    int i;
    object *tmp;

    if (randomly)
        i = object_find_free_spot(op, op->map, x, y, 0, SIZEOFFREE);
    else
        i = object_find_first_free_spot(op, op->map, x, y);

    if (i == -1)
        return 0; /* No free spot */

    op = HEAD(op);
    object_remove(op);
    tmp = object_insert_in_map_at(op, op->map, originator, 0, x+freearr_x[i], y+freearr_y[i]);
    if (op && op->type == PLAYER)
        map_newmap_cmd(&op->contr->socket);
    if (tmp)
        return 0;
    else
        return 1;
}

/**
 * Teleport an item around a nearby random teleporter of specified type.
 *
 * It is basically used so that shop_mats and normal teleporters can
 * be used close to each other and not have the player put to the
 * one of another type.
 *
 * @param teleporter
 * what is teleporting user.
 * @param tele_type
 * what object type user can be put on. this is either set to SHOP_MAT or TELEPORTER.
 * @param user
 * what object to teleport.
 * @retval 1
 * user was destroyed.
 * @retval 0
 * user is still valid, but may have moved or not.
 * @todo fix weird return values.
 */
int teleport(object *teleporter, uint8 tele_type, object *user) {
    object *altern[120]; /* Better use c/malloc here in the future */
    int i, j, k, nrofalt = 0;
    object *other_teleporter, *tmp;
    mapstruct *m;
    sint16 sx, sy;

    if (user == NULL)
        return 0;
    user = HEAD(user);

    /* Find all other teleporters within range.  This range
     * should really be setable by some object attribute instead of
     * using hard coded values.
     */
    for (i = -5; i < 6; i++)
        for (j = -5; j < 6; j++) {
            if (i == 0 && j == 0)
                continue;
            /* Perhaps this should be extended to support tiled maps */
            if (OUT_OF_REAL_MAP(teleporter->map, teleporter->x+i, teleporter->y+j))
                continue;
            FOR_MAP_PREPARE(teleporter->map, teleporter->x+i, teleporter->y+j, tmp) {
                if (tmp->type == tele_type) {
                    altern[nrofalt++] = tmp;
                    break;
                }
            } FOR_MAP_FINISH();
        }

    if (!nrofalt) {
        LOG(llevError, "No alternative teleporters around!\n");
        return 0;
    }

    other_teleporter = altern[RANDOM()%nrofalt];
    k = object_find_free_spot(user, other_teleporter->map, other_teleporter->x, other_teleporter->y, 1, 9);

    /* if k==-1, unable to find a free spot.  If this is shop
     * mat that the player is using, find someplace to move
     * the player - otherwise, player can get trapped in the shops
     * that appear in random dungeons.  We basically just make
     * sure the space isn't no pass (eg wall), and don't care
     * about is alive.
     */
    if (k == -1) {
        if (tele_type == SHOP_MAT && user->type == PLAYER) {
            for (k = 1; k < 9; k++) {
                if (get_map_flags(other_teleporter->map, &m,
                                  other_teleporter->x+freearr_x[k],
                                  other_teleporter->y+freearr_y[k], &sx, &sy)&P_OUT_OF_MAP)
                    continue;

                if (!OB_TYPE_MOVE_BLOCK(user, GET_MAP_MOVE_BLOCK(m, sx, sy)))
                    break;
            }
            if (k == 9) {
                LOG(llevError, "Shop mat %s (%d, %d) is in solid rock?\n", other_teleporter->name, other_teleporter->x, other_teleporter->y);
                /* Teleport player on top of blocked destination: this prevents
                 * players from being trapped inside shops if the destination
                 * is blocked with earth walls.
                 */
                k = 0;
            }
        } else
            return 0;
    }

    object_remove(user);

    tmp = object_insert_in_map_at(user, other_teleporter->map, NULL, 0, other_teleporter->x+freearr_x[k], other_teleporter->y+freearr_y[k]);
    if (tmp && tmp->type == PLAYER)
        map_newmap_cmd(&tmp->contr->socket);
    return (tmp == NULL);
}

/**
 * An object is pushed by another which is trying to take its place.
 *
 * @param op
 * what is being pushed.
 * @param dir
 * pushing direction.
 * @param pusher
 * what is pushing op.
 */
void recursive_roll(object *op, int dir, object *pusher) {
    char name[MAX_BUF];

    query_name(op, name, MAX_BUF);
    if (!roll_ob(op, dir, pusher)) {
        draw_ext_info_format(NDI_UNIQUE, 0, pusher, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_FAILURE,
                             "You fail to push the %s.",
                             name);
        return;
    }
    (void)move_ob(pusher, dir, pusher);
    draw_ext_info_format(NDI_BLACK, 0, pusher, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                         "You move the %s.",
                         name);
    return;
}

/**
 * Checks if an objects fits on a specified spot.
 *
 * This is a new version of blocked, this one handles objects
 * that can be passed through by monsters with the CAN_PASS_THRU defined.
 *
 * Very new version handles also multipart objects
 * This is currently only used for the boulder roll code.
 *
 * @param op
 * what object to fit.
 * @param m
 * @param x
 * @param y
 * where to put op.
 * @retval 1
 * object does not fit.
 * @retval 0
 * object fits.
 */

static int try_fit(object *op, mapstruct *m, int x, int y) {
    object *more;
    sint16 tx, ty;
    int mflags;
    mapstruct *m2;

    op = HEAD(op);
    for (more = op; more; more = more->more) {
        tx = x+more->x-op->x;
        ty = y+more->y-op->y;

        mflags = get_map_flags(m, &m2, tx, ty, &tx, &ty);

        if (mflags&P_OUT_OF_MAP)
            return 1;

        FOR_MAP_PREPARE(m2, tx, ty, tmp) {
            if (tmp->head == op || tmp == op)
                continue;

            if ((QUERY_FLAG(tmp, FLAG_ALIVE) && tmp->type != DOOR))
                return 1;

            if (OB_MOVE_BLOCK(op, tmp))
                return 1;
        } FOR_MAP_FINISH();
    }
    return 0;
}

/**
 * An object is being pushed, and may push other objects.
 *
 * This is not perfect yet.
 * it does not roll objects behind multipart objects properly.
 * Support for rolling multipart objects is questionable.
 *
 * @param op
 * what is being pushed.
 * @param dir
 * pushing direction.
 * @param pusher
 * what is pushing op.
 * @retval 0
 * op couldn't move.
 * @retval 1
 * op, and potentially other objects, moved.
 */

static int roll_ob(object *op, int dir, object *pusher) {
    sint16 x, y;
    int flags;
    mapstruct *m;
    MoveType move_block;

    op = HEAD(op);
    x = op->x+freearr_x[dir];
    y = op->y+freearr_y[dir];

    if (!QUERY_FLAG(op, FLAG_CAN_ROLL)
    || (op->weight && random_roll(0, op->weight/50000-1, pusher, PREFER_LOW) > pusher->stats.Str))
        return 0;

    m = op->map;
    flags = get_map_flags(m, &m, x, y, &x, &y);

    if (flags&(P_OUT_OF_MAP|P_IS_ALIVE))
        return 0;

    move_block = GET_MAP_MOVE_BLOCK(m, x, y);

    /* If the target space is not blocked, no need to look at the objects on it */
    if ((op->move_type&move_block) == op->move_type) {
        FOR_MAP_PREPARE(m, x, y, tmp) {
            if (tmp->head == op)
                continue;
            if (OB_MOVE_BLOCK(op, tmp) && !roll_ob(tmp, dir, pusher))
                return 0;
        } FOR_MAP_FINISH();
    }
    if (try_fit(op, m, x, y))
        return 0;

    object_remove(op);
    object_insert_in_map_at(op, op->map, pusher, 0, op->x+freearr_x[dir], op->y+freearr_y[dir]);
    return 1;
}

/**
 * Something is pushing some other object.
 *
 * @param who
 * object being pushed.
 * @param dir
 * pushing direction.
 * @param pusher
 * what is pushing who.
 * @retval 1
 * if pushing invokes a attack
 * @retval 0
 * no attack during pushing.
 * @todo fix return value which is weird for last case.
 */
int push_ob(object *who, int dir, object *pusher) {
    int str1, str2;
    object *owner;

    who = HEAD(who);
    owner = object_get_owner(who);

    /* Wake up sleeping monsters that may be pushed */
    CLEAR_FLAG(who, FLAG_SLEEP);

    /* player change place with his pets or summoned creature */
    /* TODO: allow multi arch pushing. Can't be very difficult */
    if (who->more == NULL
    && (owner == pusher || (owner != NULL && owner->type == PLAYER && owner->contr->party != NULL && owner->contr->party == pusher->contr->party))) {
        int temp;
        mapstruct *m;

        object_remove(who);
        object_remove(pusher);
        temp = pusher->x;
        pusher->x = who->x;
        who->x = temp;

        temp = pusher->y;
        pusher->y = who->y;
        who->y = temp;

        m = pusher->map;
        pusher->map = who->map;
        who->map = m;

        object_insert_in_map_at(who, who->map, pusher, 0, who->x, who->y);
        object_insert_in_map_at(pusher, pusher->map, pusher, 0, pusher->x, pusher->y);

        /* we presume that if the player is pushing his put, he moved in
         * direction 'dir'.  I can' think of any case where this would not be
         * the case.  Putting the map_scroll should also improve performance some.
         */
        if (pusher->type == PLAYER) {
            esrv_map_scroll(&pusher->contr->socket, freearr_x[dir], freearr_y[dir]);
            pusher->contr->socket.update_look = 1;
            pusher->contr->socket.look_position = 0;
        }
        return 0;
    }

    /* We want ONLY become enemy of evil, unaggressive monster. We must RUN in them */
    /* In original we have here a unaggressive check only - that was the reason why */
    /* we so often become an enemy of friendly monsters... */
    /* funny: was they set to unaggressive 0 (= not so nice) they don't attack */
    if (owner != pusher
    && pusher->type == PLAYER
    && who->type != PLAYER
    && !QUERY_FLAG(who, FLAG_FRIENDLY)
    && !QUERY_FLAG(who, FLAG_NEUTRAL)) {
        if (pusher->contr->run_on) { /* only when we run */
            draw_ext_info_format(NDI_UNIQUE, 0, pusher,
                                 MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_FAILURE,
                                 "You start to attack %s !!",
                                 who->name);
            CLEAR_FLAG(who, FLAG_UNAGGRESSIVE); /* the sucker don't like you anymore */
            object_set_enemy(who, pusher);
            return 1;
        } else {
            draw_ext_info_format(NDI_UNIQUE, 0, pusher,
                                 MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_FAILURE,
                                 "You avoid attacking %s.",
                                 who->name);
        }
    }

    /* now, lets test stand still. we NEVER can push stand_still monsters. */
    if (QUERY_FLAG(who, FLAG_STAND_STILL)) {
        draw_ext_info_format(NDI_UNIQUE, 0, pusher,
                             MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_FAILURE,
                             "You can't push %s.",
                             who->name);
        return 0;
    }

    /* This block is basically if you are pushing friendly but
     * non pet creaturs.
     * It basically does a random strength comparision to
     * determine if you can push someone around.  Note that
     * this pushes the other person away - its not a swap.
     */

    str1 = (who->stats.Str > 0 ? who->stats.Str : who->level);
    str2 = (pusher->stats.Str > 0 ? pusher->stats.Str : pusher->level);
    if (QUERY_FLAG(who, FLAG_WIZ)
    || random_roll(str1, str1/2+str1*2, who, PREFER_HIGH) >= random_roll(str2, str2/2+str2*2, pusher, PREFER_HIGH)
    || !move_object(who, dir)) {
        if (who->type == PLAYER) {
            draw_ext_info_format(NDI_UNIQUE, 0, who, MSG_TYPE_VICTIM, MSG_TYPE_VICTIM_WAS_PUSHED,
                                 "%s tried to push you.",
                                 pusher->name);
        }
        return 0;
    }

    /* If we get here, the push succeeded.
     * Let everyone know the status.
     */
    if (who->type == PLAYER) {
        draw_ext_info_format(NDI_UNIQUE, 0, who, MSG_TYPE_VICTIM, MSG_TYPE_VICTIM_WAS_PUSHED,
                             "%s pushed you.",
                             pusher->name);
    }
    if (pusher->type == PLAYER) {
        draw_ext_info_format(NDI_UNIQUE, 0, pusher, MSG_TYPE_VICTIM, MSG_TYPE_VICTIM_WAS_PUSHED,
                             "You pushed %s back.",
                             who->name);
    }

    return 1;
}

/**
 * Move an object one square toward a specified destination on the same map.
 * The move takes into account blocked squares for op, and things like that.
 * No check is done to know if the object has enough speed to move.
 *
 * @param op object to move
 * @param x
 * @param y destination coordinates
 * @return
 * 0 if op is on the specified spot, 1 if it moved towards the goal, 2 if it didn't find any path to the goal.
 */
int move_to(object *op, int x, int y) {
    int direction;

    if (op->x == x && op->y == y)
        return 0;

    if (GET_MAP_FLAGS(op->map, x, y)&P_OUT_OF_MAP)
        return 2;

    direction = monster_compute_path(op, GET_MAP_OB(op->map, x, y), -1);
    if (direction == -1)
        return 2;

    op->direction = direction;
    op->facing = direction;
    if (op->animation_id)
        animate_object(op, op->direction);

    /* can fail, as the direction computing takes into account the blocked state,
     * except for the final spot... */
    if (move_ob(op, direction, op) == 0)
        return 2;

    return 1;
}
