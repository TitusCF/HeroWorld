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
 * Party-structure related functions.
 */

#include "global.h"
#include "sproto.h"


static partylist *firstparty = NULL; /**< Keeps track of first party in list */
static partylist *lastparty = NULL;  /**< Keeps track of last party in list */


/**
 * Forms the party struct for a party called 'partyname'. it is the
 * responsibility of the caller to ensure that the name is unique.
 * New item is placed on the party list.
 * @param op
 * party creator.
 * @param partyname
 * the party name.
 * @return
 * new party or NULL if the name is not unique.
 */
partylist *party_form(object *op, const char *partyname) {
    partylist *party;
    char buf[MAX_BUF];

    snprintf(buf, sizeof(buf), "%s", partyname);
    replace_unprintable_chars(buf);

    if (party_find(buf) != NULL)
        return NULL;

    party_leave(op);
    party = (partylist *)malloc(sizeof(partylist));
    party->partyname = strdup_local(buf);
#ifdef PARTY_KILL_LOG
    party->total_exp = 0;
    party->kills = 0;
#endif
    party->passwd[0] = '\0';
    party->next = NULL;
    party->partyleader = strdup_local(op->name);
    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                         "You have formed party: %s",
                         party->partyname);
    op->contr->party = party;

    if (lastparty) {
        lastparty->next = party;
        lastparty = lastparty->next;
    } else {
        firstparty = party;
        lastparty = firstparty;
    }

    return party;
}

/**
 * Makes a player join a party. Leaves the a former party if necessary.
 *
 * @param op
 * the player
 * @param party
 * the party to join
 */
void party_join(object *op, partylist *party) {
    char buf[MAX_BUF];

    party_leave(op);

    op->contr->party = party;
    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                         "You have joined party: %s\n",
                         party->partyname);
    snprintf(buf, MAX_BUF, "%s joins party %s", op->name, party->partyname);
    party_send_message(op, buf);
}

/**
 * Makes a player leave his party. Does nothing if the player is not member of
 * a party.
 *
 * @param op
 * the player
 */
void party_leave(object *op) {
    char buf[MAX_BUF];

    if (op->contr->party == NULL) {
        return;
    }

    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                         "You leave party %s.",
                         op->contr->party->partyname);
    snprintf(buf, sizeof(buf), "%s leaves party %s.", op->name, op->contr->party->partyname);
    party_send_message(op, buf);

    /*
     * The player might have previously been a member of a party, if so, he will be leaving
     * it, so check if there are any other members and if not, delete the party
     */
    if (op->contr->party != NULL) {
        int party_found;
        player *pl;

        party_found = 0;
        for (pl = first_player; pl != NULL; pl = pl->next) {
            if (pl != op->contr && pl->party == op->contr->party) {
                party_found = 1;
                break;
            }
        }
        if (!party_found)
            party_remove(op->contr->party);
    }

    op->contr->party = NULL;
}

/**
 * Find a party by name.
 *
 * @param partyname
 * the party's name to find
 * @return
 * the party or NULL if no such party exists
 */
partylist *party_find(const char *partyname) {
    partylist *party;

    for (party = firstparty; party; party = party->next) {
        if (strcmp(party->partyname, partyname) == 0)
            return party;
    }
    return NULL;
}

/**
 * Removes and frees a party. Removes all members from the party.
 *
 * @param party
 * the party to remove
 *
 * @todo clean/simplify the mess.
 */
void party_remove(partylist *party) {
    partylist *tmpparty;
    partylist *previousparty;
    partylist *nextparty;

    if (firstparty == NULL) {
        LOG(llevError, "party_remove: I was asked to remove party %s, but no parties are defined\n",
            party->partyname);
        return;
    }

    /* special case-ism for parties at the beginning and end of the list */
    if (party == firstparty) {
        if (lastparty == party)
            lastparty = NULL;
        firstparty = firstparty->next;
        free(party->partyleader);
        free(party->partyname);
        free(party);
        return;
    } else if (party == lastparty) {
        for (tmpparty = firstparty; tmpparty->next != NULL; tmpparty = tmpparty->next) {
            if (tmpparty->next == party) {
                lastparty = tmpparty;
                free(party->partyleader);
                free(party->partyname);
                free(party);
                lastparty->next = NULL;
                return;
            }
        }
    }
    for (tmpparty = firstparty; tmpparty->next != NULL; tmpparty = tmpparty->next)
        if (tmpparty->next == party) {
            previousparty = tmpparty;
            nextparty = tmpparty->next->next;
            /* this should be safe, because we already dealt with the lastparty case */

            previousparty->next = nextparty;
            free(party->partyleader);
            free(party->partyname);
            free(party);
            return;
        }
}

/**
 * Returns the first party from the list of all parties.
 *
 * @return
 * the first party or NULL if no party exists
 */
partylist *party_get_first(void) {
    return firstparty;
}

/**
 * Returns the next party from the list of all parties.
 *
 * @param party
 * the party to use
 * @return
 * the next party or NULL if party is the last one in list
 */
partylist *party_get_next(const partylist *party) {
    return party->next;
}

/**
 * Remove unused parties (no players), this could be made to scale a lot better.
 */
void party_obsolete_parties(void) {
    int player_count;
    player *pl;
    partylist *party;
    partylist *next = NULL;

    if (!firstparty)
        return; /* we can't obsolete parties if there aren't any */
    for (party = firstparty; party != NULL; party = next) {
        next = party->next;
        player_count = 0;
        for (pl = first_player; pl != NULL; pl = pl->next)
            if (pl->party == party)
                player_count++;
        if (player_count == 0)
            party_remove(party);
    }
}

/**
 * Returns the party's password.
 *
 * @param party
 * the party to query
 * @return
 * the password or an empty string if the party has no password
 */
const char *party_get_password(const partylist *party) {
    return party->passwd;
}

/**
 * Sets a party's password.
 *
 * @param party
 * the party to change
 * @param password
 * the new password to set
 */
void party_set_password(partylist *party, const char *password) {
    snprintf(party->passwd, sizeof(party->passwd), "%s", password);
    replace_unprintable_chars(party->passwd);
}

/**
 * Checks whether a given password matches the party's password.
 *
 * @param party
 * the party to check
 * @param password
 * the password to check for
 * @return
 * whether the password matches
 */
int party_confirm_password(const partylist *party, const char *password) {
    return strcmp(party->passwd, password) == 0;
}

/**
 * Send a message to all party members except the speaker.
 *
 * @param op
 * player talking.
 * @param message
 * message to send.
 *
 * @todo
 * should be moved to player.c?
 */
void party_send_message(object *op, const char *message) {
    player *pl;

    for (pl = first_player; pl != NULL; pl = pl->next)
        if (pl->ob->contr->party == op->contr->party && pl->ob != op)
            draw_ext_info(NDI_WHITE, 0, pl->ob, MSG_TYPE_COMMUNICATION, MSG_TYPE_COMMUNICATION_PARTY,
                          message);
}

/**
 * Returns the name of the party's leader.
 *
 * @param party
 * the party to query
 * @return
 * the leader's name
 */
const char *party_get_leader(const partylist *party) {
    return party->partyleader;
}

#ifdef PARTY_KILL_LOG
/**
 * Logs a kill for a party.
 *
 * @param party
 * party to log for.
 * @param killer
 * name of the killer.
 * @param dead
 * victim's name.
 * @param exp
 * how much experience was gained.
 */
void party_add_kill(partylist *party, const char *killer, const char *dead, long exp) {
    int i, pos;

    if (party->kills >= PARTY_KILL_LOG) {
        pos = PARTY_KILL_LOG-1;
        for (i = 0; i < PARTY_KILL_LOG-1; i++)
            memcpy(&(party->party_kills[i]), &(party->party_kills[i+1]), sizeof(party->party_kills[0]));
    } else
        pos = party->kills;
    party->kills++;
    party->total_exp += exp;
    party->party_kills[pos].exp = exp;
    strncpy(party->party_kills[pos].killer, killer, MAX_NAME);
    strncpy(party->party_kills[pos].dead, dead, MAX_NAME);
    party->party_kills[pos].killer[MAX_NAME] = 0;
    party->party_kills[pos].dead[MAX_NAME] = 0;
}
#endif
