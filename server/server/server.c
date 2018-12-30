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
 * Main server functions.
 */

#include <assert.h>
#include "global.h"

#ifdef HAVE_TIME_H
#include <time.h>
#endif

#ifndef WIN32
#  include <unistd.h>
#  include <sys/types.h>
#endif

#ifdef HAVE_DES_H
#include <des.h>
#else
#  ifdef HAVE_CRYPT_H
#  include <crypt.h>
#  endif
#endif

#include "object.h"
#include "sproto.h"
#include "tod.h"
#include "version.h"

#include "../random_maps/random_map.h"
#include "../random_maps/rproto.h"
#include "path.h"

/** Ingame days. */
static const char days[7][4] = {
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat"
};

/**
 * Displays basic game version information.
 *
 * @param op
 * who is requesting the verison information.
 */
void version(object *op) {
    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_VERSION,
                         "This is Crossfire v%s",
                         FULL_VERSION);
    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_VERSION,
"The authors can be reached at crossfire@metalforge.org");
}

/**
 * Gives basic start information to player.
 *
 * @param op
 * player.
 */
void start_info(object *op) {
    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_LOGIN,
                         "Welcome to Crossfire, v%s!\nPress `?' for help\n",
                         VERSION);

    draw_ext_info_format(NDI_UNIQUE|NDI_ALL|NDI_DK_ORANGE, 5, op,
                         MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_PLAYER,
                         "%s entered the game.",
                         op->name);
}

/**
 * Encrypt a string. Used for password storage on disk.
 *
 * Really, there is no reason to crypt the passwords  any system.  But easier
 * to just leave this enabled for backward compatibility.  Put the
 * simple case at top - no encryption - makes it easier to read.
 *
 * @param str
 * string to crypt.
 * @param salt
 * salt to crypt with.
 * @return
 * crypted str.
 * @todo make thread-safe?
 */
const char *crypt_string(const char *str, const char *salt) {
#if defined(WIN32) || (defined(__FreeBSD__) && !defined(HAVE_LIBDES))
    return(str);
#else
    static const char *const c = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";
    char s[2];

    if (salt == NULL)
        s[0] = c[RANDOM()%(int)strlen(c)],
        s[1] = c[RANDOM()%(int)strlen(c)];
    else
        s[0] = salt[0],
        s[1] = salt[1];

#  ifdef HAVE_LIBDES
    return (char *)des_crypt(str, s);
#  endif
    /* Default case - just use crypt */
    return (char *)crypt(str, s);
#endif
}

/**
 * Check if 2 passwords match.
 *
 * @param typed
 * entered password. Not crypted.
 * @param crypted
 * password to check against. Must be crypted.
 * @return
 * 1 if the passwords match, 0 else.
 */
int check_password(const char *typed, const char *crypted) {
    /* If crypted is an empty string, crypt_string() will return null, leading to issues with the strcmp. */
    if (strlen(crypted) == 0) {
        return strlen(typed) == 0 ? 1 : 0;
    }
    return !strcmp(crypt_string(typed, crypted), crypted);
}

/**
 * This is a basic little function to put the player back to his
 * savebed.  We do some error checking - its possible that the
 * savebed map may no longer exist, so we make sure the player
 * goes someplace.
 *
 * @param op
 * player.
 */
void enter_player_savebed(object *op) {
    mapstruct *oldmap = op->map;
    object *tmp;

    tmp = object_new();

    EXIT_PATH(tmp) = add_string(op->contr->savebed_map);
    EXIT_X(tmp) = op->contr->bed_x;
    EXIT_Y(tmp) = op->contr->bed_y;
    enter_exit(op, tmp);
    /* If the player has not changed maps and the name does not match
     * that of the savebed, his savebed map is gone.  Lets go back
     * to the emergency path.  Update what the players savebed is
     * while we're at it.
     */
    if (oldmap == op->map && strcmp(op->contr->savebed_map, oldmap->path)) {
        LOG(llevDebug, "Player %s savebed location %s is invalid - going to emergency location (%s)\n", settings.emergency_mapname, op->name, op->contr->savebed_map);
        strcpy(op->contr->savebed_map, settings.emergency_mapname);
        op->contr->bed_x = settings.emergency_x;
        op->contr->bed_y = settings.emergency_y;
        free_string(op->contr->savebed_map);
        EXIT_PATH(tmp) = add_string(op->contr->savebed_map);
        EXIT_X(tmp) = op->contr->bed_x;
        EXIT_Y(tmp) = op->contr->bed_y;
        enter_exit(op, tmp);
    }
    object_free2(tmp, FREE_OBJ_NO_DESTROY_CALLBACK);
}

/**
 * Moves the player and pets from current map (if any) to
 * new map.
 *
 * @param op
 * player to move.
 * @param newmap
 * @param x
 * @param y
 * new location. If (x, y) point to an out of map point, will use default map coordinates.
 */
static void enter_map(object *op, mapstruct *newmap, int x, int y) {
    mapstruct *oldmap = op->map;

    if (out_of_map(newmap, x, y)) {
        LOG(llevError, "enter_map: supplied coordinates are not within the map! (%s: %d, %d)\n", newmap->path, x, y);
        /* If op has invalid (probably -1,-1) coordinates, force them to a correct value, else issues later on. */
        if (op->x == x)
            op->x = MAP_ENTER_X(newmap);
        if (op->y == y)
            op->y = MAP_ENTER_Y(newmap);
        x = MAP_ENTER_X(newmap);
        y = MAP_ENTER_Y(newmap);
        if (out_of_map(newmap, x, y)) {
            LOG(llevError, "enter_map: map %s provides invalid default enter location (%d, %d) > (%d, %d)\n", newmap->path, x, y, MAP_WIDTH(newmap), MAP_HEIGHT(newmap));
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_FAILURE,
                          "The exit is closed");
            return;
        }
    }
    /* try to find a spot for the player */
    if (ob_blocked(op, newmap, x, y)) {        /* First choice blocked */
        /* We try to find a spot for the player, starting closest in.
         * We could use object_find_first_free_spot(), but that doesn't randomize it at all,
         * So for example, if the north space is free, you would always end up there even
         * if other spaces around are available.
         * Note that for the second and third calls, we could start at a position other
         * than one, but then we could end up on the other side of walls and so forth.
         */
        int i = object_find_free_spot(op, newmap, x, y, 1, SIZEOFFREE1+1);
        if (i == -1) {
            i = object_find_free_spot(op, newmap, x, y, 1, SIZEOFFREE2+1);
            if (i == -1)
                i = object_find_free_spot(op, newmap, x, y, 1, SIZEOFFREE);
        }
        if (i != -1) {
            x += freearr_x[i];
            y += freearr_y[i];
        } else {
            /* not much we can do in this case. */
            LOG(llevInfo, "enter_map: Could not find free spot for player - will dump on top of object (%s: %d, %d)\n", newmap->path, x, y);
        }
    } /* end if looking for free spot */

    /* If it is a player login, he has yet to be inserted anyplace.
     * otherwise, we need to deal with removing the playe here.
     */
    if (!QUERY_FLAG(op, FLAG_REMOVED))
        object_remove(op);
    if (op->map != NULL) {
        /* Lauwenmark : Here we handle the MAPLEAVE global event */
        execute_global_event(EVENT_MAPLEAVE, op, op->map);
    }
    /* object_remove clears these so they must be reset after the object_remove() call */
    object_insert_in_map_at(op, newmap, NULL, INS_NO_WALK_ON, x, y);

    /* Lauwenmark : Here we handle the MAPENTER global event */
    execute_global_event(EVENT_MAPENTER, op, op->map);

    if (op->contr) {
        send_background_music(op->contr, newmap->background_music);
    }

    newmap->timeout = 0;
    object_set_enemy(op, NULL);

    if (op->contr) {
        strcpy(op->contr->maplevel, newmap->path);
        op->contr->count = 0;
    }

    /* Update any golems */
    if (op->type == PLAYER && op->contr->ranges[range_golem] != NULL) {
        int i = object_find_free_spot(op->contr->ranges[range_golem], newmap, x, y, 1, SIZEOFFREE);
        object_remove(op->contr->ranges[range_golem]);
        if (i == -1) {
            remove_friendly_object(op->contr->ranges[range_golem]);
            object_free_drop_inventory(op->contr->ranges[range_golem]);
            op->contr->ranges[range_golem] = NULL;
            op->contr->golem_count = 0;
        } else {
            object_insert_in_map_at(op->contr->ranges[range_golem], newmap, NULL, 0, x+freearr_x[i], y+freearr_y[i]);
            op->contr->ranges[range_golem]->direction = find_dir_2(op->x-op->contr->ranges[range_golem]->x, op->y-op->contr->ranges[range_golem]->y);
        }
    }
    op->direction = 0;

    /* since the players map is already loaded, we don't need to worry
     * about pending objects.
     */
    pets_remove_all();

    /* If the player is changing maps, we need to do some special things
     * Do this after the player is on the new map - otherwise the force swap of the
     * old map does not work.
     */
    if (oldmap != newmap) {
        if (oldmap) { /* adjust old map */
            if (oldmap->players <= 0) /* can be less than zero due to errors in tracking this */
                set_map_timeout(oldmap);
        }
    }
    swap_below_max(newmap->path);

    if (op->type == PLAYER)
        map_newmap_cmd(&op->contr->socket);
}

/**
 * Applies the map timeout.
 *
 * @param oldmap
 * map to process.
 */
void set_map_timeout(mapstruct *oldmap) {
#if MAP_MAXTIMEOUT
    oldmap->timeout = MAP_TIMEOUT(oldmap);
    /* Do MINTIMEOUT first, so that MAXTIMEOUT is used if that is
     * lower than the min value.
     */
#if MAP_MINTIMEOUT
    if (oldmap->timeout < MAP_MINTIMEOUT) {
        oldmap->timeout = MAP_MINTIMEOUT;
    }
#endif
    if (oldmap->timeout > MAP_MAXTIMEOUT) {
        oldmap->timeout = MAP_MAXTIMEOUT;
    }
#else
    /* save out the map */
    swap_map(oldmap);
#endif /* MAP_MAXTIMEOUT */
}

/**
 * Takes a path and replaces all / with _
 * We do a strcpy so that we do not change the original string.
 *
 * @param file
 * path to clean.
 * @param newpath
 * buffer that will contain the cleaned path. Should be at least as long as file.
 * @param size
 * length of newpath.
 * @return
 * newpath.
 */
static char *clean_path(const char *file, char *newpath, int size) {
    char *cp;

    snprintf(newpath, size, "%s", file);
    for (cp = newpath; *cp != '\0'; cp++) {
        if (*cp == '/')
            *cp = '_';
    }
    return newpath;
}

/**
 * Takes a path and replaces all _ with /
 * This basically undoes clean_path().
 * We do a strcpy so that we do not change the original string.
 * We are smart enough to start after the last / in case we
 * are getting passed a string that points to a unique map
 * path.
 *
 * @param src
 * path to unclean.
 * @param newpath
 * buffer that will contain the uncleaned path. Should be at least as long as file.
 * @param size
 * length of newpath.
 * @return
 * newpath.
 */
static char *unclean_path(const char *src, char *newpath, int size) {
    char *cp;

    cp = strrchr(src, '/');
    if (cp)
        snprintf(newpath, size, "%s", cp+1);
    else
        snprintf(newpath, size, "%s", src);

    for (cp = newpath; *cp != '\0'; cp++) {
        if (*cp == '_')
            *cp = '/';
    }
    return newpath;
}


/**
 * The player is trying to enter a randomly generated map.  In this case, generate the
 * random map as needed.
 *
 * @param pl
 * player.
 * @param exit_ob
 * exit containing random map parameters.
 */
static void enter_random_map(object *pl, object *exit_ob) {
    mapstruct *new_map;
    char newmap_name[HUGE_BUF], *cp;
    static int reference_number = 0;
    RMParms rp;

    memset(&rp, 0, sizeof(RMParms));
    rp.Xsize = -1;
    rp.Ysize = -1;
    rp.region = get_region_by_map(exit_ob->map);
    if (exit_ob->msg)
        set_random_map_variable(&rp, exit_ob->msg);
    rp.origin_x = exit_ob->x;
    rp.origin_y = exit_ob->y;
    strcpy(rp.origin_map, pl->map->path);

    /* If we have a final_map, use it as a base name to give some clue
     * as where the player is.  Otherwise, use the origin map.
     * Take the last component (after the last slash) to give
     * shorter names without bogus slashes.
     */
    if (rp.final_map[0]) {
        cp = strrchr(rp.final_map, '/');
        if (!cp)
            cp = rp.final_map;
    } else {
        char buf[HUGE_BUF];

        cp = strrchr(rp.origin_map, '/');
        if (!cp)
            cp = rp.origin_map;
        /* Need to strip of any trailing digits, if it has them */
        snprintf(buf, sizeof(buf), "%s", cp);
        while (isdigit(buf[strlen(buf)-1]))
            buf[strlen(buf)-1] = 0;
        cp = buf;
    }

    snprintf(newmap_name, sizeof(newmap_name), "/random/%s%04d", cp+1, reference_number++);

    /* now to generate the actual map. */
    new_map = generate_random_map(newmap_name, &rp, NULL);

    /* Update the exit_ob so it now points directly at the newly created
     * random maps.  Not that it is likely to happen, but it does mean that a
     * exit in a unique map leading to a random map will not work properly.
     * It also means that if the created random map gets reset before
     * the exit leading to it, that the exit will no longer work.
     */
    if (new_map) {
        int x, y;

        x = EXIT_X(exit_ob) = MAP_ENTER_X(new_map);
        y = EXIT_Y(exit_ob) = MAP_ENTER_Y(new_map);
        EXIT_PATH(exit_ob) = add_string(newmap_name);
        snprintf(new_map->path, sizeof(new_map->path), "%s", newmap_name);
        enter_map(pl, new_map, x, y);
    }
}

/**
 * The player is trying to enter a non-randomly generated template map.  In this
 * case, use a map file for a template.
 *
 * @param pl
 * player.
 * @param exit_ob
 * exit containing template map parameters.
 */
static void enter_fixed_template_map(object *pl, object *exit_ob) {
    mapstruct *new_map;
    char tmpnum[32], exitpath[HUGE_BUF], resultname[HUGE_BUF], tmpstring[HUGE_BUF], *sourcemap;
    char new_map_name[MAX_BUF];

    /* Split the exit path string into two parts, one
     * for where to store the map, and one for were
     * to generate the map from.
     */
    snprintf(exitpath, sizeof(exitpath), "%s", EXIT_PATH(exit_ob)+2);
    sourcemap = strchr(exitpath, '!');
    if (!sourcemap) {
        draw_ext_info_format(NDI_UNIQUE, 0, pl,
                             MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_FAILURE,
                             "The %s is closed.",
                             exit_ob->name);
        /* Should only occur when no source map is set.
         */
        LOG(llevError, "enter_fixed_template_map: Exit %s (%d,%d) on map %s has no source template.\n", exit_ob->name, exit_ob->x, exit_ob->y, exit_ob->map->path);
        return;
    }
    *sourcemap++ = '\0';

    /* If we are not coming from a template map, we can use relative directories
     * for the map to generate from.
     */
    if (!exit_ob->map->is_template) {
        /* We can't use exitpath directly, as sourcemap points there. */
        path_combine_and_normalize(exit_ob->map->path, sourcemap, tmpstring, sizeof(tmpstring));
        snprintf(exitpath, sizeof(exitpath), "%s", tmpstring);
        sourcemap = exitpath;
    }

    /* Do replacement of %x, %y, and %n to the x coord of the exit, the y coord
     * of the exit, and the name of the map the exit is on, respectively.
     */
    snprintf(tmpnum, sizeof(tmpnum), "%d", exit_ob->x);
    replace(exitpath, "%x", tmpnum, resultname,  sizeof(resultname));

    snprintf(tmpnum, sizeof(tmpnum), "%d", exit_ob->y);
    snprintf(tmpstring, sizeof(tmpstring), "%s", resultname);
    replace(tmpstring, "%y", tmpnum, resultname,  sizeof(resultname));

    snprintf(tmpstring, sizeof(tmpstring), "%s", resultname);
    replace(tmpstring, "%n", exit_ob->map->name, resultname,  sizeof(resultname));

    /* If we are coming from another template map, use reletive paths unless
     * indicated otherwise.
     */
    if (exit_ob->map->is_template && (resultname[0] != '/')) {
        path_combine_and_normalize(exit_ob->map->path, resultname, new_map_name, sizeof(new_map_name));
    } else {
        create_template_pathname(resultname, new_map_name, sizeof(new_map_name));
    }

    /* Attempt to load the map, if unable to, then
     * create the map from the template.
     */
    new_map = ready_map_name(new_map_name, MAP_PLAYER_UNIQUE);
    if (!new_map) {
        char path[MAX_BUF];

        create_pathname(sourcemap, path, MAX_BUF);
        new_map = load_original_map(path, MAP_PLAYER_UNIQUE);
    }

    if (new_map) {
        /* set the path of the map to where it should be
         * so we don't just save over the source map.
         */
        snprintf(new_map->path, sizeof(new_map->path), "%s", new_map_name);
        new_map->is_template = 1;
        enter_map(pl, new_map, EXIT_X(exit_ob), EXIT_Y(exit_ob));
    } else {
        draw_ext_info_format(NDI_UNIQUE, 0, pl,
                             MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_FAILURE,
                             "The %s is closed.",
                             exit_ob->name);
        /* Should only occur when an invalid source map is set.
         */
        LOG(llevDebug, "enter_fixed_template_map: Exit %s (%d,%d) on map %s leads no where.\n", exit_ob->name, exit_ob->x, exit_ob->y, exit_ob->map->path);
    }
}

/**
 * The player is trying to enter a randomly generated template map.  In this
 * case, generate the map as needed.
 *
 * @param pl
 * player.
 * @param exit_ob
 * exit containing random template map parameters.
 */
static void enter_random_template_map(object *pl, object *exit_ob) {
    mapstruct *new_map;
    char tmpnum[32], resultname[HUGE_BUF], tmpstring[HUGE_BUF];
    char new_map_name[MAX_BUF];
    RMParms rp;

    /* Do replacement of %x, %y, and %n to the x coord of the exit, the y coord
     * of the exit, and the name of the map the exit is on, respectively.
     */
    snprintf(tmpnum, sizeof(tmpnum), "%d", exit_ob->x);
    replace(EXIT_PATH(exit_ob)+3, "%x", tmpnum, resultname,  sizeof(resultname));

    snprintf(tmpnum, sizeof(tmpnum), "%d", exit_ob->y);
    snprintf(tmpstring, sizeof(tmpstring), "%s", resultname);
    replace(tmpstring, "%y", tmpnum, resultname,  sizeof(resultname));

    snprintf(tmpstring, sizeof(tmpstring), "%s", resultname);
    replace(tmpstring, "%n", exit_ob->map->name, resultname,  sizeof(resultname));

    /* If we are coming from another template map, use reletive paths unless
     * indicated otherwise.
     */
    if (exit_ob->map->is_template && (resultname[0] != '/')) {
        path_combine_and_normalize(exit_ob->map->path, resultname, new_map_name, sizeof(new_map_name));
    } else {
        create_template_pathname(resultname, new_map_name, sizeof(new_map_name));
    }

    new_map = ready_map_name(new_map_name, MAP_PLAYER_UNIQUE);
    if (!new_map) {
        memset(&rp, 0, sizeof(RMParms));
        rp.Xsize = -1;
        rp.Ysize = -1;
        rp.region = get_region_by_map(exit_ob->map);
        if (exit_ob->msg)
            set_random_map_variable(&rp, exit_ob->msg);
        rp.origin_x = exit_ob->x;
        rp.origin_y = exit_ob->y;
        strcpy(rp.origin_map, pl->map->path);

        /* now to generate the actual map. */
        new_map = generate_random_map(new_map_name, &rp, NULL);
    }

    /* Update the exit_ob so it now points directly at the newly created
     * random maps.  Not that it is likely to happen, but it does mean that a
     * exit in a unique map leading to a random map will not work properly.
     * It also means that if the created random map gets reset before
     * the exit leading to it, that the exit will no longer work.
     */
    if (new_map) {
        int x, y;

        x = EXIT_X(exit_ob) = MAP_ENTER_X(new_map);
        y = EXIT_Y(exit_ob) = MAP_ENTER_Y(new_map);
        new_map->is_template = 1;
        enter_map(pl, new_map, x, y);
    }
}

/**
 * Player is entering a unique map.
 *
 * @param op
 * player.
 * @param exit_ob
 * exit containing unique map information.
 */
static void enter_unique_map(object *op, object *exit_ob) {
    char apartment[HUGE_BUF], path[MAX_BUF];
    mapstruct        *newmap;

    if (EXIT_PATH(exit_ob)[0] == '/') {
        snprintf(apartment, sizeof(apartment), "%s/%s/%s/%s", settings.localdir, settings.playerdir, op->name, clean_path(EXIT_PATH(exit_ob), path, sizeof(path)));
        newmap = ready_map_name(apartment, MAP_PLAYER_UNIQUE);
        if (!newmap) {
            create_pathname(EXIT_PATH(exit_ob), path, sizeof(path));
            newmap = load_original_map(path, MAP_PLAYER_UNIQUE);
        }
    } else { /* relative directory */
        char reldir[HUGE_BUF], tmpc[HUGE_BUF], *cp;

        if (exit_ob->map->unique) {
            unclean_path(exit_ob->map->path, reldir, sizeof(reldir));

            /* Need to copy this over, as clean_path only has one static return buffer */
            clean_path(reldir, tmpc, sizeof(tmpc));
            /* Remove final component, if any */
            if ((cp = strrchr(tmpc, '_')) != NULL)
                *cp = 0;

            snprintf(apartment, sizeof(apartment), "%s/%s/%s/%s_%s", settings.localdir, settings.playerdir, op->name, tmpc, clean_path(EXIT_PATH(exit_ob), path, sizeof(path)));

            newmap = ready_map_name(apartment, MAP_PLAYER_UNIQUE);
            if (!newmap) {
                create_pathname(path_combine_and_normalize(reldir, EXIT_PATH(exit_ob), tmpc, sizeof(tmpc)), path, sizeof(path));
                newmap = load_original_map(path, MAP_PLAYER_UNIQUE);
            }
        } else {
            /* The exit is unique, but the map we are coming from is not unique.  So
             * use the basic logic - don't need to demangle the path name
             */
            path_combine_and_normalize(exit_ob->map->path, EXIT_PATH(exit_ob), reldir, sizeof(reldir));
            snprintf(apartment, sizeof(apartment), "%s/%s/%s/%s", settings.localdir, settings.playerdir, op->name, clean_path(reldir, path, sizeof(path)));
            newmap = ready_map_name(apartment, MAP_PLAYER_UNIQUE);
            if (!newmap) {
                path_combine_and_normalize(exit_ob->map->path, EXIT_PATH(exit_ob), reldir, sizeof(reldir));
                newmap = ready_map_name(reldir, 0);
                if (newmap)
                    apply_auto_fix(newmap);
            }
        }
    }

    if (newmap) {
        snprintf(newmap->path, sizeof(newmap->path), "%s", apartment);
        newmap->unique = 1;
        enter_map(op, newmap, EXIT_X(exit_ob), EXIT_Y(exit_ob));
    } else {
        draw_ext_info_format(NDI_UNIQUE, 0, op,
                             MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_FAILURE,
                             "The %s is closed.",
                             exit_ob->name);
        /* Perhaps not critical, but I would think that the unique maps
         * should be new enough this does not happen.  This also creates
         * a strange situation where some players could perhaps have visited
         * such a map before it was removed, so they have the private
         * map, but other players can't get it anymore.
         */
        LOG(llevDebug, "enter_unique_map: Exit %s (%d,%d) on map %s is leads no where.\n", exit_ob->name, exit_ob->x, exit_ob->y, exit_ob->map->path);
    }
}

/**
 * Try to move op to the exit op->maplevel points to.
 * Will abort() if maplevel is invalid and the emergency map is invalid too.
 * @param op what to move, must not be NULL and must be a player.
 */
static void enter_maplevel_exit(object *op) {
    int flags = 0, x = op->x, y = op->y;
    mapstruct *newmap;

    assert(op != NULL);
    assert(op->type == PLAYER);

    /* Hypothetically, I guess its possible that a standard map matches
     * the localdir, but that seems pretty unlikely - unlikely enough that
     * I'm not going to attempt to try to deal with that possibility.
     * We use the fact that when a player saves on a unique map, it prepends
     * the localdir to that name.  So its an easy way to see of the map is
     * unique or not.
     */
    if (!strncmp(op->contr->maplevel, settings.localdir, strlen(settings.localdir)))
        flags = MAP_PLAYER_UNIQUE;

    /* newmap returns the map (if already loaded), or loads it for us. */
    newmap = ready_map_name(op->contr->maplevel, flags);
    if (!newmap) {
        LOG(llevError, "enter_exit: Map at '%s' doesn't exist!\n",
                op->contr->maplevel);
        newmap = ready_map_name(settings.emergency_mapname, 0);
        x = settings.emergency_x;
        y = settings.emergency_y;
        /* If we can't load the emergency map, something is probably really
         * screwed up, so bail out now.
         */
        if (!newmap) {
            LOG(llevError, "enter_exit: could not load emergency map? Fatal error\n");
            abort();
        }

        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_MISC, 0,
                "You find yourself somewhere unexpected...");
    }
    /* as a special case, if coordinates are (-1, -1), then the item should
     * be put at the default location. Used for loginmethod 0 (old clients). */
    if (x == -1 && y == -1) {
        x = MAP_ENTER_X(newmap);
        y = MAP_ENTER_Y(newmap);
    }
    enter_map(op, newmap, x, y);
}

/**
 * Tries to move 'op' to exit_ob.
 *
 * This is used when loading the player.
 *
 * Largely redone by MSW 2001-01-21 - this function was overly complex
 * and had some obscure bugs.
 *
 * May call abort() in some situations.
 *
 * @param op
 * character or monster that is using the exit.
 * @param exit_ob
 * exit object (boat, door, teleporter, etc.). if null, then op->contr->maplevel contains that map to
 * move the object to.
 */
void enter_exit(object *op, object *exit_ob) {
#define PORTAL_DESTINATION_NAME "Town portal destination" /* this one should really be in a header file */
    /* It may be nice to support other creatures moving across
     * exits, but right now a lot of the code looks at op->contr,
     * so thta is an RFE.
     */
    if (op->type != PLAYER)
        return;

    /* Need to remove player from transport */
    if (op->contr->transport)
        ob_apply(op->contr->transport, op, AP_UNAPPLY);

    if (!exit_ob) {
        enter_maplevel_exit(op);
        return;
    }

    assert(EXIT_PATH(exit_ob) != NULL);

    /* check to see if we make a template map */
    if (EXIT_PATH(exit_ob) && EXIT_PATH(exit_ob)[1] == '@') {
        if (EXIT_PATH(exit_ob)[2] == '!') {
            /* generate a template map randomly */
            enter_random_template_map(op, exit_ob);
        } else {
            /* generate a template map from a fixed template */
            enter_fixed_template_map(op, exit_ob);
        }
    }
    /* check to see if we make a randomly generated map */
    else if (EXIT_PATH(exit_ob) && EXIT_PATH(exit_ob)[1] == '!') {
        enter_random_map(op, exit_ob);
    } else if (QUERY_FLAG(exit_ob, FLAG_UNIQUE)) {
        enter_unique_map(op, exit_ob);
    } else {
        int x = EXIT_X(exit_ob), y = EXIT_Y(exit_ob);
        /* 'Normal' exits that do not do anything special
        * Simple enough we don't need another routine for it.
        */
        mapstruct *newmap;
        if (exit_ob->map) {
            char tmp_path[HUGE_BUF];

            path_combine_and_normalize(exit_ob->map->path, EXIT_PATH(exit_ob), tmp_path, sizeof(tmp_path));
            newmap = ready_map_name(tmp_path, 0);
            /* Random map was previously generated, but is no longer about.  Lets generate a new
             * map.
             */
            if (!newmap && !strncmp(EXIT_PATH(exit_ob), "/random/", 8)) {
                /* Maps that go down have a message set.  However, maps that go
                 * up, don't.  If the going home has reset, there isn't much
                 * point generating a random map, because it won't match the maps.
                 */
                if (exit_ob->msg) {
                    enter_random_map(op, exit_ob);
                } else {
                    draw_ext_info_format(NDI_UNIQUE, 0, op,
                                         MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_FAILURE,
                                         "The %s is closed.",
                                         exit_ob->name);
                    return;
                }

                /* For exits that cause damages (like pits).  Don't know if any
                 * random maps use this or not.
                 */
                if (exit_ob->stats.dam && op->type == PLAYER)
                    hit_player(op, exit_ob->stats.dam, exit_ob, exit_ob->attacktype, 1);
                return;
            }
        } else {
            /* For word of recall and other force objects
             * They contain the full pathname of the map to go back to,
             * so we don't need to normalize it.
             * But we do need to see if it is unique or not
             */
            if (!strncmp(EXIT_PATH(exit_ob), settings.localdir, strlen(settings.localdir)))
                newmap = ready_map_name(EXIT_PATH(exit_ob), MAP_PLAYER_UNIQUE);
            else
                newmap = ready_map_name(EXIT_PATH(exit_ob), 0);
        }
        if (!newmap) {
            if (exit_ob->name)
                draw_ext_info_format(NDI_UNIQUE, 0, op,
                                     MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_FAILURE,
                                     "The %s is closed.",
                                     exit_ob->name);
            /* don't cry to momma if name is not set - as in tmp objects
             * used by the savebed code and character creation */
            return;
        }

        if (x == -1 && y == -1) {
            x = MAP_ENTER_X(newmap);
            y = MAP_ENTER_Y(newmap);
        }

        /* mids 02/13/2002 if exit is damned, update players death & WoR home-position and delete town portal */
        if (QUERY_FLAG(exit_ob, FLAG_DAMNED)) {
            object *tmp;

            /* remove an old force with a slaying field == PORTAL_DESTINATION_NAME */
            tmp = object_find_by_type_and_slaying(op, FORCE, PORTAL_DESTINATION_NAME);
            if (tmp) {
                object_remove(tmp);
                object_free2(tmp, FREE_OBJ_NO_DESTROY_CALLBACK);
            }

            path_combine_and_normalize(exit_ob->map->path, EXIT_PATH(exit_ob), op->contr->savebed_map, sizeof(op->contr->savebed_map));
            op->contr->bed_x = EXIT_X(exit_ob), op->contr->bed_y = EXIT_Y(exit_ob);
            save_player(op, 1);
            /* LOG(llevDebug, "enter_exit: Taking damned exit %s to (%d,%d) on map %s\n",
             *     exit_ob->name ? exit_ob->name : "(none)", exit_ob->x, exit_ob->y,
             *     path_combine_and_normalize(exit_ob->map->path, EXIT_PATH(exit_ob))); */
        }

        enter_map(op, newmap, x, y);
    }
    /* For exits that cause damages (like pits) */
    if (exit_ob->stats.dam && op->type == PLAYER)
        hit_player(op, exit_ob->stats.dam, exit_ob, exit_ob->attacktype, 1);
}

/**
 * Do all player-related stuff before objects have been updated.
 *
 * @sa process_players2().
 */
static void process_players1(void) {
    int flag;
    player *pl, *plnext;

    /* Basically, we keep looping until all the players have done their actions. */
    for (flag = 1; flag != 0; ) {
        flag = 0;
        for (pl = first_player; pl != NULL; pl = plnext) {
            plnext = pl->next; /* In case a player exits the game in handle_player() */

            if (pl->ob == NULL)
                continue;

            /* Only do this on the first pass - what we are recording
             * here is the number of ticks the player has been online - not
             * how many actions/moves the player has done.
             */
            if (!flag) pl->ticks_played++;

            /** Handle DM follow command */
            if (pl->followed_player) {
                player *followed = find_player_partial_name(pl->followed_player);
                if (followed && followed->ob && followed->ob->map) {
                    rv_vector rv;

                    if (!get_rangevector(pl->ob, followed->ob, &rv, 0) || rv.distance > 4) {
                        int space = object_find_free_spot(pl->ob, followed->ob->map, followed->ob->x, followed->ob->y, 1, 25);
                        if (space == -1)
                            /** This is a DM, just teleport on the top of player. */
                            space = 0;
                        object_remove(pl->ob);
                        object_insert_in_map_at(pl->ob, followed->ob->map, NULL, 0, followed->ob->x+freearr_x[space], followed->ob->y+freearr_y[space]);
                        map_newmap_cmd(&pl->socket);
                    }
                } else {
                    draw_ext_info_format(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_DM, "Player %s left or ambiguous name.", pl->followed_player);
                    FREE_AND_CLEAR_STR(pl->followed_player);
                }
            } /** End of follow */

            if (pl->ob->speed_left > 0) {
                if (handle_newcs_player(pl->ob))
                    flag = 1;
            } /* end if player has speed left */

            /* If the player is not actively playing, don't make a
             * backup save - nothing to save anyway.  Plus, the
             * map may not longer be valid.  This can happen when the
             * player quits - they exist for purposes of tracking on the map,
             * but don't actually reside on any actual map.
             */
            if (QUERY_FLAG(pl->ob, FLAG_REMOVED))
                continue;

#ifdef AUTOSAVE
            /* check for ST_PLAYING state so that we don't try to save off when
             * the player is logging in.
             */
            if ((pl->last_save_tick+AUTOSAVE) < pticks && pl->state == ST_PLAYING) {
                /* Don't save the player on unholy ground.  Instead, increase the
                 * tick time so it will be about 10 seconds before we try and save
                 * again.
                 */
                if (get_map_flags(pl->ob->map, NULL, pl->ob->x, pl->ob->y, NULL, NULL)&P_NO_CLERIC) {
                    pl->last_save_tick += 100;
                } else {
                    save_player(pl->ob, 1);
                    pl->last_save_tick = pticks;
                    hiscore_check(pl->ob, 1);
                }
            }
#endif
        } /* end of for loop for all the players */
    } /* for flag */
    for (pl = first_player; pl != NULL; pl = pl->next) {
        int has_action = 1;

        pl->ob->weapon_speed_left += pl->ob->weapon_speed;
        if (pl->ob->weapon_speed_left > 1.0)
            pl->ob->weapon_speed_left = 1.0;

        pl->socket.sounds_this_tick = 0;

        if (settings.casting_time == TRUE) {
            if (pl->ob->casting_time > 0) {
                pl->ob->casting_time--;
                has_action = 0;
            }
        }
        /* If the character is idle (standing around resting) increase
         * regen rates.
         */
        if (has_action && pl->ob->speed_left > 0) {
            pl->ob->last_heal -= 2;
            pl->ob->last_sp -= 2;
            pl->ob->last_grace -= 2;
            pl->ob->last_eat += 2;        /* Slow down food consumption */
        }
        do_some_living(pl->ob);
    }
}

/**
 * Do all player-related stuff after objects have been updated.
 *
 * @sa process_players1().
 *
 * @todo explain why 2 passes for players.
 */
static void process_players2(void) {
    player *pl;

    /* Then check if any players should use weapon-speed instead of speed */
    for (pl = first_player; pl != NULL; pl = pl->next) {
        /* The code that did weapon_sp handling here was out of place -
         * this isn't called until after the player has finished there
         * actions, and is thus out of place.  All we do here is bounds
         * checking.
         */
        if (pl->has_hit) {
            /* This needs to be here - if the player is running, we need to
             * clear this each tick, but new commands are not being received
             * so execute_newserver_command() is never called
             */
            pl->has_hit = 0;
        } else if (pl->ob->speed_left > pl->ob->speed)
            pl->ob->speed_left = pl->ob->speed;
    }
}

#define SPEED_DEBUG

/**
 * Process all active objects.
 */
void process_events(void) {
    object *op;
    object marker;
    tag_t tag;

    process_players1();

    memset(&marker, 0, sizeof(object));
    /* Put marker object at beginning of active list */
    marker.active_next = active_objects;

    if (marker.active_next)
        marker.active_next->active_prev = &marker;
    marker.active_prev = NULL;
    active_objects = &marker;

    while (marker.active_next) {
        op = marker.active_next;
        tag = op->count;

        /* Move marker forward - swap op and marker */
        op->active_prev = marker.active_prev;

        if (op->active_prev)
            op->active_prev->active_next = op;
        else
            active_objects = op;

        marker.active_next = op->active_next;

        if (marker.active_next)
            marker.active_next->active_prev = &marker;
        marker.active_prev = op;
        op->active_next = &marker;

        /* Now process op */
        if (QUERY_FLAG(op, FLAG_FREED)) {
            LOG(llevError, "BUG: process_events(): Free object on list\n");
            op->speed = 0;
            object_update_speed(op);
            continue;
        }

        /* I've seen occasional crashes due to this - the object is removed,
         * and thus the map it points to (last map it was on) may be bogus
         * The real bug is to try to find out the cause of this - someone
         * is probably calling object_remove() without either an insert_ob or
         * object_free_drop_inventory() afterwards, leaving an object dangling.
	 * But I'd rather log this and continue on instead of crashing.
         * Don't remove players - when a player quits, the object is in
         * sort of a limbo, of removed, but something we want to keep
         * around.
         */
        if (QUERY_FLAG(op, FLAG_REMOVED)
        && op->type != PLAYER
        && op->map
        && op->map->in_memory != MAP_IN_MEMORY) {
            StringBuffer *sb;
            char *diff;

            LOG(llevError, "BUG: process_events(): Removed object on list\n");
            sb = stringbuffer_new();
            object_dump(op, sb);
            diff = stringbuffer_finish(sb);
            LOG(llevError, "%s\n", diff);
            free(diff);
            object_free2(op, FREE_OBJ_NO_DESTROY_CALLBACK);
            continue;
        }

        if (!op->speed) {
            LOG(llevError, "BUG: process_events(): Object %s has no speed, but is on active list\n", op->arch->name);
            object_update_speed(op);
            continue;
        }

        if (op->map == NULL
        && op->env == NULL
        && op->name
        && op->type != MAP) {
            LOG(llevError, "BUG: process_events(): Object without map or inventory is on active list: %s (%d)\n", op->name, op->count);
            op->speed = 0;
            object_update_speed(op);
            continue;
        }

        /* Seen some cases where process_object() is crashing because
         * the object is on a swapped out map.  But can't be sure if
         * something in the chain of events caused the object to
         * change maps or was just never removed - this will
         * give some clue as to its state before call to
         * process_object
         */
        if (op->map && op->map->in_memory != MAP_IN_MEMORY) {
            LOG(llevError, "BUG: process_events(): Processing object on swapped out map: %s (%d), map=%s\n", op->name, op->count, op->map->path);
        }

        /* Animate the object.  Bug of feature that andim_speed
         * is based on ticks, and not the creatures speed?
         */
        if ((op->anim_speed && op->last_anim >= op->anim_speed)
        || (op->temp_animation_id && op->last_anim >= op->temp_anim_speed)) {
            op->state++;
            if ((op->type == PLAYER) || (op->type == MONSTER))
                animate_object(op, op->facing);
            else
                animate_object(op, op->direction);
            op->last_anim = 1;
        } else {
            op->last_anim++;
        }

        if (op->speed_left > 0) {
            --op->speed_left;
            process_object(op);
            if (object_was_destroyed(op, tag))
                continue;
        }
        if (settings.casting_time == TRUE && op->casting_time > 0)
            op->casting_time--;
        if (op->speed_left <= 0)
            op->speed_left += FABS(op->speed);
    }

    /* Remove marker object from active list */
    if (marker.active_prev != NULL)
        marker.active_prev->active_next = NULL;
    else
        active_objects = NULL;

    process_players2();
}

/**
 * Remove temporary map files.
 *
 * @todo check logic, why is file only removed if map is in memory?
 */
void clean_tmp_files(void) {
    mapstruct *m, *next;

    LOG(llevInfo, "Cleaning up...\n");

    /* We save the maps - it may not be intuitive why, but if there are unique
     * items, we need to save the map so they get saved off.  Perhaps we should
     * just make a special function that only saves the unique items.
     */
    for (m = first_map; m != NULL; m = next) {
        next = m->next;
        if (m->in_memory == MAP_IN_MEMORY) {
            /* If we want to reuse the temp maps, swap it out (note that will also
             * update the log file.  Otherwise, save the map (mostly for unique item
             * stuff).  Note that the clean_tmp_map is called after the end of
             * the for loop but is in the #else bracket.  IF we are recycling the maps,
             * we certainly don't want the temp maps removed.
             */

            /* XXX The above comment is dead wrong */
            if (settings.recycle_tmp_maps == TRUE)
                swap_map(m);
            else {
                save_map(m, SAVE_MODE_NORMAL); /* note we save here into a overlay map */
                clean_tmp_map(m);
            }
        }
    }
    write_todclock(); /* lets just write the clock here */
}

/** Clean up everything and exit. */
void cleanup(void) {
    LOG(llevDebug, "Cleanup called.  freeing data.\n");
    clean_tmp_files();
    write_book_archive();
    accounts_save();

#ifdef MEMORY_DEBUG
    free_all_maps();
    free_style_maps();
#endif
    cleanupPlugins();
#ifdef MEMORY_DEBUG
    free_all_archs();
    free_all_treasures();
    free_all_artifacts();
    free_all_images();
    free_all_newserver();
    free_all_recipes();
    free_all_readable();
    free_all_god();
    free_all_anim();
    i18n_free();
    free_loader();
    free_globals();
    free_server();
    object_free_all_data();
    free_knowledge();
    free_quest();
    free_quest_definitions();
    /* See what the string data that is out there that hasn't been freed. */
    /*    LOG(llevDebug, "%s", ss_dump_table(0xff));*/
#endif
    exit(0);
}

/**
 * Player logs out, or was disconnected.
 *
 * @param pl
 * player.
 * @param draw_exit
 * if set, display leaving message to other players.
 */
void leave(player *pl, int draw_exit) {
    if (!QUERY_FLAG(pl->ob, FLAG_REMOVED)) {
        pets_terminate_all(pl->ob);
        object_remove(pl->ob);
    }

    pl->socket.status = Ns_Dead;
    LOG(llevInfo, "LOGOUT: Player named %s from ip %s\n", pl->ob->name, pl->socket.host);

    strcpy(pl->ob->contr->killer, "left");
    hiscore_check(pl->ob, 1);

    /* If this player is the captain of the transport, need to do
     * some extra work.  By the time we get here, object_remove()
     * should have already been called.
     */
    if (pl->transport && pl->transport->contr == pl) {
        /* If inv is a non player, inv->contr will be NULL, but that
         * is OK.
         */
        if (pl->transport->inv)
            pl->transport->contr = pl->transport->inv->contr;
        else
            pl->transport->contr = NULL;

        if (pl->transport->contr) {
            char name[MAX_BUF];

            query_name(pl->transport, name, MAX_BUF);
            draw_ext_info_format(NDI_UNIQUE, 0, pl->transport->contr->ob,
                                 MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_PLAYER,
                                 "%s has left.  You are now the captain of %s",
                                 pl->ob->name, name);
        }
    }

    if (pl->ob->map) {
        if (pl->ob->map->in_memory == MAP_IN_MEMORY)
            pl->ob->map->timeout = MAP_TIMEOUT(pl->ob->map);
        pl->ob->map = NULL;
    }
    pl->ob->type = DEAD_OBJECT; /* To avoid problems with inventory window */
    party_leave(pl->ob);
    /* If a hidden dm dropped connection do not create
    * inconsistencies by showing that they have left the game
    */
    if (!(QUERY_FLAG(pl->ob, FLAG_WIZ) && pl->ob->contr->hidden)
    && (pl != NULL && draw_exit) && (pl->state != ST_GET_NAME && pl->state != ST_GET_PASSWORD && pl->state != ST_CONFIRM_PASSWORD))
        draw_ext_info_format(NDI_UNIQUE|NDI_ALL|NDI_DK_ORANGE, 5, NULL,
                             MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_PLAYER,
                             "%s left the game.",
                             pl->ob->name);
}

/**
 * Checks if server should be started.
 *
 * @return
 * 1 if play is forbidden, 0 if ok.
 * @todo document forbidden stuff.
 */
int forbid_play(void) {
#if !defined(_IBMR2) && !defined(___IBMR2) && defined(PERM_FILE)
    char buf[MAX_BUF], day[MAX_BUF];
    FILE *fp;
    time_t clock;
    struct tm *tm;
    int i, start, stop, forbit = 0;

    clock = time(NULL);
    tm = (struct tm *)localtime(&clock);

    snprintf(buf, sizeof(buf), "%s/%s", settings.confdir, PERM_FILE);
    if ((fp = fopen(buf, "r")) == NULL)
        return 0;

    while (fgets(buf, sizeof(buf), fp)) {
        if (buf[0] == '#')
            continue;
        if (!strncmp(buf, "msg", 3)) {
            if (forbit)
                while (fgets(buf, sizeof(buf), fp))   /* print message */
                    fputs(buf, logfile);
            break;
        } else if (sscanf(buf, "%s %d%*c%d\n", day, &start, &stop) != 3) {
            LOG(llevDebug, "Warning: Incomplete line in permission file ignored.\n");
            continue;
        }

        for (i = 0; i < 7; i++) {
            if (!strncmp(buf, days[i], 3)
            && (tm->tm_wday == i)
            && (tm->tm_hour >= start)
            && (tm->tm_hour < stop))
                forbit = 1;
        }
    }

    fclose(fp);

    return forbit;
#else
    return 0;
#endif
}

extern unsigned long todtick;

/**
 * Collection of functions to call from time to time.
 *
 * Modified 2000-1-14 MSW to use the global pticks count to determine how
 * often to do things.  This will allow us to spred them out more often.
 * I use prime numbers for the factor count - in that way, it is less likely
 * these actions will fall on the same tick (compared to say using 500/2500/15000
 * which would mean on that 15,000 tick count a whole bunch of stuff gets
 * done).  Of course, there can still be times where multiple specials are
 * done on the same tick, but that will happen very infrequently
 *
 * I also think this code makes it easier to see how often we really are
 * doing the various things.
 */
static void do_specials(void) {

    if (!(pticks%10))
        knowledge_process_incremental();

#ifdef WATCHDOG
    if (!(pticks%503))
        watchdog();
#endif

    if (!(pticks%PTICKS_PER_CLOCK))
        tick_the_clock();

    if (!(pticks%509))
        flush_old_maps();    /* Clears the tmp-files of maps which have reset */

    if (!(pticks%2503))
        fix_weight();        /* Hack to fix weightproblems caused by bugs */

    if (!(pticks%2521))
        metaserver_update();    /* 2500 ticks is about 5 minutes */

    if (!(pticks%2531))
        accounts_save();

    if (!(pticks%5003))
        write_book_archive();

    if (!(pticks%5009))
        clean_friendly_list();

    if (!(pticks%5011))
        party_obsolete_parties();

    if (!(pticks%12503))
        fix_luck();
}

/**
 * Server main function.
 *
 * @param argc
 * length of argv.
 * @param argv
 * command-line options.
 * @return
 * 0.
 */
int server_main(int argc, char **argv) {
#ifdef WIN32 /* ---win32 this sets the win32 from 0d0a to 0a handling */
    _fmode = _O_BINARY;
    bRunning = 1;
#endif

#ifndef WIN32
    /* Here we check that we aren't root or suid */
    if (getuid() == 0 || geteuid() == 0) {
        fprintf(stderr,
                "Running crossfire-server as root is a bad idea; aborting!\n"
                "Please run it again as a normal, unprivileged user.\n"
                );
        return 1;
    }
#endif

#ifdef DEBUG_MALLOC_LEVEL
    malloc_debug(DEBUG_MALLOC_LEVEL);
#endif

    init(argc, argv);
    initPlugins();        /* GROS - Init the Plugins */
#ifdef WIN32
    while (bRunning) {
#else
    for (;;) {
#endif
        nroferrors = 0;

        do_server();
        process_events();         /* "do" something with objects with speed */
        cftimer_process_timers(); /* Process the crossfire Timers */
        /* Lauwenmark : Here we handle the CLOCK global event */
        execute_global_event(EVENT_CLOCK);
        check_active_maps(); /* Removes unused maps after a certain timeout */
        do_specials();       /* Routines called from time to time. */

        sleep_delta();       /* Sleep proper amount of time before next tick */
    }
#if 0 /* unreachable */
    emergency_save(0);
    cleanup();
    return 0;
#endif
}
