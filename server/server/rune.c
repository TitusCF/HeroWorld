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
 *
 * All rune-related functions.
 */

#include <global.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#endif
#include <spells.h>


#ifndef sqr
#define sqr(x) ((x)*(x))
#endif


/**
 * Player is attempting to write a magical rune.
 * This function does all checks for paths, sp/gr, ...
 *
 * @param op
 * rune writer.
 * @param caster
 * object used for casting this rune.
 * @param spell
 * writing spell.
 * @param dir
 * orientation of rune, direction rune's contained spell will be cast in, if applicable
 * @param runename
 * name of the rune or message displayed by the rune for a rune of marking.
 * @retval 0
 * no rune was written.
 * @retval 1
 * rune written.
 */
int write_rune(object *op, object *caster, object *spell, int dir, const char *runename) {
    object *rune_spell, *rune;
    char buf[MAX_BUF];
    mapstruct *m;
    sint16 nx, ny, gr;

    if (!dir) {
        dir = 1;
    }

    nx = op->x+freearr_x[dir];
    ny = op->y+freearr_y[dir];
    m = op->map;

    if (get_map_flags(m, &m, nx, ny, &nx, &ny)) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                      "Can't make a rune there!");
        return 0;
    }
    FOR_MAP_PREPARE(m, nx, ny, tmp)
        if (tmp->type == RUNE) {
                draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                              "You can't write a rune there.");
                return 0;
        }
    FOR_MAP_FINISH();

    if (spell->other_arch) {
        rune_spell = arch_to_object(spell->other_arch);
    } else {
        /* Player specified spell.  The player has to know the spell, so
         * lets just look through the players inventory see if they know it
         * use the object_matches_string() for our typical matching method.
         */
        int bestmatch = 0, ms;

        if (!runename || *runename == 0) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                          "Write a rune of what?");
            return 0;
        }

        rune_spell = NULL;
        FOR_INV_PREPARE(op, tmp) {
            if (tmp->type == SPELL) {
                ms = object_matches_string(op, tmp, runename);
                if (ms > bestmatch) {
                    bestmatch = ms;
                    rune_spell = tmp;
                }
            }
        } FOR_INV_FINISH();
        if (!rune_spell) {
            draw_ext_info_format(NDI_UNIQUE, 0, op,
                                 MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                                 "You don't know any spell named %s",
                                 runename);
            return 0;
        }
        if (rune_spell->skill != spell->skill) {
            draw_ext_info_format(NDI_UNIQUE, 0, op,
                                 MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                                 "You can't cast %s with %s",
                                 rune_spell->name, spell->name);
            return 0;
        }
        if (caster->path_denied&spell->path_attuned) {
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                                 "%s belongs to a spell path denied to you.",
                                 rune_spell->name);
            return 0;
        }
        if (caster_level(caster, rune_spell) < rune_spell->level) {
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                                 "%s is beyond your ability to cast!",
                                 rune_spell->name);
            return 0;
        }
        if (SP_level_spellpoint_cost(caster, rune_spell, SPELL_MANA) > op->stats.sp) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                          "You don't have enough mana.");
            return 0;
        }
        gr = SP_level_spellpoint_cost(caster, rune_spell, SPELL_GRACE);
        if ((gr > 0) && (gr > op->stats.grace)) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                          "You don't have enough grace.");
            return 0;
        }
        op->stats.grace -= gr;
        op->stats.sp -= SP_level_spellpoint_cost(caster, rune_spell, SPELL_MANA);
    }
    /* already proper rune.  Note this should only be the case if other_arch was set */
    if (rune_spell->type == RUNE) {
        rune = rune_spell;
    } else {
        object *tmp;

        rune = create_archetype(GENERIC_RUNE);
        snprintf(buf, sizeof(buf), "You set off a rune of %s\n", rune_spell->name);
        object_set_msg(rune, buf);
        tmp = object_new();
        object_copy(rune_spell, tmp);
        object_insert_in_ob(tmp, rune);
        if (spell->face != blank_face)
            rune->face = spell->face;
    }
    rune->level = caster_level(caster, spell);
    rune->stats.Cha = rune->level/2;  /* the invisibility parameter */
    rune->direction = dir;  /* where any spell will go upon detonation */
    object_set_owner(rune, op); /* runes without need no owner */
    set_spell_skill(op, caster, spell, rune);
    object_insert_in_map_at(rune, m, op, 0, nx, ny);
    return 1;
}

/**
 * This function handles those runes which detonate but do not cast spells.
 * Typically, poisoned or diseased runes.
 *
 * @param op
 * rune.
 * @param victim
 * victim of the rune.
 */
static void rune_attack(object *op, object *victim) {
    if (victim) {
        tag_t tag = victim->count;
        hit_player(victim, op->stats.dam, op, op->attacktype, 1);
        if (object_was_destroyed(victim, tag))
            return;
        /*  if there's a disease in the needle, put it in the player */
        if (HAS_RANDOM_ITEMS(op))
            create_treasure(op->randomitems, op, 0, (victim->map ? victim->map->difficulty : 1), 0);
        if (op->inv && op->inv->type == DISEASE) {
            object *disease = op->inv;

            infect_object(victim, disease, 1);
            object_remove(disease);
            object_free_drop_inventory(disease);
        }
    } else
        hit_map(op, 0, op->attacktype, 1);
}

/**
 * This function generalizes attacks by runes/traps.  This ought to make
 * it possible for runes to attack from the inventory,
 * it'll spring the trap on the victim.
 *
 * @param trap
 * trap that activates.
 * @param victim
 * victim of the trap.
 */
void spring_trap(object *trap, object *victim) {
    object *env;
    tag_t trap_tag = trap->count;
    rv_vector rv;
    int i, has_spell;

    /* Prevent recursion */
    if (trap->stats.hp <= 0)
        return;

    if (QUERY_FLAG(trap, FLAG_IS_LINKED))
        use_trigger(trap);

    /* Check if this trap casts a spell */
    has_spell = ((trap->inv && trap->inv->type == SPELL) || (trap->other_arch && trap->other_arch->clone.type == SPELL));

    env = object_get_env_recursive(trap);

    /* If the victim is not next to this trap, and the trap doesn't cast
     * a spell, don't set it off.
     */
    if (!get_rangevector(env, victim, &rv, 0) || (rv.distance > 1 && !has_spell))
        return;

    /* Only living objects can trigger runes that don't cast spells, as
     * doing direct damage to a non-living object doesn't work anyway.
     * Typical example is an arrow attacking a door.
     */
    if (!QUERY_FLAG(victim, FLAG_ALIVE) && !has_spell)
        return;

    if (!QUERY_FLAG(trap, FLAG_LIFESAVE))
        trap->stats.hp--;  /*decrement detcount */

    if (victim && victim->type == PLAYER && trap->msg != NULL && trap->msg[0] != '\0')
        draw_ext_info(NDI_UNIQUE, 0, victim, MSG_TYPE_APPLY, MSG_TYPE_APPLY_TRAP,
                      trap->msg);

    /*  Flash an image of the trap on the map so the poor sod
     *   knows what hit him.
     */
    trap_show(trap, env);

    /* Only if it is a spell do we proceed here */
    if (has_spell) {
        object *spell;

        /* This is necessary if the trap is inside something else */
        object_remove(trap);
        object_insert_in_map_at(trap, victim->map, trap, 0, victim->x, victim->y);

        if (object_was_destroyed(trap, trap_tag))
            return;

        for (i = 0; i < MAX(1, trap->stats.maxhp); i++) {
            if (trap->inv)
                cast_spell(trap, trap, trap->direction, trap->inv, NULL);
            else {
                spell = arch_to_object(trap->other_arch);
                cast_spell(trap, trap, trap->direction, spell, NULL);
                object_free_drop_inventory(spell);
            }
        }
    } else {
        rune_attack(trap, victim);
        if (object_was_destroyed(trap, trap_tag))
            return;
    }

    if (trap->stats.hp <= 0) {
        trap->type = SIGN;  /* make the trap impotent */
        trap->stats.food = 20;  /* make it stick around until its spells are gone */
        SET_FLAG(trap, FLAG_IS_USED_UP);
    }
}

/**
 * Someone is trying to disarm a rune. The actual attempt is done in trap_disarm().
 *
 * @param op
 * object trying to disarm.
 * @param caster
 * object casting the disarm spell.
 * @param spell
 * actual spell for casting.
 * @param skill
 * skill to disarm runes.
 * @param dir
 * direction to disarm.
 * @retval 0
 * rune wasn't disarmed.
 * @retval 1
 * a rune was disarmed.
 */
int dispel_rune(object *op, object *caster, object *spell, object *skill, int dir) {
    object *rune;
    int mflags;
    sint16 x, y;
    mapstruct *m;

    x = op->x+freearr_x[dir];
    y = op->y+freearr_y[dir];
    m = op->map;

    mflags = get_map_flags(m, &m, x, y, &x, &y);

    /* Should we perhaps not allow player to disable traps if a monster/
     * player is standing on top?
     */
    if (mflags&P_OUT_OF_MAP) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                      "There's nothing there!");
        return 0;
    }

    /* This can happen if a player does a 'magic rune of dispel'.  Without
     * any skill, chance of success is zero, and we don't know who to tell
     * (as otherwise we would have a skill pointer).  Plus, trap_disarm()
     * presumes skill is not null and will crash if it is.
     */
    if (!skill)
        return 0;

    rune = NULL;
    FOR_MAP_PREPARE(m, x, y, tmp) {
        object *tmp2;

        if (tmp->type == RUNE || tmp->type == TRAP) {
            rune = tmp;
            break;
        }

        /* we could put a probability chance here, but since nothing happens
         * if you fail, no point on that.  I suppose we could do a level
         * comparison so low level players can't erase high level players runes.
         */
        if (tmp->type == SIGN && !strcmp(tmp->arch->name, "rune_mark")) {
            object_remove(tmp);
            object_free_drop_inventory(tmp);
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_SUCCESS,
                          "You wipe out the rune of marking!");
            return 1;
        }

        /* now search tmp's inventory for traps
         * This is for chests, where the rune is in the chests inventory.
         */
        tmp2 = object_find_by_type2(tmp, RUNE, TRAP);
        if (tmp2 != NULL) {
            rune = tmp2;
            break;
        }
    } FOR_MAP_FINISH();

    /* no rune there. */
    if (rune == NULL) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                      "There's nothing there!");
        return 0;
    }
    trap_disarm(op, rune, 0, skill);
    return 1;
}

/**
 * Should op see trap?
 * @param op
 * living that could spot the trap.
 * @param trap
 * trap that is invisible.
 * @retval 0
 * trap wasn't spotted.
 * @retval 1
 * trap was spotted.
 */
int trap_see(object *op, object *trap) {
    int chance;

    chance = random_roll(0, 99, op, PREFER_HIGH);

    /* decide if we see the rune or not */
    if ((trap->stats.Cha == 1)
    || (chance > MIN(95, MAX(5, ((int)((float)(op->map->difficulty+trap->level+trap->stats.Cha-op->level)/10.0*50.0)))))) {
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                             "You spot a %s!",
                             trap->name);
        return 1;
    }
    return 0;
}

/**
 * Handles showing a trap/rune detonation.
 * @param trap
 * trap that detonates.
 * @param where
 * object at the location to detonate.
 * @retval 0
 * no animation inserted.
 * @retval 1
 * animation inserted.
 */
int trap_show(object *trap, object *where) {
    object *tmp2;

    if (where == NULL)
        return 0;
    tmp2 = create_archetype("runedet");
    tmp2->face = GET_ANIMATION(trap, 0);
    object_insert_in_map_at(tmp2, where->map, NULL, 0, where->x, where->y);
    return 1;
}

/**
 * Try to disarm a trap/rune.
 *
 * @param disarmer
 * object disarming the trap/rune.
 * @param trap
 * trap to disarm.
 * @param risk
 * if 0, trap/rune won't spring if disarm failure. Else it will spring.
 * @param skill
 * spell used to disarm.
 * @return
 * experience to award, 0 for failure.
 */
int trap_disarm(object *disarmer, object *trap, int risk, object *skill) {
    int trapworth;  /* need to compute the experience worth of the trap
                     before we kill it */

    /* this formula awards a more reasonable amount of exp */
    trapworth = MAX(1, trap->level)*disarmer->map->difficulty*
                sqr(MAX(trap->stats.dam, trap->inv ? trap->inv->level : 1))/skill->level;

    if (!(random_roll(0, (MAX(2, MIN(20, trap->level-skill->level+5-disarmer->stats.Dex/2))-1), disarmer, PREFER_LOW))) {
        object *owner;

        draw_ext_info_format(NDI_UNIQUE, 0, disarmer,
                             MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                             "You successfully disarm the %s!",
                             trap->name);
        destroy_object(trap);
        /* If it is your own trap, (or any players trap), don't you don't
         * get exp for it.
         */
        owner = object_get_owner(trap);
        if (owner != NULL && owner->type != PLAYER && risk)
            return trapworth;
        else
            return 1; /* give minimal exp and say success */
    } else {
        draw_ext_info_format(NDI_UNIQUE, 0, disarmer,
                             MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_FAILURE,
                             "You fail to disarm the %s.",
                             trap->name);
        if (!(random_roll(0, (MAX(2, skill->level-trap->level+disarmer->stats.Dex/2-6))-1, disarmer, PREFER_LOW))
        && risk) {
            draw_ext_info(NDI_UNIQUE, 0, disarmer,
                          MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                          "In fact, you set it off!");
            spring_trap(trap, disarmer);
        }
        return 0;
    }
}
