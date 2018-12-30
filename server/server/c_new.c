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
 * This file deals with administrative commands from the client.
 */

#include <global.h>
#include <commands.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#endif

#ifndef tolower
/** Simple macro to convert a letter to lowercase. */
#define tolower(C) (((C) >= 'A' && (C) <= 'Z') ? (C)-'A'+'a' : (C))
#endif

/**
 * Compare function for commands.
 *
 * @param a
 * @param b
 * commands to compare.
 * @retval -1
 * a is less then b.
 * @retval 0
 * a and b are equals.
 * @retval 1
 * a is greater than b.
 */
static int compare_A(const void *a, const void *b) {
    return strcmp(((const command_array_struct *)a)->name,
                  ((const command_array_struct *)b)->name);
}

/**
 * Finds the specified command in the command array. Utility function.
 *
 * @param cmd
 * command to find. Will be put to lowercase.
 * @param commarray
 * commands to search into.
 * @param commsize
 * length of commarray.
 * @return
 * matching command, NULL for no match.
 */
static command_array_struct *find_command_element(const char *cmd, command_array_struct *commarray, int commsize) {
    command_array_struct *asp, dummy;

    dummy.name = cmd;
    asp = (command_array_struct *)bsearch((void *)&dummy,
                                          (void *)commarray, commsize,
                                          sizeof(command_array_struct),
                                          compare_A);
    return asp;
}

/**
 * Player issued a command, let's handle it.
 *
 * This function is called from the new client/server code.
 *
 * @param pl
 * player who is issuing the command
 * @param command
 * the actual command with its arguments.
 */
void execute_newserver_command(object *pl, char *command) {
    command_array_struct *csp, sent;
    char *cp, *low;

    pl->contr->has_hit = 0;

    /*
     * remove trailing spaces from commant
     */
    cp = command+strlen(command)-1;
    while ((cp >= command) && (*cp == ' ')) {
        *cp = '\0';
        cp--;
    }
    cp = strchr(command, ' ');
    if (cp) {
        *(cp++) = '\0';
        while (*cp == ' ')
            cp++;
    } else {
        cp = strchr(command, '\0');
    }

    for (low = command; *low; low++)
        *low = tolower(*low);

    csp = find_plugin_command(command, &sent);
    if (!csp)
        csp = find_command_element(command, Commands, CommandsSize);
    if (!csp)
        csp = find_command_element(command, CommunicationCommands,
                                   CommunicationCommandSize);
    if (!csp && QUERY_FLAG(pl, FLAG_WIZ))
        csp = find_command_element(command, WizCommands, WizCommandsSize);

    if (csp == NULL) {
        draw_ext_info_format(NDI_UNIQUE, 0, pl,
                             MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                             "'%s' is not a valid command.",
                             command);
        return;
    }

    pl->speed_left -= csp->time;

    /* A character time can never exceed his speed (which in many cases,
     * if wearing armor, is less than one.)  Thus, in most cases, if
     * the command takes 1.0, the player's speed will be less than zero.
     * it is only really an issue if time goes below -1
     * Due to various reasons that are too long to go into here, we will
     * actually still execute player even if his time is less than 0,
     * but greater than -1.  This is to improve the performance of the
     * new client/server.  In theory, it shouldn't make much difference.
     */

    if (csp->time && pl->speed_left < -2.0) {
        LOG(llevDebug, "execute_newclient_command: Player issued command that takes more time than he has left.\n");
    }
    csp->func(pl, cp);
}

/**
 * Player wants to start running.
 *
 * @param op
 * player.
 * @param params
 * additional parameters.
 */
void command_run(object *op, const char *params) {
    int dir;

    dir = atoi(params);
    if (dir < 0 || dir >= 9) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Can't run into a non adjacent square.");
        return;
    }
    op->contr->run_on = 1;
    move_player(op, dir);
}

/**
 * Player wants to stop running.
 *
 * @param op
 * player.
 * @param params
 * ignored.
 * @return
 * 1.
 */
void command_run_stop(object *op, const char *params) {
    op->contr->run_on = 0;
}

/**
 * Player wants to start furing.
 *
 * @param op
 * player.
 * @param params
 * additional parameters.
 */
void command_fire(object *op, const char *params) {
    int dir;

    dir = atoi(params);
    if (dir < 0 || dir >= 9) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Can't fire to a non adjacent square.");
        return;
    }
    op->contr->fire_on = 1;
    move_player(op, dir);
}

/**
 * Player wants to stop firing.
 *
 * @param op
 * player.
 * @param params
 * ignored.
 */
void command_fire_stop(object *op, const char *params) {
    op->contr->fire_on = 0;
}
