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
 * Those functions are used by DMs.
 * @todo explain item stack, item specifier for commands.
 */

#include <global.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#endif
#include <spells.h>
#include <treasure.h>
#include <skills.h>

/* Defines for DM item stack **/
#define STACK_SIZE         50   /**< Stack size, static */
/** Values for 'from' field of get_dm_object() */
enum {
    STACK_FROM_NONE    = 0,   /**< Item was not found */
    STACK_FROM_TOP     = 1,   /**< Item is stack top */
    STACK_FROM_STACK   = 2,   /**< Item is somewhere in stack */
    STACK_FROM_NUMBER  = 3    /**< Item is a number (may be top) */
};

/**
 * Enough of the DM functions seem to need this that I broke
 * it out to a seperate function.  name is the person
 * being saught, op is who is looking for them.  This
 * prints diagnostics messages, and returns the
 * other player, or NULL otherwise.
 *
 * @param op
 * player searching someone.
 * @param name
 * name to search for.
 * @return
 * player, or NULL if player can't be found.
 */
static player *get_other_player_from_name(object *op, const char *name) {
    player *pl;

    if (!name)
        return NULL;

    for (pl = first_player; pl != NULL; pl = pl->next)
        if (!strncmp(pl->ob->name, name, MAX_NAME))
            break;

    if (pl == NULL) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "No such player.");
        return NULL;
    }

    if (pl->ob == op) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "You can't do that to yourself.");
        return NULL;
    }
    if (pl->state != ST_PLAYING) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "That player is in no state for that right now.");
        return NULL;
    }
    return pl;
}

/**
 * Remove an item from the wizard's item stack.
 *
 * @param pl
 * wizard.
 */
static void dm_stack_pop(player *pl) {
    if (!pl->stack_items || !pl->stack_position) {
        draw_ext_info(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Empty stack!");
        return;
    }

    pl->stack_position--;
    draw_ext_info_format(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                         "Popped item from stack, %d left.",
                         pl->stack_position);
}

/**
 * Get current stack top item for player.
 * Returns NULL if no stacked item.
 * If stacked item disappeared (freed), remove it.
 *
 * Ryo, august 2004
 *
 * @param pl
 * wizard.
 * @return
 * item on top of stack, or NULL if deleted/stack empty.
 */
static object *dm_stack_peek(player *pl) {
    object *ob;

    if (!pl->stack_position) {
        draw_ext_info(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Empty stack!");
        return NULL;
    }

    ob = object_find_by_tag_global(pl->stack_items[pl->stack_position-1]);
    if (!ob) {
        draw_ext_info(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                      "Stacked item was removed!");
        dm_stack_pop(pl);
        return NULL;
    }

    return ob;
}

/**
 * Push specified item on player stack.
 * Inform player of position.
 * Initializes variables if needed.
 *
 * @param pl
 * wizard.
 * @param item
 * item to put on stack.
 */
static void dm_stack_push(player *pl, tag_t item) {
    if (!pl->stack_items) {
        pl->stack_items = (tag_t *)malloc(sizeof(tag_t)*STACK_SIZE);
        memset(pl->stack_items, 0, sizeof(tag_t)*STACK_SIZE);
    }

    if (pl->stack_position == STACK_SIZE) {
        draw_ext_info(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Item stack full!");
        return;
    }

    pl->stack_items[pl->stack_position] = item;
    draw_ext_info_format(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                         "Item stacked as %d.",
                         pl->stack_position);
    pl->stack_position++;
}

/**
 * Checks 'params' for object code.
 *
 * Can be:
 *  - empty => get current object stack top for player
 *  - number => get item with that tag, stack it for future use
 *  - $number => get specified stack item
 *  - "me" => player himself
 *
 * At function exit, params points to first non-object char
 *
 * 'from', if not NULL, contains at exit:
 *  - ::STACK_FROM_NONE => object not found
 *  - ::STACK_FROM_TOP => top item stack, may be NULL if stack was empty
 *  - ::STACK_FROM_STACK => item from somewhere in the stack
 *  - ::STACK_FROM_NUMBER => item by number, pushed on stack
 *
 * Ryo, august 2004
 *
 * @param pl
 * wizard.
 * @param params
 * object specified.
 * @param from
 * @return
 * pointed object, or NULL if nothing suitable was found.
 */
static object *get_dm_object(player *pl, const char **params, int *from) {
    int item_tag, item_position;
    object *ob;

    if (!pl)
        return NULL;

    if (**params == '\0') {
        if (from)
            *from = STACK_FROM_TOP;
        /* No parameter => get stack item */
        return dm_stack_peek(pl);
    }

    /* Let's clean white spaces */
    while (**params == ' ')
        (*params)++;

    /* Next case: number => item tag */
    if (sscanf(*params, "%d", &item_tag)) {
        /* Move parameter to next item */
        while (isdigit(**params))
            (*params)++;

        /* And skip blanks, too */
        while (**params == ' ')
            (*params)++;

        /* Get item */
        ob = object_find_by_tag_global(item_tag);
        if (!ob) {
            if (from)
                *from = STACK_FROM_NONE;
            draw_ext_info_format(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                                 "No such item %d!",
                                 item_tag);
            return NULL;
        }

        /* Got one, let's push it on stack */
        dm_stack_push(pl, item_tag);
        if (from)
            *from = STACK_FROM_NUMBER;
        return ob;
    }

    /* Next case: $number => stack item */
    if (sscanf(*params, "$%d", &item_position)) {
        /* Move parameter to next item */
        (*params)++;

        while (isdigit(**params))
            (*params)++;
        while (**params == ' ')
            (*params)++;

        if (item_position >= pl->stack_position) {
            if (from)
                *from = STACK_FROM_NONE;
            draw_ext_info_format(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                                 "No such stack item %d!",
                                 item_position);
            return NULL;
        }

        ob = object_find_by_tag_global(pl->stack_items[item_position]);
        if (!ob) {
            if (from)
                *from = STACK_FROM_NONE;
            draw_ext_info_format(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                                 "Stack item %d was removed.",
                                 item_position);
            return NULL;
        }

        if (from)
            *from = item_position < pl->stack_position-1 ? STACK_FROM_STACK : STACK_FROM_TOP;
        return ob;
    }

    /* Next case: 'me' => return pl->ob */
    if (!strncmp(*params, "me", 2)) {
        if (from)
            *from = STACK_FROM_NUMBER;
        dm_stack_push(pl, pl->ob->count);

        /* Skip to next token */
        (*params) += 2;
        while (**params == ' ')
            (*params)++;

        return pl->ob;
    }

    /* Last case: get stack top */
    if (from)
        *from = STACK_FROM_TOP;
    return dm_stack_peek(pl);
}

/**
 * This command will stress server.
 *
 * It will basically load all world maps (so 900 maps).
 *
 * @param op
 * DM wanting to test the server.
 * @param params
 * option, must be "TRUE" for the test to happen.
 */
void command_loadtest(object *op, const char *params) {
    uint32 x, y;
    char buf[1024];

    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DEBUG,
                  "loadtest will stress server through teleporting at different map places. "
                  "Use at your own risk.  Very long loop used so server may have to be reset. "
                  "type loadtest TRUE to run");
    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DEBUG,
                         "{%s}",
                         params);
    if (*params == '\0')
        return;
    if (strncmp(params, "TRUE", 4))
        return;

    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DEBUG,
                         "gogogo");

    for (x = 0; x < settings.worldmaptilesx; x++) {
        for (y = 0; y < settings.worldmaptilesy; y++) {
            snprintf(buf, sizeof(buf), "/world/world_%u_%u", x+settings.worldmapstartx, y+settings.worldmapstarty);
            command_goto(op, buf);
        }
    }
}

/**
 * Actually hides or unhides specified player (obviously a DM).
 *
 * @param op
 * DM hiding.
 * @param silent_dm
 * if non zero, other players are informed of DM entering/leaving, else they just think someone left/entered.
 */
static void do_wizard_hide(object *op, int silent_dm) {
    if (op->contr->hidden) {
        op->contr->hidden = 0;
        op->invisible = 1;
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                      "You are no longer hidden from other players");
        op->map->players++;
        draw_ext_info_format(NDI_UNIQUE|NDI_ALL|NDI_DK_ORANGE, 5, NULL,
                             MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_PLAYER,
                             "%s has entered the game.",
                             op->name);
        if (!silent_dm) {
            draw_ext_info(NDI_UNIQUE|NDI_ALL|NDI_LT_GREEN, 1, NULL,
                          MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM,
                          "The Dungeon Master has arrived!");
        }
    } else {
        op->contr->hidden = 1;
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                      "Other players will no longer see you.");
        op->map->players--;
        if (!silent_dm) {
            draw_ext_info(NDI_UNIQUE|NDI_ALL|NDI_LT_GREEN, 1, NULL,
                          MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM,
                          "The Dungeon Master is gone..");
        }
        draw_ext_info_format(NDI_UNIQUE|NDI_ALL|NDI_DK_ORANGE, 5, NULL,
                             MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_PLAYER,
                             "%s leaves the game.",
                             op->name);
        draw_ext_info_format(NDI_UNIQUE|NDI_ALL|NDI_DK_ORANGE, 5, NULL,
                             MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_PLAYER,
                             "%s left the game.",
                             op->name);
    }
}

/**
 * Wizard 'hide' command.
 *
 * @param op
 * DM wanting to hide.
 * @param params
 * ignored.
 */
void command_hide(object *op, const char *params) {
    do_wizard_hide(op, 0);
}

/**
 * This finds and returns the object which matches the name or
 * object number (specified via num \#whatever).
 *
 * @param params
 * object to find.
 * @return
 * suitable object, or NULL if none found.
 */
static object *find_object_both(const char *params) {
    if (params[0] == '#')
        return object_find_by_tag_global(atol(params+1));
    else
        return object_find_by_name_global(params);
}

/**
 * Sets the god for some objects.
 *
 * @param op
 * DM wanting to change an object.
 * @param params
 * command options. Should contain two values, first the object to change, followed by the god to change it to.
 */
void command_setgod(object *op, const char *params) {
    object *ob;
    const object *god;
    char *str;

    if (*params == '\0' || !(str = strchr(params, ' '))) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Usage: setgod object god");
        return;
    }

    /* kill the space, and set string to the next param */
    *str++ = '\0';
    if (!(ob = find_object_both(params))) {
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                             "Set whose god - can not find object %s?",
                             params);
        return;
    }

    /*
     * Perhaps this is overly restrictive?  Should we perhaps be able
     * to rebless altars and the like?
     */
    if (ob->type != PLAYER) {
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                             "%s is not a player - can not change its god",
                             ob->name);
        return;
    }

    god = find_god(str);
    if (god == NULL) {
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                             "No such god %s.",
                             str);
        return;
    }

    become_follower(ob, god);
}

/**
 * Kicks a player from the server.
 *
 * @param op
 * DM kicking.
 * @param params
 * player to kick. Must be a full name match.
 */
static void command_kick2(object *op, const char *params) {
    struct pl *pl;

    for (pl = first_player; pl != NULL; pl = pl->next) {
        if ((*params == '\0' || !strcmp(pl->ob->name, params)) && pl->ob != op) {
            object *op;
            int removed = 0;

            op = pl->ob;
            if (!QUERY_FLAG(op, FLAG_REMOVED)) {
                /* Avion : Here we handle the KICK global event */
                execute_global_event(EVENT_KICK, op, *params == '\0' ? NULL : params);
                object_remove(op);
                removed = 1;
            }
            op->direction = 0;
            draw_ext_info_format(NDI_UNIQUE|NDI_ALL|NDI_RED, 5, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM,
                                 "%s is kicked out of the game.",
                                 op->name);
            strcpy(op->contr->killer, "left");
            hiscore_check(op, 0); /* Always check score */

            /*
             * not sure how the player would be freed, but did see
             * a crash here - if that is the case, don't save the
             * the player.
             */
            if (!removed && !QUERY_FLAG(op, FLAG_FREED)) {
                (void)save_player(op, 0);
                if (op->map)
                    op->map->players--;
            }
#if MAP_MAXTIMEOUT
            if (op->map)
                op->map->timeout = MAP_TIMEOUT(op->map);
#endif
            pl->socket.status = Ns_Dead;
        }
    }
}

/**
 * Add player's IP to ban_file and kick them off the server.
 *
 * I know most people have dynamic IPs but this is more of a short term
 * solution if they have to get a new IP to play maybe they'll calm down.
 * This uses the banish_file in the local directory *not *the ban_file
 * The action is logged with a ! for easy searching. -tm
 *
 * @param op
 * DM banishing.
 * @param params
 * player to banish. Must be a complete name match.
 * @return
 * 1.
 */
void command_banish(object *op, const char *params) {
    player *pl;
    FILE *banishfile;
    char buf[MAX_BUF];
    time_t now;

    if (*params == '\0') {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Usage: banish <player>.");
        return;
    }

    pl = get_other_player_from_name(op, params);
    if (!pl)
        return;

    snprintf(buf, sizeof(buf), "%s/%s", settings.localdir, BANISHFILE);

    if ((banishfile = fopen(buf, "a")) == NULL) {
        LOG(llevDebug, "Could not find file banish_file.\n");
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Could not find banish_file.");
        return;
    }

    now = time(NULL);
    /*
     * Record this as a comment - then we don't have to worry about changing
     * the parsing code.
     */
    fprintf(banishfile, "# %s (%s) banned by %s at %s\n", pl->ob->name, pl->socket.host, op->name, ctime(&now));
    fprintf(banishfile, "*@%s\n", pl->socket.host);
    fclose(banishfile);

    LOG(llevDebug, "! %s banned %s from IP: %s.\n", op->name, pl->ob->name, pl->socket.host);

    draw_ext_info_format(NDI_UNIQUE|NDI_RED, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                         "You banish %s",
                         pl->ob->name);

    draw_ext_info_format(NDI_UNIQUE|NDI_ALL|NDI_RED, 5, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM,
                         "%s banishes %s from the land!",
                         op->name, pl->ob->name);
    command_kick2(op, pl->ob->name);
}

/**
 * Kicks a player from the server.
 *
 * @param op
 * DM kicking.
 * @param params
 * player to kick. Must be a full name match.
 */
void command_kick(object *op, const char *params) {
    command_kick2(op, params);
}

/**
 * Saves the op's map as an overlay - objects are persisted.
 *
 * @param op
 * DM wanting to save.
 * @param params
 * ignored.
 */
void command_overlay_save(object *op, const char *params) {
    if (!op)
        return;

    if (save_map(op->map, SAVE_MODE_OVERLAY) < 0)
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                      "Overlay save error!");
    else
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                      "Current map has been saved as an overlay.");
}

/**
 * Removes the overlay for op's current map.
 *
 * @param op
 * DM acting.
 * @param params
 * ignored.
 */
void command_overlay_reset(object *op, const char *params) {
    char filename[MAX_BUF];
    struct stat stats;

    create_overlay_pathname(op->map->path, filename, MAX_BUF);
    if (!stat(filename, &stats))
        if (!unlink(filename))
            draw_ext_info(NDI_UNIQUE, 0, op,  MSG_TYPE_COMMAND,  MSG_TYPE_COMMAND_DM,
                          "Overlay successfully removed.");
        else
            draw_ext_info(NDI_UNIQUE, 0, op,  MSG_TYPE_COMMAND,  MSG_TYPE_COMMAND_DM,
                          "Overlay couldn't be removed.");
    else
        draw_ext_info(NDI_UNIQUE, 0, op,  MSG_TYPE_COMMAND,  MSG_TYPE_COMMAND_DM,
                      "No overlay for current map.");
}

/**
 * A simple toggle for the no_shout field. AKA the MUZZLE command.
 *
 * @param op
 * wizard toggling.
 * @param params
 * player to mute/unmute.
 */
void command_toggle_shout(object *op, const char *params) {
    player *pl;

    if (*params == '\0') {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Usage: toggle_shout <player>.");
        return;
    }

    pl = get_other_player_from_name(op, params);
    if (!pl)
        return;

    if (pl->ob->contr->no_shout == 0) {
        pl->ob->contr->no_shout = 1;

        draw_ext_info(NDI_UNIQUE|NDI_RED, 0, pl->ob, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM,
                      "You have been muzzled by the DM!");
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                             "You muzzle %s.",
                             pl->ob->name);

        /* Avion : Here we handle the MUZZLE global event */
        execute_global_event(EVENT_MUZZLE, pl->ob, params);

        return;
    }

    pl->ob->contr->no_shout = 0;
    draw_ext_info(NDI_UNIQUE|NDI_ORANGE, 0, pl->ob, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM,
                  "You are allowed to shout and chat again.");
    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                         "You remove %s's muzzle.",
                         pl->ob->name);
}

/**
 * Totally shutdowns the server.
 *
 * @param op
 * wizard shutting down the server.
 * @param params
 * ignored.
 */
void command_shutdown(object *op, const char *params) {
    /*
     * We need to give op - command_kick expects it.  however, this means
     * the op won't get kicked off, so we do it ourselves
     */
    command_kick2(op, "");
    hiscore_check(op, 0); /* Always check score */
    (void)save_player(op, 0);
    play_again(op);
    cleanup();
    /* not reached */
}

/**
 * Wizard teleports to a map.
 *
 * @param op
 * wizard teleporting.
 * @param params
 * map to teleport to. Can be absolute or relative path.
 */
void command_goto(object *op, const char *params) {
    const char *name;
    object *dummy;

    if (!op)
        return ;

    if (*params == '\0') {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Go to what level?");
        return;
    }

    name = params;
    dummy = object_new();
    dummy->map = op->map;
    EXIT_PATH(dummy) = add_string(name);
    EXIT_X(dummy) = -1;
    EXIT_Y(dummy) = -1;
    dummy->name = add_string(name);

    enter_exit(op, dummy);
    object_free2(dummy, FREE_OBJ_NO_DESTROY_CALLBACK);
    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                         "Difficulty: %d.",
                         op->map->difficulty);
}

/**
 * Freezes a player for a specified tick count, 100 by default.
 *
 * @param op
 * wizard freezing the player.
 * @param params
 * optional tick count, followed by player name.
 */
void command_freeze(object *op, const char *params) {
    int ticks;
    player *pl;

    if (*params == '\0') {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Usage: freeze [ticks] <player>.");
        return;
    }

    ticks = atoi(params);
    if (ticks) {
        while ((isdigit(*params) || isspace(*params)) && *params != 0)
            params++;
        if (*params == 0) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                          "Usage: freeze [ticks] <player>.");
            return;
        }
    } else
        ticks = 100;

    pl = get_other_player_from_name(op, params);
    if (!pl)
        return;

    draw_ext_info(NDI_UNIQUE|NDI_RED, 0, pl->ob, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM,
                  "You have been frozen by the DM!");

    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                         "You freeze %s for %d ticks",
                         pl->ob->name, ticks);

    pl->ob->speed_left = -(pl->ob->speed*ticks);
}

/**
 * Put a player into jail, taking into account cursed exits and player's region.
 * @param who player to put in jail
 * @retval 0 player was moved to jail.
 * @retval -1 no jail found.
 * @retval -2 couldn't move to jail (map loading error, or already at jail's position).
 * @retval -3 op isn't a player.
 */
int player_arrest(object *who) {
    object *dummy;
    mapstruct *cur;
    int x, y;

    if (who->type != PLAYER)
        return -3;

    dummy = get_jail_exit(who);
    if (!dummy) {
        return -1;
    }
    cur = who->map;
    x = who->x;
    y = who->y;
    enter_exit(who, dummy);
    object_free2(dummy, FREE_OBJ_NO_DESTROY_CALLBACK);

    if (cur == who->map && x == who->x && y == who->y)
        return -2;

    return 0;
}

/**
 * Wizard jails player.
 *
 * @param op
 * wizard.
 * @param params
 * player to jail.
 */
void command_arrest(object *op, const char *params) {
    player *pl;
    int ret;

    if (!op)
        return;
    if (*params == '\0') {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Usage: arrest <player>.");
        return;
    }
    pl = get_other_player_from_name(op, params);
    if (!pl)
        return;

    ret = player_arrest(pl->ob);
    if (ret == -1) {
        /* we have nowhere to send the prisoner....*/
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Can't jail player, there is no map to hold them");
        return;
    }
    if (ret == -2) {
        /* something prevented jailing the player */
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
          "Can't jail player, map loading issue or already in jail's position");
        return;

    }

    draw_ext_info(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM,
                  "You have been arrested.");
    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                         "Jailed %s",
                         pl->ob->name);
    LOG(llevInfo, "Player %s arrested by %s\n", pl->ob->name, op->name);
}

/**
 * Summons player near DM.
 * @param op
 * DM.
 * @param params
 * player to summon.
 */
void command_summon(object *op, const char *params) {
    int i;
    object *dummy;
    player *pl;

    if (!op)
        return;

    if (*params == '\0') {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Usage: summon <player>.");
        return;
    }

    pl = get_other_player_from_name(op, params);
    if (!pl)
        return;

    i = object_find_free_spot(op, op->map, op->x, op->y, 1, 9);
    if (i == -1) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Can not find a free spot to place summoned player.");
        return;
    }

    dummy = object_new();
    EXIT_PATH(dummy) = add_string(op->map->path);
    EXIT_X(dummy) = op->x+freearr_x[i];
    EXIT_Y(dummy) = op->y+freearr_y[i];
    enter_exit(pl->ob, dummy);
    object_free2(dummy, FREE_OBJ_NO_DESTROY_CALLBACK);
    draw_ext_info(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM,
                  "You are summoned.");
    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                         "You summon %s",
                         pl->ob->name);
}

/**
 * Teleport next to target player.
 *
 * @param op
 * DM teleporting.
 * @param params
 * options sent by player.
 */
/* mids 01/16/2002 */
void command_teleport(object *op, const char *params) {
    int i;
    object *dummy;
    player *pl;

    if (!op)
        return;

    if (*params == '\0') {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Usage: teleport <player>.");
        return;
    }

    pl = find_player_partial_name(params);
    if (!pl) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "No such player or ambiguous name.");
        return;
    }

    i = object_find_free_spot(pl->ob, pl->ob->map, pl->ob->x, pl->ob->y, 1, 9);
    if (i == -1) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Can not find a free spot to teleport to.");
        return;
    }

    dummy = object_new();
    EXIT_PATH(dummy) = add_string(pl->ob->map->path);
    EXIT_X(dummy) = pl->ob->x+freearr_x[i];
    EXIT_Y(dummy) = pl->ob->y+freearr_y[i];
    enter_exit(op, dummy);
    object_free2(dummy, FREE_OBJ_NO_DESTROY_CALLBACK);
    if (!op->contr->hidden)
        draw_ext_info(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM,
                      "You see a portal open.");
    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                         "You teleport to %s",
                         pl->ob->name);
}

/**
 * Wizard wants to create an object.
 *
 * This function is a real mess, because we're stucking getting
 * the entire item description in one block of text, so we just
 * can't simply parse it - we need to look for double quotes
 * for example.  This could actually get much simpler with just a
 * little help from the client - if we could get line breaks, it
 * makes parsing much easier, eg, something like:
 * - arch dragon
 * - name big nasty creature
 * - hp 5
 * - sp 30
 *
 * which is much easier to parse than
 * dragon name "big nasty creature" hp 5 sp 30
 * for example.
 *
 * @param op
 * wizard.
 * @param params
 * object description.
 * @todo enable line breaks in command.
 */
void command_create(object *op, const char *params) {
    object *tmp = NULL;
    uint32 i;
    int magic, set_magic = 0, set_nrof = 0, gotquote, gotspace;
    uint32 nrof;
    char *cp, *bp, *bp2, *bp3, *endline, cpy[MAX_BUF];
    archetype *at, *at_spell = NULL;
    const artifact *art = NULL;

    if (!op)
        return;

    if (*params == '\0') {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Usage: create [nr] [magic] <archetype> [ of <artifact>] [variable_to_patch setting]");
        return;
    }
    strncpy(cpy, params, sizeof(cpy));
    bp = cpy;

    /* We need to know where the line ends */
    endline = bp+strlen(bp);

    if (sscanf(bp, "%u ", &nrof)) {
        if ((bp = strchr(cpy, ' ')) == NULL) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                          "Usage: create [nr] [magic] <archetype> [ of <artifact>] [variable_to_patch setting]");
            return;
        }
        bp++;
        set_nrof = 1;
        LOG(llevDebug, "%s creates: (%u) %s\n", op->name, nrof, bp);
    }
    if (sscanf(bp, "%d ", &magic)) {
        if ((bp = strchr(bp, ' ')) == NULL) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                          "Usage: create [nr] [magic] <archetype> [ of <artifact>] [variable_to_patch setting]");
            return;
        }
        bp++;
        set_magic = 1;
        LOG(llevDebug, "%s creates: (%d) (%d) %s\n", op->name, nrof, magic, bp);
    }
    if ((cp = strstr(bp, " of ")) != NULL) {
        *cp = '\0';
        cp += 4;
    }
    for (bp2 = bp; *bp2; bp2++) {
        if (*bp2 == ' ') {
            *bp2 = '\0';
            bp2++;
            break;
        }
    }

    if ((at = try_find_archetype(bp)) == NULL) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "No such archetype.");
        return;
    }

    if (cp) {
        char spell_name[MAX_BUF], *fsp = NULL;

        /*
         * Try to find a spell object for this. Note that
         * we also set up spell_name which is only
         * the first word.
         */

        at_spell = try_find_archetype(cp);
        if (!at_spell || at_spell->clone.type != SPELL)
            at_spell = find_archetype_by_object_name(cp);
        if (!at_spell || at_spell->clone.type != SPELL) {
            strcpy(spell_name, cp);
            fsp = strchr(spell_name, ' ');
            if (fsp) {
                *fsp = 0;
                fsp++;
                at_spell = try_find_archetype(spell_name);

                /* Got a spell, update the first string pointer */
                if (at_spell && at_spell->clone.type == SPELL)
                    bp2 = cp+strlen(spell_name)+1;
                else
                    at_spell = NULL;
            } else
                at_spell = NULL;
        }

        /* OK - we didn't find a spell - presume the 'of'
         * in this case means its an artifact.
         */
        if (!at_spell) {
            if (find_artifactlist(at->clone.type) == NULL) {
                draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                                     "No artifact list for type %d\n",
                                     at->clone.type);
            } else {
                art = find_artifactlist(at->clone.type)->items;

                do {
                    if (!strcmp(art->item->name, cp) && legal_artifact_combination(&at->clone, art))
                        break;
                    art = art->next;
                } while (art != NULL);
                if (!art) {
                    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                                         "No such artifact ([%d] of %s)",
                                         at->clone.type, cp);
                }
            }
            LOG(llevDebug, "%s creates: (%d) (%d) (%s) of (%s)\n", op->name, set_nrof ? nrof : 0, set_magic ? magic : 0, bp, cp);
        }
    } /* if cp */

    /* rods and potions can get their spell from the artifact */
    if ((at->clone.type == ROD || at->clone.type == POTION) && !at_spell && (!art || !art->item->other_arch)) {
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                             "Unable to find spell %s for object that needs it, or it is of wrong type",
                             cp);
        return;
    }
    if ((at->clone.type == WAND || at->clone.type == SCROLL || at->clone.type == SPELLBOOK)
    && !at_spell) {
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                             "Unable to find spell %s for object that needs it, or it is of wrong type",
                             cp);
        return;
    }

    /*
     * Rather than have two different blocks with a lot of similar code,
     * just create one object, do all the processing, and then determine
     * if that one object should be inserted or if we need to make copies.
     */
    tmp = object_create_arch(at);
    if (settings.real_wiz == FALSE)
        SET_FLAG(tmp, FLAG_WAS_WIZ);
    if (set_magic)
        set_abs_magic(tmp, magic);
    if (art)
        give_artifact_abilities(tmp, art->item);
    if (need_identify(tmp)) {
        SET_FLAG(tmp, FLAG_IDENTIFIED);
        CLEAR_FLAG(tmp, FLAG_KNOWN_MAGICAL);
        object_give_identified_properties(tmp);
    }

    /*
     * This entire block here tries to find variable pairings,
     * eg, 'hp 4' or the like.  The mess here is that values
     * can be quoted (eg "my cool sword");  So the basic logic
     * is we want to find two spaces, but if we got a quote,
     * any spaces there don't count.
     */
    while (*bp2 && bp2 <= endline) {
        gotspace = 0;
        gotquote = 0;
        /* find the first quote */
        for (bp3 = bp2; *bp3 && gotspace < 2 && gotquote < 2; bp3++) {
            /* Found a quote - now lets find the second one */
            if (*bp3 == '"') {
                *bp3 = ' ';
                bp2 = bp3+1;    /* Update start of string */
                bp3++;
                gotquote++;
                while (*bp3) {
                    if (*bp3 == '"') {
                        *bp3 = '\0';
                        gotquote++;
                    } else
                        bp3++;
                }
            } else if (*bp3 == ' ') {
                gotspace++;
            }
        }

        /*
         * If we got two spaces, send the second one to null.
         * if we've reached the end of the line, increase gotspace -
         * this is perfectly valid for the list entry listed.
         */
        if (gotspace == 2 || gotquote == 2) {
            bp3--;      /* Undo the extra increment */
            *bp3 = '\0';
        } else if (*bp3 == '\0')
            gotspace++;

        if ((gotquote && gotquote != 2)
        || (gotspace != 2 && gotquote != 2)) {
            /*
             * Unfortunately, we've clobbered lots of values, so printing
             * out what we have probably isn't useful.  Break out, because
             * trying to recover is probably won't get anything useful
             * anyways, and we'd be confused about end of line pointers
             * anyways.
             */
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                                 "Malformed create line: %s",
                                 bp2);
            break;
        }
        /* bp2 should still point to the start of this line,
         * with bp3 pointing to the end
         */
        if (set_variable(tmp, bp2) == -1)
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                                 "Unknown variable %s",
                                 bp2);
        else
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                                 "(%s#%d)->%s",
                                 tmp->name, tmp->count, bp2);
        bp2 = bp3+1;
    }

    if (at->clone.nrof) {
        if (at_spell)
            object_insert_in_ob(arch_to_object(at_spell), tmp);

        if (set_nrof)
            tmp->nrof = nrof;

        if (at->clone.randomitems != NULL && !at_spell) {
            create_treasure(at->clone.randomitems, tmp, 0, op->map->difficulty, 0);
            if (QUERY_FLAG(tmp, FLAG_MONSTER)) {
                monster_check_apply_all(tmp);
            }
        }

        /* Multipart objects can't be in inventory, put'em on floor. */
        if (!tmp->more) {
            tmp = object_insert_in_ob(tmp, op);
        } else {
            object_insert_in_map_at(tmp, op->map, op, 0, op->x, op->y);
        }

        /* Let's put this created item on stack so dm can access it easily. */
        dm_stack_push(op->contr, tmp->count);

        return;
    }

    for (i = 0; i < (set_nrof ? nrof : 1); i++) {
        archetype *atmp;
        object *prev = NULL, *head = NULL, *dup;

        for (atmp = at; atmp != NULL; atmp = atmp->more) {
            dup = arch_to_object(atmp);

            if (at_spell)
                object_insert_in_ob(arch_to_object(at_spell), dup);

            /*
             * The head is what contains all the important bits,
             * so just copying it over should be fine.
             */
            if (head == NULL) {
                head = dup;
                object_copy(tmp, dup);
            }
            if (settings.real_wiz == FALSE)
                SET_FLAG(dup, FLAG_WAS_WIZ);
            dup->x = op->x+dup->arch->clone.x;
            dup->y = op->y+dup->arch->clone.y;
            dup->map = op->map;

            if (head != dup) {
                dup->head = head;
                prev->more = dup;
            }
            prev = dup;
        }

        if (QUERY_FLAG(head, FLAG_ALIVE)) {
            object *check = head;
            int size_x = 0;
            int size_y = 0;

            while (check) {
                size_x = MAX(size_x, check->arch->clone.x);
                size_y = MAX(size_y, check->arch->clone.y);
                check = check->more;
            }

            if (out_of_map(op->map, head->x+size_x, head->y+size_y)) {
                if (head->x < size_x || head->y < size_y) {
                    dm_stack_pop(op->contr);
                    object_free2(head, FREE_OBJ_NO_DESTROY_CALLBACK);
                    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                                  "Object too big to insert in map, or wrong position.");
                    object_free2(tmp, FREE_OBJ_NO_DESTROY_CALLBACK);
                    return;
                }

                check = head;
                while (check) {
                    check->x -= size_x;
                    check->y -= size_y;
                    check = check->more;
                }
            }

            object_insert_in_map_at(head, op->map, op, 0, head->x, head->y);
        } else
            head = object_insert_in_ob(head, op);

        /* Let's put this created item on stack so dm can access it easily. */
        /* Wonder if we really want to push all of these, but since
         * things like rods have nrof 0, we want to cover those.
         */
        dm_stack_push(op->contr, head->count);

        if (at->clone.randomitems != NULL && !at_spell) {
            create_treasure(at->clone.randomitems, head, 0, op->map->difficulty, 0);
            if (QUERY_FLAG(head, FLAG_MONSTER)) {
                monster_check_apply_all(head);
            }
        }
    }

    /* free the one we used to copy */
    object_free2(tmp, FREE_OBJ_NO_DESTROY_CALLBACK);
}

/*
 * Now follows dm-commands which are also acceptable from sockets
 */

/**
 * Shows the inventory or some item.
 *
 * @param op
 * player.
 * @param params
 * object count to get the inventory of. If NULL then defaults to op.
 */
void command_inventory(object *op, const char *params) {
    object *tmp;
    int i;

    if (*params == '\0') {
        inventory(op, NULL);
        return;
    }

    if (!sscanf(params, "%d", &i) || (tmp = object_find_by_tag_global(i)) == NULL) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Inventory of what object (nr)?");
        return;
    }

    inventory(op, tmp);
}

/**
 * Player is asking for her skills.
 *
 * Just show player's their skills for now. Dm's can
 * already see skills w/ inventory command - b.t.
 *
 * @param op
 * player.
 * @param params
 * optional skill restriction.
 * @todo move out of this file as it is used by all players.
 */
void command_skills(object *op, const char *params) {
    show_skills(op, *params == '\0' ? NULL : params);
}

/**
 * Dumps the difference between an object and its archetype.
 *
 * @param op
 * wiard.
 * @param params
 * object to dump.
 */
void command_dump(object *op, const char *params) {
    object *tmp;
    StringBuffer *sb;
    char *diff;

    tmp = get_dm_object(op->contr, &params, NULL);
    if (!tmp)
        return;

    sb = stringbuffer_new();
    object_dump(tmp, sb);
    diff = stringbuffer_finish(sb);
    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM, diff);
    free(diff);
    if (QUERY_FLAG(tmp, FLAG_OBJ_ORIGINAL))
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                      "Object is marked original");
}

/**
 *  When DM is possessing a monster, flip aggression on and off, to allow
 * better motion.
 *
 * @param op
 * wiard.
 * @param params
 * ignored.
 */
void command_mon_aggr(object *op, const char *params) {
    if (op->enemy || !QUERY_FLAG(op, FLAG_UNAGGRESSIVE)) {
        object_set_enemy(op, NULL);
        SET_FLAG(op, FLAG_UNAGGRESSIVE);
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                      "Aggression turned OFF");
    } else {
        CLEAR_FLAG(op, FLAG_FRIENDLY);
        CLEAR_FLAG(op, FLAG_UNAGGRESSIVE);
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                      "Aggression turned ON");
    }
}

/**
 * DM can possess a monster.  Basically, this tricks the client into thinking
 * a given monster, is actually the player it controls.  This allows a DM
 * to inhabit a monster's body, and run around the game with it.
 * This function is severely broken - it has tons of hardcoded values,
 *
 * @param op
 * wizard wanting to possess something.
 * @param params
 * monster to possess.
 * @todo fix and reactivate the function, or totally trash.
 */
void command_possess(object *op, const char *params) {
    object *victim;
    player *pl;
    int i;
    char buf[MAX_BUF];

    victim = NULL;
    if (*params != '\0') {
        if (sscanf(params, "%d", &i))
            victim = object_find_by_tag_global(i);
        else if (sscanf(params, "%s", buf))
            victim = object_find_by_name_global(buf);
    }
    if (victim == NULL) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Patch what object (nr)?");
        return;
    }

    if (victim == op) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "As insane as you are, I cannot allow you to possess yourself.");
        return;
    }

    /* make the switch */
    pl = op->contr;
    victim->contr = pl;
    pl->ob = victim;
    victim->type = PLAYER;
    SET_FLAG(victim, FLAG_WIZ);

    /* basic patchup */
    /* The use of hard coded values is terrible.  Note
     * that really, to be fair, this shouldn't get changed at
     * all - if you are possessing a kobold, you should have the
     * same limitations.  As it is, as more body locations are added,
     * this will give this player more locations than perhaps
     * they should be allowed.
     */
    for (i = 0; i < NUM_BODY_LOCATIONS; i++)
        if (i == 1 || i == 6 || i == 8 || i == 9)
            victim->body_info[i] = 2;
        else
            victim->body_info[i] = 1;

    esrv_new_player(pl, 80); /* just pick a weight, we don't care */
    esrv_send_inventory(victim, victim);

    fix_object(victim);

    do_some_living(victim);
}

/**
 * Wizard wants to altar an object.
 * @param op
 * wizard.
 * @param params
 * object and what to patch.
 */
void command_patch(object *op, const char *params) {
    const char *arg, *arg2;
    object *tmp;

    tmp = get_dm_object(op->contr, &params, NULL);
    if (!tmp)
        /* Player already informed of failure */
        return;

    /* params set to first value by get_dm_default */
    arg = params;
    if (*arg == '\0') {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Patch what values?");
        return;
    }

    if ((arg2 = strchr(arg, ' ')))
        arg2++;
    if (settings.real_wiz == FALSE)
        SET_FLAG(tmp, FLAG_WAS_WIZ); /* To avoid cheating */
    if (set_variable(tmp, arg) == -1)
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                             "Unknown variable %s",
                             arg);
    else {
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                             "(%s#%d)->%s=%s",
                             tmp->name, tmp->count, arg, arg2);
    }
}

/**
 * Remove an object from its position.
 *
 * @param op
 * wizard.
 * @param params
 * object to remove.
 */
void command_remove(object *op, const char *params) {
    object *tmp;
    int from;

    tmp = get_dm_object(op->contr, &params, &from);
    if (!tmp) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Remove what object (nr)?");
        return;
    }

    if (tmp->type == PLAYER) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Unable to remove a player!");
        return;
    }

    if (QUERY_FLAG(tmp, FLAG_REMOVED)) {
        char name[MAX_BUF];

        query_name(tmp, name, MAX_BUF);
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                             "%s is already removed!",
                             name);
        return;
    }

    if (from != STACK_FROM_STACK)
        /* Item is either stack top, or is a number thus is now stack top, let's remove it  */
        dm_stack_pop(op->contr);

    /* Always work on the head - otherwise object will get in odd state */
    tmp = HEAD(tmp);
    if (tmp->speed != 0) {
        tmp->speed = 0;
        object_update_speed(tmp);
    }
    object_remove(tmp);
}

/**
 * Totally free an object.
 * @param op
 * wizard.
 * @param params
 * object to free.
 */
void command_free(object *op, const char *params) {
    object *tmp;
    int from;

    tmp = get_dm_object(op->contr, &params, &from);

    if (!tmp) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Free what object (nr)?");
        return;
    }

    if (from != STACK_FROM_STACK)
        /* Item is either stack top, or is a number thus is now stack top, let's remove it  */
        dm_stack_pop(op->contr);

    tmp = HEAD(tmp);
    if (!QUERY_FLAG(tmp, FLAG_REMOVED)) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                      "Warning: item was not removed, will do so now.");
        object_remove(tmp);
    }

    object_free_drop_inventory(tmp);
}

/**
 * This adds exp to a player.  We now allow adding to a specific skill.
 *
 * @param op
 * wizard.
 * @param params
 * should be "player quantity [skill]".
 */
void command_addexp(object *op, const char *params) {
    char buf[MAX_BUF], skill[MAX_BUF];
    int i, q;
    object *skillob = NULL;
    player *pl;

    skill[0] = '\0';
    if ((*params == '\0')
    || (strlen(params) > MAX_BUF)
    || ((q = sscanf(params, "%s %d %[^\r\n]", buf, &i, skill)) < 2)) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Usage: addexp player quantity [skill].");
        return;
    }

    for (pl = first_player; pl != NULL; pl = pl->next)
        if (!strncmp(pl->ob->name, buf, MAX_NAME))
            break;

    if (pl == NULL) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "No such player.");
        return;
    }

    if (q >= 3) {
        skillob = find_skill_by_name(pl->ob, skill);
        if (!skillob) {
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                                 "Unable to find skill %s in %s",
                                 skill, buf);
            return;
        }

        i = check_exp_adjust(skillob, i);
        skillob->stats.exp += i;
        calc_perm_exp(skillob);
        player_lvl_adj(pl->ob, skillob);
    }

    pl->ob->stats.exp += i;
    calc_perm_exp(pl->ob);
    player_lvl_adj(pl->ob, NULL);

    if (settings.real_wiz == FALSE)
        SET_FLAG(pl->ob, FLAG_WAS_WIZ);
}

/**
 * Changes the server speed.
 *
 * @param op
 * wizard.
 * @param params
 * new speed, or NULL to see the speed.
 */
void command_speed(object *op, const char *params) {
    int i;

    if (*params == '\0' || !sscanf(params, "%d", &i)) {
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                             "Current speed is %d",
                             max_time);
        return;
    }

    set_max_time(i);
    reset_sleep();
    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                         "The speed is changed to %d.",
                         i);
}

/**************************************************************************/
/* Mods made by Tyler Van Gorder, May 10-13, 1992.                        */
/* CSUChico : tvangod@cscihp.ecst.csuchico.edu                            */
/**************************************************************************/

/**
 * Displays the statistics of a player.
 *
 * @param op
 * wizard.
 * @param params
 * player's name.
 */
void command_stats(object *op, const char *params) {
    player *pl;

    if (*params == '\0') {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Who?");
        return;
    }

    pl = find_player_partial_name(params);
    if (pl == NULL) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "No such player.");
        return;
    }

    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                         "[Fixed]Statistics for %s:", pl->ob->name);

    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                         "[fixed]Str : %-2d      H.P. : %-4d  MAX : %d",
                         pl->ob->stats.Str, pl->ob->stats.hp, pl->ob->stats.maxhp);

    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                         "[fixed]Dex : %-2d      S.P. : %-4d  MAX : %d",
                         pl->ob->stats.Dex, pl->ob->stats.sp, pl->ob->stats.maxsp);

    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                         "[fixed]Con : %-2d        AC : %-4d  WC  : %d",
                         pl->ob->stats.Con, pl->ob->stats.ac, pl->ob->stats.wc);

    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                         "[fixed]Int : %-2d    Damage : %d",
                         pl->ob->stats.Int, pl->ob->stats.dam);

    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                         "[fixed]Wis : %-2d       EXP : %"FMT64,
                         pl->ob->stats.Wis, pl->ob->stats.exp);

    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                         "[fixed]Pow : %-2d    Grace : %d",
                         pl->ob->stats.Pow, pl->ob->stats.grace);

    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                         "[fixed]Cha : %-2d      Food : %d",
                         pl->ob->stats.Cha, pl->ob->stats.food);
}

/**
 * Changes an object's statistics.
 *
 * @param op
 * wizard.
 * @param params
 * parameters, should be "player statistic new_value".
 * @todo use get_other_player_from_name(). Isn't this useless with the command_patch()?
 */
void command_abil(object *op, const char *params) {
    char thing[20], thing2[20];
    int iii;
    player *pl;

    iii = 0;
    thing[0] = '\0';
    thing2[0] = '\0';
    if (*params == '\0'
    || sscanf(params, "%s %s %d", thing, thing2, &iii) != 3
    || thing[0] == '\0') {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Who?");
        return;
    }

    if (thing2[0] == '\0') {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "You can't change that.");
        return;
    }

    if (iii < MIN_STAT || iii > settings.max_stat) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Illegal range of stat.\n");
        return;
    }

    for (pl = first_player; pl != NULL; pl = pl->next) {
        if (!strcmp(pl->ob->name, thing)) {
            if (settings.real_wiz == FALSE)
                SET_FLAG(pl->ob, FLAG_WAS_WIZ);
            if (!strcmp("str", thing2))
                pl->ob->stats.Str = iii, pl->orig_stats.Str = iii;
            if (!strcmp("dex", thing2))
                pl->ob->stats.Dex = iii, pl->orig_stats.Dex = iii;
            if (!strcmp("con", thing2))
                pl->ob->stats.Con = iii, pl->orig_stats.Con = iii;
            if (!strcmp("wis", thing2))
                pl->ob->stats.Wis = iii, pl->orig_stats.Wis = iii;
            if (!strcmp("cha", thing2))
                pl->ob->stats.Cha = iii, pl->orig_stats.Cha = iii;
            if (!strcmp("int", thing2))
                pl->ob->stats.Int = iii, pl->orig_stats.Int = iii;
            if (!strcmp("pow", thing2))
                pl->ob->stats.Pow = iii, pl->orig_stats.Pow = iii;
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                                 "%s has been altered.",
                                 pl->ob->name);
            fix_object(pl->ob);
            return;
        }
    }

    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                  "No such player.");
}

/**
 * Resets a map.
 *
 * @param op
 * wizard.
 * @param params
 * map to reset. Can be "." for current op's map, or a map path.
 */
void command_reset(object *op, const char *params) {
    mapstruct *m;
    object *dummy = NULL, *tmp = NULL;
    char path[HUGE_BUF];
    const char *space, *confirmation = NULL;
    int res = 0;

    if (*params == '\0') {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Reset what map [name]?");
        return;
    }

    space = strchr(params, ' ');
    if (space != NULL) {
        confirmation = params;
        params = space + 1;
    }

    if (strcmp(params, ".") == 0)
        snprintf(path, sizeof(path), "%s", op->map->path);
    else
        path_combine_and_normalize(op->map->path, params, path, sizeof(path));
    m = has_been_loaded(path);
    if (m == NULL) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "No such map.");
        return;
    }

    if (confirmation) {
        if (strcmp(params, ".") == 0 && m->unique) {
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                                 "Can't reset a player unique map while on it, use 'reset full-reset %s' while not on it.",
                                 m->path);
            return;
        }

        if (strncmp("full-reset", confirmation, strlen("full-reset"))) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                          "Invalid confirmation, must be 'full-reset'.");
            return;
        }
    }

    /* Forbid using reset on our own map when we're in a transport, as
     * it has the displeasant effect of crashing the server.
     * - gros, July 25th 2006 */
    if ((op->contr && op->contr->transport) && (op->map == m)) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "You need to disembark first.");
        return;
    }

    snprintf(path, sizeof(path), "%s", m->path);

    if (m->in_memory != MAP_SWAPPED) {
        if (m->in_memory != MAP_IN_MEMORY) {
            LOG(llevError, "Tried to swap out map which was not in memory.\n");
            return;
        }

        /*
         * Only attempt to remove the player that is doing the reset, and not other
         * players or wiz's.
         */
        if (op->map == m) {
            if (strncmp(m->path, "/random/", 8) == 0) {
                /* This is not a very satisfying solution - it would be much better
                 * to recreate a random map with the same seed value as the old one.
                 * Unfortunately, I think recreating the map would require some
                 * knowledge about its 'parent', which appears very non-trivial to
                 * me.
                 * On the other hand, this should prevent the freeze that this
                 * situation caused. - gros, 26th July 2006.
                 */
                draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                              "You cannot reset a random map when inside it.");
                return;
            }

            dummy = object_new();
            dummy->map = NULL;
            EXIT_X(dummy) = op->x;
            EXIT_Y(dummy) = op->y;
            EXIT_PATH(dummy) = add_string(op->map->path);
            object_remove(op);
            op->map = NULL;
            tmp = op;
        }
        res = swap_map(m);
    }

    if (res < 0 || m->in_memory != MAP_SWAPPED) {
        player *pl;
        int playercount = 0;

        /* Need to re-insert player if swap failed for some reason */
        if (tmp) {
            object_insert_in_map_at(op, m, NULL, 0, op->x, op->y);
            object_free2(dummy, FREE_OBJ_NO_DESTROY_CALLBACK);
        }

        if (res < 0 && res != SAVE_ERROR_PLAYER)
            /* no need to warn if player on map, code below checks that. */
            draw_ext_info_format(NDI_UNIQUE|NDI_RED, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                                 "Reset failed, error code: %d.", res);
        else {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                          "Reset failed, couldn't swap map, the following players are on it:");
            for (pl = first_player; pl != NULL; pl = pl->next) {
                if (pl->ob->map == m && pl->ob != op) {
                    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                                  pl->ob->name);
                    playercount++;
                }
            }
            if (!playercount)
                draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                              "hmm, I don't see any other players on this map, something else is the problem.");
            return;
        }
    }

    /* Here, map reset succeeded. */

    if (m && m->in_memory == MAP_SWAPPED) {

        if (confirmation) {
            map_remove_unique_files(m);
            LOG(llevDebug, "DM %s fully resetting map %s.\n", op->name, m->path);
        } else
            LOG(llevDebug, "DM %s resetting map %s.\n", op->name, m->path);

        /* setting this effectively causes an immediate reload */
        m->reset_time = 1;
        flush_old_maps();
    }

    if (confirmation)
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                             "Fully resetting map %s.",
                             path);
    else
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                             "Resetting map %s.",
                             path);

    if (tmp) {
        enter_exit(tmp, dummy);
        object_free2(dummy, FREE_OBJ_NO_DESTROY_CALLBACK);
    }

    if (confirmation == NULL) {
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                         "Use 'reset full-reset %s' to fully reset the map.", params);
    }
}

/**
 * Steps down from wizard mode.
 *
 * @param op
 * wizard.
 * @param params
 * ignored.
 */
void command_nowiz(object *op, const char *params) { /* 'noadm' is alias */
    CLEAR_FLAG(op, FLAG_WIZ);
    CLEAR_FLAG(op, FLAG_WIZPASS);
    CLEAR_FLAG(op, FLAG_WIZCAST);
    if (op->contr->followed_player)
        FREE_AND_CLEAR_STR(op->contr->followed_player);

    if (settings.real_wiz == TRUE)
        CLEAR_FLAG(op, FLAG_WAS_WIZ);
    if (op->contr->hidden) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                      "You are no longer hidden from other players");
        op->map->players++;
        draw_ext_info_format(NDI_UNIQUE|NDI_ALL|NDI_DK_ORANGE, 5, NULL, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_PLAYER,
                             "%s has entered the game.",
                             op->name);
        op->contr->hidden = 0;
        op->invisible = 1;
    } else
        draw_ext_info(NDI_UNIQUE|NDI_ALL|NDI_LT_GREEN, 1, NULL, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM,
                      "The Dungeon Master is gone..");

    update_los(op);
}

/**
 * object *op is trying to become dm.
 * pl_name is name supplied by player.  Restrictive DM will make it harder
 * for socket users to become DM - in that case, it will check for the players
 * character name.
 *
 * @param op
 * player wishing to become DM.
 * @param pl_name
 * player's name.
 * @param pl_passwd
 * entered password.
 * @param pl_host
 * player's host.
 * @retval 0
 * invalid credentials.
 * @retval 1
 * op can become DM.
 * @todo can't name/host be found from op? What is RESTRICTIVE_DM?
 */
static int checkdm(object *op, const char *pl_name, const char *pl_passwd, const char *pl_host) {
    FILE *dmfile;
    char buf[MAX_BUF];
    char line_buf[160], name[160], passwd[160], host[160];

#ifdef RESTRICTIVE_DM
    *pl_name = op->name ? op->name : "*";
#endif

    snprintf(buf, sizeof(buf), "%s/%s", settings.confdir, DMFILE);
    if ((dmfile = fopen(buf, "r")) == NULL) {
        LOG(llevDebug, "Could not find DM file.\n");
        return 0;
    }

    while (fgets(line_buf, 160, dmfile) != NULL) {
        if (line_buf[0] == '#')
            continue;
        if (sscanf(line_buf, "%[^:]:%[^:]:%s\n", name, passwd, host) != 3) {
            LOG(llevError, "Warning - malformed dm file entry: %s\n", line_buf);
        } else if ((!strcmp(name, "*") || (pl_name && !strcmp(pl_name, name)))
        && (!strcmp(passwd, "*") || !strcmp(passwd, pl_passwd))
        && (!strcmp(host, "*") || !strcmp(host, pl_host))) {
            fclose(dmfile);
            return (1);
        }
    }
    fclose(dmfile);
    return (0);
}

/**
 * Actually changes a player to wizard.
 *
 * @param op
 * player.
 * @param params
 * password.
 * @param silent
 * if zero, don't inform players of the mode change.
 * @retval 0
 * no mode change.
 * @retval 1
 * op is now a wizard.
 */
static int do_wizard_dm(object *op, const char *params, int silent) {
    if (!op->contr)
        return 0;

    if (QUERY_FLAG(op, FLAG_WIZ)) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "You are already the Dungeon Master!");
        return 0;
    }

    if (checkdm(op, op->name, (*params != '\0' ? params : "*"), op->contr->socket.host)) {
        SET_FLAG(op, FLAG_WIZ);
        SET_FLAG(op, FLAG_WAS_WIZ);
        SET_FLAG(op, FLAG_WIZPASS);
        SET_FLAG(op, FLAG_WIZCAST);
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                      "Ok, you are the Dungeon Master!");
        /*
         * Remove setting flying here - that won't work, because next
        * fix_object() is called that will get cleared - proper solution
         * is probably something like a wiz_force which gives that and any
         * other desired abilities.
         */
        clear_los(op);
        op->contr->write_buf[0] = '\0';

        if (!silent)
            draw_ext_info(NDI_UNIQUE|NDI_ALL|NDI_LT_GREEN, 1, NULL,
                          MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM,
                          "The Dungeon Master has arrived!");

        return 1;
    }

    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                  "Sorry Pal, I don't think so.");
    op->contr->write_buf[0] = '\0';
    return 0;
}

/**
 * Actual command to perhaps become dm.  Changed around a bit in version 0.92.2
 * to allow people on sockets to become dm, and allow better dm file
 *
 * @param op
 * player wishing to become wizard.
 * @param params
 * password.
 * @return
 * 0 unless op isn't a player.
 */
void command_dm(object *op, const char *params) {
    do_wizard_dm(op, params, 0);
}

/**
 * Wizard wants to become invisible.
 *
 * @param op
 * wizard.
 * @param params
 * ignored.
 */
void command_invisible(object *op, const char *params) {
    if (op) {
        op->invisible += 100;
        object_update(op, UP_OBJ_FACE);
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                      "You turn invisible.");
    }
}

/**
 * Returns spell object (from archetypes) by name.
 * Used for wizard's learn spell/prayer.
 *
 * Ignores archetypes "spelldirect_xxx" since these archetypes are not used
 * anymore (but may still be present in some player's inventories and thus
 * cannot be removed). We have to ignore them here since they have the same
 * name than other "spell_xxx" archetypes and would always conflict.
 **
 * @param op
 * player issuing the command.
 * @param spell_name
 * spell to find.
 * @return
 * NULL if 0 or more than one spell matches, spell object else.
 * @todo remove the spelldirect_xxx test?
 */
static object *get_spell_by_name(object *op, const char *spell_name) {
    archetype *ar;
    archetype *found;
    int conflict_found;
    size_t spell_name_length;

    /* First check for full name matches. */
    conflict_found = 0;
    found = NULL;
    for (ar = first_archetype; ar != NULL; ar = ar->next) {
        if (ar->clone.type != SPELL)
            continue;

        if (strncmp(ar->name, "spelldirect_", 12) == 0)
            continue;

        if (strcmp(ar->clone.name, spell_name) != 0)
            continue;

        if (found != NULL) {
            if (!conflict_found) {
                conflict_found = 1;
                draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                                     "More than one archetype matches the spell name %s:",
                                     spell_name);
                draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                                     "- %s",
                                     found->name);
            }
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                                 "- %s",
                                 ar->name);
            continue;
        }

        found = ar;
    }

    /* No match if more more than one archetype matches. */
    if (conflict_found)
        return NULL;

    /* Return if exactly one archetype matches. */
    if (found != NULL)
        return arch_to_object(found);

    /* No full match found: now check for partial matches. */
    spell_name_length = strlen(spell_name);
    conflict_found = 0;
    found = NULL;
    for (ar = first_archetype; ar != NULL; ar = ar->next) {
        if (ar->clone.type != SPELL)
            continue;

        if (strncmp(ar->name, "spelldirect_", 12) == 0)
            continue;

        if (strncmp(ar->clone.name, spell_name, spell_name_length) != 0)
            continue;

        if (found != NULL) {
            if (!conflict_found) {
                conflict_found = 1;
                draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                                     "More than one spell matches %s:",
                                     spell_name);
                draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                                     "- %s",
                                     found->clone.name);
            }
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                                 "- %s",
                                 ar->clone.name);
            continue;
        }

        found = ar;
    }

    /* No match if more more than one archetype matches. */
    if (conflict_found)
        return NULL;

    /* Return if exactly one archetype matches. */
    if (found != NULL)
        return arch_to_object(found);

    /* No spell found: just print an error message. */
    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                         "The spell %s does not exist.",
                         spell_name);
    return NULL;
}

/**
 * Wizards wants to learn a spell.
 *
 * @param op
 * wizard.
 * @param params
 * spell name to learn.
 * @param special_prayer
 * if set, special (god-given) prayer.
 */
static void command_learn_spell_or_prayer(object *op, const char *params, int special_prayer) {
    object *tmp;

    if (op->contr == NULL || *params == '\0') {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Which spell do you want to learn?");
        return;
    }

    tmp = get_spell_by_name(op, params);
    if (tmp == NULL) {
        return;
    }

    if (check_spell_known(op, tmp->name)) {
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                             "You already know the spell %s.",
                             tmp->name);
        return;
    }

    do_learn_spell(op, tmp, special_prayer);
    object_free2(tmp, FREE_OBJ_NO_DESTROY_CALLBACK);
}

/**
 * Wizard wants to learn a regular spell.
 *
 * @param op
 * wizard.
 * @param params
 * spell name.
 */
void command_learn_spell(object *op, const char *params) {
    command_learn_spell_or_prayer(op, params, 0);
}

/**
 * Wizard wants to learn a god-given spell.
 *
 * @param op
 * wizard.
 * @param params
 * spell name.
 */
void command_learn_special_prayer(object *op, const char *params) {
    command_learn_spell_or_prayer(op, params, 1);
}

/**
 * Wizard wishes to forget a spell.
 *
 * @param op
 * wizard.
 * @param params
 * spell name to forget.
 */
void command_forget_spell(object *op, const char *params) {
    object *spell;

    if (op->contr == NULL || *params == '\0') {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Which spell do you want to forget?");
        return;
    }

    spell = lookup_spell_by_name(op, params);
    if (spell == NULL) {
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                             "You do not know the spell %s.",
                             params);
        return;
    }

    do_forget_spell(op, spell->name);
}

/**
 * Lists all plugins currently loaded with their IDs and full names.
 *
 * @param op
 * wizard.
 * @param params
 * ignored.
 */
void command_listplugins(object *op, const char *params) {
    plugins_display_list(op);
}

/**
 * Loads the given plugin. The DM specifies the name of the library to load (no
 * pathname is needed). Do not ever attempt to load the same plugin more than
 * once at a time, or bad things could happen.
 *
 * @param op
 * DM loading a plugin.
 * @param params
 * should be the plugin's name, eg cfpython.so
 */
void command_loadplugin(object *op, const char *params) {
    char buf[MAX_BUF];

    if (*params == '\0') {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Load which plugin?");
        return;
    }

    strcpy(buf, LIBDIR);
    strcat(buf, "/plugins/");
    strcat(buf, params);
    LOG(llevDebug, "Requested plugin file is %s\n", buf);
    if (plugins_init_plugin(buf) == 0) {
        LOG(llevInfo, "DM %s loaded plugin %s\n", op->name, params);
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                             "Plugin %s successfully loaded.",
                             params);
    } else
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                             "Could not load plugin %s.",
                             params);
}

/**
 * Unloads the given plugin. The DM specified the ID of the library to unload.
 * Note that some things may behave strangely if the correct plugins are not
 * loaded.
 *
 * @param op
 * DM unloading a plugin.
 * @param params
 * should be the plugin's internal name, eg Python
 */
void command_unloadplugin(object *op, const char *params) {
    if (*params == '\0') {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Remove which plugin?");
        return;
    }

    if (plugins_remove_plugin(params) == 0) {
        LOG(llevInfo, "DM %s unloaded plugin %s\n", op->name, params);
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                             "Plugin %s successfully removed.",
                             params);
    } else
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                             "Could not remove plugin %s.",
                             params);
}

/**
 * A players wants to become DM and hide.
 * Let's see if that's authorized.
 * Make sure to not tell anything to anyone.
 *
 * @param op
 * wizard.
 * @param params
 * password.
 */
void command_dmhide(object *op, const char *params) {
    if (!do_wizard_dm(op, params, 1))
        return;

    do_wizard_hide(op, 1);
}

/**
 * Pop the stack top.
 *
 * @param op
 * wizard.
 * @param params
 * ignored.
 */
void command_stack_pop(object *op, const char *params) {
    dm_stack_pop(op->contr);
}

/**
 * Push specified item on stack.
 *
 * @param op
 * wizard.
 * @param params
 * object specifier.
 */
void command_stack_push(object *op, const char *params) {
    object *ob;
    int from;
    ob = get_dm_object(op->contr, &params, &from);

    if (ob && from != STACK_FROM_NUMBER)
        /* Object was from stack, need to push it again */
        dm_stack_push(op->contr, ob->count);
}

/**
 * Displays stack contents.
 *
 * @param op
 * wizard.
 * @param params
 * ignored.
 */
void command_stack_list(object *op, const char *params) {
    int item;
    object *display;
    player *pl = op->contr;

    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                  "Item stack contents:");

    for (item = 0; item < pl->stack_position; item++) {
        display = object_find_by_tag_global(pl->stack_items[item]);
        if (display)
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                                 " %d : %s [%d]",
                                 item, display->name, display->count);
        else
            /* Item was freed */
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                                 " %d : (lost item: %d)",
                                 item, pl->stack_items[item]);
    }
}

/**
 * Empty DM item stack.
 *
 * @param op
 * wizard.
 * @param params
 * ignored.
 */
void command_stack_clear(object *op, const char *params) {
    op->contr->stack_position = 0;
    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                  "Item stack cleared.");
}

/**
 * Get a diff of specified items.
 * Second item is compared to first, and differences displayed.
 *
 * @note
 * get_ob_diff() works the opposite way (first compared to 2nd),
 * but it's easier with stack functions to do it this way, so you can do:
 * - stack_push \<base\>
 * - stack_push \<object to be compared\>
 * - diff
 * - patch xxx <---- applies to object compared to base, easier :)
 *
 * Ryo, august 2004
 *
 * @param op
 * wizard.
 * @param params
 * object specifier.
 */
void command_diff(object *op, const char *params) {
    object *left, *right;
    char *diff;
    StringBuffer *sb;
    int left_from, right_from;

    left = get_dm_object(op->contr, &params, &left_from);
    if (!left) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Compare to what item?");
        return;
    }

    if (left_from == STACK_FROM_NUMBER)
        /* Item was stacked, remove it else right will be the same... */
        dm_stack_pop(op->contr);

    right = get_dm_object(op->contr, &params, &right_from);

    if (!right) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Compare what item?");
        return;
    }

    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                  "Item difference:");

    if (left_from == STACK_FROM_TOP && right_from == STACK_FROM_TOP) {
        /*
         * Special case: both items were taken from stack top.
         * Override the behaviour, taking left as item just below top, if exists.
         * See function description for why.
         * Besides, if we don't do anything, compare an item to itself, not really useful.
         */
        if (op->contr->stack_position > 1) {
            left = object_find_by_tag_global(op->contr->stack_items[op->contr->stack_position-2]);
            if (left)
                draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                              "(Note: first item taken from undertop)");
            else
                /* Stupid case: item under top was freed, fallback to stack top */
                left = right;
        }
    }

    sb = stringbuffer_new();
    get_ob_diff(sb, left, right);
    diff = stringbuffer_finish(sb);
    if (*diff == '\0') {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM, "Objects are the same.");
    } else {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM, diff);
    }
    free(diff);
}

/**
 * Puts an object into another.
 * @param op
 * wizard.
 * @param params
 * object specifier.
 */
void command_insert_into(object *op, const char *params) {
    object *left, *right, *inserted;
    int left_from, right_from;
    char what[MAX_BUF], where[MAX_BUF];

    left = get_dm_object(op->contr, &params, &left_from);
    if (!left) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Insert into what object?");
        return;
    }

    if (left_from == STACK_FROM_NUMBER)
        /* Item was stacked, remove it else right will be the same... */
        dm_stack_pop(op->contr);

    right = get_dm_object(op->contr, &params, &right_from);

    if (!right) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Insert what item?");
        return;
    }

    if (left_from == STACK_FROM_TOP && right_from == STACK_FROM_TOP) {
        /*
        * Special case: both items were taken from stack top.
        * Override the behaviour, taking left as item just below top, if exists.
        * See function description for why.
        * Besides, can't insert an item into itself.
        */
        if (op->contr->stack_position > 1) {
            left = object_find_by_tag_global(op->contr->stack_items[op->contr->stack_position-2]);
            if (left)
                draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                              "(Note: item to insert into taken from undertop)");
            else
                /* Stupid case: item under top was freed, fallback to stack top */
                left = right;
        }
    }

    if (left == right) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Can't insert an object into itself!");
        return;
    }

    if (right->type == PLAYER) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Can't insert a player into something!");
        return;
    }

    if (!QUERY_FLAG(right, FLAG_REMOVED))
        object_remove(right);
    inserted = object_insert_in_ob(right, left);
    if (left->type == PLAYER) {
        if (inserted != right)
            /* item was merged, so updating name and such. */
            esrv_update_item(UPD_WEIGHT|UPD_NAME|UPD_NROF, left, inserted);
    }
    query_name(inserted, what, MAX_BUF);
    query_name(left, where, MAX_BUF);
    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DM,
                         "Inserted %s in %s",
                         what, where);
}

/**
 * Displays information about styles loaded for random maps.
 *
 * @param op
 * wizard.
 * @param params
 * ignored.
 */
void command_style_map_info(object *op, const char *params) {
    extern mapstruct *styles;
    mapstruct *mp;
    int maps_used = 0, mapmem = 0, objects_used = 0, x, y;

    for (mp = styles; mp != NULL; mp = mp->next) {
        maps_used++;
        mapmem += MAP_WIDTH(mp)*MAP_HEIGHT(mp)*(sizeof(object *)+sizeof(MapSpace))+sizeof(mapstruct);
        for (x = 0; x < MAP_WIDTH(mp); x++) {
            for (y = 0; y < MAP_HEIGHT(mp); y++) {
                FOR_MAP_PREPARE(mp, x, y, tmp)
                    objects_used++;
                FOR_MAP_FINISH();
            }
        }
    }
    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_MAPS,
                         "[fixed]Style maps loaded:    %d",
                         maps_used);
    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_MAPS,
                  "[fixed]Memory used, not");
    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_MAPS,
                         "[fixed]including objects:    %d",
                         mapmem);
    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_MAPS,
                         "[fixed]Style objects:        %d",
                         objects_used);
    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_MAPS,
                         "[fixed]Mem for objects:      %lu",
                         (unsigned long)(objects_used*sizeof(object)));
}

/**
 * DM wants to follow a player, or stop following a player.
 *
 * @param op
 * wizard.
 * @param params
 * player to follow. If NULL, stop following player.
 */
void command_follow(object *op, const char *params) {
    player *other;

    if (*params == '\0') {
        if (op->contr->followed_player != NULL) {
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM, "You stop following %s.", op->contr->followed_player);
            FREE_AND_CLEAR_STR(op->contr->followed_player);
        }
        return;
    }

    other = find_player_partial_name(params);
    if (!other) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM, "No such player or ambiguous name.");
        return;
    }
    if (other == op->contr) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM, "You can't follow yourself.");
        return;
    }

    if (op->contr->followed_player)
        FREE_AND_CLEAR_STR(op->contr->followed_player);

    op->contr->followed_player = add_string(other->ob->name);
    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM, "Following %s.", op->contr->followed_player);
}

void command_purge_quest(object *op, const char * param) {
    free_quest();
    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM, "Purged quest state.");
}

void command_purge_quest_definitions(object *op, const char * param) {
    free_quest_definitions();
    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM, "Purged quests definitions.");
}

/**
 * Player wants to dump object below her.
 *
 * @param op
 * player asking for information.
 * @param params
 * unused.
 */
void command_dumpbelow(object *op, const char *params) {
    if (op && op->below) {
        StringBuffer *sb;
        char *diff;

        sb = stringbuffer_new();
        object_dump(op->below, sb);
        diff = stringbuffer_finish(sb);
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_SUBTYPE_NONE, diff);
        free(diff);

        /* Let's push that item on the dm's stack */
        dm_stack_push(op->contr, op->below->count);
    }
}

/**
 * Wizard wants to know some server settings, so display.
 * @param op wizard asking for settings.
 * @param ignored ignored additional text.
 */
void command_settings(object *op, const char *ignored) {
    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM, i18n(op, "Server settings:"));

    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM, i18n(op, " * item power factor: %2f"), settings.item_power_factor);

    if (settings.not_permadeth) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM, i18n(op, " * death is not permanent"));
    } else if (settings.resurrection) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM, i18n(op, " * permanent death, resurrection is enabled"));
    } else {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM, i18n(op, " * permanent death, resurrection is NOT enabled"));
    }

    if (settings.set_title) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM, i18n(op, " * players can set their title"));
    } else {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM, i18n(op, " * players can't set their title"));
    }

    if (settings.spell_encumbrance) {
        if (settings.spell_failure_effects) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM, i18n(op, " * too much equipment can lead to spell failure and ill effects"));
        } else {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM, i18n(op, " * too much equipment can lead to spell failure but no ill effects"));
        }
    } else {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM, i18n(op, " * too much equipment can't lead to spell failure"));
    }

    if (settings.casting_time) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM, i18n(op, " * casting takes time"));
    } else {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM, i18n(op, " * casting is immediate"));
    }

    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM, i18n(op, " * permanent experience: %d%%"), settings.permanent_exp_ratio);
    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM, i18n(op, " * death penalty %d%% or %d levels"), settings.death_penalty_ratio, settings.death_penalty_level);

    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM, i18n(op, " * friendly fire: %d%%"), settings.set_friendly_fire);

    if (settings.no_player_stealing) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM, i18n(op, " * players can't steal from other players"));
    } else {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM, i18n(op, " * players can steal from other players"));
    }

    if (settings.create_home_portals) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM, i18n(op, " * players can create portals from their apartments"));
    } else {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM, i18n(op, " * players can't create portals from their apartments"));
    }

    if (settings.allow_denied_spells_writing) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM, i18n(op, " * players can write spells they are denied"));
    } else {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM, i18n(op, " * players can't write spells they are denied"));
    }
}
