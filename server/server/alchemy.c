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

/* March 96 - Laid down original code. -b.t. thomas@astro.psu.edu */

/**
 * @file
 * This contains all alchemy-related functions.
 */

#include <global.h>
#include <object.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#endif
#include <skills.h>
#include <spells.h>
#include <assert.h>

/** define this for some helpful debuging information */
#if 0
#define ALCHEMY_DEBUG
#endif

/** define this for loads of (marginal) debuging information */
#if 0
#define EXTREME_ALCHEMY_DEBUG
#endif

/** Random cauldrons effects */
static const char *const cauldron_effect [] = {
    "vibrates briefly",
    "produces a cloud of steam",
    "emits bright flames",
    "pours forth heavy black smoke",
    "emits sparks",
    "shoots out small flames",
    "whines painfully",
    "hiccups loudly",
    "wheezes",
    "burps",
    "shakes",
    "rattles",
    "makes chugging sounds",
    "smokes heavily for a while"
};


static int is_defined_recipe(const recipe *rp, const object *cauldron, object *caster);
static const recipe *find_recipe(const recipelist *fl, int formula, object *ingredients);
static int content_recipe_value(object *op);
static int numb_ob_inside(const object *op);
static void alchemy_failure_effect(object *op, object *cauldron, const recipe *rp, int danger);
static object *attempt_recipe(object *caster, object *cauldron, int ability, const recipe *rp, int nbatches, int ignore_cauldron);
static int calc_alch_danger(object *caster, object *cauldron, const recipe *rp);
static object *make_item_from_recipe(object *cauldron, const recipe *rp);
static void remove_contents(object *first_ob, object *save_item);
static void adjust_product(object *item, int lvl, int yield);
static object *find_transmution_ob(object *first_ingred, const recipe *rp, size_t *rp_arch_index, int create_item);
static void attempt_do_alchemy(object *caster, object *cauldron);

/** Returns a random selection from cauldron_effect[] */
static const char *cauldron_sound(void) {
    int size = sizeof(cauldron_effect)/sizeof(char *);

    return cauldron_effect[rndm(0, size-1)];
}

/**
 * Compute the success probability of a recipe.
 *
 * The scale is kind of linear, with 3 steps depending on the level difference between recipe and skill.
 *
 * @param rp recipe to attempt.
 * @param skill skill being used.
 * @param cauldron provides the magic of the used device.
 * @return chance between 0.01 and .95.
 */
static float recipe_chance(const recipe *rp, const object *skill, const object *cauldron) {
    int cauldron_add_skill;

    assert(rp);
    assert(skill);
    assert(cauldron);

    cauldron_add_skill = (cauldron->magic + 1) / 2;

    if (skill->level + cauldron_add_skill < rp->diff - 10)
        return MAX(.01, .3 - (rp->diff - 10 - skill->level - cauldron_add_skill) * .03);

    if (skill->level + cauldron_add_skill <= rp->diff + 10)
        return .5 + .02 * (float)(skill->level + cauldron_add_skill - rp->diff);

    return MIN(.95, .70 + (skill->level + cauldron_add_skill - rp->diff - 10) * .01);
}

/**
 * Main part of the ALCHEMY code. From this we call fctns
 * that take a look at the contents of the 'cauldron' and, using these ingredients,
 * we construct an integer formula value which is referenced (randomly) against a
 * formula list (the formula list chosen is based on the # contents of the cauldron).
 *
 * If we get a match between the recipe indicated in cauldron contents and a
 * randomly chosen one, an item is created and experience awarded. Otherwise
 * various failure effects are possible (getting worse and worse w/ # cauldron
 * ingredients). Note that the 'item' to be made can be *anything *listed on
 * the artifacts list in lib/artifacts which has a recipe listed in lib/formulae.
 *
 * To those wondering why I am using the funky formula index method:
 *   1) I want to match recipe to ingredients regardless of ordering.
 *   2) I want a fast search for the 'right' recipe.
 *
 * Note: it is just possible that a totally different combination of
 * ingredients will result in a match with a given recipe. This is not a bug!
 * There is no good reason (in my mind) why alchemical processes have to be
 * unique -- such a 'feature' is one reason why players might want to experiment
 * around. :)
 * -b.t.
 *
 * @param caster
 * who is doing alchemy.
 * @param cauldron
 * the cauldron in which alchemy should take place.
 */
static void attempt_do_alchemy(object *caster, object *cauldron) {
    const recipelist *fl;
    const recipe *rp = NULL;
    float success_chance;
    int numb, ability = 1;
    int formula = 0;
    object *item, *skop;

    if (caster->type != PLAYER)
        return; /* only players for now */

    /* if no ingredients, no formula! lets forget it */
    if (!(formula = content_recipe_value(cauldron))) {
        draw_ext_info_format(NDI_UNIQUE, 0, caster, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                             "The %s is empty.",
                             cauldron->name);
        return;
    }

    numb = numb_ob_inside(cauldron);
    if ((fl = get_formulalist(numb))) {
        if (QUERY_FLAG(caster, FLAG_WIZ)) {
            rp = find_recipe(fl, formula, cauldron->inv);
            if (rp != NULL) {
#ifdef ALCHEMY_DEBUG
                if (strcmp(rp->title, "NONE"))
                    LOG(llevDebug, "WIZ got formula: %s of %s\n", rp->arch_name[0], rp->title);
                else
                    LOG(llevDebug, "WIZ got formula: %s (nbatches:%d)\n", rp->arch_name[0], formula/rp->index);
#endif
                attempt_recipe(caster, cauldron, ability, rp, formula/rp->index, !is_defined_recipe(rp, cauldron, caster));
            } else
                LOG(llevDebug, "WIZ couldn't find formula for ingredients.\n");
            return;
        } /* End of WIZ alchemy */

        /* find the recipe */
        rp = find_recipe(fl, formula, cauldron->inv);
        if (rp != NULL) {
            uint64 value_ingredients;
            uint64 value_item;
            int attempt_shadow_alchemy;

            /* the caster gets an increase in ability based on thier skill lvl */
            if (rp->skill != NULL) {
                skop = find_skill_by_name(caster, rp->skill);
                if (!skop) {
                    draw_ext_info(NDI_UNIQUE, 0, caster, MSG_TYPE_SKILL, MSG_TYPE_SKILL_MISSING,
                                  "You do not have the proper skill for this recipe");
                } else {
                    ability += skop->level*((4.0+cauldron->magic)/4.0);
                }
            } else {
                LOG(llevDebug, "Recipe %s has NULL skill!\n", rp->title);
                return;
            }

            if (rp->cauldron == NULL) {
                LOG(llevDebug, "Recipe %s has NULL cauldron!\n", rp->title);
                return;
            }

            if (rp->min_level != 0 && skop->level < rp->min_level) {
                draw_ext_info(NDI_UNIQUE, 0, caster, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                    "You aren't skilled enough to try this recipe.");
                return;
            }

            /* determine value of ingredients */
            value_ingredients = 0;
            FOR_INV_PREPARE(cauldron, tmp)
                value_ingredients += query_cost(tmp, NULL, BS_TRUE);
            FOR_INV_FINISH();

            attempt_shadow_alchemy = !is_defined_recipe(rp, cauldron, caster);

            /* create the object **FIRST**, then decide whether to keep it. */
            if ((item = attempt_recipe(caster, cauldron, ability, rp, formula/rp->index, attempt_shadow_alchemy)) != NULL) {
                /*  compute base chance of recipe success */
                success_chance = recipe_chance(rp, skop, cauldron);

#ifdef ALCHEMY_DEBUG
                LOG(llevDebug, "percent success chance =  %f ab%d / diff%d*lev%d\n", success_chance, ability, rp->diff, item->level);
#endif

                value_item = query_cost(item, NULL, BS_TRUE|BS_IDENTIFIED|BS_NOT_CURSED);
                if (attempt_shadow_alchemy && value_item > value_ingredients) {
#ifdef ALCHEMY_DEBUG
#ifndef WIN32
                    LOG(llevDebug, "Forcing failure for shadow alchemy recipe because price of ingredients (%llu) is less than price of result (%llu).\n", value_ingredients, value_item);
#else
                    LOG(llevDebug, "Forcing failure for shadow alchemy recipe because price of ingredients (%I64d) is less than price of result (%I64d).\n", value_ingredients, value_item);
#endif
#endif
                /* roll the dice */
                } else if (random_roll(0, 101, caster, PREFER_LOW) <= 100.0*success_chance) {
                    change_exp(caster, rp->exp, rp->skill, SK_EXP_NONE);
                    return;
                }
            }
        }
    }
    /* if we get here, we failed!! */
    alchemy_failure_effect(caster, cauldron, rp, calc_alch_danger(caster, cauldron, rp));
}

/**
 * Recipe value of the entire contents of a container.
 * This appears to just generate a hash value, which I guess for now works
 * ok, but the possibility of duplicate hashes is certainly possible - msw
 *
 * @param op
 * contained for which to generate a hash.
 * @return
 * hash value.
 */
static int content_recipe_value(object *op) {
    char name[MAX_BUF];
    int tval = 0, formula = 0;

    FOR_INV_PREPARE(op, tmp) {
        tval = 0;
        strcpy(name, tmp->name);
        if (tmp->title)
            snprintf(name, sizeof(name), "%s %s", tmp->name, tmp->title);
        tval = (strtoint(name)*(tmp->nrof ? tmp->nrof : 1));
#ifdef ALCHEMY_DEBUG
        LOG(llevDebug, "Got ingredient %d %s(%d)\n", tmp->nrof ? tmp->nrof : 1, name, tval);
#endif
        formula += tval;
    } FOR_INV_FINISH();
#ifdef ALCHEMY_DEBUG
    LOG(llevDebug, " Formula value=%d\n", formula);
#endif
    return formula;
}

/**
 * Returns the total number of items in op, excluding ones in item's items.
 * @param op
 * container.
 * @return
 * total item count.
 */
static int numb_ob_inside(const object *op) {
    int o_number = 0;

    FOR_INV_PREPARE(op, tmp)
        o_number++;
    FOR_INV_FINISH();
#ifdef ALCHEMY_DEBUG
    LOG(llevDebug, "numb_ob_inside(%s): found %d ingredients\n", op->name, o_number);
#endif
    return o_number;
}

/**
 * Essentially a wrapper for make_item_from_recipe() and
 * object_insert_in_ob(). If the caster has some alchemy skill, then they might
 * gain some exp from (successfull) fabrication of the product.
 * If nbatches==-1, don't give exp for this creation (random generation/
 * failed recipe)
 * If ignore_cauldron, don't check if we are using the matching cauldron
 * type (shadow alchemy)
 *
 * @param caster
 * who is trying to do alchemy.
 * @param cauldron
 * container used for alchemy.
 * @param ability
 * ?
 * @param rp
 * recipe attempted.
 * @param nbatches
 * ?
 * @param ignore_cauldron
 * if 0, checks the recipe uses the right cauldron type, else no check is done.
 * @return
 * generated item, can be NULL if contents were destroyed.
 * @todo check meaning of ability/nbatches.
 */
static object *attempt_recipe(object *caster, object *cauldron, int ability, const recipe *rp, int nbatches, int ignore_cauldron) {
    object *item = NULL, *skop;
    /* this should be passed to this fctn, not effiecent cpu use this way */
    int batches = abs(nbatches);

    /* is the cauldron the right type? */
    if (!ignore_cauldron && (strcmp(rp->cauldron, cauldron->arch->name) != 0)) {
        draw_ext_info(NDI_UNIQUE, 0, caster, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                      "You are not using the proper facilities for this formula.");
        return NULL;
    }

    skop = find_skill_by_name(caster, rp->skill);
    /* does the caster have the skill? */
    if (!skop)
        return NULL;

    /* code required for this recipe, search the caster */
    if (rp->keycode) {
        object *tmp;

        tmp = object_find_by_type_and_slaying(caster, FORCE, rp->keycode);
        if (tmp == NULL) { /* failure--no code found */
            draw_ext_info(NDI_UNIQUE, 0, caster, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                          "You know the ingredients, but not the technique.  Go learn how to do this recipe.");
            return NULL;
        }
    }

#ifdef EXTREME_ALCHEMY_DEBUG
    LOG(llevDebug, "attempt_recipe(): got %d nbatches\n", nbatches);
    LOG(llevDebug, "attempt_recipe(): using recipe %s\n", rp->title ? rp->title : "unknown");
#endif

    if ((item = make_item_from_recipe(cauldron, rp)) != NULL) {
        remove_contents(cauldron->inv, item);
        /* adj lvl, nrof on caster level */
        adjust_product(item, ability, rp->yield ? (rp->yield*batches) : batches);

        if (item->type == POTION) {
            item->level = MAX(item->level, skop->level);
        }

        if (!item->env && (item = object_insert_in_ob(item, cauldron)) == NULL) {
            draw_ext_info(NDI_UNIQUE, 0, caster, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                          "Nothing happened.");
        } else {
            draw_ext_info_format(NDI_UNIQUE, 0, caster,
                                 MSG_TYPE_SKILL, MSG_TYPE_SKILL_SUCCESS,
                                 "The %s %s.",
                                 cauldron->name, cauldron_sound());
        }
    }
    return item;
}

/**
 * We adjust the nrof of the final product, based
 * on the item's default parameters, and the relevant caster skill level.
 * @param item
 * item to adjust.
 * @param adjust
 * nrof adjustment parameter, the higher the better.
 * @param yield
 * how many products the recipe returns at maximum.
 */
static void adjust_product(object *item, int adjust, int yield) {
    int nrof = 1;

    if (!yield)
        yield = 1;
    if (adjust <= 0)
        adjust = 1; /* lets avoid div by zero! */
    if (item->nrof) {
        nrof = (1.0-1.0/(adjust/10.0+1.0))*(rndm(0, yield-1)+rndm(0, yield-1)+rndm(0, yield-1))+1;
        if (nrof > yield)
            nrof = yield;
        item->nrof = nrof;
    }
}

/**
 * Using a list of items and a recipe to make an artifact.
 *
 * @param cauldron the cauldron (including the ingredients) used to make the item
 *
 * @param rp the recipe to make the artifact from
 *
 * @return the newly created object, NULL if something failed
 */

static object *make_item_from_recipe(object *cauldron, const recipe *rp) {
    const artifact *art = NULL;
    object *item = NULL;
    size_t rp_arch_index;

    if (rp == NULL)
        return (object *)NULL;

    /* Find the appropriate object to transform...*/
    if ((item = find_transmution_ob(cauldron->inv, rp, &rp_arch_index, 1)) == NULL) {
        LOG(llevDebug, "make_alchemy_item(): failed to create alchemical object.\n");
        return (object *)NULL;
    }

    /* If item is already in container, we need to remove its weight, since it can change later on. */
    if (item->env != NULL)
        object_sub_weight(cauldron, item->weight*(item->nrof != 0 ? item->nrof : 1));

    /* Find the appropriate artifact template...*/
    if (strcmp(rp->title, "NONE")) {
        if ((art = locate_recipe_artifact(rp, rp_arch_index)) == NULL) {
            LOG(llevError, "make_alchemy_item(): failed to locate recipe artifact.\n");
            LOG(llevDebug, "  --requested recipe: %s of %s.\n", rp->arch_name[0], rp->title);
            return (object *)NULL;
        }
        transmute_materialname(item, art->item);
        give_artifact_abilities(item, art->item);
        if (need_identify(item) && QUERY_FLAG(item, FLAG_IDENTIFIED)) {
            /* artifacts are generated unidentified, so if the item is identified we must give it its real properties. */
            object_give_identified_properties(item);
        }
    }
    if (item->env != NULL)
        object_add_weight(cauldron, item->weight*(item->nrof != 0 ? item->nrof : 1));

    if (QUERY_FLAG(cauldron, FLAG_CURSED))
        SET_FLAG(item, FLAG_CURSED);
    if (QUERY_FLAG(cauldron, FLAG_DAMNED))
        SET_FLAG(item, FLAG_DAMNED);

    return item;
}

/**
 * Looks through the ingredient list. If we find a
 * suitable object in it - we will use that to make the requested artifact.
 * Otherwise the code returns a 'generic' item if create_item is set. -b.t.
 *
 * @param first_ingred pointer to first item to check
 * @param rp recipe the player is trying
 * @param rp_arch_index pointer to return value; set to arch index for recipe;
 * set to zero if not using a transmution formula
 * @param create_item if set, will create a generic item if no suitable item is found.
 * @return
 * NULL if no suitable item was found and create_item is 0, existing or new item else.
 */
static object *find_transmution_ob(object *first_ingred, const recipe *rp, size_t *rp_arch_index, int create_item) {
    object *item = NULL;

    *rp_arch_index = 0;

    if (rp->transmute) { /* look for matching ingredient/prod archs */
        item = first_ingred;
        FOR_OB_AND_BELOW_PREPARE(item) {
            size_t i;

            for (i = 0; i < rp->arch_names; i++) {
                if (strcmp(item->arch->name, rp->arch_name[i]) == 0) {
                    *rp_arch_index = i;
                    break;
                }
            }
            if (i < rp->arch_names)
                break;
        } FOR_OB_AND_BELOW_FINISH();
    }

    /* failed, create a fresh object. Note no nrof>1 because that would
     * allow players to create massive amounts of artifacts easily */
    if (create_item && (!item || item->nrof > 1)) {
        *rp_arch_index = RANDOM()%rp->arch_names;
        item = create_archetype(rp->arch_name[*rp_arch_index]);
    }

#ifdef ALCHEMY_DEBUG
    LOG(llevDebug, "recipe calls for%stransmution.\n", rp->transmute ? " " : " no ");
    if (item != NULL) {
        LOG(llevDebug, " find_transmutable_ob(): returns arch %s(sp:%d)\n", item->arch->name, item->stats.sp);
    }
#endif

    return item;
}

/**
 * Ouch. We didnt get the formula we wanted.
 * This fctn simulates the backfire effects--worse effects as the level
 * increases. If SPELL_FAILURE_EFFECTS is defined some really evil things
 * can happen to the would be alchemist. This table probably needs some
 * adjustment for playbalance. -b.t.
 *
 * @param op
 * who tried to do alchemy.
 * @param cauldron
 * container that was used.
 * @param rp
 * recipe that failed, can be NULL.
 * @param danger
 * danger value, the higher the more evil the effect.
 */
static void alchemy_failure_effect(object *op, object *cauldron, const recipe *rp, int danger) {
    int level = 0;

    if (!op || !cauldron)
        return;

    /** Recipe specifies a special failure archetype, so use it instead of evil random things. */
    if (rp && rp->failure_arch) {
        object *failure = create_archetype(rp->failure_arch);
        if (!failure) {
            LOG(llevError, "invalid failure_arch %s for recipe %s\n", rp->failure_arch, rp->title);
            return;
        }

        remove_contents(cauldron->inv, NULL);

        object_insert_in_ob(failure, cauldron);
        if (rp->failure_message) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE, rp->failure_message);
        }
        return;
    }

    if (danger > 1)
        level = random_roll(1, danger, op, PREFER_LOW);

#ifdef ALCHEMY_DEBUG
    LOG(llevDebug, "Alchemy_failure_effect(): using level=%d\n", level);
#endif

    /* possible outcomes based on level */
    if (level < 25) {         /* INGREDIENTS USED/SLAGGED */
        object *item = NULL;

        if (rndm(0, 2)) { /* slag created */
            object *tmp;
            int weight = 0;
            uint16 material = M_STONE;

            FOR_INV_PREPARE(cauldron, tmp) { /* slag has coadded ingredient properties */
                weight += tmp->weight;
                if (!(material&tmp->material))
                    material |= tmp->material;
            } FOR_INV_FINISH();
            tmp = create_archetype("rock");
            tmp->weight = weight;
            tmp->value = 0;
            tmp->material = material;
            tmp->materialname = add_string("stone");
            free_string(tmp->name);
            tmp->name = add_string("slag");
            if (tmp->name_pl)
                free_string(tmp->name_pl);
            tmp->name_pl = add_string("slags");
            item = object_insert_in_ob(tmp, cauldron);
            CLEAR_FLAG(tmp, FLAG_CAN_ROLL);
            CLEAR_FLAG(tmp, FLAG_NO_PICK);
            tmp->move_block = 0;
        }
        remove_contents(cauldron->inv, item);
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                             "The %s %s.",
                             cauldron->name, cauldron_sound());
        return;
    } else if (level < 40) {                 /* MAKE TAINTED ITEM */
        object *tmp = NULL;

        /*
         * Note by Nicolas Weeger 2010-09-26
         * This is an incorrect part.
         * Calling again attempt_recipe in case of failure will apply again the artifact
         * combination to the item.
         * This leads to items with eg 100% resist, or more.
         * So use the actual item in the cauldron, don't retry the recipe.
         * This should fix bug #2020224: buggy(?) crafting yields.
         *
        if (!rp)
            if ((rp = get_random_recipe((recipelist *)NULL)) == NULL)
                return;
         */

        if ((tmp = cauldron->inv)) /*attempt_recipe(op, cauldron, 1, rp, -1, 0)))*/ {
            if (!QUERY_FLAG(tmp, FLAG_CURSED)) { /* curse it */
                SET_FLAG(tmp, FLAG_CURSED);
                CLEAR_FLAG(tmp, FLAG_KNOWN_CURSED);
                CLEAR_FLAG(tmp, FLAG_IDENTIFIED);
            }

            /* the apply code for potions already deals with cursed
             * potions, so any code here is basically ignored.
             */
            if (tmp->type == FOOD) {
                tmp->stats.hp = random_roll(0, 149, op, PREFER_LOW);
            }
            tmp->value = 0; /* unsaleable item */

            /* change stats downward */
            do {
                change_attr_value(&tmp->stats, rndm(0, 6), -1*(rndm(1, 3)));
            } while (rndm(0, 2));
        }
        return;
    }
#if 0
    /*
    Note: this does not work as expected...
    At this point there is only one item in the cauldron, and get_formulalist(0) will return
    the first formula list for recipes with 1 ingredient.
    So disable this, and just use the next case.
     */

    if (level == 40) {                   /* MAKE RANDOM RECIPE */
        recipelist *fl;
        int numb = numb_ob_inside(cauldron);

        fl = get_formulalist(numb-1); /* take a lower recipe list */
        if (fl && (rp = get_random_recipe(fl)))
            /* even though random, don't grant user any EXP for it */
            (void)attempt_recipe(op, cauldron, 1, rp, -1, 0);
        else
            alchemy_failure_effect(op, cauldron, rp, level-1);
        return;

    } else
#endif
        if (level < 45) {                 /* INFURIATE NPC's */
        /* this is kind of kludgy I know...*/
        object_set_enemy(cauldron, op);
        monster_npc_call_help(cauldron);
        object_set_enemy(cauldron, NULL);

        alchemy_failure_effect(op, cauldron, rp, level-5);
        return;
    } else if (level < 50) {                 /* MINOR EXPLOSION/FIREBALL */
        object *tmp;

        remove_contents(cauldron->inv, NULL);
        switch (rndm(0, 2)) {
        case 0:
            tmp = create_archetype("bomb");
            tmp->stats.dam = random_roll(1, level, op, PREFER_LOW);
            tmp->stats.hp = random_roll(1, level, op, PREFER_LOW);
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                                 "The %s creates a bomb!",
                                 cauldron->name);
            break;

        default:
            tmp = create_archetype("fireball");
            tmp->stats.dam = random_roll(1, level, op, PREFER_LOW)/5+1;
            tmp->stats.hp = random_roll(1, level, op, PREFER_LOW)/10+2;
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                                 "The %s erupts in flame!",
                                 cauldron->name);
            break;
        }
        object_insert_in_map_at(tmp, op->map, NULL, 0, cauldron->x, cauldron->y);
        return;
    } else if (level < 60) {                 /* CREATE MONSTER */
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                             "The %s %s.", cauldron->name, cauldron_sound());
        remove_contents(cauldron->inv, NULL);
        return;
    } else if (level < 80) {                 /* MAJOR FIRE */
        object *fb = create_archetype(SP_MED_FIREBALL);

        remove_contents(cauldron->inv, NULL);
        fire_arch_from_position(cauldron, cauldron, cauldron->x, cauldron->y, 0, fb);
        object_free_drop_inventory(fb);
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                             "The %s erupts in flame!",
                             cauldron->name);
        return;
    } else if (level < 100) {                /* WHAMMY the CAULDRON */
        if (!QUERY_FLAG(cauldron, FLAG_CURSED)) {
            SET_FLAG(cauldron, FLAG_CURSED);
            CLEAR_FLAG(cauldron, FLAG_KNOWN_CURSED);
            CLEAR_FLAG(cauldron, FLAG_IDENTIFIED);
        } else
            cauldron->magic--;
        cauldron->magic -= random_roll(0, 4, op, PREFER_LOW);
        if (rndm(0, 1)) {
            remove_contents(cauldron->inv, NULL);
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                                 "Your %s turns darker then makes a gulping sound!",
                                 cauldron->name);
        } else
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                                 "Your %s becomes darker.",
                                 cauldron->name);
        return;
    } else if (level < 110) {                /* SUMMON EVIL MONSTERS */
        object *tmp = get_random_mon(level/5);

        remove_contents(cauldron->inv, NULL);
        if (!tmp)
            alchemy_failure_effect(op, cauldron, rp, level);
        else if (summon_hostile_monsters(cauldron, random_roll(1, 10, op, PREFER_LOW), tmp->arch->name))
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                                 "The %s %s and then pours forth monsters!",
                                 cauldron->name, cauldron_sound());
        return;
    } else if (level < 150) {                /* COMBO EFFECT */
        int roll = rndm(1, 3);
        while (roll) {
            alchemy_failure_effect(op, cauldron, rp, level-39);
            roll--;
        }
        return;
    } else if (level == 151) {               /* CREATE RANDOM ARTIFACT */
        object *tmp;

        /* this is meant to be better than prior possiblity,
         * in this one, we allow *any *valid alchemy artifact
           * to be made (rather than only those on the given
         * formulalist) */
        if (!rp)
            rp = get_random_recipe((recipelist *)NULL);
        if (rp && (tmp = create_archetype(rp->arch_name[RANDOM()%rp->arch_names]))) {
            generate_artifact(tmp, random_roll(1, op->level/2+1, op, PREFER_HIGH)+1);
            if ((tmp = object_insert_in_ob(tmp, cauldron))) {
                remove_contents(cauldron->inv, tmp);
                draw_ext_info_format(NDI_UNIQUE, 0, op,
                                     MSG_TYPE_SKILL, MSG_TYPE_SKILL_SUCCESS,
                                     "The %s %s.",
                                     cauldron->name, cauldron_sound());
            }
        }
        return;
    } else {                /* MANA STORM - watch out!! */
        object *tmp = create_archetype(LOOSE_MANA);
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                      "You unwisely release potent forces!");
        remove_contents(cauldron->inv, NULL);
        cast_magic_storm(op, tmp, level);
        return;
    }
}

/**
 * All but object "save_item" are elimentated from
 * the container list. Note we have to becareful to remove the inventories
 * of objects in the cauldron inventory (ex icecube has stuff in it).
 * @param first_ob
 * container from which to remove.
 * @param save_item
 * what item to not remove. Can be NULL.
 */
static void remove_contents(object *first_ob, object *save_item) {
    object *tmp;

    tmp = first_ob;
    FOR_OB_AND_BELOW_PREPARE(tmp) {
        if (tmp != save_item) {
            if (tmp->inv)
                remove_contents(tmp->inv, NULL);
            object_remove(tmp);
            object_free_drop_inventory(tmp);
        }
    } FOR_OB_AND_BELOW_FINISH();
}

/**
 *"Danger" level, will determine how bad the backfire
 * could be if the user fails to concoct a recipe properly. Factors include
 * the number of ingredients, the magical nature of ingredients,
 * the user's effective level, the user's Int and the enchantment on the
 * mixing device (aka "cauldron"). Higher values of 'danger' indicate more
 * danger. Note that we assume that we have had the caster ready the alchemy
 * skill *before *this routine is called. (no longer auto-readies that skill)
 * -b.t.
 *
 * @param caster
 * who is trying alchemy.
 * @param cauldron
 * container used.
 * @param rp
 * recipe attempted.
 * @return
 * danger value.
 */
static int calc_alch_danger(object *caster, object *cauldron, const recipe *rp) {
    int danger = 0;

    /* Knowing alchemy skill reduces yer risk */
    danger -= caster->chosen_skill ? caster->chosen_skill->level : caster->level;

    /* better cauldrons reduce risk */
    danger -= cauldron->magic;

    /* Higher Int, lower the risk */
    danger -= 3*(caster->stats.Int-15);

    /* Ingredients. */
    FOR_INV_PREPARE(cauldron, item) {
        /* Nicolas Weeger: no reason why this is the case.
         danger += item->weight / 100;
         */
        danger++;
        if (QUERY_FLAG(item, FLAG_CURSED) || QUERY_FLAG(item, FLAG_DAMNED))
            danger += 5;
    } FOR_INV_FINISH();

    if (rp == NULL)
        danger += 110;
    else
        danger += rp->diff*3;

    /* Using a bad device is *majorly *stupid */
    if (QUERY_FLAG(cauldron, FLAG_CURSED))
        danger += 80;
    if (QUERY_FLAG(cauldron, FLAG_DAMNED))
        danger += 200;

#ifdef ALCHEMY_DEBUG
    LOG(llevDebug, "calc_alch_danger() returned danger=%d\n", danger);
#endif

    return danger;
}

/**
 * Determines if ingredients in a container match the
 * proper ingredients for a recipe.
 *
 * This functions tries to find each defined ingredient in the container. It is
 * the defined recipe iff
 *  - the number of ingredients of the recipe and in the container is equal
 *  - all ingredients of the recipe are found in the container
 *  - the number of batches is the same for all ingredients
 *
 * @param rp
 * recipe to check.
 * @param cauldron
 * container that holds the ingredients.
 * @param caster
 * who is trying to cast.
 * @return
 * 1 if the ingredients match the recipe, 0 if not.
 */
static int is_defined_recipe(const recipe *rp, const object *cauldron, object *caster) {
    uint32 batches_in_cauldron;
    const linked_char *ingredient;
    int number;

    /* check for matching number of ingredients */
    number = 0;
    for (ingredient = rp->ingred; ingredient != NULL; ingredient = ingredient->next)
        number++;
    FOR_INV_PREPARE(cauldron, ob)
        number--;
    FOR_INV_FINISH();
    if (number != 0)
        return 0;

    /* check for matching ingredients */
    batches_in_cauldron = 0;
    for (ingredient = rp->ingred; ingredient != NULL; ingredient = ingredient->next) {
        uint32 nrof;
        const char *name;
        int ok;

        /* determine and remove nrof from name */
        name = ingredient->name;
        nrof = 0;
        while (isdigit(*name)) {
            nrof = 10*nrof+(*name-'0');
            name++;
        }
        if (nrof == 0)
            nrof = 1;
        while (*name == ' ')
            name++;

        /* find the current ingredient in the cauldron */
        ok = 0;
        FOR_INV_PREPARE(cauldron, ob) {
            char name_ob[MAX_BUF];
            const char *name2;

            if (ob->title == NULL)
                name2 = ob->name;
            else {
                snprintf(name_ob, sizeof(name_ob), "%s %s", ob->name, ob->title);
                name2 = name_ob;
            }

            if (strcmp(name2, name) == 0) {
                if (ob->nrof%nrof == 0) {
                    uint32 batches;

                    batches = ob->nrof/nrof;
                    if (batches_in_cauldron == 0) {
                        batches_in_cauldron = batches;
                        ok = 1;
                    } else if (batches_in_cauldron == batches)
                        ok = 1;
                }
                break;
            }
        } FOR_INV_FINISH();
        if (!ok)
            return(0);
    }

    return(1);
}

/**
 * Find a recipe from a recipe list that matches the given formula. If there
 * is more than one matching recipe, it selects a random one. If at least one
 * transmuting recipe matches, it only considers matching transmuting recipes.
 *
 * @param fl
 * list containing the potential formulae based on the number of ingredients.
 * @param formula
 * hash of the ingredients.
 * @param ingredients
 * ingredients, linked through the 'below' field.
 * @return
 * one matching recipe, or NULL if no recipe matches
 */
static const recipe *find_recipe(const recipelist *fl, int formula, object *ingredients) {
    const recipe *rp;
    const recipe *result;       /* winning recipe, or NULL if no recipe found */
    int recipes_matching; /* total number of matching recipes so far */
    int transmute_found;  /* records whether a transmuting recipe was found so far */
    size_t rp_arch_index;

#ifdef EXTREME_ALCHEMY_DEBUG
    LOG(llevDebug, "looking for formula %d:\n", formula);
#endif
    result = NULL;
    recipes_matching = 0;
    transmute_found = 0;
    for (rp = fl->items; rp != NULL; rp = rp->next) {
        /* check if recipe matches at all */
        if (formula%rp->index != 0) {
#ifdef EXTREME_ALCHEMY_DEBUG
            LOG(llevDebug, " formula %s of %s (%d) does not match\n", rp->arch_name[0], rp->title, rp->index);
#endif
            continue;
        }

        if (rp->transmute && find_transmution_ob(ingredients, rp, &rp_arch_index, 0) != NULL) {
#ifdef EXTREME_ALCHEMY_DEBUG
            LOG(llevDebug, " formula %s of %s (%d) is a matching transmuting formula\n", rp->arch_name[rp_arch_index], rp->title, rp->index);
#endif
            /* transmution recipe with matching base ingredient */
            if (!transmute_found) {
                transmute_found = 1;
                recipes_matching = 0;
            }
        } else if (transmute_found) {
#ifdef EXTREME_ALCHEMY_DEBUG
            LOG(llevDebug, " formula %s of %s (%d) matches but is not a matching transmuting formula\n", rp->arch_name[0], rp->title, rp->index);
#endif
            /* "normal" recipe found after previous transmution recipe => ignore this recipe */
            continue;
        }
#ifdef EXTREME_ALCHEMY_DEBUG
        else {
            LOG(llevDebug, " formula %s of %s (%d) matches\n", rp->arch_name[0], rp->title, rp->index);
        }
#endif

        if (rndm(0, recipes_matching) == 0)
            result = rp;

        recipes_matching++;
    }

    if (result == NULL) {
#ifdef ALCHEMY_DEBUG
        LOG(llevDebug, "couldn't find formula for ingredients.\n");
#endif
        return NULL;
    }

#ifdef ALCHEMY_DEBUG
    if (strcmp(result->title, "NONE") != 0)
        LOG(llevDebug, "got formula: %s of %s (nbatches:%d)\n", result->arch_name[0], result->title, formula/result->index);
    else
        LOG(llevDebug, "got formula: %s (nbatches:%d)\n", result->arch_name[0], formula/result->index);
#endif
    return result;
}

/**
 * Handle use_skill for alchemy-like items.
 * @param op
 * player trying to do alchemy.
 * @return
 * 1 if any recipe was attempted, 0 else.
 * @note
 * Will inform player if attempting to use unpaid cauldron or ingredient.
 * @todo
 * check if no superflous message when 2 cauldrons on same spot, one unpaid? (shouldn't happen, but well).
 **/
int use_alchemy(object *op) {
    object *unpaid_cauldron = NULL;
    object *unpaid_item = NULL;
    int did_alchemy = 0;
    char name[MAX_BUF];

    if (QUERY_FLAG(op, FLAG_WIZ))
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM, "Note: alchemy in wizard-mode.\n");

    FOR_MAP_PREPARE(op->map, op->x, op->y, tmp) {
        if (QUERY_FLAG(tmp, FLAG_IS_CAULDRON)) {
            if (QUERY_FLAG(tmp, FLAG_UNPAID)) {
                unpaid_cauldron = tmp;
                continue;
            }
            unpaid_item = object_find_by_flag(tmp, FLAG_UNPAID);
            if (unpaid_item != NULL)
                continue;

            attempt_do_alchemy(op, tmp);
            if (QUERY_FLAG(tmp, FLAG_APPLIED))
                esrv_send_inventory(op, tmp);
            did_alchemy = 1;
        }
    } FOR_MAP_FINISH();
    if (unpaid_cauldron) {
        query_base_name(unpaid_cauldron, 0, name, MAX_BUF);
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                             "You must pay for your %s first!",
                             name);
    } else if (unpaid_item) {
        query_base_name(unpaid_item, 0, name, MAX_BUF);
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                             "You must pay for your %s first!",
                             name);
    }

    return did_alchemy;
}
