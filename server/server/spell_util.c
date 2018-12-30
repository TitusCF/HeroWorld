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
 * Spell-related helper functions.
 */

#include <global.h>
#include <spells.h>
#include <object.h>
#include <errno.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#endif
#include <sounds.h>
#include <assert.h>

#include "living.h"

extern const char *const spell_mapping[];

/**
 * This returns a random spell from 'ob'.  If skill is set, then
 * the spell must be of this skill, it can be NULL in which case all
 * matching spells are used.
 *
 * @param ob
 * object to find spells in.
 * @param skill
 * skill the spell should match, NULL if can match any spell.
 * @return
 * random spell, or NULL if none found.
 * @todo change skill to sstring.
 */
object *find_random_spell_in_ob(object *ob, const char *skill) {
    int k = 0, s;

    FOR_INV_PREPARE(ob, tmp)
        if (tmp->type == SPELL && (!skill || tmp->skill == skill))
            k++;
    FOR_INV_FINISH();

    /* No spells, no need to progess further */
    if (!k)
        return NULL;

    s = RANDOM()%k;
    FOR_INV_PREPARE(ob, tmp)
        if (tmp->type == SPELL && (!skill || tmp->skill == skill)) {
            if (!s)
                return tmp;
            else
                s--;
        }
    FOR_INV_FINISH();

    /* Should never get here, but just in case */
    return NULL;
}

/**
 * Utility function to assign the correct skill when casting.
 *
 * Relatively simple function that gets used a lot.
 * Basically, it sets up the skill pointer for the spell being
 * cast.  If op is really casting the spell, then the skill
 * is whatever skill the spell requires.
 * if instead caster (rod, horn, wand, etc) is casting the skill,
 * then they get exp for the skill that you need to use for
 * that object (use magic device).
 *
 * @param op
 * object casting the spell.
 * @param caster
 * object used to cast the spell (rod, wand, ...).
 * @param spob
 * spell object.
 * @param dest
 * object to set the skill for.
 */
void set_spell_skill(object *op, object *caster, object *spob, object *dest) {
    if (dest->skill)
        FREE_AND_CLEAR_STR(dest->skill);
    if (caster == op && spob->skill)
        dest->skill = add_refcount(spob->skill);
    else if (caster->skill)
        dest->skill = add_refcount(caster->skill);
}

/**
 * It goes through the spells looking for any
 * obvious errors.  This was most useful in debugging when re-doing
 * all the spells to catch simple errors.  To use it all the time
 * will result in it spitting out messages that aren't really errors.
 */
void check_spells(void) {
#ifdef SPELL_DEBUG
    int i;
    archetype *at;

    LOG(llevDebug, "Checking spells...\n");

    for (at = first_archetype; at; at = at->next) {
        if (at->clone.type == SPELL) {
            if (at->clone.skill) {
                for (i = 1; i < NUM_SKILLS; i++)
                    if (!strcmp(skill_names[i], at->clone.skill))
                        break;
                if (i == NUM_SKILLS) {
                    LOG(llevError, "Spell %s has improper associated skill %s\n", at->name, at->clone.skill);
                }
            }
            /* other_arch is already checked for in the loader */
        }
    }

    i = 0;
    while (spell_mapping[i]) {
        if (!find_archetype(spell_mapping[i])) {
            LOG(llevError, "Unable to find spell mapping %s (%i)\n", spell_mapping[i], i);
        }
        i++;
    }
    LOG(llevDebug, "Checking spells completed.\n");
#endif
}

/**
 * Dumps all the spells - now also dumps skill associated with the spell.
 * not sure what this would be used for, as the data seems pretty
 * minimal, but easy enough to keep around.
 */
void dump_spells(void) {
    archetype *at;
    int banner = 0;

    for (at = first_archetype; at; at = at->next) {
        if (at->clone.type == SPELL) {
            fprintf(stderr, "%s:%s:%s:%s:%d\n", at->clone.name ? at->clone.name : "null",
                    at->name, at->clone.other_arch ? at->clone.other_arch->name : "null",
                    at->clone.skill ? at->clone.skill : "null", at->clone.level);
        }
    }

    for (at = first_archetype; at; at = at->next) {
        if (at->clone.type == SPELL && at->clone.path_attuned == 0) {
            if (banner == 0) {
                banner = 1;
                fprintf(stderr, "Spells with no path set:\n");
            }

            fprintf(stderr, "- %s\n", at->clone.name ? at->clone.name : "null");
        }
    }
}

/**
 * Inserts into map a spell effect based on other_arch.
 *
 * @param spob
 * spell object to insert object from.
 * @param x
 * @param y
 * @param map
 * coordinates to put the effect at.
 * @param originator
 * what causes the effect to be inserted. Can be NULL.
 */

void spell_effect(object *spob, int x, int y, mapstruct *map, object *originator) {
    if (spob->other_arch !=  NULL) {
        object *effect = arch_to_object(spob->other_arch);
        object_insert_in_map_at(effect, map, originator, 0, x, y);
    }
}

/**
 * This function takes a caster and spell and presents the
 * effective level the caster needs to be to cast the spell.
 * Basically, it just adjusts the spell->level with attuned/repelled
 * spellpaths.
 *
 * @param caster
 * person casting the spell.
 * @param spell
 * spell object.
 * @return
 * adjusted level.
 */
int min_casting_level(const object *caster, const object *spell) {
    int new_level;

    if (caster->path_denied&spell->path_attuned) {
        /* This case is not a bug, just the fact that this function is
         * usually called BEFORE checking for path_deny. -AV
         */
        return 1;
    }
    new_level = spell->level
                +((caster->path_repelled&spell->path_attuned) ? +2 : 0)
                +((caster->path_attuned&spell->path_attuned) ? -2 : 0);
    return MAX(new_level, 1);
}


/**
 * This function returns the effective level the spell
 * is being cast at.
 * Note that I changed the repelled/attuned bonus to 2 from 5.
 * This is because the new code compares casting_level against
 * min_caster_level, so the difference is effectively 4
 *
 * @param caster
 * person casting the spell.
 * @param spell
 * spell object.
 * @return
 * adjusted level.
 */
int caster_level(const object *caster, const object *spell) {
    int level = caster->level;

    /* If this is a player, try to find the matching skill */
    if (caster->type == PLAYER && spell->skill) {
        int i;

        for (i = 0; i < NUM_SKILLS; i++)
            if (caster->contr->last_skill_ob[i]
            && caster->contr->last_skill_ob[i]->skill == spell->skill) {
                level = caster->contr->last_skill_ob[i]->level;
                break;
            }
    }
    /* Got valid caster level.  Now adjust for attunement */
    level += ((caster->path_repelled&spell->path_attuned) ? -2 : 0)
            +((caster->path_attuned&spell->path_attuned) ? 2 : 0);

    /* Always make this at least 1.  If this is zero, we get divide by zero
     * errors in various places.
     */
    if (level < 1)
        level = 1;
    return level;
}

/**
 * Scales the spellpoint cost of a spell by it's increased effectiveness.
 * Some of the lower level spells become incredibly vicious at high
 * levels.  Very cheap mass destruction.  This function is
 * intended to keep the sp cost related to the effectiveness.
 *
 * Note that it is now possible for a spell to cost both grace and
 * mana.  In that case, we return which ever value is higher.
 *
 * @param caster
 * what is casting the spell.
 * @param spell
 * spell object.
 * @param flags
 * one of @ref SPELL_xxx.
 * @return
 * sp/mana points cost.
 */
sint16 SP_level_spellpoint_cost(object *caster, object *spell, int flags) {
    int sp, grace, level = caster_level(caster, spell);

    if (settings.spellpoint_level_depend == TRUE) {
        if (spell->stats.sp && spell->stats.maxsp) {
            sp = (int)(spell->stats.sp*(1.0+MAX(0, (float)(level-spell->level)/(float)spell->stats.maxsp)));
        } else
            sp = spell->stats.sp;

        sp *= PATH_SP_MULT(caster, spell);
        if (!sp && spell->stats.sp)
            sp = 1;

        if (spell->stats.grace && spell->stats.maxgrace) {
            grace = (int)(spell->stats.grace*(1.0+MAX(0, (float)(level-spell->level)/(float)spell->stats.maxgrace)));
        } else
            grace = spell->stats.grace;

        grace *= PATH_SP_MULT(caster, spell);
        if (spell->stats.grace && !grace)
            grace = 1;
    } else {
        sp = spell->stats.sp*PATH_SP_MULT(caster, spell);
        if (spell->stats.sp && !sp)
            sp = 1;
        grace = spell->stats.grace*PATH_SP_MULT(caster, spell);
        if (spell->stats.grace && !grace)
            grace = 1;
    }
    if (flags == SPELL_HIGHEST)
        return MAX(sp, grace);
    else if (flags == SPELL_GRACE)
        return grace;
    else if (flags == SPELL_MANA)
        return sp;
    else {
        LOG(llevError, "SP_level_spellpoint_cost: Unknown flags passed: %d\n", flags);
        return 0;
    }
}

/**
 * Returns adjusted damage based on the caster.
 *
 * @param caster
 * who is casting.
 * @param spob
 * spell we are adjusting.
 * @return
 * adjusted damage.
 */
int SP_level_dam_adjust(const object *caster, const object *spob) {
    int level = caster_level(caster, spob);
    int adj = level-min_casting_level(caster, spob);

    if (adj < 0)
        adj = 0;
    if (spob->dam_modifier)
        adj /= spob->dam_modifier;
    else
        adj = 0;
    return adj;
}

/**
 * Adjust the duration of the spell based on level.
 * This is basically the same as SP_level_dam_adjust() above,
 * but instead looks at the level_modifier value.
 *
 * @param caster
 * who is casting.
 * @param spob
 * spell we are adjusting.
 * @return
 * adjusted duration.
 */
int SP_level_duration_adjust(const object *caster, const object *spob) {
    int level = caster_level(caster, spob);
    int adj = level-min_casting_level(caster, spob);

    if (adj < 0)
        adj = 0;
    if (spob->duration_modifier)
        adj /= spob->duration_modifier;
    else
        adj = 0;

    return adj;
}

/**
 * Adjust the range of the spell based on level.
 * This is basically the same as SP_level_dam_adjust() above,
 * but instead looks at the level_modifier value.
 *
 * @param caster
 * who is casting.
 * @param spob
 * spell we are adjusting.
 * @return
 * adjusted range.
 */
int SP_level_range_adjust(const object *caster, const object *spob) {
    int level = caster_level(caster, spob);
    int adj = level-min_casting_level(caster, spob);

    if (adj < 0)
        adj = 0;
    if (spob->range_modifier)
        adj /= spob->range_modifier;
    else
        adj = 0;

    return adj;
}

/**
 * Returns adjusted wc based on the caster and the spell.
 *
 * @param caster
 * who is casting.
 * @param spob
 * spell we are adjusting.
 * @return
 * adjusted wc (positive is best).
 */
int SP_level_wc_adjust(const object *caster, const object *spob) {
    int level = caster_level(caster, spob);
    int adj = level - min_casting_level(caster, spob), irate;
    sstring rate;

    rate = object_get_value(spob, "wc_increase_rate");

    if (rate == NULL)
        return 0;

    if (adj < 0)
        adj = 0;

    irate = atoi(rate);
    if (irate > 0)
        adj /= irate;
    else
        adj = 0;
    return adj;
}

/**
 * Checks to see if player knows the spell.  If the name is the same
 * as an existing spell, we presume they know it.
 *
 * @param op
 * object we're looking into.
 * @param name
 * spell name. Doesn't need to be a shared string.
 * @return
 * 1 if op knows the spell, 0 if it don't.
 */
object *check_spell_known(object *op, const char *name) {
    return object_find_by_type_and_name(op, SPELL, name);
}

/**
 * Look at object 'op' and see if they know the spell
 * spname. This is pretty close to check_spell_known
 * above, but it uses a looser matching mechanism.
 *
 * @param op
 * object we're searching the inventory.
 * @param spname
 * partial spell name.
 * @returns
 * matching spell object, or NULL. If we match multiple spells but don't get an exact match, we also return NULL.
 */
object *lookup_spell_by_name(object *op, const char *spname) {
    object *spob1 = NULL, *spob2 = NULL;
    int nummatch = 0;

    if (spname == NULL)
        return NULL;

    /* Try to find the spell.  We store the results in spob1
     * and spob2 - spob1 is only taking the length of
     * the past spname, spob2 uses the length of the spell name.
     */
    FOR_INV_PREPARE(op, spob) {
        if (spob->type == SPELL) {
            if (!strncmp(spob->name, spname, strlen(spname))) {
                if (strlen(spname) == strlen(spob->name))
                    /* Perfect match, return it. */
                    return spob;
                nummatch++;
                spob1 = spob;
            } else if (!strncmp(spob->name, spname, strlen(spob->name))) {
                /* if spells have ambiguous names, it makes matching
                 * really difficult.  (eg, fire and fireball would
                 * fall into this category).  It shouldn't be hard to
                 * make sure spell names don't overlap in that fashion.
                 */
                if (spob2)
                    LOG(llevError, "Found multiple spells with overlapping base names: %s, %s\n", spob2->name, spob->name);
                spob2 = spob;
            }
        }
    } FOR_INV_FINISH();
    /* if we have best match, return it.  Otherwise, if we have one match
     * on the loser match, return that, otehrwise null
     */
    if (spob2)
        return spob2;
    if (spob1 && nummatch == 1)
        return spob1;
    return NULL;
}

/**
 * Decides weither the (spell-)object sp_op will
 * be reflected from the given mapsquare. Returns 1 if true.
 *
 * (Note that for living creatures there is a small chance that
 * reflect_spell fails.)
 *
 * Caller should be sure it passes us valid map coordinates
 * eg, updated for tiled maps.
 *
 * @param m
 * @param x
 * @param y
 * position of the object to test.
 * @param sp_op
 * spell object to test.
 * @return
 * 1 if reflected, 0 else.
 */
int reflwall(mapstruct *m, int x, int y, object *sp_op) {
    if (OUT_OF_REAL_MAP(m, x, y))
        return 0;
    FOR_MAP_PREPARE(m, x, y, op)
        if (QUERY_FLAG(op, FLAG_REFL_SPELL)
        && (!QUERY_FLAG(op, FLAG_ALIVE) || (rndm(0, 99)) < 90-(sp_op->level/10)))
            return 1;
    FOR_MAP_FINISH();
    return 0;
}

/**
 * Creates object new_op in direction dir or if that is blocked, beneath the player (op).
 * we pass 'caster', but don't use it for anything.
 * This is really just a simple wrapper function .
 *
 * @param op
 * who is casting.
 * @param new_op
 * object to insert.
 * @param dir
 * direction to insert into. Can be 0.
 * @return
 * direction that the object was actually placed in.
 */
int cast_create_obj(object *op, object *new_op, int dir) {
    mapstruct *m;
    sint16  sx, sy;

    if (dir
    && ((get_map_flags(op->map, &m, op->x+freearr_x[dir], op->y+freearr_y[dir], &sx, &sy)&P_OUT_OF_MAP)
        || OB_TYPE_MOVE_BLOCK(op, GET_MAP_MOVE_BLOCK(m, sx, sy)))) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_INFO,
                      "Something is in the way. You cast it at your feet.");
        dir = 0;
    }
    if (dir == 0)
        object_insert_in_map_at(new_op, op->map, op, INS_BELOW_ORIGINATOR, op->x+freearr_x[dir], op->y+freearr_y[dir]);
    else
        object_insert_in_map_at(new_op, op->map, op, 0, op->x+freearr_x[dir], op->y+freearr_y[dir]);
    return dir;
}

/**
 * Returns true if it is ok to put spell op on the space/may provided.
 *
 * @param m
 * @param x
 * @param y
 * coordinates to test.
 * @param op
 * spell to test for.
 * @param immune_stop
 * basically the attacktype of the spell (why
 * passed as a different value, not sure of).  If immune_stop
 * has the AT_MAGIC bit set, and there is a counterwall
 * on the space, the object doesn't get placed.  if immune_stop
 * does not have AT_MAGIC, then counterwalls do not effect the spell.
 * @return
 * 1 if we can add op, 0 else.
 */
int ok_to_put_more(mapstruct *m, sint16 x, sint16 y, object *op, uint32 immune_stop) {
    int mflags;
    mapstruct *mp;

    mp = m;
    mflags = get_map_flags(m, &mp, x, y, &x, &y);

    if (mflags&P_OUT_OF_MAP)
        return 0;

    if (OB_TYPE_MOVE_BLOCK(op, GET_MAP_MOVE_BLOCK(mp, x, y)))
        return 0;

    FOR_MAP_PREPARE(mp, x, y, tmp) {
        /* If there is a counterspell on the space, and this
         * object is using magic, don't progess.  I believe we could
         * leave this out and let in progress, and other areas of the code
         * will then remove it, but that would seem to to use more
         * resources, and may not work as well if a player is standing
         * on top of a counterwall spell (may hit the player before being
         * removed.)  On the other hand, it may be more dramatic for the
         * spell to actually hit the counterwall and be sucked up.
         */
        if ((tmp->attacktype&AT_COUNTERSPELL)
        && (tmp->type != PLAYER)
        && !QUERY_FLAG(tmp, FLAG_MONSTER)
        && (tmp->type != WEAPON)
        && (tmp->type != BOW)
        && (tmp->type != ARROW)
        && (tmp->type != GOLEM)
        && (immune_stop&AT_MAGIC))
            return 0;

        /* This is to prevent 'out of control' spells.  Basically, this
         * limits one spell effect per space per spell.  This is definitely
         * needed for performance reasons, and just for playability I believe.
         * there are no such things as multispaced spells right now, so
         * we don't need to worry about the head.
         * We only need to go down this path is maxhp is set on both objects -
         * otherwise, no reason to check.  But if we do check, we need to
         * do some extra work, looking in the spell_tags[] of each object,
         * if they have it set.
         */
        if (tmp->type == op->type
        && tmp->subtype == op->subtype
        && tmp->stats.maxhp
        && op->stats.maxhp) {
            if ((tmp->stats.maxhp == op->stats.maxhp)
            || (tmp->spell_tags && OB_SPELL_TAG_MATCH(tmp, (tag_t)op->stats.maxhp))
            || (op->spell_tags && OB_SPELL_TAG_MATCH(op, (tag_t)tmp->stats.maxhp))) {
                statistics.spell_suppressions++;
                return 0;
            }

            /* if both objects have spell tags, then if the two tags entries
             * from either match, that also counts.  Need to check
             * the spell_tags, because 0 values are allowed to match
             */
            if (op->spell_tags && tmp->spell_tags) {
                int i;

                for (i = 0; i < SPELL_TAG_SIZE; i++) {
                    if (op->spell_tags[i] && op->spell_tags[i] == tmp->spell_tags[i])
                        statistics.spell_suppressions++;
                    return 0;
                }
            }
        }
        /* Perhaps we should also put checks in for no magic and unholy
         * ground to prevent it from moving along?
         */
    } FOR_MAP_FINISH();
    /* If it passes the above tests, it must be OK */
    return 1;
}

/**
 * Fires an archetype.
 *
 * @note
 * it uses op->map for the map for these coordinates, which is probably a really bad idea.
 *
 * @param op
 * person firing the object.
 * @param caster
 * object casting the spell.
 * @param x
 * @param y
 * where to fire the spell.
 * @param dir
 * direction to fire in.
 * @param spell
 * spell that is being fired.  It uses other_arch for the archetype to fire.
 * @return
 * 0 on failure, 1 on success.
 * @todo check the note?
 * @note
 * fire_bullet() has been merged into fire_arch_from_position()
 */
int fire_arch_from_position(object *op, object *caster, sint16 x, sint16 y, int dir, object *spell) {
    object *tmp;
    int mflags;
    mapstruct *m;

    if (dir < 0 || dir > 8) {
        LOG(llevError, "Invalid direction %d in fire_arch_from_position for %s\n", dir, spell->name);
        dir = RANDOM() % 8 + 1;
    }

    if (spell->other_arch == NULL)
        return 0;

    if (spell->type != SPELL)
        LOG(llevError, "Unexpected object type %d in fire_arch_from_position for %s\n", spell->type, spell->name);

    m = op->map;
    mflags = get_map_flags(m, &m, x, y, &x, &y);
    if (mflags&P_OUT_OF_MAP) {
        return 0;
    }

    tmp = arch_to_object(spell->other_arch);
    if (tmp == NULL)
        return 0;

    mflags = get_map_flags(m, &tmp->map, x, y, &tmp->x, &tmp->y);
    if (mflags&P_OUT_OF_MAP) {
        object_free_drop_inventory(tmp);
        return 0;
    }

    if (spell->subtype == SP_BULLET) {
        /*  peterm:  level dependency for bolts  */
        tmp->stats.dam = spell->stats.dam+SP_level_dam_adjust(caster, spell);
        tmp->attacktype = spell->attacktype;
        if (spell->slaying)
            tmp->slaying = add_refcount(spell->slaying);

        tmp->range = 50;

        /* Need to store duration/range for the ball to use */
        tmp->stats.hp = spell->duration+SP_level_duration_adjust(caster, spell);
        tmp->stats.maxhp = spell->range+SP_level_range_adjust(caster, spell);
        tmp->dam_modifier = spell->stats.food+SP_level_dam_adjust(caster, spell);

        tmp->direction = dir;
        object_update_turn_face(tmp);

        object_set_owner(tmp, op);
    } else {
        if (spell->subtype != SP_MAGIC_MISSILE && spell->subtype != SP_MOVING_BALL)
            LOG(llevError, "Unexpected object subtype %d in fire_arch_from_position for %s\n", spell->subtype, spell->name);

        /*
         * Things like firewalls and such can fire even if blocked, since the
         * origin is themselves which block things...
         * This fixes bug #3536508.
         */
        if (caster->type == PLAYER && OB_TYPE_MOVE_BLOCK(tmp, GET_MAP_MOVE_BLOCK(tmp->map, tmp->x, tmp->y))) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                "You can't cast the spell on top of a wall!");
            object_free_drop_inventory(tmp);
            return 0;
        }

        tmp->stats.dam = spell->stats.dam+SP_level_dam_adjust(caster, spell);
        tmp->duration = spell->duration+SP_level_duration_adjust(caster, spell);
        /* code in time.c uses food for some things, duration for others */
        tmp->stats.food = tmp->duration;
        tmp->range = spell->range+SP_level_range_adjust(caster, spell);
        tmp->attacktype = spell->attacktype;
        if (object_get_owner(op) != NULL)
            object_copy_owner(tmp, op);
        else
            object_set_owner(tmp, op);
        tmp->level = caster_level(caster, spell);
    }

    tmp->direction = dir;
    set_spell_skill(op, caster, spell, tmp);

    if (spell->subtype == SP_BULLET) {
        if (OB_TYPE_MOVE_BLOCK(tmp, GET_MAP_MOVE_BLOCK(tmp->map, tmp->x, tmp->y))) {
            if (!QUERY_FLAG(tmp, FLAG_REFLECTING)) {
                object_free_drop_inventory(tmp);
                return 0;
            }
            tmp->direction = absdir(tmp->direction+4);
            x += DIRX(tmp);
            y += DIRY(tmp);
            mflags = get_map_flags(m, &m, x, y, &x, &y);
            if (mflags&P_OUT_OF_MAP) {
                object_free_drop_inventory(tmp);
                return 0;
            }
            tmp->x = x;
            tmp->y = y;
            tmp->map = m;
        }

        tmp = object_insert_in_map_at(tmp, tmp->map, op, 0, tmp->x, tmp->y);
        if (tmp != NULL)
            check_bullet(tmp);
    } else /*if (spell->subtype == SP_MAGIC_MISSILE || spell->subtype == SP_MOVING_BALL) */ {
        /* needed for AT_HOLYWORD, AT_GODPOWER stuff */
        if (tmp->attacktype&AT_HOLYWORD || tmp->attacktype&AT_GODPOWER) {
            if (!tailor_god_spell(tmp, op))
                return 0;
        }
        object_update_turn_face(tmp);

        tmp = object_insert_in_map_at(tmp, tmp->map, op, 0, tmp->x, tmp->y);
        if (tmp != NULL)
            ob_process(tmp);
    }

    return 1;
}

/*****************************************************************************
 *
 * Code related to rods - perhaps better located in another file?
 *
 ****************************************************************************/

/**
 * Regenerates a rod's charges.
 *
 * @param rod
 * rod to regenerate.
 */
void regenerate_rod(object *rod) {
    if (rod->stats.hp < rod->stats.maxhp) {
        rod->stats.hp += 1+rod->stats.maxhp/10;

        if (rod->stats.hp > rod->stats.maxhp)
            rod->stats.hp = rod->stats.maxhp;
    }
}

/**
 * Drain charges from a rod.
 *
 * @param rod
 * rod to drain.
 */
void drain_rod_charge(object *rod) {
    rod->stats.hp -= SP_level_spellpoint_cost(rod, rod->inv, SPELL_HIGHEST);
}

/**
 * Drains a charge from a wand. Handles animation fix, and player update if required.
 *
 * @param wand
 * wand to drain. Must be of type WAND.
 */
void drain_wand_charge(object *wand) {
    assert(wand->type == WAND);

    if (!(--wand->stats.food)) {
        object *tmp;
        if (wand->arch) {
            CLEAR_FLAG(wand, FLAG_ANIMATE);
            wand->face = wand->arch->clone.face;
            wand->speed = 0;
            object_update_speed(wand);
        }
        tmp = object_get_player_container(wand);
        if (tmp)
            esrv_update_item(UPD_ANIM, tmp, wand);
    }
}

/**
 * This function is commonly used to find a friendly target for
 * spells such as heal or protection or armour
 * @param op
 * what is looking for the target (which can be a player).
 * @param dir
 * direction we are looking in.
 * @return
 * object found, or NULL if no good object.
 */
object *find_target_for_friendly_spell(object *op, int dir) {
    object *tmp;
    mapstruct *m;
    sint16 x, y;
    int mflags;

    /* I don't really get this block - if op isn't a player or rune,
     * we then make the owner of this object the target.
     * The owner could very well be no where near op.
     */
    if (op->type != PLAYER && op->type != RUNE) {
        tmp = object_get_owner(op);
        /* If the owner does not exist, or is not a monster, than apply the spell
         * to the caster.
         */
        if (!tmp || !QUERY_FLAG(tmp, FLAG_MONSTER))
            tmp = op;
    } else {
        m = op->map;
        x =  op->x+freearr_x[dir];
        y =  op->y+freearr_y[dir];

        mflags = get_map_flags(m, &m, x, y, &x, &y);

        if (mflags&P_OUT_OF_MAP)
            tmp = NULL;
        else {
            for (tmp = GET_MAP_OB(m, x, y); tmp != NULL; tmp = tmp->above) {
                if (tmp->type == PLAYER)
                    break;
            }
        }
    }
    /* didn't find a player there, look in current square for a player */
    if (tmp == NULL)
        FOR_MAP_PREPARE(op->map, op->x, op->y, tmp) {
            if (tmp->type == PLAYER)
                return tmp;
            /* Don't forget to browse inside transports ! - gros 2006/07/25 */
            if (tmp->type == TRANSPORT) {
                object *inv;

                for (inv = tmp->inv; inv; inv = inv->below) {
                    if ((inv->type == PLAYER) && (op == inv))
                        return inv;
                }
            }
        } FOR_MAP_FINISH();
    return tmp;
}



/**
 * Search what direction a spell should go in, first the center square
 * then some close squares in the given map at the given coordinates for
 * live objects.
 *
 * It will not consider the object given as exclude (= caster) among possible
 * live objects. If the caster is a player, the spell will go after
 * monsters/generators only. If not, the spell will hunt players only.
 *
 * Exception is player on a battleground, who will be targeted unless excluded.
 *
 * @param m
 * @param x
 * @param y
 * where to search from.
 * @param exclude
 * what object to avoid. Can be NULL, in which case all bets are off.
 * @return
 * direction toward the first/closest live object if it finds any, otherwise -1.
 */
int spell_find_dir(mapstruct *m, int x, int y, object *exclude) {
    int i;
    sint16 nx, ny;
    int owner_type = 0, mflags;
    object *tmp;
    mapstruct *mp;
    int dirs[SIZEOFFREE];

    if (exclude != NULL) {
        exclude = HEAD(exclude);
        owner_type = exclude->type;
    }

    get_search_arr(dirs);
    for (i = 1; i < SIZEOFFREE; i++) {
        nx = x+freearr_x[dirs[i]];
        ny = y+freearr_y[dirs[i]];
        mp = m;
        mflags = get_map_flags(m, &mp, nx, ny, &nx, &ny);
        if (mflags&(P_OUT_OF_MAP|P_BLOCKSVIEW))
            continue;

        for (tmp = GET_MAP_OB(mp, nx, ny); tmp != NULL; tmp = tmp->above) {
            object *head;

            head = HEAD(tmp);
            if ((owner_type != PLAYER || QUERY_FLAG(head, FLAG_MONSTER) || QUERY_FLAG(head, FLAG_GENERATOR) || (head->type == PLAYER && op_on_battleground(head, NULL, NULL, NULL)))
            && (owner_type == PLAYER || head->type == PLAYER)
            && head != exclude
            && can_see_monsterP(m, x, y, dirs[i])) {
                return freedir[dirs[i]];
            }
        }
    }
    return -1;  /* flag for "keep going the way you were" */
}



/**
 * Puts a monster named monstername near by
 * op.  This creates the treasures for the monsters, and
 * also deals with multipart monsters properly.
 *
 * @param op
 * victim.
 * @param monstername
 * monster's archetype name.
 * @return
 * 1 if a monster was put, 0 else (no free space around the spot or invalid monster name).
 * @todo there is a multipart-aware archetype conversion function, use it.
 */
static int put_a_monster(object *op, const char *monstername) {
    object *tmp, *head = NULL, *prev = NULL;
    archetype *at;
    int dir;

    /* Handle cases where we are passed a bogus mosntername */
    at = find_archetype(monstername);
    if (at == NULL)
        return 0;

    /* find a free square nearby
     * first we check the closest square for free squares
     */

    dir = object_find_first_free_spot(&at->clone, op->map, op->x, op->y);
    if (dir != -1) {
        /* This is basically grabbed for generate monster.  Fixed 971225 to
         * insert multipart monsters properly
         */
        while (at != NULL) {
            tmp = arch_to_object(at);
            tmp->x = op->x+freearr_x[dir]+at->clone.x;
            tmp->y = op->y+freearr_y[dir]+at->clone.y;
            tmp->map = op->map;
            if (head) {
                tmp->head = head;
                prev->more = tmp;
            }
            if (!head)
                head = tmp;
            prev = tmp;
            at = at->more;
        }

        if (head->randomitems)
            create_treasure(head->randomitems, head, GT_INVISIBLE, op->map->difficulty, 0);

        object_insert_in_map_at(head, op->map, op, 0, op->x, op->y);

        /* thought it'd be cool to insert a burnout, too.*/
        tmp = create_archetype("burnout");
        object_insert_in_map_at(tmp, op->map, op, 0, op->x+freearr_x[dir], op->y+freearr_y[dir]);
        return 1;
    } else {
        return 0;
    }
}

/**
 * Summons hostile monsters and places them in nearby squares.
 *
 * @note
 * this is not used by any spells (summon evil monsters
 * use to call this, but best I can tell, that spell/ability was
 * never used.  This is however used by various failures on the
 * players part (alchemy, reincarnation, etc)
 *
 * @param op
 * the summoner.
 * @param n
 * number of monsters.
 * @param monstername
 * name of the monster to summon, should be a valid archetype.
 * @return
 * number of monsters actually put on the map.
 */
int summon_hostile_monsters(object *op, int n, const char *monstername) {
    int i, put = 0;

    for (i = 0; i < n; i++)
        put += put_a_monster(op, monstername);

    return put;
}


/**
 * This routine shuffles the attack of op to one of the
 * ones in the list. It does this at random.  It also
 * chooses a face appropriate to the attack that is
 * being committed by that square at the moment.
 *
 * right now it's being used by color spray and create pool of
 * chaos.
 *
 * This could really be a better implementation - the
 * faces and attacktypes above are hardcoded, which is never
 * good.  The faces refer to faces in the animation sequence.
 * Not sure how to do better - but not having it hardcoded
 * would be nice.
 *
 * I also fixed a bug here in that attacktype was |= -
 * to me, that would be that it would quickly get all
 * attacktypes, which probably wasn't the intent.  MSW 2003-06-03
 *
 * @param op
 * object to change.
 * @param change_face
 * if set, also changes the face, else only changes the attacktype.
 */
void shuffle_attack(object *op, int change_face) {
    int i;

    i = rndm(0, 21);
    op->attacktype = ATTACKS[i].attacktype|AT_MAGIC;
    if (change_face) {
        SET_ANIMATION(op, ATTACKS[i].face);
    }
}


/**
 * Called when a player fails at casting a prayer.
 *
 * @param op
 * player.
 * @param failure
 * basically how much grace they had.
 * @param power
 * how much grace the spell would normally take to cast.
 */
static void prayer_failure(object *op, int failure, int power) {
    const char *godname;
    object *tmp;

    godname = determine_god(op);
    if (!strcmp(godname, "none"))
        godname = "Your spirit";

    if (failure <= -20 && failure > -40) { /* wonder */
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                             "%s gives a sign to renew your faith.",
                             godname);
        tmp = create_archetype(SPELL_WONDER);
        cast_cone(op, op, 0, tmp);
        object_free_drop_inventory(tmp);
    } else if (failure <= -40 && failure > -60) { /* confusion */
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                      "Your diety touches your mind!");
        confuse_living(op, op, 99);
    } else if (failure <= -60 && failure > -150) { /* paralysis */
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                             "%s requires you to pray NOW. You comply, ignoring all else.",
                             godname);

        paralyze_living(op, 99);
    } else if (failure <= -150) { /* blast the immediate area */
        tmp = create_archetype(GOD_POWER);
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                             "%s smites you!",
                             godname);
        /* Put a cap on power - this is effectively cost of the spell minus
         * characters current grace.  Thus, if spell costs 30 grace and
         * character has -100 grace, this is cast as a level 130 spell.
         * Things start to break in those cases.
         */
        cast_magic_storm(op, tmp, power > 50 ? 50 : power);
    }
}

/**
 * Handles the various effects for differing degrees of failure badness.
 *
 * @param op
 * player that failed.
 * @param failure
 * random value of how badly you failed.
 * @param power
 * how many spellpoints you'd normally need for the spell.
 * @param skill
 * skill needed to cast the spell.
 */
void spell_failure(object *op, int failure, int power, object *skill) {
    object *tmp;

    if (settings.spell_failure_effects == FALSE)
        return;

    if (failure <= -20 && failure > -40) { /* wonder */
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                      "Your spell causes an unexpected effect.");
        tmp = create_archetype(SPELL_WONDER);
        cast_cone(op, op, 0, tmp);
        object_free_drop_inventory(tmp);
    } else if (failure <= -40 && failure > -60) { /* confusion */
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                      "Your magic recoils on you, making you confused!");
        confuse_living(op, op, 99);
    } else if (failure <= -60 && failure > -80) { /* paralysis */
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                      "Your magic stuns you!");
        paralyze_living(op, 99);
    } else if (failure <= -80) { /* blast the immediate area */
        object *tmp;

        /* Safety check to make sure we don't get any mana storms in scorn */
        if (get_map_flags(op->map, NULL, op->x, op->y, NULL, NULL)&P_NO_MAGIC) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                          "The magic warps and you are turned inside out!");
            hit_player(op, 9998, op, AT_INTERNAL, 1);
        } else {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                          "You lose control of the mana! The uncontrolled magic blasts you!");
            tmp = create_archetype(LOOSE_MANA);
            tmp->level = skill->level;

            /* increase the area of destruction a little for more powerful spells */
            tmp->range += isqrt(power);

            if (power > 25)
                tmp->stats.dam = 25+isqrt(power);
            else
                tmp->stats.dam = power; /* nasty recoils! */

            tmp->stats.maxhp = tmp->count;
            if (tmp->stats.maxhp == 0)
                tmp->stats.maxhp = 1;
            object_insert_in_map_at(tmp, op->map, NULL, 0, op->x, op->y);
        }
    }
}

/**
 * Determines if an item can be transmuted after a cast failure.
 * @param op item to check.
 * @return 1 if op can be transmuted, 0 else.
 *
 * @note
 * can_be_tansmuted() has been renamed to can_be_transmuted_to_flower()
*/
static int can_be_transmuted_to_flower(object *op) {
    if (op->invisible)
        return 0;

    if (op->type == POTION || op->type == SCROLL || op->type == WAND || op->type == ROD || op->type == WEAPON)
        return 1;

    return 0;
}

/**
 * This transforms one random item of op to a flower.
 *
 * Only some items are considered, mostly magical ones.
 *
 * Transformed item is put in a force in a flower, weights are adjusted.
 *
 * See remove_force() in server/time.c to see object removal.
 *
 * @param op
 * player who is the victim.
 */
static void transmute_item_to_flower(object *op) {
    object *force;
    object *item;
    object *flower;
    object *first = NULL;
    int count = 0;
    char name[HUGE_BUF];

    FOR_INV_PREPARE(op, item)
        if (can_be_transmuted_to_flower(item)) {
            if (!first)
                first = item;
            count++;
        }
    FOR_INV_FINISH();

    if (count == 0)
        return;

    count = rndm(0, count-1);
    for (item = first; item; item = item->below) {
        if (can_be_transmuted_to_flower(item)) {
            count--;
            if (count < 0)
                break;
        }
    }

    if (!item)
        return;

    force = create_archetype(FORCE_NAME);
    force->duration = 100+rndm(0, 10)*100;
    force->subtype = FORCE_TRANSFORMED_ITEM;
    force->speed = 1;
    object_update_speed(force);

    flower = create_archetype("flowers_permanent");

    if (QUERY_FLAG(item, FLAG_APPLIED))
        apply_manual(op, item, AP_NOPRINT|AP_IGNORE_CURSE|AP_UNAPPLY);
    object_remove(item);
    flower->weight = item->nrof ? ((sint32)item->nrof)*item->weight : item->weight;
    item->weight = 0;
    esrv_del_item(op->contr, item);
    object_insert_in_ob(item, force);

    query_short_name(item, name, HUGE_BUF);
    draw_ext_info_format(NDI_UNIQUE, 0, op,
                         MSG_TYPE_ITEM, MSG_TYPE_ITEM_CHANGE,
                         "Your %s turns to a flower!",
                         name);

    object_insert_in_ob(force, flower);
    flower = object_insert_in_ob(flower, op);
    esrv_send_item(op, flower);
}

/**
 * Randomly swaps 2 stats of op.
 *
 * Swapping is merely a FORCE inserted into op's inventory.
 *
 * Used for spell casting when confused.
 *
 * @param op
 * player who is the victim.
 */
static void swap_random_stats(object *op) {
    object *force;
    int first, second;

    first = RANDOM()%NUM_STATS;
    second = RANDOM()%(NUM_STATS-1);
    if (second >= first)
        second++;

    draw_ext_info_format(NDI_UNIQUE, 0, op,
                         MSG_TYPE_VICTIM, MSG_TYPE_VICTIM_SPELL,
                         "You suddenly feel really weird!");

    force = create_archetype(FORCE_NAME);
    force->duration = 100+rndm(0, 10)*100;
    force->speed = 1;
    SET_FLAG(force, FLAG_APPLIED);
    set_attr_value(&force->stats, second, get_attr_value(&op->stats, first)-get_attr_value(&op->stats, second));
    set_attr_value(&force->stats, first, get_attr_value(&op->stats, second)-get_attr_value(&op->stats, first));
    object_update_speed(force);
    object_insert_in_ob(force, op);
    change_abil(op, force);
    fix_object(op);
}

/**
 * This does a random effect for op, which tried to cast a spell in a confused state.
 *
 * Note that random spell casting is handled in cast_spell itself.
 *
 * Used for spell casting when confused.
 *
 * @param op
 * player for which to produce a random effect.
 */
static void handle_spell_confusion(object *op) {
    switch (RANDOM()%2) {
    case 0:
        transmute_item_to_flower(op);
        break;

    case 1:
        swap_random_stats(op);
        break;
    }
}

/**
 * Check if the player attempting to cast a spell has the required items, and eat those.
 * Will warn of insufficient resources if failure. Only eats if all items are found, else don't do anything.
 * @param op who is casting.
 * @param spell_ob spell being cast.
 * @return 0 if some items were missing, and warn player, 1 if all if ok.
 */
static int spell_consume_items(object *op, const object *spell_ob) {
    sstring requirements;
    char *copy;
    char *ingredients[10];
    object *found[10];
    int count, i;
    uint32 nrof[10];
    char name_ob[MAX_BUF];
    const char *name2;

    if (op->type != PLAYER)
        return 1;

    requirements = object_get_value(spell_ob, "casting_requirements");
    if (!requirements)
        /* no special requirements */
        return 1;

    /* find items */
    copy = strdup_local(requirements);
    count = split_string(copy, ingredients, 10, ',');

    /* first pass, find items */
    for (i = 0; i < count; i++) {
        nrof[i] = 0;
        found[i] = NULL;
        while (isdigit(*ingredients[i])) {
            nrof[i] = 10*nrof[i]+(*(ingredients[i])-'0');
            ingredients[i]++;
        }
        if (nrof[i] == 0)
            nrof[i] = 1;
        while (*ingredients[i] == ' ')
            ingredients[i]++;

        /* now find item in op's inv */
        FOR_INV_PREPARE(op, check) {

            if (check->title == NULL)
                name2 = check->name;
            else {
                snprintf(name_ob, sizeof(name_ob), "%s %s", check->name, check->title);
                name2 = name_ob;
            }

            if (strcmp(name2, ingredients[i]) == 0) {
                found[i] = check;
                break;
            }
        } FOR_INV_FINISH();

        if (found[i] == NULL) {
            draw_ext_info_format(NDI_UNIQUE, 0, op,
                MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                "Casting this spell requires %s, but you don't have any.",
                ingredients[i]);
            free(copy);
            return 0;
        }

        if (found[i]->nrof < nrof[i]) {
            draw_ext_info_format(NDI_UNIQUE, 0, op,
                MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                "Casting this spell requires %d %s, but you only have %d.",
                nrof[i], found[i]->name_pl, found[i]->nrof);
            free(copy);
            return 0;
        }
    }

    free(copy);

    /* ok, found ingredients, remove'em */
    for (i = 0; i < count; i++) {
        object_decrease_nrof(found[i], nrof[i]);
    }

    /* all right, spell can be cast */
    return 1;
}

/**
 * Main dispatch when someone casts a spell.
 *
 * Will decrease mana/gr points, check for skill, confusion and such.
 *
 * Note that this function is really a dispatch routine that calls other
 * functions - it just blindly returns what ever value those functions
 * return.  So if your writing a new function that is called from this,
 * it should also return 1 on success, 0 on failure.
 *
 * if it is a player casting the spell (op->type == PLAYER, op == caster),
 * this function will decrease the mana/grace appropriately.  For other
 * objects, the caller should do what it considers appropriate.
 *
 * @param op
 * creature that is owner of the object that is casting the spell -
 *    eg, the player or monster.
 * @param caster
 * actual object (wand, potion) casting the spell. can be same as op.
 * @param dir
 * direction to cast in.  Note in some cases, if the spell
 * is self only, dir really doesn't make a difference.
 * @param spell_ob
 * spell object that is being cast.  From that, we can determine what to do.
 * *@param stringarg
 * any options that are being used.  It can be NULL.  Almost
 * certainly, only players will set it.  It is basically used as optional
 * parameters to a spell (eg, item to create, information for marking runes,
 * etc.
 * @return
 * 1 on successful cast, or 0 on error. These values should really
 * be swapped, so that 0 is successful, and non zero is failure, with a code
 * of what it failed.
 * @todo return a failure value?
 */
int cast_spell(object *op, object *caster, int dir, object *spell_ob, char *stringarg) {
    const char *godname;
    int success = 0, mflags, cast_level = 0;
    rangetype old_shoottype;
    object *skill = NULL;
    int confusion_effect = 0;

    old_shoottype = op->contr ? op->contr->shoottype : range_none;

    if (!spell_ob) {
        LOG(llevError, "cast_spell: null spell object passed\n");
        return 0;
    }
    godname = determine_god(op);
    if (!strcmp(godname, "none"))
        godname = "A random spirit";

    /* the caller should set caster to op if appropriate */
    if (!caster) {
        LOG(llevError, "cast_spell: null caster object passed\n");
        return 0;
    }
    if (spell_ob->anim_suffix)
        apply_anim_suffix(caster, spell_ob->anim_suffix);

    /* Handle some random effect if confused. */
    if (QUERY_FLAG(op, FLAG_CONFUSED) && caster == op && op->type == PLAYER) {
        if (rndm(0, 5) < 4) {
            spell_ob = find_random_spell_in_ob(op, NULL);
            draw_ext_info_format(NDI_UNIQUE, 0, op,
                                 MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                                 "In your confused state, you're not sure of what you cast!");
        } else
            /* We fall through to deplate sp/gr, and do some checks. */
            confusion_effect = 1;
    }

    /* if caster is a spell casting object, this normally shouldn't be
     * an issue, because they don't have any spellpaths set up.
     */
    if ((caster->path_denied&spell_ob->path_attuned) && !QUERY_FLAG(caster, FLAG_WIZ)) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                      "That spell path is denied to you.");
        return 0;
    }

    /* if it is a player casting the spell, and they are really casting it
     * (vs it coming from a wand, scroll, or whatever else), do some
     * checks.  We let monsters do special things - eg, they
     * don't need the skill, bypass level checks, etc. The monster function
     * should take care of that.
     * Remove the wiz check here and move it further down - some spells
     * need to have the right skill pointer passed, so we need to
     * at least process that code.
     */
    if (op->type == PLAYER && op == caster) {
        cast_level = caster_level(caster, spell_ob);
        if (spell_ob->skill) {
            skill = find_skill_by_name(op, spell_ob->skill);
            if (!skill) {
                draw_ext_info_format(NDI_UNIQUE, 0, op,
                                     MSG_TYPE_SKILL, MSG_TYPE_SKILL_MISSING,
                                     "You need the skill %s to cast %s.",
                                     spell_ob->skill, spell_ob->name);
                return 0;
            }
            if (min_casting_level(op, spell_ob) > cast_level && !QUERY_FLAG(op, FLAG_WIZ)) {
                draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                              "You lack enough skill to cast that spell.");
                return 0;
            }
        }
        /* If the caster is the wiz, they don't ever fail, and don't have
         * to have sufficient grace/mana.
         */
        if (!QUERY_FLAG(op, FLAG_WIZ)) {
            if (SP_level_spellpoint_cost(caster, spell_ob, SPELL_MANA)
            && SP_level_spellpoint_cost(caster, spell_ob, SPELL_MANA) >  op->stats.sp) {
                draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                              "You don't have enough mana.");
                return 0;
            }
            if (SP_level_spellpoint_cost(caster, spell_ob, SPELL_GRACE)
            && SP_level_spellpoint_cost(caster, spell_ob, SPELL_GRACE) > op->stats.grace) {
                if (random_roll(0, op->stats.Wis-1, op, PREFER_HIGH)+op->stats.grace-10*SP_level_spellpoint_cost(caster, spell_ob, SPELL_GRACE)/op->stats.maxgrace > 0) {
                    draw_ext_info_format(NDI_UNIQUE, 0, op,
                                         MSG_TYPE_SPELL, MSG_TYPE_SPELL_INFO,
                                         "%s grants your prayer, though you are unworthy.",
                                         godname);
                } else {
                    prayer_failure(op, op->stats.grace, SP_level_spellpoint_cost(caster, spell_ob, SPELL_GRACE)-op->stats.grace);
                    draw_ext_info_format(NDI_UNIQUE, 0, op,
                                         MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                                         "%s ignores your prayer.",
                                         godname);
                    return 0;
                }
            }

            /* player/monster is trying to cast the spell.  might fumble it */
            if (spell_ob->stats.grace
                && random_roll(0, 99, op, PREFER_HIGH) < (get_cleric_chance(op->stats.Wis) * spell_ob->level/MAX(1, op->level))) {
                play_sound_player_only(op->contr, SOUND_TYPE_SPELL, spell_ob, 0, "fumble");
                draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                              "You fumble the spell.");
                if (settings.casting_time == TRUE) {
                    op->casting_time = -1;
                }
                op->stats.grace -= random_roll(1, SP_level_spellpoint_cost(caster, spell_ob, SPELL_GRACE), op, PREFER_LOW);
                return 0;
            } else if (spell_ob->stats.sp) {
                int failure = random_roll(0, 199, op, PREFER_HIGH)-op->contr->encumbrance+op->level-spell_ob->level+35;

                if (failure < 0) {
                    draw_ext_info(NDI_UNIQUE, 0, op,
                                  MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                                  "You bungle the spell because you have too much heavy equipment in use.");
                    if (settings.spell_failure_effects == TRUE)
                        spell_failure(op, failure, SP_level_spellpoint_cost(caster, spell_ob, SPELL_MANA), skill);
                    op->contr->shoottype = old_shoottype;
                    op->stats.sp -= random_roll(0, SP_level_spellpoint_cost(caster, spell_ob, SPELL_MANA), op, PREFER_LOW);
                    return 0;
                }
            }

            /* ensure the potentially required items are eaten */
            if (!spell_consume_items(op, spell_ob))
                /* already warned by the function */
                return 0;
        }
    }

    mflags = get_map_flags(op->map, NULL, op->x, op->y, NULL, NULL);

    /* See if we can cast a spell here.  If the caster and op are
     * not alive, then this would mean that the mapmaker put the
     * objects on the space - presume that they know what they are
     * doing.
     */
    if (spell_ob->type == SPELL
    && caster->type != POTION
    && !QUERY_FLAG(op, FLAG_WIZCAST)
    && (QUERY_FLAG(caster, FLAG_ALIVE) || QUERY_FLAG(op, FLAG_ALIVE))
    && !QUERY_FLAG(op, FLAG_MONSTER)
    && (((mflags&P_NO_MAGIC) && spell_ob->stats.sp) || ((mflags&P_NO_CLERIC) && spell_ob->stats.grace))) {
        if (op->type != PLAYER)
            return 0;

        if ((mflags&P_NO_CLERIC) && spell_ob->stats.grace)
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                                 "This ground is unholy!  %s ignores you.",
                                 godname);
        else
            switch (op->contr->shoottype) {
            case range_magic:
                draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                              "Something blocks your spellcasting.");
                break;

            case range_misc:
                draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
                              "Something blocks the magic of your item.");
                break;
            case range_golem:
                draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
                              "Something blocks the magic of your scroll.");
                break;

            default:
                break;
            }
        return 0;
    }

    if (caster == op && caster->type ==PLAYER && settings.casting_time == TRUE && spell_ob->type == SPELL) {
        if (op->casting_time == -1) { /* begin the casting */
            op->casting_time = spell_ob->casting_time*PATH_TIME_MULT(op, spell_ob);
            op->spell = spell_ob;
            /* put the stringarg into the object struct so that when the
            * spell is actually cast, it knows about the stringarg.
            * necessary for the invoke command spells.
            */
            if (stringarg) {
                op->spellarg = strdup_local(stringarg);
            } else
                op->spellarg = NULL;
            return 0;
        } else if (op->casting_time != 0) {
            if (op->type == PLAYER)
                draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_INFO,
                              "You are casting!");
            return 0;
        } else {    /* casting_time == 0 */
            op->casting_time = -1;
            spell_ob = op->spell;
            stringarg = op->spellarg;
        }
    } else {
        if (!QUERY_FLAG(caster, FLAG_WIZ)) {
            /* Take into account how long it takes to cast the spell.
             * if the player is casting it, then we use the time in
             * the spell object.  If it is a spell object, have it
             * take two ticks.  Things that cast spells on the players
             * behalf (eg, altars, and whatever else) shouldn't cost
             * the player any time.
             * Ignore casting time for firewalls
             */
            if (caster == op && caster->type != FIREWALL) {
                op->speed_left -= spell_ob->casting_time*PATH_TIME_MULT(op, spell_ob)*FABS(op->speed);
                /* Other portions of the code may also decrement the speed of the player, so
                 * put a lower limit so that the player isn't stuck here too long
                 */
                if ((spell_ob->casting_time > 0)
                && op->speed_left < -spell_ob->casting_time*PATH_TIME_MULT(op, spell_ob)*FABS(op->speed))
                    op->speed_left = -spell_ob->casting_time*PATH_TIME_MULT(op, spell_ob)*FABS(op->speed);
            } else if (caster->type == WAND
            || caster->type == ROD
            || caster->type == POTION
            || caster->type == SCROLL) {
                op->speed_left -= 2*FABS(op->speed);
            }
        }
    }

    if (op->type == PLAYER && op == caster && !QUERY_FLAG(caster, FLAG_WIZ)) {
        op->stats.grace -= SP_level_spellpoint_cost(caster, spell_ob, SPELL_GRACE);
        op->stats.sp -= SP_level_spellpoint_cost(caster, spell_ob, SPELL_MANA);
    }

    /* We want to try to find the skill to properly credit exp.
     * for spell casting objects, the exp goes to the skill the casting
     * object requires.
     */
    if (op != caster && !skill && caster->skill && !QUERY_FLAG(op, FLAG_MONSTER)) {
        skill = find_skill_by_name(op, caster->skill);
        if (!skill) {
            char name[MAX_BUF];

            query_name(caster, name, MAX_BUF);
            draw_ext_info_format(NDI_UNIQUE, 0, op,
                                 MSG_TYPE_SKILL, MSG_TYPE_SKILL_MISSING,
                                 "You lack the skill %s to use the %s",
                                 caster->skill, name);
            return 0;
        }
        change_skill(op, skill, 0);    /* needed for proper exp credit */
    }

    /* Need to get proper ownership for spells cast via runes - these are not
     * the normal 'rune of fire', but rather the magic runes that let the player
     * put some other spell into the rune (glyph, firetrap, magic rune, etc)
     */
    if (caster->type == RUNE) {
        object *owner = object_get_owner(caster);

        if (owner)
            skill = find_skill_by_name(owner, caster->skill);
    }

    if (confusion_effect) {
        /* If we get here, the confusion effect was 'random effect', so do it and bail out. */
        draw_ext_info_format(NDI_UNIQUE, 0, op,
                             MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                             "In your confused state, you can't control the magic!");
        handle_spell_confusion(op);
        return 0;
    }

    play_sound_map(SOUND_TYPE_SPELL, caster, dir, spell_ob->name);

    switch (spell_ob->subtype) {
        /* The order of case statements is same as the order they show up
         * in in spells.h.
         */
    case SP_RAISE_DEAD:
        success = cast_raise_dead_spell(op, caster, spell_ob, dir, stringarg);
        break;

    case SP_RUNE:
        success = write_rune(op, caster, spell_ob, dir, stringarg);
        break;

    case SP_MAKE_MARK:
        success = write_mark(op, spell_ob, stringarg);
        break;

    case SP_BOLT:
        success = fire_bolt(op, caster, dir, spell_ob);
        break;

    case SP_BULLET:
        success = fire_arch_from_position(op, caster, op->x+freearr_x[dir], op->y+freearr_y[dir], dir, spell_ob);
        break;

    case SP_CONE:
        success = cast_cone(op, caster, dir, spell_ob);
        break;

    case SP_BOMB:
        success = create_bomb(op, caster, dir, spell_ob);
        break;

    case SP_WONDER:
        success = cast_wonder(op, caster, dir, spell_ob);
        break;

    case SP_SMITE:
        success = cast_smite_spell(op, caster, dir, spell_ob);
        break;

    case SP_MAGIC_MISSILE:
        success = fire_arch_from_position(op, caster, op->x, op->y, dir, spell_ob);
        break;

    case SP_SUMMON_GOLEM:
        success = pets_summon_golem(op, caster, dir, spell_ob);
        if (success || op->contr == NULL)
            old_shoottype = range_golem;
        else
            /*
             * if the spell failed (something in the way for instance),
             * don't change the range so the player can try easily after that
             */
            old_shoottype = op->contr->shoottype;
        break;

    case SP_DIMENSION_DOOR:
        /* dimension door needs the actual caster, because that is what is
         * moved.
         */
        success = dimension_door(op, caster, spell_ob, dir);
        break;

    case SP_MAGIC_MAPPING:
        if (op->type == PLAYER) {
            spell_effect(spell_ob, op->x, op->y, op->map, op);
            draw_magic_map(op);
            success = 1;
        } else
            success = 0;
        break;

    case SP_MAGIC_WALL:
        success = magic_wall(op, caster, dir, spell_ob);
        break;

    case SP_DESTRUCTION:
        success = cast_destruction(op, caster, spell_ob);
        break;

    case SP_PERCEIVE_SELF:
        success = perceive_self(op);
        break;

    case SP_WORD_OF_RECALL:
        success = cast_word_of_recall(op, caster, spell_ob);
        break;

    case SP_INVISIBLE:
        success = cast_invisible(op, caster, spell_ob);
        break;

    case SP_PROBE:
        if (op != caster)
            cast_level = caster->level;
        else
            cast_level = skill != NULL ? skill->level : op->level;
        success = probe(op, caster, spell_ob, dir, cast_level);
        break;

    case SP_HEALING:
        success = cast_heal(op, caster, spell_ob, dir);
        break;

    case SP_CREATE_FOOD:
        success = cast_create_food(op, caster, spell_ob, dir, stringarg);
        break;

    case SP_EARTH_TO_DUST:
        success = cast_earth_to_dust(op, caster, spell_ob);
        break;

    case SP_CHANGE_ABILITY:
        success = cast_change_ability(op, caster, spell_ob, dir, 0);
        break;

    case SP_BLESS:
        success = cast_bless(op, caster, spell_ob, dir);
        break;

    case SP_CURSE:
        success = cast_curse(op, caster, spell_ob, dir);
        break;

    case SP_SUMMON_MONSTER:
        success = pets_summon_object(op, caster, spell_ob, dir, stringarg);
        break;

    case SP_CHARGING:
        success = recharge(op, caster, spell_ob);
        break;

    case SP_POLYMORPH:
#if 0
        /* Not great, but at least provide feedback so if players do have
         * polymorph (ie, find it as a preset item or left over from before
         * it was disabled), they get some feedback.
         */
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                      "The spell fizzles");
        success = 0;
#else
        success = cast_polymorph(op, caster, spell_ob, dir);
#endif
        break;

    case SP_ALCHEMY:
        success = alchemy(op, caster, spell_ob);
        break;

    case SP_REMOVE_CURSE:
        success = remove_curse(op, caster, spell_ob);
        break;

    case SP_IDENTIFY:
        success = cast_identify(op, caster, spell_ob);
        break;

    case SP_DETECTION:
        success = cast_detection(op, caster, spell_ob);
        break;

    case SP_MOOD_CHANGE:
        success = mood_change(op, caster, spell_ob);
        break;

    case SP_MOVING_BALL:
        if (spell_ob->path_repelled
        && (spell_ob->path_repelled&caster->path_attuned) != spell_ob->path_repelled) {
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                                 "You lack the proper attunement to cast %s",
                                 spell_ob->name);
            success = 0;
        } else
            success = fire_arch_from_position(op, caster, op->x, op->y, dir, spell_ob);
        break;

    case SP_SWARM:
        success = fire_swarm(op, caster, spell_ob, dir);
        break;

    case SP_CHANGE_MANA:
        success = cast_transfer(op, caster, spell_ob, dir);
        break;

    case SP_DISPEL_RUNE:
        /* in rune.c */
        success = dispel_rune(op, caster, spell_ob, skill, dir);
        break;

    case SP_CREATE_MISSILE:
        success = cast_create_missile(op, caster, spell_ob, dir, stringarg);
        break;

    case SP_CONSECRATE:
        success = cast_consecrate(op, caster, spell_ob);
        break;

    case SP_ANIMATE_WEAPON:
        success = animate_weapon(op, caster, spell_ob, dir);
        old_shoottype = range_golem;
        break;

    case SP_LIGHT:
        success = cast_light(op, caster, spell_ob, dir);
        break;

    case SP_CHANGE_MAP_LIGHT:
        success = cast_change_map_lightlevel(op, caster, spell_ob);
        break;

    case SP_FAERY_FIRE:
        success = cast_destruction(op, caster, spell_ob);
        break;

    case SP_CAUSE_DISEASE:
        success = cast_cause_disease(op, caster, spell_ob, dir);
        break;

    case SP_AURA:
        success = create_aura(op, caster, spell_ob);
        break;

    case SP_TOWN_PORTAL:
        success = cast_create_town_portal(op, caster, spell_ob, dir);
        break;

    case SP_ITEM_CURSE_BLESS:
        success = cast_item_curse_or_curse(op, caster, spell_ob);
        break;

    case SP_ELEM_SHIELD:
        success = create_aura(op, caster, spell_ob);
        break;

    default:
        LOG(llevError, "cast_spell: Unhandled spell subtype %d\n", spell_ob->subtype);
    }

    /* FIXME - we need some better sound suppport */
    /* free the spell arg */
    if (settings.casting_time == TRUE) {
        free(stringarg);
        stringarg = NULL;
    }
    /* perhaps a bit of a hack, but if using a wand, it has to change the skill
     * to something like use_magic_item, but you really want to be able to fire
     * it again.
     */
    if (op->contr)
        op->contr->shoottype = old_shoottype;

    return success;
}

/**
 * Stores in the spell when to warn player of expiration.
 *
 * @param spell
 * spell we're considering.
 */
void store_spell_expiry(object *spell) {
    /* Keep when to warn the player of expiration */
    char dur[10];
    int i = spell->duration/5;

    if (!i)
        i = 1;
    snprintf(dur, sizeof(dur), "%d", i);
    object_set_value(spell, "spell_expiry_warn_1", dur, 1);
    i = i/5;
    if (i > 0) {
        snprintf(dur, sizeof(dur), "%d", i);
        object_set_value(spell, "spell_expiry_warn_2", dur, 1);
    }
}

/**
 * Checks if player should be warned of soon expiring spell.
 *
 * Should be called at each move of the spell. Will use key stored by store_spell_expiry().
 * If the spell effect/force isn't in a player's inventory, won't do anything.
 *
 * @param spell
 * force or spell whose effects will expire.
 */
void check_spell_expiry(object *spell) {
    const char *key;

    if (!spell->env || !spell->env->type == PLAYER)
        return;

    key = object_get_value(spell, "spell_expiry_warn_1");
    if (key != NULL) {
        if (spell->duration == atoi(key)) {
            draw_ext_info_format(NDI_UNIQUE|NDI_NAVY, 0, spell->env, MSG_TYPE_SPELL, MSG_TYPE_SPELL_INFO,
                                 "The effects of your %s are draining out.", spell->name);
            return;
        }
    }
    key = object_get_value(spell, "spell_expiry_warn_2");
    if (key != NULL) {
        if (spell->duration == atoi(key)) {
            draw_ext_info_format(NDI_UNIQUE|NDI_NAVY, 0, spell->env, MSG_TYPE_SPELL, MSG_TYPE_SPELL_INFO,
                                 "The effects of your %s are about to expire.", spell->name);
            return;
        }
    }
}

/**
 * Adjusts rod attributes. This function must be called after a new rod has
 * been created.
 *
 * @param rod
 * the rod to update
 */
void rod_adjust(object *rod) {
    /*
     * Add 50 to both level an divisor to keep prices a little
     * more reasonable.  Otherwise, a high level version of a
     * low level spell can be worth tons a money (eg, level 20
     * rod, level 2 spell = 10 time multiplier).  This way, the
     * value are a bit more reasonable.
     */
    rod->value = rod->value*rod->inv->value*(rod->level+50)/(rod->inv->level+50);

    /*
     * Maxhp is used to denote how many 'charges' the rod holds
     * before.
     */
    rod->stats.maxhp = MAX(rod->stats.maxhp, 2)*SP_level_spellpoint_cost(rod, rod->inv, SPELL_HIGHEST);
    rod->stats.hp = rod->stats.maxhp;
}
