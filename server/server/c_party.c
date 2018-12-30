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
 * Party-related functions and variables.
 */

#include <global.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#endif
#include <spells.h>

/**
 * Is the password the player entered to join a party the right one?
 *
 * @param op
 * player. Must have party_to_join correctly set.
 * @retval 0
 * password is correct.
 * @retval 1
 * invalid password or party not found.
 */
int confirm_party_password(object *op) {
    const partylist *party;

    party = party_find(op->contr->party_to_join->partyname);
    return party == NULL || !party_confirm_password(party, op->contr->write_buf+1);
}

/**
 * Player entered a party password.
 *
 * @param op
 * player.
 */
void receive_party_password(object *op) {
    if (confirm_party_password(op) == 0) {
        party_join(op, op->contr->party_to_join);
        op->contr->party_to_join = NULL;
        player_set_state(op->contr, ST_PLAYING);
        return;
    }

    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                  "You entered the wrong password");
    op->contr->party_to_join = NULL;
    player_set_state(op->contr, ST_PLAYING);
}

/**
 * 'gsay' command, talks to party.
 *
 * @param op
 * player.
 * @param params
 * message.
 */
void command_gsay(object *op, const char *params) {
    char party_params[MAX_BUF];

    if (*params == '\0') {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR, "Say what?");
        return;
    }
    strcpy(party_params, "say ");
    strcat(party_params, params);
    command_party(op, party_params);
}

/**
 * Give help for party commands.
 *
 * @param op
 * player.
 */
static void party_help(object *op) {
    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_HELP,
                  "To form a party type: party form <partyname>. "
                  "To join a party type: party join <partyname> "
                  "If the party has a passwd, it will you prompt you for it. "
                  "For a list of current parties type: party list. "
                  "To leave a party type: party leave "
                  "To change a passwd for a party type: party passwd <password> "
                  "There is an 8 character maximum password length. "
                  "To talk to party members type: party say <msg> "
                  "To see who is in your party: party who "
#ifdef PARTY_KILL_LOG
                  "To see what you've killed, type: party kills"
#endif
                 );
}

/**
 * 'party' command, subdivided in different sub commands.
 *
 * @param op
 * player.
 * @param params
 * additional parameters.
 * 1.
 * @todo split in different functions. clean the 'form' mess.
 */
void command_party(object *op, const char *params) {
    char buf[MAX_BUF];

    if (*params == '\0') {
        if (op->contr->party == NULL) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                          "You are not a member of any party. "
                          "For help try: party help");
        } else {
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                                 "You are a member of party %s.",
                                 op->contr->party->partyname);
        }
        return;
    }
    if (strcmp(params, "help") == 0) {
        party_help(op);
        return;
    }
#ifdef PARTY_KILL_LOG
    if (!strncmp(params, "kills", 5)) {
        int i, max;
        char chr;
        float exp;
        partylist *tmpparty;

        if (op->contr->party == NULL) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                          "You are not a member of any party.");
            return;
        }
        tmpparty = op->contr->party;
        if (!tmpparty->kills) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                          "You haven't killed anything yet.");
            return;
        }
        max = tmpparty->kills-1;
        if (max > PARTY_KILL_LOG-1)
            max = PARTY_KILL_LOG-1;
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                      "[fixed]Killed          |          Killer|     Exp\n----------------+----------------+--------"
                      "Killed          |          Killer|     Exp\n----------------+----------------+--------");


        for (i = 0; i <= max; i++) {
            exp = tmpparty->party_kills[i].exp;
            chr = ' ';
            if (exp > 1000000) {
                exp /= 1000000;
                chr = 'M';
            } else if (exp > 1000) {
                exp /= 1000;
                chr = 'k';
            }

            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                                 "[fixed]%16s|%16s|%6.1f%c",
                                 tmpparty->party_kills[i].dead,
                                 tmpparty->party_kills[i].killer, exp, chr);
        }
        exp = tmpparty->total_exp;
        chr = ' ';
        if (exp > 1000000) {
            exp /= 1000000;
            chr = 'M';
        } else if (exp > 1000) {
            exp /= 1000;
            chr = 'k';
        }

        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                      "[fixed]----------------+----------------+--------");
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                             "Totals: %d kills, %.1f%c exp", tmpparty->kills,
                             exp, chr);
        return;
    }
#endif /* PARTY_KILL_LOG */
    if (strncmp(params, "say ", 4) == 0) {
        if (op->contr->party == NULL) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                          "You are not a member of any party.");
            return;
        }
        params += 4;
        snprintf(buf, MAX_BUF-1, "<%s> %s says: %s", op->contr->party->partyname, op->name, params);
        party_send_message(op, buf);
        draw_ext_info_format(NDI_WHITE, 0, op, MSG_TYPE_COMMUNICATION, MSG_TYPE_COMMUNICATION_PARTY,
                             "<%s> You say: %s",
                             op->contr->party->partyname, params);
        return;
    }

    if (strncmp(params, "form ", 5) == 0) {
        params += 5;

        if (party_form(op, params) == NULL) {
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                                 "The party %s already exists, pick another name",
                                 params);
            return;
        }
        return;
    } /* form */

    if (strcmp(params, "leave") == 0) {
        if (op->contr->party == NULL) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                          "You are not a member of any party.");
            return;
        }
        party_leave(op);
        return;
    }
    if (strcmp(params, "who") == 0) {
        if (op->contr->party == NULL) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                          "You are not a member of any party.");
            return;
        }
        list_players(op, NULL, op->contr->party);
        return;
    } /* leave */

    if (strncmp(params, "passwd ", 7) == 0) {
        params += 7;

        if (op->contr->party == NULL) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                          "You are not a member of a party");
            return;
        }

        if (strlen(params) > 8) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                          "The password must not exceed 8 characters");
            return;
        }

        party_set_password(op->contr->party, params);
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                             "The password for party %s is %s",
                             op->contr->party->partyname, party_get_password(op->contr->party));

        snprintf(buf, MAX_BUF, "Password for party %s is now %s, changed by %s",
                 op->contr->party->partyname, party_get_password(op->contr->party), op->name);
        party_send_message(op, buf);
        return;
    } /* passwd */

    if (strcmp(params, "list") == 0) {
        partylist *party;

        if (party_get_first() == NULL) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                          "There are no parties active right now");
            return;
        }

        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                      "[fixed]Party name                       Leader\n----------                       ------");
        for (party = party_get_first(); party != NULL; party = party_get_next(party)) {
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                                 "[fixed]%-32s %s",
                                 party->partyname, party_get_leader(party));
        }

        return;
    } /* list */

    if (strncmp(params, "join ", 5) == 0) {
        partylist *party;

        params += 5;

        party = party_find(params);
        if (party == NULL) {
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                                 "Party %s does not exist.  You must form it first.",
                                 params);
            return;
        }

        if (op->contr->party == party) {
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                                 "You are already a member of party: %s",
                                 party->partyname);
            return;
        }

        if (get_party_password(op, party)) {
            return;
        }

        party_join(op, party);
        return;
    } /* join */

    party_help(op);
}

/** Valid modes for 'party_rejoin', indexed by ::party_rejoin_mode. */
static const char *rejoin_modes[] = {
    "no",
    "if_exists",
    "always",
    NULL
};

/**
 * Handles the 'party_rejoin' command.
 * @param op
 * player.
 * @param params
 * optional parameters.
 */
void command_party_rejoin(object *op, const char *params) {
    int mode;

    if (*params == '\0') {
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                             "party rejoin: %s", rejoin_modes[op->contr->rejoin_party]);
        return;
    }
    for (mode = 0; rejoin_modes[mode] != NULL; mode++) {
        if (strcmp(rejoin_modes[mode], params) == 0) {
            op->contr->rejoin_party = mode;
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                                 "party rejoin is now: %s", rejoin_modes[op->contr->rejoin_party]);
            return;
        }
    }
    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                         "invalid mode: %50s", params);
}
