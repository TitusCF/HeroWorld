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
 * This file contains all the spell attack code.  Grouping this code
 * together should hopefully make it easier to find the relevant bits
 * of code.
 *
 * @todo
 * put parameters in the same order, use same name.
 */

#include <global.h>
#include <object.h>
#include <living.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#endif
#include <spells.h>
#include <sounds.h>

/***************************************************************************
 *
 * BOLT CODE
 *
 ***************************************************************************/

/**
 * Cast a bolt-like spell.
 *
 * We remove the magic flag - that can be derived from
 * spob->attacktype.
 * This function sets up the appropriate owner and skill
 * pointers.
 *
 * @param op
 * who is casting the spell.
 * @param caster
 * what object is casting the spell (rod, ...).
 * @param dir
 * firing direction.
 * @param spob
 * spell object for the bolt.
 * @retval 0
 * no bolt could be fired.
 * @retval 1
 * bolt was fired (but may have been destroyed already).
 */
int fire_bolt(object *op, object *caster, int dir, object *spob) {
    object *tmp = NULL;
    int mflags;

    if (!spob->other_arch)
        return 0;

    tmp = arch_to_object(spob->other_arch);
    if (tmp == NULL)
        return 0;

    /*  peterm:  level dependency for bolts  */
    tmp->stats.dam = spob->stats.dam+SP_level_dam_adjust(caster, spob);
    tmp->attacktype = spob->attacktype;
    if (spob->slaying)
        tmp->slaying = add_refcount(spob->slaying);
    tmp->range = spob->range+SP_level_range_adjust(caster, spob);
    tmp->duration = spob->duration+SP_level_duration_adjust(caster, spob);
    tmp->stats.Dex = spob->stats.Dex;
    tmp->stats.Con = spob->stats.Con;

    tmp->direction = dir;
    object_update_turn_face(tmp);

    object_set_owner(tmp, op);
    set_spell_skill(op, caster, spob, tmp);

    mflags = get_map_flags(op->map, &tmp->map, op->x+DIRX(tmp), op->y+DIRY(tmp), &tmp->x, &tmp->y);
    if (mflags&P_OUT_OF_MAP) {
        object_free_drop_inventory(tmp);
        return 0;
    }
    if (OB_TYPE_MOVE_BLOCK(tmp, GET_MAP_MOVE_BLOCK(tmp->map, tmp->x, tmp->y))) {
        if (!QUERY_FLAG(tmp, FLAG_REFLECTING)) {
            object_free_drop_inventory(tmp);
            return 0;
        }
        tmp->direction = absdir(tmp->direction+4);
        tmp->map = op->map;
        tmp->x = op->x;
        tmp->y = op->y;
    }
    tmp = object_insert_in_map_at(tmp, op->map, op, 0, tmp->x, tmp->y);
    if (tmp != NULL)
        ob_process(tmp);
    return 1;
}

/***************************************************************************
 *
 * BULLET/BALL CODE
 *
 ***************************************************************************/

/**
 * Causes an object to explode, eg, a firebullet, poison cloud ball, etc.
 *
 * @param op
 * the object to explode.
 */
void explode_bullet(object *op) {
    tag_t op_tag = op->count;
    object *tmp, *owner;

    if (op->other_arch == NULL) {
        LOG(llevError, "BUG: explode_bullet(): op without other_arch\n");
        object_remove(op);
        object_free_drop_inventory(op);
        return;
    }

    if (op->env) {
        object *env;

        env = object_get_env_recursive(op);
        if (env->map == NULL || out_of_map(env->map, env->x, env->y)) {
            LOG(llevError, "BUG: explode_bullet(): env out of map\n");
            object_remove(op);
            object_free_drop_inventory(op);
            return;
        }
        object_remove(op);
        object_insert_in_map_at(op, env->map, op, INS_NO_MERGE|INS_NO_WALK_ON, env->x, env->y);
    } else if (out_of_map(op->map, op->x, op->y)) {
        LOG(llevError, "BUG: explode_bullet(): op out of map\n");
        object_remove(op);
        object_free_drop_inventory(op);
        return;
    }

    if (op->attacktype) {
        hit_map(op, 0, op->attacktype, 1);
        if (object_was_destroyed(op, op_tag))
            return;
    }

    /* other_arch contains what this explodes into */
    tmp = arch_to_object(op->other_arch);

    object_copy_owner(tmp, op);
    if (tmp->skill)
        FREE_AND_CLEAR_STR(tmp->skill);
    if (op->skill)
        tmp->skill = add_refcount(op->skill);

    owner = object_get_owner(op);
    if ((tmp->attacktype&AT_HOLYWORD || tmp->attacktype&AT_GODPOWER) && owner && !tailor_god_spell(tmp, owner)) {
        object_remove(op);
        object_free_drop_inventory(op);
        return;
    }

    /* special for bombs - it actually has sane values for these */
    if (op->type == SPELL_EFFECT && op->subtype == SP_BOMB) {
        tmp->attacktype = op->attacktype;
        tmp->range = op->range;
        tmp->stats.dam = op->stats.dam;
        tmp->duration = op->duration;
    } else {
        if (op->attacktype&AT_MAGIC)
            tmp->attacktype |= AT_MAGIC;
        /* Spell doc describes what is going on here */
        tmp->stats.dam = op->dam_modifier;
        tmp->range = op->stats.maxhp;
        tmp->duration = op->stats.hp;
    }

    /* Used for spell tracking - just need a unique val for this spell -
     * the count of the parent should work fine.
     */
    tmp->stats.maxhp = op->count;
    if (tmp->stats.maxhp == 0)
        tmp->stats.maxhp = 1;

    /* Set direction of cone explosion */
    if (tmp->type == SPELL_EFFECT && tmp->subtype == SP_CONE)
        tmp->stats.sp = op->direction;

    /* Prevent recursion */
    op->move_on = 0;

    object_insert_in_map_at(tmp, op->map, op, 0, op->x, op->y);
    /* remove the firebullet */
    if (!object_was_destroyed(op, op_tag)) {
        object_remove(op);
        object_free_drop_inventory(op);
    }
}

/**
 * Checks to see what op should do, given the space it is on (eg, explode,
 * damage player, etc).
 *
 * @param op
 * object to check.
 */
void check_bullet(object *op) {
    tag_t op_tag = op->count, tmp_tag;
    int dam, mflags;
    mapstruct *m;
    sint16 sx, sy;

    mflags = get_map_flags(op->map, &m, op->x, op->y, &sx, &sy);

    if (!(mflags&P_IS_ALIVE) && !OB_TYPE_MOVE_BLOCK(op, GET_MAP_MOVE_BLOCK(m, sx, sy)))
        return;

    if (op->other_arch) {
        /* explode object will also remove op */
        explode_bullet(op);
        return;
    }

    /* If nothing alive on this space, no reason to do anything further */
    if (!(mflags&P_IS_ALIVE))
        return;

    FOR_MAP_PREPARE(op->map, op->x, op->y, tmp) {
        if (QUERY_FLAG(tmp, FLAG_ALIVE)) {
            tmp_tag = tmp->count;
            dam = hit_player(tmp, op->stats.dam, op, op->attacktype, 1);
            if (object_was_destroyed(op, op_tag) || !object_was_destroyed(tmp, tmp_tag) || (op->stats.dam -= dam) < 0) {
                if (!QUERY_FLAG(op, FLAG_REMOVED)) {
                    object_remove(op);
                    object_free_drop_inventory(op);
                    return;
                }
            }
        }
    } FOR_MAP_FINISH();
}

/*****************************************************************************
 *
 * CONE RELATED FUNCTIONS
 *
 *****************************************************************************/

/**
 * Drops an object based on what is in the cone's "other_arch".
 *
 * @param op
 * what object should drop.
 */
void cone_drop(object *op) {
    object *new_ob = arch_to_object(op->other_arch);

    new_ob->level = op->level;
    object_set_owner(new_ob, object_get_owner(op));

    /* preserve skill ownership */
    if (op->skill && op->skill != new_ob->skill) {
        if (new_ob->skill)
            free_string(new_ob->skill);
        new_ob->skill = add_refcount(op->skill);
    }
    object_insert_in_map_at(new_ob, op->map, op, 0, op->x, op->y);
}

/**
 * Casts a cone spell.
 *
 * @param op
 * person firing the object.
 * @param caster
 * object casting the spell.
 * @param dir
 * direction to fire in.
 * @param spell
 * spell that is being fired.  It uses other_arch for the archetype
 * to fire.
 * @retval 0
 * couldn't cast.
 * @retval 1
 * successful cast.
 */
int cast_cone(object *op, object *caster, int dir, object *spell) {
    object *tmp;
    int i, success = 0, range_min = -1, range_max = 1;
    mapstruct *m;
    sint16 sx, sy;
    MoveType movetype;

    if (!spell->other_arch)
        return 0;

    if (op->type == PLAYER && QUERY_FLAG(op, FLAG_UNDEAD) && op->attacktype&AT_TURN_UNDEAD) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR, "Your undead nature prevents you from turning undead!");
        return 0;
    }

    if (!dir) {
        range_min = 0;
        range_max = 8;
    }

    /* Need to know what the movetype of the object we are about
     * to create is, so we can know if the space we are about to
     * insert it into is blocked.
     */
    movetype = spell->other_arch->clone.move_type;

    for (i = range_min; i <= range_max; i++) {
        sint16 x, y, d;

        /* We can't use absdir here, because it never returns
         * 0.  If this is a rune, we want to hit the person on top
         * of the trap (d==0).  If it is not a rune, then we don't want
         * to hit that person.
         */
        d = dir+i;
        while (d < 0)
            d += 8;
        while (d > 8)
            d -= 8;

        /* If it's not a rune, we don't want to blast the caster.
         * In that case, we have to see - if dir is specified,
         * turn this into direction 8.  If dir is not specified (all
         * direction) skip - otherwise, one line would do more damage
         * because 0 direction will go through 9 directions - necessary
         * for the rune code.
         */
        if (caster->type != RUNE && d == 0) {
            if (dir != 0)
                d = 8;
            else
                continue;
        }

        x = op->x+freearr_x[d];
        y = op->y+freearr_y[d];

        if (get_map_flags(op->map, &m, x, y, &sx, &sy)&P_OUT_OF_MAP)
            continue;

        if ((movetype&GET_MAP_MOVE_BLOCK(m, sx, sy)) == movetype)
            continue;

        success = 1;
        tmp = arch_to_object(spell->other_arch);
        object_set_owner(tmp, op);
        set_spell_skill(op, caster, spell, tmp);
        tmp->level = caster_level(caster, spell);
        tmp->attacktype = spell->attacktype;

        /* holy word stuff */
        if ((tmp->attacktype&AT_HOLYWORD) || (tmp->attacktype&AT_GODPOWER)) {
            if (!tailor_god_spell(tmp, op))
                return 0;
        }

        if (dir)
            tmp->stats.sp = dir;
        else
            tmp->stats.sp = i;

        tmp->range = spell->range+SP_level_range_adjust(caster, spell);

        /* If casting it in all directions, it doesn't go as far */
        if (dir == 0) {
            tmp->range /= 4;
            if (tmp->range < 2 && spell->range >= 2)
                tmp->range = 2;
        }
        tmp->stats.dam = spell->stats.dam+SP_level_dam_adjust(caster, spell);
        tmp->duration = spell->duration+SP_level_duration_adjust(caster, spell);

        /* Special bonus for fear attacks */
        if (tmp->attacktype&AT_FEAR) {
            if (caster->type == PLAYER)
                tmp->duration += get_fear_bonus(caster->stats.Cha);
            else
                tmp->duration += caster->level/3;
        }
        if (tmp->attacktype&(AT_HOLYWORD|AT_TURN_UNDEAD)) {
            if (caster->type == PLAYER)
                tmp->duration += get_turn_bonus(caster->stats.Wis)/5;
            else
                tmp->duration += caster->level/3;
        }

        if (!(tmp->move_type&MOVE_FLY_LOW))
            LOG(llevDebug, "cast_cone(): arch %s doesn't have flying 1\n", spell->other_arch->name);

        if (!tmp->move_on && tmp->stats.dam) {
            LOG(llevDebug, "cast_cone(): arch %s doesn't have move_on set\n", spell->other_arch->name);
        }

        /* This is used for tracking spells so that one effect doesn't hit
         * a single space too many times.
         */
        tmp->stats.maxhp = tmp->count;
        if (tmp->stats.maxhp == 0)
            tmp->stats.maxhp = 1;

        object_insert_in_map_at(tmp, m, op, 0, sx, sy);

        if (tmp->other_arch)
            cone_drop(tmp);
    }
    return success;
}

/****************************************************************************
 *
 * BOMB related code
 *
 ****************************************************************************/

/**
 * Create a bomb.
 *
 * @param op
 * who is casting.
 * @param caster
 * what object is casting.
 * @param dir
 * cast direction.
 * @param spell
 * spell object to cast.
 * @retval 0
 * no bomb was placed.
 * @retval 1
 * bomb was placed on map.
 */
int create_bomb(object *op, object *caster, int dir, object *spell) {
    object *tmp;
    int mflags;
    sint16 dx = op->x+freearr_x[dir], dy = op->y+freearr_y[dir];
    mapstruct *m;

    mflags = get_map_flags(op->map, &m, dx, dy, &dx, &dy);
    if ((mflags&P_OUT_OF_MAP) || (GET_MAP_MOVE_BLOCK(m, dx, dy)&MOVE_WALK)) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR, "There is something in the way.");
        return 0;
    }
    tmp = arch_to_object(spell->other_arch);

    /*  level dependencies for bomb  */
    tmp->range = spell->range+SP_level_range_adjust(caster, spell);
    tmp->stats.dam = spell->stats.dam+SP_level_dam_adjust(caster, spell);
    tmp->duration = spell->duration+SP_level_duration_adjust(caster, spell);
    tmp->attacktype = spell->attacktype;

    object_set_owner(tmp, op);
    set_spell_skill(op, caster, spell, tmp);
    object_insert_in_map_at(tmp, m, op, 0, dx, dy);
    return 1;
}

/****************************************************************************
 *
 * smite related spell code.
 *
 ****************************************************************************/

/**
 * Returns the pointer to the first monster in the direction which is pointed to by op.
 *
 * This is used by finger of death and the 'smite' spells.
 *
 * @author b.t.
 * @param op
 * caster - really only used for the source location.
 * @param dir
 * direction to look in.
 * @param range
 * how far out to look.
 * @param type
 * type of spell - either ::SPELL_MANA or ::SPELL_GRACE.
 * This info is used for blocked magic/unholy spaces.
 * @return
 * suitable victim, or NULL if none was found.
 */
static object *get_pointed_target(object *op, int dir, int range, int type) {
    object *target;
    sint16 x, y;
    int dist, mflags;
    mapstruct *mp;

    if (dir == 0)
        return NULL;

    for (dist = 1; dist < range; dist++) {
        x = op->x+freearr_x[dir]*dist;
        y = op->y+freearr_y[dir]*dist;
        mp = op->map;
        mflags = get_map_flags(op->map, &mp, x, y, &x, &y);

        if (mflags&P_OUT_OF_MAP)
            return NULL;
        if ((type&SPELL_MANA) && (mflags&P_NO_MAGIC))
            return NULL;
        if ((type&SPELL_GRACE) && (mflags&P_NO_CLERIC))
            return NULL;
        if (GET_MAP_MOVE_BLOCK(mp, x, y)&MOVE_FLY_LOW)
            return NULL;

        if (mflags&P_IS_ALIVE) {
            target = map_find_by_flag(mp, x, y, FLAG_MONSTER);
            if (target != NULL) {
                return target;
            }
        }
    }
    return NULL;
}

/**
 * The priest points to a creature and causes a 'godly curse' to descend.
 *
 * @param op
 * who is casting.
 * @param caster
 * what object is casting.
 * @param dir
 * cast direction.
 * @param spell
 * spell object to cast.
 * @retval 0
 * spell had no effect.
 * @retval 1
 * something was affected by the spell.
 */
int cast_smite_spell(object *op, object *caster, int dir, object *spell) {
    object *effect, *target;
    const object *god = find_god(determine_god(op));
    int range;

    range = spell->range+SP_level_range_adjust(caster, spell);
    target = get_pointed_target(op, dir, range, spell->stats.grace ? SPELL_GRACE : SPELL_MANA);

    /* Bunch of conditions for casting this spell.  Note that only
     * require a god if this is a cleric spell (requires grace).
     * This makes this spell much more general purpose - it can be used
     * by wizards also, which is good, because I think this is a very
     * interesting spell.
     * if it is a cleric spell, you need a god, and the creature
     * can't be friendly to your god.
     */

    if (!target
        || QUERY_FLAG(target, FLAG_REFL_SPELL)
        || (!god && spell->stats.grace)
        || (target->title && god && !strcmp(target->title, god->name))
        || (target->race && god && strstr(target->race, god->race))) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE, "Your request is unheeded.");
        return 0;
    }

    if (spell->other_arch)
        effect = arch_to_object(spell->other_arch);
    else
        return 0;

    /* tailor the effect by priest level and worshipped God */
    effect->level = caster_level(caster, spell);
    effect->attacktype = spell->attacktype;
    if (effect->attacktype&(AT_HOLYWORD|AT_GODPOWER)) {
        if (tailor_god_spell(effect, op))
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_SUCCESS,
                                 "%s answers your call!",
                                 determine_god(op));
        else {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE, "Your request is ignored.");
            return 0;
        }
    }

    /* size of the area of destruction */
    effect->range = spell->range+SP_level_range_adjust(caster, spell);
    effect->duration = spell->duration+SP_level_range_adjust(caster, spell);

    if (effect->attacktype&AT_DEATH) {
        effect->level = spell->stats.dam+SP_level_dam_adjust(caster, spell);

        /* casting death spells at undead isn't a good thing */
        if QUERY_FLAG(target, FLAG_UNDEAD) {
            if (random_roll(0, 2, op, PREFER_LOW)) {
                draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE, "Idiot! Your spell boomerangs!");
                effect->x = op->x;
                effect->y = op->y;
            } else {
                char target_name[HUGE_BUF];

                query_name(target, target_name, HUGE_BUF);
                draw_ext_info_format(NDI_UNIQUE, 0, op,
                                     MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                                     "The %s looks stronger!",
                                     target_name);
                target->stats.hp = target->stats.maxhp*2;
                object_free_drop_inventory(effect);
                return 0;
            }
        }
    } else {
        /* how much woe to inflict :) */
        effect->stats.dam = spell->stats.dam+SP_level_dam_adjust(caster, spell);
    }

    if (effect->type == SPELL_EFFECT && effect->subtype == SP_EXPLOSION) {
        /* Used for spell tracking - just need a unique val for this spell -
         * the count of the parent should work fine.
         *
         * Without this the server can easily get overloaded at high level
         * spells.
         */
        effect->stats.maxhp = spell->count;
        if (effect->stats.maxhp == 0)
            effect->stats.maxhp = 1;
    }

    object_set_owner(effect, op);
    set_spell_skill(op, caster, spell, effect);

    /* ok, tell it where to be, and insert! */
    object_insert_in_map_at(effect, target->map, op, 0, target->x, target->y);

    return 1;
}

/****************************************************************************
 * Destruction
 ****************************************************************************/

/**
 * Makes living objects glow. We do this by creating a force and
 * inserting it in the object.
 *
 * Creatures denied the path of light are unaffected.
 *
 * @param op
 * what to make glow.
 * @param radius
 * glow radius.
 * @param time
 * glow duration. If 0, the object glows permanently.
 * @retval 0
 * nothing happened.
 * @retval 1
 * op is now glowing.
 * @author b.t.
 */
static int make_object_glow(object *op, int radius, int time) {
    object *tmp;

    /* some things are unaffected... */
    if (op->path_denied&PATH_LIGHT)
        return 0;

    tmp = create_archetype(FORCE_NAME);
    tmp->speed = 0.01;
    tmp->stats.food = time;
    SET_FLAG(tmp, FLAG_IS_USED_UP);
    tmp->glow_radius = radius;
    if (tmp->glow_radius > MAX_LIGHT_RADII)
        tmp->glow_radius = MAX_LIGHT_RADII;

    tmp->x = op->x;
    tmp->y = op->y;
    if (tmp->speed < MIN_ACTIVE_SPEED)
        tmp->speed = MIN_ACTIVE_SPEED; /* safety */
    tmp = object_insert_in_ob(tmp, op);
    if (tmp->glow_radius > op->glow_radius)
        op->glow_radius = tmp->glow_radius;

    if (!tmp->env || op != tmp->env) {
        LOG(llevError, "make_object_glow() failed to insert glowing force in %s\n", op->name);
        return 0;
    }
    return 1;
}

/**
 * Hit all monsters around the caster.
 *
 * @param op
 * who is casting.
 * @param caster
 * what object is casting.
 * @param spell_ob
 * spell object to cast.
 * @return
 * 1.
 */
int cast_destruction(object *op, object *caster, object *spell_ob) {
    int i, j, range, mflags, friendly = 0, dam, dur;
    sint16 sx, sy;
    mapstruct *m;
    object *tmp;
    const char *skill;

    range = spell_ob->range+SP_level_range_adjust(caster, spell_ob);
    dam = spell_ob->stats.dam+SP_level_dam_adjust(caster, spell_ob);
    dur = spell_ob->duration+SP_level_duration_adjust(caster, spell_ob);
    if (QUERY_FLAG(op, FLAG_FRIENDLY) || op->type == PLAYER)
        friendly = 1;

    /* destruction doesn't use another spell object, so we need
     * update op's skill pointer so that exp is properly awarded.
     * We do some shortcuts here - since this is just temporary
     * and we'll reset the values back, we don't need to go through
     * the full share string/free_string route.
     */
    skill = op->skill;
    if (caster == op)
        op->skill = spell_ob->skill;
    else if (caster->skill)
        op->skill = caster->skill;
    else
        op->skill = NULL;

    change_skill(op, find_skill_by_name(op, op->skill), 1);

    for (i = -range; i < range; i++) {
        for (j = -range; j < range; j++) {
            m = op->map;
            sx = op->x+i;
            sy = op->y+j;
            mflags = get_map_flags(m, &m, sx, sy, &sx, &sy);
            if (mflags&P_OUT_OF_MAP)
                continue;
            if (mflags&P_IS_ALIVE) {
                tmp = NULL;
                FOR_MAP_PREPARE(m, sx, sy, inv) {
                    tmp = inv;
                    if (QUERY_FLAG(tmp, FLAG_ALIVE) || tmp->type == PLAYER)
                        break;
                } FOR_MAP_FINISH();
                if (tmp) {
                    tmp = HEAD(tmp);
                    if ((friendly && !QUERY_FLAG(tmp, FLAG_FRIENDLY) && tmp->type != PLAYER)
                        || (!friendly && (QUERY_FLAG(tmp, FLAG_FRIENDLY) || tmp->type == PLAYER))) {
                        if (spell_ob->subtype == SP_DESTRUCTION) {
                            hit_player(tmp, dam, op, spell_ob->attacktype, 0);
                            if (spell_ob->other_arch) {
                                tmp = arch_to_object(spell_ob->other_arch);
                                object_insert_in_map_at(tmp, m, op, 0, sx, sy);
                            }
                        } else if (spell_ob->subtype == SP_FAERY_FIRE && tmp->resist[ATNR_MAGIC] != 100) {
                            if (make_object_glow(tmp, 1, dur) && spell_ob->other_arch) {
                                object *effect = arch_to_object(spell_ob->other_arch);
                                object_insert_in_map_at(effect, m, op, 0, sx, sy);
                            }
                        }
                    }
                }
            }
        }
    }
    op->skill = skill;
    return 1;
}

/***************************************************************************
 *
 * CURSE
 *
 ***************************************************************************/

/**
 * Curse an object, reducing its statistics.
 *
 * @param op
 * who is casting.
 * @param caster
 * what object is casting.
 * @param spell_ob
 * spell object to cast.
 * @param dir
 * cast direction.
 * @retval 0
 * curse had no effect.
 * @retval 1
 * something was cursed.
 */
int cast_curse(object *op, object *caster, object *spell_ob, int dir) {
    const object *god = find_god(determine_god(op));
    object *tmp, *force;

    tmp = get_pointed_target(op, (dir == 0) ? op->direction : dir, spell_ob->range, SPELL_GRACE);
    if (!tmp) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE, "There is no one in that direction to curse.");
        return 0;
    }

    /* If we've already got a force of this type, don't add a new one. */
    force = NULL;
    FOR_INV_PREPARE(tmp, inv) {
        if (inv->type == FORCE && inv->subtype == FORCE_CHANGE_ABILITY)  {
            if (inv->name == spell_ob->name) {
                force = inv;
                break;
            } else if (spell_ob->race && spell_ob->race == inv->name) {
                draw_ext_info_format(NDI_UNIQUE, 0, op,
                                     MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                                     "You can not cast %s while %s is in effect",
                                     spell_ob->name, force->name_pl);
                return 0;
            }
        }
    } FOR_INV_FINISH();

    if (force == NULL) {
        force = create_archetype(FORCE_NAME);
        force->subtype = FORCE_CHANGE_ABILITY;
        free_string(force->name);
        if (spell_ob->race)
            force->name = add_refcount(spell_ob->race);
        else
            force->name = add_refcount(spell_ob->name);
        free_string(force->name_pl);
        force->name_pl = add_refcount(spell_ob->name);
    } else {
        int duration;

        duration = spell_ob->duration+SP_level_duration_adjust(caster, spell_ob)*50;
        if (duration > force->duration) {
            force->duration = duration;
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_SUCCESS, "You recast the spell while in effect.");
        } else {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE, "Recasting the spell had no effect.");
        }
        return 1;
    }
    force->duration = spell_ob->duration+SP_level_duration_adjust(caster, spell_ob)*50;
    force->speed = 1.0;
    force->speed_left = -1.0;
    SET_FLAG(force, FLAG_APPLIED);

    if (god) {
        if (spell_ob->last_grace)
            force->path_repelled = god->path_repelled;
        if (spell_ob->last_grace)
            force->path_denied = god->path_denied;
        draw_ext_info_format(NDI_UNIQUE, 0, tmp, MSG_TYPE_VICTIM, MSG_TYPE_VICTIM_SPELL,
                             "You are a victim of %s's curse!",
                             god->name);
    } else
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE, "Your curse seems empty.");


    if (tmp != op && op->type == PLAYER)
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_SUCCESS,
                             "You curse %s!",
                             tmp->name);

    force->stats.ac = spell_ob->stats.ac;
    force->stats.wc = spell_ob->stats.wc;

    change_abil(tmp, force);     /* Mostly to display any messages */
    object_insert_in_ob(force, tmp);
    fix_object(tmp);

    if (spell_ob->other_arch != NULL && tmp->map != NULL) {
        object_insert_in_map_at(arch_to_object(spell_ob->other_arch), tmp->map, NULL, INS_ON_TOP, tmp->x, tmp->y);
    }

    return 1;
}

/**********************************************************************
 * mood change
 * Arguably, this may or may not be an attack spell.  But since it
 * effects monsters, it seems best to put it into this file
 ***********************************************************************/

/**
 * This covers the various spells that change the moods of monsters - makes
 * them angry, peaceful, friendly, etc.
 *
 * @param op
 * who is casting.
 * @param caster
 * what object is casting.
 * @param spell
 * spell object to cast.
 * @return
 * 1.
 */
int mood_change(object *op, object *caster, object *spell) {
    object *tmp, *head;
    const object *god;
    int done_one, range, mflags, level, at, best_at, immunity_chance = 50;
    sint16 x, y, nx, ny;
    mapstruct *m;
    const char *race;

    /* We pre-compute some values here so that we don't have to keep
     * doing it over and over again.
     */
    god = find_god(determine_god(op));
    level = caster_level(caster, spell);
    range = spell->range+SP_level_range_adjust(caster, spell);
    race = object_get_value(spell, "immunity_chance");
    if (race != NULL) {
        immunity_chance = atoi(race);
        if (immunity_chance < 0 || immunity_chance > 100) {
            LOG(llevError, "ignoring invalid immunity_chance %d for %s\n", immunity_chance, spell->arch->name);
            immunity_chance = 50;
        }
    }

    /* On the bright side, no monster should ever have a race of GOD_...
     * so even if the player doesn't worship a god, if race=GOD_.., it
     * won't ever match anything.
     */
    if (!spell->race)
        race = NULL;
    else if (god && !strcmp(spell->race, "GOD_SLAYING"))
        race = god->slaying;
    else if (god && !strcmp(spell->race, "GOD_FRIEND"))
        race = god->race;
    else
        race = spell->race;

    for (x = op->x-range; x <= op->x+range; x++)
        for (y = op->y-range; y <= op->y+range; y++) {
            done_one = 0;
            m = op->map;
            mflags = get_map_flags(m, &m, x, y, &nx, &ny);
            if (mflags&P_OUT_OF_MAP)
                continue;

            /* If there is nothing living on this space, no need to go further */
            if (!(mflags&P_IS_ALIVE))
                continue;

            head = map_find_by_flag(m, nx, ny, FLAG_MONSTER);
            /* There can be living objects that are not monsters */
            if (!head || head->type == PLAYER)
                continue;

            /* Make sure the race is OK.  Likewise, only effect undead if spell specifically allows it */
            if (race && head->race && !strstr(race, head->race))
                continue;
            if (QUERY_FLAG(head, FLAG_UNDEAD) && !QUERY_FLAG(spell, FLAG_UNDEAD))
                continue;

            /* Now do a bunch of stuff related to saving throws */
            best_at = -1;
            if (spell->attacktype) {
                for (at = 0; at < NROFATTACKS; at++)
                    if (spell->attacktype&(1<<at))
                        if (best_at == -1 || head->resist[at] > head->resist[best_at])
                            best_at = at;

                if (best_at == -1)
                    at = 0;
                else {
                    if (head->resist[best_at] == 100)
                        continue;
                    else
                        at = head->resist[best_at]/5;
                }
                at -= level/5;
                if (did_make_save(head, head->level, at))
                    continue;
            } else {   /* spell->attacktype */
                const char *value;

                /*
                 * Spell has no attacktype (charm&such), so we'll have a specific saving:
                 * if spell level < monster level, no go
                 * else, chance of effect = 20+min(50, 2*(spell level-monster level))
                 *
                 * The chance will then be in the range [20-70] percent, not too bad.
                 *
                 * This is required to fix the 'charm monster' abuse, where a player level 1 can
                 * charm a level 125 monster...
                 *
                 * Ryo, august 14th
                 */
                if (head->level > level)
                    continue;
                if (random_roll(0, 100, caster, PREFER_LOW) >= (20+MIN(50, 2*(level-head->level)))) {
                    /* Additionnally, randomly make the monster immune to that spell. */
                    if (random_roll(0, 100, caster, PREFER_HIGH) <= immunity_chance) {
                        object_set_value(head, "no_mood_change", "1", 1);
                    }
                    continue;
                }

                /*
                 * There was no way to ensure immunity, so added a key/value for that.
                 * Nicolas, september 2010.
                 */
                value = object_get_value(head, "no_mood_change");
                if (value && strcmp(value, "1") == 0)
                    continue;
            }

            /* Done with saving throw.  Now start effecting the monster */

            /* aggravation */
            if (QUERY_FLAG(spell, FLAG_MONSTER)) {
                CLEAR_FLAG(head, FLAG_SLEEP);
                if (QUERY_FLAG(head, FLAG_FRIENDLY))
                    remove_friendly_object(head);

                done_one = 1;
                object_set_enemy(head, op);
            }

            /* calm monsters */
            if (QUERY_FLAG(spell, FLAG_UNAGGRESSIVE) && !QUERY_FLAG(head, FLAG_UNAGGRESSIVE)) {
                SET_FLAG(head, FLAG_UNAGGRESSIVE);
                object_set_enemy(head, NULL);
                done_one = 1;
            }

            /* berserk monsters */
            if (QUERY_FLAG(spell, FLAG_BERSERK) && !QUERY_FLAG(head, FLAG_BERSERK)) {
                SET_FLAG(head, FLAG_BERSERK);
                done_one = 1;
            }
            /* charm */
            if (QUERY_FLAG(spell, FLAG_NO_ATTACK) && !QUERY_FLAG(head, FLAG_FRIENDLY)) {
                SET_FLAG(head, FLAG_FRIENDLY);
                /* Prevent uncontrolled outbreaks of self replicating monsters.
                   Typical use case is charm, go somewhere, use aggravation to make hostile.
                   This could lead to fun stuff like mice outbreak in bigworld and server crawl. */
                CLEAR_FLAG(head, FLAG_GENERATOR);
                object_set_owner(head, op);
                set_spell_skill(op, caster, spell, head);
                add_friendly_object(head);
                head->attack_movement = PETMOVE;
                done_one = 1;
                share_exp(op, head->stats.exp/2, head->skill, SK_EXP_ADD_SKILL);
                head->stats.exp = 0;
            }

            /* If a monster was affected, put an effect in */
            if (done_one && spell->other_arch) {
                tmp = arch_to_object(spell->other_arch);
                object_insert_in_map_at(tmp, m, op, 0, nx, ny);
            }
        } /* for y */

    return 1;
}


/**
 * The following routine creates a swarm of objects. It actually sets up a
 * specific swarm object, which then fires off all the parts of the swarm.
 *
 * @param op
 * who is casting.
 * @param caster
 * what object is casting.
 * @param spell
 * spell object to cast.
 * @param dir
 * cast direction.
 * @retval 0
 * nothing happened.
 * @retval 1
 * swarm was placed on map.
 */
int fire_swarm(object *op, object *caster, object *spell, int dir) {
    object *tmp;
    int i;

    if (!spell->other_arch)
        return 0;

    tmp = create_archetype(SWARM_SPELL);
    object_set_owner(tmp, op);       /* needed so that if swarm elements kill, caster gets xp.*/
    set_spell_skill(op, caster, spell, tmp);

    tmp->level = caster_level(caster, spell);   /*needed later, to get level dep. right.*/
    tmp->spell = arch_to_object(spell->other_arch);

    tmp->attacktype = tmp->spell->attacktype;

    if (tmp->attacktype&AT_HOLYWORD || tmp->attacktype&AT_GODPOWER) {
        if (!tailor_god_spell(tmp, op))
            return 1;
    }
    tmp->duration = SP_level_duration_adjust(caster, spell);
    for (i = 0; i < spell->duration; i++)
        tmp->duration += die_roll(1, 3, op, PREFER_HIGH);

    tmp->direction = dir;
    tmp->invisible = 1;
    object_insert_in_map_at(tmp, op->map, op, 0, op->x, op->y);
    return 1;
}

/**
 * Illuminates something on a map, or try to blind a living thing.
 *
 * See the spells documentation file for why this is its own function.
 *
 * @param op
 * who is casting.
 * @param caster
 * what object is casting.
 * @param spell
 * spell object to cast.
 * @param dir
 * cast direction.
 * @retval 0
 * no effect.
 * @retval 1
 * lighting successful.
 */
int cast_light(object *op, object *caster, object *spell, int dir) {
    object *target = NULL, *tmp = NULL;
    sint16 x, y;
    int dam, mflags;
    mapstruct *m;

    dam = spell->stats.dam+SP_level_dam_adjust(caster, spell);

    if (!dir) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR, "In what direction?");
        return 0;
    }

    x = op->x+freearr_x[dir];
    y = op->y+freearr_y[dir];
    m = op->map;

    mflags = get_map_flags(m, &m, x, y, &x, &y);

    if (mflags&P_OUT_OF_MAP) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE, "Nothing is there.");
        return 0;
    }

    if (mflags&P_IS_ALIVE && spell->attacktype) {
        target = map_find_by_flag(m, x, y, FLAG_MONSTER);
        if (target != NULL) {
            /* oky doky. got a target monster. Lets make a blinding attack */
            (void)hit_player(target, dam, op, spell->attacktype, 1);
            return 1; /* one success only! */
        }
    }

    /* no live target, perhaps a wall is in the way? */
    if (OB_TYPE_MOVE_BLOCK(op, GET_MAP_MOVE_BLOCK(m, x, y))) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR, "Something is in the way.");
        return 0;
    }

    /* ok, looks groovy to just insert a new light on the map */
    tmp = arch_to_object(spell->other_arch);
    if (!tmp) {
        LOG(llevError, "Error: spell arch for cast_light() missing.\n");
        return 0;
    }
    tmp->stats.food = spell->duration+SP_level_duration_adjust(caster, spell);
    if (tmp->glow_radius) {
        tmp->glow_radius = spell->range+SP_level_range_adjust(caster, spell);
        if (tmp->glow_radius > MAX_LIGHT_RADII)
            tmp->glow_radius = MAX_LIGHT_RADII;
    }
    object_insert_in_map_at(tmp, m, op, 0, x, y);
    return 1;
}

/**
 * Let's try to infect something.
 *
 * @param op
 * who is casting.
 * @param caster
 * what object is casting.
 * @param spell
 * spell object to cast.
 * @param dir
 * cast direction.
 * @retval 0
 * no one caught anything.
 * @retval 1
 * at least one living was affected.
 */
int cast_cause_disease(object *op, object *caster, object *spell, int dir) {
    sint16 x, y;
    int i, mflags, range, dam_mod, dur_mod;
    object *target_head;
    mapstruct *m;

    x = op->x;
    y = op->y;

    /* If casting from a scroll, no direction will be available, so refer to the
     * direction the player is pointing.
     */
    if (!dir)
        dir = op->facing;
    if (!dir)
        return 0;     /* won't find anything if casting on ourself, so just return */

    /* Calculate these once here */
    range = spell->range+SP_level_range_adjust(caster, spell);
    dam_mod = SP_level_dam_adjust(caster, spell);
    dur_mod = SP_level_duration_adjust(caster, spell);

    /* search in a line for a victim */
    for (i = 1; i < range; i++) {
        x = op->x+i*freearr_x[dir];
        y = op->y+i*freearr_y[dir];
        m = op->map;

        mflags = get_map_flags(m, &m, x, y, &x, &y);

        if (mflags&P_OUT_OF_MAP)
            return 0;

        /* don't go through walls - presume diseases are airborne */
        if (GET_MAP_MOVE_BLOCK(m, x, y)&MOVE_FLY_LOW)
            return 0;

        /* Only bother looking on this space if there is something living here */
        if (mflags&P_IS_ALIVE) {
            /* search this square for a victim */
            FOR_MAP_PREPARE(m, x, y, walk) {
                /* Flags for monster is set on head only, so get it now */
                target_head = HEAD(walk);
                if (QUERY_FLAG(target_head, FLAG_MONSTER) || (target_head->type == PLAYER)) {  /* found a victim */
                    object *disease = arch_to_object(spell->other_arch);

                    object_set_owner(disease, op);
                    set_spell_skill(op, caster, spell, disease);
                    disease->stats.exp = 0;
                    disease->level = caster_level(caster, spell);

                    /* do level adjustments */
                    if (disease->stats.wc)
                        disease->stats.wc += dur_mod/2;

                    if (disease->magic > 0)
                        disease->magic += dur_mod/4;

                    if (disease->stats.maxhp > 0)
                        disease->stats.maxhp += dur_mod;

                    if (disease->stats.maxgrace > 0)
                        disease->stats.maxgrace += dur_mod;

                    if (disease->stats.dam) {
                        if (disease->stats.dam > 0)
                            disease->stats.dam += dam_mod;
                        else
                            disease->stats.dam -= dam_mod;
                    }

                    if (disease->last_sp) {
                        disease->last_sp -= 2*dam_mod;
                        if (disease->last_sp < 1)
                            disease->last_sp = 1;
                    }

                    if (disease->stats.maxsp) {
                        if (disease->stats.maxsp > 0)
                            disease->stats.maxsp += dam_mod;
                        else
                            disease->stats.maxsp -= dam_mod;
                    }

                    if (disease->stats.ac)
                        disease->stats.ac += dam_mod;

                    if (disease->last_eat)
                        disease->last_eat -= dam_mod;

                    if (disease->stats.hp)
                        disease->stats.hp -= dam_mod;

                    if (disease->stats.sp)
                        disease->stats.sp -= dam_mod;

                    if (infect_object(target_head, disease, 1)) {
                        object *flash;  /* visual effect for inflicting disease */

                        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_SUCCESS, "You inflict %s on %s!", disease->name, target_head->name);

                        object_free_drop_inventory(disease); /* don't need this one anymore */
                        flash = create_archetype(ARCH_DETECT_MAGIC);
                        object_insert_in_map_at(flash, walk->map, op, 0, x, y);
                        return 1;
                    }
                    object_free_drop_inventory(disease);
                } /* Found a victim */
            } FOR_MAP_FINISH(); /* Search squares for living creature */
        } /* if living creature on square */
    } /* for range of spaces */
    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE, "No one caught anything!");
    return 1;
}
