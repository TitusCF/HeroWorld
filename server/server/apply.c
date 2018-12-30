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
 * Handles objects being applied, and their effect.
 */

#include <global.h>
#include <living.h>
#include <spells.h>
#include <skills.h>
#include <tod.h>

#ifndef __CEXTRACT__
#include <sproto.h>
#endif

/* Want this regardless of rplay. */
#include <sounds.h>

/* need math lib for double-precision and pow() in dragon_eat_flesh() */
#include <math.h>

static int apply_check_apply_restrictions(object *who, object *op, int aflags);
static int apply_check_personalized_blessings(object *who, const object *op);
static int apply_check_item_power(const object *who, const object *op, int aflags);
static int apply_check_owner(const object *who, const object *op, int aflags);
static void apply_update_ranged_skill(const object *who, object *op, int aflags);

/**
 * Can transport hold object op?
 * This is a pretty trivial function,
 * but in the future, possible transport may have more restrictions
 * or weight reduction like containers
 *
 * @param transport
 * transport to check.
 * @param op
 * object we're trying to insert.
 * @param nrof
 * number of op.
 * @return
 * 1 if can hold, 0 else.
 */
int transport_can_hold(const object *transport, const object *op, int nrof) {
    return op->weight*nrof+transport->carrying <= transport->weight_limit;
}

/**
 * Check if op should abort moving victim because of it's race or slaying.
 *
 * @param op
 * detector or equivalent we're testing. Note that its type is not checked.
 * @param victim
 * object trying to move on op.
 * @return
 * 1 if it should abort, 0 if it should continue.
 */
int should_director_abort(const object *op, const object *victim) {
    int arch_flag, name_flag, race_flag;

    /* Never move doors, it messes things. */
    if (victim->type == DOOR)
        return 1;

    /* Get flags to determine what of arch, name, and race should be
     * checked. This is stored in subtype, and is a bitmask, the LSB
     * is the arch flag, the next is the name flag, and the last is
     * the race flag. Also note, if subtype is set to zero, that also
     * goes to defaults of all affecting it. Examples:
     * subtype 1: only arch
     * subtype 3: arch or name
     * subtype 5: arch or race
     * subtype 7: all three
     */
    if (op->subtype) {
        arch_flag = op->subtype&1;
        name_flag = op->subtype&2;
        race_flag = op->subtype&4;
    } else {
        arch_flag = 1;
        name_flag = 1;
        race_flag = 1;
    }
    /* If the director has race set, only affect objects with a arch,
     * name or race that matches.
     */
    if (op->race
    && (!(victim->arch && arch_flag && victim->arch->name) || strcmp(op->race, victim->arch->name))
    && (!(victim->name && name_flag) || strcmp(op->race, victim->name))
    && (!(victim->race && race_flag) || strcmp(op->race, victim->race)))
        return 1;

    /* If the director has slaying set, only affect objects where none
     * of arch, name, or race match.
     */
    if (op->slaying
    && ((victim->arch && arch_flag && victim->arch->name && !strcmp(op->slaying, victim->arch->name))
        || (victim->name && name_flag && !strcmp(op->slaying, victim->name))
        || (victim->race && race_flag && !strcmp(op->slaying, victim->race))))
        return 1;

    return 0;
}

/**
 * This checks whether the object has a "on_use_yield" field,
 * and if so generated and drops matching item.
 *
 * @param tmp
 * item that was applied.
 *
 * @note
 * handle_apply_yield() has been renamed to apply_handle_yield()
 */
void apply_handle_yield(object *tmp) {
    const char *yield;

    yield = object_get_value(tmp, "on_use_yield");
    if (yield != NULL) {
        object *drop = create_archetype(yield);
        if (tmp->env)
            drop = object_insert_in_ob(drop, tmp->env);
        else
            object_insert_in_map_at(drop, tmp->map, tmp, INS_BELOW_ORIGINATOR, tmp->x, tmp->y);
    }
}

/**
 * Makes an object's face the main face, which is supposed to be the "closed" one.
 *
 * Sets an object's face to the 'face' in the archetype.
 * Meant for showing containers opening and closing.
 *
 * @param op
 * Object to set face on
 *
 * @return TRUE if face changed
 */
int set_object_face_main(object *op) {
    int newface = op->arch->clone.face->number;
    sstring saved = object_get_value(op, "face_closed");

    if (saved)
        newface = find_face(saved, newface);
    if (newface && op->face != &new_faces[newface]) {
        op->face = &new_faces[newface];
        return TRUE;
    }
    return FALSE;
}

/**
 * Makes an object's face the other_arch face, supposed to be the "opened" one.
 *
 * Sets an object's face to the other_arch 'face'.
 * Meant for showing containers opening and closing.
 *
 * @param op
 * Object to set face on
 *
 * @return TRUE if face changed
 */
static int set_object_face_other(object *op) {
    sstring custom;
    int newface = 0;

    if (op->face && op->other_arch && op->other_arch->clone.face)
        newface = op->other_arch->clone.face->number;

    if (op->face != op->arch->clone.face) {
        /* object has a custom face, save it so it gets correctly restored later. */
        object_set_value(op, "face_closed", op->face->name, 1);
    }

    custom = object_get_value(op, "face_opened");
    if (custom)
        newface = find_face(custom, newface);
    if (newface && op->face->number != newface) {
        op->face = &new_faces[newface];
        return TRUE;
    }
    return FALSE;
}

/**
 * Handle apply on containers.  This is for
 * containers that are applied by a player, whether in inventory or
 * on the ground: eg, sacks, luggage, etc.
 *
 * Moved to own function and added many features [Tero.Haatanen(at)lut.fi]
 * This version is for client/server mode.
 *
 * Reminder - there are three states for any container - closed (non applied),
 * applied (not open, but objects that match get tossed into it), and open
 * (applied flag set, and op->container points to the open container)
 *
 * @param op
 * player.
 * @param sack
 * container the player is opening or closing.
 * @return
 * 1 if an object is applied somehow or another, 0 if error/no apply
 *
 * @author Eneq(at)(csd.uu.se)
 */
int apply_container(object *op, object *sack) {
    char name_sack[MAX_BUF], name_tmp[MAX_BUF];
    object *tmp = op->container;

    if (op->type != PLAYER)
        return 0; /* This might change */

    if (sack == NULL || sack->type != CONTAINER) {
        LOG(llevError, "apply_container: %s is not container!\n", sack ? sack->name : "NULL");
        return 0;
    }

    /* If we have a currently open container, then it needs
     * to be closed in all cases if we are opening this one up.
     * We then fall through if appropriate for opening the new
     * container.
     */
    if (op->container && QUERY_FLAG(op->container, FLAG_APPLIED)) {
        tag_t tmp_tag = op->container->count;

        if (op->container->env != op) { /* if container is on the ground */
            op->container->move_off = 0;
        }

        /* Query name before the close event, as the container could be destroyed. */
        query_name(op->container, name_tmp, MAX_BUF);

        /* Lauwenmark: Handle for plugin close event */
        if (execute_event(tmp, EVENT_CLOSE, op, NULL, NULL, SCRIPT_FIX_ALL) != 0)
            return 1;

        draw_ext_info_format(NDI_UNIQUE, 0, op,
                             MSG_TYPE_APPLY, MSG_TYPE_APPLY_UNAPPLY,
                             "You close %s.",
                             name_tmp);

        op->container = NULL;
        if (op->contr != NULL)
            op->contr->socket.container_position = 0;

        /* The container may have been destroyed by the event handler. */
        if (!object_was_destroyed(tmp, tmp_tag)) {
            CLEAR_FLAG(tmp, FLAG_APPLIED);
            if (set_object_face_main(tmp))
                esrv_update_item(UPD_FLAGS|UPD_FACE, op, tmp);
            else
                esrv_update_item(UPD_FLAGS, op, tmp);
        }
        if (tmp == sack)
            return 1;
    }

    query_name(sack, name_sack, MAX_BUF);

    /* If the player is trying to open it (which he must be doing
     * if we got here), and it is locked, check to see if player
     * has the equipment to open it.
     */

    if (sack->slaying) { /* it's locked */
        tmp = find_key(op, op, sack);
        if (tmp) {
            query_name(tmp, name_tmp, MAX_BUF);
            draw_ext_info_format(NDI_UNIQUE, 0, op,
                                 MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
                                 "You unlock %s with %s.",
                                 name_sack, name_tmp);
        } else {
            draw_ext_info_format(NDI_UNIQUE, 0, op,
                                 MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
                                 "You don't have the key to unlock %s.",
                                 name_sack);
            return 0;
        }
    }

    /* By the time we get here, we have made sure any other container
     * has been closed and if this is a locked container, the player
     * has the key to open it.
     */

    /* There are really two cases - the sack is either on the ground,
     * or the sack is part of the player's inventory.  If on the ground,
     * we assume that the player is opening it, since if it was being
     * closed, that would have been taken care of above.
     */


    if (sack->env != op) {
        /* Hypothetical case - the player is trying to open a sack
         * that belongs to someone else.  This normally should not
         * happen, but a misbehaving client/player could
         * try to do it, so let's handle it gracefully.
         */
        if (sack->env) {
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
                                 "You can't open %s",
                                 name_sack);
            return 0;
        }

        if (sack->nrof > 1) {
            object *left = object_split(sack, sack->nrof-1, NULL, 0);

            object_insert_in_map_at(left, sack->map, NULL, INS_NO_MERGE, sack->x, sack->y);
            /* recompute the name so it's nice */
            query_name(sack, name_sack, MAX_BUF);
        }

        /* set it so when the player walks off, we can unapply the sack */
        sack->move_off = MOVE_ALL;      /* trying force closing it */

        CLEAR_FLAG(sack, FLAG_APPLIED);
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
                             "You open %s.",
                             name_sack);
        SET_FLAG(sack, FLAG_APPLIED);
        op->container = sack;
        if (op->contr != NULL)
            op->contr->socket.container_position = 0;

        if (set_object_face_other(sack))
            esrv_update_item(UPD_FLAGS|UPD_FACE, op, sack);
        else
            esrv_update_item(UPD_FLAGS, op, sack);
        esrv_send_inventory(op, sack);
    } else { /* sack is in players inventory */
        if (QUERY_FLAG(sack, FLAG_APPLIED)) {  /* readied sack becoming open */
            CLEAR_FLAG(sack, FLAG_APPLIED);
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
                                 "You open %s.",
                                 name_sack);
            SET_FLAG(sack, FLAG_APPLIED);
            op->container = sack;
            if (op->contr != NULL)
                op->contr->socket.container_position = 0;

            if (set_object_face_other(sack))
                esrv_update_item(UPD_FLAGS|UPD_FACE, op, sack);
            else
                esrv_update_item(UPD_FLAGS, op, sack);
            esrv_send_inventory(op, sack);
        } else {
            object *left = NULL;

            if (sack->nrof > 1)
                left = object_split(sack, sack->nrof-1, NULL, 1);

            CLEAR_FLAG(sack, FLAG_APPLIED);
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
                                 "You readied %s.",
                                 name_sack);
            SET_FLAG(sack, FLAG_APPLIED);
            esrv_update_item(UPD_FLAGS, op, sack);

            if (left) {
                object_insert_in_ob(left, sack->env);
                esrv_send_item(op, left);
            }
        }
    }
    return 1;
}

/**
 * Actually makes op learn spell.
 * Informs player of new spell and binding.
 *
 * @param op
 * player who'll learn the spell.
 * @param spell
 * spell to learn.
 * @param special_prayer
 * 1 for god-given prayer, 0 else.
 */
void do_learn_spell(object *op, object *spell, int special_prayer) {
    object *tmp;

    if (op->type != PLAYER) {
        LOG(llevError, "BUG: do_learn_spell(): not a player\n");
        return;
    }

    /* Upgrade special prayers to normal prayers */
    tmp = check_spell_known(op, spell->name);
    if (tmp != NULL) {
        if (special_prayer && !QUERY_FLAG(tmp, FLAG_STARTEQUIP)) {
            LOG(llevError, "BUG: do_learn_spell(): spell already known, but not marked as startequip\n");
            return;
        }
        return;
    }

    play_sound_player_only(op->contr, SOUND_TYPE_SPELL, spell, 0, "learn");
    tmp = object_new();
    object_copy(spell, tmp);
    object_insert_in_ob(tmp, op);

    if (special_prayer)
        SET_FLAG(tmp, FLAG_STARTEQUIP);

    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
                         "Type 'bind cast %s to store the spell in a key.",
                         spell->name);

    esrv_add_spells(op->contr, tmp);
}

/**
 * Erases spell from player's inventory. Will inform player of loss.
 *
 * @param op
 * player.
 * @param spell
 * spell name to forget.
 */
void do_forget_spell(object *op, const char *spell) {
    object *spob;

    if (op->type != PLAYER) {
        LOG(llevError, "BUG: do_forget_spell(): not a player\n");
        return;
    }
    spob = check_spell_known(op, spell);
    if (spob == NULL) {
        LOG(llevError, "BUG: do_forget_spell(): spell not known\n");
        return;
    }

    draw_ext_info_format(NDI_UNIQUE|NDI_NAVY, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_CURSED,
                         "You lose knowledge of %s.",
                         spell);
    player_unready_range_ob(op->contr, spob);
    esrv_remove_spell(op->contr, spob);
    object_remove(spob);
    object_free2(spob, 0);
}

/**
 * Checks if an item is restricted to a race. Non players and DMs can always apply.
 *
 * @param who
 * living thing trying to apply an item.
 * @param item
 * item being applied.
 * @return
 * 0 if item can't be applied, 1 else.
 *
 * @note
 * check_race_restrictions() has been renamed to apply_check_race_restrictions()
 */
static int apply_check_race_restrictions(object *who, object *item) {
    char buf[MAX_BUF];
    sstring restriction;

    if (who->type != PLAYER || QUERY_FLAG(who, FLAG_WIZ))
        return 1;

    restriction = object_get_value(item, "race_restriction");
    if (!restriction)
        return 1;

    snprintf(buf, sizeof(buf), ":%s:", who->race);
    buf[sizeof(buf)-1] = '\0';

    if (strstr(restriction, buf) != NULL)
        return 1;

    query_name(item, buf, sizeof(buf));
    draw_ext_info_format(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_PROHIBITION, "Somehow you can't seem to use the %s.", buf);

    return 0;
}

/**
 * Main apply handler.
 *
 * Checks for unpaid items before applying.
 *
 * @param op
 * ::object causing tmp to be applied.
 * @param tmp
 * ::object being applied.
 * @param aflag
 * special (always apply/unapply) flags.  Nothing is done with
 * them in this function - they are passed to apply_special().
 * @return
 * - 0: player or monster can't apply objects of that type
 * - 1: has been applied, or there was an error applying the object
 * - 2: objects of that type can't be applied if not in inventory
 *
 * @note
 * manual_apply() has been renamed to apply_manual()
 */
int apply_manual(object *op, object *tmp, int aflag) {
    tmp = HEAD(tmp);

    if (QUERY_FLAG(tmp, FLAG_UNPAID) && !QUERY_FLAG(tmp, FLAG_APPLIED)) {
        if (op->type == PLAYER) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
                          "You should pay for it first.");
            return METHOD_SILENT_ERROR;
        }
        return 0;   /* monsters just skip unpaid items */
    }

    if (!apply_check_race_restrictions(op, tmp))
        return METHOD_SILENT_ERROR;

    /* Lauwenmark: Handle for plugin apply event */
    if (execute_event(tmp, EVENT_APPLY, op, NULL, NULL, SCRIPT_FIX_ALL) != 0)
        return METHOD_OK;

    if (op->contr)
        play_sound_player_only(op->contr, SOUND_TYPE_ITEM, tmp, 0, "apply");

    return ob_apply(tmp, op, aflag);
}

/**
 * Living thing is applying an object.
 *
 * @param pl
 * ::object causing op to be applied.
 * @param op
 * ::object being applied.
 * @param aflag
 * special (always apply/unapply) flags.  Nothing is done with
 * them in this function - they are passed to apply_special().
 * @param quiet
 * if 1, suppresses the "don't know how to apply" and "you must get it first"
 * messages as needed by apply_by_living_below().  There can still be
 * "but you are floating high above the ground" messages.
 * @return
 * - 0: player or monster can't apply objects of that type
 * - 1: has been applied, or there was an error applying the object
 * - 2: objects of that type can't be applied if not in inventory
 *
 * @note
 * player_apply() has been renamed to apply_by_living()
 */
int apply_by_living(object *pl, object *op, int aflag, int quiet) {
    int tmp;

    if (op->env == NULL && (pl->move_type&MOVE_FLYING)) {
        /* player is flying and applying object not in inventory */
        if (!QUERY_FLAG(pl, FLAG_WIZ) && !(op->move_type&MOVE_FLYING)) {
            draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
                          "But you are floating high above the ground!");
            return 0;
        }
    }

    /* Check for PLAYER to avoid a DM to disappear in a puff of smoke if
     * applied.
     */
    if (op->type != PLAYER
    && QUERY_FLAG(op, FLAG_WAS_WIZ)
    && !QUERY_FLAG(pl, FLAG_WAS_WIZ)) {
        play_sound_map(SOUND_TYPE_ITEM, op, 0, "evaporate");
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
                      "The object disappears in a puff of smoke!");
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
                      "It must have been an illusion.");
        object_remove(op);
        object_free2(op, 0);
        return 1;
    }

    tmp = apply_manual(pl, op, aflag);
    if (!quiet) {
        if (tmp == METHOD_UNHANDLED) {
            char name[MAX_BUF];

            query_name(op, name, MAX_BUF);
            draw_ext_info_format(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
                                 "I don't know how to apply the %s.",
                                 name);
        } else if (tmp == METHOD_ERROR)
            draw_ext_info_format(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
                                 "You must get it first!\n");
        else if (tmp == METHOD_SILENT_ERROR)
            return tmp;
    }
    if (tmp == METHOD_OK) {
        if (op->anim_suffix != NULL)
            apply_anim_suffix(pl, op->anim_suffix);
    }
    return tmp;
}

/**
 * Attempt to apply the object 'below' the player.
 * If the player has an open container, we use that for below, otherwise
 * we use the ground.
 *
 * @param pl
 * player.
 *
 * @note
 * player_apply_below() has been renamed to apply_by_living_below()
 */
void apply_by_living_below(object *pl) {
    object *tmp;
    int floors;

    if (pl->contr->transport && pl->contr->transport->type == TRANSPORT) {
        ob_apply(pl->contr->transport, pl, 0);
        return;
    }

    /* If using a container, set the starting item to be the top
     * item in the container.  Otherwise, use the map.
     */
    tmp = pl->container != NULL ? pl->container->inv : pl->below;

    /* This is perhaps more complicated.  However, I want to make sure that
     * we don't use a corrupt pointer for the next object, so we get the
     * next object in the stack before applying.  This is can only be a
     * problem if apply_by_living() has a bug in that it uses the object but
     * does not return a proper value.
     */
    floors = 0;
    FOR_OB_AND_BELOW_PREPARE(tmp) {
        if (QUERY_FLAG(tmp, FLAG_IS_FLOOR))
            floors++;
        else if (floors > 0)
            return;   /* process only floor objects after first floor object */

        /* If it is visible, player can apply it.  If it is applied by
         * person moving on it, also activate.  Added code to make it
         * so that at least one of players movement types be that which
         * the item needs.
         */
        if (!tmp->invisible || (tmp->move_on&pl->move_type)) {
            if (apply_by_living(pl, tmp, 0, 1) == METHOD_OK)
                return;
        }
        if (floors >= 2)
            return;   /* process at most two floor objects */
    } FOR_OB_AND_BELOW_FINISH();
}

/**
 * Unapplies specified item.
 * No check done on cursed/damned.
 * Break this out of apply_special() - this is just done
 * to keep the size of apply_special() to a more manageable size.
 *
 * @param who
 * living that has op removed.
 * @param op
 * ::object.
 * @param aflags
 * combination of @ref AP_xxx "AP_xxx" flags.
 * @return
 * 0.
 */
static int unapply_special(object *who, object *op, int aflags) {
    char name[MAX_BUF];

    if (op->type != LAMP)
        CLEAR_FLAG(op, FLAG_APPLIED);
    query_name(op, name, MAX_BUF);
    switch (op->type) {
    case WEAPON:
        if (!(aflags&AP_NOPRINT))
            draw_ext_info_format(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_UNAPPLY,
                                 "You unwield %s.",
                                 name);
        (void)change_abil(who, op);
        CLEAR_FLAG(who, FLAG_READY_WEAPON);
        who->current_weapon = NULL;
        clear_skill(who);
        break;

    case SKILL:         /* allows objects to impart skills */
    case SKILL_TOOL:
        if (op != who->chosen_skill)
            LOG(llevError, "BUG: unapply_special(): applied skill is not a chosen skill\n");
        if (who->type == PLAYER) {
            if (who->contr->shoottype == range_skill)
                who->contr->shoottype = range_none;
            if (!op->invisible) {
                if (!(aflags&AP_NOPRINT))
                    draw_ext_info_format(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_UNAPPLY,
                                         "You stop using the %s.",
                                         name);
            } else {
                if (!(aflags&AP_NOPRINT))
                    draw_ext_info_format(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_UNAPPLY,
                                         "You can no longer use the skill: %s.",
                                         op->skill);
            }
        }
        (void)change_abil(who, op);
        who->chosen_skill = NULL;
        CLEAR_FLAG(who, FLAG_READY_SKILL);
        break;

    case ARMOUR:
    case HELMET:
    case SHIELD:
    case RING:
    case BOOTS:
    case GLOVES:
    case AMULET:
    case GIRDLE:
    case BRACERS:
    case CLOAK:
        if (!(aflags&AP_NOPRINT))
            draw_ext_info_format(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_UNAPPLY,
                                 "You unwear %s.",
                                 name);
        (void)change_abil(who, op);
        break;

    case BOW:
    case WAND:
    case ROD:
        clear_skill(who);
        if (!(aflags&AP_NOPRINT))
            draw_ext_info_format(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_UNAPPLY,
                                 "You unready %s.",
                                 name);
        if (who->type == PLAYER)
            who->contr->shoottype = range_none;
        else if (op->type == BOW)
            CLEAR_FLAG(who, FLAG_READY_BOW);
        else
            CLEAR_FLAG(who, FLAG_READY_RANGE);
        break;

    case BUILDER:
        if (!(aflags&AP_NOPRINT))
            draw_ext_info_format(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_UNAPPLY,
                                 "You unready %s.",
                                 name);
        who->contr->shoottype = range_none;
        who->contr->ranges[range_builder] = NULL;
        break;

    default:
        if (!(aflags&AP_NOPRINT))
            draw_ext_info_format(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_UNAPPLY,
                                 "You unapply %s.",
                                 name);
        break;
    }

    fix_object(who);

    if (!(aflags&AP_NO_MERGE)) {
        object *tmp;

        tmp = object_merge(op, NULL);
        if (who->type == PLAYER) {
            if (tmp) {  /* it was merged */
                op = tmp;
            }
            esrv_update_item(UPD_FLAGS, who, op);
        }
    }
    return 0;
}

/**
 * Returns the object that is using body location 'loc'.
 * Note that 'start' is the first object to start examine - we
 * then go through the below of this.  In this way, you can do
 * something like:
 * tmp = get_item_from_body_location(who->inv, 1);
 * if (tmp) tmp1 = get_item_from_body_location(tmp->below, 1);
 * to find the second object that may use this location, etc.
 *
 * Don't return invisible objects unless they are skill objects.
 * Invisible other objects that use up body locations can be used as restrictions.
 *
 * @param start
 * object to start from.
 * @param loc
 * body position to search. Must be between 0 and NUM_BODY_LOCATIONS-1.
 * @return
 * object at position, NULL if none.
 */
static object *get_item_from_body_location(object *start, int loc) {
    object *tmp;

    if (!start)
        return NULL;

    tmp = start;
    FOR_OB_AND_BELOW_PREPARE(tmp)
        if (QUERY_FLAG(tmp, FLAG_APPLIED)
        && tmp->body_info[loc]
        && (!tmp->invisible || tmp->type == SKILL))
            return tmp;
    FOR_OB_AND_BELOW_FINISH();
    return NULL;
}

/**
 * Remove equipment so an object can be applied.
 *
 * This should only be called when it is known
 * that there are objects to unapply.  This makes pretty heavy
 * use of get_item_from_body_location().  It makes no intelligent choice
 * on objects - rather, the first that matched is used.
 *
 * if aflags has AP_PRINT set, we instead print out what to unapply
 * instead of doing it.  This is a lot less code than having
 * another function that does just that.
 *
 * @param who
 * living trying to apply op.
 * @param op
 * ::object being applied.
 * @param aflags
 * combination of @ref AP_xxx "AP_xxx" flags.
 * @return
 * 0 on success, 1 if there is some problem.
 */
static int unapply_for_ob(object *who, object *op, int aflags) {
    int i;
    object *tmp = NULL, *last;
    char name[MAX_BUF];

    /* If we are applying a shield or weapon, unapply any equipped shield
     * or weapons first - only allowed to use one weapon/shield at a time.
     */
    if (op->type == WEAPON || op->type == SHIELD) {
        FOR_INV_PREPARE(who, tmp) {
            if (QUERY_FLAG(tmp, FLAG_APPLIED) && tmp->type == op->type) {
                if (!(aflags&AP_IGNORE_CURSE)
                && !(aflags&AP_PRINT)
                && (QUERY_FLAG(tmp, FLAG_CURSED) || QUERY_FLAG(tmp, FLAG_DAMNED))) {
                    /* In this case, we want to try and remove a
                     * cursed item. While we know it won't work, we
                     * want unapply_special to at least generate the
                     * message.
                     */
                    if (!(aflags&AP_NOPRINT)) {
                        query_name(tmp, name, MAX_BUF);
                        draw_ext_info_format(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_UNAPPLY,
                                             "No matter how hard you try, you just can't remove %s.",
                                             name);
                    }
                    return 1;
                }

                if (aflags&AP_PRINT) {
                    query_name(tmp, name, MAX_BUF);
                    draw_ext_info(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_UNAPPLY,
                                  name);
                } else
                    unapply_special(who, tmp, aflags);
            }
        } FOR_INV_FINISH();
    }

    for (i = 0; i < NUM_BODY_LOCATIONS; i++) {
        /* this used up a slot that we need to free */
        if (op->body_info[i]) {
            last = who->inv;

            /* We do a while loop - may need to remove several items
             * in order to free up enough slots.
             */
            while (who->body_used[i]+op->body_info[i] < 0) {
                tmp = get_item_from_body_location(last, i);
                if (!tmp)
                    return 1;

                /* If just printing, we don't care about cursed status */
                if ((aflags&AP_IGNORE_CURSE)
                || (aflags&AP_PRINT)
                || (!(QUERY_FLAG(tmp, FLAG_CURSED) || QUERY_FLAG(tmp, FLAG_DAMNED)))) {
                    if (aflags&AP_PRINT) {
                        query_name(tmp, name, MAX_BUF);
                        draw_ext_info(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_UNAPPLY,
                                      name);
                    } else
                        unapply_special(who, tmp, aflags);
                } else {
                    /* Cursed item that we can't unequip - tell the player.
                     * Note this could be annoying if this is just one of a
                     * few, so it may not be critical (eg, putting on a
                     * ring and you have one cursed ring.)
                     */
                    if (!(aflags&AP_NOPRINT)) {
                        query_name(tmp, name, MAX_BUF);
                        draw_ext_info_format(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_UNAPPLY,
                                             "The %s just won't come off",
                                             name);
                    }
                }
                last = tmp->below;
            }
            /* if we got here, this slot is freed up - otherwise, if it
             * wasn't freed up, the return in the !tmp would have
             * kicked in.
             */
        } /* if op is using this body location */
    } /* for body locations */
    return 0;
}

/**
 * Checks to see if 'who' can apply object 'op'.
 *
 * @param who
 * living thing trying to apply op.
 * @param op
 * object applied.
 * @return
 * 0 if apply can be done without anything special.
 * Otherwise returns a bitmask of @ref CAN_APPLY_xxx "CAN_APPLY_xxx" - potentially several of these may be
 * set, but largely depends on circumstance - in the future, processing
 * may be  pruned once we know some status (eg, once CAN_APPLY_NEVER
 * is set, do we really are what the other flags may be?)
 * See include/define.h for detailed description of the meaning of
 * these return values.
 *
 * @note
 * can_apply_object() has been renamed to apply_can_apply_object()
 */
int apply_can_apply_object(const object *who, const object *op) {
    int i, retval = 0;
    object *tmp = NULL, *ws = NULL;

    /* Players have 2 'arm's, so they could in theory equip 2 shields or
     * 2 weapons, but we don't want to let them do that.  So if they are
     * trying to equip a weapon or shield, see if they already have one
     * in place and store that way.
     */
    if (op->type == WEAPON || op->type == SHIELD) {
        tmp = object_find_by_type_applied(who, op->type);
        if (tmp != NULL) {
            retval = CAN_APPLY_UNAPPLY;
            ws = tmp;
        }
    }

    for (i = 0; i < NUM_BODY_LOCATIONS; i++) {
        if (op->body_info[i]) {
            /* Item uses more slots than we have */
            if (FABS(op->body_info[i]) > who->body_info[i]) {
                /* Could return now for efficiently - rest of info
                 * below isn't really needed.
                 */
                retval |= CAN_APPLY_NEVER;
            } else if (who->body_used[i]+op->body_info[i] < 0) {
                /* in this case, equipping this would use more free
                 * spots than we have.
                 */
                object *tmp1;

                /* if we have an applied weapon/shield, and unapply
                 * it would free enough slots to equip the new item,
                 * then just set this can continue.  We don't care
                 * about the logic below - if you have shield equipped
                 * and try to equip another shield, there is only one
                 * choice.  However, the check for the number of body
                 * locations does take into the account cases where what
                 * is being applied may be two handed for example.
                 */
                if (ws) {
                    if (who->body_used[i]-ws->body_info[i]+op->body_info[i] >= 0) {
                        retval |= CAN_APPLY_UNAPPLY;
                        continue;
                    }
                }

                tmp1 = get_item_from_body_location(who->inv, i);
                if (!tmp1)
                    retval |= CAN_APPLY_NEVER;
                else {
                    /* need to unapply something.  However, if this
                     * something is different than we had found before,
                     * it means they need to apply multiple objects
                     */
                    retval |= CAN_APPLY_UNAPPLY;
                    if (!tmp)
                        tmp = tmp1;
                    else if (tmp != tmp1)
                        retval |= CAN_APPLY_UNAPPLY_MULT;
                    /* This object isn't using up all the slots, so
                     * there must be another.  If so, and if the new
                     * item doesn't need all the slots, the player
                     * then has a choice.
                     */
                    if (who->body_used[i]-tmp1->body_info[i] != who->body_info[i]
                    && FABS(op->body_info[i]) < who->body_info[i])
                        retval |= CAN_APPLY_UNAPPLY_CHOICE;

                    /* Does unequipping 'tmp1' free up enough slots
                     * for this to be equipped?  If not, there must
                     * be something else to unapply.
                     */
                    if (who->body_used[i]+op->body_info[i]-tmp1->body_info[i] < 0)
                        retval |= CAN_APPLY_UNAPPLY_MULT;
                }
            } /* if not enough free slots */
        } /* if this object uses location i */
    } /* for i -> num_body_locations loop */

    /* Do checks for can_use_weapon/shield/armour. */
    if (IS_WEAPON(op) && !QUERY_FLAG(who, FLAG_USE_WEAPON))
        retval |= CAN_APPLY_RESTRICTION;
    if (IS_SHIELD(op) && !QUERY_FLAG(who, FLAG_USE_SHIELD))
        retval |= CAN_APPLY_RESTRICTION;
    if (IS_ARMOR(op) && !QUERY_FLAG(who, FLAG_USE_ARMOUR))
        retval |= CAN_APPLY_RESTRICTION;

    if (who->type != PLAYER) {
        if ((op->type == WAND || op->type == ROD)
        && !QUERY_FLAG(who, FLAG_USE_RANGE))
            retval |= CAN_APPLY_RESTRICTION;
        if (op->type == BOW && !QUERY_FLAG(who, FLAG_USE_BOW))
            retval |= CAN_APPLY_RESTRICTION;
        if (op->type == RING && !QUERY_FLAG(who, FLAG_USE_RING))
            retval |= CAN_APPLY_RESTRICTION;
    }
    return retval;
}

/**
 * This checks to see of the player (who) is sufficient level to use a weapon
 * with improves improvements (typically last_eat).  We take an int here
 * instead of the object so that the improvement code can pass along the
 * increased value to see if the object is usable.
 * we return 1 (true) if the player can use the weapon.
 * See ../types/weapon_improver/weapon_improver.c
 *
 * @param who
 * living to check
 * @param improves
 * improvement level.
 * @return
 * 1 if who can use the item, 0 else.
 */
int apply_check_weapon_power(const object *who, int improves) {
    return (who->level/5)+5 >= improves;
}

/**
 * Apply an object.
 *
 * This function doesn't check for unpaid items, but check other restrictions.
 *
 * Usage example:  apply_special (who, op, AP_UNAPPLY | AP_IGNORE_CURSE)
 *
 * @param who
 * object using op. It can be a monster.
 * @param op
 * object being used. Should be an equipment type item,
 * eg, one which you put on and keep on for a while, and not something
 * like a potion or scroll.
 * @param aflags
 * combination of @ref AP_xxx "AP_xxx" flags.
 * @return
 * 1 if the action could not be completed, 0 on success.
 * However, success is a matter of meaning - if the
 * user passes the 'apply' flag to an object already applied,
 * nothing is done, and 0 is returned.
 */
int apply_special(object *who, object *op, int aflags) {
    int basic_flag = aflags&AP_BASIC_FLAGS;
    object *tmp, *skop;
    char name_op[MAX_BUF];

    if (who == NULL) {
        LOG(llevError, "apply_special() from object without environment.\n");
        return 1;
    }

    if (op->env != who)
        return 1;   /* op is not in inventory */

    /* trying to unequip op */
    if (QUERY_FLAG(op, FLAG_APPLIED)) {
        /* always apply, so no reason to unapply */
        if (basic_flag == AP_APPLY)
            return 0;

        if (!(aflags&AP_IGNORE_CURSE)
        && (QUERY_FLAG(op, FLAG_CURSED) || QUERY_FLAG(op, FLAG_DAMNED))) {
            if (!(aflags&AP_NOPRINT)) {
                query_name(op, name_op, MAX_BUF);
                draw_ext_info_format(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_UNAPPLY,
                                     "No matter how hard you try, you just can't remove %s.",
                                     name_op);
            }
            return 1;
        }
        return unapply_special(who, op, aflags);
    }

    if (basic_flag == AP_UNAPPLY)
        return 0;

    if (!apply_check_apply_restrictions(who, op, aflags))
        return 1;

    if (op->skill && op->type != SKILL && op->type != SKILL_TOOL) {
        skop = find_skill_by_name(who, op->skill);
        if (!skop) {
            if (!(aflags&AP_NOPRINT))
                draw_ext_info_format(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
                                     "You need the %s skill to use this item!",
                                     op->skill);
            if (who->type == PLAYER)
                return 1;

            /* monsters do not care about missing skills */
        } else
            /* While experience will be credited properly, we want to
             * change the skill so that the dam and wc get updated
             */
            change_skill(who, skop, (aflags&AP_NOPRINT));
    } else
        skop = NULL;

    if (!apply_check_item_power(who, op, aflags))
        return 1;

    if (!apply_check_personalized_blessings(who, op))
        return 1;

    /* Ok.  We are now at the state where we can apply the new object.
     * Note that we don't have the checks for can_use_...
     * below - that is already taken care of by apply_can_apply_object().
     */

    tmp = op->nrof <= 1 ? NULL : object_split(op, op->nrof-1, NULL, 0);

    switch (op->type) {
    case WEAPON:
        if (!apply_check_weapon_power(who, op->last_eat)) {
            if (!(aflags&AP_NOPRINT))
                draw_ext_info(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
                    "That weapon is too powerful for you to use.  It would consume your soul!");
            if (tmp != NULL)
                (void)object_insert_in_ob(tmp, who);
            return 1;
        }

        if (!apply_check_owner(who, op, aflags)) {
            if (tmp != NULL)
                (void)object_insert_in_ob(tmp, who);
            return 1;
        }

        SET_FLAG(op, FLAG_APPLIED);

        if (skop)
            change_skill(who, skop, 1);
        SET_FLAG(who, FLAG_READY_WEAPON);

        if (!(aflags&AP_NOPRINT)) {
            query_name(op, name_op, MAX_BUF);
            draw_ext_info_format(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
                "You wield %s.",
                name_op);
        }

        (void)change_abil(who, op);
        break;

    case ARMOUR:
    case HELMET:
    case SHIELD:
    case BOOTS:
    case GLOVES:
    case GIRDLE:
    case BRACERS:
    case CLOAK:
    case RING:
    case AMULET:
        SET_FLAG(op, FLAG_APPLIED);
        if (!(aflags&AP_NOPRINT)) {
            query_name(op, name_op, MAX_BUF);
            draw_ext_info_format(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
                                 "You wear %s.",
                                 name_op);
        }
        (void)change_abil(who, op);
        break;

        /* this part is needed for skill-tools */
    case SKILL:
    case SKILL_TOOL:
        if (who->chosen_skill) {
            LOG(llevError, "BUG: apply_special(): can't apply two skills\n");
            return 1;
        }

        apply_update_ranged_skill(who, op, aflags);
        SET_FLAG(op, FLAG_APPLIED);
        (void)change_abil(who, op);
        who->chosen_skill = op;
        SET_FLAG(who, FLAG_READY_SKILL);
        break;

    case BOW:
        if (!apply_check_weapon_power(who, op->last_eat)) {
            if (!(aflags&AP_NOPRINT))
                draw_ext_info(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
                    "That weapon is too powerful for you to use.  It would consume your soul!");
            if (tmp != NULL)
                (void)object_insert_in_ob(tmp, who);
            return 1;
        }

        if (!apply_check_owner(who, op, aflags)) {
            if (tmp != NULL)
                (void)object_insert_in_ob(tmp, who);
            return 1;
        }
        /*FALLTHROUGH*/
    case WAND:
    case ROD:
        /* check for skill, alter player status */
        SET_FLAG(op, FLAG_APPLIED);
        if (skop)
            change_skill(who, skop, 0);
        if (!(aflags&AP_NOPRINT)) {
            query_name(op, name_op, MAX_BUF);
            draw_ext_info_format(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
                                 "You ready %s.",
                                 name_op);
        }
        if (who->type == PLAYER) {
            if (op->type == BOW) {
                (void)change_abil(who, op);
                if (!(aflags&AP_NOPRINT)) {
                    query_name(op, name_op, MAX_BUF);
                    draw_ext_info_format(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
                                         "You will now fire %s with %s.",
                                         op->race ? op->race : "nothing",
                                         name_op);
                }
                who->contr->shoottype = range_bow;
            } else
                who->contr->shoottype = range_misc;
        } else {
            if (op->type == BOW)
                SET_FLAG(who, FLAG_READY_BOW);
            else
                SET_FLAG(who, FLAG_READY_RANGE);
        }
        break;

    case BUILDER:
        if (who->contr->ranges[range_builder])
            unapply_special(who, who->contr->ranges[range_builder], 0);
        who->contr->shoottype = range_builder;
        who->contr->ranges[range_builder] = op;
        if (!(aflags&AP_NOPRINT)) {
            query_name(op, name_op, MAX_BUF);
            draw_ext_info_format(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
                                 "You ready your %s.",
                                 name_op);
        }
        break;

    default:
        query_name(op, name_op, MAX_BUF);
        draw_ext_info_format(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
                             "You apply %s.",
                             name_op);
        break;
    } /* end of switch op->type */

    SET_FLAG(op, FLAG_APPLIED);

    if (tmp != NULL)
        tmp = object_insert_in_ob(tmp, who);

    fix_object(who);

    /* We exclude spell casting objects.  The fire code will set the
     * been applied flag when they are used - until that point,
     * you don't know anything about them.
     */
    if (who->type == PLAYER && op->type != WAND && op->type != ROD)
        SET_FLAG(op, FLAG_BEEN_APPLIED);

    if (QUERY_FLAG(op, FLAG_CURSED) || QUERY_FLAG(op, FLAG_DAMNED)) {
        if (who->type == PLAYER) {
            draw_ext_info(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_CURSED,
                          "Oops, it feels deadly cold!");
            SET_FLAG(op, FLAG_KNOWN_CURSED);
        }
    }
    if (who->type == PLAYER)
        esrv_update_item(UPD_NROF|UPD_FLAGS|UPD_WEIGHT, who, op);
    return 0;
}

/**
 * Map was just loaded, handle op's initialization.
 *
 * Generates shop floor's item, and treasures.
 *
 * @param op
 * object to initialize.
 * @return
 * 1 if object was initialized, 0 else.
 *
 * @note
 * auto_apply() has been renamed to apply_auto()
 */
int apply_auto(object *op) {
    object *tmp;

    switch (op->type) {
    case SHOP_FLOOR:
        if (!HAS_RANDOM_ITEMS(op))
            return 0;
        do {
            int i;

            i = 10; /* let's give it 10 tries */
            while ((tmp = generate_treasure(op->randomitems, op->stats.exp ? (int)op->stats.exp : MAX(op->map->difficulty, 5))) == NULL
            && --i)
                ;
            if (tmp == NULL)
                return 0;
            if (QUERY_FLAG(tmp, FLAG_CURSED) || QUERY_FLAG(tmp, FLAG_DAMNED)) {
                object_free2(tmp, FREE_OBJ_NO_DESTROY_CALLBACK);
                tmp = NULL;
            }
        } while (!tmp);
        SET_FLAG(tmp, FLAG_UNPAID);
        object_insert_in_map_at(tmp, op->map, NULL, 0, op->x, op->y);
        CLEAR_FLAG(op, FLAG_AUTO_APPLY);
        identify(tmp);
        return 1;
        break;

    case TREASURE:
        if (QUERY_FLAG(op, FLAG_IS_A_TEMPLATE))
            return 0;
        while (op->stats.hp-- > 0)
            create_treasure(op->randomitems, op, (op->map ? GT_ENVIRONMENT : 0) | (QUERY_FLAG(op, FLAG_BLESSED) ? GT_ONLY_GOOD : 0), op->stats.exp ? (int)op->stats.exp : op->map == NULL ? 14 : op->map->difficulty, 0);

        /* If we generated an object and put it in this object's
         * inventory, move it to the parent object as the current
         * object is about to disappear.  An example of this item
         * is the random_ *stuff that is put inside other objects.
         */
        FOR_INV_PREPARE(op, tmp) {
            object_remove(tmp);
            if (op->env)
                object_insert_in_ob(tmp, op->env);
            else
                object_free_drop_inventory(tmp);
        } FOR_INV_FINISH();
        object_remove(op);
        object_free2(op, FREE_OBJ_NO_DESTROY_CALLBACK);
        break;
    }
    return 0;
}

/**
 * Go through the entire map (only the first time
 * when an original map is loaded) and performs special actions for
 * certain objects (most initialization of chests and creation of
 * treasures and stuff).  Calls apply_auto() if appropriate.
 *
 * @param m
 * map to fix.
 *
 * @note
 * fix_auto_apply() has been renamed to apply_auto_fix()
 */

void apply_auto_fix(mapstruct *m) {
    int x, y;

    if (m == NULL)
        return;

    for (x = 0; x < MAP_WIDTH(m); x++)
        for (y = 0; y < MAP_HEIGHT(m); y++)
            FOR_MAP_PREPARE(m, x, y, tmp) {
                if (tmp->inv) {
                    FOR_INV_PREPARE(tmp, invtmp) {
                        if (QUERY_FLAG(invtmp, FLAG_AUTO_APPLY))
                            apply_auto(invtmp);
                        else if (invtmp->type == TREASURE && HAS_RANDOM_ITEMS(invtmp)) {
                            while (invtmp->stats.hp-- > 0)
                                create_treasure(invtmp->randomitems, invtmp, 0, m->difficulty, 0);
                            invtmp->randomitems = NULL;
                        } else if (invtmp && invtmp->arch
                        && invtmp->type != TREASURE
                        && invtmp->type != SPELL
                        && invtmp->type != CLASS
                        && HAS_RANDOM_ITEMS(invtmp)) {
                            create_treasure(invtmp->randomitems, invtmp, 0, m->difficulty, 0);
                            /* Need to clear this so that we never try to
                             * create treasure again for this object
                             */
                            invtmp->randomitems = NULL;
                        }
                    } FOR_INV_FINISH();
                    /* This is really temporary - the code at the
                     * bottom will also set randomitems to null.
                     * The problem is there are bunches of maps/players
                     * already out there with items that have spells
                     * which haven't had the randomitems set
                     * to null yet.
                     * MSW 2004-05-13
                     *
                     * And if it's a spellbook, it's better to set
                     * randomitems to NULL too, else you get two spells
                     * in the book ^_-
                     * Ryo 2004-08-16
                     */
                    if (tmp->type == WAND
                    || tmp->type == ROD
                    || tmp->type == SCROLL
                    || tmp->type == FIREWALL
                    || tmp->type == POTION
                    || tmp->type == ALTAR
                    || tmp->type == SPELLBOOK)
                        tmp->randomitems = NULL;
                }

                if (QUERY_FLAG(tmp, FLAG_AUTO_APPLY))
                    apply_auto(tmp);
                else if ((tmp->type == TREASURE || tmp->type == CONTAINER)
                && HAS_RANDOM_ITEMS(tmp)) {
                    while (tmp->stats.hp-- > 0)
                        create_treasure(tmp->randomitems, tmp, 0, m->difficulty, 0);
                    tmp->randomitems = NULL;
                } else if (tmp->type == TIMED_GATE) {
                    object *head = HEAD(tmp);

                    if (QUERY_FLAG(head, FLAG_IS_LINKED)) {
                        tmp->speed = 0;
                        object_update_speed(tmp);
                    }
                }
                /* This function can be called every time a map is loaded,
                 * even when swapping back in.  As such, we don't want to
                 * create the treasure over and over again, so after we
                 * generate the treasure, blank out randomitems so if it
                 * is swapped in again, it won't make anything. This is a
                 * problem for the above objects, because they have
                 * counters which say how many times to make the treasure.
                 */
                else if (tmp
                && tmp->arch
                && tmp->type != PLAYER
                && tmp->type != TREASURE
                && tmp->type != SPELL
                && tmp->type != PLAYER_CHANGER
                && tmp->type != CLASS
                && HAS_RANDOM_ITEMS(tmp)) {
                    create_treasure(tmp->randomitems, tmp, 0, m->difficulty, 0);
                    tmp->randomitems = NULL;
                }

                if (QUERY_FLAG(tmp, FLAG_MONSTER))
                    monster_check_apply_all(tmp);
            } FOR_MAP_FINISH();

    for (x = 0; x < MAP_WIDTH(m); x++)
        for (y = 0; y < MAP_HEIGHT(m); y++)
            FOR_MAP_PREPARE(m, x, y, tmp) {
                if (tmp->above
                && (tmp->type == TRIGGER_BUTTON || tmp->type == TRIGGER_PEDESTAL))
                    check_trigger(tmp, tmp->above);
            } FOR_MAP_FINISH();
}

/**
 * op made some mistake with a scroll, this takes care of punishment.
 * scroll_failure()- hacked directly from spell_failure
 *
 * If settings.spell_failure_effects is FALSE, the only nasty things
 * that can happen are weird spell cast, or mana drain.
 *
 * @param op
 * who failed.
 * @param failure
 * what kind of nasty things happen.
 * @param power
 * the higher the value, the worse the thing that happens.
 */
void scroll_failure(object *op, int failure, int power) {
    if (abs(failure/4) > power)
        power = abs(failure/4); /* set minimum effect */

    if (failure <= -1 && failure > -15) {/* wonder */
        object *tmp;

        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_FAILURE,
                      "Your spell warps!");
        tmp = create_archetype(SPELL_WONDER);
        cast_wonder(op, op, 0, tmp);
        if (op->stats.sp < 0)
            /* For some reason the sp can become negative here. */
            op->stats.sp = 0;
        object_free2(tmp, FREE_OBJ_NO_DESTROY_CALLBACK);
        return;
    }

    if (settings.spell_failure_effects == TRUE) {
        if (failure <= -35 && failure > -60) { /* confusion */
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_FAILURE,
                          "The magic recoils on you!");
            confuse_living(op, op, power);
            return;
        }

        if (failure <= -60 && failure > -70) {/* paralysis */
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_FAILURE,
                          "The magic recoils and paralyzes you!");
            paralyze_living(op, power);
            return;
        }

        if (failure <= -70 && failure > -80) {/* blind */
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_FAILURE,
                          "The magic recoils on you!");
            blind_living(op, op, power);
            return;
        }

        if (failure <= -80) {/* blast the immediate area */
            object *tmp;

            tmp = create_archetype(LOOSE_MANA);
            cast_magic_storm(op, tmp, power);
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_FAILURE,
                          "You unlease uncontrolled mana!");
            object_free2(tmp, FREE_OBJ_NO_DESTROY_CALLBACK);
            return;
        }
    }
    /* Either no spell failure on this server, or wrong values,
     * in any case let's punish.
     */
    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_FAILURE,
                  "Your mana is drained!");
    op->stats.sp -= random_roll(0, power-1, op, PREFER_LOW);
    if (op->stats.sp < 0)
        op->stats.sp = 0;
}

/**
 * Applies (race) changes to a player.
 * @param pl
 * object to change.
 * @param change
 * what kind of changes to apply. Should be of type CLASS.
 * @param limit_stats
 * uses the AC_PLAYER_STAT defines from define.h:
 * AC_PLAYER_STAT_LIMIT: Limit stats to racial maximum
 * AC_PLAYER_STAT_NO_CHANGE: Do not make any stat adjustments
 */
void apply_changes_to_player(object *pl, object *change, int limit_stats) {
    int i, j;
    int excess_stat = 0;  /* if the stat goes over the maximum
                           * for the race, put the excess stat some
                           * where else.
                           */

    if (change->type != CLASS) return;

    /* the following code assigns stats up to the stat max
     * for the race, and if the stat max is exceeded,
     * tries to randomly reassign the excess stat
     */
    if (! (limit_stats & AC_PLAYER_STAT_NO_CHANGE)) {
        for (i = 0; i < NUM_STATS; i++) {
            sint8 stat = get_attr_value(&pl->contr->orig_stats, i);
            int race_bonus = get_attr_value(&pl->arch->clone.stats, i);

            stat += get_attr_value(&change->stats, i);
            if (limit_stats & AC_PLAYER_STAT_LIMIT) {
                if (stat > 20+race_bonus) {
                    excess_stat++;
                    stat = 20+race_bonus;
                } else if (stat < 1) {
                    /* I didn't see any code here before to make sure minimum
                     * stats were enforced - maybe it was just dumb
                     * luck that one would not have a stat low enough that then
                     * has a stat penalty for class that would bring it negative?
                     * I imagine a negative stat would crash the server pretty
                     * quickly - MSW, Sept 2010
                     */
                    excess_stat += stat;
                    stat = 1;
                }
            }
            set_attr_value(&pl->contr->orig_stats, i, stat);
        }

        /* Maybe we should randomly deduct stats in this case?
         * It's will go away sometime soon in any case.
         */
        if (excess_stat < 0) excess_stat = 0;

        /* We don't put an explicit check for limit_stats here -
         * excess stat will never be 0 if limit_stats is not
         * true.
         */
        for (j = 0; excess_stat > 0 && j < 100; j++) {
            /* try 100 times to assign excess stats */
            int i = rndm(0, NUM_STATS-1);
            int stat = get_attr_value(&pl->contr->orig_stats, i);
            int race_bonus = get_attr_value(&pl->arch->clone.stats, i);

            if (i == CHA)
                continue; /* exclude cha from this */
            if (stat < 20+race_bonus) {
                change_attr_value(&pl->contr->orig_stats, i, 1);
                excess_stat--;
            }
        }
    }
    /* Done with stat processing */

    /* insert the randomitems from the change's treasurelist into
     * the player ref: player.c
     */
    if (change->randomitems != NULL)
        give_initial_items(pl, change->randomitems);


    /* set up the face, for some races. */

    /* first, look for the force object banning changing the
     * face.  Certain races never change face with class.
     */
    if (object_find_by_name(pl, "NOCLASSFACECHANGE") == NULL) {
        pl->animation_id = GET_ANIM_ID(change);
        pl->face = change->face;

        if (QUERY_FLAG(change, FLAG_ANIMATE))
            SET_FLAG(pl, FLAG_ANIMATE);
        else
            CLEAR_FLAG(pl, FLAG_ANIMATE);
    }

    if (change->anim_suffix) {
        char buf[MAX_BUF];
        int anim;

        snprintf(buf, MAX_BUF, "%s_%s", animations[pl->animation_id].name, change->anim_suffix);
        anim = try_find_animation(buf);
        if (anim) {
            pl->animation_id = anim;
            pl->anim_speed = -1;
            CLEAR_FLAG(pl, FLAG_ANIMATE);
            animate_object(pl, pl->facing);
        }
    }
    /* Hard coding in class name is a horrible idea - lets
     * use the supported mechanism for this
     */
    if (object_present_in_ob_by_name(FORCE, "no weapon force", pl))
        CLEAR_FLAG(pl, FLAG_USE_WEAPON);

}

void legacy_apply_container(object *op, object *sack) {
    apply_container(op, sack);
}

/**
 * Checks for general apply restrictions (no body, prohibited by god, conflicts
 * with other items, etc.)
 *
 * @param who
 * the object applying the item
 * @param op
 * the item being applied
 * @param aflags
 * combinaison of @ref AP_xxx "AP_xxx" flags
 * @return
 * whether applying is possible
 */
static int apply_check_apply_restrictions(object *who, object *op, int aflags) {
    int i;

    i = apply_can_apply_object(who, op);
    if (i == 0)
        return 1;

    /* Can't just apply this object.  Lets see why not and what to do */

    if (i&CAN_APPLY_NEVER) {
        if (!(aflags&AP_NOPRINT)) {
            char name_op[MAX_BUF];

            query_name(op, name_op, MAX_BUF);
            draw_ext_info_format(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_BADBODY,
                "You don't have the body to use a %s",
                name_op);
        }
        return 0;
    }

    if (i&CAN_APPLY_RESTRICTION) {
        if (!(aflags&AP_NOPRINT)) {
            char name_op[MAX_BUF];

            query_name(op, name_op, MAX_BUF);
            draw_ext_info_format(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_PROHIBITION,
                "You have a prohibition against using a %s",
                name_op);
        }
        return 0;
    }

    if (who->type != PLAYER) {
        /* Some error, so don't try to equip something more */
        return !unapply_for_ob(who, op, aflags);
    }

    if (who->contr->unapply == unapply_never
    || (i&CAN_APPLY_UNAPPLY_CHOICE && who->contr->unapply == unapply_nochoice)) {
        if (!(aflags&AP_NOPRINT))
            draw_ext_info(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_UNAPPLY,
                "You need to unapply some item(s):");
        unapply_for_ob(who, op, AP_PRINT);
        return 0;
    }

    if (who->contr->unapply == unapply_always
    || !(i&CAN_APPLY_UNAPPLY_CHOICE)) {
        return !unapply_for_ob(who, op, aflags);
    }

    return 1;
}

/**
 * Checks for item power restrictions when applying an item.
 *
 * @param who
 * the object applying the item
 * @param op
 * the item being applied
 * @param aflags
 * combinaison of @ref AP_xxx "AP_xxx" flags
 * @return
 * whether applying is possible
 */
static int apply_check_item_power(const object *who, const object *op, int aflags) {
    if (who->type != PLAYER)
        return 1;

    if (op->item_power == 0
    || op->item_power+who->contr->item_power <= settings.item_power_factor*who->level)
        return 1;

    if (!(aflags&AP_NOPRINT))
        draw_ext_info(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
            "Equipping that combined with other items would consume your soul!");
    return 0;
}

/**
 * If personalized blessings are activated, the weapon can bite
 * the wielder if he/she is not the one who initially blessed it.
 * Chances of being hurt depend on the experience amount
 * ("willpower") the object has, compared to the experience
 * amount of the wielder.
 *
 * @param who
 * the object applying the item
 * @param op
 * the item being applied
 * @return
 * whether applying is possible
 */
static int apply_check_personalized_blessings(object *who, const object *op) {
    const char *owner;
    const char *will;
    long item_will;
    long margin;
    const char *msg;
    int random_effect;
    int damage_percentile;

    if (!settings.personalized_blessings) {
        return 1;
    }

    owner = object_get_value(op, "item_owner");
    if (owner == NULL || strcmp(owner, who->name) == 0)
        return 1;

    will = object_get_value(op, "item_willpower");
    item_will = will != NULL ? atol(will) : 0;
    if (item_will > who->stats.exp) {
        draw_ext_info_format(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
            "This %s refuses to serve you - it keeps evading your hand !",
            op->name);
        return 0;
    }

    margin = item_will != 0 ? who->stats.exp/item_will : who->stats.exp;
    random_effect = random_roll(0, 100, who, 1)-margin*20;
    if (random_effect > 80) {
        msg = "You don't know why, but you have the feeling that the %s is angry at you !";
        damage_percentile = 60;
    } else if (random_effect > 60) {
        msg = "The %s seems to look at you nastily !";
        damage_percentile = 45;
    } else if (random_effect > 40) {
        msg = "You have the strange feeling that the %s is annoyed...";
        damage_percentile = 30;
    } else if (random_effect > 20) {
        msg = "The %s seems tired, or bored, in a way. Very strange !";
        damage_percentile = 15;
    } else if (random_effect > 0) {
        msg = "You hear the %s sighing !";
        damage_percentile = 0;
    } else {
        msg = NULL;
        damage_percentile = 0;
    }
    if (msg != NULL)
        draw_ext_info_format(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
            msg, op->name);
    if (damage_percentile > 0) {
        int weapon_bite = (who->stats.hp*damage_percentile)/100;
        if (weapon_bite < 1)
            weapon_bite = 1;
        who->stats.hp -= weapon_bite;
        draw_ext_info_format(NDI_UNIQUE, 0, who, MSG_TYPE_VICTIM, MSG_TYPE_VICTIM_WAS_HIT,
            "You get a nasty bite in the hand !");
    }

    return 1;
}

/**
 * Checks that the item's owner matches the applier. If the weapon does not
 * have the name as the character, can't use it. (Ragnarok's sword attempted to
 * be used by Foo: won't work).
 *
 * @param who
 * the object applying the item
 * @param op
 * the item being applied
 * @param aflags
 * combination of @ref AP_xxx "AP_xxx" flags
 * @return
 * whether applying is possible
 */
static int apply_check_owner(const object *who, const object *op, int aflags) {
    const char *quotepos;

    if (op->level == 0)
        return 1;

    quotepos = strstr(op->name, "'");
    if (quotepos == NULL || strncmp(op->name, who->name, quotepos-op->name) == 0)
        return 1;

    if (!(aflags&AP_NOPRINT))
        draw_ext_info(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
            "The weapon does not recognize you as its owner.");
    return 0;
}

/**
 * Updates ranged skill information.
 *
 * @param who
 * the object applying the item
 * @param op
 * the item being applied
 * @param aflags
 * combination of @ref AP_xxx "AP_xxx" flags
 */
static void apply_update_ranged_skill(const object *who, object *op, int aflags) {
    if (who->type != PLAYER) {
        return;
    }

    who->contr->shoottype = range_skill;
    who->contr->ranges[range_skill] = op;
    if (op->invisible) {
        if (!(aflags&AP_NOPRINT))
            draw_ext_info_format(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
                "Readied skill: %s.",
                op->skill ? op->skill : op->name);
    } else {
        if (!(aflags&AP_NOPRINT)) {
            char name_op[MAX_BUF];

            query_name(op, name_op, MAX_BUF);
            draw_ext_info_format(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
                "You ready %s.",
                name_op);
            draw_ext_info_format(NDI_UNIQUE, 0, who, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
                "You can now use the skill: %s.",
                op->skill);
        }
    }
}
