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
 * This handles triggers, buttons, altars and associated objects.
 */

#include <global.h>
#include <sproto.h>

static objectlink *get_button_links(const object *button);

/**
 * Trigger every object in an objectlink. This was originally
 * part of push_button but has been extracted to make it
 * possible to trigger the connected object on a map
 * from a plugin without requiring a source object.
 * This method will take care of calling EVENT_TRIGGER of all
 * elligible object in list (see state parameter)
 * @param ol the objectlink to trigger. This can be acquire from map
 * @param cause the object that cause this path to trigger, may be NULL
 * @param state which object to apply.
 *          0=all object with FLAG_ACTIVATE_ON_PUSH
 *          other=all object with FLAG_ACTIVATE_ON_RELEASE
 */
void trigger_connected(objectlink *ol, object *cause, const int state) {
    object *tmp;

    for (; ol; ol = ol->next) {
        object *part;

        if (!ol->ob || ol->ob->count != ol->id) {
            LOG(llevError, "Internal error in trigger_connect. No object associated with link id (%u) (cause='%s'.\n", ol->id, (cause && cause->name) ? cause->name : "");
            continue;
        }
        /* a button link object can become freed when the map is saving.  As
         * a map is saved, objects are removed and freed, and if an object is
         * on top of a button, this function is eventually called.  If a map
         * is getting moved out of memory, the status of buttons and levers
         * probably isn't important - it will get sorted out when the map is
         * re-loaded.  As such, just exit this function if that is the case.
         */

        if (QUERY_FLAG(ol->ob, FLAG_FREED))
            return;
        tmp = ol->ob;

        /* if the criteria isn't appropriate, don't do anything */
        if (state && !QUERY_FLAG(tmp, FLAG_ACTIVATE_ON_PUSH))
            continue;
        if (!state && !QUERY_FLAG(tmp, FLAG_ACTIVATE_ON_RELEASE))
            continue;

        /*
         * (tchize) call the triggers of the activated object.
         * tmp = activated object
         * op is activator (aka button)
         */
        if (execute_event(tmp, EVENT_TRIGGER, cause, NULL, NULL, SCRIPT_FIX_ALL) != 0)
            continue;

        switch (tmp->type) {
        case GATE:
        case HOLE:
            tmp->value = tmp->stats.maxsp ? !state : state;
            tmp->speed = 0.5;
            object_update_speed(tmp);
            break;

        case CF_HANDLE:
            SET_ANIMATION(tmp, (tmp->value = tmp->stats.maxsp ? !state : state));
            object_update(tmp, UP_OBJ_FACE);
            break;

        case SIGN:
            if (!tmp->stats.food || tmp->last_eat < tmp->stats.food) {
                ext_info_map(NDI_UNIQUE|NDI_NAVY, tmp->map,
                    MSG_TYPE_SIGN, MSG_SUBTYPE_NONE,
                    tmp->msg);
                if (tmp->stats.food)
                    tmp->last_eat++;
            }
            break;

        case ALTAR:
            tmp->value = 1;
            SET_ANIMATION(tmp, tmp->value);
            object_update(tmp, UP_OBJ_FACE);
            break;

        case BUTTON:
        case PEDESTAL:
            tmp->value = state;
            SET_ANIMATION(tmp, tmp->value);
            object_update(tmp, UP_OBJ_FACE);
            break;

        case TIMED_GATE:
            for (part = tmp; tmp != NULL; tmp = tmp->more) {
                part->speed = tmp->arch->clone.speed;
                part->value = tmp->arch->clone.value;
                part->stats.sp = 1;
                part->stats.hp = tmp->stats.maxhp;
                object_update_speed(part);
            }
            break;

        case DIRECTOR:
        case FIREWALL:
            if (!QUERY_FLAG(tmp, FLAG_ANIMATE) && tmp->type == FIREWALL)
                move_firewall(tmp);
            else {
                if ((tmp->stats.sp += tmp->stats.maxsp) > 8) /* next direction */
                    tmp->stats.sp = ((tmp->stats.sp-1)%8)+1;
                animate_turning(tmp);
            }
            break;

        default:
            ob_trigger(tmp, cause, state);
        }
    }
}

/**
 * Push the specified object.  This can affect other buttons/gates/handles
 * altars/pedestals/holes in the whole map.
 * Changed the routine to loop through _all_ linked objects.
 * Better hurry with that linked list...
 * @param op
 * object to push.
 */
void push_button(object *op) {
    /* LOG(llevDebug, "push_button: %s (%d)\n", op->name, op->count); */
    trigger_connected(get_button_links(op), op, op->value);
}

/**
 * Updates everything connected with the button op.
 * After changing the state of a button, this function must be called
 * to make sure that all gates and other buttons connected to the
 * button reacts to the (eventual) change of state.
 * @param op
 * object to update.
 */
void update_button(object *op) {
    object *tmp, *head;
    int tot, any_down = 0, old_value = op->value;
    objectlink *ol;

    /* LOG(llevDebug, "update_button: %s (%d)\n", op->name, op->count); */
    for (ol = get_button_links(op); ol; ol = ol->next) {
        if (!ol->ob || ol->ob->count != ol->id) {
            LOG(llevDebug, "Internal error in update_button (%s).\n", op->name);
            continue;
        }

        tmp = ol->ob;
        if (tmp->type == BUTTON) {
            tot = 0;
            FOR_ABOVE_PREPARE(tmp, ab)
                /* Bug? The pedestal code below looks for the head of
                 * the object, this bit doesn't.  I'd think we should check
                 * for head here also.  Maybe it also makese sense to
                 * make the for ab=tmp->above loop common, and alter
                 * behaviour based on object within that loop?
                 */

                /* Basically, if the move_type matches that on what the
                 * button wants, we count it.  The second check is so that
                 * objects don't move (swords, etc) will count.  Note that
                 * this means that more work is needed to make buttons
                 * that are only triggered by flying objects.
                 */
                if ((ab->move_type&tmp->move_on) || ab->move_type == 0)
                    tot += ab->weight*(ab->nrof ? ab->nrof : 1)+ab->carrying;
            FOR_ABOVE_FINISH();

            tmp->value = (tot >= tmp->weight) ? 1 : 0;
            if (tmp->value)
                any_down = 1;
        } else if (tmp->type == PEDESTAL) {
            tmp->value = 0;
            FOR_ABOVE_PREPARE(tmp, ab) {
                head = ab->head ? ab->head : ab;
                /* Same note regarding move_type for buttons above apply here. */
                if (((head->move_type&tmp->move_on) || ab->move_type == 0)
                && (head->race == tmp->slaying
                    || ((head->type == SPECIAL_KEY) && (head->slaying == tmp->slaying))
                    || (!strcmp(tmp->slaying, "player") && head->type == PLAYER)))
                    tmp->value = 1;
            } FOR_ABOVE_FINISH();
            if (tmp->value)
                any_down = 1;
        }
    }
    if (any_down) /* If any other buttons were down, force this to remain down */
        op->value = 1;

    /* If this button hasn't changed, don't do anything */
    if (op->value != old_value) {
        SET_ANIMATION(op, op->value);
        object_update(op, UP_OBJ_FACE);
        push_button(op); /* Make all other buttons the same */
    }
}

/**
 * Updates every button on the map (by calling update_button() for them).
 */
void update_buttons(mapstruct *m) {
    objectlink *ol;
    oblinkpt *obp;

    for (obp = m->buttons; obp; obp = obp->next)
        for (ol = obp->link; ol; ol = ol->next) {
            if (!ol->ob || ol->ob->count != ol->id) {
                LOG(llevError, "Internal error in update_button (%s (%dx%d):%u, connected %ld).\n",
                    ol->ob ? ol->ob->name : "null",
                    ol->ob ? ol->ob->x : -1,
                    ol->ob ? ol->ob->y : -1,
                    ol->id,
                    obp->value);
                continue;
            }
            if (ol->ob->type == BUTTON || ol->ob->type == PEDESTAL) {
                update_button(ol->ob);
                break;
            }
        }
}

/**
 * Toggles the state of specified button.
 * @param op
 * object to toggle.
 */
void use_trigger(object *op) {
    /* Toggle value */
    op->value = !op->value;
    push_button(op);
}

/**
 * Animates one step of object.
 * @param op
 * object to animate.
 * @note
 * animate_object() should be used instead of this,
 * but it can't handle animations in the 8 directions
 * @todo
 * check if object is really animated?
 */
void animate_turning(object *op) {
    if (++op->state >= NUM_ANIMATIONS(op)/8)
        op->state = 0;
    SET_ANIMATION(op, (op->stats.sp-1)*NUM_ANIMATIONS(op)/8+op->state);
    object_update(op, UP_OBJ_FACE);
}

#define ARCH_SACRIFICE(xyz) ((xyz)->slaying)
#define NROF_SACRIFICE(xyz) ((uint32)(xyz)->stats.food)

/**
 * Helper function to check if the item matches altar's requested sacrifice.
 * The number of objects is not taken into account.
 *
 * @param altar
 * altar we're checking for. Can't be NULL.
 * @param sacrifice
 * what object to check for. Can't be NULL.
 * @return
 * 1 if object is suitable for the altar (number not taken into account), 0 else.
 */
static int matches_sacrifice(const object *altar, const object *sacrifice) {
    char name[MAX_BUF];

    if ((QUERY_FLAG(sacrifice, FLAG_ALIVE) && object_get_value(altar, "accept_alive") == NULL)
    || QUERY_FLAG(sacrifice, FLAG_IS_LINKED)
    || sacrifice->type == PLAYER)
        return 0;

    query_base_name(sacrifice, 0, name, MAX_BUF);
    if (ARCH_SACRIFICE(altar) == sacrifice->arch->name
    || ARCH_SACRIFICE(altar) == sacrifice->name
    || ARCH_SACRIFICE(altar) == sacrifice->slaying
    || (!strcmp(ARCH_SACRIFICE(altar), name)))
        return 1;

    if (strcmp(ARCH_SACRIFICE(altar), "money") == 0
    && sacrifice->type == MONEY)
        return 1;

    return 0;
}

/**
 * Checks whether the altar has enough to sacrifice.
 *
 * Function put in (0.92.1) so that identify altars won't grab money
 * unnecessarily - we can see if there is sufficient money, see if something
 * needs to be identified, and then remove money if needed.
 *
 * 0.93.4: Linked objects (ie, objects that are connected) can not be
 * sacrificed.  This fixes a bug of trying to put multiple altars/related
 * objects on the same space that take the same sacrifice.
 *
 * The function will now check for all items sitting on the altar, so that the player
 * can put various matching but non merging items on the altar.
 *
 * This function can potentially remove other items, if remove_others is set.
 *
 * @param altar
 * item to which there is a sacrifice
 * @param sacrifice
 * object that may be sacrifed
 * @param remove_others
 * if 1, will remove enough items apart sacrifice to compensate for not having enough in sacrifice itself.
 * @param[out] toremove
 * will contain the nrof of sacrifice to really remove to finish operating. Will be set if not NULL only
 * if the function returns 1.
 * @return
 * 1 if the sacrifice meets the needs of the altar, 0 else
 */
int check_altar_sacrifice(const object *altar, const object *sacrifice, int remove_others, int *toremove) {
    int money;
    uint32 wanted, rest;

    if (!matches_sacrifice(altar, sacrifice))
        /* New dropped object doesn't match the altar, other objects already on top are not enough to
         * activate altar, else they would have disappeared. */
        return 0;

    /* Check item is paid for. */
    if (QUERY_FLAG(sacrifice, FLAG_UNPAID)) {
        return 0;
    }

    money = (strcmp(ARCH_SACRIFICE(altar), "money") == 0) ? 1 : 0;

    /* Easy checks: newly dropped object is enough for sacrifice. */
    if (money && sacrifice->nrof*sacrifice->value >= NROF_SACRIFICE(altar)) {
        if (toremove) {
            *toremove = NROF_SACRIFICE(altar)/sacrifice->value;
            /* Round up any sacrifices.  Altars don't make change either */
            if (NROF_SACRIFICE(altar)%sacrifice->value)
                (*toremove)++;
        }
        return 1;
    }

    if (!money && NROF_SACRIFICE(altar) <= (sacrifice->nrof ? sacrifice->nrof : 1)) {
        if (toremove)
            *toremove = NROF_SACRIFICE(altar);
        return 1;
    }

    if (money) {
        wanted = NROF_SACRIFICE(altar)-sacrifice->nrof*sacrifice->value;
    } else {
        wanted = NROF_SACRIFICE(altar)-(sacrifice->nrof ? sacrifice->nrof : 1);
    }
    rest = wanted;

    /* Ok, now we check if we got enough with other items.
     * We only check items above altar, and not checking again sacrifice.
     */
    FOR_ABOVE_PREPARE(altar, tmp) {
        if (wanted <= 0)
            break;
        if (tmp == sacrifice || !matches_sacrifice(altar, tmp))
            continue;
        if (money)
            wanted -= tmp->nrof*tmp->value;
        else
            wanted -= (tmp->nrof ? tmp->nrof : 1);
    } FOR_ABOVE_FINISH();

    if (wanted > 0)
        /* Not enough value, let's bail out. */
        return 0;

    /* From there on, we do have enough objects for the altar. */

    /* Last dropped object will be totally eaten in any case. */
    if (toremove)
        *toremove = sacrifice->nrof ? sacrifice->nrof : 1;

    if (!remove_others)
        return 1;

    /* We loop again, this time to remove what we need. */
    FOR_ABOVE_PREPARE(altar, tmp) {
        if (rest <= 0)
            break;
        if (tmp == sacrifice || !matches_sacrifice(altar, tmp))
            continue;
        if (money) {
            wanted = tmp->nrof*tmp->value;
            if (rest > wanted) {
                object_remove(tmp);
                rest -= wanted;
            } else {
                wanted = rest/tmp->value;
                if (rest%tmp->value)
                    wanted++;
                object_decrease_nrof(tmp, wanted);
                return 1;
            }
        } else
            if (rest > (tmp->nrof ? tmp->nrof : 1)) {
                rest -= (tmp->nrof ? tmp->nrof : 1);
                object_remove(tmp);
            } else {
                object_decrease_nrof(tmp, rest);
                return 1;
            }
    } FOR_ABOVE_FINISH();

    /* Something went wrong, we'll be nice and accept the sacrifice anyway. */
    LOG(llevError, "check_altar_sacrifice on %s: found objects to sacrifice, but couldn't remove them??\n", altar->map->path);
    return 1;
}

/**
 * Checks if sacrifice was accepted and removes sacrificed
 * objects.  Might be better to
 * call check_altar_sacrifice (above) than depend on the return value,
 * since operate_altar will remove the sacrifice also.
 *
 * If this function returns 1, '*sacrifice' is modified to point to the
 * remaining sacrifice, or is set to NULL if the sacrifice was used up.
 *
 * @param altar
 * item to which there is a sacrifice
 * @param sacrifice
 * object that may be sacrifed
 * @return
 * 1 if sacrifice was accepted, else 0
 */
int operate_altar(object *altar, object **sacrifice) {
    int number;

    if (!altar->map) {
        LOG(llevError, "BUG: operate_altar(): altar has no map\n");
        return 0;
    }

    if (!altar->slaying || altar->value)
        return 0;

    if (!check_altar_sacrifice(altar, *sacrifice, 1, &number))
        return 0;

    /* check_altar_sacrifice fills in number for us. */
    *sacrifice = object_decrease_nrof(*sacrifice, number);

    if (altar->msg)
        ext_info_map(NDI_BLACK, altar->map, MSG_TYPE_DIALOG, MSG_TYPE_DIALOG_ALTAR, altar->msg);
    return 1;
}

/**
 * @todo document?
 */
static void trigger_move(object *op, int state) { /* 1 down and 0 up */
    op->stats.wc = state;
    if (state) {
        use_trigger(op);
        if (op->stats.exp > 0) /* check sanity  */
            op->speed = 1.0/op->stats.exp;
        else
            op->speed = 1.0;
        object_update_speed(op);
        op->speed_left = -1;
    } else {
        use_trigger(op);
        op->speed = 0;
        object_update_speed(op);
    }
}

/**
 * @todo document properly
 * cause != NULL: something has moved on top of op
 *
 * cause == NULL: nothing has moved, we have been called from
 * animate_trigger().
 *
 * TRIGGER_ALTAR: Returns 1 if 'cause' was destroyed, 0 if not.
 *
 * TRIGGER: Returns 1 if handle could be moved, 0 if not.
 *
 * TRIGGER_BUTTON, TRIGGER_PEDESTAL: Returns 0.
 */
int check_trigger(object *op, object *cause) {
    int push = 0, tot = 0;
    int in_movement = op->stats.wc || op->speed;

    switch (op->type) {
    case TRIGGER_BUTTON:
        if (op->weight > 0) {
            if (cause) {
                FOR_ABOVE_PREPARE(op, tmp)
                    /* Comment reproduced from update_buttons():
                     * Basically, if the move_type matches that on what the
                     * button wants, we count it.  The second check is so that
                     * objects that don't move (swords, etc) will count.  Note that
                     * this means that more work is needed to make buttons
                     * that are only triggered by flying objects.
                     */
                    if ((tmp->move_type&op->move_on) || tmp->move_type == 0) {
                        tot += tmp->weight*(tmp->nrof ? tmp->nrof : 1)+tmp->carrying;
                    }
                FOR_ABOVE_FINISH();
                if (tot >= op->weight)
                    push = 1;
                if (op->stats.ac == push)
                    return 0;
                op->stats.ac = push;
                if (NUM_ANIMATIONS(op) > 1) {
                    SET_ANIMATION(op, push);
                    object_update(op, UP_OBJ_FACE);
                }
                if (in_movement || !push)
                    return 0;
            }
            trigger_move(op, push);
        }
        return 0;

    case TRIGGER_PEDESTAL:
        if (cause) {
            FOR_ABOVE_PREPARE(op, tmp) {
                object *head = tmp->head ? tmp->head : tmp;

                /* See comment in TRIGGER_BUTTON about move_types */
                if (((head->move_type&op->move_on) || head->move_type == 0)
                && (head->race == op->slaying || (!strcmp(op->slaying, "player") && head->type == PLAYER))) {
                    push = 1;
                    break;
                }
            } FOR_ABOVE_FINISH();
            if (op->stats.ac == push)
                return 0;
            op->stats.ac = push;
            if (NUM_ANIMATIONS(op) > 1) {
                SET_ANIMATION(op, push);
                object_update(op, UP_OBJ_FACE);
            }
            if (in_movement || !push)
                return 0;
        }
        trigger_move(op, push);
        return 0;

    case TRIGGER_ALTAR:
        if (cause) {
            if (in_movement)
                return 0;
            if (operate_altar(op, &cause)) {
                if (NUM_ANIMATIONS(op) > 1) {
                    SET_ANIMATION(op, 1);
                    object_update(op, UP_OBJ_FACE);
                }

                if (op->last_sp >= 0) {
                    trigger_move(op, 1);
                    if (op->last_sp > 0)
                        op->last_sp = -op->last_sp;
                    } else {
                    /* for trigger altar with last_sp, the ON/OFF
                     * status (-> +/- value) is "simulated":
                     */
                    op->value = !op->value;
                    trigger_move(op, 1);
                    op->last_sp = -op->last_sp;
                        op->value = !op->value;
                }
                return cause == NULL;
            } else {
                return 0;
            }
        } else {
            if (NUM_ANIMATIONS(op) > 1) {
                SET_ANIMATION(op, 0);
                object_update(op, UP_OBJ_FACE);
            }

            /* If trigger_altar has "last_sp > 0" set on the map,
             * it will push the connected value only once per sacrifice.
             * Otherwise (default), the connected value will be
             * pushed twice: First by sacrifice, second by reset! -AV
             */
            if (!op->last_sp)
                trigger_move(op, 0);
            else {
                op->stats.wc = 0;
                op->value = !op->value;
                op->speed = 0;
                object_update_speed(op);
            }
        }
        return 0;

    case TRIGGER:
        if (cause) {
            if (in_movement)
                return 0;
            push = 1;
        }
        if (NUM_ANIMATIONS(op) > 1) {
            SET_ANIMATION(op, push);
            object_update(op, UP_OBJ_FACE);
        }
        trigger_move(op, push);
        return 1;

    default:
        LOG(llevDebug, "Unknown trigger type: %s (%d)\n", op->name, op->type);
        return 0;
    }
}

/**
 * Links specified object in the map.
 * @param button
 * object to link. Must not be NULL.
 * @param map
 * map we are on. Should not be NULL.
 * @param connected
 * connection value for the item.
 */
void add_button_link(object *button, mapstruct *map, int connected) {
    oblinkpt *obp;
    objectlink *ol = get_objectlink();

    if (!map) {
        LOG(llevError, "Tried to add button-link without map.\n");
        free_objectlink(ol);
        return;
    }

    SET_FLAG(button, FLAG_IS_LINKED);

    ol->ob = button;
    ol->id = button->count;

    for (obp = map->buttons; obp && obp->value != connected; obp = obp->next)
        ;

    if (obp) {
        ol->next = obp->link;
        obp->link = ol;
    } else {
        obp = get_objectlinkpt();
        obp->value = connected;

        obp->next = map->buttons;
        map->buttons = obp;
        obp->link = ol;
    }
}

/**
 * Remove the object from the linked lists of buttons in the map.
 * This is only needed by editors.
 * @param op
 * object to remove. Must be on a map, and linked.
 */
void remove_button_link(object *op) {
    oblinkpt *obp;
    objectlink **olp, *ol;

    if (op->map == NULL) {
        LOG(llevError, "remove_button_link() in object without map.\n");
        return;
    }
    if (!QUERY_FLAG(op, FLAG_IS_LINKED)) {
        LOG(llevError, "remove_button_linked() in unlinked object.\n");
        return;
    }

    for (obp = op->map->buttons; obp; obp = obp->next)
        for (olp = &obp->link; (ol = *olp); olp = &ol->next)
            if (ol->ob == op) {
/*              LOG(llevDebug, "Removed link %d in button %s and map %s.\n",
                    obp->value, op->name, op->map->path);
*/
                *olp = ol->next;
                free(ol);
                return;
            }
    LOG(llevError, "remove_button_linked(): couldn't find object.\n");
    CLEAR_FLAG(op, FLAG_IS_LINKED);
}

/**
 * Return the first objectlink in the objects linked to this one
 * @param button
 * object to check. Must not be NULL.
 * @return
 * ::objectlink for this object, or NULL.
 */
static objectlink *get_button_links(const object *button) {
    oblinkpt *obp;
    objectlink *ol;

    if (!button->map)
        return NULL;

    for (obp = button->map->buttons; obp; obp = obp->next)
        for (ol = obp->link; ol; ol = ol->next)
            if (ol->ob == button && ol->id == button->count)
                return obp->link;
    return NULL;
}

/**
 * Returns the first value linked to this button.
 * Made as a separate function to increase efficiency
 * @param button
 * object to check. Must not be NULL.
 * @return
 * connection value, or 0 if not connected.
 */
int get_button_value(const object *button) {
    oblinkpt *obp;
    objectlink *ol;

    if (!button->map)
        return 0;

    for (obp = button->map->buttons; obp; obp = obp->next)
        for (ol = obp->link; ol; ol = ol->next)
            if (ol->ob == button && ol->id == button->count)
                return obp->value;
    return 0;
}

/**
 * Checks object and its inventory for specific item.
 *
 * It will descend through containers to find the object.
 *              slaying = match object slaying flag
 *              race = match object archetype name flag
 *              hp = match object type (excpt type '0'== PLAYER)
 *      title = match object title
 * Searching by title only is not recommended, as it can be a rather slow
 * operation; use it in combination with archetype or type.

 * @param op
 * object of which to search inventory
 * @param trig
 * what to search
 * @return
 * object that matches, or NULL if none matched.
 */
object *check_inv_recursive(object *op, const object *trig) {
    object *ret = NULL;

    /* First check the object itself. */
    if ((!trig->stats.hp || (op->type == trig->stats.hp))
    && (!trig->slaying || (op->slaying == trig->slaying))
    && (!trig->race || (op->arch->name == trig->race))
    && (!trig->title || (op->title == trig->title)))
        return op;

    FOR_INV_PREPARE(op, tmp) {
        if (tmp->inv) {
            ret = check_inv_recursive(tmp, trig);
            if (ret)
                return ret;
        } else if ((!trig->stats.hp || (tmp->type == trig->stats.hp))
        && (!trig->slaying || (tmp->slaying == trig->slaying))
        && (!trig->race || (tmp->arch->name == trig->race))
        && (!trig->title || (tmp->title == trig->title)))
            return tmp;
    } FOR_INV_FINISH();

    return NULL;
}

/**
 * Function to search the inventory,
 * of a player and then based on a set of conditions,
 * the square will activate connected items.
 *
 * Monsters can't trigger this square (for now)
 * Values are:   last_sp = 1/0 obj/no obj triggers
 *               last_heal = 1/0  remove/dont remove obj if triggered
 * -b.t. (thomas@nomad.astro.psu.edu
 *
 * @param op
 * object to check. Must be a player.
 * @param trig
 * trigger object that may be activated.
 */
void check_inv(object *op, object *trig) {
    object *match;

    if (op->type != PLAYER)
        return;

    match = check_inv_recursive(op, trig);
    if (match && trig->last_sp) {
        if (trig->last_heal)
            object_decrease_nrof_by_one(match);
        use_trigger(trig);
    } else if (!match && !trig->last_sp)
        use_trigger(trig);
}

/**
 * This does a minimal check of the button link consistency for object
 * map.  All it really does it much sure the object id link that is set
 * matches what the object has.
 *
 * Will log to error level.
 *
 * @param map
 * map to check.
 */
void verify_button_links(const mapstruct *map) {
    oblinkpt *obp;
    objectlink *ol;

    if (!map)
        return;

    for (obp = map->buttons; obp; obp = obp->next) {
        for (ol = obp->link; ol; ol = ol->next) {
            if (ol->id != ol->ob->count)
                LOG(llevError, "verify_button_links: object %s on list is corrupt (%u!=%u)\n", ol->ob->name, ol->id, ol->ob->count);
        }
    }
}
