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
 * Various spell effects, non attacks.
 * @todo use the same parameter names/orders.
 */

#include <global.h>
#include <object.h>
#include <living.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#endif
#include <spells.h>
#include <sounds.h>

/**
 * This is really used mostly for spell fumbles and the like.
 *
 * @param op
 * what is casting this.
 * @param tmp
 * object to propagate.
 * @param lvl
 * how nasty should the propagation be.
 */
void cast_magic_storm(object *op, object *tmp, int lvl) {
    if (!tmp)
        return; /* error */
    tmp->level = op->level;
    tmp->range += lvl/5;  /* increase the area of destruction */
    tmp->duration += lvl/5;

    /* Put a cap on duration for this - if the player fails in their
     * apartment, don't want it to go on so long that it kills them
     * multiple times.  Also, damage already increases with level,
     * so don't really need to increase the duration as much either.
     */
    if (tmp->duration >= 40)
        tmp->duration = 40;
    tmp->stats.dam = lvl; /* nasty recoils! */
    tmp->stats.maxhp = tmp->count; /* tract single parent */
    if (tmp->stats.maxhp == 0)
        tmp->stats.maxhp = 1;
    object_insert_in_map_at(tmp, op->map, op, 0, op->x, op->y);
}

/**
 * Recharge wands.
 *
 * @param op
 * who is casting.
 * @param caster
 * what is casting.
 * @param spell_ob
 * spell object.
 * @retval 0
 * nothing happened.
 * @retval 1
 * wand was recharged, or destroyed.
 */
int recharge(object *op, object *caster, object *spell_ob) {
    object *wand, *tmp;
    int ncharges;
    char name[MAX_BUF];

    wand = find_marked_object(op);
    if (wand == NULL || wand->type != WAND) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                      "You need to mark the wand you want to recharge.");
        return 0;
    }
    if (!(random_roll(0, 3, op, PREFER_HIGH))) {
        query_name(wand, name, MAX_BUF);
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                             "The %s vibrates violently, then explodes!",
                             name);
        play_sound_map(SOUND_TYPE_ITEM, wand, 0, "explode");
        object_remove(wand);
        object_free2(wand, 0);
        tmp = create_archetype("fireball");
        tmp->stats.dam = (spell_ob->stats.dam+SP_level_dam_adjust(caster, spell_ob))/10;
        if (!tmp->stats.dam)
            tmp->stats.dam = 1;
        tmp->stats.hp = tmp->stats.dam/2;
        if (tmp->stats.hp < 2)
            tmp->stats.hp = 2;
        object_insert_in_map_at(tmp, op->map, NULL, 0, op->x, op->y);
        return 1;
    }

    ncharges = (spell_ob->stats.dam+SP_level_dam_adjust(caster, spell_ob));
    if (wand->inv && wand->inv->level)
        ncharges /= wand->inv->level;
    else {
        query_name(wand, name, MAX_BUF);
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                             "Your %s is broken.",
                             name);
        return 0;
    }
    if (!ncharges)
        ncharges = 1;

    wand->stats.food += ncharges;
    query_name(wand, name, MAX_BUF);
    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_SUCCESS,
                         "The %s glows with power.",
                         name);

    if (wand->arch && QUERY_FLAG(&wand->arch->clone, FLAG_ANIMATE)) {
        SET_FLAG(wand, FLAG_ANIMATE);
        wand->speed = wand->arch->clone.speed;
        object_update_speed(wand);
    }
    return 1;
}

/******************************************************************************
 * Start of polymorph related functions.
 *
 * Changed around for 0.94.3 - it will now look through and use all the
 * possible choices for objects/monsters (before it was the first 80 -
 * arbitrary hardcoded limit in this file.)  Doing this will be a bit
 * slower however - while before, it traversed the archetypes once and
 * stored them into an array, it will now potentially traverse it
 * an average of 1.5 times.  This is probably more costly on the polymorph
 * item function, since it is possible a couple lookups might be needed before
 * an item of proper value is generated.
 */

/**
 * Takes a living object (op) and turns it into
 * another monster of some sort.
 * @param op
 * object to try to polymorph. Should be monster.
 * @param level
 * level of the polymorph spell.
 */
static void polymorph_living(object *op, int level) {
    archetype *at;
    int x = op->x, y = op->y, numat = 0, choice, friendly;
    mapstruct *map = op->map;
    object *owner;

    op = HEAD(op);

    /* High level creatures are immune, as are creatures immune to magic.  Otherwise,
     * give the creature a saving throw.
     */
    if (op->level >= level*2
    || did_make_save(op, op->level, op->resist[ATNR_MAGIC]/10)
    || (op->resist[ATNR_MAGIC] == 100))
        return;

    object_remove(op);

    /* First, count up the number of legal matches */
    for (at = first_archetype; at != NULL; at = at->next)
        if ((QUERY_FLAG((&at->clone), FLAG_MONSTER) == QUERY_FLAG(op, FLAG_MONSTER))
        && (object_find_free_spot(&at->clone, map, x, y, 0, SIZEOFFREE) != -1)) {
            numat++;
        }
    if (!numat) {
        object_insert_in_map_at(op, map, NULL, 0, x, y);
        return; /* no valid matches? if so, return */
    }

    /* Next make a choice, and loop through until we get to it */
    choice = rndm(0, numat-1);
    for (at = first_archetype; at != NULL; at = at->next)
        if ((QUERY_FLAG((&at->clone), FLAG_MONSTER) == QUERY_FLAG(op, FLAG_MONSTER)) && (object_find_free_spot(&at->clone, map, x, y, 0, SIZEOFFREE) != -1)) {
            if (!choice)
                break;
            else
                choice--;
        }

    /* Look through the monster.  Unapply anything they have applied,
     * and remove any spells.  Note that if this is extended
     * to players, that would need to get fixed somehow.
     */
    FOR_INV_PREPARE(op, tmp) {
        if (QUERY_FLAG(tmp, FLAG_APPLIED))
            apply_manual(op, tmp, 0);
        if (tmp->type == SPELL) {
            object_remove(tmp);
            object_free2(tmp, 0);
        }
    } FOR_INV_FINISH();

    /* Preserve some values for the new object */
    owner = object_get_owner(op);
    friendly = QUERY_FLAG(op, FLAG_FRIENDLY);
    if (friendly)
        remove_friendly_object(op);

    object_copy(&(at->clone), op);
    if (owner != NULL)
        object_set_owner(op, owner);
    if (friendly) {
        SET_FLAG(op, FLAG_FRIENDLY);
        op->attack_movement = PETMOVE;
        add_friendly_object(op);
    } else
        CLEAR_FLAG(op, FLAG_FRIENDLY);

    /* Put the new creature on the map */
    if ((op = object_insert_in_map_at(op, map, owner, 0, x, y)) == NULL)
        return;

    if (HAS_RANDOM_ITEMS(op))
        create_treasure(op->randomitems, op, GT_INVISIBLE, map->difficulty, 0);

    /* Apply any objects. */
    monster_check_apply_all(op);
}


/**
 * Destroys item from polymorph failure
 * @note
 * We should probably do something
 * more clever ala nethack - create an iron golem or
 * something.
 * @param who
 * who cast the spell.
 * @param op
 * spell victim.
 */
static void polymorph_melt(object *who, object *op) {
    /* Not unique */
    char name[MAX_BUF];

    query_name(op, name, MAX_BUF);
    if (op->nrof > 1)
        draw_ext_info_format(NDI_GREY, 0, who, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                             "The %s glow red, melt and evaporate!",
                             name);
    else
        draw_ext_info_format(NDI_GREY, 0, who, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                             "The %s glows red, melts and evaporates!",
                             name);
    play_sound_map(SOUND_TYPE_ITEM, op, 0, "evaporate");
    object_remove(op);
    object_free2(op, 0);
    return;
}

/**
 * Changes an item to another item of similar type.
 * @param who
 * spell caster.
 * @param op
 * object being polymorphed. Should be a non living thing.
 * @param level
 * spell level, required for monster resistance.
 */
static void polymorph_item(object *who, object *op, int level) {
    archetype *at;
    int max_value, difficulty, tries = 0, choice, charges = op->stats.food, numat = 0;
    object *new_ob;
    mapstruct *m;
    sint16 x, y;

    /* We try and limit the maximum value of the changed object. */
    max_value = op->value*2;
    if (max_value > 2000*(level/10))
        max_value = 2000*(level/10)+(max_value-2000*(level/10))/3;

    /* Look through and try to find matching items.  Can't turn into something
     * invisible.  Also, if the value is too high now, it would almost
     * certainly be too high below.
     */
    for (at = first_archetype; at != NULL; at = at->next) {
        if (at->clone.type == op->type
        && !at->clone.invisible
        && at->clone.value > 0
        && at->clone.value < max_value
        && !QUERY_FLAG(&at->clone, FLAG_NO_DROP)
        && !QUERY_FLAG(&at->clone, FLAG_STARTEQUIP))
            numat++;
    }

    if (!numat)
        return;

    difficulty = op->magic*5;
    if (difficulty < 0)
        difficulty = 0;
    new_ob = object_new();
    do {
        choice = rndm(0, numat-1);
        for (at = first_archetype; at != NULL; at = at->next) {
            if (at->clone.type == op->type
            && !at->clone.invisible
            && at->clone.value > 0
            && at->clone.value < max_value
            && !QUERY_FLAG(&at->clone, FLAG_NO_DROP)
            && !QUERY_FLAG(&at->clone, FLAG_STARTEQUIP)) {
                if (!choice)
                    break;
                else
                    choice--;
            }
        }
        object_copy(&(at->clone), new_ob);
        fix_generated_item(new_ob, op, difficulty, FABS(op->magic), GT_ENVIRONMENT);
        ++tries;
    } while (new_ob->value > max_value && tries < 10);
    if (new_ob->invisible) {
        LOG(llevError, "polymorph_item: fix_generated_object made %s invisible?!\n", new_ob->name);
        object_free2(new_ob, FREE_OBJ_NO_DESTROY_CALLBACK);
        return;
    }

    /* Unable to generate an acceptable item?  Melt it */
    if (tries == 10) {
        polymorph_melt(who, op);
        object_free2(new_ob, FREE_OBJ_NO_DESTROY_CALLBACK);
        return;
    }

    if (op->nrof && new_ob->nrof) {
        new_ob->nrof = op->nrof;
        /* decrease the number of items */
        if (new_ob->nrof > 2)
            new_ob->nrof -= rndm(0, op->nrof/2-1);
    }

    /* We don't want rings to keep sustenance/hungry status. There are probably
     *  other cases too that should be checked.
     */
    if (charges && op->type != RING && op->type != FOOD)
        op->stats.food = charges;

    x = op->x;
    y = op->y;
    m = op->map;
    object_remove(op);
    object_free2(op, FREE_OBJ_NO_DESTROY_CALLBACK);
    /*
     * Don't want objects merged or re-arranged, as it then messes up the
     * order
     */
    object_insert_in_map_at(new_ob, m, new_ob, INS_NO_MERGE|INS_NO_WALK_ON, x, y);
}

/**
 * Handles polymorphing an object, living or not.
 * Will avoid some specific items (flying arrows and such).
 * @param op
 * object being polymorphed.
 * @param who
 * spell caster.
 * @param level
 * spell level.
 */
void polymorph(object *op, object *who, int level) {
    int tmp;

    /* Can't polymorph players right now */
    /* polymorphing generators opens up all sorts of abuses */
    if (op->type == PLAYER || QUERY_FLAG(op, FLAG_GENERATOR))
        return;

    if (QUERY_FLAG(op, FLAG_MONSTER)) {
        polymorph_living(op, level);
        return;
    }
    /* If it is a living object of some other type, don't handle
     * it now.
     */
    if (QUERY_FLAG(op, FLAG_ALIVE))
        return;

    /* Don't want to morph flying arrows, etc... */
    if (FABS(op->speed) > 0.001 && !QUERY_FLAG(op, FLAG_ANIMATE))
        return;

    /* Do some sanity checking here.  type=0 is unknown, objects
     * without archetypes are not good.  As are a few other
     * cases.
     */
    if (op->type == 0
    || op->arch == NULL
    || QUERY_FLAG(op, FLAG_NO_PICK)
    || op->move_block
    || op->type == TREASURE)
        return;

    tmp = rndm(0, 7);
    if (tmp)
        polymorph_item(who, op, level);
    else
        polymorph_melt(who, op);
}


/**
 * Polymorph spell casting.
 * @param op
 * who is casting the spell.
 * @param caster
 * object used to cast spell.
 * @param spell_ob
 * spell itself.
 * @param dir
 * casting direction. 0 won't have any effect.
 * @return
 * Returns 0 on illegal cast, otherwise 1.
 */
int cast_polymorph(object *op, object *caster, object *spell_ob, int dir) {
    object *tmp;
    int range, mflags, maxrange, level;
    mapstruct *m;

    if (dir == 0)
        return 0;

    maxrange = spell_ob->range+SP_level_range_adjust(caster, spell_ob);
    level = caster_level(caster, spell_ob);
    for (range = 1; range < maxrange; range++) {
        sint16 x = op->x+freearr_x[dir]*range, y = op->y+freearr_y[dir]*range;
        object *image;

        m = op->map;
        mflags = get_map_flags(m, &m, x, y, &x, &y);

        if (mflags&(P_NO_MAGIC|P_OUT_OF_MAP))
            break;

        if (GET_MAP_MOVE_BLOCK(m, x, y)&MOVE_FLY_LOW)
            break;

        /* Get the top most object */
        for (tmp = GET_MAP_OB(m, x, y); tmp != NULL && tmp->above != NULL; tmp = tmp->above)
            ;

        /* Now start polymorphing the objects, top down */
        FOR_OB_AND_BELOW_PREPARE(tmp) {
            /* Once we find the floor, no need to go further */
            if (QUERY_FLAG(tmp, FLAG_IS_FLOOR))
                break;
            polymorph(tmp, op, level);
        } FOR_OB_AND_BELOW_FINISH();
        image = arch_to_object(spell_ob->other_arch);
        image->stats.food = 5;
        image->speed_left = 0.1;
        object_insert_in_map_at(image, m, op, 0, x, y);
    }
    return 1;
}

/**
 * Create a missile (nonmagic - magic +4). Will either create bolts or arrows
 * based on whether a crossbow or bow is equiped. If neither, it defaults to
 * arrows.
 * Sets the plus based on the casters level. It is also settable with the
 * invoke command. If the caster attempts to create missiles with too
 * great a plus, the default is used.
 * The # of arrows created also goes up with level, so if a 30th level mage
 * wants LOTS of arrows, and doesn't care what the plus is he could
 * create nonnmagic arrows, or even -1, etc...
 *
 * @param op
 * who is casting.
 * @param caster
 * what is casting.
 * @param spell
 * actual spell object.
 * @param dir
 * casting direction.
 * @param stringarg
 * optional parameter specifying what kind of items to create.
 * @retval 0
 * no missile created.
 * @retval
 * missiles were created.
 */
int cast_create_missile(object *op, object *caster, object *spell, int dir, const char *stringarg) {
    int missile_plus = 0, bonus_plus = 0;
    const char *missile_name;
    object *tmp, *missile;
    tag_t tag;

    tmp = object_find_by_type_applied(op, BOW);
    missile_name = tmp != NULL ? tmp->race : "arrow";

    missile_plus = spell->stats.dam+SP_level_dam_adjust(caster, spell);

    if (!strcmp(missile_name, "arrows"))
        missile_name = "arrow";
    else if (!strcmp(missile_name, "crossbow bolts"))
        missile_name = "bolt";

    if (find_archetype(missile_name) == NULL) {
        LOG(llevDebug, "Cast create_missile: could not find archetype %s\n", missile_name);
        return 0;
    }
    missile = create_archetype(missile_name);

    if (stringarg) {
        /* If it starts with a letter, presume it is a description */
        if (isalpha(*stringarg)) {
            artifact *al = find_artifactlist(missile->type)->items;

            for (; al != NULL; al = al->next)
                if (!strcasecmp(al->item->name, stringarg))
                    break;

            if (!al) {
                object_free2(missile, FREE_OBJ_NO_DESTROY_CALLBACK);
                draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                                     "No such object %ss of %s",
                                     missile_name, stringarg);
                return 0;
            }
            if (al->item->slaying) {
                object_free2(missile, FREE_OBJ_NO_DESTROY_CALLBACK);
                draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                                     "You are not allowed to create %ss of %s",
                                     missile_name, stringarg);
                return 0;
            }
            give_artifact_abilities(missile, al->item);
            /* These special arrows cost something extra.  Don't have them also be
            * magical - otherwise, in most cases, not enough will be created.
            * I don't want to get into the parsing both plus and type.
            */
            bonus_plus = 1+(al->item->value/5);
            missile_plus = 0;
        } else if (atoi(stringarg) < missile_plus)
            missile_plus = atoi(stringarg);
    }
    if (missile_plus > 4)
        missile_plus = 4;
    else if (missile_plus < -4)
        missile_plus = -4;

    missile->nrof = spell->duration+SP_level_duration_adjust(caster, spell);
    if (missile->nrof <= 3*(missile_plus+bonus_plus)) {
        object_free2(missile, FREE_OBJ_NO_DESTROY_CALLBACK);
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                             "This item is too powerful for you to create!");
        return 0;
    }
    missile->nrof -= 3*(missile_plus+bonus_plus);
    if (missile->nrof < 1)
        missile->nrof = 1;

    missile->magic = missile_plus;
    /* Can't get any money for these objects */
    missile->value = 0;

    SET_FLAG(missile, FLAG_IDENTIFIED);
    tag = missile->count;

    if (!cast_create_obj(op, missile, dir)
    && op->type == PLAYER
    && !object_was_destroyed(missile, tag)) {
        pick_up(op, missile);
    }
    return 1;
}


/**
 * Create food.
 * Allows the choice of what sort of food object to make.
 * If stringarg is NULL, it will create food dependent on level  --PeterM
 *
 * @param op
 * who is casting.
 * @param caster
 * what is casting.
 * @param spell_ob
 * actual spell object.
 * @param dir
 * casting direction.
 * @param stringarg
 * optional parameter specifying what kind of items to create.
 * @retval 0
 * no food created.
 * @retval
 * food was created.
 */
int cast_create_food(object *op, object *caster, object *spell_ob, int dir, const char *stringarg) {
    int food_value;
    archetype *at = NULL;
    object *new_op;

    food_value = spell_ob->stats.food+50*SP_level_duration_adjust(caster, spell_ob);

    if (stringarg) {
        at = find_archetype_by_object_type_name(FOOD, stringarg);
        if (at == NULL)
            at = find_archetype_by_object_type_name(DRINK, stringarg);
        if (at == NULL || at->clone.stats.food > food_value)
            stringarg = NULL;
    }

    if (!stringarg) {
        archetype *at_tmp;

        /* We try to find the archetype with the maximum food value.
         * This removes the dependency of hard coded food values in this
         * function, and addition of new food types is automatically added.
         * We don't use flesh types because the weight values of those need
         * to be altered from the donor.
         */

        /* We assume the food items don't have multiple parts */
        for (at_tmp = first_archetype; at_tmp != NULL; at_tmp = at_tmp->next) {
            if (at_tmp->clone.type == FOOD || at_tmp->clone.type == DRINK) {
                /* Basically, if the food value is something that is creatable
                 * under the limits of the spell and it is higher than
                 * the item we have now, take it instead.
                 */
                if (at_tmp->clone.stats.food <= food_value
                && (!at || at_tmp->clone.stats.food > at->clone.stats.food))
                    at = at_tmp;
            }
        }
    }
    /* Pretty unlikely (there are some very low food items), but you never
     * know
     */
    if (!at) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                      "You don't have enough experience to create any food.");
        return 0;
    }

    food_value /= at->clone.stats.food;
    new_op = object_new();
    object_copy(&at->clone, new_op);
    new_op->nrof = food_value;

    new_op->value = 0;
    if (new_op->nrof < 1)
        new_op->nrof = 1;

    cast_create_obj(op, new_op, dir);
    return 1;
}

/**
 * Try to get information about a living thing.
 *
 * @param op
 * who is casting.
 * @param caster
 * what is casting.
 * @param spell_ob
 * spell object being cast.
 * @param dir
 * cast direction.
 * @param level
 * probe level.
 * @retval 0
 * nothing probed.
 * @retval 1
 * something was probed.
 */
int probe(object *op, object *caster, object *spell_ob, int dir, int level) {
    int r, mflags, maxrange;
    mapstruct *m;

    if (!dir) {
        examine_monster(op, op, level);
        return 1;
    }
    maxrange = spell_ob->range+SP_level_range_adjust(caster, spell_ob);
    for (r = 1; r < maxrange; r++) {
        sint16 x = op->x+r*freearr_x[dir], y = op->y+r*freearr_y[dir];

        m = op->map;
        mflags = get_map_flags(m, &m, x, y, &x, &y);

        if (mflags&P_OUT_OF_MAP)
            break;

        if (!QUERY_FLAG(op, FLAG_WIZCAST) && (mflags&P_NO_MAGIC)) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                          "Something blocks your magic.");
            return 0;
        }
        if (mflags&P_IS_ALIVE) {
            FOR_MAP_PREPARE(m, x, y, tmp)
                if (QUERY_FLAG(tmp, FLAG_ALIVE) && (tmp->type == PLAYER || QUERY_FLAG(tmp, FLAG_MONSTER))) {
                    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_SUCCESS,
                                  "You detect something.");
                    examine_monster(op, HEAD(tmp), level);
                    return 1;
                }
            FOR_MAP_FINISH();
        }
    }
    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                  "You detect nothing.");
    return 1;
}


/**
 * This checks to see if 'pl' is invisible to 'mon'.
 * Does race check, undead check, etc
 * Returns TRUE if mon can't see pl, false
 * otherwise.  This doesn't check range, walls, etc.  It
 * only checks the racial adjustments, and in fact that
 * pl is invisible.
 *
 * @param pl
 * potentially invisible object.
 * @param mon
 * who may see pl.
 * @retval 0
 * mon can see pl.
 * @retval 1
 * mon can't see pl.
 */
int makes_invisible_to(object *pl, object *mon) {
    if (!pl->invisible)
        return 0;
    if (pl->type == PLAYER) {
        /* If race isn't set, then invisible unless it is undead */
        if (!pl->contr->invis_race) {
            if (QUERY_FLAG(mon, FLAG_UNDEAD))
                return 0;
            return 1;
        }
        /* invis_race is set if we get here */
        if (!strcmp(pl->contr->invis_race, "undead") && is_true_undead(mon))
            return 1;
        /* No race, can't be invisible to it */
        if (!mon->race)
            return 0;
        if (strstr(mon->race, pl->contr->invis_race))
            return 1;
        /* Nothing matched above, return 0 */
        return 0;
    } else {
        /* monsters are invisible to everything */
        return 1;
    }
}

/**
 * Makes the player or character invisible.
 * Note the spells to 'stack', but perhaps in odd ways.
 * the duration for all is cumulative.
 * In terms of invis undead/normal invis, it is the last one cast that
 * will determine if you are invisible to undead or normal monsters.
 * For improved invis, if you cast it with a one of the others, you
 * lose the improved part of it, and the above statement about undead/
 * normal applies.
 *
 * @param op
 * who is casting.
 * @param caster
 * what is casting.
 * @param spell_ob
 * actual spell object.
 * @retval 0
 * invisibility was already in action.
 * @retval 1
 * op is now invisible.
 */
int cast_invisible(object *op, object *caster, object *spell_ob) {
    object *tmp;

    if (op->invisible > 1000) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                      "You can not extend the duration of your invisibility any further");
        return 0;
    }

    /* Remove the switch with 90% duplicate code - just handle the differences with
     * and if statement or two.
     */
    op->invisible += spell_ob->duration+SP_level_duration_adjust(caster, spell_ob);
    /* max duration */
    if (op->invisible > 1000)
        op->invisible = 1000;

    if (op->type == PLAYER) {
        if (op->contr->invis_race)
            FREE_AND_CLEAR_STR(op->contr->invis_race);
        if (spell_ob->race)
            op->contr->invis_race = add_refcount(spell_ob->race);
        if (QUERY_FLAG(spell_ob, FLAG_MAKE_INVIS))
            op->contr->tmp_invis = 0;
        else
            op->contr->tmp_invis = 1;

        op->contr->hidden = 0;
    }
    if (makes_invisible_to(op, op))
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_SUCCESS,
                      "You can't see your hands!");
    else
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_SUCCESS,
                      "You feel more transparent!");

    object_update(op, UP_OBJ_FACE);

    /* Only search the active objects - only these should actually do
     * harm to the player.
     */
    for (tmp = active_objects; tmp != NULL; tmp = tmp->active_next)
        if (tmp->enemy == op)
            object_set_enemy(tmp, NULL);
    return 1;
}

/**
 * Basically destroys earthwalls in the area.
 *
 * @param op
 * who is casting.
 * @param caster
 * what is casting.
 * @param spell_ob
 * actual spell object.
 * @retval 0
 * op isn't a player.
 * @retval 1
 * op is a player.
 */
int cast_earth_to_dust(object *op, object *caster, object *spell_ob) {
    int range, i, j, mflags;
    sint16 sx, sy;
    mapstruct *m;

    if (op->type != PLAYER)
        return 0;

    range = spell_ob->range+SP_level_range_adjust(caster, spell_ob);

    for (i = -range; i < range; i++)
        for (j = -range; j < range; j++) {
            sx = op->x+i;
            sy = op->y+j;
            m = op->map;
            mflags = get_map_flags(m, &m, sx, sy, &sx, &sy);

            if (mflags&P_OUT_OF_MAP)
                continue;

            /* If the space doesn't block, no wall here to remove
             * Don't care too much what it blocks - this allows for
             * any sort of earthwall/airwall/waterwall, etc
             * type effects.
             */
            if (GET_MAP_MOVE_BLOCK(m, sx, sy)) {
                FOR_MAP_PREPARE(m, sx, sy, tmp)
                    if (tmp && QUERY_FLAG(tmp, FLAG_TEAR_DOWN))
                        hit_player(tmp, 9998, op, AT_PHYSICAL, 0);
                FOR_MAP_FINISH();
            }
        }
    return 1;
}

/**
 * Word of recall causes the player to return 'home'.
 * we put a force into the player object, so that there is a
 * time delay effect.
 *
 * @param op
 * who is casting.
 * @param caster
 * what is casting.
 * @param spell_ob
 * actual spell object.
 * @retval 0
 * op isn't a player.
 * @retval 1
 * word of recall initiated.
 */
int cast_word_of_recall(object *op, object *caster, object *spell_ob) {
    object *dummy;
    int time;

    if (op->type != PLAYER)
        return 0;

    if (object_find_by_type_subtype(op, SPELL_EFFECT, SP_WORD_OF_RECALL)) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_SUCCESS,
                      "You feel a force starting to build up inside you.");
        return 1;
    }

    dummy = create_archetype(FORCE_NAME);
    if (dummy == NULL) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                      "Oops, program error!");
        LOG(llevError, "cast_word_of_recall: create_archetype(force) failed!\n");
        return 0;
    }
    time = spell_ob->duration-SP_level_duration_adjust(caster, spell_ob);
    if (time < 1)
        time = 1;

    /* value of speed really doesn't make much difference, as long as it is
     * positive.  Lower value may be useful so that the problem doesn't
     * do anything really odd if it say a -1000 or something.
     */
    dummy->speed = 0.002;
    object_update_speed(dummy);
    dummy->speed_left = -dummy->speed*time;
    dummy->type = SPELL_EFFECT;
    dummy->subtype = SP_WORD_OF_RECALL;

    /* If we could take advantage of enter_player_savebed() here, it would be
     * nice, but until the map load fails, we can't.
     */
    EXIT_PATH(dummy) = add_string(op->contr->savebed_map);
    EXIT_X(dummy) = op->contr->bed_x;
    EXIT_Y(dummy) = op->contr->bed_y;

    (void)object_insert_in_ob(dummy, op);
    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_SUCCESS,
                  "You feel a force starting to build up inside you.");
    return 1;
}

/**
 * wonder is really just a spell that will likely cast another
 * spell.
 *
 * @param op
 * who is casting.
 * @param caster
 * what is casting.
 * @param dir
 * casting direction.
 * @param spell_ob
 * actual spell object.
 * @todo
 * doesn't it decrease sp without checking?
 */
int cast_wonder(object *op, object *caster, int dir, object *spell_ob) {
    object *newspell;

    if (!rndm(0, 3))
        return cast_cone(op, caster, dir, spell_ob);

    if (spell_ob->randomitems) {
        newspell = generate_treasure(spell_ob->randomitems, caster->level);
        if (!newspell) {
            LOG(llevError, "cast_wonder: Unable to get a spell!\n");
            return 0;
        }
        if (newspell->type != SPELL) {
            LOG(llevError, "cast_wonder: spell returned is not a spell (%d, %s)!\n", newspell->type, newspell->name);
            return 0;
        }
        /* Prevent inifinite recursion */
        if (newspell->subtype == SP_WONDER) {
            LOG(llevError, "cast_wonder: spell returned is another wonder spell!\n");
            return 0;
        }
        return cast_spell(op, caster, dir, newspell, NULL);
    }
    return 1;
}

/**
 * Living thing wants to know information.
 *
 * @param op
 * who wants information.
 * @return
 * 1.
 */
int perceive_self(object *op) {
    char *cp, buf[MAX_BUF];
    archetype *at = find_archetype(ARCH_DEPLETION);
    object *tmp;
    const object *god;
    int i;
    StringBuffer *immunity;

    god = find_god(determine_god(op));
    if (god)
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_PERCEIVE_SELF,
                             "You worship %s",
                             god->name);
    else
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_PERCEIVE_SELF,
                      "You worship no god");

    tmp = arch_present_in_ob(at, op);

    cp = stringbuffer_finish(describe_item(op, op, NULL));

    if (*cp == '\0' && tmp == NULL)
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_PERCEIVE_SELF,
                      "You feel very mundane");
    else {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_PERCEIVE_SELF,
                      "You have:");
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_PERCEIVE_SELF,
                      cp);
        if (tmp != NULL) {
            for (i = 0; i < NUM_STATS; i++) {
                if (get_attr_value(&tmp->stats, i) < 0) {
                    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_PERCEIVE_SELF,
                                         "Your %s is depleted by %d",
                                         statname[i], -(get_attr_value(&tmp->stats, i)));
                }
            }
        }
    }
    free(cp);

    if (op->glow_radius > 0)
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_PERCEIVE_SELF,
                      "You glow in the dark.");

    immunity = NULL;
    for (tmp = op->inv; tmp; tmp = tmp->below) {
        if (tmp->type == SIGN) {
            if (immunity == NULL) {
                immunity = stringbuffer_new();
                stringbuffer_append_string(immunity, "You have been exposed to: ");
            } else {
                stringbuffer_append_string(immunity, ", ");
            }
            stringbuffer_append_string(immunity, tmp->name);
            if (tmp->level > 100)
                stringbuffer_append_string(immunity, " (full immunity)");
            else if (tmp->level > 70)
                stringbuffer_append_string(immunity, " (high immunity)");
            else if (tmp->level > 20)
                stringbuffer_append_string(immunity, " (partial immunity)");
        }
    }

    if (immunity != NULL) {
        cp = stringbuffer_finish(immunity);
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_PERCEIVE_SELF, cp);
        free(cp);
    }

    if (is_dragon_pl(op)) {
        /* now grab the 'dragon_ability'-force from the player's inventory */
        tmp = object_find_by_type_and_arch_name(op, FORCE, "dragon_ability_force");
        if (tmp != NULL) {
            StringBuffer *levels = NULL;
            int i;

            if (tmp->stats.exp == 0) {
                snprintf(buf, sizeof(buf), "Your metabolism isn't focused on anything.");
            } else {
                snprintf(buf, sizeof(buf), "Your metabolism is focused on %s.", change_resist_msg[tmp->stats.exp]);
            }
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_PERCEIVE_SELF,
                          buf);

            for (i = 0; i < NROFATTACKS; i++) {
                if (atnr_is_dragon_enabled(i) && tmp->resist[i] > 0) {
                    if (levels == NULL) {
                        levels = stringbuffer_new();
                        stringbuffer_append_string(levels, "Ability levels:\n");
                    }
                    stringbuffer_append_printf(levels, "- %s: %d\n", change_resist_msg[i], tmp->resist[i]);
                }
            }

            if (levels != NULL) {
                cp = stringbuffer_finish(levels);
                draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_PERCEIVE_SELF, cp);
                free(cp);
            }
        }
    }
    return 1;
}

/**
 * This function cast the spell of town portal for op.
 *
 * The spell operates in two passes. During the first one a place
 * is marked as a destination for the portal. During the second one,
 * 2 portals are created, one in the position the player cast it and
 * one in the destination place. The portal are synchronized and 2 forces
 * are inserted in the player to destruct the portal next time player
 * creates a new portal pair.
 * This spell has a side effect that it allows people to meet each other
 * in a permanent, private, apartments by making a town portal from it
 * to the town or another public place. So, check if the map is unique and if
 * so return an error
 *
 * Code by Tchize (david.delbecq@usa.net)
 *
 * @param op
 * who is casting.
 * @param caster
 * what is casting.
 * @param spell
 * actual spell object.
 * @param dir
 * casting direction.
 * @retval 0
 * spell was insuccessful for some reason.
 * @retval 1
 * spell worked.
 */
int cast_create_town_portal(object *op, object *caster, object *spell, int dir) {
    object *dummy, *force, *old_force, *tmp;
    archetype *perm_portal;
    char portal_name [1024], portal_message [1024];
    sint16 exitx, exity;
    mapstruct *exitmap;
    int op_level, x, y;

    /* Check to see if the map the player is currently on is a per player unique
     * map.  This can be determined in that per player unique maps have the
     * full pathname listed. Ignore if settings.create_home_portals is true.
     */
    if (!settings.create_home_portals) {
        if (!strncmp(op->map->path, settings.localdir, strlen(settings.localdir))) {
            draw_ext_info(NDI_UNIQUE|NDI_NAVY, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                          "You can't cast that here.");
            return 0;
        }
    }

    /* Check to see if the player is on a transport */
    if (op->contr && op->contr->transport) {
        draw_ext_info(NDI_UNIQUE|NDI_NAVY, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                      "You need to exit the transport to cast that.");
        return 0;
    }

    /* The first thing to do is to check if we have a marked destination
     * dummy is used to make a check inventory for the force
     */
    dummy = arch_to_object(spell->other_arch);
    if (dummy == NULL) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                      "Oops, program error!");
        LOG(llevError, "object_new failed (force in cast_create_town_portal for %s!\n", op->name);
        return 0;
    }
    force = check_inv_recursive(op, dummy);

    if (force == NULL) {
        /* Here we know there is no destination marked up.
         * We have 2 things to do:
         * 1. Mark the destination in the player inventory.
         * 2. Let the player know it worked.
         */
        free_string(dummy->name);
        dummy->name = add_string(op->map->path);
        EXIT_X(dummy) = op->x;
        EXIT_Y(dummy) = op->y;
        dummy->weapontype = op->map->last_reset_time.tv_sec;
        object_insert_in_ob(dummy, op);
        draw_ext_info(NDI_UNIQUE|NDI_NAVY, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_SUCCESS,
                      "You fix this place in your mind and feel that you "
                      "can come here from anywhere.");
        return 1;
    }
    object_free2(dummy, FREE_OBJ_NO_DESTROY_CALLBACK);

    /* Here we know where the town portal should go to
     * We should kill any existing portal associated with the player.
     * Than we should create the 2 portals.
     * For each of them, we need:
     *    - To create the portal with the name of the player+destination map
     *    - set the owner of the town portal
     *    - To mark the position of the portal in the player's inventory
     *      for easier destruction.
     *
     *  The mark works has follow:
     *   slaying: Existing town portal
     *   hp, sp : x & y of the associated portal
     *   name   : name of the portal
     *   race   : map the portal is in
     */

    /* First step: killing existing town portals */
    dummy = create_archetype(spell->race);
    if (dummy == NULL) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                      "Oops, program error!");
        LOG(llevError, "object_new failed (force) in cast_create_town_portal for %s!\n", op->name);
        return 0;
    }
    perm_portal = find_archetype(spell->slaying);

    /* To kill a town portal, we go trough the player's inventory,
     * for each marked portal in player's inventory,
     *   -We try load the associated map (if impossible, consider the portal destructed)
     *   -We find any portal in the specified location.
     *      If it has the good name, we destruct it.
     *   -We destruct the force indicating that portal.
     */
    while ((old_force = check_inv_recursive(op, dummy))) {
        exitx = EXIT_X(old_force);
        exity = EXIT_Y(old_force);
        LOG(llevDebug, "Trying to kill a portal in %s (%d,%d)\n", old_force->race, exitx, exity);

        if (!strncmp(old_force->race, settings.localdir, strlen(settings.localdir)))
            exitmap = ready_map_name(old_force->race, MAP_PLAYER_UNIQUE);
        else
            exitmap = ready_map_name(old_force->race, 0);

        if (exitmap) {
            tmp = map_find_by_archetype(exitmap, exitx, exity, perm_portal);
            FOR_OB_AND_ABOVE_PREPARE(tmp)
                if (tmp->name == old_force->name) {
                    object_remove(tmp);
                    object_free2(tmp, 0);
                    break;
                }
            FOR_OB_AND_ABOVE_FINISH();

            /* kill any opening animation there is */
            tmp = map_find_by_archetype(exitmap, exitx, exity, find_archetype("town_portal_open"));
            FOR_OB_AND_ABOVE_PREPARE(tmp)
                if (tmp->name == old_force->name) {
                object_remove(tmp);
                object_free2(tmp, FREE_OBJ_FREE_INVENTORY);
                break;
            }
            FOR_OB_AND_ABOVE_FINISH();
        }
        object_remove(old_force);
        object_free2(old_force, 0);
        LOG(llevDebug, "\n");
    }
    object_free2(dummy, FREE_OBJ_NO_DESTROY_CALLBACK);

    /* Creating the portals.
     * The very first thing to do is to ensure
     * access to the destination map.
     * If we can't, don't fizzle. Simply warn player.
     * This ensure player pays his mana for the spell
     * because HE is responsible for forgetting.
     * 'force' is the destination of the town portal, which we got
     * from the players inventory above.
     */

    /* Ensure exit map is loaded*/
    if (!strncmp(force->name, settings.localdir, strlen(settings.localdir)))
        exitmap = ready_map_name(force->name, MAP_PLAYER_UNIQUE);
    else
        exitmap = ready_map_name(force->name, 0);

    /* If we were unable to load (ex. random map deleted), warn player*/
    if (exitmap == NULL) {
        draw_ext_info(NDI_UNIQUE|NDI_NAVY, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                      "Something strange happens. You can't remember where to go!?");
        object_remove(force);
        object_free2(force, 0);
        return 1;
    } else if (exitmap->last_reset_time.tv_sec != force->weapontype) {
        draw_ext_info(NDI_UNIQUE|NDI_NAVY, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                      "The spell effect has expired.");
        object_remove(force);
        object_free2(force, 0);
        return 1;
    }

    op_level = caster_level(caster, spell);
    if (op_level < 15)
        snprintf(portal_message, 1024, "\nThe air moves around you and\na huge smell of ammonia surrounds you as you pass through %s's tiny portal\nPouah!\n", op->name);
    else if (op_level < 30)
        snprintf(portal_message, 1024, "\n%s's portal smells of ozone.\nYou do a lot of movements and finally pass through the small hole in the air\n", op->name);
    else if (op_level < 60)
        snprintf(portal_message, 1024, "\nA shining door opens in the air in front of you, showing you the path to another place.\n");
    else
        snprintf(portal_message, 1024, "\nAs you walk through %s's portal, flowers come out from the ground around you.\nYou feel awed.\n", op->name);

    /* Create a portal in front of player
     * dummy contain the portal and
     * force contain the track to kill it later
     */

    snprintf(portal_name, 1024, "%s's portal to %s", op->name, force->name);
    dummy = create_archetype(spell->slaying); /*The portal*/
    if (dummy == NULL) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                      "Oops, program error!");
        LOG(llevError, "object_new failed (perm_magic_portal) in cast_create_town_portal for %s!\n", op->name);
        return 0;
    }
    EXIT_PATH(dummy) = add_string(force->name);
    EXIT_X(dummy) = EXIT_X(force);
    EXIT_Y(dummy) = EXIT_Y(force);
    FREE_AND_COPY(dummy->name, portal_name);
    FREE_AND_COPY(dummy->name_pl, portal_name);
    object_set_msg(dummy, portal_message);
    dummy->race = add_string(op->name);   /*Save the owner of the portal*/

    /* create a nice animation */
    tmp = create_archetype("town_portal_open");
    FREE_AND_COPY(tmp->name, portal_name);
    FREE_AND_COPY(tmp->name_pl, portal_name);
    object_insert_in_ob(dummy, tmp);
    /* and put it on the floor, when it ends the portal will be on the ground */
    cast_create_obj(op, tmp, 0);
    x = tmp->x;
    y = tmp->y;

    /* Now we need to to create a town portal marker inside the player
     * object, so on future castings, we can know that he has an active
     * town portal.
     */
    tmp = create_archetype(spell->race);
    if (tmp == NULL) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                      "Oops, program error!");
        LOG(llevError, "object_new failed (force) in cast_create_town_portal for %s!\n", op->name);
        return 0;
    }
    tmp->race = add_string(op->map->path);
    FREE_AND_COPY(tmp->name, portal_name);
    EXIT_X(tmp) = x;
    EXIT_Y(tmp) = y;
    object_insert_in_ob(tmp, op);

    /* Create a portal in the destination map
     * dummy contain the portal and
     * force the track to kill it later
     * the 'force' variable still contains the 'reminder' of
     * where this portal goes to.
     */
    snprintf(portal_name, 1024, "%s's portal to %s", op->name, op->map->path);
    dummy = create_archetype(spell->slaying);  /*The portal*/
    if (dummy == NULL) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                      "Oops, program error!");
        LOG(llevError, "object_new failed (perm_magic_portal) in cast_create_town_portal for %s!\n", op->name);
        return 0;
    }
    EXIT_PATH(dummy) = add_string(op->map->path);
    EXIT_X(dummy) = op->x;
    EXIT_Y(dummy) = op->y;
    FREE_AND_COPY(dummy->name, portal_name);
    FREE_AND_COPY(dummy->name_pl, portal_name);
    object_set_msg(dummy, portal_message);
    dummy->race = add_string(op->name);   /*Save the owner of the portal*/

    /* animation here too */
    tmp = create_archetype("town_portal_open");
    FREE_AND_COPY(tmp->name, portal_name);
    FREE_AND_COPY(tmp->name_pl, portal_name);
    object_insert_in_ob(dummy, tmp);
    /* and put it on the floor, when it ends the portal will be on the ground */
    object_insert_in_map_at(tmp, exitmap, op, 0, EXIT_X(force), EXIT_Y(force));
    x = tmp->x;
    y = tmp->y;

    /* Now we create another town portal marker that
     * points back to the one we just made
     */
    tmp = create_archetype(spell->race);
    if (tmp == NULL) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                      "Oops, program error!");
        LOG(llevError, "object_new failed (force) in cast_create_town_portal for %s!\n", op->name);
        return 0;
    }
    tmp->race = add_string(force->name);
    FREE_AND_COPY(tmp->name, portal_name);
    EXIT_X(tmp) = x;
    EXIT_Y(tmp) = y;
    object_insert_in_ob(tmp, op);

    /* Describe the player what happened
     */
    draw_ext_info(NDI_UNIQUE|NDI_NAVY, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_SUCCESS,
                  "You see air moving and showing you the way home.");
    object_remove(force); /* Delete the force inside the player*/
    object_free2(force, 0);
    return 1;
}


/**
 * This creates magic walls.  Really, it can create most any object,
 * within some reason.
 *
 * @param op
 * who is casting.
 * @param caster
 * what is casting.
 * @param dir
 * casting direction.
 * @param spell_ob
 * actual spell object.
 * @retval 0
 * spell failed.
 * @retval 1
 * spell was successful.
 */
int magic_wall(object *op, object *caster, int dir, object *spell_ob) {
    object *tmp, *tmp2;
    int i, posblocked, negblocked, maxrange;
    sint16 x, y;
    mapstruct *m;
    const char *name;
    archetype *at;

    if (!dir) {
        dir = op->facing;
        x = op->x;
        y = op->y;
    } else {
        x = op->x+freearr_x[dir];
        y = op->y+freearr_y[dir];
    }
    m = op->map;

    if ((spell_ob->move_block || x != op->x || y != op->y)
    && (get_map_flags(m, &m, x, y, &x, &y)&(P_OUT_OF_MAP|P_IS_ALIVE)
    || ((spell_ob->move_block&GET_MAP_MOVE_BLOCK(m, x, y)) == spell_ob->move_block))) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                      "Something is in the way.");
        return 0;
    }
    if (spell_ob->other_arch) {
        tmp = arch_to_object(spell_ob->other_arch);
    } else if (spell_ob->race) {
        char buf1[MAX_BUF];

        snprintf(buf1, sizeof(buf1), spell_ob->race, dir);
        at = find_archetype(buf1);
        if (!at) {
            LOG(llevError, "summon_wall: Unable to find archetype %s\n", buf1);
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                          "This spell is broken.");
            return 0;
        }
        tmp = arch_to_object(at);
    } else {
        LOG(llevError, "magic_wall: spell %s lacks other_arch\n", spell_ob->name);
        return 0;
    }

    if (tmp->type == SPELL_EFFECT) {
        tmp->attacktype = spell_ob->attacktype;
        tmp->duration = spell_ob->duration+SP_level_duration_adjust(caster, spell_ob);
        tmp->stats.dam = spell_ob->stats.dam+SP_level_dam_adjust(caster, spell_ob);
        tmp->range = 0;
    } else if (QUERY_FLAG(tmp, FLAG_ALIVE)) {
        tmp->stats.hp = spell_ob->duration+SP_level_duration_adjust(caster, spell_ob);
        tmp->stats.maxhp = tmp->stats.hp;
        object_set_owner(tmp, op);
        set_spell_skill(op, caster, spell_ob, tmp);
    }
    if (QUERY_FLAG(spell_ob, FLAG_IS_USED_UP) || QUERY_FLAG(tmp, FLAG_IS_USED_UP)) {
        tmp->stats.food = spell_ob->duration+SP_level_duration_adjust(caster, spell_ob);
        SET_FLAG(tmp, FLAG_IS_USED_UP);
    }
    if (QUERY_FLAG(spell_ob, FLAG_TEAR_DOWN)) {
        tmp->stats.hp = spell_ob->stats.dam+SP_level_dam_adjust(caster, spell_ob);
        tmp->stats.maxhp = tmp->stats.hp;
        SET_FLAG(tmp, FLAG_TEAR_DOWN);
        SET_FLAG(tmp, FLAG_ALIVE);
    }

    /* This can't really hurt - if the object doesn't kill anything,
     * these fields just won't be used.
     */
    object_set_owner(tmp, op);
    set_spell_skill(op, caster, spell_ob, tmp);
    tmp->level = caster_level(caster, spell_ob)/2;

    name = tmp->name;
    if ((tmp = object_insert_in_map_at(tmp, m, op, 0, x, y)) == NULL) {
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                             "Something destroys your %s",
                             name);
        return 0;
    }
    /* If this is a spellcasting wall, need to insert the spell object */
    if (tmp->other_arch && tmp->other_arch->clone.type == SPELL)
        object_insert_in_ob(arch_to_object(tmp->other_arch), tmp);

    /*  This code causes the wall to extend some distance in
     * each direction, or until an obstruction is encountered.
     * posblocked and negblocked help determine how far the
     * created wall can extend, it won't go extend through
     * blocked spaces.
     */
    maxrange = spell_ob->range+SP_level_range_adjust(caster, spell_ob);
    posblocked = 0;
    negblocked = 0;

    for (i = 1; i <= maxrange; i++) {
        int dir2;

        dir2 = (dir < 4) ? (dir+2) : dir-2;

        x = tmp->x+i*freearr_x[dir2];
        y = tmp->y+i*freearr_y[dir2];
        m = tmp->map;

        if (!(get_map_flags(m, &m, x, y, &x, &y)&(P_OUT_OF_MAP|P_IS_ALIVE))
        && ((spell_ob->move_block&GET_MAP_MOVE_BLOCK(m, x, y)) != spell_ob->move_block)
        && !posblocked) {
            tmp2 = object_new();
            object_copy(tmp, tmp2);
            object_insert_in_map_at(tmp2, m, op, 0, x, y);
            /* If this is a spellcasting wall, need to insert the spell object */
            if (tmp2->other_arch && tmp2->other_arch->clone.type == SPELL)
                object_insert_in_ob(arch_to_object(tmp2->other_arch), tmp2);
        } else
            posblocked = 1;

        x = tmp->x-i*freearr_x[dir2];
        y = tmp->y-i*freearr_y[dir2];
        m = tmp->map;

        if (!(get_map_flags(m, &m, x, y, &x, &y)&(P_OUT_OF_MAP|P_IS_ALIVE))
        && ((spell_ob->move_block&GET_MAP_MOVE_BLOCK(m, x, y)) != spell_ob->move_block)
        && !negblocked) {
            tmp2 = object_new();
            object_copy(tmp, tmp2);
            object_insert_in_map_at(tmp2, m, op, 0, x, y);
            if (tmp2->other_arch && tmp2->other_arch->clone.type == SPELL)
                object_insert_in_ob(arch_to_object(tmp2->other_arch), tmp2);
        } else
            negblocked = 1;
    }

    if (QUERY_FLAG(tmp, FLAG_BLOCKSVIEW))
        update_all_los(op->map, op->x, op->y);

    return 1;
}

/**
 * Teleport through some doors and space.
 *
 * @param op
 * who is casting.
 * @param caster
 * what is casting.
 * @param spob
 * actual spell object.
 * @param dir
 * casting direction.
 * @retval 0
 * spell failure.
 * @retval 1
 * spell was successful.
 */
int dimension_door(object *op, object *caster, object *spob, int dir) {
    uint32 dist, maxdist;
    int  mflags;
    mapstruct *m;
    sint16  sx, sy;

    if (op->type != PLAYER)
        return 0;

    if (!dir) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                      "In what direction?");
        return 0;
    }

    /* Given the new outdoor maps, can't let players dimension door for
     * ever, so put limits in.
     */
    maxdist = spob->range+SP_level_range_adjust(caster, spob);

    if (op->contr->count) {
        if (op->contr->count > maxdist) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                          "You can't dimension door that far!");
            return 0;
        }

        for (dist = 0; dist < op->contr->count; dist++) {
            mflags = get_map_flags(op->map, &m,
                                   op->x+freearr_x[dir]*(dist+1),
                                   op->y+freearr_y[dir]*(dist+1),
                                   &sx, &sy);

            if (mflags&(P_NO_MAGIC|P_OUT_OF_MAP))
                break;

            if ((mflags&P_BLOCKSVIEW)
            && OB_TYPE_MOVE_BLOCK(op, GET_MAP_MOVE_BLOCK(m, sx, sy)))
                break;
        }

        if (dist < op->contr->count) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                          "Something blocks the magic of the spell.");
            op->contr->count = 0;
            return 0;
        }
        op->contr->count = 0;

        /* Remove code that puts player on random space on maps.  IMO,
         * a lot of maps probably have areas the player should not get to,
         * but may not be marked as NO_MAGIC (as they may be bounded
         * by such squares).  Also, there are probably treasure rooms and
         * lots of other maps that protect areas with no magic, but the
         * areas themselves don't contain no magic spaces.
         */
        /* This call here is really just to normalize the coordinates */
        mflags = get_map_flags(op->map, &m, op->x+freearr_x[dir]*dist, op->y+freearr_y[dir]*dist,
                               &sx, &sy);
        if (mflags&P_IS_ALIVE || OB_TYPE_MOVE_BLOCK(op, GET_MAP_MOVE_BLOCK(m, sx, sy))) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                          "You cast your spell, but nothing happens.");
            return 1; /* Maybe the penalty should be more severe... */
        }
    } else {
        /* Player didn't specify a distance, so lets see how far
         * we can move the player.  Don't know why this stopped on
         * spaces that blocked the players view.
         */

        for (dist = 0; dist < maxdist; dist++) {
            mflags = get_map_flags(op->map, &m,
                                   op->x+freearr_x[dir]*(dist+1),
                                   op->y+freearr_y[dir]*(dist+1),
                                   &sx, &sy);

            if (mflags&(P_NO_MAGIC|P_OUT_OF_MAP))
                break;

            if ((mflags&P_BLOCKSVIEW)
            && OB_TYPE_MOVE_BLOCK(op, GET_MAP_MOVE_BLOCK(m, sx, sy)))
                break;
        }

        /* If the destination is blocked, keep backing up until we
         * find a place for the player.
         */
        for (; dist > 0; dist--) {
            if (get_map_flags(op->map, &m, op->x+freearr_x[dir]*dist, op->y+freearr_y[dir]*dist, &sx, &sy)&(P_OUT_OF_MAP|P_IS_ALIVE))
                continue;

            if (!OB_TYPE_MOVE_BLOCK(op, GET_MAP_MOVE_BLOCK(m, sx, sy)))
                break;
        }
        if (!dist) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                          "Your spell failed!");
            return 0;
        }
    }

    if (op->contr->transport && op->contr->transport->type == TRANSPORT) {
        ob_apply(op->contr->transport, op, 0);
        if (op->contr->transport) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                          "Your spell failed!");
            return 0;
        }
    }

    /* Actually move the player now */
    object_remove(op);
    if ((op = object_insert_in_map_at(op, op->map, op, 0, op->x+freearr_x[dir]*dist, op->y+freearr_y[dir]*dist)) == NULL)
        return 1;

    if (op->type == PLAYER)
        map_newmap_cmd(&op->contr->socket);
    op->speed_left = -FABS(op->speed)*5; /* Freeze them for a short while */
    return 1;
}


/**
 * Heals something.
 * @param op
 * who is casting.
 * @param caster
 * what is casting.
 * @param spell
 * actual spell object.
 * @param dir
 * casting direction.
 * @todo check spurious cure_disease call (shouldn't the spell's level be sent?) and return check value (always 1).
 */
int cast_heal(object *op, object *caster, object *spell, int dir) {
    object *target;
    archetype *at;
    object *poison;
    int heal = 0, success = 0;

    target = find_target_for_friendly_spell(op, dir);

    if (target == NULL)
        return 0;

    /* Figure out how many hp this spell might cure.
     * could be zero if this spell heals effects, not damage.
     */
    heal = spell->stats.dam;
    if (spell->stats.hp)
        heal += random_roll(spell->stats.hp, 6, op, PREFER_HIGH)+spell->stats.hp;

    if (heal) {
        if (target->stats.hp >= target->stats.maxhp) {
            draw_ext_info(NDI_UNIQUE, 0, target, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                          "You are already fully healed.");
        } else {
            /* See how many points we actually heal.  Instead of messages
             * based on type of spell, we instead do messages based
             * on amount of damage healed.
             */
            if (heal > (target->stats.maxhp-target->stats.hp))
                heal = target->stats.maxhp-target->stats.hp;
            target->stats.hp += heal;

            if (target->stats.hp >= target->stats.maxhp) {
                draw_ext_info(NDI_UNIQUE, 0, target, MSG_TYPE_SPELL, MSG_TYPE_SPELL_HEAL,
                              "You feel just fine!");
            } else if (heal > 50) {
                draw_ext_info(NDI_UNIQUE, 0, target, MSG_TYPE_SPELL, MSG_TYPE_SPELL_HEAL,
                              "Your wounds close!");
            } else if (heal > 25) {
                draw_ext_info(NDI_UNIQUE, 0, target, MSG_TYPE_SPELL, MSG_TYPE_SPELL_HEAL,
                              "Your wounds mostly close.");
            } else if (heal > 10) {
                draw_ext_info(NDI_UNIQUE, 0, target, MSG_TYPE_SPELL, MSG_TYPE_SPELL_HEAL,
                              "Your wounds start to fade.");
            } else {
                draw_ext_info(NDI_UNIQUE, 0, target, MSG_TYPE_SPELL, MSG_TYPE_SPELL_HEAL,
                              "Your wounds start to close.");
            }
            success = 1;
        }
    }
    if (spell->attacktype&AT_DISEASE)
        if (cure_disease(target, op, caster && caster->type != PLAYER ? caster->skill : spell->skill))
            success = 1;

    if (spell->attacktype&AT_POISON) {
        at = find_archetype("poisoning");
        poison = arch_present_in_ob(at, target);
        if (poison) {
            success = 1;
            draw_ext_info(NDI_UNIQUE, 0, target, MSG_TYPE_SPELL, MSG_TYPE_SPELL_HEAL,
                          "Your body feels cleansed");
            poison->stats.food = 1;
        }
    }
    if (spell->attacktype&AT_CONFUSION) {
        poison = object_present_in_ob_by_name(FORCE, "confusion", target);
        if (poison) {
            success = 1;
            draw_ext_info(NDI_UNIQUE, 0, target, MSG_TYPE_SPELL, MSG_TYPE_SPELL_HEAL,
                          "Your mind feels clearer");
            poison->duration = 1;
        }
    }
    if (spell->attacktype&AT_BLIND) {
        at = find_archetype("blindness");
        poison = arch_present_in_ob(at, target);
        if (poison) {
            success = 1;
            draw_ext_info(NDI_UNIQUE, 0, target, MSG_TYPE_SPELL, MSG_TYPE_SPELL_HEAL,
                          "Your vision begins to return.");
            poison->stats.food = 1;
        }
    }
    if (spell->last_sp && target->stats.sp < target->stats.maxsp) {
        target->stats.sp += spell->last_sp;
        if (target->stats.sp > target->stats.maxsp)
            target->stats.sp = target->stats.maxsp;
        success = 1;
        draw_ext_info(NDI_UNIQUE, 0, target, MSG_TYPE_SPELL, MSG_TYPE_SPELL_HEAL,
                      "Magical energies surge through your body!");
    }
    if (spell->last_grace && target->stats.grace < target->stats.maxgrace) {
        target->stats.grace += spell->last_grace;
        if (target->stats.grace > target->stats.maxgrace)
            target->stats.grace = target->stats.maxgrace;
        success = 1;
        draw_ext_info(NDI_UNIQUE, 0, target, MSG_TYPE_SPELL, MSG_TYPE_SPELL_HEAL,
                      "You feel redeemed with you god!");
    }
    if (spell->stats.food && target->stats.food < 999) {
        target->stats.food += spell->stats.food;
        if (target->stats.food > 999)
            target->stats.food = 999;
        success = 1;
        /* We could do something a bit better like the messages for healing above */
        draw_ext_info(NDI_UNIQUE, 0, target, MSG_TYPE_SPELL, MSG_TYPE_SPELL_HEAL,
                      "You feel your belly fill with food");
    }
    return success;
}


/**
 * This is used for the spells that gain stats.  There are no spells
 * right now that icnrease wis/int/pow on a temp basis, so no
 * good comments for those.
 */
static const char *const no_gain_msgs[NUM_STATS] = {
    "You grow no stronger.",
    "You grow no more agile.",
    "You don't feel any healthier.",
    "no wis",
    "You are no easier to look at.",
    "no int",
    "no pow"
};

/**
 * Cast some stat-improving spell.
 *
 * @param op
 * who is casting.
 * @param caster
 * what is casting.
 * @param spell_ob
 * actual spell object.
 * @param dir
 * casting direction.
 * @param silent
 * if non zero, don't say when the spell is already is effect.
 * @retval 0
 * spell failed.
 * @retval 1
 * spell was successful.
 * @todo weird check on duration? since you'll never get there since a force would have been found?
 */
int cast_change_ability(object *op, object *caster, object *spell_ob, int dir, int silent) {
    object *tmp;
    object *force = NULL;
    int i;

    /* if dir = 99 op defaults to tmp, eat_special_food() requires this. */
    if (dir != 0) {
        tmp = find_target_for_friendly_spell(op, dir);
    } else {
        tmp = op;
    }

    if (tmp == NULL)
        return 0;

    /* If we've already got a force of this type, don't add a new one. */
    FOR_INV_PREPARE(tmp, tmp2)
        if (tmp2->type == FORCE && tmp2->subtype == FORCE_CHANGE_ABILITY)  {
            if (tmp2->name == spell_ob->name) {
                force = tmp2;    /* the old effect will be "refreshed" */
                break;
            } else if (spell_ob->race && spell_ob->race == tmp2->name) {
                if (!silent)
                    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                                         "You can not cast %s while %s is in effect",
                                         spell_ob->name, tmp2->name_pl);
                return 0;
            }
        }
    FOR_INV_FINISH();
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
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_SUCCESS,
                          "You recast the spell while in effect.");

            if (spell_ob->other_arch != NULL && tmp->map != NULL) {
                object_insert_in_map_at(arch_to_object(spell_ob->other_arch), tmp->map, NULL, INS_ON_TOP, tmp->x, tmp->y);
            }

        } else {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                          "Recasting the spell had no effect.");
        }
        return 1;
    }
    force->duration = spell_ob->duration+SP_level_duration_adjust(caster, spell_ob)*50;
    if (op->type == PLAYER)
        store_spell_expiry(force);
    force->speed = 1.0;
    force->speed_left = -1.0;
    SET_FLAG(force, FLAG_APPLIED);

    /* Now start processing the effects.  First, protections */
    for (i = 0; i < NROFATTACKS; i++) {
        if (spell_ob->resist[i]) {
            force->resist[i] = spell_ob->resist[i]+SP_level_dam_adjust(caster, spell_ob);
            if (force->resist[i] > 100)
                force->resist[i] = 100;
        }
    }
    if (spell_ob->stats.hp)
        force->stats.hp = spell_ob->stats.hp+SP_level_dam_adjust(caster, spell_ob);

    if (tmp->type == PLAYER) {
        /* Stat adjustment spells */
        for (i = 0; i < NUM_STATS; i++) {
            sint8 stat = get_attr_value(&spell_ob->stats, i), k, sm;

            if (stat) {
                sm = 0;
                for (k = 0; k < stat; k++)
                    sm += rndm(1, 3);

                if ((get_attr_value(&tmp->stats, i)+sm) > (15+5*stat)) {
                    sm = (15+5*stat)-get_attr_value(&tmp->stats, i);
                    if (sm < 0)
                        sm = 0;
                }
                set_attr_value(&force->stats, i, sm);
                if (!sm)
                    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                                  no_gain_msgs[i]);
            }
        }
    }

    force->move_type = spell_ob->move_type;

    if (QUERY_FLAG(spell_ob, FLAG_SEE_IN_DARK))
        SET_FLAG(force, FLAG_SEE_IN_DARK);

    if (QUERY_FLAG(spell_ob, FLAG_XRAYS))
        SET_FLAG(force, FLAG_XRAYS);

    /* Haste/bonus speed */
    if (spell_ob->stats.exp) {
        if (op->speed > 0.5)
            force->stats.exp = (float)spell_ob->stats.exp/(op->speed+0.5);
        else
            force->stats.exp = spell_ob->stats.exp;
    }

    force->stats.wc = spell_ob->stats.wc;
    force->stats.ac = spell_ob->stats.ac;
    force->attacktype = spell_ob->attacktype;

    SET_FLAG(tmp, FLAG_NO_FIX_PLAYER); /* we don't want object_insert_in_ob() to call fix_object. */
    object_insert_in_ob(force, tmp);
    CLEAR_FLAG(tmp, FLAG_NO_FIX_PLAYER);
    change_abil(tmp, force); /* Display any relevant messages, and call fix_object to update the player */

    if (spell_ob->other_arch != NULL && tmp->map != NULL) {
        object_insert_in_map_at(arch_to_object(spell_ob->other_arch), tmp->map, NULL, INS_ON_TOP, tmp->x, tmp->y);
    }

    return 1;
}

/**
 * Improve statistics of some living object.
 *
 * @param op
 * who is casting.
 * @param caster
 * what is casting.
 * @param spell_ob
 * actual spell object.
 * @param dir
 * casting direction.
 * @retval 0
 * spell failed.
 * @retval 1
 * spell was successful.
 */
int cast_bless(object *op, object *caster, object *spell_ob, int dir) {
    int i;
    const object *god = find_god(determine_god(op));
    object *force = NULL, *tmp;

    /* if dir = 99 op defaults to tmp, eat_special_food() requires this. */
    if (dir != 0) {
        tmp = find_target_for_friendly_spell(op, dir);
    } else {
        tmp = op;
    }

    /* If we've already got a force of this type, don't add a new one. */
    FOR_INV_PREPARE(tmp, tmp2)
        if (tmp2->type == FORCE && tmp2->subtype == FORCE_CHANGE_ABILITY)  {
            if (tmp2->name == spell_ob->name) {
                force = tmp2;    /* the old effect will be "refreshed" */
                break;
            } else if (spell_ob->race && spell_ob->race == tmp2->name) {
                draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                                     "You can not cast %s while %s is in effect",
                                     spell_ob->name, tmp2->name_pl);
                return 0;
            }
        }
    FOR_INV_FINISH();
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
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_SUCCESS,
                          "You recast the spell while in effect.");
        } else {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                          "Recasting the spell had no effect.");
        }
        return 0;
    }
    force->duration = spell_ob->duration+SP_level_duration_adjust(caster, spell_ob)*50;
    force->speed = 1.0;
    force->speed_left = -1.0;

    if (!god) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                      "Your blessing seems empty.");
    } else {
        /* Only give out good benefits, and put a max on it */
        for (i = 0; i < NROFATTACKS; i++) {
            if (god->resist[i] > 0) {
                force->resist[i] = MIN(god->resist[i], spell_ob->resist[ATNR_GODPOWER]);
            }
        }
        force->path_attuned |= god->path_attuned;
        if (spell_ob->attacktype) {
            force->attacktype |= god->attacktype|AT_PHYSICAL;
            if (god->slaying)
                force->slaying = add_string(god->slaying);
        }
        if (tmp != op) {
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_SUCCESS,
                                 "You bless %s.",
                                 tmp->name);
            draw_ext_info_format(NDI_UNIQUE, 0, tmp, MSG_TYPE_SPELL, MSG_TYPE_SPELL_TARGET,
                                 "%s blessed you.",
                                 op->name);
        } else {
            draw_ext_info_format(NDI_UNIQUE, 0, tmp, MSG_TYPE_SPELL, MSG_TYPE_SPELL_TARGET,
                                 "You are blessed by %s!",
                                 god->name);
        }
    }
    force->stats.wc = spell_ob->stats.wc;
    force->stats.ac = spell_ob->stats.ac;

    store_spell_expiry(force);
    object_insert_in_ob(force, tmp);
    SET_FLAG(force, FLAG_APPLIED);
    change_abil(tmp, force); /* To display any messages, will call fix_object() */

    if (spell_ob->other_arch != NULL && tmp->map != NULL) {
        object_insert_in_map_at(arch_to_object(spell_ob->other_arch), tmp->map, NULL, INS_ON_TOP, tmp->x, tmp->y);
    }
    return 1;
}



/*
 * Alchemy code by Mark Wedel
 *
 * This code adds a new spell, called alchemy.  Alchemy will turn
 * objects to gold nuggets, the value of the gold nuggets being
 * from 5% to 40% of that of the item itself depending on casting level.
 * It uses the value of the object before charisma adjustments, because
 * the nuggets themselves will be will be adjusted by charisma when sold.
 *
 * Large nuggets are worth 25 gp each (base).  You will always get
 * the maximum number of large nuggets you could get.
 * Small nuggets are worth 1 gp each (base).  You will get from 0
 * to the max amount of small nuggets as you could get.
 *
 * For example, if an item is worth 110 gold, you will get
 * 4 large nuggets, and from 0-10 small nuggets.
 *
 * There is also a chance (1:30) that you will get nothing at all
 * for the object.  There is also a maximum weight that will be
 * alchemied.
 */

#define SMALL_NUGGET "smallnugget"
#define LARGE_NUGGET "largenugget"

/**
 * Compute how many nuggets an object is worth, and remove it.
 *
 * @param value_adj
 * how much to adjust the cost of obj.
 * @param obj
 * object to convert.
 * @param[out] small_nuggets
 * how many small nuggets obj gives.
 * @param[out] large_nuggets
 * how many large nuggets obj gives.
 * @param[out] weight
 * the weight of the object.
 */
static void alchemy_object(float value_adj, object *obj, int *small_nuggets, int *large_nuggets, int *weight) {
    uint64 value = query_cost(obj, NULL, BS_TRUE);
    uint64 small_value, large_value; /**< Value of nuggets. */

    /* Multiply the value of the object by value_adj, which should range
     * from 0.05 to 0.40. Set value to 0 instead if unpaid.
     */
    if (QUERY_FLAG(obj, FLAG_UNPAID))
        value = 0;
    else
        value *= value_adj;

    small_value = query_cost(&find_archetype(SMALL_NUGGET)->clone, NULL, BS_TRUE);
    large_value = query_cost(&find_archetype(LARGE_NUGGET)->clone, NULL, BS_TRUE);

    /* Give half of what value_adj says when we alchemy money (This should
     * hopefully make it so that it isn't worth it to alchemy money, sell
     * the nuggets, alchemy the gold from that, etc.
     */
    if (value && (obj->type == MONEY || obj->type == GEM))
        value /= 2;

    if ((obj->value > 0) && rndm(0, 29)) {
        int count;

        count = value/large_value;
        *large_nuggets += count;
        value -= (uint64)count*large_value;
        count = value/small_value;
        *small_nuggets += count;
    }

    /* Turn 25 small nuggets into 1 large nugget.  If the value
     * of large nuggets is not evenly divisable by the small nugget
     * value, take off an extra small_nugget (Assuming small_nuggets!=0)
     */
    if (*small_nuggets*small_value >= large_value) {
        (*large_nuggets)++;
        *small_nuggets -= large_value/small_value;
        if (*small_nuggets && large_value%small_value)
            (*small_nuggets)--;
    }

    if (weight != NULL) {
        *weight += obj->weight;
    }

    object_remove(obj);
    object_free2(obj, FREE_OBJ_NO_DESTROY_CALLBACK);
}

/**
 * Place gold nuggets on the map.
 * @param op
 * player who is casting the spell. Just used so nuggets are inserted below her.
 * @param m
 * map to insert to.
 * @param small_nuggets
 * @param large_nuggets
 * how many nuggets to place.
 * @param x
 * @param y
 * where to place the nuggets.
 */
static void place_alchemy_objects(object *op, mapstruct *m, int small_nuggets, int large_nuggets, int x, int y) {
    object *tmp;
    int flag = 0;

    /* Put any nuggets below the player, but we can only pass this
     * flag if we are on the same space as the player
     */
    if (x == op->x && y == op->y && op->map == m)
        flag = INS_BELOW_ORIGINATOR;

    if (small_nuggets) {
        tmp = create_archetype(SMALL_NUGGET);
        tmp-> nrof = small_nuggets;
        object_insert_in_map_at(tmp, m, op, flag, x, y);
    }
    if (large_nuggets) {
        tmp = create_archetype(LARGE_NUGGET);
        tmp-> nrof = large_nuggets;
        object_insert_in_map_at(tmp, m, op, flag, x, y);
    }
}

/**
 * Change items to gold nuggets. Only works for players.
 *
 * @param op
 * who is casting.
 * @param caster
 * what is casting.
 * @param spell_ob
 * actual spell object.
 * @retval 0
 * op isn't a player.
 * @retval 1
 * op is a player.
 */
int alchemy(object *op, object *caster, object *spell_ob) {
    int x, y, weight = 0, weight_max, large_nuggets, small_nuggets, mflags;
    sint16 nx, ny;
    float value_adj;
    mapstruct *mp;

    if (op->type != PLAYER)
        return 0;

    /* Put a maximum weight of items that can be alchemied.  Limits the power
     * some, and also prevents people from alcheming every table/chair/clock
     * in sight
     */
    weight_max = spell_ob->duration+SP_level_duration_adjust(caster, spell_ob);
    weight_max *= 1000;

    /* Set value_adj to be a multiplier for how much of the original value
     * will be in the nuggets. Starts at 0.05, increasing by 0.01 per casting
     * level, maxing out at 0.40.
     */
    value_adj = (SP_level_dam_adjust(caster, spell_ob)/100.00)+0.05;

    if (value_adj > 0.40)
        value_adj = 0.40;

    for (y = op->y-1; y <= op->y+1; y++) {
        for (x = op->x-1; x <= op->x+1; x++) {
            nx = x;
            ny = y;

            mp = op->map;

            mflags = get_map_flags(mp, &mp, nx, ny, &nx, &ny);

            if (mflags&(P_OUT_OF_MAP|P_NO_MAGIC))
                continue;

            /* Treat alchemy a little differently - most spell effects
             * use fly as the movement type - for alchemy, consider it
             * ground level effect.
             */
            if (GET_MAP_MOVE_BLOCK(mp, nx, ny)&MOVE_WALK)
                continue;

            small_nuggets = 0;
            large_nuggets = 0;

            FOR_MAP_PREPARE(mp, nx, ny, tmp) {
                if (tmp->weight > 0 && !QUERY_FLAG(tmp, FLAG_NO_PICK)
                && !QUERY_FLAG(tmp, FLAG_ALIVE)
                && !QUERY_FLAG(tmp, FLAG_IS_CAULDRON)) {
                    if (tmp->inv) {
                        FOR_INV_PREPARE(tmp, tmp1)
                            if (tmp1->weight > 0 && !QUERY_FLAG(tmp1, FLAG_NO_PICK)
                            && !QUERY_FLAG(tmp1, FLAG_ALIVE)
                            && !QUERY_FLAG(tmp1, FLAG_IS_CAULDRON))
                                alchemy_object(value_adj, tmp1, &small_nuggets, &large_nuggets, &weight);
                        FOR_INV_FINISH();
                    }
                    alchemy_object(value_adj, tmp, &small_nuggets, &large_nuggets, &weight);

                    if (weight > weight_max) {
                        place_alchemy_objects(op, mp, small_nuggets, large_nuggets, nx, ny);
                        return 1;
                    }
                } /* is alchemable object */
            } FOR_MAP_FINISH(); /* process all objects on this space */

            /* Insert all the nuggets at one time.  This probably saves time, but
             * it also prevents us from alcheming nuggets that were just created
             * with this spell.
             */
            place_alchemy_objects(op, mp, small_nuggets, large_nuggets, nx, ny);
        }
    }

    /* reset this so that if player standing on a big pile of stuff,
     * it is redrawn properly.
     */
    op->contr->socket.look_position = 0;
    return 1;
}


/**
 * This function removes the cursed/damned status on equipped
 * items.
 *
 * @param op
 * who is casting.
 * @param caster
 * what is casting.
 * @param spell
 * actual spell object.
 * @return
 * how many items were affected.
 * @todo why is the value set to 0?
 */
int remove_curse(object *op, object *caster, object *spell) {
    int success = 0, was_one = 0;

    FOR_INV_PREPARE(op, tmp)
        if (QUERY_FLAG(tmp, FLAG_APPLIED)
        && ((QUERY_FLAG(tmp, FLAG_CURSED) && QUERY_FLAG(spell, FLAG_CURSED))
            || (QUERY_FLAG(tmp, FLAG_DAMNED) && QUERY_FLAG(spell, FLAG_DAMNED)))) {
            was_one++;
            if (tmp->level <= caster_level(caster, spell)) {
                success++;
                if (QUERY_FLAG(spell, FLAG_DAMNED))
                    CLEAR_FLAG(tmp, FLAG_DAMNED);

                CLEAR_FLAG(tmp, FLAG_CURSED);
                CLEAR_FLAG(tmp, FLAG_KNOWN_CURSED);
                tmp->value = 0; /* Still can't sell it */
                if (op->type == PLAYER)
                    esrv_update_item(UPD_FLAGS, op, tmp);
            }
        }
    FOR_INV_FINISH();

    if (op->type == PLAYER) {
        if (success) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_SUCCESS,
                          "You feel like some of your items are looser now.");
        } else {
            if (was_one)
                draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                              "You failed to remove the curse.");
            else
                draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                              "You are not using any cursed items.");
        }
    }
    return success;
}

/**
 * This alters player's marked item's cursed or blessed status, based on the spell_ob's fields.
 *
 * @param op
 * player casting the spell.
 * @param caster
 * what object was used to cast the spell.
 * @param spell_ob
 * spell itself.
 * @return
 * 1 if item was changed, 0 else.
 */
int cast_item_curse_or_curse(object *op, object *caster, object *spell_ob) {
    object *marked = find_marked_object(op);
    char name[HUGE_BUF];

    if (!marked) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                      "You need to mark an item first!");
        return 0;
    }

    if ((QUERY_FLAG(marked, FLAG_CURSED) && QUERY_FLAG(spell_ob, FLAG_CURSED))
        || (QUERY_FLAG(marked, FLAG_BLESSED) && QUERY_FLAG(spell_ob, FLAG_BLESSED))) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                      "The spell has no effect");
        return 0;
    }

    query_short_name(marked, name, HUGE_BUF);
    if (QUERY_FLAG(spell_ob, FLAG_CURSED)) {
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_SUCCESS,
                             "Your %s emits a dark light for a few seconds.", name);
        SET_FLAG(marked, FLAG_CURSED);
        CLEAR_FLAG(marked, FLAG_KNOWN_CURSED);
        CLEAR_FLAG(marked, FLAG_IDENTIFIED);
        esrv_update_item(UPD_FLAGS, op, marked);
        return 1;
    }

    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_SUCCESS,
                         "Your %s glows blue for a few seconds.", name);
    SET_FLAG(marked, FLAG_BLESSED);
    SET_FLAG(marked, FLAG_KNOWN_BLESSED);
    SET_FLAG(marked, FLAG_STARTEQUIP);
    esrv_update_item(UPD_FLAGS, op, marked);
    return 1;
}

/**
 * Identifies objects in the players inventory/on the ground.
 *
 * @param op
 * who is casting.
 * @param caster
 * what is casting.
 * @param spell
 * actual spell object.
 * @retval 0
 * nothing was identified.
 * @retval 1
 * at least one object was identified.
 */
int cast_identify(object *op, object *caster, object *spell) {
    int success = 0, num_ident;
    char desc[MAX_BUF];

    num_ident = spell->stats.dam+SP_level_dam_adjust(caster, spell);

    if (num_ident < 1)
        num_ident = 1;

    FOR_INV_PREPARE(op, tmp)
        if (!QUERY_FLAG(tmp, FLAG_IDENTIFIED) && !tmp->invisible &&  need_identify(tmp)) {
            tmp = identify(tmp);
            if (op->type == PLAYER) {
                draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_ITEM, MSG_TYPE_ITEM_INFO,
                                     "You have %s.",
                                     ob_describe(tmp, op, desc, sizeof(desc)));
                if (tmp->msg) {
                    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_ITEM, MSG_TYPE_ITEM_INFO,
                                         "The item has a story:\n%s",
                                         tmp->msg);
                }
            }
            num_ident--;
            success = 1;
            if (!num_ident)
                break;
        }
    FOR_INV_FINISH();
    /* If all the power of the spell has been used up, don't go and identify
     * stuff on the floor.  Only identify stuff on the floor if the spell
     * was not fully used.
     */
    if (num_ident) {
        FOR_MAP_PREPARE(op->map, op->x, op->y, tmp)
            if (!QUERY_FLAG(tmp, FLAG_IDENTIFIED)
            && !tmp->invisible
            && need_identify(tmp)) {
                tmp = identify(tmp);
                if (op->type == PLAYER) {
                    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_ITEM, MSG_TYPE_ITEM_INFO,
                                         "On the ground is %s.",
                                         ob_describe(tmp, op, desc, sizeof(desc)));
                    if (tmp->msg) {
                        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_ITEM, MSG_TYPE_ITEM_INFO,
                                             "The item has a story:\n%s",
                                             tmp->msg);
                    }
                    esrv_update_item(UPD_FLAGS|UPD_NAME, op, tmp);
                }
                num_ident--;
                success = 1;
                if (!num_ident)
                    break;
            }
        FOR_MAP_FINISH();
    }
    if (!success)
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                      "You can't reach anything unidentified.");
    else {
        spell_effect(spell, op->x, op->y, op->map, op);
    }
    return success;
}

/**
 * Detect magic or invisible items.
 *
 * @param op
 * who is casting.
 * @param caster
 * what is casting.
 * @param spell
 * actual spell object.
 * @return
 * 1.
 */
int cast_detection(object *op, object *caster, object *spell) {
    object *tmp, *last, *detect;
    const object *god;
    int done_one, range, mflags, floor, level;
    sint16 x, y, nx, ny;
    mapstruct *m;

    /* We precompute some values here so that we don't have to keep
     * doing it over and over again.
     */
    god = find_god(determine_god(op));
    level = caster_level(caster, spell);
    range = spell->range+SP_level_range_adjust(caster, spell);

    for (x = op->x-range; x <= op->x+range; x++)
        for (y = op->y-range; y <= op->y+range; y++) {
            m = op->map;
            mflags = get_map_flags(m, &m, x, y, &nx, &ny);
            if (mflags&P_OUT_OF_MAP)
                continue;

            /* For most of the detections, we only detect objects above the
             * floor.  But this is not true for show invisible.
             * Basically, we just go and find the top object and work
             * down - that is easier than working up.
             */

            last = NULL;
            FOR_MAP_PREPARE(m, nx, ny, tmp)
                last = tmp;
            FOR_MAP_FINISH();
            /* Shouldn't happen, but if there are no objects on a space, this
             * would happen.
             */
            if (!last)
                continue;

            done_one = 0;
            floor = 0;
            detect = NULL;
            tmp = last;
            FOR_OB_AND_BELOW_PREPARE(tmp) {
                /* show invisible */
                if (QUERY_FLAG(spell, FLAG_MAKE_INVIS)
                    /* Might there be other objects that we can make visibile? */
                && (tmp->invisible && (QUERY_FLAG(tmp, FLAG_MONSTER) ||
                                        (tmp->type == PLAYER && !QUERY_FLAG(tmp, FLAG_WIZ)) ||
                                        tmp->type == CF_HANDLE ||
                                        tmp->type == TRAPDOOR || tmp->type == EXIT || tmp->type == HOLE ||
                                        tmp->type == BUTTON || tmp->type == TELEPORTER ||
                                        tmp->type == GATE || tmp->type == LOCKED_DOOR ||
                                        tmp->type == WEAPON || tmp->type == ALTAR || tmp->type == SIGN ||
                                        tmp->type == TRIGGER_PEDESTAL || tmp->type == SPECIAL_KEY ||
                                        tmp->type == TREASURE || tmp->type == BOOK ||
                                        tmp->type == HOLY_ALTAR))) {
                    if (random_roll(0, level-1, op, PREFER_HIGH) > tmp->level) {
                        tmp->invisible = 0;
                        done_one = 1;
                    }
                }
                if (QUERY_FLAG(tmp, FLAG_IS_FLOOR))
                    floor = 1;

                /* All detections below this point don't descend beneath the floor,
                 * so just continue on.  We could be clever and look at the type of
                 * detection to completely break out if we don't care about objects beneath
                 * the floor, but once we get to the floor, not likely a very big issue anyways.
                 */
                if (floor)
                    continue;

                /* I had thought about making detect magic and detect curse
                 * show the flash the magic item like it does for detect monster.
                 * however, if the object is within sight, this would then make it
                 * difficult to see what object is magical/cursed, so the
                 * effect wouldn't be as apparent.
                 */

                /* detect magic */
                if (QUERY_FLAG(spell, FLAG_KNOWN_MAGICAL)
                && !QUERY_FLAG(tmp, FLAG_KNOWN_MAGICAL)
                && !QUERY_FLAG(tmp, FLAG_IDENTIFIED)
                && is_magical(tmp)) {
                    SET_FLAG(tmp, FLAG_KNOWN_MAGICAL);
                    /* make runes more visible */
                    if (tmp->type == RUNE && tmp->attacktype&AT_MAGIC)
                        tmp->stats.Cha /= 4;
                    done_one = 1;
                }
                /* detect monster */
                if (QUERY_FLAG(spell, FLAG_MONSTER)
                && (QUERY_FLAG(tmp, FLAG_MONSTER) || (tmp->type == PLAYER && !QUERY_FLAG(tmp, FLAG_WIZ)))) {
                    done_one = 2;
                    if (!detect)
                        detect = tmp;
                }
                /* Basically, if race is set in the spell, then the creatures race must
                 * match that.  if the spell race is set to GOD, then the gods opposing
                 * race must match.
                 */
                if (spell->race
                && QUERY_FLAG(tmp, FLAG_MONSTER)
                && tmp->race
                && ((!strcmp(spell->race, "GOD") && god && god->slaying && strstr(god->slaying, tmp->race)) || (strstr(spell->race, tmp->race)))) {
                    done_one = 2;
                    if (!detect)
                        detect = tmp;
                }
                if (QUERY_FLAG(spell, FLAG_KNOWN_CURSED)
                && !QUERY_FLAG(tmp, FLAG_KNOWN_CURSED)
                && (QUERY_FLAG(tmp, FLAG_CURSED) || QUERY_FLAG(tmp, FLAG_DAMNED))) {
                    SET_FLAG(tmp, FLAG_KNOWN_CURSED);
                    done_one = 1;
                }
            } FOR_OB_AND_BELOW_FINISH(); /* for stack of objects on this space */

            /* Code here puts an effect of the spell on the space, so you can see
             * where the magic is.
             */
            if (done_one) {
                object *detect_ob;
                int dx = nx, dy = ny;

                /* if this is set, we want to copy the face */
                if (done_one == 2 && detect) {
                    /*
                     * We can't simply copy the face to a single item, because
                     * multipart objects need to have multipart glows.
                     * So copy the initial item, erase some properties, and use that.
                     */

                    object *part;
                    int flag;

                    dx = HEAD(detect)->x;
                    dy = HEAD(detect)->y;

                    detect_ob = object_create_arch(HEAD(detect)->arch);
                    for (part = detect_ob; part != NULL; part = part->more) {
                        if (part->arch->reference_count > 0)
                            part->arch->reference_count++;
                        part->last_anim = 0;
                        part->type = spell->other_arch->clone.type;
                        for (flag = 0; flag < 4; flag++) {
                            part->flags[flag] = spell->other_arch->clone.flags[flag];
                        }
                        part->stats.food = spell->other_arch->clone.stats.food;
                        part->last_anim = 0;
                        part->speed = spell->other_arch->clone.speed;
                        part->speed_left = spell->other_arch->clone.speed_left;
                        part->move_allow = spell->other_arch->clone.move_allow;
                        part->move_block = spell->other_arch->clone.move_block;
                        part->move_type = spell->other_arch->clone.move_type;
                        part->glow_radius = spell->other_arch->clone.glow_radius;
                        part->invisible = spell->other_arch->clone.invisible;
                        part->weight = spell->other_arch->clone.weight;
                        part->map_layer = spell->other_arch->clone.map_layer;
                        FREE_AND_COPY(part->name, spell->other_arch->clone.name);

                        /* by default, the detect_ob is already animated */
                        if (!QUERY_FLAG(detect, FLAG_ANIMATE))
                            CLEAR_FLAG(part, FLAG_ANIMATE);
                    }
                    object_update_speed(detect_ob);
                } else
                    detect_ob = arch_to_object(spell->other_arch);

                object_insert_in_map_at(detect_ob, m, op, 0, dx, dy);
            }
        } /* for processing the surrounding spaces */


    /* Now process objects in the players inventory if detect curse or magic */
    if (QUERY_FLAG(spell, FLAG_KNOWN_CURSED) || QUERY_FLAG(spell, FLAG_KNOWN_MAGICAL)) {
        done_one = 0;
        FOR_INV_PREPARE(op, tmp) {
            if (!tmp->invisible && !QUERY_FLAG(tmp, FLAG_IDENTIFIED)) {
                if (QUERY_FLAG(spell, FLAG_KNOWN_MAGICAL)
                && is_magical(tmp)
                && !QUERY_FLAG(tmp, FLAG_KNOWN_MAGICAL)) {
                    SET_FLAG(tmp, FLAG_KNOWN_MAGICAL);
                    if (op->type == PLAYER)
                        esrv_send_item(op, tmp);
                }
                if (QUERY_FLAG(spell, FLAG_KNOWN_CURSED)
                && !QUERY_FLAG(tmp, FLAG_KNOWN_CURSED)
                && (QUERY_FLAG(tmp, FLAG_CURSED) || QUERY_FLAG(tmp, FLAG_DAMNED))) {
                    SET_FLAG(tmp, FLAG_KNOWN_CURSED);
                    if (op->type == PLAYER)
                        esrv_send_item(op, tmp);
                }
            } /* if item is not identified */
        } FOR_INV_FINISH(); /* for the players inventory */
    } /* if detect magic/curse and object is a player */
    return 1;
}

/**
 * Checks if victim has overcharged mana, and if so does some fireball.
 * @param victim
 * who may have overcharged.
 * @param caster_level
 * caster's (skill) level whose spell did cause the overcharge.
 */
static void charge_mana_effect(object *victim, int caster_level) {
    /* Prevent explosions for objects without mana. Without this check, doors
     * will explode, too.
     */
    if (victim->stats.maxsp <= 0)
        return;

    draw_ext_info(NDI_UNIQUE, 0, victim, MSG_TYPE_SPELL, MSG_TYPE_SPELL_TARGET,
                  "You feel energy course through you.");

    if (victim->stats.sp >= victim->stats.maxsp*2) {
        object *tmp;

        draw_ext_info(NDI_UNIQUE, 0, victim, MSG_TYPE_VICTIM, MSG_TYPE_VICTIM_SPELL,
                      "Your head explodes!");

        /* Explodes a fireball centered at player */
        tmp = create_archetype(EXPLODING_FIREBALL);
        tmp->dam_modifier = random_roll(1, caster_level, victim, PREFER_LOW)/5+1;
        tmp->stats.maxhp = random_roll(1, caster_level, victim, PREFER_LOW)/10+2;
        object_insert_in_map_at(tmp, victim->map, NULL, 0, victim->x, victim->y);
        victim->stats.sp = 2*victim->stats.maxsp;
    } else if (victim->stats.sp >= victim->stats.maxsp*1.88) {
        draw_ext_info(NDI_UNIQUE, NDI_ORANGE, victim, MSG_TYPE_SPELL, MSG_TYPE_SPELL_TARGET,
                      "You feel like your head is going to explode.");
    } else if (victim->stats.sp >= victim->stats.maxsp*1.66) {
        draw_ext_info(NDI_UNIQUE, 0, victim, MSG_TYPE_SPELL, MSG_TYPE_SPELL_TARGET,
                      "You get a splitting headache!");
    } else if (victim->stats.sp >= victim->stats.maxsp*1.5) {
        draw_ext_info(NDI_UNIQUE, 0, victim, MSG_TYPE_SPELL, MSG_TYPE_SPELL_TARGET,
                      "Chaos fills your world.");
        confuse_living(victim, victim, 99);
    } else if (victim->stats.sp >= victim->stats.maxsp*1.25) {
        draw_ext_info(NDI_UNIQUE, 0, victim, MSG_TYPE_SPELL, MSG_TYPE_SPELL_TARGET,
                      "You start hearing voices.");
    }
}

/**
 * This spell transfers sp from the player to another person.
 * We let the target go above their normal maximum SP.
 *
 * @param op
 * who is casting.
 * @param caster
 * what is casting.
 * @param spell
 * actual spell object.
 * @param dir
 * casting direction.
 * @retval 0
 * no transfer happened.
 * @retval 1
 * transfer happened.
 */
int cast_transfer(object *op, object *caster, object *spell, int dir) {
    object *plyr = NULL;
    sint16 x, y;
    mapstruct *m;
    int mflags;

    m = op->map;
    x =  op->x+freearr_x[dir];
    y = op->y+freearr_y[dir];

    mflags = get_map_flags(m, &m, x, y, &x, &y);
    if (!(mflags&P_OUT_OF_MAP) && mflags&P_IS_ALIVE) {
        FOR_MAP_PREPARE(m, x, y, tmp)
            plyr = tmp;
            if (plyr != op && QUERY_FLAG(plyr, FLAG_ALIVE))
                break;
        FOR_MAP_FINISH();
    }

    /* If we did not find a player in the specified direction, transfer
     * to anyone on top of us. This is used for the rune of transference mostly.
     */
    if (plyr == NULL)
        FOR_MAP_PREPARE(op->map, op->x, op->y, tmp)
            plyr = tmp;
            if (plyr != op && QUERY_FLAG(plyr, FLAG_ALIVE))
                break;
        FOR_MAP_FINISH();

    if (!plyr) {
        draw_ext_info(NDI_BLACK, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                      "There is no one there.");
        return 0;
    }

    /* give sp */
    if (spell->stats.dam > 0) {
        plyr->stats.sp += spell->stats.dam+SP_level_dam_adjust(caster, spell);
        charge_mana_effect(plyr, caster_level(caster, spell));
        return 1;
    /* suck sp away.  Can't suck sp from yourself */
    } else if (op != plyr) {
        /* old dragin magic used floats.  easier to just use ints and divide by 100 */

        int rate = -spell->stats.dam+SP_level_dam_adjust(caster, spell), sucked;

        if (rate > 95)
            rate = 95;

        sucked = (plyr->stats.sp*rate)/100;
        plyr->stats.sp -= sucked;
        if (QUERY_FLAG(op, FLAG_ALIVE)) {
            /* Player doesn't get full credit */
            sucked = (sucked*rate)/100;
            op->stats.sp += sucked;
            if (sucked > 0) {
                charge_mana_effect(op, caster_level(caster, spell));
            }
        }
        return 1;
    }
    return 0;
}

/**
 * Nullifies spell effects.
 * Basically, if the object has a magic attacktype, this may nullify it.
 *
 * @param op
 * counterspell object.
 * @param dir
 * direction it was cast in.
 */
void counterspell(object *op, int dir) {
    object *head;
    int mflags;
    mapstruct *m;
    sint16  sx, sy;

    sx = op->x+freearr_x[dir];
    sy = op->y+freearr_y[dir];
    m = op->map;
    mflags = get_map_flags(m, &m, sx, sy, &sx, &sy);
    if (mflags&P_OUT_OF_MAP)
        return;

    FOR_MAP_PREPARE(m, sx, sy, tmp) {
        object *owner;

        /* Need to look at the head object - otherwise, if tmp
         * points to a monster, we don't have all the necessary
         * info for it.
         */
        head = HEAD(tmp);

        /* don't attack our own spells */
        owner = object_get_owner(tmp);
        if (owner != NULL && owner == object_get_owner(op))
            continue;

        /* Basically, if the object is magical and not counterspell,
         * we will more or less remove the object.  Don't counterspell
         * monsters either.
         */

        if (head->attacktype&AT_MAGIC
        && !(head->attacktype&AT_COUNTERSPELL)
        && !QUERY_FLAG(head, FLAG_MONSTER)
        && (op->level > head->level)) {
            object_remove(head);
            object_free2(head, 0);
        } else switch (head->type) {
            case SPELL_EFFECT:
                if ((op->level > head->level) && !op->stats.food && !op->speed_left) {
                    object_remove(head);
                    object_free2(head, 0);
                }
                break;

                /* I really don't get this rune code that much - that
                 * random chance seems really low.
                 */
            case RUNE:
                if (rndm(0, 149) == 0) {
                    head->stats.hp--;  /* weaken the rune */
                    if (!head->stats.hp) {
                        object_remove(head);
                        object_free2(head, 0);
                    }
                }
                break;
            }
    } FOR_MAP_FINISH();
}

/**
 * A spell to make an altar your god's.
 *
 * @param op
 * who is casting.
 * @param caster
 * what is casting.
 * @param spell
 * actual spell object.
 * @retval 0
 * no consecration happened.
 * @retval 1
 * an altar waas consecrated.
 */
int cast_consecrate(object *op, object *caster, object *spell) {
    char buf[MAX_BUF];
    const object *god = find_god(determine_god(op));

    if (!god) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                      "You can't consecrate anything if you don't worship a god!");
        return 0;
    }

    FOR_BELOW_PREPARE(op, tmp) {
        if (QUERY_FLAG(tmp, FLAG_IS_FLOOR))
            break;
        if (tmp->type == HOLY_ALTAR) {
            if (tmp->level > caster_level(caster, spell)) {
                draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                                     "You are not powerful enough to reconsecrate the %s",
                                     tmp->name);
                return 0;
            } else {
                /* If we got here, we are consecrating an altar */
                object *new_altar;
                size_t letter;
                archetype *altar_arch;

                snprintf(buf, MAX_BUF, "altar_");
                letter = strlen(buf);
                strncpy(buf+letter, god->name, MAX_BUF-letter);
                for (; letter < strlen(buf); letter++)
                    buf[letter] = tolower(buf[letter]);
                altar_arch = find_archetype(buf);
                if (!altar_arch) {
                    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                                         "You fail to consecrate the altar.");
                    LOG(llevError, "cast_consecrate: can't find altar %s for god %s\n", buf, god->name);
                    return 0;
                }
                new_altar = arch_to_object(altar_arch);
                new_altar->level = tmp->level;
                object_insert_in_map_at(new_altar, tmp->map, tmp, INS_BELOW_ORIGINATOR, tmp->x, tmp->y);
                object_remove(tmp);
                draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_SUCCESS,
                                     "You consecrated the altar to %s!",
                                     god->name);
                return 1;
            }
        }
    } FOR_BELOW_FINISH();
    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                  "You are not standing over an altar!");
    return 0;
}

/**
 * Generalization of staff_to_snake().  Makes a golem out of the caster's weapon.
 * The golem is based on the archetype specified, modified by the caster's level
 * and the attributes of the weapon.  The weapon is inserted in the golem's
 * inventory so that it falls to the ground when the golem dies.
 * This code was very odd - code early on would only let players use the spell,
 * yet the code was full of player checks.  I've presumed that the code
 * that only let players use it was correct, and removed all the other
 * player checks. MSW 2003-01-06
 *
 * @param op
 * who is casting.
 * @param caster
 * what is casting.
 * @param spell
 * actual spell object.
 * @param dir
 * casting direction.
 * @retval 0
 * spell failure.
 * @retval 1
 * spell was successful.
 */
int animate_weapon(object *op, object *caster, object *spell, int dir) {
    object *weapon, *tmp;
    char buf[MAX_BUF];
    int a, i;
    sint16 x, y;
    mapstruct *m;
    materialtype_t *mt;

    if (!spell->other_arch) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                      "Oops, program error!");
        LOG(llevError, "animate_weapon failed: spell %s missing other_arch!\n", spell->name);
        return 0;
    }

    /* exit if it's not a player using this spell. */
    if (op->type != PLAYER)
        return 0;

    /* if player already has a golem, abort */
    if (op->contr->ranges[range_golem] != NULL && op->contr->golem_count == op->contr->ranges[range_golem]->count) {
        pets_control_golem(op->contr->ranges[range_golem], dir);
        return 0;
    }

    /* if no direction specified, pick one */
    if (!dir)
        dir = object_find_free_spot(NULL, op->map, op->x, op->y, 1, 9);

    m = op->map;
    x = op->x+freearr_x[dir];
    y = op->y+freearr_y[dir];

    /* if there's no place to put the golem, abort */
    if ((dir == -1)
    || (get_map_flags(m, &m, x, y, &x, &y)&P_OUT_OF_MAP)
    || ((spell->other_arch->clone.move_type&GET_MAP_MOVE_BLOCK(m, x, y)) == spell->other_arch->clone.move_type)) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                      "There is something in the way.");
        return 0;
    }

    /* Use the weapon marked by the player. */
    weapon = find_marked_object(op);

    if (!weapon) {
        draw_ext_info(NDI_BLACK, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                      "You must mark a weapon to use with this spell!");
        return 0;
    }
    if (spell->race && strcmp(weapon->arch->name, spell->race)) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                      "The spell fails to transform your weapon.");
        return 0;
    }
    if (weapon->type != WEAPON) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                      "You need to mark a weapon to animate it.");
        return 0;
    }
    if (QUERY_FLAG(weapon, FLAG_UNPAID)) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                      "You need to pay for the weapon to animate it.");
        return 0;
    }
    if (QUERY_FLAG(weapon, FLAG_APPLIED)) {
        char wn[MAX_BUF];

        query_name(weapon, wn, MAX_BUF);
        draw_ext_info_format(NDI_BLACK, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                             "You need to unequip %s before using it in this spell",
                             wn);
        return 0;
    }

    if (weapon->nrof > 1) {
        tmp = object_split(weapon, 1, NULL, 0);
        esrv_update_item(UPD_NROF, op, weapon);
        weapon = tmp;
    }

    /* create the golem object */
    tmp = arch_to_object(spell->other_arch);

    /* if animated by a player, give the player control of the golem */
    CLEAR_FLAG(tmp, FLAG_MONSTER);
    SET_FLAG(tmp, FLAG_FRIENDLY);
    tmp->stats.exp = 0;
    add_friendly_object(tmp);
    tmp->type = GOLEM;
    object_set_owner(tmp, op);
    set_spell_skill(op, caster, spell, tmp);
    op->contr->ranges[range_golem] = tmp;
    op->contr->shoottype = range_golem;
    op->contr->golem_count = tmp->count;

    /* Give the weapon to the golem now.  A bit of a hack to check the
     * removed flag - it should only be set if object_split() was
     * used above.
     */
    if (!QUERY_FLAG(weapon, FLAG_REMOVED))
        object_remove(weapon);
    object_insert_in_ob(weapon, tmp);

    /* To do everything necessary to let a golem use the weapon is a pain,
     * so instead, just set it as equipped (otherwise, we need to update
     * body_info, skills, etc)
     */
    SET_FLAG(tmp, FLAG_USE_WEAPON);
    SET_FLAG(weapon, FLAG_APPLIED);
    fix_object(tmp);

    /* There used to be 'odd' code that basically seemed to take the absolute
     * value of the weapon->magic an use that.  IMO, that doesn't make sense -
     * if you're using a crappy weapon, it shouldn't be as good.
     */

    /* modify weapon's animated wc */
    tmp->stats.wc = tmp->stats.wc
        -SP_level_range_adjust(caster, spell)
        -5*weapon->stats.Dex
        -2*weapon->stats.Str
        -weapon->magic;
    if (tmp->stats.wc < -127)
        tmp->stats.wc = -127;

    /* Modify hit points for weapon */
    tmp->stats.maxhp = tmp->stats.maxhp
        +spell->duration
        +SP_level_duration_adjust(caster, spell)
        +8*weapon->magic
        +12*weapon->stats.Con;
    if (tmp->stats.maxhp < 0)
        tmp->stats.maxhp = 10;
    tmp->stats.hp = tmp->stats.maxhp;

    /* Modify weapon's damage */
    tmp->stats.dam = spell->stats.dam
        +SP_level_dam_adjust(caster, spell)
        +weapon->stats.dam
        +weapon->magic
        +5*weapon->stats.Str;
    if (tmp->stats.dam < 0)
        tmp->stats.dam = 127;

    /* attacktype */
    if (!tmp->attacktype)
        tmp->attacktype = AT_PHYSICAL;

    mt = NULL;
    if (op->materialname != NULL)
        mt = name_to_material(op->materialname);
    if (mt != NULL) {
        for (i = 0; i < NROFATTACKS; i++)
            tmp->resist[i] = 50-(mt->save[i]*5);
        a = mt->save[0];
    } else {
        for (i = 0; i < NROFATTACKS; i++)
            tmp->resist[i] = 5;
        a = 10;
    }
    /* Set weapon's immunity */
    tmp->resist[ATNR_CONFUSION] = 100;
    tmp->resist[ATNR_POISON] = 100;
    tmp->resist[ATNR_SLOW] = 100;
    tmp->resist[ATNR_PARALYZE] = 100;
    tmp->resist[ATNR_TURN_UNDEAD] = 100;
    tmp->resist[ATNR_FEAR] = 100;
    tmp->resist[ATNR_DEPLETE] = 100;
    tmp->resist[ATNR_DEATH] = 100;
    tmp->resist[ATNR_BLIND] = 100;

    /* Improve weapon's armour value according to best save vs. physical of its material */

    if (a > 14)
        a = 14;
    tmp->resist[ATNR_PHYSICAL] = 100-(int)((100.0-(float)tmp->resist[ATNR_PHYSICAL])/(30.0-2.0*a));

    /* Determine golem's speed */
    tmp->speed = 0.4+0.1*SP_level_range_adjust(caster, spell);

    if (tmp->speed > 3.33)
        tmp->speed = 3.33;

    if (!spell->race) {
        snprintf(buf, sizeof(buf), "animated %s", weapon->name);
        if (tmp->name)
            free_string(tmp->name);
        tmp->name = add_string(buf);

        tmp->face = weapon->face;
        tmp->animation_id = weapon->animation_id;
        tmp->anim_speed = weapon->anim_speed;
        tmp->last_anim = weapon->last_anim;
        tmp->state = weapon->state;
        if (QUERY_FLAG(weapon, FLAG_ANIMATE)) {
            SET_FLAG(tmp, FLAG_ANIMATE);
        } else {
            CLEAR_FLAG(tmp, FLAG_ANIMATE);
        }
        object_update_speed(tmp);
    }

    /*  make experience increase in proportion to the strength of the summoned creature. */
    tmp->stats.exp *= 1+(MAX(spell->stats.maxgrace, spell->stats.sp)/caster_level(caster, spell));

    tmp->speed_left = -1;
    tmp->direction = dir;
    object_insert_in_map_at(tmp, m, op, 0, x, y);
    return 1;
}

/**
 * This changes the light level for the entire map.
 *
 * @param op
 * who is casting.
 * @param caster
 * what is casting.
 * @param spell
 * actual spell object.
 * @retval 0
 * light not affected.
 * @retval 1
 * light changed.
 */
int cast_change_map_lightlevel(object *op, object *caster, object *spell) {
    int success;

    if (!op->map)
        return 0;  /* shouldnt happen */

    success = change_map_light(op->map, spell->stats.dam);
    if (!success) {
        if (spell->stats.dam < 0)
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                          "It can be no brighter here.");
        else
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_FAILURE,
                          "It can be no darker here.");
    }
    return success;
}

/**
 * Create an aura spell object and put it in the player's inventory.
 * This is also used for elemental shields - the creation is the same
 * just the 'move' code is different.
 *
 * @param op
 * who is casting.
 * @param caster
 * what is casting.
 * @param spell
 * actual spell object.
 * @return
 * 1.
 */
int create_aura(object *op, object *caster, object *spell) {
    int refresh = 0, i;
    object *new_aura;

    new_aura = arch_present_in_ob(spell->other_arch, op);
    if (new_aura)
        refresh = 1;
    else
        new_aura = arch_to_object(spell->other_arch);

    new_aura->duration = spell->duration+10*SP_level_duration_adjust(caster, spell);
    if (op->type == PLAYER)
        store_spell_expiry(new_aura);

    new_aura->stats.dam = spell->stats.dam+SP_level_dam_adjust(caster, spell);

    object_set_owner(new_aura, op);
    set_spell_skill(op, caster, spell, new_aura);
    new_aura->attacktype = spell->attacktype;

    new_aura->level = caster_level(caster, spell);

    /* Code below is so that auras can also provide resistances.  For
     * example, fire shield both does damage to nearby enemies and also
     * provides some protection to fire.  We need to use a different
     * FORCE object for this, as putting it in with the aura object
     * just puts too many meanings into that one object.  Because
     * the duration of this force object is the same, we don't need
     * to set up spell expiry on it - this second object is really
     * an internal mechanic that should be invisible to the player.
     */
    for (i = 0; i < NROFATTACKS; i++) {
        if (spell->resist[i]) {
            int refresh1=1;
            object *force;

            force = object_present_in_ob_by_name(FORCE, spell->name, op);
            if (!force) {
                force=create_archetype(FORCE_NAME);
                force->subtype = FORCE_CHANGE_ABILITY;
                free_string(force->name);
                force->name = add_refcount(spell->name);
                free_string(force->name_pl);
                force->name_pl = add_refcount(spell->name);
                refresh1=0;
            }
            force->duration = new_aura->duration;
            force->speed = new_aura->speed;
            memcpy(&force->resist, spell->resist, sizeof(spell->resist));
            SET_FLAG(force, FLAG_APPLIED);

            if (!refresh1)
                object_insert_in_ob(force, op);
            change_abil(op, new_aura);
            fix_object(op);
            break;
        }
    }

    if (refresh)
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_SUCCESS,
                      "You recast the spell while in effect.");
    else
        object_insert_in_ob(new_aura, op);
    return 1;
}

/**
 * This writes a rune that contains the appropriate message.
 * There really aren't any adjustments we make.
 *
 * @param op
 * who is casting.
 * @param spell
 * actual spell cast.
 * @param msg
 * message to write.
 * @retval 0
 * failure.
 * @retval 1
 * success.
 */
int write_mark(object *op, object *spell, const char *msg) {
    char rune[HUGE_BUF];
    object *tmp;

    if (!msg || msg[0] == 0) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                      "Write what?");
        return 0;
    }

    if (strcasestr_local(msg, "endmsg")) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_ERROR,
                      "Trying to cheat are we?");
        LOG(llevInfo, "write_rune: player %s tried to write bogus rune %s\n", op->name, msg);
        return 0;
    }

    if (!spell->other_arch)
        return 0;
    tmp = arch_to_object(spell->other_arch);
    strncpy(rune, msg, HUGE_BUF-2);
    rune[HUGE_BUF-2] = 0;
    strcat(rune, "\n");
    tmp->race = add_string(op->name);   /*Save the owner of the rune*/
    object_set_msg(tmp, rune);
    object_insert_in_map_at(tmp, op->map, op, INS_BELOW_ORIGINATOR, op->x, op->y);
    return 1;
}
