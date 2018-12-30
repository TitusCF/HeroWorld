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
 * Player login/logout/save functions.
 */

#include <global.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#endif
#include <spells.h>
#include <loader.h>
#include <define.h>

static void copy_file(const char *filename, FILE *fpout);

/**
 * Save all players.
 *
 * @param flag
 * if non zero, it means that we want to try and save everyone, but
 * keep the game running.  Thus, we don't want to free any information.
 */
void emergency_save(int flag) {
#ifndef NO_EMERGENCY_SAVE
    player *pl;

    trying_emergency_save = 1;
    LOG(llevError, "Emergency save:  ");
    for (pl = first_player; pl != NULL; pl = pl->next) {
        if (!pl->ob) {
            LOG(llevError, "No name, ignoring this.\n");
            continue;
        }
        LOG(llevError, "%s ", pl->ob->name);
        draw_ext_info(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_ADMIN,  MSG_TYPE_ADMIN_LOADSAVE,
                      "Emergency save...");

        /* If we are not exiting the game (ie, this is sort of a backup save), then
         * don't change the location back to the village.  Note that there are other
         * options to have backup saves be done at the starting village
         */
        if (!flag) {
            strcpy(pl->maplevel, first_map_path);
            if (pl->ob->map != NULL)
                pl->ob->map = NULL;
            pl->ob->x = -1;
            pl->ob->y = -1;
        }
        if (!save_player(pl->ob, flag)) {
            LOG(llevError, "(failed) ");
            draw_ext_info(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_LOADSAVE,
                          "Emergency save failed, checking score...");
        }
        hiscore_check(pl->ob, 1);
    }
    LOG(llevError, "\n");
#else
    LOG(llevInfo, "Emergency saves disabled, no save attempted\n");
#endif
}

/**
 * Totally deletes a character. The contents of its directory are effectively totally removed.
 * Used when a player 'quits' the game, or dies on a server with permadeath and no resurrect.
 *
 * @param name
 * player to delete.
 */
void delete_character(const char *name) {
    char buf[MAX_BUF];

    snprintf(buf, sizeof(buf), "%s/%s/%s", settings.localdir, settings.playerdir, name);
    /* this effectively does an rm -rf on the directory */
    remove_directory(buf);
}

/**
 * This verify that a character of name exits, and that it matches
 * password.
 *
 * @param name
 * player name.
 * @param password
 * player's password, not encrypted.
 * @retval 0
 * there is match.
 * @retval 1
 * no such player.
 * @retval 2
 * incorrect password.
 */
int verify_player(const char *name, char *password) {
    char buf[MAX_BUF];
    FILE *fp;

    if (strpbrk(name, "/.\\") != NULL) {
        LOG(llevError, "Username contains illegal characters: %s\n", name);
        return 1;
    }

    snprintf(buf, sizeof(buf), "%s/%s/%s/%s.pl", settings.localdir, settings.playerdir, name, name);
    if (strlen(buf) >= sizeof(buf)-1) {
        LOG(llevError, "Username too long: %s\n", name);
        return 1;
    }

    fp = fopen(buf, "r");
    if (fp == NULL)
        return 1;

    /* Read in the file until we find the password line.  Our logic could
     * be a bit better on cleaning up the password from the file, but since
     * it is written by the program, I think it is fair to assume that the
     * syntax should be pretty standard.
     */
    while (fgets(buf, MAX_BUF-1, fp) != NULL) {
        if (!strncmp(buf, "password ", 9)) {
            buf[strlen(buf)-1] = 0; /* remove newline */
            if (check_password(password, buf+9)) {
                fclose(fp);
                return 0;
            }

            fclose(fp);
            return 2;
        }
    }
    LOG(llevDebug, "Could not find a password line in player %s\n", name);
    fclose(fp);
    return 1;
}

/**
 * Ensure player's name is valid.
 *
 * @param me
 * player to report to.
 * @param name
 * name to check.
 * @retval 0
 * invalid name.
 * @retval 1
 * valid name.
 */
int check_name(player *me, const char *name) {
    if (*name == '\0') {
        draw_ext_info(NDI_UNIQUE, 0, me->ob, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_LOGIN,
                      "Your username cannot be blank.");
        return 0;
    }

    if (!playername_ok(name)) {
        draw_ext_info(NDI_UNIQUE, 0, me->ob, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_LOGIN,
                      "That name contains illegal characters. Use letters, hyphens and underscores only. Hyphens and underscores are not allowed as the first character.");
        return 0;
    }
    if (strlen(name) >= MAX_NAME) {
        draw_ext_info_format(NDI_UNIQUE, 0, me->ob, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_LOGIN,
                             "That name is too long. (Max length: %d characters)", MAX_NAME);
        return 0;
    }

    return 1;
}

/**
 * Recursively object_free_drop_inventory() op and its inventory.
 *
 * @param op
 * object to totally object_free_drop_inventory().
 * @todo trash that function
 */
void destroy_object(object *op) {
    while (op->inv != NULL)
        destroy_object(op->inv);

    if (!QUERY_FLAG(op, FLAG_REMOVED))
        object_remove(op);
    object_free_drop_inventory(op);
}

/**
 * Saves a player to disk.
 *
 * @param op
 * player to save.
 * @param flag
 * if is set, it's only backup, ie dont remove objects from inventory.
 * If BACKUP_SAVE_AT_HOME is set, and the flag is set, then the player
 * will be saved at the emergency save location.
 * @return
 * non zero if successful.
 */
int save_player(object *op, int flag) {
    FILE *fp;
    char filename[MAX_BUF], *tmpfilename, backupfile[MAX_BUF];
    object *container = NULL;
    player *pl = op->contr;
    int i, wiz = QUERY_FLAG(op, FLAG_WIZ);
    long checksum;
#ifdef BACKUP_SAVE_AT_HOME
    sint16 backup_x, backup_y;
#endif

    if (!op->stats.exp)
        return 0; /* no experience, no save */

    flag &= 1;

    if (!pl->name_changed || (!flag && !op->stats.exp)) {
        if (!flag) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_LOADSAVE,
                          "Your game is not valid, game not saved.");
        }
        return 0;
    }

    /* Sanity check - some stuff changes this when player is exiting */
    if (op->type != PLAYER)
        return 0;

    /* Prevent accidental saves if connection is reset after player has
     * mostly exited.
     */
    if (pl->state != ST_PLAYING && pl->state != ST_GET_PARTY_PASSWORD)
        return 0;

    if (flag == 0)
        pets_terminate_all(op);

    /* Update information on this character.  Only do it if it is eligible for
     * for saving.
     */
    pl->socket.account_chars = account_char_add(pl->socket.account_chars, pl);
    if (pl->socket.account_name) {
        account_char_save(pl->socket.account_name, pl->socket.account_chars);
        /* Add this character to the account.  This really only comes up
         * for new characters, at which time we want to wait until save -
         * otherwise there is a good chance that character will be
         * terminated.
         */
        if (!account_get_account_for_char(pl->ob->name))
            account_add_player_to_account(pl->socket.account_name, pl->ob->name);
    }


    snprintf(filename, sizeof(filename), "%s/%s/%s/%s.pl", settings.localdir, settings.playerdir, op->name, op->name);
    make_path_to_file(filename);
    fp = tempnam_secure(settings.tmpdir, NULL, &tmpfilename);
    if (!fp) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_LOADSAVE,
                      "Can't get secure temporary file for save.");
        LOG(llevDebug, "Can't get secure temporary file for save.\n");
        return 0;
    }

    /* Eneq(@csd.uu.se): If we have an open container hide it. */
    if (op->container)  {
        container = op->container;
        op->container = NULL;
    }

    fprintf(fp, "password %s\n", pl->password);
    if (settings.set_title == TRUE)
        if (player_has_own_title(pl))
            fprintf(fp, "title %s\n", player_get_own_title(pl));

    fprintf(fp, "gen_hp %d\n", pl->gen_hp);
    fprintf(fp, "gen_sp %d\n", pl->gen_sp);
    fprintf(fp, "gen_grace %d\n", pl->gen_grace);
    fprintf(fp, "listening %d\n", pl->listening);
    fprintf(fp, "shoottype %d\n", pl->shoottype);
    fprintf(fp, "bowtype %d\n", pl->bowtype);
    fprintf(fp, "petmode %d\n", pl->petmode);
    fprintf(fp, "peaceful %d\n", pl->peaceful);
    fprintf(fp, "no_shout %d\n", pl->no_shout);
    fprintf(fp, "digestion %d\n", pl->digestion);
    fprintf(fp, "pickup %u\n", pl->mode);
    /*
     * outputs_sync and outputs_count are now unused in favor of the facility
     * being supported on the client instead of in the server, but for now,
     * set sane values in case an older server is run on a new player file.
     * Once the server is officially 2.x, this should likely be removed.
     */
    fprintf(fp, "outputs_sync %d\n", 16);
    fprintf(fp, "outputs_count %d\n", 1);
    /* Match the enumerations but in string form */
    fprintf(fp, "usekeys %s\n", pl->usekeys == key_inventory ? "key_inventory" : (pl->usekeys == keyrings ? "keyrings" : "containers"));
    /* Match the enumerations but in string form */
    fprintf(fp, "unapply %s\n", pl->unapply == unapply_nochoice ? "unapply_nochoice" : (pl->unapply == unapply_never ? "unapply_never" : "unapply_always"));
    if (pl->unarmed_skill) fprintf(fp, "unarmed_skill %s\n", pl->unarmed_skill);

#ifdef BACKUP_SAVE_AT_HOME
    if (op->map != NULL && flag == 0)
#else
    if (op->map != NULL)
#endif
        fprintf(fp, "map %s\n", op->map->path);
    else
        fprintf(fp, "map %s\n", settings.emergency_mapname);

    fprintf(fp, "savebed_map %s\n", pl->savebed_map);
    fprintf(fp, "bed_x %d\nbed_y %d\n", pl->bed_x, pl->bed_y);
    fprintf(fp, "Str %d\n", pl->orig_stats.Str);
    fprintf(fp, "Dex %d\n", pl->orig_stats.Dex);
    fprintf(fp, "Con %d\n", pl->orig_stats.Con);
    fprintf(fp, "Int %d\n", pl->orig_stats.Int);
    fprintf(fp, "Pow %d\n", pl->orig_stats.Pow);
    fprintf(fp, "Wis %d\n", pl->orig_stats.Wis);
    fprintf(fp, "Cha %d\n", pl->orig_stats.Cha);

    fprintf(fp, "lev_array %d\n", MIN(op->level, 10));
    for (i = 1; i <= MIN(op->level, 10) && i <= 10; i++) {
        fprintf(fp, "%d\n", pl->levhp[i]);
        fprintf(fp, "%d\n", pl->levsp[i]);
        fprintf(fp, "%d\n", pl->levgrace[i]);
    }
    fprintf(fp, "party_rejoin_mode %d\n", pl->rejoin_party);
    if (pl->party != NULL) {
        fprintf(fp, "party_rejoin_name %s\n", pl->party->partyname);
        fprintf(fp, "party_rejoin_password %s\n", party_get_password(pl->party));
    }
    fprintf(fp, "language %s\n", i18n_get_language_code(pl->language));
    fprintf(fp, "ticks_played %u\n", pl->ticks_played);
    fprintf(fp, "endplst\n");

    SET_FLAG(op, FLAG_NO_FIX_PLAYER);
    CLEAR_FLAG(op, FLAG_WIZ);
#ifdef BACKUP_SAVE_AT_HOME
    if (flag) {
        backup_x = op->x;
        backup_y = op->y;
        op->x = -1;
        op->y = -1;
    }
    /* Save objects, but not unpaid objects.  Don't remove objects from
     * inventory.
     */
    i = save_object(fp, op, SAVE_FLAG_NO_REMOVE);
    if (flag) {
        op->x = backup_x;
        op->y = backup_y;
    }
#else
    i = save_object(fp, op, SAVE_FLAG_SAVE_UNPAID|SAVE_FLAG_NO_REMOVE); /* don't check and don't remove */
#endif

    if (wiz)
        SET_FLAG(op, FLAG_WIZ);

    if (fclose(fp) != 0 || i != SAVE_ERROR_OK) { /* make sure the write succeeded */
        draw_ext_info(NDI_UNIQUE|NDI_RED, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_LOADSAVE,
                      "Can't save character!");
        draw_ext_info_format(NDI_ALL_DMS|NDI_RED, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_LOADSAVE, "Save failure for player %s!", op->name);
        unlink(tmpfilename);
        free(tmpfilename);
        return 0;
    }

    CLEAR_FLAG(op, FLAG_NO_FIX_PLAYER);

    if (!flag) {
        while (op->inv != NULL)
            destroy_object(op->inv);

        /* destroying objects will most likely destroy the pointer
         * in op->contr->ranges[], so clear the range to a safe value.
         */
        op->contr->shoottype = range_none;
    }

    checksum = 0;
    snprintf(backupfile, sizeof(backupfile), "%s.tmp", filename);
    rename(filename, backupfile);
    fp = fopen(filename, "w");
    if (!fp) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_LOADSAVE,
                      "Can't open file for save.");
        unlink(tmpfilename);
        free(tmpfilename);
        return 0;
    }
    fprintf(fp, "checksum %lx\n", checksum);
    copy_file(tmpfilename, fp);
    unlink(tmpfilename);
    free(tmpfilename);
    if (fclose(fp) == EOF) { /* got write error */
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_LOADSAVE,
                      "Can't close file for save.");
        rename(backupfile, filename); /* Restore the original */
        return 0;
    } else
        unlink(backupfile);

    /* Eneq(@csd.uu.se): Reveal the container if we have one. */
    if (flag && container != NULL)
        op->container = container;

    if (!flag)
        esrv_send_inventory(op, op);

    chmod(filename, SAVE_MODE);

    /* if this is the first player save, quest or knowledge states can be unsaved */
    if (!op->contr->has_directory) {
        op->contr->has_directory = 1;
        knowledge_first_player_save(op->contr);
        quest_first_player_save(op->contr);
    }

    return 1;
}

/**
 * Copy a file.
 *
 * @param filename
 * source file.
 * @param fpout
 * where to copy to.
 */
static void copy_file(const char *filename, FILE *fpout) {
    FILE *fp;
    char buf[MAX_BUF];

    fp = fopen(filename, "r");
    if (fp == NULL) {
        LOG(llevError, "copy_file failed to open \"%s\", player file(s) may be corrupt.\n", filename);
        return;
    }
    while (fgets(buf, MAX_BUF, fp) != NULL)
        fputs(buf, fpout);
    fclose(fp);
}

/**
 * Simple function to print errors when password is
 * not correct, and reinitialise the name.
 *
 * @param op
 * player.
 */
static void wrong_password(object *op) {
    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_LOGIN,
                  "\nA character with this name already exists. "
                  "Please choose another name, or make sure you entered your "
                  "password correctly.\n");

    FREE_AND_COPY(op->name, "noname");
    FREE_AND_COPY(op->name_pl, "noname");

    op->contr->socket.password_fails++;
    if (op->contr->socket.password_fails >= MAX_PASSWORD_FAILURES) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_LOGIN,
                      "You gave an incorrect password too many times, "
                      "you will now be dropped from the server.");

        LOG(llevInfo, "A player connecting from %s has been dropped for password failure\n",
            op->contr->socket.host);

        op->contr->socket.status = Ns_Dead; /* the socket loop should handle the rest for us */
    } else
        get_name(op);
}

/**
 * Actually login a player, load from disk and such.
 *
 * @param op
 * player.
 * @param check_pass
 * If true, we should do password checking.  Otherwise, we just log this
 * character in.  This later setting is used for the account code,
 * where that already does authentication.
 *
 * @todo describe connect/login/logout/disconnect process.
 */
void check_login(object *op, int check_pass) {
    FILE *fp;
    char filename[MAX_BUF];
    char buf[MAX_BUF], bufall[MAX_BUF];
    int i, value;
    uint32 uvalue;
    player *pl = op->contr, *pltmp;
    int correct = 0;
    time_t elapsed_save_time = 0;
    struct stat statbuf;
    char *party_name = NULL, party_password[9];

    strcpy(pl->maplevel, first_map_path);
    party_password[0] = 0;

    /* Check if this matches a connected player, and if yes disconnect old / connect new. */
    for (pltmp = first_player; pltmp != NULL; pltmp = pltmp->next) {
        if (pltmp != pl && pltmp->ob->name != NULL && !strcmp(pltmp->ob->name, op->name)) {
            if (!check_pass || check_password(pl->write_buf+1, pltmp->password)) {
                /* We could try and be more clever and re-assign the existing
                 * object to the new player, etc.  However, I'm concerned that
                 * there may be a lot of other state that still needs to be sent
                 * in that case (we can't make any assumptions on what the
                 * client knows, as maybe the client crashed), so treating it
                 * as just a normal login is the safest and easiest thing to do.
                 */

                pltmp->socket.status = Ns_Dead;

                save_player(pltmp->ob, 0);
                leave(pltmp, 1);
                final_free_player(pltmp);
                break;
            }
            if (check_pass) {
                wrong_password(op);
                return;
            }
        }
    }

    snprintf(filename, sizeof(filename), "%s/%s/%s/%s.pl", settings.localdir, settings.playerdir, op->name, op->name);

    /* If no file, must be a new player, so lets get confirmation of
     * the password.  Return control to the higher level dispatch,
     * since the rest of this just deals with loading of the file.
     */
    fp = fopen(filename, "r");
    if (fp == NULL) {
        confirm_password(op);
        return;
    }
    if (fstat(fileno(fp), &statbuf)) {
        LOG(llevError, "Unable to stat %s?\n", filename);
        elapsed_save_time = 0;
    } else {
        elapsed_save_time = time(NULL)-statbuf.st_mtime;
        if (elapsed_save_time < 0) {
            LOG(llevError, "Player file %s was saved in the future? (%ld time)\n", filename, (long)elapsed_save_time);
            elapsed_save_time = 0;
        }
    }

    if (fgets(bufall, MAX_BUF, fp) != NULL) {
        if (!strncmp(bufall, "checksum ", 9)) {
            (void)fgets(bufall, MAX_BUF, fp);
        }
        if (sscanf(bufall, "password %s\n", buf)) {
            /* New password scheme: */
            correct = check_password(pl->write_buf+1, buf);
            if (!check_pass) {
                /* We want to preserve the password.  Normally,
                 * pl->password is filled in when user enters
                 * data in the password prompt, but with new login,
                 * there is no password prompt.
                 */
                strncpy(pl->password, buf, 15);
                pl->password[15] = 0;
            }
        }
        /* Old password mode removed - I have no idea what it
         * was, and the current password mechanism has been used
         * for at least several years.
         */
    }
    if (!correct && check_pass) {
        wrong_password(op);
        fclose(fp);
        return;
    }

#ifdef SAVE_INTERVAL
    pl->last_save_time = time(NULL);
#endif /* SAVE_INTERVAL */
    pl->party = NULL;
    if (settings.search_items == TRUE)
        pl->search_str[0] = '\0';
    pl->name_changed = 1;
    pl->orig_stats.Str = 0;
    pl->orig_stats.Dex = 0;
    pl->orig_stats.Con = 0;
    pl->orig_stats.Int = 0;
    pl->orig_stats.Pow = 0;
    pl->orig_stats.Wis = 0;
    pl->orig_stats.Cha = 0;
    strcpy(pl->savebed_map, first_map_path);
    pl->bed_x = 0,
    pl->bed_y = 0;
    pl->spellparam[0] = '\0';

    /* Loop through the file, loading the rest of the values */
    while (fgets(bufall, MAX_BUF, fp) != NULL) {
        char *val_string, *p;

        sscanf(bufall, "%s %d\n", buf, &value);

        val_string = bufall + strlen(buf) +1;
        p = strchr(val_string, '\n');
        if (p != NULL)
            *p = '\0';

        /* uvalue is an unsigned value.  Since at least a
         * couple different things want an usigned value, cleaner
         * to just do it once here vs everyplace it may be needed.
         */

        uvalue = strtoul(val_string, (char **)NULL, 10);

        if (!strcmp(buf, "endplst"))
            break;
        if (!strcmp(buf, "title") && settings.set_title == TRUE)
            player_set_own_title(pl, val_string);
        else if (!strcmp(buf, "unarmed_skill"))
            pl->unarmed_skill = add_string(val_string);
        else if (!strcmp(buf, "explore"))
            ; /* ignore: explore mode has been removed */
        else if (!strcmp(buf, "gen_hp"))
            pl->gen_hp = value;
        else if (!strcmp(buf, "shoottype"))
            pl->shoottype = (rangetype)value;
        else if (!strcmp(buf, "bowtype"))
            pl->bowtype = (bowtype_t)value;
        else if (!strcmp(buf, "petmode"))
            pl->petmode = (petmode_t)value;
        else if (!strcmp(buf, "gen_sp"))
            pl->gen_sp = value;
        else if (!strcmp(buf, "gen_grace"))
            pl->gen_grace = value;
        else if (!strcmp(buf, "listening"))
            pl->listening = value;
        else if (!strcmp(buf, "peaceful"))
            pl->peaceful = value;
        else if (!strcmp(buf, "no_shout"))
            pl->no_shout = value;
        else if (!strcmp(buf, "digestion"))
            pl->digestion = value;
        else if (!strcmp(buf, "pickup")) {
            pl->mode = uvalue;
        }
        else if (!strcmp(buf, "map"))
            snprintf(pl->maplevel, sizeof(pl->maplevel), "%s", val_string);
        else if (!strcmp(buf, "savebed_map"))
            snprintf(pl->savebed_map, sizeof(pl->savebed_map), "%s", val_string);
        else if (!strcmp(buf, "bed_x"))
            pl->bed_x = value;
        else if (!strcmp(buf, "bed_y"))
            pl->bed_y = value;
        else if (!strcmp(buf, "Str"))
            pl->orig_stats.Str = value;
        else if (!strcmp(buf, "Dex"))
            pl->orig_stats.Dex = value;
        else if (!strcmp(buf, "Con"))
            pl->orig_stats.Con = value;
        else if (!strcmp(buf, "Int"))
            pl->orig_stats.Int = value;
        else if (!strcmp(buf, "Pow"))
            pl->orig_stats.Pow = value;
        else if (!strcmp(buf, "Wis"))
            pl->orig_stats.Wis = value;
        else if (!strcmp(buf, "Cha"))
            pl->orig_stats.Cha = value;
        else if (!strcmp(buf, "usekeys")) {
            if (!strcmp(val_string, "key_inventory"))
                pl->usekeys = key_inventory;
            else if (!strcmp(val_string, "keyrings"))
                pl->usekeys = keyrings;
            else if (!strcmp(val_string, "containers"))
                pl->usekeys = containers;
            else
                LOG(llevDebug, "load_player: got unknown usekeys type: %s\n", val_string);
        } else if (!strcmp(buf, "unapply")) {
            if (!strcmp(val_string, "unapply_nochoice"))
                pl->unapply = unapply_nochoice;
            else if (!strcmp(val_string, "unapply_never"))
                pl->unapply = unapply_never;
            else if (!strcmp(val_string, "unapply_always"))
                pl->unapply = unapply_always;
            else
                LOG(llevDebug, "load_player: got unknown unapply type: %s\n", val_string);
        } else if (!strcmp(buf, "lev_array")) {
            for (i = 1; i <= value; i++) {
                int j;

                fscanf(fp, "%d\n", &j);
                if (j < 3)
                    j = 3;
                else if (j > 9)
                    j = 9;
                pl->levhp[i] = j;
                fscanf(fp, "%d\n", &j);
                if (j < 2)
                    j = 2;
                else if (j > 6)
                    j = 6;
                pl->levsp[i] = j;
                fscanf(fp, "%d\n", &j);
                if (j < 1)
                    j = 1;
                else if (j > 3)
                    j = 3;
                pl->levgrace[i] = j;
            }
        } else if (!strcmp(buf, "party_rejoin_mode"))
            pl->rejoin_party = value;
        else if (!strcmp(buf, "party_rejoin_name"))
            party_name = strdup_local(val_string);
        else if (!strcmp(buf, "party_rejoin_password")) {
            strncpy(party_password, val_string, sizeof(party_password));
            party_password[sizeof(party_password) - 1] = 0;
        } else if (!strcmp(buf, "language")) {
            pl->language = i18n_get_language_by_code(val_string);
        }
        else if (!strcmp(buf, "ticks_played")) {
            pl->ticks_played = uvalue;
        }
    } /* End of loop loading the character file */

    /* on first login via account, this player does not exist anyplace -
     * so don't remove them.
     */
    if (!QUERY_FLAG(op, FLAG_REMOVED))
        object_remove(op);
    op->speed = 0;
    object_update_speed(op);
    /*FIXME dangerous call, object_reset() should be used to init freshly allocated obj struct!*/
    object_reset(op);
    op->contr = pl;
    pl->ob = op;
    /* this loads the standard objects values. */
    load_object(fp, op, LO_NEWFILE, 0);
    fclose(fp);

    CLEAR_FLAG(op, FLAG_NO_FIX_PLAYER);

    strncpy(pl->title, op->arch->clone.name, sizeof(pl->title)-1);
    pl->title[sizeof(pl->title)-1] = '\0';

    /* If the map where the person was last saved does not exist,
     * restart them on their home-savebed. This is good for when
     * maps change between versions
     * First, we check for partial path, then check to see if the full
     * path (for unique player maps)
     */
    if (has_been_loaded(pl->maplevel) == NULL
    && check_path(pl->maplevel, 1) == -1
    && check_path(pl->maplevel, 0) == -1) {
        strcpy(pl->maplevel, pl->savebed_map);
        op->x = pl->bed_x,
        op->y = pl->bed_y;
        /* if the map was a shop, the player can have unpaid items, remove them. */
        remove_unpaid_objects(op, NULL, 1);
    }

    /* If player saved beyond some time ago, and the feature is
     * enabled, put the player back on his savebed map.
     */
    if ((settings.reset_loc_time > 0) && (elapsed_save_time > settings.reset_loc_time)) {
        strcpy(pl->maplevel, pl->savebed_map);
        op->x = pl->bed_x, op->y = pl->bed_y;
        /* if the map was a shop, the player can have unpaid items, remove them. */
        remove_unpaid_objects(op, NULL, 1);
    }

    /* make sure he's a player--needed because of class change. */
    op->type = PLAYER;

    enter_exit(op, NULL);

    pl->name_changed = 1;
    player_set_state(pl, ST_PLAYING);
#ifdef AUTOSAVE
    pl->last_save_tick = pticks;
#endif
    op->carrying = object_sum_weight(op);

    link_player_skills(op);

    if (!legal_range(op, op->contr->shoottype))
        op->contr->shoottype = range_none;

    /* if it's a dragon player, set the correct title here */
    if (is_dragon_pl(op) && op->inv != NULL) {
        object *abil, *skin;

        abil = object_find_by_type_and_arch_name(op, FORCE, "dragon_ability_force");
        skin = object_find_by_type_and_arch_name(op, FORCE, "dragon_skin_force");
        set_dragon_name(op, abil, skin);
    }

    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_LOGIN,
                  "Welcome Back!");
    draw_ext_info_format(NDI_UNIQUE|NDI_ALL|NDI_DK_ORANGE, 5, NULL,
                         MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_PLAYER,
                         "%s has entered the game.",
                         pl->ob->name);

    /* Lauwenmark : Here we handle the LOGIN global event */
    execute_global_event(EVENT_LOGIN, pl, pl->socket.host);
    op->contr->socket.update_look = 1;
    /* If the player should be dead, call kill_player for them
     * Only check for hp - if player lacks food, let the normal
     * logic for that to take place.  If player is permanently
     * dead, and not using permadeath mode, the kill_player will
     * set the play_again flag, so return.
     */
    if (op->stats.hp < 0) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_LOGIN,
                      "Your character was dead last time you played.");
        kill_player(op, NULL);
        if (pl->state != ST_PLAYING)
            return;
    }
    LOG(llevInfo, "LOGIN: Player named %s from ip %s\n", op->name,
        op->contr->socket.host);

    /* Do this after checking for death - no reason sucking up bandwidth if
     * the data isn't needed.
     */
    esrv_new_player(op->contr, op->weight+op->carrying);
    /* Need to do these after esvr_new_player, as once the client
     * sees that, it wipes any info it has about the player.
     */
    esrv_add_spells(op->contr, NULL);

    /* Need to call fix_object now - program modified so that it is not
     * called during the load process (FLAG_NO_FIX_PLAYER set when
     * saved)
     * Moved ahead of the esrv functions, so proper weights will be
     * sent to the client.  Needs to be after esvr_add_spells, otherwise
     * we'll try to update spells from fix_object.
     */
    fix_object(op);

    pl->has_directory = 1;

    esrv_send_inventory(op, op);
    esrv_send_pickup(pl);
    quest_send_initial_states(pl);
    knowledge_send_known(pl);

    CLEAR_FLAG(op, FLAG_FRIENDLY);

    /* can_use_shield is a new flag.  However, the can_use.. seems to largely come
     * from the class, and not race.  I don't see any way to get the class information
     * to then update this.  I don't think this will actually break anything - anyone
     * that can use armour should be able to use a shield.  What this may 'break'
     * are features new characters get, eg, if someone starts up with a Q, they
     * should be able to use a shield.  However, old Q's won't get that advantage.
     */
    if (QUERY_FLAG(op, FLAG_USE_ARMOUR))
        SET_FLAG(op, FLAG_USE_SHIELD);

    /* Rejoin party if needed. */
    if (pl->rejoin_party != party_rejoin_no && party_name != NULL) {
        partylist *party;

        party = party_find(party_name);
        if (!party && pl->rejoin_party == party_rejoin_always) {
            party = party_form(op, party_name);
            if (party)
                party_set_password(party, party_password);
        }
        if (party && !pl->party && party_confirm_password(party, party_password)) {
            party_join(op, party);
        }

        if (pl->party)
            snprintf(buf, MAX_BUF, "Rejoined party %s.", party->partyname);
        else
            snprintf(buf, MAX_BUF, "Couldn't rejoin party %s: %s.", party_name, party ? "invalid password." : "no such party.");
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                      buf);
    }
    free(party_name);
}
