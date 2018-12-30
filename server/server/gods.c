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
 * All this functions handle gods: give presents, punish, and so on.
 *
 * Oct 3, 1995 - Code laid down for initial gods, priest alignment, and
 * monster race initialization. b.t.
 *
 * Sept 1996 - moved code over to object -oriented gods -b.t.
 */

#include <global.h>
#include <living.h>
#include <object.h>
#include <spells.h>
#include <sounds.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#endif

static int worship_forbids_use(object *op, object *exp_obj, uint32 flag, const char *string);
static void stop_using_item(object *op, int type, int number);
static void update_priest_flag(const object *god, object *exp_ob, uint32 flag);
static void god_intervention(object *op, const object *god, object *skill, object *altar);
static int god_examines_priest(object *op, const object *god);
static int god_examines_item(const object *god, object *item);
static const char *get_god_for_race(const char *race);
static void remove_special_prayers(object *op, const object *god);

/**
 * Returns the id of specified god.
 *
 * @param name
 * god to search for.
 * @return
 * identifier of god, -1 if not found.
 * @todo
 * couldn't == be used for comparison, if name is a shared string?
 */
static int lookup_god_by_name(const char *name) {
    int godnr = -1;
    size_t nmlen = strlen(name);

    if (name && strcmp(name, "none")) {
        godlink *gl;
        for (gl = first_god; gl; gl = gl->next)
            if (!strncmp(name, gl->name, MIN(strlen(gl->name), nmlen)))
                break;
        if (gl)
            godnr = gl->id;
    }
    return godnr;
}

/**
 * Returns pointer to specified god's object through pntr_to_god_obj().
 *
 * @note
 * returned object shouldn't be modified.
 *
 * @param name
 * god's name.
 * @return
 * pointer to god's object, NULL if doesn't match any god.
 */
const object *find_god(const char *name) {
    godlink *gl;

    if (!name)
        return NULL;

    for (gl = first_god; gl; gl = gl->next) {
        if (!strcmp(name, gl->name))
            return pntr_to_god_obj(gl);
    }

    return NULL;
}

/**
 * Determines if op worships a god.
 * Returns the godname if they do or "none" if they have no god.
 * In the case of an NPC, if they have no god, we try and guess
 * who they should worship based on their race. If that fails we
 * give them a random one.
 *
 * @param op
 * object to get name of.
 * @return
 * god name, "none" if nothing suitable.
 */
const char *determine_god(object *op) {
    int godnr = -1;
    const char *godname;

    /* spells */
    if ((op->type == SPELL || op->type == SPELL_EFFECT)
    && op->title) {
        if (lookup_god_by_name(op->title) >= 0)
            return op->title;
    }

    if (op->type != PLAYER && QUERY_FLAG(op, FLAG_ALIVE)) {
        /* find a god based on race */
        if (!op->title) {
            if (op->race != NULL) {
                godname = get_god_for_race(op->race);
                if (godname != NULL) {
                    op->title = add_string(godname);
                }
            }
        }

        /* find a random god */
        if (!op->title) {
            godlink *gl = first_god;

            godnr = rndm(1, gl->id);
            while (gl) {
                if (gl->id == godnr)
                    break;
                gl = gl->next;
            }
            op->title = add_string(gl->name);
        }

        return op->title;
    }

    /* The god the player worships is in the praying skill (native skill
     * not skill tool).  Since a player can only have one instance of
     * that skill, once we find it, we can return, either with the
     * title or "none".
     */
    if (op->type == PLAYER) {
        object *tmp;

        tmp = object_find_by_type_subtype(op, SKILL, SK_PRAYING);
        if (tmp != NULL) {
            return tmp->title != NULL ? tmp->title : "none";
        }
    }
    return ("none");
}

/**
 * Compares 2 strings.
 * @param s1
 * @param s2
 * strings to compare.
 * @return
 * 1 if s1 and s2 are the same - either both NULL, or strcmp( ) == 0.
 */
static int same_string(const char *s1, const char *s2) {
    if (s1 == NULL)
        return s2 == NULL;
    else
        return s2 != NULL && strcmp(s1, s2) == 0;
}

/**
 * Removes from a player's inventory all items bestowed by
 * a particular god.  Intended mainly for use in punishing
 * characters for switching gods.
 *
 * @param pl
 *     the player object
 *
 * @param op
 *     the object to be searched for items
 *
 * @param god
 *     the god whose objects to remove
 *
 */
static void follower_remove_given_items(object *pl, object *op, const object *god) {
    const char *given_by;

    /* search the inventory */
    FOR_INV_PREPARE(op, tmp) {
        given_by = object_get_value(tmp, "divine_giver_name");
        if (given_by == god->name) {
            char name[HUGE_BUF];

            query_short_name(tmp, name, HUGE_BUF);
            /* Send the client a message. */
            if (tmp->nrof > 1)
                draw_ext_info_format(NDI_UNIQUE, 0, pl, MSG_TYPE_ITEM, MSG_TYPE_ITEM_REMOVE,
                                     "The %s crumble to dust!",
                                     name);
            else
                draw_ext_info_format(NDI_UNIQUE, 0, pl, MSG_TYPE_ITEM, MSG_TYPE_ITEM_REMOVE,
                                     "The %s crumbles to dust!",
                                     name);

            object_remove(tmp);    /* remove obj from players inv. */
            object_free_drop_inventory(tmp);
        } else if (tmp->inv)
            follower_remove_given_items(pl, tmp, god);
    } FOR_INV_FINISH();
}

/**
 * Checks for any occurrence of the given 'item' in the inventory of 'op' (recursively).
 * @param op
 * object to check.
 * @param item
 * object to check for.
 * @return
 * 1 if found, else 0.
 */
static int follower_has_similar_item(object *op, object *item) {
    FOR_INV_PREPARE(op, tmp) {
        if (tmp->type == item->type
        && same_string(tmp->name, item->name)
        && same_string(tmp->title, item->title)
        && tmp->msg == item->msg
        && same_string(tmp->slaying, item->slaying))
            return 1;
        if (tmp->inv && follower_has_similar_item(tmp, item))
            return 1;
    } FOR_INV_FINISH();
    return 0;
}

/**
 * God gives an item to the player. Inform player of the present.
 *
 * @param op
 * who is getting the treasure.
 * @param god
 * god giving the present.
 * @param tr
 * object to give. Should be a single object on list.
 * @return
 * 0 if nothing was given, 1 else.
 */
static int god_gives_present(object *op, const object *god, treasure *tr) {
    object *tmp;
    char name[HUGE_BUF];

    if (follower_has_similar_item(op, &tr->item->clone))
        return 0;

    tmp = arch_to_object(tr->item);

    /*
     * Give inventory if needed, for instance Lythander's pipe.
     * Use high level and magic so something IS put in inventory.
     */
    fix_generated_item(tmp, NULL, 120, 120, GT_ONLY_GOOD);

    /* And just in case nothing was put and something is needed, bail out. */
    if ((tmp->type == ROD || tmp->type == WAND) && (tmp->inv == NULL)) {
        object_free2(tmp, 0);
        return 0;
    }

    query_short_name(tmp, name, HUGE_BUF);
    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_ITEM, MSG_TYPE_ITEM_ADD,
                         "%s lets %s appear in your hands.",
                         god->name, name);
    /**
     * Mark what god gave it, so it can be taken vengefully later!
     */
    object_set_value(tmp, "divine_giver_name", god->name, TRUE);
    object_insert_in_ob(tmp, op);
    return 1;
}

/**
 * Player prays at altar.
 * Checks for god changing, divine intervention, and so on.
 *
 * @param pl
 * player praying.
 * @param altar
 * altar player's praying on. Doesn't need to be consecrated.
 * @param skill
 * praying skill.
 */
void pray_at_altar(object *pl, object *altar, object *skill) {
    const object *pl_god = find_god(determine_god(pl));

    /* Lauwenmark: Handle for plugin altar-praying (apply) event */
    if (execute_event(altar, EVENT_APPLY, pl, NULL, NULL, SCRIPT_FIX_ALL) != 0)
        return;

    /* If non consecrate altar, don't do anything */
    if (!altar->other_arch)
        return;

    /* hmm. what happend depends on pl's current god, level, etc */
    if (!pl_god) { /*new convert */
        become_follower(pl, &altar->other_arch->clone);
        return;
    }

    if (!strcmp(pl_god->name, altar->other_arch->clone.name)) {
        /* pray at your gods altar */
        int bonus = (pl->stats.Wis+skill->level)/10;

        /* we can get neg grace up faster */
        if (pl->stats.grace < 0)
            pl->stats.grace += (bonus > -1*(pl->stats.grace/10) ? bonus : -1*(pl->stats.grace/10));
        /* we can super-charge grace to 2x max */
        if (pl->stats.grace < (2*pl->stats.maxgrace)) {
            pl->stats.grace += bonus/2;
        }
        if (pl->stats.grace > (2*pl->stats.maxgrace)) {
            pl->stats.grace = (2*pl->stats.maxgrace);
        }

        /* Every once in a while, the god decides to checkup on their
         * follower, and may intervene to help them out.
         */
        bonus = MAX(1, bonus+MAX(pl->stats.luck, -3)); /* -- DAMN -- */

        if (((random_roll(0, 399, pl, PREFER_LOW))-bonus) < 0)
            god_intervention(pl, pl_god, skill, altar);
    } else { /* praying to another god! */
        uint64 loss = 0;
        int angry = 1;

        /* I believe the logic for detecting opposing gods was completely
         * broken - I think it should work now.  altar->other_arch
         * points to the god of this altar (which we have
         * already verified is non null).  pl_god->other_arch
         * is the opposing god - we need to verify that exists before
         * using its values.
         */
        if (pl_god->other_arch
        && (altar->other_arch->name == pl_god->other_arch->name)) {
            angry = 2;
            if (random_roll(0, skill->level+2, pl, PREFER_LOW)-5 > 0) {
                object *tmp;

                /* you really screwed up */
                angry = 3;
                draw_ext_info_format(NDI_UNIQUE|NDI_NAVY, 0, pl, MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_GOD,
                                     "Foul Priest! %s punishes you!",
                                     pl_god->name);
                tmp = create_archetype(LOOSE_MANA);
                cast_magic_storm(pl, tmp, pl_god->level+20);
            } else
                draw_ext_info_format(NDI_UNIQUE|NDI_NAVY, 0, pl, MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_GOD,
                                     "Foolish heretic! %s is livid!",
                                     pl_god->name);
        } else
            draw_ext_info_format(NDI_UNIQUE|NDI_NAVY, 0, pl, MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_GOD,
                                 "Heretic! %s is angered!",
                                 pl_god->name);

        /* whether we will be successfull in defecting or not -
         * we lose experience from the clerical experience obj
         */

        loss = angry*(skill->stats.exp/10);
        if (loss)
            change_exp(pl, -random_roll64(0, loss, pl, PREFER_LOW), skill ? skill->skill : "none", SK_SUBTRACT_SKILL_EXP);

        /* May switch Gods, but its random chance based on our current level
         * note it gets harder to swap gods the higher we get
         */
        if ((angry == 1) && !(random_roll(0, skill->level, pl, PREFER_LOW))) {
            if (become_follower(pl, &altar->other_arch->clone))
                remove_special_prayers(pl, pl_god);
        } else {
            /* toss this player off the altar.  He can try again. */
            draw_ext_info(NDI_UNIQUE|NDI_NAVY, 0, pl, MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_GOD,
                          "A divine force pushes you off the altar.");

            move_player(pl, absdir(pl->facing+4)); /* back him off the way he came. */
        }
    }
}

/**
 * Removes special prayers given by a god.
 * After this function, op shouldn't know any prayer granted by god.
 * Prayers will be given when player prays on god's altar, so not handled now.
 *
 * @param op
 * player to remove prayers from.
 * @param god
 * god we're removing the prayers.
 */
static void remove_special_prayers(object *op, const object *god) {
    treasure *tr;
    int remove = 0;

    if (god->randomitems == NULL) {
        LOG(llevError, "BUG: remove_special_prayers(): god %s without randomitems\n", god->name);
        return;
    }


    /* Outer loop iterates over all special prayer marks */
    FOR_INV_PREPARE(op, tmp) {
        /* we mark special prayers with the STARTEQUIP flag, so if it isn't
         * in that category, not something we need to worry about.
         */
        if (tmp->type != SPELL || !QUERY_FLAG(tmp, FLAG_STARTEQUIP))
            continue;

         /* Inner loop tries to find the special prayer in the god's treasure
         * list. We default that the spell should not be removed.
        */
        remove = 0;
        for (tr = god->randomitems->items; tr; tr = tr->next) {
            if (tr->item == NULL)
                continue;

            /* Basically, see if the matching spell is granted by this god. */

            if (tr->item->clone.type == SPELL && tr->item->clone.name == tmp->name) {
                remove = 1;
                break;
            }
        }
        if (remove) {
            /* just do the work of removing the spell ourselves - we already
             * know that the player knows the spell
             */
            draw_ext_info_format(NDI_UNIQUE|NDI_NAVY, 0, op,
                                 MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_GOD,
                                 "You lose knowledge of %s.",
                                 tmp->name);
            player_unready_range_ob(op->contr, tmp);
            object_remove(tmp);
            object_free_drop_inventory(tmp);
        }
    } FOR_INV_FINISH();
}

/**
 * This function is called whenever a player has
 * switched to a new god. It handles basically all the stat changes
 * that happen to the player, including the removal of godgiven
 * items (from the former cult).
 * Handles race restrictions on god, and will punish player if needed.
 *
 * @param op
 * player switching cults.
 * @param new_god
 * new god to worship.
 * @return
 * 1 if successfully converted, 0 if the god doesn't like the race, or something else.
 * @todo isn't there duplication with remove_special_prayers() for spell removing?
 * @todo split the check to make this function only actually become follower
 */
int become_follower(object *op, const object *new_god) {
    const object *old_god = NULL;                      /* old god */
    treasure *tr;
    object *skop;
    int i, sk_applied;
    int undeadified = 0; /* Turns to true if changing god can changes the undead
                          * status of the player.*/
    old_god = find_god(determine_god(op));

    /* take away any special god-characteristic items. */
    FOR_INV_PREPARE(op, item) {
        /* remove all invisible startequip items which are
         *  not skill, exp or force
          */
        if (QUERY_FLAG(item, FLAG_STARTEQUIP)
        && item->invisible
        && (item->type != SKILL)
        && (item->type != FORCE)
        && (item->type != SPELL)) {
            player_unready_range_ob(op->contr, item);
            object_remove(item);
            object_free_drop_inventory(item);
        }
    } FOR_INV_FINISH();

    /* remove any items given by the old god */
    if (old_god) {
        /* Changed to use the new "divine_giver_name" key_value
         *   so it can reliably delete enchanted items.  Now it loops
         *   through the player's inventory, instead of the god's
         *   treasure list.
         */
        follower_remove_given_items(op, op, old_god);
    }

    if (!op || !new_god)
        return 0;

    if (op->race && new_god->slaying && strstr(op->race, new_god->slaying)) {
        draw_ext_info_format(NDI_UNIQUE|NDI_NAVY, 0, op, MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_GOD,
                             "Fool! %s detests your kind!",
                             new_god->name);
        if (random_roll(0, op->level-1, op, PREFER_LOW)-5 > 0) {
            object *tmp = create_archetype(LOOSE_MANA);
            cast_magic_storm(op, tmp, new_god->level+10);
        }
        return 0;
    }

    /* give the player any special god-characteristic-items. */
    for (tr = new_god->randomitems->items; tr != NULL; tr = tr->next) {
        if (tr->item
        && tr->item->clone.invisible
        && tr->item->clone.type != SPELLBOOK
        && tr->item->clone.type != BOOK
        && tr->item->clone.type != SPELL)
            god_gives_present(op, new_god, tr);
    }

    draw_ext_info_format(NDI_UNIQUE|NDI_NAVY, 0, op, MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_GOD,
                         "You become a follower of %s!",
                         new_god->name);

    skop = object_find_by_type_subtype(op, SKILL, SK_PRAYING);
    /* Player has no skill - give them the skill */
    if (!skop) {
        /* The archetype should always be defined - if we crash here because it doesn't,
         * things are really messed up anyways.
         */
        skop = give_skill_by_name(op, get_archetype_by_type_subtype(SKILL, SK_PRAYING)->clone.skill);
        link_player_skills(op);
    }

    sk_applied = QUERY_FLAG(skop, FLAG_APPLIED); /* save skill status */

    /* Clear the "undead" status. We also need to force a call to change_abil,
     * so I set undeadified for that.
     * - gros, 21th July 2006.
     */
    if ((old_god) && (QUERY_FLAG(old_god, FLAG_UNDEAD))) {
        CLEAR_FLAG(skop, FLAG_UNDEAD);
        undeadified = 1;
    }

    if (skop->title) { /* get rid of old god */
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_GOD,
                             "%s's blessing is withdrawn from you.",
                             skop->title);

        /* The point of this is to really show what abilities the player just lost */
        if (sk_applied || undeadified) {
            CLEAR_FLAG(skop, FLAG_APPLIED);
            (void)change_abil(op, skop);
        }
        free_string(skop->title);
    }

    /* now change to the new gods attributes to exp_obj */
    skop->title = add_string(new_god->name);
    skop->path_attuned = new_god->path_attuned;
    skop->path_repelled = new_god->path_repelled;
    skop->path_denied = new_god->path_denied;
    /* copy god's resistances */
    memcpy(skop->resist, new_god->resist, sizeof(new_god->resist));

    /* make sure that certain immunities do NOT get passed
     * to the follower!
     */
    for (i = 0; i < NROFATTACKS; i++)
        if (skop->resist[i] > 30
        && (i == ATNR_FIRE || i == ATNR_COLD || i == ATNR_ELECTRICITY || i == ATNR_POISON))
            skop->resist[i] = 30;

    skop->stats.hp = (sint16)new_god->last_heal;
    skop->stats.sp = (sint16)new_god->last_sp;
    skop->stats.grace = (sint16)new_god->last_grace;
    skop->stats.food = (sint16)new_god->last_eat;
    skop->stats.luck = (sint8)new_god->stats.luck;
    /* gods may pass on certain flag properties */
    update_priest_flag(new_god, skop, FLAG_SEE_IN_DARK);
    update_priest_flag(new_god, skop, FLAG_REFL_SPELL);
    update_priest_flag(new_god, skop, FLAG_REFL_MISSILE);
    update_priest_flag(new_god, skop, FLAG_STEALTH);
    update_priest_flag(new_god, skop, FLAG_MAKE_INVIS);
    update_priest_flag(new_god, skop, FLAG_UNDEAD);
    update_priest_flag(new_god, skop, FLAG_BLIND);
    update_priest_flag(new_god, skop, FLAG_XRAYS); /* better have this if blind! */
    update_priest_flag(new_god, skop, FLAG_USE_WEAPON);
    update_priest_flag(new_god, skop, FLAG_USE_ARMOUR);
    update_priest_flag(new_god, skop, FLAG_USE_SHIELD);

    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_GOD,
                         "You are bathed in %s's aura.",
                         new_god->name);

    /* Weapon/armour use are special...handle flag toggles here as this can
     * only happen when gods are worshipped and if the new priest could
     * have used armour/weapons in the first place.
     *
     * This also can happen for monks which cannot use weapons. In this case
     * do not allow to use weapons even if the god otherwise would allow it.
     */
    if (!object_present_in_ob_by_name(FORCE, "no weapon force", op)) {
        if (worship_forbids_use(op, skop, FLAG_USE_WEAPON, "weapons"))
            stop_using_item(op, WEAPON, 2);
    }
    update_priest_flag(new_god, skop, FLAG_USE_ARMOUR);

    if (worship_forbids_use(op, skop, FLAG_USE_ARMOUR, "armour")) {
        stop_using_item(op, ARMOUR, 1);
        stop_using_item(op, HELMET, 1);
        stop_using_item(op, BOOTS, 1);
        stop_using_item(op, GLOVES, 1);
    }

    if (worship_forbids_use(op, skop, FLAG_USE_SHIELD, "shield"))
        stop_using_item(op, SHIELD, 1);

    SET_FLAG(skop, FLAG_APPLIED);
    (void)change_abil(op, skop);

    /* return to previous skill status */
    if (!sk_applied)
        CLEAR_FLAG(skop, FLAG_APPLIED);

    // check is now done after converting
    //remove_special_prayers(op, new_god);

    return 1;
}

/**
 * Forbids or let player use something item type.
 * @param op
 * player.
 * @param exp_obj
 * praying skill.
 * @param flag
 * FLAG_xxx to check against.
 * @param string
 * what flag corresponds to ("weapons", "shield", ...).
 * @return
 * 1 if player was changed, 0 if no change.
 */
static int worship_forbids_use(object *op, object *exp_obj, uint32 flag, const char *string) {
    if (QUERY_FLAG(&op->arch->clone, flag)) {
        if (QUERY_FLAG(op, flag) != QUERY_FLAG(exp_obj, flag)) {
            update_priest_flag(exp_obj, op, flag);
            if (QUERY_FLAG(op, flag))
                draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_GOD,
                                     "You may use %s again.",
                                     string);
            else {
                draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_GOD,
                                     "You are forbidden to use %s.",
                                     string);
                return 1;
            }
        }
    }
    return 0;
}

/**
 * Unapplies up to number worth of items of type type, ignoring curse status.
 * This is used when the player gets forbidden to use eg weapons.
 *
 * @param op
 * player we're considering.
 * @param type
 * item type to remove.
 * @param number
 * maximum number of items to unapply. Must be positive.
 */
static void stop_using_item(object *op, int type, int number) {
    FOR_INV_PREPARE(op, tmp)
        if (tmp->type == type && QUERY_FLAG(tmp, FLAG_APPLIED)) {
            apply_special(op, tmp, AP_UNAPPLY|AP_IGNORE_CURSE);
            number--;
            if (number <= 0)
                break;
        }
    FOR_INV_FINISH();
}

/**
 * If the god does/doesnt have this flag, we
 * give/remove it from the experience object if it doesnt/does
 * already exist.
 *
 * @param god
 * god object.
 * @param exp_ob
 * player's praying skill object.
 * @param flag
 * flag to consider.
 */
static void update_priest_flag(const object *god, object *exp_ob, uint32 flag) {
    if (QUERY_FLAG(god, flag) && !QUERY_FLAG(exp_ob, flag))
        SET_FLAG(exp_ob, flag);
    else if (QUERY_FLAG(exp_ob, flag) && !QUERY_FLAG(god, flag)) {
        /*  When this is called with the exp_ob set to the player,
         * this check is broken, because most all players arch
         * allow use of weapons.  I'm not actually sure why this
         * check is here - I guess if you had a case where the
         * value in the archetype (wisdom) should over ride the restrictions
         * the god places on it, this may make sense.  But I don't think
         * there is any case like that.
         */

/*      if (!(QUERY_FLAG(&(exp_ob->arch->clone), flag)))*/
        CLEAR_FLAG(exp_ob, flag);
    }
}


/**
 * Determines the archetype for holy servant and god avatar.
 *
 * Possible monsters are stored as invisible books in god's inventory,
 * one having the right name is selected randomly.
 *
 * @param god
 * god for which we want something.
 * @param type
 * what the summon type is. Must be a shared string.
 * @return
 * random archetype matching the type, NULL if none found.
 */
archetype *determine_holy_arch(const object *god, const char *type) {
    treasure *tr;
    int count;
    object *item;

    if (!god || !god->randomitems) {
        LOG(llevError, "BUG: determine_holy_arch(): no god or god without randomitems\n");
        return NULL;
    }

    count = 0;
    for (tr = god->randomitems->items; tr != NULL; tr = tr->next) {
        if (!tr->item)
            continue;
        item = &tr->item->clone;
        if (item->type == BOOK && item->invisible && item->name == type)
            count++;
    }
    if (count == 0) {
        return NULL;
    }

    count = rndm(1, count);

    for (tr = god->randomitems->items; tr != NULL; tr = tr->next) {
        if (!tr->item)
            continue;
        item = &tr->item->clone;
        if (item->type == BOOK && item->invisible && item->name == type) {
            count--;
            if (count == 0)
                return item->other_arch;
        }
    }

    return NULL;
}

/**
 * God helps player by removing curse and/or damnation.
 *
 * @param op
 * player to help.
 * @param remove_damnation
 * if set, also removes damned items.
 * @return
 * 1 if at least one item was uncursed, 0 else.
 */
static int god_removes_curse(object *op, int remove_damnation) {
    int success = 0;

    FOR_INV_PREPARE(op, tmp) {
        if (tmp->invisible)
            continue;
        if (QUERY_FLAG(tmp, FLAG_DAMNED) && !remove_damnation)
            continue;
        if (QUERY_FLAG(tmp, FLAG_CURSED) || QUERY_FLAG(tmp, FLAG_DAMNED)) {
            success = 1;
            CLEAR_FLAG(tmp, FLAG_DAMNED);
            CLEAR_FLAG(tmp, FLAG_CURSED);
            CLEAR_FLAG(tmp, FLAG_KNOWN_CURSED);
            if (op->type == PLAYER)
                esrv_update_item(UPD_FLAGS, op, tmp);
        }
    } FOR_INV_FINISH();

    if (success)
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_PRAY,
                      "You feel like someone is helping you.");
    return success;
}

/**
 * Converts a level and difficulty to a magic/enchantment value for eg weapons.
 * @param level
 * level
 * @param difficulty
 * difficulty. Must be 1 or more.
 * @return
 * number of enchantments for level and difficulty.
 */
static int follower_level_to_enchantments(int level, int difficulty) {
    if (difficulty < 1) {
        LOG(llevError, "follower_level_to_enchantments(): difficulty %d is invalid\n", difficulty);
        return 0;
    }

    if (level <= 20)
        return level/difficulty;
    if (level <= 40)
        return (20+(level-20)/2)/difficulty;
    return (30+(level-40)/4)/difficulty;
}

/**
 * Utility function for improving the magic on a weapon.
 * Affected weapon is the applied one (weapon or bow). This utility function
 * improves the weapon magic on a weapon being enchanted by a god. This was
 * necessary because the same block of the code was being called from two
 * places in the god_enchants_weapon(...) function.
 *
 * @param op
 * player
 * @param tr
 * treasure list item for enchanting weapon, contains the enchantment level.
 * @param weapon
 * weapon being modified
 * @param skill
 * praying skill of op.
 * @return
 * 0 if weapon wasn't changed, 1 if changed.
 */
static int improve_weapon_magic(object *op, object *tr, object *weapon, object *skill) {
    int tmp = follower_level_to_enchantments(skill->level, tr->level);

    if (weapon->magic < tmp) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ITEM, MSG_TYPE_ITEM_CHANGE,
                      "A phosphorescent glow envelops your weapon!");
        weapon->magic++;
        if (op->type == PLAYER)
            esrv_update_item(UPD_NAME, op, weapon);
        weapon->item_power++;
        return 1;
    }

    return 0;
}

/**
 * God wants to enchant weapon.
 * Affected weapon is the applied one (weapon or bow). It's checked to make sure
 * it isn't a weapon for another god. If all is all right, update weapon with
 * attacktype, slaying and such.
 *
 * @param op
 * player
 * @param god
 * god enchanting weapon.
 * @param tr
 * treasure list item for enchanting weapon, contains the enchantment level.
 * @param skill
 * praying skill of op.
 * @return
 * 0 if weapon wasn't changed, 1 if changed.
 */
static int god_enchants_weapon(object *op, const object *god, object *tr, object *skill) {
    char buf[MAX_BUF];
    object *weapon;
    uint32 attacktype;

    weapon = object_find_by_type_applied(op, WEAPON);
    if (weapon == NULL)
        weapon = object_find_by_type_applied(op, BOW);
    if (weapon == NULL || god_examines_item(god, weapon) <= 0)
        return 0;

    if (weapon->item_power >= MAX_WEAPON_ITEM_POWER) {
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_ITEM, MSG_TYPE_ITEM_INFO,
                             "%s considers your %s is not worthy to be enchanted any more.",
                             god->name,
                             weapon->name);
        return 0;
    }

    /* If personalized_blessings is activated, then the god's name is
     * associated with the weapon, as well as the owner (the one who blesses it),
     * and a "weapon willpower", which is equivalent to the owner's experience
     * in praying when the weapon is blessed.
     * Those values are used later, when another player attempts to wield the
     * weapon - nasty things may happen to those who do not deserve to use it ! :)
     */
    if (settings.personalized_blessings) {
        const char *divine_owner = object_get_value(weapon, "divine_blessing_name");
        const char *owner = object_get_value(weapon, "item_owner");
        object *skillop = NULL;

        if (divine_owner != NULL && strcmp(divine_owner, god->name) != 0) {
            /* Huho... Another god already blessed this one ! */
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_ITEM, MSG_TYPE_ITEM_INFO,
                                 "Your %s already belongs to %s !",
                                 weapon->name, divine_owner);
            return 0;
        }

        if ((owner != NULL) && (strcmp(owner, op->name) != 0)) {
            /* Maybe the weapon itself will not agree ? */
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_ITEM, MSG_TYPE_ITEM_INFO,
                                 "The %s is not yours, and is magically protected against such changes !",
                                 weapon->name);
            return 0;
        }
        skillop = find_skill_by_number(op, SK_PRAYING);
        if (skillop == NULL) {
            LOG(llevError, "god_enchants_weapon: no praying skill object found ?!\n");
            snprintf(buf, sizeof(buf), "%d", 1);
        } else
            snprintf(buf, sizeof(buf), "%"FMT64, skillop->stats.exp);
        object_set_value(weapon, "divine_blessing_name", god->name, TRUE);
        object_set_value(weapon, "item_owner", op->name, TRUE);
        object_set_value(weapon, "item_willpower", buf, TRUE);
    }

    /* First give it a title, so other gods won't touch it */
    if (!weapon->title) {
        snprintf(buf, sizeof(buf), "of %s", god->name);
        weapon->title = add_string(buf);
        if (op->type == PLAYER)
            esrv_update_item(UPD_NAME, op, weapon);
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ITEM, MSG_TYPE_ITEM_CHANGE,
                      "Your weapon quivers as if struck!");
    }

    /* Allow the weapon to slay enemies */
    if (!weapon->slaying && god->slaying) {
        weapon->slaying = add_string(god->slaying);
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_ITEM, MSG_TYPE_ITEM_CHANGE,
                             "Your %s now hungers to slay enemies of your god!",
                             weapon->name);
        weapon->item_power++;
        return 1;
    }

    /* Add the gods attacktype */
    attacktype = (weapon->attacktype == 0) ? AT_PHYSICAL : weapon->attacktype;
    if ((attacktype&god->attacktype) != god->attacktype) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ITEM, MSG_TYPE_ITEM_CHANGE,
                      "Your weapon suddenly glows!");
        weapon->attacktype = attacktype|god->attacktype;
        weapon->item_power++;
        return 1;
    }

    /* Higher magic value */
    return improve_weapon_magic(op, tr, weapon, skill);
}

/**
 * Every once in a while the god will intervene to help the worshiper.
 * Later, this fctn can be used to supply quests, etc for the
 * priest. -b.t.
 * called from pray_at_altar() currently.
 *
 * @param op
 * player praying.
 * @param god
 * god player is praying to.
 * @param skill
 * player's praying skill.
 * @param altar
 * where the player is praying.
 */
static void god_intervention(object *op, const object *god, object *skill, object *altar) {
    treasure *tr;

    if (!god || !god->randomitems) {
        LOG(llevError, "BUG: god_intervention(): no god or god without randomitems\n");
        return;
    }

    // removed on 2009-12-12 because the function now removes prayers NOT from god.
    //remove_special_prayers(op, god);

    /* lets do some checks of whether we are kosher with our god */
    if (god_examines_priest(op, god) < 0)
        return;

    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_PRAY,
                  "You feel a holy presence!");

    if (altar->anim_suffix != NULL)
        apply_anim_suffix(altar, altar->anim_suffix);

    for (tr = god->randomitems->items; tr != NULL; tr = tr->next) {
        object *item;

        if (tr->chance <= random_roll(0, 99, op, PREFER_HIGH))
            continue;

        /* Treasurelist - generate some treasure for the follower */
        if (tr->name) {
            treasurelist *tl = find_treasurelist(tr->name);
            if (tl == NULL)
                continue;

            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ITEM, MSG_TYPE_ITEM_ADD,
                          "Something appears before your eyes.  You catch it before it falls to the ground.");

            create_treasure(tl, op, GT_STARTEQUIP|GT_ONLY_GOOD|GT_UPDATE_INV, skill->level, 0);
            return;
        }

        if (!tr->item) {
            LOG(llevError, "BUG: empty entry in %s's treasure list\n", god->name);
            continue;
        }
        item = &tr->item->clone;

        /* Grace limit */
        if (item->type == BOOK && item->invisible
        && strcmp(item->name, "grace limit") == 0) {
            if (op->stats.grace < item->stats.grace
            || op->stats.grace < op->stats.maxgrace) {
                object *tmp;

                /* Follower lacks the required grace for the following
                 * treasure list items. */

                tmp = create_archetype(HOLY_POSSESSION);
                cast_change_ability(op, op, tmp, 0, 1);
                object_free_drop_inventory(tmp);
                return;
            }
            continue;
        }

        /* Restore grace */
        if (item->type == BOOK && item->invisible
        && strcmp(item->name, "restore grace") == 0) {
            if (op->stats.grace >= 0)
                continue;
            op->stats.grace = random_roll(0, 9, op, PREFER_HIGH);
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_PRAY,
                          "You are returned to a state of grace.");
            return;
        }

        /* Heal damage */
        if (item->type == BOOK && item->invisible
            && strcmp(item->name, "restore hitpoints") == 0) {
            if (op->stats.hp >= op->stats.maxhp)
                continue;
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_PRAY,
                          "A white light surrounds and heals you!");
            op->stats.hp = op->stats.maxhp;
            return;
        }

        /* Restore spellpoints */
        if (item->type == BOOK
        && item->invisible
        && strcmp(item->name, "restore spellpoints") == 0) {
            int max = op->stats.maxsp*(item->stats.maxsp/100.0);
            /* Restore to 50 .. 100%, if sp < 50% */
            int new_sp = random_roll(1000, 1999, op, PREFER_HIGH)/2000.0*max;
            if (op->stats.sp >= max/2)
                continue;
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_PRAY,
                          "A blue lightning strikes your head but doesn't hurt you!");
            op->stats.sp = new_sp;
        }

        /* Various heal spells */
        if (item->type == BOOK && item->invisible
            && strcmp(item->name, "heal spell") == 0) {
            object *tmp;
            int success;

            tmp = create_archetype_by_object_name(item->slaying);

            success = cast_heal(op, op, tmp, 0);
            object_free_drop_inventory(tmp);
            if (success)
                return;

            continue;
        }

        /* Remove curse */
        if (item->type == BOOK && item->invisible
            && strcmp(item->name, "remove curse") == 0) {
            if (god_removes_curse(op, 0))
                return;

            continue;
        }

        /* Remove damnation */
        if (item->type == BOOK && item->invisible
            && strcmp(item->name, "remove damnation") == 0) {
            if (god_removes_curse(op, 1))
                return;

            continue;
        }

        /* Heal depletion */
        if (item->type == BOOK && item->invisible
            && strcmp(item->name, "heal depletion") == 0) {
            object *depl;
            archetype *at;
            int i;

            if ((at = find_archetype(ARCH_DEPLETION)) == NULL) {
                LOG(llevError, "Could not find archetype depletion.\n");
                continue;
            }
            depl = arch_present_in_ob(at, op);
            if (depl == NULL)
                continue;
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_PRAY,
                          "Shimmering light surrounds and restores you!");
            for (i = 0; i < NUM_STATS; i++)
                if (get_attr_value(&depl->stats, i))
                    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ATTRIBUTE,
                                  MSG_TYPE_ATTRIBUTE_BAD_EFFECT_END,
                                  restore_msg[i]);
            object_remove(depl);
            object_free_drop_inventory(depl);
            fix_object(op);
            return;
        }

        /* Voices */
        if (item->type == BOOK && item->invisible
            && strcmp(item->name, "voice_behind") == 0) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_PRAY,
                          "You hear a voice from behind you, but you don't dare to "
                          "turn around:");
            draw_ext_info(NDI_WHITE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_PRAY,
                          item->msg);
            return;
        }

        /* Messages */
        if (item->type == BOOK && item->invisible
            && strcmp(item->name, "message") == 0) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_PRAY,
                          item->msg);
            return;
        }

        /* Enchant weapon */
        if (item->type == BOOK && item->invisible
            && strcmp(item->name, "enchant weapon") == 0) {
            if (god_enchants_weapon(op, god, item, skill))
                return;

            continue;
        }

        /* Spellbooks - works correctly only for prayers */
        if (item->type == SPELL) {
            if (check_spell_known(op, item->name))
                continue;
            if (item->level > skill->level)
                continue;

            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_PRAY,
                                 "%s grants you use of a special prayer!",
                                 god->name);
            do_learn_spell(op, item, 1);
            return;
        }

        /* Other gifts */
        if (!item->invisible) {
            if (god_gives_present(op, god, tr))
                return;

            continue;
        }
        /* else ignore it */
    }

    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_PRAY,
                  "You feel rapture.");
}

/**
 * Checks and maybe punishes someone praying.
 * All applied items are examined, if player is using more items of other gods,
 * s/he loses experience in praying or general experience if no praying.
 *
 * @param op
 * player the god examines.
 * @param god
 * god examining the player.
 * @return
 * negative value is god is not pleased, else positive value, the higher the better.
 */
static int god_examines_priest(object *op, const object *god) {
    int reaction = 1;
    object *skop;

    FOR_INV_PREPARE(op, item)
        if (QUERY_FLAG(item, FLAG_APPLIED)) {
            reaction += god_examines_item(god, item)*(item->magic ? abs(item->magic) : 1);
        }
    FOR_INV_FINISH();

    /* well, well. Looks like we screwed up. Time for god's revenge */
    if (reaction < 0) {
        int loss = 10000000;
        int angry = abs(reaction);

        skop = object_find_by_type_subtype(op, SKILL, SK_PRAYING);
        if (skop)
            loss = 0.05*(float)skop->stats.exp;
        change_exp(op, -random_roll(0, loss*angry-1, op, PREFER_LOW), skop ? skop->skill : "none", SK_SUBTRACT_SKILL_EXP);
        if (random_roll(0, angry, op, PREFER_LOW)) {
            object *tmp = create_archetype(LOOSE_MANA);

            cast_magic_storm(op, tmp, op->level+(angry*3));
        }
        draw_ext_info_format(NDI_UNIQUE|NDI_NAVY, 0, op, MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_GOD,
                             "%s becomes angry and punishes you!",
                             god->name);
    }
    return reaction;
}

/**
 * God checks item the player is using.
 * If you are using the item of an enemy
 * god, it can be bad...-b.t.
 *
 * @param god
 * god checking.
 * @param item
 * item to check.
 * @retval -1
 * item is bad.
 * @retval 0
 * item is neutral.
 * @retval 1
 * item is good.
 */
static int god_examines_item(const object *god, object *item) {
    char buf[MAX_BUF];

    if (!god || !item)
        return 0;

    if (!item->title)
        return 1; /* unclaimed item are ok */

    snprintf(buf, sizeof(buf), "of %s", god->name);
    if (!strcmp(item->title, buf))
        return 1; /* belongs to that God */

    if (god->title) { /* check if we have any enemy blessed item*/
        snprintf(buf, sizeof(buf), "of %s", god->title);
        if (!strcmp(item->title, buf)) {
            if (item->env) {
                char name[MAX_BUF];

                query_name(item, name, MAX_BUF);
                draw_ext_info_format(NDI_UNIQUE|NDI_NAVY, 0, item->env, MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_GOD,
                                     "Heretic! You are using %s!",
                                     name);
            }
            return -1;
        }
    }
    return 0; /* item is sacred to a non-enemy god/or is otherwise magical */
}

/**
 * Returns a string that is the name of the god that should be natively worshipped by a
 * creature of who has race *race
 * if we can't find a god that is appropriate, we return NULL
 *
 * @param race
 * race we're getting the god of.
 * @return
 * NULL if no matching race, else god's name.
 */
static const char *get_god_for_race(const char *race) {
    godlink *gl = first_god;
    const char *godname = NULL;

    if (race == NULL)
        return NULL;
    while (gl) {
        if (gl->arch->clone.race && !strcasecmp(gl->arch->clone.race, race)) {
            godname = gl->name;
            break;
        }
        gl = gl->next;
    }
    return godname;
}

/**
 * Changes the attributes of cone, smite, and ball spells as needed by the code.
 *
 * @param spellop
 * spell object to change.
 * @param caster
 * what is casting spellop (player, spell, ...).
 * @return
 * 0 if there was no race to assign to the slaying field of the spell, but
 * the spell attacktype contains AT_HOLYWORD, 1 else.
 */
int tailor_god_spell(object *spellop, object *caster) {
    const object *god = find_god(determine_god(caster));
    int caster_is_spell = 0;

    if (caster->type == SPELL_EFFECT || caster->type == SPELL)
        caster_is_spell = 1;

    /* if caster is a rune or the like, it doesn't worship anything.  However,
     * if this object is owned by someone, then the god that they worship
     * is relevant, so use that.
     */
    if (!god && object_get_owner(caster))
        god = find_god(determine_god(object_get_owner(caster)));

    if (!god || (spellop->attacktype&AT_HOLYWORD && !god->race)) {
        if (!caster_is_spell)
            draw_ext_info(NDI_UNIQUE, 0, caster, MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_GOD,
                          "This prayer is useless unless you worship an appropriate god");
        else
            LOG(llevError, "BUG: tailor_god_spell(): no god\n");
        object_free_drop_inventory(spellop);
        return 0;
    }

    /* either holy word or godpower attacks will set the slaying field */
    if (spellop->attacktype&AT_HOLYWORD || spellop->attacktype&AT_GODPOWER) {
        if (spellop->slaying) {
            free_string(spellop->slaying);
            spellop->slaying = NULL;
        }
        if (!caster_is_spell)
            spellop->slaying = god->slaying ? add_string(god->slaying) : NULL;
        else if (caster->slaying)
            spellop->slaying = add_string(caster->slaying);
    }

    /* only the godpower attacktype adds the god's attack onto the spell */
    if (spellop->attacktype&AT_GODPOWER)
        spellop->attacktype = spellop->attacktype|god->attacktype;

    /* tack on the god's name to the spell */
    if (spellop->attacktype&AT_HOLYWORD || spellop->attacktype&AT_GODPOWER) {
        if (spellop->title)
            free_string(spellop->title);
        spellop->title = add_string(god->name);
        if (spellop->title) {
            char buf[MAX_BUF];

            snprintf(buf, sizeof(buf), "%s of %s", spellop->name, spellop->title);
            FREE_AND_COPY(spellop->name, buf);
            FREE_AND_COPY(spellop->name_pl, buf);
        }
    }

    return 1;
}
