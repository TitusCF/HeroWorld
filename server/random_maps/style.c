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
 * @file
 * Those functions deal with style for random maps.
 * @todo
 * couldn't load_dir() be merged with a function in common library?
 */

#include <global.h>
#include <random_map.h>
#ifndef WIN32 /* ---win32 exclude headers */
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../include/autoconf.h"
#endif /* win32 */

/**
 * Char comparison for sorting purposes.
 * @param p1
 * @param p2
 * pointers to compare.
 * @return
 * return of strcmp() on pointed strings.
 */
static int pointer_strcmp(const void *p1, const void *p2)
{
    const char *s1 = *(const char * const *)p1;
    const char *s2 = *(const char * const *)p2;

    return(strcmp(s1, s2));
}

/**
 * This is our own version of scandir/select_regular_files/sort.
 * To support having subdirectories in styles, we need to know
 * if in fact the directory we read is a subdirectory.  However,
 * we can't get that through the normal dirent entry.  So
 * instead, we do our own where we do have the full directory
 * path so can do stat calls to see if in fact it is a directory.
 * dir is the name of the director to scan.
 * namelist is the array of file names returned.  IT needs to be
 * freed by the caller.
 * skip_dirs controls our behavioru - if nonzero, we don't
 * skip any subdirectories - if zero, we store those away,
 * since there are cases where we want to choose a random
 * directory.
 * @param dir
 * directory to list.
 * @param[out] namelist
 * list of files found.
 * @param skip_dirs
 * if non 0, don't list directories.
 * @return
 * -1 if dir is invalid, number of files else.
 */
int load_dir(const char *dir, char ***namelist, int skip_dirs)
{
    DIR *dp;
    struct dirent *d;
    int entries = 0, entry_size = 0;
    char name[NAME_MAX+1], **rn = NULL;
    struct stat sb;

    dp = opendir(dir);
    if (dp == NULL) {
        return -1;
    }

    while ((d = readdir(dp)) != NULL) {
        if (skip_dirs) {
            snprintf(name, sizeof(name), "%s/%s", dir, d->d_name);
            stat(name, &sb);
            if (S_ISDIR(sb.st_mode)) {
                continue;
            }
        }

        if (entries == entry_size) {
            entry_size += 10;
            rn = realloc(rn, sizeof(char *)*entry_size);
        }
        rn[entries] = strdup_local(d->d_name);
        entries++;
    }
    (void)closedir(dp);

    /* We don't return -1 for this case here as directory isn't invalid as such.
     * Most likely directory was empty. In theory it could also be due to
     * realloc failing to allocate.
     */
    if (rn == NULL) {
        return 0;
    }

    qsort(rn, entries, sizeof(char *), pointer_strcmp);

    *namelist = rn;
    return entries;
}

/**
 * Loaded styles maps cache, to avoid having to load all the time.
 */
mapstruct *styles = NULL;

/**
 * Loads specified map (or take it from cache list).
 * @param style_name
 * map to load.
 * @return
 * map.
 */
mapstruct *load_style_map(char *style_name)
{
    mapstruct *style_map;

    /* Given a file.  See if its in memory */
    for (style_map = styles; style_map != NULL; style_map = style_map->next) {
        if (!strcmp(style_name, style_map->path)) {
            return style_map;
        }
    }
    style_map = load_original_map(style_name, MAP_STYLE);
    /* Remove it from global list, put it on our local list */
    if (style_map) {
        mapstruct *tmp;

        if (style_map == first_map) {
            first_map = style_map->next;
        } else {
            for (tmp = first_map; tmp && tmp->next != style_map; tmp = tmp->next)
                ;
            if (tmp) {
                tmp->next = style_map->next;
            }
        }
        style_map->next = styles;
        styles = style_map;
    }
    return style_map;
}

/**
 * Loads and returns the map requested.
 * Dirname, for example, is "/styles/wallstyles", stylename, is,
 * for example, "castle", difficulty is -1 when difficulty is
 * irrelevant to the style.  If dirname is given, but stylename
 * isn't, and difficult is -1, it returns a random style map.
 * Otherwise, it tries to match the difficulty given with a style
 * file, named style_name_# where # is an integer
 * @param dirname
 * where to look.
 * @param stylename
 * style to use, can be NULL.
 * @param difficulty
 * style difficulty.
 * @return
 * style, or NULL if none suitable.
 * @todo
 * better document.
 */
mapstruct *find_style(const char *dirname, const char *stylename, int difficulty)
{
    char style_file_path[256];
    char style_file_full_path[256];
    mapstruct *style_map = NULL;
    struct stat file_stat;
    int i, only_subdirs = 0;

    /* if stylename exists, set style_file_path to that file.*/
    if (stylename && strlen(stylename) > 0) {
        snprintf(style_file_path, sizeof(style_file_path), "%s/%s", dirname, stylename);
    } else { /* otherwise, just use the dirname.  We'll pick a random stylefile.*/
        snprintf(style_file_path, sizeof(style_file_path), "%s", dirname);
    }

    /* is what we were given a directory, or a file? */
    snprintf(style_file_full_path, sizeof(style_file_full_path), "%s/maps%s", settings.datadir, style_file_path);
    if (stat(style_file_full_path, &file_stat) == 0
            && !S_ISDIR(file_stat.st_mode)) {
        style_map = load_style_map(style_file_path);
    }
    if (style_map == NULL) { /* maybe we were given a directory! */
        char **namelist;
        int n;
        char style_dir_full_path[256];

        /* get the names of all the files in that directory */
        snprintf(style_dir_full_path, sizeof(style_dir_full_path), "%s/maps%s", settings.datadir, style_file_path);

        /* First, skip subdirectories.  If we don't find anything, then try again
         * without skipping subdirs.
         */
        n = load_dir(style_dir_full_path, &namelist, 1);
        if (n <= 0) {
            n = load_dir(style_dir_full_path, &namelist, 0);
            only_subdirs = 1;
        }

        if (n <= 0) {
            return NULL;    /* nothing to load.  Bye. */
        }

        /* Picks a random map.  Note that if this is all directories,
         * we know it won't be able to load, so save a few ticks.
         * the door handling checks for this failure and handles
         * it properly.
         */
        if (difficulty == -1) { /* pick a random style from this dir. */
            if (only_subdirs) {
                style_map = NULL;
            } else {
                char *p;

                p = strchr(style_file_path, '\0');
                snprintf(p, style_file_path+sizeof(style_file_path)-p, "/%s", namelist[RANDOM()%n]);
                style_map = load_style_map(style_file_path);
            }
        } else { /* find the map closest in difficulty */
            int min_dist = 32000, min_index = -1;
            char *p;

            for (i = 0; i < n; i++) {
                int dist;
                char *mfile_name = strrchr(namelist[i], '_')+1;

                if ((mfile_name-1) == NULL) { /* since there isn't a sequence, */
                    int q;

                    /*pick one at random to recurse */
                    style_map = find_style(style_file_path, namelist[RANDOM()%n], difficulty);
                    for (q = 0; q < n; q++) {
                        free(namelist[q]);
                    }
                    free(namelist);
                    return style_map;
                } else {
                    dist = abs(difficulty-atoi(mfile_name));
                    if (dist < min_dist) {
                        min_dist = dist;
                        min_index = i;
                    }
                }
            }
            /* presumably now we've found the "best" match for the
               difficulty. */
            p = strchr(style_file_path, '\0');
            snprintf(p, style_file_path+sizeof(style_file_path)-p, "/%s", namelist[min_index]);
            style_map = load_style_map(style_file_path);
        }
        for (i = 0; i < n; i++) {
            free(namelist[i]);
        }
        free(namelist);
    }
    return style_map;
}

/**
 * Picks a random object from a style map.
 * Redone by MSW so it should be faster and not use static
 * variables to generate tables.
 * @param style
 * map to pick from.
 * @return
 * random object. Can be NULL.
 */
object *pick_random_object(mapstruct *style)
{
    int x, y, limit = 0;
    object *new_obj;

    /* while returning a null object will result in a crash, that
     * is actually preferable to an infinite loop.  That is because
     * most servers will automatically restart in case of crash.
     * Change the logic on getting the random space - shouldn't make
     * any difference, but this seems clearer to me.
     */
    do {
        limit++;
        x = RANDOM()%MAP_WIDTH(style);
        y = RANDOM()%MAP_HEIGHT(style);
        new_obj = GET_MAP_OB(style, x, y);
    } while (new_obj == NULL && limit < 1000);
    return HEAD(new_obj);
}

/**
 * Frees cached style maps.
 */
void free_style_maps(void)
{
    mapstruct *next;
    int  style_maps = 0;

    /* delete_map will try to free it from the linked list,
     * but won't find it, so we need to do it ourselves
     */
    while (styles) {
        next = styles->next;
        delete_map(styles);
        styles = next;
        style_maps++;
    }
    LOG(llevDebug, "free_style_maps: Freed %d maps\n", style_maps);
}
