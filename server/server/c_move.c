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
 * Move-related (north, east, ...) commands.
 */

#include <global.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#endif
#include <skills.h>

/**
 * A player is moving in a direction, but this may indicate firing.
 *
 * @param op
 * player moving.
 * @param params
 * optional parameters for moving (fire, run).
 * @param dir
 * moving direction.
 */
static void move_internal(object *op, const char *params, int dir) {
    if (*params != '\0') {
        if (params[0] == 'f') {
            if (!op->contr->fire_on) {
                op->contr->fire_on = 1;
                move_player(op, dir);
                op->contr->fire_on = 0;
                return;
            }
        } else if (params[0] == 'r' && !op->contr->run_on)
            op->contr->run_on = 1;
    }
    move_player(op, dir);
}

/**
 * 'east' command.
 * @param op
 * player.
 * @param params
 * optional parameters for moving (fire, run).
 */
void command_east(object *op, const char *params) {
    move_internal(op, params, 3);
}

/**
 * 'north' command.
 * @param op
 * player.
 * @param params
 * optional parameters for moving (fire, run).
 */
void command_north(object *op, const char *params) {
    move_internal(op, params, 1);
}

/**
 * 'northeast' command.
 * @param op
 * player.
 * @param params
 * optional parameters for moving (fire, run).
 */
void command_northeast(object *op, const char *params) {
    move_internal(op, params, 2);
}

/**
 * 'northwest' command.
 * @param op
 * player.
 * @param params
 * optional parameters for moving (fire, run).
 */
void command_northwest(object *op, const char *params) {
    move_internal(op, params, 8);
}

/**
 * 'south' command.
 * @param op
 * player.
 * @param params
 * optional parameters for moving (fire, run).
 */
void command_south(object *op, const char *params) {
    move_internal(op, params, 5);
}

/**
 * 'southeast' command.
 * @param op
 * player.
 * @param params
 * optional parameters for moving (fire, run).
 */
void command_southeast(object *op, const char *params) {
    move_internal(op, params, 4);
}

/**
 * 'southwest' command.
 * @param op
 * player.
 * @param params
 * optional parameters for moving (fire, run).
 */
void command_southwest(object *op, const char *params) {
    move_internal(op, params, 6);
}

/**
 * 'west' command.
 * @param op
 * player.
 * @param params
 * optional parameters for moving (fire, run).
 */
void command_west(object *op, const char *params) {
    move_internal(op, params, 7);
}

/**
 * 'stay' command. Used to specify to fire under oneself.
 * @param op
 * player.
 * @param params
 * optional parameters for moving (fire, run).
 */
void command_stay(object *op, const char *params) {
    if (!op->contr->fire_on && params[0] != 'f')
        return;
    fire(op, 0);
}
