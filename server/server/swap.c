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
 * All those functions deal with swapping maps to disk to claim memory space.
 */

#include <global.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#endif
#include <object.h>

/**
 * Writes out information on all the temporary maps.  It is called by
 * swap_map().
 *
 * Will LOG() in case of error.
 */
static void write_map_log(void) {
    FILE *fp;
    mapstruct *map;
    char buf[MAX_BUF];
    long current_time = time(NULL);

    snprintf(buf, sizeof(buf), "%s/temp.maps", settings.localdir);
    if (!(fp = fopen(buf, "w"))) {
        LOG(llevError, "Could not open %s for writing\n", buf);
        return;
    }
    for (map = first_map; map != NULL; map = map->next) {
        /* If tmpname is null, it is probably a unique player map,
         * so don't save information on it.
         */
        if (map->in_memory != MAP_IN_MEMORY
        && (map->tmpname != NULL)
        && (strncmp(map->path, "/random", 7))) {
            /* the 0 written out is a leftover from the lock number for
             * unique items and second one is from encounter maps.
             * Keep using it so that old temp files continue
             * to work.
             */
            fprintf(fp, "%s:%s:%ld:0:0:%d:0:%d\n", map->path, map->tmpname,
                    (map->reset_time == -1 ? -1 : map->reset_time-current_time),
                    map->difficulty,
                    map->darkness);
        }
    }
    fclose(fp);
}

/**
 * Reads temporary maps information from disk.
 *
 * Will LOG() in case of error.
 */
void read_map_log(void) {
    FILE *fp;
    mapstruct *map;
    char buf[MAX_BUF];
    int do_los, darkness, lock;
    long sec = seconds();

    snprintf(buf, sizeof(buf), "%s/temp.maps", settings.localdir);
    if (!(fp = fopen(buf, "r"))) {
        LOG(llevDebug, "Could not open %s for reading\n", buf);
        return;
    }
    while (fgets(buf, MAX_BUF, fp) != NULL) {
        char *tmp[3];

        map = get_linked_map();
        /* scanf doesn't work all that great on strings, so we break
         * out that manually.  strdup is used for tmpname, since other
         * routines will try to free that pointer.
         */
        if (split_string(buf, tmp, sizeof(tmp)/sizeof(*tmp), ':') != 3) {
            LOG(llevDebug, "%s/temp.maps: ignoring invalid line: %s\n", settings.localdir, buf);
            continue;
        }

        strcpy(map->path, tmp[0]);
        map->tmpname = strdup_local(tmp[1]);

        /* Lock is left over from the lock items - we just toss it now.
         * We use it twice - second one is from encounter, but as we
         * don't care about the value, this works fine
         */
        sscanf(tmp[2], "%u:%d:%d:%hu:%d:%d\n", &map->reset_time, &lock, &lock, &map->difficulty, &do_los, &darkness);

        map->in_memory = MAP_SWAPPED;
        map->darkness = darkness;
        map->timeout = 0;

        /* When the reset time is saved out, it is adjusted so that
         * the current time is subtracted (thus, it is saved as number
         * of seconds from current time that it should reset).  We need
         * to add in the current seconds for this to work right.
         * On metalforge, strange behavior was observed with really high
         * reset times - I don't know how they got to that state,
         * but easy enough to do some sanity checking here.
         */
        map->reset_time += sec;
        if (map->reset_time > (sec+MAP_MAXRESET))
            map->reset_time = 0;
    }
    fclose(fp);
}

/**
 * Swaps a map to disk.
 *
 * After calling this function, the map will be:
 * - swapped to disk
 * - kept in memory in case of save error
 *
 * @param map
 * map to swap.
 * @return
 * one of @ref SAVE_ERROR_xxx "SAVE_ERROR_xxx".
 */
int swap_map(mapstruct *map) {
    player *pl;
    int res;

    if (map->in_memory != MAP_IN_MEMORY) {
        LOG(llevError, "Tried to swap out map which was not in memory.\n");
        return SAVE_ERROR_NOT_IN_MEMORY;
    }
    for (pl = first_player; pl != NULL; pl = pl->next)
        if (pl->ob == NULL || (!(QUERY_FLAG(pl->ob, FLAG_REMOVED)) && pl->ob->map == map))
            break;

    if (pl != NULL) {
        LOG(llevDebug, "Wanted to swap out map with player.\n");
        return SAVE_ERROR_PLAYER;
    }
    pets_remove_all(); /* Give them a chance to follow */

    /* Update the reset time.  Only do this is STAND_STILL is not set */
    if (!map->fixed_resettime)
        set_map_reset_time(map);

    /* If it is immediate reset time, don't bother saving it - just get
     * rid of it right away.
     */
    if (map->reset_time <= seconds()) {
        mapstruct *oldmap = map;

        LOG(llevDebug, "Resetting map %s.\n", map->path);
        /* Lauwenmark : Here we handle the MAPRESET global event */
        execute_global_event(EVENT_MAPRESET, map);
        map = map->next;
        delete_map(oldmap);
        return SAVE_ERROR_OK;
    }

    if ((res = save_map(map, SAVE_MODE_NORMAL)) < 0) {
        LOG(llevError, "Failed to swap map %s.\n", map->path);
        /* This is sufficiently critical to mandate to warn all DMs. */
        draw_ext_info_format(NDI_ALL_DMS|NDI_UNIQUE|NDI_RED, -1, NULL, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_LOADSAVE,
                             "Failed to swap map %s!", map->path);
        /* Map is *not *swapped. */
        map->in_memory = MAP_IN_MEMORY;
        return res;
    }

    free_map(map);

    if (settings.recycle_tmp_maps == TRUE)
        write_map_log();

    return SAVE_ERROR_OK;
}

/**
 * Finds maps in memory to reset.
 *
 * @todo
 * The check for MAX_OBJECTS_LWM is wrongly placed, and should be moved elsewhere.
 */
void check_active_maps(void) {
    mapstruct *map, *next;

    for (map = first_map; map != NULL; map = next) {
        next = map->next;
        if (map->in_memory != MAP_IN_MEMORY)
            continue;
        if (!map->timeout)
            continue;
        if (--(map->timeout) > 0)
            continue;
        /* If LWM is set, we only swap maps out when we run out of objects */
#ifndef MAX_OBJECTS_LWM
        swap_map(map);
#endif
    }
}

/**
 * Returns the map with the lowest timeout variable (not 0).
 *
 * @param except_level
 * path of map to ignore for reset. Musn't be NULL.
 * @return
 * map, or NULL if no map is ready for reset.
 */
static mapstruct *map_least_timeout(const char *except_level) {
    mapstruct *map, *chosen = NULL;
    int timeout = MAP_MAXTIMEOUT+1;

    for (map = first_map; map != NULL; map = map->next)
        if (map->in_memory == MAP_IN_MEMORY
        && strcmp(map->path, except_level)
        && map->timeout
        && map->timeout < timeout)
            chosen = map, timeout = map->timeout;
    return chosen;
}

/**
 * Tries to swap out maps which are still in memory, because
 * of MAP_TIMEOUT until used objects is below MAX_OBJECTS or there are
 * no more maps to swap.
 *
 * @param except_level
 * path of map to ignore for reset. Musn't be NULL.
 */
void swap_below_max(const char *except_level) {
    mapstruct *map;

    if (nrofallocobjects-nroffreeobjects < MAX_OBJECTS)
        return;
    for (;;) {
#ifdef MAX_OBJECTS_LWM
        if (nrofallocobjects-nroffreeobjects < MAX_OBJECTS_LWM)
            return;
#else
        if (nrofallocobjects-nroffreeobjects < MAX_OBJECTS)
            return;
#endif
        if ((map = map_least_timeout(except_level)) == NULL)
            return;
        LOG(llevDebug, "Trying to swap out %s before its time.\n", map->path);
        map->timeout = 0;
        swap_map(map);
    }
}

/**
 * Returns the count of players on a map, calculated from player list.
 *
 * @param m
 * map we want the count of players on.
 * @param show_all
 * if true, show everyone. If not, don't show hidden players (dms)
 * @return
 * player count.
 *
 * @todo
 * this doesn't take into account transports. Should be removed when mapstruct::players is valid.
 */
int players_on_map(mapstruct *m, int show_all) {
    player *pl;
    int nr = 0;

    for (pl = first_player; pl != NULL; pl = pl->next)
        if (pl->ob != NULL
        && !QUERY_FLAG(pl->ob, FLAG_REMOVED)
        && pl->ob->map == m
        && (show_all || !pl->hidden))
            nr++;
    return nr;
}

/**
 * Removes tmp-files of maps which are going to be reset next time
 * they are visited.
 * This is very useful if the tmp-disk is very full.
 */
void flush_old_maps(void) {
    mapstruct *m, *oldmap;
    long sec;
    sec = seconds();

    m = first_map;
    while (m) {
        /* There can be cases (ie death) where a player leaves a map and the timeout
         * is not set so it isn't swapped out.
         */
        if ((m->in_memory == MAP_IN_MEMORY)
        && (m->timeout == 0)
        && !players_on_map(m, TRUE)) {
            set_map_timeout(m);
        }

        /* per player unique maps are never really reset.  However, we do want
         * to perdiocially remove the entries in the list of active maps - this
         * generates a cleaner listing if a player issues the map commands, and
         * keeping all those swapped out per player unique maps also has some
         * memory and cpu consumption.
         * We do the cleanup here because there are lots of places that call
         * swap map, and doing it within swap map may cause problems as
         * the functions calling it may not expect the map list to change
         * underneath them.
         */
        if ((m->unique || m->is_template) && m->in_memory == MAP_SWAPPED) {
            LOG(llevDebug, "Resetting map %s.\n", m->path);
            oldmap = m;
            m = m->next;
            delete_map(oldmap);
        } else if (m->in_memory != MAP_SWAPPED
        || m->tmpname == NULL
        || sec < m->reset_time) {
            m = m->next;
        } else {
            LOG(llevDebug, "Resetting map %s.\n", m->path);
            /* Lauwenmark : Here we handle the MAPRESET global event */
            execute_global_event(EVENT_MAPRESET, m);
            clean_tmp_map(m);
            oldmap = m;
            m = m->next;
            delete_map(oldmap);
        }
    }
}
