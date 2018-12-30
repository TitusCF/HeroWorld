/*
 * Crossfire -- cooperative multi-player graphical RPG and adventure game
 *
 * Copyright (c) 1999-2013 Mark Wedel and the Crossfire Development Team
 * Copyright (c) 1992 Frank Tore Johansen
 *
 * Crossfire is free software and comes with ABSOLUTELY NO WARRANTY. You are
 * welcome to redistribute it under certain conditions. For details, please
 * see COPYING and LICENSE.
 *
 * The authors can be reached via e-mail at <crossfire@metalforge.org>.
 */

/**
 * @file server/account_char.c
 *
 * This file contains logic of dealing with characters that associated with an
 * account.  The main purpose of this file is to retrieve and store information
 * about characters to communicate this to the player.  For example, the
 * amount of experience, class, race, etc of the character.
 *
 * In order to help performance, this information is stored in a file
 * for each account.  In this way, opening one file and reading the
 * data gets the information needed - much more efficient than opening
 * and reading many player files.  In general, this information is
 * only loaded/used when the account is logged in, and the data is stored
 * in the socket structure.
 *
 * Format of the file is:
 * Account Name:Class:Race:Level:Face (text form):Party:Map
 * Mark:Fighter:Orc:18:warrior.111:foobar:Scorn
 *
 * Addtional fields can be added as necessary.
 *
 * Note about characters vs players:
 * In many parts of the code, a character may be called a player,
 * but that is misleading, since a player suggests the person sitting
 * at the computer, and they play a character.  In the past, there was
 * a 1:1 mapping between players and characters, but with accounts, there
 * can be several _characters_ associated with an account, and it will
 * be more typical that the account maps 1:1 with a player.  As such,
 * saying this account has 10 players may be confusing or misconstrued -
 * what is really the case is that this account has 10 characters.
 */

#include <assert.h>
#include <global.h>
#include <object.h>
#include <account_char.h>
#include <stdlib.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#endif

/** Number of fields in the accounts file.  These are colon seperated */
#define NUM_ACCOUNT_CHAR_FIELDS 8

/**
 * Name of the directory containing account information.  I can not ever see a reason why this
 * name would not work, but may as well still make it easy to change it.
 */
/**
 * FixMe: Shouldn't something like this go in the settings file?
 */
#define ACCOUNT_DIR "account"

/**
 * For a given account name, load the character information and
 * return it. It is the responsibility of the caller to call account_char_free()
 * on the returned value to free it.
 * @param account_name
 * Name of the account.  The name should be validated before this routine
 * is called (eg, passed checks for legitimate characters and logged in)
 * @return
 * linked list of the character data.
 */
Account_Char *account_char_load(const char *account_name) {
    char fname[MAX_BUF], buf[VERY_BIG_BUF];
    FILE *fp;
    Account_Char *first = NULL, *ac, *last = NULL;

    snprintf(fname, MAX_BUF, "%s/%s/%s", settings.localdir, ACCOUNT_DIR, account_name);
    fp = fopen(fname, "r");
    if (!fp) {
        /* This may not in fact be a critical error - for a new account, there
         * may not be any data associated with it.
         */
        LOG(llevInfo, "Warning: Unable to open %s\n", fname);
        return NULL;
    }
    while (fgets(buf, VERY_BIG_BUF, fp)) {
        char *tmp[NUM_ACCOUNT_CHAR_FIELDS], *cp;

        /* Ignore any comment lines */
        if (buf[0] == '#') continue;

        /* remove newline */
        cp = strchr(buf, '\n');
        if (cp) *cp = '\0';

        if (split_string(buf, tmp, NUM_ACCOUNT_CHAR_FIELDS, ':') != NUM_ACCOUNT_CHAR_FIELDS) {
            if (!tmp[7]) {
                LOG(llevError, "Outdated entry in %s: %s\n", fname, buf);
                tmp[7] = (char *) add_string("0");
            } else {
                LOG(llevError, "Corrupt entry in %s: %s\n", fname, buf);
                continue;
            }
        }
        ac = malloc(sizeof (Account_Char));
        ac->name = add_string(tmp[0]);
        ac->character_class = add_string(tmp[1]);
        ac->race = add_string(tmp[2]);
        ac->level = strtoul(tmp[3], (char**) NULL, 10);
        ac->face = add_string(tmp[4]);
        ac->party = add_string(tmp[5]);
        ac->map = add_string(tmp[6]);
        ac->isDead = strtoul(tmp[7], (char**) NULL, 10);

        ac->next = NULL;

        /* We tack on to the end of the list - in this way,
         * the order of the file remains the same.
         */
        if (last)
            last->next = ac;
        else
            first = ac;
        last = ac;

    }
    fclose(fp);
    return (first);
}

/**
 * Saves the character information for the given account.
 * @param account
 * account name to save data for.
 * @param chars
 * previously loaded/generated list of character information for this account.
 */
void account_char_save(const char *account, Account_Char *chars) {
    char fname[MAX_BUF], fname1[MAX_BUF];
    FILE *fp;
    Account_Char *ac;

    /* It is certanly possibly that all characters for an account have
     * been removed/deleted - in that case, we just want to remove this
     * file.
     */
    if (chars == NULL) {
        snprintf(fname, MAX_BUF, "%s/%s/%s", settings.localdir, ACCOUNT_DIR, account);
        unlink(fname);
        return;
    }

    snprintf(fname, MAX_BUF, "%s/%s/%s.new", settings.localdir, ACCOUNT_DIR, account);
    fp = fopen(fname, "w");
    if (fp == NULL) {
        char err[MAX_BUF];

        LOG(llevError, "Cannot open accounts file %s: %s\n", fname,
                strerror_local(errno, err, sizeof (err)));
        return;
    }

    fprintf(fp, "# This file should not be edited while the server is running.\n");
    fprintf(fp, "# Otherwise, any changes made may be overwritten by the server\n");
    for (ac = chars; ac; ac = ac->next) {
        fprintf(fp, "%s:%s:%s:%d:%s:%s:%s:%d\n",
                ac->name, ac->character_class, ac->race, ac->level,
                ac->face, ac->party, ac->map, ac->isDead);
    }
    fclose(fp);

    /* We write to a new file name - in that way, if we crash while writing the file,
     * we are not left with a corrupted file.  So now we need to rename it to the
     * file to use.
     */
    snprintf(fname1, MAX_BUF, "%s/%s/%s", settings.localdir, ACCOUNT_DIR, account);
    unlink(fname1);
    rename(fname, fname1);
}

/**
 * This adds a player to the list of accounts.  We check to see if the player
 * has already been added to this account - if so, we just update
 * the infromation.  Note that all strings are filled in, even if that
 * may just be a blank field.  This simplifies a lot of the code instead
 * of having to deal with NULL values.
 * Note that this routine is also used to update existing entries -
 * if the character already exists, we update it, otherwise it is added.
 *
 * @param chars
 * Existing list of characters for account.  May be NULL.
 * @param pl
 * Player structure to add
 * @return
 * Returns new list of characters for account.
 */
Account_Char *account_char_add(Account_Char *chars, player *pl) {

    Account_Char *ap, *last = NULL;

    for (ap = chars; ap; ap = ap->next) {
        if (!strcasecmp(ap->name, pl->ob->name)) break;
        last = ap;
    }
    /* If ap is not NULL, it means we found a match.
     * Rather than checking to see if values have changed, just
     * update them.
     */
    if (ap) {
        /* We know the name can not be changing, as otherwise
         * we wouldn't have gotten a match. So no need to
         * update that.
         */
#if 0
        /* As of right now, the class of the character is not stored
         * anyplace, so we don't know what it is.  Keep this code here
         * until it can be determined.
         */
        free_string(ap->character_class);
        ap->character_class = add_string();
#else
        ap->character_class = add_string("");
#endif

        free_string(ap->race);
        /* This looks pretty nasty.  Basically, the player object is
         * the race archetype, but its name has been changed to the player
         * name.  So we have to go back to the actual original archetype,
         * the clone object in there, to get the name.
         */
        ap->race = add_string(pl->ob->arch->clone.name);

        ap->level = pl->ob->level;

        /* We should try and get the best face (front view) of
         * the character - right now, we just get whatever view the character
         * happens to be facing.  Unfortunately, the animation code is such
         * that it isn't a simple matter as most of that logic is not
         * conveniently exposed.
         */
        free_string(ap->face);
        ap->face = add_string(pl->ob->face->name);

        free_string(ap->party);
        if (pl->party)
            ap->party = add_string(pl->party->partyname);
        else
            ap->party = add_string("");

        free_string(ap->map);

        /* If there is a real name set for the map, use that instead
         * of the pathname, which is what maplevel holds.  This is
         * more friendly (eg, Scorn Inn vs /scorn/inns/....)
         */
        if (pl->ob->map && pl->ob->map->name) {
            ap->map = add_string(pl->ob->map->name);
        } else {
            /* Use the stored value - this may not be as up to date, but is
             * probably more reliable, as depending when this charapter is added,
             * it may not really be on any map.
             */
            ap->map = add_string(pl->maplevel);
        }
    } else {
        /* In this case, we are adding a new entry */
        ap = malloc(sizeof (Account_Char));
        ap->name = add_string(pl->ob->name);
        ap->character_class = add_string("");
        ap->race = add_string(pl->ob->arch->clone.name);
        ap->level = pl->ob->level;
        ap->face = add_string(pl->ob->face->name);
        if (pl->party)
            ap->party = add_string(pl->party->partyname);
        else
            ap->party = add_string("");
        ap->map = add_string(pl->maplevel);
        /* The character cannot be dead already */
        ap->isDead = 0;

        ap->next = NULL;
        if (last)
            last->next = ap;
        else
            chars = ap;
    }
    return chars;
}

/**
 * This removes a character on this account.  This is typically used
 * when a character has been deleted, and not for general cleanup
 *
 * @param chars
 * Existing list of characters for account.
 * @param pl_name
 * The name of the character
 * @return
 * Returns new list of characters for account.
 */
Account_Char *account_char_remove(Account_Char *chars, const char *pl_name) {
    Account_Char *ap, *last = NULL;

    for (ap = chars; ap; ap = ap->next) {
        if (!strcasecmp(ap->name, pl_name)) break;
        last = ap;
    }
    /* If we didn't find this character, nothing to do */
    if (!ap) return (chars);

    /* As per previous notes, these should never be NULL */
    free_string(ap->name);
    free_string(ap->character_class);
    free_string(ap->race);
    free_string(ap->face);
    free_string(ap->party);
    free_string(ap->map);

    /* remove this link, or update head of list as appropriate */
    if (last) {
        last->next = ap->next;
    } else {
        chars = ap->next;
    }
    free(ap);
    return (chars);

}

/**
 * This frees all data associated with the character information.
 *
 * @param chars
 * Data to free.  The caller should make sure it no longer uses
 * any data in this list.
 */
void account_char_free(Account_Char *chars) {
    Account_Char *ap, *next;

    for (ap = chars; ap; ap = next) {
        next = ap->next;

        free_string(ap->name);
        free_string(ap->character_class);
        free_string(ap->race);
        free_string(ap->face);
        free_string(ap->party);
        free_string(ap->map);
        free(ap);
    }
}

/**
 * This will edit the character account information so that the character
 * that just died in permadeath will be listed as such in the accounts file.
 *
 * @param op
 * The player experiencing permadeath.
 * @return
 * 0 for success, 1 for failure
 */
int make_perma_dead(object *op) {
    player *pl = op->contr;
    if (!pl) {
        return 1;
    }
    /* Is this necessary? I'm not sure. It was in the code I found to use as an example */
    pl = get_player(pl);
    /* Make sure there is an account name to do things to */
    if (!pl->socket.account_name) {
        return 1;
    }
    /* Load the appropriate account for the action. */
    Account_Char *chars = account_char_load(pl->socket.account_name);
    /* Find the right character. */
    Account_Char *ac;
    for (ac = chars; ac; ac = ac->next) {
        if (strcmp(ac->name, op->name) == 0)
            break;
    }
    /* This character is dead */
    ac->isDead = 1;
    account_char_save(pl->socket.account_name, chars);
    return 0;
}

/**
 * This will edit the character account information so that the character
 * that was just resurrected in permadeath will be listed as such in the accounts file.
 *
 * @param account
 * The account of the resurrected character.
 * @param player
 * The name of the resurrected character.
 * @return
 * 0 for success, 1 for failure
 */
int unmake_perma_dead(char *account, char *player) {
    /*
     * If no account name, then there is nothing to do here.
     * The character was dead before the account was kept track of.
     */
    if (!account)
        return 1;
    /* Load the appropriate account for the action. */
    Account_Char *chars = account_char_load(account);
    /* Find the right character. */
    Account_Char *ac;
    for (ac = chars; ac; ac = ac->next) {
        if (strcmp(ac->name, player) == 0)
            break;
    }
    /* This character is alive */
    ac->isDead = 0;
    account_char_save(account, chars);
    return 0;
}
