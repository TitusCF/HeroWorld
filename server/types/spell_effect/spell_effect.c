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

/** @file
 * The implementation of the Spell Effect class of objects.
 * @todo Split the subtype functions into their own file each, and split large functions.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static method_ret spell_effect_type_move_on(ob_methods *context, object *trap, object *victim, object *originator);
static method_ret spell_effect_type_process(ob_methods *context, object *op);

static void move_bolt(object *op);
static void move_bullet(object *op);
static void explosion(object *op);
static void move_cone(object *op);
static void animate_bomb(object *op);
static void move_missile(object *op);
static void execute_word_of_recall(object *op);
static void move_ball_spell(object *op);
static void move_swarm_spell(object *op);
static void move_aura(object *aura);

static void forklightning(object *op, object *tmp);
static void check_spell_knockback(object *op);

/**
 * Initializer for the SPELL_EFFECT object type.
 */
void init_type_spell_effect(void) {
    register_move_on(SPELL_EFFECT, spell_effect_type_move_on);
    register_process(SPELL_EFFECT, spell_effect_type_process);
}

/**
 * Move on this Spell Effect object.
 * @param context The method context
 * @param trap The Spell Effect we're moving on
 * @param victim The object moving over this one
 * @param originator The object that caused the move_on event
 * @return METHOD_OK
 */
static method_ret spell_effect_type_move_on(ob_methods *context, object *trap, object *victim, object *originator) {
    if (common_pre_ob_move_on(trap, victim, originator) == METHOD_ERROR)
        return METHOD_OK;

    switch (trap->subtype) {
    case SP_CONE:
        if (QUERY_FLAG(victim, FLAG_ALIVE)
        && trap->speed
        && trap->attacktype)
            hit_player(victim, trap->stats.dam, trap, trap->attacktype, 0);
        break;

    case SP_MAGIC_MISSILE:
        if (QUERY_FLAG(victim, FLAG_ALIVE)) {
            tag_t spell_tag = trap->count;

            hit_player(victim, trap->stats.dam, trap, trap->attacktype, 1);
            if (!object_was_destroyed(trap, spell_tag)) {
                object_remove(trap);
                object_free_drop_inventory(trap);
            }
        }
        break;

    case SP_MOVING_BALL:
        if (QUERY_FLAG(victim, FLAG_ALIVE))
            hit_player(victim, trap->stats.dam, trap, trap->attacktype, 1);
        else if (victim->material || victim->materialname)
            save_throw_object(victim, trap->attacktype, trap);
        break;
    }
    common_post_ob_move_on(trap, victim, originator);
    return METHOD_OK;
}

/**
 * Handle ob_process for all SPELL_EFFECT objects.
 * @param context The method context
 * @param op The spell effect that's being processed.
 * @return METHOD_OK
 */
static method_ret spell_effect_type_process(ob_methods *context, object *op) {
    switch (op->subtype) {
    case SP_BOLT:
        move_bolt(op);
        break;

    case SP_BULLET:
        move_bullet(op);
        break;

    case SP_EXPLOSION:
        explosion(op);
        break;

    case SP_CONE:
        move_cone(op);
        break;

    case SP_BOMB:
        animate_bomb(op);
        break;

    case SP_MAGIC_MISSILE:
        move_missile(op);
        break;

    case SP_WORD_OF_RECALL:
        execute_word_of_recall(op);
        break;

    case SP_MOVING_BALL:
        move_ball_spell(op);
        break;

    case SP_SWARM:
        move_swarm_spell(op);
        break;

    case SP_AURA:
        move_aura(op);
        break;
    }
    return METHOD_OK;
}

/**
 * Moves bolt 'op'. Basically, it just advances a space, and checks for various
 * things that may stop it.
 * @param op The bolt object moving.
 */
static void move_bolt(object *op) {
    object *tmp;
    int mflags;
    sint16 x, y;
    mapstruct *m;

    if (--(op->duration) < 0) {
        object_remove(op);
        object_free_drop_inventory(op);
        return;
    }
    hit_map(op, 0, op->attacktype, 1);

    if (op->weight)
        check_spell_knockback(op);

    if (!op->direction)
        return;

    if (--op->range < 0) {
        op->range = 0;
    } else {
        x = op->x+DIRX(op);
        y = op->y+DIRY(op);
        m = op->map;
        mflags = get_map_flags(m, &m, x, y, &x, &y);

        if (mflags&P_OUT_OF_MAP)
            return;

        /* We are about to run into something - we may bounce */
        /* Calling reflwall is pretty costly, as it has to look at all the objects
         * on the space.  So only call reflwall if we think the data it returns
         * will be useful.
         */
        if (OB_TYPE_MOVE_BLOCK(op, GET_MAP_MOVE_BLOCK(m, x, y))
        || ((mflags&P_IS_ALIVE) && reflwall(m, x, y, op))) {
            if (!QUERY_FLAG(op, FLAG_REFLECTING))
                return;

            /* Since walls don't run diagonal, if the bolt is in
             * one of 4 main directions, it just reflects back in the
             * opposite direction.  However, if the bolt is travelling
             * on the diagonal, it is trickier - eg, a bolt travelling
             * northwest bounces different if it hits a north/south
             * wall (bounces to northeast) vs an east/west (bounces
             * to the southwest.
             */
            if (op->direction&1) {
                op->direction = absdir(op->direction+4);
            } else {
                int left, right;
                int mflags;

                /* Need to check for P_OUT_OF_MAP: if the bolt is tavelling
                 * over a corner in a tiled map, it is possible that
                 * op->direction is within an adjacent map but either
                 * op->direction-1 or op->direction+1 does not exist.
                 */
                mflags = get_map_flags(op->map, &m, op->x+freearr_x[absdir(op->direction-1)], op->y+freearr_y[absdir(op->direction-1)], &x, &y);

                left = (mflags&P_OUT_OF_MAP) ? 0 : OB_TYPE_MOVE_BLOCK(op, GET_MAP_MOVE_BLOCK(m, x, y));

                mflags = get_map_flags(op->map, &m, op->x+freearr_x[absdir(op->direction+1)], op->y+freearr_y[absdir(op->direction+1)], &x, &y);
                right = (mflags&P_OUT_OF_MAP) ? 0 : OB_TYPE_MOVE_BLOCK(op, GET_MAP_MOVE_BLOCK(m, x, y));

                if (left == right)
                    op->direction = absdir(op->direction+4);
                else if (left)
                    op->direction = absdir(op->direction+2);
                else if (right)
                    op->direction = absdir(op->direction-2);
            }
            object_update_turn_face(op); /* A bolt *must *be IS_TURNABLE */
            return;
        } else { /* Create a copy of this object and put it ahead */
            tmp = object_new();
            object_copy(op, tmp);
            tmp->speed_left = -0.1;
            tmp = object_insert_in_map_at(tmp, op->map, op, 0, op->x+DIRX(op), op->y+DIRY(op));
            /* To make up for the decrease at the top of the function */
            tmp->duration++;

            /* New forking code.  Possibly create forks of this object
             * going off in other directions.
             */

            if (rndm(0, 99) < tmp->stats.Dex) {  /* stats.Dex % of forking */
                forklightning(op, tmp);
            }
            /* In this way, the object left behind sticks on the space, but
             * doesn't create any bolts that continue to move onward.
             */
            op->range = 0;
        } /* copy object and move it along */
    } /* if move bolt along */
}

/**
 * Moves bullet 'op'. Basically, we move 'op' one square, and if it hits
 * something, call check_bullet. This function is only applicable to bullets,
 * but not to all fired arches (eg, bolts).
 * @param op The bullet being moved.
 */
static void move_bullet(object *op) {
    sint16 new_x, new_y;
    int mflags;
    mapstruct *m;

    /* Reached the end of its life - remove it */
    if (--op->range <= 0) {
        if (op->other_arch) {
            explode_bullet(op);
        } else {
            object_remove(op);
            object_free_drop_inventory(op);
        }
        return;
    }

    new_x = op->x+DIRX(op);
    new_y = op->y+DIRY(op);
    m = op->map;
    mflags = get_map_flags(m, &m, new_x, new_y, &new_x, &new_y);

    if (mflags&P_OUT_OF_MAP) {
        object_remove(op);
        object_free_drop_inventory(op);
        return;
    }

    if (!op->direction || OB_TYPE_MOVE_BLOCK(op, GET_MAP_MOVE_BLOCK(m, new_x, new_y))) {
        if (op->other_arch) {
            explode_bullet(op);
        } else {
            object_remove(op);
            object_free_drop_inventory(op);
        }
        return;
    }

    object_remove(op);
    if ((op = object_insert_in_map_at(op, m, op, 0, new_x, new_y)) == NULL)
        return;

    if (reflwall(op->map, op->x, op->y, op)) {
        op->direction = absdir(op->direction+4);
        object_update_turn_face(op);
    } else {
        check_bullet(op);
    }
}

/**
 * Expands an explosion. op is a piece of the explosion - this expans it in the
 * different directions. At least that is what I think this does.
 * @param op piece of explosion expanding.
 */
static void explosion(object *op) {
    object *tmp;
    mapstruct *m = op->map;
    int i;

    if (--(op->duration) < 0) {
        object_remove(op);
        object_free_drop_inventory(op);
        return;
    }
    hit_map(op, 0, op->attacktype, 0);
//     if (op->weight)
    check_spell_knockback(op);

    if (op->range > 0) {
        for (i = 1; i < 9; i++) {
            sint16 dx, dy;
	    sint16 Dx, Dy;
            dx = op->x+freearr_x[i];
            dy = op->y+freearr_y[i];
            /* ok_to_put_more already does things like checks for walls,
             * out of map, etc.
             */
            if (ok_to_put_more(op->map, dx, dy, op, op->attacktype)) {
                tmp = object_new();
                object_copy(op, tmp);
                tmp->state = 0;
                tmp->speed_left = -0.21;
                tmp->range--;
                tmp->value = 0;
		Dx=dx-op->x;
		Dy=dy-op->y;
		if (Dx==-1 && Dy==-1){
		  tmp->direction=8;
		}
		if (Dx==0 && Dy==-1){
		  tmp->direction=1;
		}
		if (Dx==1 && Dy==-1){
		  tmp->direction=2;
		}
		if (Dx==1 && Dy==0){
		  tmp->direction=3;
		}
		if (Dx==1 && Dy==1){
		  tmp->direction=4;
		}
		if (Dx==0 && Dy==1){
		  tmp->direction=5;
		}
		if (Dx==-1 && Dy==-1){
		  tmp->direction=6;
		}
		if (Dx==-1 && Dy==0){
		  tmp->direction=7;
		}

                object_insert_in_map_at(tmp, m, op, 0, dx, dy);

            }
        }
        /* Reset range so we don't try to propogate anymore.
         * Call object_merge_spell() to see if we can merge with another
         * spell on the space.
         */
        op->range = 0;
        object_merge_spell(op, op->x, op->y);
    }
}

/**
 * Causes cone object 'op' to move a space/hit creatures.
 * @param op cone object moving.
 */
static void move_cone(object *op) {
    int i;
    tag_t tag;

    /* if no map then hit_map will crash so just ignore object */
    if (!op->map) {
        LOG(llevError, "Tried to move_cone object %s without a map.\n", op->name ? op->name : "unknown");
        op->speed = 0;
        object_update_speed(op);
        return;
    }

    /* lava saves it's life, but not yours  :) */
    if (QUERY_FLAG(op, FLAG_LIFESAVE)) {
        hit_map(op, 0, op->attacktype, 0);
        return;
    }

    tag = op->count;
    hit_map(op, 0, op->attacktype, 0);

    /* Check to see if we should push anything.
     * Spell objects with weight push whatever they encounter to some
     * degree.
     */
    if (op->weight)
        check_spell_knockback(op);

    if (object_was_destroyed(op, tag))
        return;

    if ((op->duration--) < 0) {
        object_remove(op);
        object_free_drop_inventory(op);
        return;
    }

    /* Object has hit maximum range, so don't have it move
     * any further.  When the duration above expires,
     * then the object will get removed.
     */
    if (--op->range < 0) {
        op->range = 0;    /* just so it doesn't wrap */
        return;
    }

    for (i = -1; i < 2; i++) {
        sint16 x = op->x+freearr_x[absdir(op->stats.sp+i)];
        sint16 y = op->y+freearr_y[absdir(op->stats.sp+i)];

        if (ok_to_put_more(op->map, x, y, op, op->attacktype)) {
            object *tmp = object_new();

            object_copy(op, tmp);
            tmp->duration = op->duration+1;

            /* Use for spell tracking - see ok_to_put_more() */
            tmp->stats.maxhp = op->stats.maxhp;
            object_insert_in_map_at(tmp, op->map, op, 0, x, y);
            if (tmp->other_arch)
                cone_drop(tmp);
        }
    }
}

/**
 * This handles an exploding bomb.
 * @param op The original bomb object.
 */
static void animate_bomb(object *op) {
    int i;
    object *env, *tmp;
    archetype *at;

    if (op->state != NUM_ANIMATIONS(op)-1)
        return;

    env = object_get_env_recursive(op);

    if (op->env) {
        if (env->map == NULL)
            return;

        object_remove(op);
        if ((op = object_insert_in_map_at(op, env->map, op, 0, env->x, env->y)) == NULL)
            return;
    }

    /* This copies a lot of the code from the fire bullet,
     * but using the cast_bullet isn't really feasible,
     * so just set up the appropriate values.
     */
    at = find_archetype(SPLINT);
    if (at) {
        for (i = 1; i < 9; i++) {
            if (out_of_map(op->map, op->x+freearr_x[i], op->y+freearr_x[i]))
                continue;
            tmp = arch_to_object(at);
            tmp->direction = i;
            tmp->range = op->range;
            tmp->stats.dam = op->stats.dam;
            tmp->duration = op->duration;
            tmp->attacktype = op->attacktype;
            object_copy_owner(tmp, op);
            if (op->skill && op->skill != tmp->skill) {
                if (tmp->skill)
                    free_string(tmp->skill);
                tmp->skill = add_refcount(op->skill);
            }
            object_update_turn_face(tmp);
            object_insert_in_map_at(tmp, op->map, op, 0, op->x+freearr_x[i], op->y+freearr_x[i]);
            ob_process(tmp);
        }
    }

    explode_bullet(op);
}

/**
 * Move a missle object.
 * @param op The missile that needs to be moved.
 */
static void move_missile(object *op) {
    int i, mflags;
    sint16 new_x, new_y;
    mapstruct *m;

    if (op->range-- <= 0) {
        object_remove(op);
        object_free_drop_inventory(op);
        return;
    }

    /* call is required to potentially clean owner, but we don't care for the result */
    object_get_owner(op);

    new_x = op->x+DIRX(op);
    new_y = op->y+DIRY(op);

    mflags = get_map_flags(op->map, &m, new_x, new_y, &new_x, &new_y);

    if (!(mflags&P_OUT_OF_MAP)
    && ((mflags&P_IS_ALIVE) || OB_TYPE_MOVE_BLOCK(op, GET_MAP_MOVE_BLOCK(m, new_x, new_y)))) {
        tag_t tag = op->count;

        hit_map(op, op->direction, AT_MAGIC, 1);
        /* Basically, missile only hits one thing then goes away.
         * we need to remove it if someone hasn't already done so.
         */
        if (!object_was_destroyed(op, tag)) {
            object_remove(op);
            object_free_drop_inventory(op);
        }
        return;
    }

    object_remove(op);
    if (!op->direction || (mflags&P_OUT_OF_MAP)) {
        object_free_drop_inventory(op);
        return;
    }
    i = spell_find_dir(m, new_x, new_y, object_get_owner(op));
    if (i > 0 && i != op->direction) {
        op->direction = adjust_dir(op->direction, i);
        object_update_turn_face(op);
    }
    object_insert_in_map_at(op, m, op, 0, new_x, new_y);
}

/**
 * Handles the actual word of recalling. Called when force in player inventory expires.
 * @param op The word of recall effect activating.
 */
static void execute_word_of_recall(object *op) {
    object *wor = op;

    while (op != NULL && op->type != PLAYER)
        op = op->env;

    if (op != NULL && op->map) {
        if ((get_map_flags(op->map, NULL, op->x, op->y, NULL, NULL)&P_NO_CLERIC) && (!QUERY_FLAG(op, FLAG_WIZCAST)))
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                "You feel something fizzle inside you.");
        else
            enter_exit(op, wor);
    }
    object_remove(wor);
    object_free_drop_inventory(wor);
}

/**
 * This handles ball type spells that just sort of wander about.
 * Note that duration is handled by process_object() in time.c
 * @param op The spell effect.
 */
static void move_ball_spell(object *op) {
    int i, j, dam_save, dir, mflags;
    sint16 nx, ny, hx, hy;
    object *owner;
    mapstruct *m;

    owner = object_get_owner(op);

    /* the following logic makes sure that the ball doesn't move into a wall,
     * and makes sure that it will move along a wall to try and get at it's
     * victim.  The block immediately below more or less chooses a random
     * offset to move the ball, eg, keep it mostly on course, with some
     * deviations.
     */

    dir = 0;
    j = rndm(0, 1);
    for (i = 1; i <= 9; i++) {
        /* i bit 0: alters sign of offset
         * other bits (i/2): absolute value of offset
         */

        int offset = ((i^j)&1) ? (i/2) : -(i/2);
        int tmpdir = absdir(op->direction+offset);

        nx = op->x+freearr_x[tmpdir];
        ny = op->y+freearr_y[tmpdir];
        if (!(get_map_flags(op->map, &m, nx, ny, &nx, &ny)&P_OUT_OF_MAP)
        && !(OB_TYPE_MOVE_BLOCK(op, GET_MAP_MOVE_BLOCK(m, nx, ny)))) {
            dir = tmpdir;
            break;
        }
    }
    if (dir == 0) {
        nx = op->x;
        ny = op->y;
        m = op->map;
    }

    object_remove(op);
    object_insert_in_map_at(op, m, op, 0, nx, ny);

    dam_save = op->stats.dam;  /* save the original dam: we do halfdam on
                                surrounding squares */

    /* loop over current square and neighbors to hit.
     * if this has an other_arch field, we insert that in
     * the surround spaces.
     */
    for (j = 0; j < 9; j++) {
        object *new_ob;

        hx = nx+freearr_x[j];
        hy = ny+freearr_y[j];

        m = op->map;
        mflags = get_map_flags(m, &m, hx, hy, &hx, &hy);

        if (mflags&P_OUT_OF_MAP)
            continue;

        /* first, don't ever, ever hit the owner.  Don't hit out
         * of the map either.
         */

        if ((mflags&P_IS_ALIVE) && (!owner || owner->x != hx || owner->y != hy || !on_same_map(owner, op))) {
            if (j)
                op->stats.dam = dam_save/2;
            hit_map(op, j, op->attacktype, 1);
        }

        /* insert the other arch */
        if (op->other_arch && !(OB_TYPE_MOVE_BLOCK(op, GET_MAP_MOVE_BLOCK(m, hx, hy)))) {
            new_ob = arch_to_object(op->other_arch);
            object_insert_in_map_at(new_ob, m, op, 0, hx, hy);
        }
    }

    /* restore to the center location and damage*/
    op->stats.dam = dam_save;

    i = spell_find_dir(op->map, op->x, op->y, object_get_owner(op));
    if (i >= 0) { /* we have a preferred direction! */
        op->direction = adjust_dir(op->direction, i);
        if (rndm(0, 3) != 0)
            op->direction = adjust_dir(op->direction, i);
        if (rndm(0, 3) == 0)
            op->direction = adjust_dir(op->direction, i);
    }
}

/*
 * This is an implementation of the swarm spell. It was written for meteor
 * swarm, but it could be used for any swarm. A swarm spell is a special type
 * of object that casts swarms of other types of spells. Which spell it casts
 * is flexible. It fires the spells from a set of squares surrounding the
 * caster, in a given direction.
 * @param op The spell effect.
 */
static void move_swarm_spell(object *op) {
    static int cardinal_adjust[9] = { -3, -2, -1, 0, 0, 0, 1, 2, 3 };
    static int diagonal_adjust[10] = { -3, -2, -2, -1, 0, 0, 1, 2, 2, 3 };
    sint16 target_x, target_y, origin_x, origin_y;
    int basedir, adjustdir;
    mapstruct *m;
    object *owner;

    owner = object_get_owner(op);
    if (op->duration == 0 || owner == NULL || owner->x != op->x || owner->y != op->y) {
        object_remove(op);
        object_free_drop_inventory(op);
        return;
    }
    op->duration--;

    basedir = op->direction;
    if (basedir == 0) {
        /* spray in all directions! 8) */
        basedir = get_random_dir();
    }

    /* new offset calculation to make swarm element distribution
     * more uniform
     */
    if (op->duration) {
        if (basedir&1) {
            adjustdir = cardinal_adjust[rndm(0, 8)];
        } else {
            adjustdir = diagonal_adjust[rndm(0, 9)];
        }
    } else {
        adjustdir = 0;  /* fire the last one from forward. */
    }

    target_x = op->x+freearr_x[absdir(basedir+adjustdir)];
    target_y = op->y+freearr_y[absdir(basedir+adjustdir)];

    /* back up one space so we can hit point-blank targets, but this
     * necessitates extra out_of_map check below
     */
    origin_x = target_x-freearr_x[basedir];
    origin_y = target_y-freearr_y[basedir];


    /* spell pointer is set up for the spell this casts.  Since this
     * should just be a pointer to the spell in some inventory,
     * it is unlikely to disappear by the time we need it.  However,
     * do some sanity checking anyways.
     */

    if (op->spell && op->spell->type == SPELL && !(get_map_flags(op->map, &m, target_x, target_y, &target_x, &target_y)&P_OUT_OF_MAP)) {
        /* Bullet spells have a bunch more customization that needs to be done */
        if (op->spell->subtype == SP_BULLET)
            fire_arch_from_position(owner, op, origin_x+freearr_x[basedir], origin_y+freearr_y[basedir], basedir, op->spell);
        else if (op->spell->subtype == SP_MAGIC_MISSILE)
            fire_arch_from_position(owner, op, origin_x, origin_y, basedir, op->spell);
    }
}

/**
 * Process an aura. An aura is a part of someone's inventory,
 * which he carries with him, but which acts on the map immediately
 * around him.
 * Aura parameters:
 * duration:  duration counter.
 * attacktype:  aura's attacktype
 * other_arch:  archetype to drop where we attack
 * @param aura The spell effect.
 */
static void move_aura(object *aura) {
    int i, mflags;
    object *env;
    mapstruct *m;

    /* auras belong in inventories */
    env = aura->env;

    /* no matter what we've gotta remove the aura...
     * we'll put it back if its time isn't up.
     */
    object_remove(aura);

    /* exit if we're out of gas */
    if (aura->duration-- < 0) {
        object_free_drop_inventory(aura);
        return;
    }

    /* auras only exist in inventories */
    if (env == NULL || env->map == NULL) {
        object_free_drop_inventory(aura);
        return;
    }

    /* we need to jump out of the inventory for a bit
     * in order to hit the map conveniently.
     */
    object_insert_in_map_at(aura, env->map, aura, 0, env->x, env->y);

    for (i = 1; i < 9; i++) {
        sint16 nx, ny;

        nx = aura->x+freearr_x[i];
        ny = aura->y+freearr_y[i];
        mflags = get_map_flags(env->map, &m, nx, ny, &nx, &ny);

        /* Consider the movement type of the person with the aura as
         * movement type of the aura.  Eg, if the player is flying, the aura
         * is flying also, if player is walking, it is on the ground, etc.
         */
        if (!(mflags&P_OUT_OF_MAP) && !(OB_TYPE_MOVE_BLOCK(env, GET_MAP_MOVE_BLOCK(m, nx, ny)))) {
            hit_map(aura, i, aura->attacktype, 0);

            if (aura->other_arch) {
                object *new_ob;

                new_ob = arch_to_object(aura->other_arch);
                object_insert_in_map_at(new_ob, m, aura, 0, nx, ny);
            }
        }
    }

    /* put the aura back in the player's inventory */
    object_remove(aura);
    object_insert_in_ob(aura, env);
    check_spell_expiry(aura);
}

/**
 * Causes op to fork.
 * @param op original bolt.
 * @param tmp first piece of the fork.
 */
static void forklightning(object *op, object *tmp) {
    int new_dir = 1;  /* direction or -1 for left, +1 for right 0 if no new bolt */
    int t_dir; /* stores temporary dir calculation */
    mapstruct *m;
    sint16 sx, sy;
    object *new_bolt;

    /* pick a fork direction.  tmp->stats.Con is the left bias
     * i.e., the chance in 100 of forking LEFT
     * Should start out at 50, down to 25 for one already going left
     * down to 0 for one going 90 degrees left off original path
     */

    if (rndm(0, 99) < tmp->stats.Con)  /* fork left */
        new_dir = -1;

    /* check the new dir for a wall and in the map*/
    t_dir = absdir(tmp->direction+new_dir);

    if (get_map_flags(tmp->map, &m, tmp->x+freearr_x[t_dir], tmp->y+freearr_y[t_dir], &sx, &sy)&P_OUT_OF_MAP)
        return;

    if (OB_TYPE_MOVE_BLOCK(tmp, GET_MAP_MOVE_BLOCK(m, sx, sy)))
        return;

    /* OK, we made a fork */
    new_bolt = object_new();

    object_copy(tmp, new_bolt);

    /* reduce chances of subsequent forking */
    new_bolt->stats.Dex -= 10;
    tmp->stats.Dex -= 10;  /* less forks from main bolt too */
    new_bolt->stats.Con += 25*new_dir; /* adjust the left bias */
    new_bolt->speed_left = -0.1;
    new_bolt->direction = t_dir;
    new_bolt->duration++;
    new_bolt->stats.dam /= 2;  /* reduce daughter bolt damage */
    new_bolt->stats.dam++;
    tmp->stats.dam /= 2;  /* reduce father bolt damage */
    tmp->stats.dam++;
    new_bolt = object_insert_in_map_at(new_bolt, m, op, 0, sx, sy);
    object_update_turn_face(new_bolt);
}

/**
 * Checks to see if a spell pushes objects as well as flies
 * over and damages them (only used for cones for now)
 * but moved here so it could be applied to bolts too
 * @param op The spell object.
 */
static void check_spell_knockback(object *op) {
    object *tmp, *tmp2; /* object on the map */
    int weight_move;
    int frictionmod = 2; /*poor man's physics - multipy targets weight by this amount */

    if (!op->weight) { /*shouldn't happen but if cone object has no weight drop out*/
        LOG(llevDebug, "DEBUG: arch weighs nothing in check_spell_knockback\n");
        return;
    } else {
        weight_move = op->weight+(op->weight*op->level)/3;
        /*LOG(llevDebug, "DEBUG: arch weighs %d and masses %d (%s,level %d)\n", op->weight, weight_move, op->name, op->level);*/
    }

    for (tmp = GET_MAP_OB(op->map, op->x, op->y); tmp != NULL; tmp = tmp->above) {
        int num_sections = 1;

        /* don't move DM */
        if (QUERY_FLAG(tmp, FLAG_WIZ))
            return;

        /* don't move parts of objects */
        if (tmp->head)
            continue;

        /* don't move floors or immobile objects */
        if (QUERY_FLAG(tmp, FLAG_IS_FLOOR) || QUERY_FLAG(tmp, FLAG_NO_PICK))
            continue;

        /* count the object's sections */
        for (tmp2 = tmp; tmp2 != NULL; tmp2 = tmp2->more)
            num_sections++;

        /* I'm not sure if it makes sense to divide by num_sections - bigger
         * objects should be harder to move, and we are moving the entire
         * object, not just the head, so the total weight should be relevant.
         */

         /* surface area? -tm */

        if (tmp->move_type&MOVE_FLYING)
            frictionmod = 1; /* flying objects loose the friction modifier */
        if (rndm(0, weight_move-1) > ((tmp->weight/num_sections)*frictionmod)) {  /* move it. */
            /* move_object is really for monsters, but looking at
             * the move_object function, it appears that it should
             * also be safe for objects.
             * This does return if successful or not, but
             * I don't see us doing anything useful with that information
             * right now.
             */
// 	    LOG(llevDebug, "trying move\n");
	    if (op->direction){
	      move_object(tmp,absdir(op->direction));
	    }

            else {
	      (move_object(tmp, absdir(op->stats.sp)));

	    }
        }
        else{
// 	  LOG(llevDebug, "did not try move, don't know why\n");
	}
    }
}
