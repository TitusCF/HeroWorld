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
 * @file map.c
 * Map-related functions.
 */

#include <stdio.h>
#include <global.h>
#include <sproto.h>

#include <loader.h>
#ifndef WIN32 /* ---win32 exclude header */
#include <unistd.h>
#endif /* win32 */

#include "path.h"

extern int nrofallocobjects, nroffreeobjects;

static void free_all_objects(mapstruct *m);

/**
 * These correspond to the layer names in map.h -
 * since some of the types can be on multiple layers,
 * names are duplicated to correspond to that layer.
 */
const char *const map_layer_name[MAP_LAYERS] = {
    "floor", "no_pick", "no_pick", "item", "item",
    "item", "living", "living", "fly", "fly"
};

/** Information about a layer. */
typedef struct Map_Layer_Info {
    uint8 high_layer;       /**< Highest layer for this group. */
    uint8 honor_visibility; /**< If 0 then don't reorder items, else allow. */
} Map_Layer_Info;

/**
 * the ob->map_layer holds the low layer.  For the update_position()
 * logic, we also need to know the higher layer and whether
 * visibility should be honored.  This table has that information,
 * so that it doesn't need to be hardcoded.
 */
static const Map_Layer_Info map_layer_info[MAP_LAYERS] = {
    { MAP_LAYER_FLOOR, 1 },
    { MAP_LAYER_NO_PICK2, 0 }, { MAP_LAYER_NO_PICK2, 0 },
    { MAP_LAYER_ITEM3, 1 }, { MAP_LAYER_ITEM3, 1 }, { MAP_LAYER_ITEM3, 1 },
    { MAP_LAYER_LIVING2, 1 }, { MAP_LAYER_LIVING2, 1 },
    { MAP_LAYER_FLY2, 1 }, { MAP_LAYER_FLY2, 1 }
};

/**
 * Checks whether map has been loaded.
 * @param name
 * path of the map to search. Can be NULL.
 * @return
 * the mapstruct which has a name matching the given argument.
 * return NULL if no match is found.
 */
mapstruct *has_been_loaded(const char *name) {
    mapstruct *map;

    if (!name || !*name)
        return NULL;

    for (map = first_map; map; map = map->next)
        if (!strcmp(name, map->path))
            break;
    return (map);
}

/**
 * This makes a path absolute outside the world of Crossfire.
 * In other words, it prepends LIBDIR/MAPDIR/ to the given path
 * and returns the pointer to a static array containing the result.
 * it really should be called create_mapname
 *
 * @param name
 * path of the map.
 * @param buf
 * buffer that will contain the full path.
 * @param size
 * buffer's length.
 * @return
 * buf.
 */
char *create_pathname(const char *name, char *buf, size_t size) {
    /* Why?  having extra / doesn't confuse unix anyplace?  Dependancies
     * someplace else in the code? msw 2-17-97
     */
    if (*name == '/')
        snprintf(buf, size, "%s/%s%s", settings.datadir, settings.mapdir, name);
    else
        snprintf(buf, size, "%s/%s/%s", settings.datadir, settings.mapdir, name);
    return buf;
}

/**
 * Same as create_pathname(), but for the overlay maps.
 /
 * @param name
 * path of the overlay map.
 * @param buf
 * buffer that will contain the full path.
 * @param size
 * buffer's length.
 */
void create_overlay_pathname(const char *name, char *buf, size_t size) {
    /* Why?  having extra / doesn't confuse unix anyplace?  Dependancies
     * someplace else in the code? msw 2-17-97
     */
    if (*name == '/')
        snprintf(buf, size, "%s/%s%s", settings.localdir, settings.mapdir, name);
    else
        snprintf(buf, size, "%s/%s/%s", settings.localdir, settings.mapdir, name);
}

/**
 * same as create_pathname(), but for the template maps.
 *
 * @param name
 * path of the template map.
 * @param buf
 * buffer that will contain the full path.
 * @param size
 * buf's length
 */
void create_template_pathname(const char *name, char *buf, size_t size) {
    /* Why?  having extra / doesn't confuse unix anyplace?  Dependancies
     * someplace else in the code? msw 2-17-97
     */
    if (*name == '/')
        snprintf(buf, size, "%s/%s%s", settings.localdir, settings.templatedir, name);
    else
        snprintf(buf, size, "%s/%s/%s", settings.localdir, settings.templatedir, name);
}

/**
 * This makes absolute path to the itemfile where unique objects
 * will be saved. Converts '/' to '@'. I think it's essier maintain
 * files than full directory structure, but if this is problem it can
 * be changed.
 *
 * @param s
 * path of the map for the item.
 * @param buf
 * buffer that will contain path. Must not be NULL.
 * @param size
 * buffer's length.
 */
static void create_items_path(const char *s, char *buf, size_t size) {
    char *t;

    if (*s == '/')
        s++;

    snprintf(buf, size, "%s/%s/", settings.localdir, settings.uniquedir);
    t = buf+strlen(buf);
    snprintf(t, buf+size-t, "%s", s);

    while (*t != '\0') {
        if (*t == '/')
            *t = '@';
        t++;
    }
}

/**
 * This function checks if a file with the given path exists.
 *
 * It tries out all the compression suffixes listed in the uncomp[] array,
 * except for Windows which checks an exact file.
 *
 * @param name
 * map path to check.
 * @param prepend_dir
 * If set, then we call create_pathname (which prepends libdir & mapdir).
 * Otherwise, we assume the name given is fully complete.
 * @return
 * -1 if it fails, otherwise the mode of the file is returned.
 *
 * @note
 * Only the editor actually cares about the writablity of this -
 * the rest of the code only cares that the file is readable.
 * when the editor goes away, the call to stat should probably be
 * replaced by an access instead (similar to the windows one, but
 * that seems to be missing the prepend_dir processing
 */
int check_path(const char *name, int prepend_dir) {
    char buf[MAX_BUF];
#ifndef WIN32
    struct stat statbuf;
    int mode = 0;
#endif

    if (prepend_dir)
        create_pathname(name, buf, MAX_BUF);
    else
        snprintf(buf, sizeof(buf), "%s", name);
#ifdef WIN32 /* ***win32: check this sucker in windows style. */
    return(_access(buf, 0));
#else

    if (stat(buf, &statbuf) != 0)
        return -1;

    if (!S_ISREG(statbuf.st_mode))
        return (-1);

    if (((statbuf.st_mode&S_IRGRP) && getegid() == statbuf.st_gid)
    || ((statbuf.st_mode&S_IRUSR) && geteuid() == statbuf.st_uid)
    || (statbuf.st_mode&S_IROTH))
        mode |= 4;

    if ((statbuf.st_mode&S_IWGRP && getegid() == statbuf.st_gid)
    || (statbuf.st_mode&S_IWUSR && geteuid() == statbuf.st_uid)
    || (statbuf.st_mode&S_IWOTH))
        mode |= 2;

    return (mode);
#endif
}

/**
 * Prints out debug-information about a map.
 * Dumping these at llevError doesn't seem right, but is
 * necessary to make sure the information is in fact logged.
 * Can be used by a DM with the dumpmap command.
 *
 * @param m
 * map to dump.
 */
void dump_map(const mapstruct *m) {
    LOG(llevError, "Map %s status: %d.\n", m->path, m->in_memory);
    LOG(llevError, "Size: %dx%d Start: %d,%d\n", MAP_WIDTH(m), MAP_HEIGHT(m), MAP_ENTER_X(m), MAP_ENTER_Y(m));

    if (m->msg != NULL)
        LOG(llevError, "Message:\n%s", m->msg);

    if (m->maplore != NULL)
        LOG(llevError, "Lore:\n%s", m->maplore);

    if (m->tmpname != NULL)
        LOG(llevError, "Tmpname: %s\n", m->tmpname);

    LOG(llevError, "Difficulty: %d\n", m->difficulty);
    LOG(llevError, "Darkness: %d\n", m->darkness);
}

/**
 * Prints out debug-information about all maps.
 * This basically just goes through all the maps and calls
 * dump_map() on each one.
 * Can be used by a DM with the dumpallmaps command.
 */
void dump_all_maps(void) {
    mapstruct *m;

    for (m = first_map; m != NULL; m = m->next) {
        dump_map(m);
    }
}

/**
 * This rolls up wall, blocks_magic, blocks_view, etc, all into
 * one function that just returns a P_.. value (see map.h)
 * it will also do map translation for tiled maps, returning
 * new values into newmap, nx, and ny.  Any and all of those
 * values can be null, in which case if a new map is needed (returned
 * by a P_NEW_MAP value, another call to get_map_from_coord
 * is needed.  The case of not passing values is if we're just
 * checking for the existence of something on those spaces, but
 * don't expect to insert/remove anything from those spaces.
 *
 * @param oldmap
 * map for which we want information.
 * @param newmap
 * if not NULL, will contain the actual map checked if not oldmap.
 * @param x
 * @param y
 * coordinates to check
 * @param nx
 * @param ny
 * if not NULL, will contain the actual coordinates checked.
 * @return
 * flags for specified position, with maybe ::P_OUT_OF_MAP or ::P_NEW_MAP set.
 */
int get_map_flags(mapstruct *oldmap, mapstruct **newmap, sint16 x, sint16 y, sint16 *nx, sint16 *ny) {
    sint16 newx, newy;
    int retval = 0;
    mapstruct *mp;

    newx = x;
    newy = y;
    mp = get_map_from_coord(oldmap, &newx, &newy);
    if (!mp)
        return P_OUT_OF_MAP;
    if (mp != oldmap)
        retval |= P_NEW_MAP;
    if (newmap)
        *newmap = mp;
    if (nx)
        *nx = newx;
    if (ny)
        *ny = newy;
    retval |= mp->spaces[newx+mp->width*newy].flags;
    return retval;
}

/**
 * Returns true if the given coordinate is blocked except by the
 * object passed is not blocking.  This is used with
 * multipart monsters - if we want to see if a 2x2 monster
 * can move 1 space to the left, we don't want its own area
 * to block it from moving there.
 *
 * @param ob
 * object we ignore. Must not be NULL.
 * @param m
 * map we're considering.
 * @param sx
 * @param sy
 * target coordinates
 * @return
 * TRUE if the space is blocked by something other than ob.
 *
 * @note
 * the coordinates & map passed in should have been updated for tiling
 * by the caller.
 */
int blocked_link(object *ob, mapstruct *m, int sx, int sy) {
    object *tmp_head;
    int mflags, blocked;

    /* Make sure the coordinates are valid - they should be, as caller should
     * have already checked this.
     */
    if (OUT_OF_REAL_MAP(m, sx, sy)) {
        LOG(llevError, "blocked_link: Passed map, x, y coordinates outside of map\n");
        return 1;
    }

    /* special hack for transports: if it's a transport with a move_type of 0, it can do on the space anyway */
    if (ob->type == TRANSPORT && ob->move_type == 0)
        return 0;

    /* Save some cycles - instead of calling get_map_flags(), just get the value
     * directly.
     */
    mflags = m->spaces[sx+m->width*sy].flags;

    blocked = GET_MAP_MOVE_BLOCK(m, sx, sy);

    /* If space is currently not blocked by anything, no need to
     * go further.  Not true for players - all sorts of special
     * things we need to do for players.
     */
    if (ob->type != PLAYER && !(mflags&P_IS_ALIVE) && (blocked == 0))
        return 0;

    /* if there isn't anytyhing alive on this space, and this space isn't
     * otherwise blocked, we can return now.  Only if there is a living
     * creature do we need to investigate if it is part of this creature
     * or another.  Likewise, only if something is blocking us do we
     * need to investigate if there is a special circumstance that would
     * let the player through (inventory checkers for example)
     */
    if (!(mflags&P_IS_ALIVE) && !OB_TYPE_MOVE_BLOCK(ob, blocked))
        return 0;

    ob = HEAD(ob);

    /* We basically go through the stack of objects, and if there is
     * some other object that has NO_PASS or FLAG_ALIVE set, return
     * true.  If we get through the entire stack, that must mean
     * ob is blocking it, so return 0.
     */
    FOR_MAP_PREPARE(m, sx, sy, tmp) {
        /* Never block part of self. */
        tmp_head = HEAD(tmp);
        if (tmp_head == ob)
            continue;
        /* This must be before the checks below.  Code for inventory checkers. */
        if (tmp->type == CHECK_INV && OB_MOVE_BLOCK(ob, tmp)) {
            /* If last_sp is set, the player/monster needs an object,
             * so we check for it.  If they don't have it, they can't
             * pass through this space.
             */
            if (tmp->last_sp) {
                if (check_inv_recursive(ob, tmp) == NULL) {
                    if (tmp->msg) {
                        /* Optionally display the reason why one cannot move
                         * there.  Note: emitting a message from this function
                         * is not very elegant.  Ideally, this should be done
                         * somewhere in server/player.c, but this is difficult
                         * for objects of type CHECK_INV that are not alive.
                         */
                        draw_ext_info(NDI_UNIQUE|NDI_NAVY, 0, ob,
                                      MSG_TYPE_ATTACK, MSG_TYPE_ATTACK_NOKEY,
                                      tmp->msg);
                    }
                    return 1;
                }
            } else {
                /* In this case, the player must not have the object -
                 * if they do, they can't pass through.
                 */
                if (check_inv_recursive(ob, tmp) != NULL) {
                    if (tmp->msg) {
                        draw_ext_info(NDI_UNIQUE|NDI_NAVY, 0, ob,
                                      MSG_TYPE_ATTACK, MSG_TYPE_ATTACK_NOKEY,
                                      tmp->msg);
                    }
                    return 1;
                }
            }
        } /* if check_inv */
        else {
            /* Broke apart a big nasty if into several here to make
             * this more readable.  first check - if the space blocks
             * movement, can't move here.
             * second - if a monster, can't move there, unless it is a
             * hidden dm
             */
            if (OB_MOVE_BLOCK(ob, tmp))
                return 1;
            if (QUERY_FLAG(tmp, FLAG_ALIVE)
            && tmp->head != ob
            && tmp != ob
            && tmp->type != DOOR
            && !(QUERY_FLAG(tmp, FLAG_WIZ) && tmp->contr->hidden))
                return 1;
        }
    } FOR_MAP_FINISH();
    return 0;
}

/**
 * Returns true if the given object can't fit in the given spot.
 * This is meant for multi space objects - for single space objecs,
 * just calling get_map_blocked and checking that against movement type
 * of object. This function goes through all the parts of the
 * multipart object and makes sure they can be inserted.
 *
 * While this doesn't call out of map, the get_map_flags does.
 *
 * This function has been used to deprecate arch_out_of_map -
 * this function also does that check, and since in most cases,
 * a call to one would follow the other, doesn't make a lot of sense to
 * have two seperate functions for this.
 *
 * This returns nonzero if this arch can not go on the space provided,
 * 0 otherwise.  the return value will contain the P_.. value
 * so the caller can know why this object can't go on the map.
 * Note that callers should not expect ::P_NEW_MAP to be set
 * in return codes - since the object is multispace - if
 * we did return values, what do you return if half the object
 * is one map, half on another.
 *
 * @param ob
 * object to test.
 * @param m
 * @param x
 * @param y
 * map and coordinates to check.
 * @return
 * 0 if the object can fit on specified space, non-zero else.
 *
 * @note
 * This used to be arch_blocked, but with new movement
 * code, we need to have actual object to check its move_type
 * against the move_block values.
 */
int ob_blocked(const object *ob, mapstruct *m, sint16 x, sint16 y) {
    archetype *tmp;
    int flag;
    mapstruct *m1;
    sint16 sx, sy;
    const object *part;

    if (ob == NULL) {
        flag = get_map_flags(m, &m1, x, y, &sx, &sy);
        if (flag&P_OUT_OF_MAP)
            return P_OUT_OF_MAP;

        /* don't have object, so don't know what types would block */
        return(GET_MAP_MOVE_BLOCK(m1, sx, sy));
    }

    for (tmp = ob->arch, part = ob; tmp != NULL; tmp = tmp->more, part = part->more) {
        flag = get_map_flags(m, &m1, x+tmp->clone.x, y+tmp->clone.y, &sx, &sy);

        if (flag&P_OUT_OF_MAP)
            return P_OUT_OF_MAP;
        if (flag&P_IS_ALIVE)
            return P_IS_ALIVE;

        /* object_find_first_free_spot() calls this function.  However, often
         * ob doesn't have any move type (when used to place exits)
         * so the AND operation in OB_TYPE_MOVE_BLOCK doesn't work.
         */

        if (ob->move_type == 0 && GET_MAP_MOVE_BLOCK(m1, sx, sy) != MOVE_ALL)
            continue;

        /* A transport without move_type for a part should go through everything for that part. */
        if (ob->type == TRANSPORT && part->move_type == 0)
            continue;

        /* Note it is intentional that we check ob - the movement type of the
         * head of the object should correspond for the entire object.
         */
        if (OB_TYPE_MOVE_BLOCK(ob, GET_MAP_MOVE_BLOCK(m1, sx, sy)))
            return AB_NO_PASS;
    }
    return 0;
}

/**
 * When the map is loaded, load_object() does not actually insert objects
 * into inventory, but just links them.  What this does is go through
 * and insert them properly.
 * @param container
 * object that contains the inventory. This is needed so that we can update the containers weight.
 *
 * @todo
 * This is unusued, should it be used somewhere?
 */
void fix_container(object *container) {
    object *tmp = container->inv;

    container->inv = NULL;
    FOR_OB_AND_BELOW_PREPARE(tmp) {
        if (tmp->inv)
            fix_container(tmp);
        (void)object_insert_in_ob(tmp, container);
    } FOR_OB_AND_BELOW_FINISH();
    /* object_sum_weight will go through and calculate what all the containers are
     * carrying.
     */
    object_sum_weight(container);
}

/**
 * Go through all the objects in a container (recursively) looking
 * for objects whose arch says they are multipart yet according to the
 * info we have, they only have the head (as would be expected when
 * they are saved).  We do have to look for the old maps that did save
 * the more sections and not re-add sections for them.
 *
 * @param container
 * object that contains the inventory.
 */
static void fix_container_multipart(object *container) {
    FOR_INV_PREPARE(container, tmp) {
        archetype *at;
        object *op, *last;

        if (tmp->inv)
            fix_container_multipart(tmp);
        /* already multipart, or non-multipart arch - don't do anything more */
        for (at = tmp->arch->more, last = tmp; at != NULL; at = at->more, last = op) {
            /* FIXME: We can't reuse object_fix_multipart() since that only
             * works for items directly on maps. Maybe factor out common code?
             */
            op = arch_to_object(at);
            op->head = tmp;
            op->env = tmp->env;
            last->more = op;
            if (tmp->name != op->name) {
                if (op->name)
                    free_string(op->name);
                op->name = add_string(tmp->name);
            }
            if (tmp->title != op->title) {
                if (op->title)
                    free_string(op->title);
                op->title = add_string(tmp->title);
            }
            CLEAR_FLAG(op, FLAG_REMOVED);
        }
    } FOR_INV_FINISH();
}

/**
 * Go through all the objects on the map looking
 * for objects whose arch says they are multipart yet according to the
 * info we have, they only have the head (as would be expected when
 * they are saved).  We do have to look for the old maps that did save
 * the more sections and not re-add sections for them.
 *
 * @param m
 * map to check.
 */
static void link_multipart_objects(mapstruct *m) {
    int x, y;

    for (x = 0; x < MAP_WIDTH(m); x++)
        for (y = 0; y < MAP_HEIGHT(m); y++)
            FOR_MAP_PREPARE(m, x, y, tmp) {
                if (tmp->inv)
                    fix_container_multipart(tmp);

                /* already multipart - don't do anything more */
                if (tmp->head || tmp->more)
                    continue;

                object_fix_multipart(tmp);
            } FOR_MAP_FINISH(); /* for objects on this space */
}

/**
 * Loads (and parses) the objects into a given map from the specified
 * file pointer.
 *
 * @param m
 * m being loaded.
 * @param fp
 * file to read from.
 * @param mapflags
 * the same as we get with load_original_map()
 */
static void load_objects(mapstruct *m, FILE *fp, int mapflags) {
    int i, j, bufstate = LO_NEWFILE;
    int unique;
    object *op, *prev = NULL, *last_more = NULL;

    op = object_new();
    op->map = m; /* To handle buttons correctly */

    while ((i = load_object(fp, op, bufstate, mapflags))) {
        /* Since the loading of the map header does not load an object
         * anymore, we need to pass LO_NEWFILE for the first object loaded,
         * and then switch to LO_REPEAT for faster loading.
         */
        bufstate = LO_REPEAT;

        /* if the archetype for the object is null, means that we
         * got an invalid object.  Don't do anything with it - the game
         * or editor will not be able to do anything with it either.
         */
        if (op->arch == NULL) {
            LOG(llevDebug, "Discarding object without arch: %s\n", op->name ? op->name : "(null)");
            continue;
        }

        /*
         * You can NOT have players on a map being loaded.
         * Trying to use such a type leads to crashes everywhere as op->contr is NULL.
         */
        if (op->type == PLAYER) {
            LOG(llevError, "Discarding invalid item with type PLAYER in map %s\n", m->path);
            continue;
        }

        /* don't use out_of_map because we don't want to consider tiling properties, we're loading a single map */
        if (op->x < 0 || op->y < 0 || op->x >= MAP_WIDTH(m) || op->y >= MAP_HEIGHT(m)) {
            LOG(llevError, " object %s not on valid map position %s:%d:%d\n", op->name ? op->name : "(null)", m->path, op->x, op->y);
            if (op->x < 0) {
                op->x = 0;
            } else if (op->x >= MAP_WIDTH(m)) {
                op->x = MAP_WIDTH(m) - 1;
            }
            if (op->y < 0) {
                op->y = 0;
            } else if (op->y >= MAP_HEIGHT(m)) {
                op->y = MAP_HEIGHT(m) - 1;
            }
        }

        switch (i) {
        case LL_NORMAL:
            /* if we are loading an overlay, put the floors on the bottom */
            if ((QUERY_FLAG(op, FLAG_IS_FLOOR) || QUERY_FLAG(op, FLAG_OVERLAY_FLOOR))
            && mapflags&MAP_OVERLAY)
                object_insert_in_map_at(op, m, op, INS_NO_MERGE|INS_NO_WALK_ON|INS_ABOVE_FLOOR_ONLY|INS_MAP_LOAD, op->x, op->y);
            else
                object_insert_in_map_at(op, m, op, INS_NO_MERGE|INS_NO_WALK_ON|INS_ON_TOP|INS_MAP_LOAD, op->x, op->y);

            if (op->inv)
                object_sum_weight(op);

            prev = op,
            last_more = op;
            break;

        case LL_MORE:
            object_insert_in_map_at(op, m, op, INS_NO_MERGE|INS_NO_WALK_ON|INS_ABOVE_FLOOR_ONLY, op->x, op->y);
            op->head = prev,
            last_more->more = op,
            last_more = op;
            break;
        }
        if (mapflags&MAP_STYLE) {
            object_remove_from_active_list(op);
        }
        op = object_new();
        op->map = m;
    }
    for (i = 0; i < m->width; i++) {
        for (j = 0; j < m->height; j++) {
            unique = 0;
            /* check for unique items, or unique squares */
            FOR_MAP_PREPARE(m, i, j, otmp) {
                if (QUERY_FLAG(otmp, FLAG_UNIQUE))
                    unique = 1;
                if (!(mapflags&(MAP_OVERLAY|MAP_PLAYER_UNIQUE) || unique))
                    SET_FLAG(otmp, FLAG_OBJ_ORIGINAL);
            } FOR_MAP_FINISH();
        }
    }
    object_free_drop_inventory(op);
    link_multipart_objects(m);
}

/**
 * This saves all the objects on the map in a non destructive fashion.
 * Modified by MSW 2001-07-01 to do in a single pass - reduces code,
 * and we only save the head of multi part objects - this is needed
 * in order to do map tiling properly.
 *
 * @param m
 * map to save.
 * @param fp
 * file where regular objects are saved.
 * @param fp2
 * file to save unique objects.
 * @param flag
 * combination of @ref SAVE_FLAG_xxx "SAVE_FLAG_xxx" flags.
 * @return
 * one of @ref SAVE_ERROR_xxx "SAVE_ERROR_xxx" value.
 */
static int save_objects(mapstruct *m, FILE *fp, FILE *fp2, int flag) {
    int i, j = 0, unique = 0, res = 0;

    /* first pass - save one-part objects */
    for (i = 0; i < MAP_WIDTH(m); i++)
        for (j = 0; j < MAP_HEIGHT(m); j++) {
            unique = 0;
            FOR_MAP_PREPARE(m, i, j, op) {
                if (QUERY_FLAG(op, FLAG_IS_FLOOR) && QUERY_FLAG(op, FLAG_UNIQUE))
                    unique = 1;

                if (op->type == PLAYER) {
                    LOG(llevDebug, "Player on map that is being saved\n");
                    continue;
                }

                if (op->head || object_get_owner(op) != NULL)
                    continue;

                if (unique || QUERY_FLAG(op, FLAG_UNIQUE))
                    res = save_object(fp2, op, SAVE_FLAG_SAVE_UNPAID|SAVE_FLAG_NO_REMOVE);
                else
                    if (flag == 0
                    || (flag == SAVE_FLAG_NO_REMOVE && (!QUERY_FLAG(op, FLAG_OBJ_ORIGINAL) && !QUERY_FLAG(op, FLAG_UNPAID))))
                        res = save_object(fp, op, SAVE_FLAG_SAVE_UNPAID|SAVE_FLAG_NO_REMOVE);

                if (res != 0)
                    return res;
            } FOR_MAP_FINISH(); /* for this space */
        } /* for this j */

    return 0;
}

/**
 * Allocates, initialises, and returns a pointer to a mapstruct.
 * Modified to no longer take a path option which was not being
 * used anyways.  MSW 2001-07-01
 *
 * @return
 * new structure.
 *
 * @note
 * will never return NULL, but call fatal() if memory error.
 */
mapstruct *get_linked_map(void) {
    mapstruct *map = (mapstruct *)calloc(1, sizeof(mapstruct));
    mapstruct *mp;

    if (map == NULL)
        fatal(OUT_OF_MEMORY);

    for (mp = first_map; mp != NULL && mp->next != NULL; mp = mp->next)
        ;
    if (mp == NULL)
        first_map = map;
    else
        mp->next = map;

    map->in_memory = MAP_SWAPPED;
    /* The maps used to pick up default x and y values from the
     * map archetype.  Mimic that behaviour.
     */
    MAP_WIDTH(map) = 16;
    MAP_HEIGHT(map) = 16;
    MAP_RESET_TIMEOUT(map) = 0;
    MAP_TIMEOUT(map) = 300;
    MAP_ENTER_X(map) = 0;
    MAP_ENTER_Y(map) = 0;
    map->last_reset_time.tv_sec = 0;
    return map;
}

/**
 * This basically allocates the dynamic array of spaces for the
 * map.
 *
 * @param m
 * map to check.
 *
 * @note
 * will never fail, since it calls fatal() if memory allocation failure.
 */
static void allocate_map(mapstruct *m) {
    m->in_memory = MAP_IN_MEMORY;
    /* Log this condition and free the storage.  We could I suppose
     * realloc, but if the caller is presuming the data will be intact,
     * that is their poor assumption.
     */
    if (m->spaces) {
        LOG(llevError, "allocate_map called with already allocated map (%s)\n", m->path);
        free(m->spaces);
    }

    m->spaces = calloc(1, MAP_WIDTH(m)*MAP_HEIGHT(m)*sizeof(MapSpace));

    if (m->spaces == NULL)
        fatal(OUT_OF_MEMORY);
}

/**
 * Creates and returns a map of the specific size.  Used
 * in random map code and the editor.
 *
 * @param sizex
 * @param sizey
 * map size.
 * @return
 * new map.
 *
 * @note
 * will never return NULL, as get_linked_map() never fails.
 */
mapstruct *get_empty_map(int sizex, int sizey) {
    mapstruct *m = get_linked_map();
    m->width = sizex;
    m->height = sizey;
    m->in_memory = MAP_SWAPPED;
    allocate_map(m);
    return m;
}

/**
 * Takes a string from a map definition and outputs a pointer to the array of shopitems
 * corresponding to that string. Memory is allocated for this, it must be freed
 * at a later date.
 * Called by parse_map_headers() below.
 *
 * @param input_string
 * shop item line.
 * @param map
 * map for which to parse the string, in case of warning.
 * @return
 * new array that should be freed by the caller.
 */
static shopitems *parse_shop_string(const char *input_string, const mapstruct *map) {
    char *shop_string, *p, *q, *next_semicolon, *next_colon;
    shopitems *items = NULL;
    int i = 0, number_of_entries = 0;
    const typedata *current_type;

    shop_string = strdup_local(input_string);
    p = shop_string;
    LOG(llevDebug, "parsing %s\n", input_string);
    /* first we'll count the entries, we'll need that for allocating the array shortly */
    while (p) {
        p = strchr(p, ';');
        number_of_entries++;
        if (p)
            p++;
    }
    p = shop_string;
    strip_endline(p);
    items = CALLOC(number_of_entries+1, sizeof(shopitems));
    memset(items, 0, (sizeof(shopitems)*number_of_entries+1));
    for (i = 0; i < number_of_entries; i++) {
        if (!p) {
            LOG(llevError, "parse_shop_string: I seem to have run out of string, that shouldn't happen.\n");
            break;
        }
        next_semicolon = strchr(p, ';');
        next_colon = strchr(p, ':');
        /* if there is a stregth specified, figure out what it is, we'll need it soon. */
        if (next_colon && (!next_semicolon || next_colon < next_semicolon))
            items[i].strength = atoi(strchr(p, ':')+1);

        if (isdigit(*p) || *p == '*') {
            items[i].typenum = *p == '*' ? -1 : atoi(p);
            current_type = get_typedata(items[i].typenum);
            if (current_type) {
                items[i].name = current_type->name;
                items[i].name_pl = current_type->name_pl;
            }
        } else { /*we have a named type, let's figure out what it is */
            q = strpbrk(p, ";:");
            if (q)
                *q = '\0';

            current_type = get_typedata_by_name(p);
            if (current_type) {
                items[i].name = current_type->name;
                items[i].typenum = current_type->number;
                items[i].name_pl = current_type->name_pl;
            } else { /* oh uh, something's wrong, let's free up this one, and try
                    * the next entry while we're at it, better print a warning
                    */
                LOG(llevError, "invalid type %s defined in shopitems for %s in string %s\n", p, map->path ? map->path : map->name, input_string);
            }
        }
        items[i].index = number_of_entries;
        if (next_semicolon)
            p = ++next_semicolon;
        else
            p = NULL;
    }
    free(shop_string);
    return items;
}

/**
 * Opposite of parse string(), this puts the string that was originally fed in to
 * the map (or something equivilent) into output_string.
 *
 * @param m
 * map we're considering.
 * @param output_string
 * string to write to.
 * @param size
 * output_string's length.
 */
static void print_shop_string(mapstruct *m, char *output_string, int size) {
    int i;
    char tmp[MAX_BUF];

    output_string[0] = '\0';
    for (i = 0; i < m->shopitems[0].index; i++) {
        if (m->shopitems[i].typenum != -1) {
            if (m->shopitems[i].strength) {
                snprintf(tmp, sizeof(tmp), "%s:%d;", m->shopitems[i].name, m->shopitems[i].strength);
            } else
                snprintf(tmp, sizeof(tmp), "%s;", m->shopitems[i].name);
        } else {
            if (m->shopitems[i].strength) {
                snprintf(tmp, sizeof(tmp), "*:%d;", m->shopitems[i].strength);
            } else
                snprintf(tmp, sizeof(tmp), "*;");
        }
        snprintf(output_string+strlen(output_string), size-strlen(output_string), "%s", tmp);
    }

    /* erase final ; else parsing back will lead to issues */
    if (strlen(output_string) > 0) {
        output_string[strlen(output_string) - 1] = '\0';
    }
}

/**
 * This loads the header information of the map.  The header
 * contains things like difficulty, size, timeout, etc.
 * this used to be stored in the map object, but with the
 * addition of tiling, fields beyond that easily named in an
 * object structure were needed, so it just made sense to
 * put all the stuff in the map object so that names actually make
 * sense.
 * This could be done in lex (like the object loader), but I think
 * currently, there are few enough fields this is not a big deal.
 * MSW 2001-07-01
 *
 * @param fp
 * file to read from.
 * @param m
 * map being read.
 * @return
 * 0 on success, 1 on failure.
 */
static int load_map_header(FILE *fp, mapstruct *m) {
    char buf[HUGE_BUF], *key = NULL, *value;

    m->width = m->height = 0;
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        char *p;

        p = strchr(buf, '\n');
        if (p == NULL) {
            LOG(llevError, "Error loading map header - did not find a newline - perhaps file is truncated?  Buf=%s\n", buf);
            return 1;
        }
        *p = '\0';

        key = buf;
        while (isspace(*key))
            key++;
        if (*key == 0)
            continue;    /* empty line */
        value = strchr(key, ' ');
        if (value) {
            *value = 0;
            value++;
            while (isspace(*value)) {
                value++;
                if (*value == '\0') {
                    /* Nothing but spaces. */
                    value = NULL;
                    break;
                }
            }
        }

        /* key is the field name, value is what it should be set
         * to.  We've already done the work to null terminate key,
         * and strip off any leading spaces for both of these.
         * We have not touched the newline at the end of the line -
         * these are needed for some values.  the end pointer
         * points to the first of the newlines.
         * value could be NULL!  It would be easy enough to just point
         * this to "" to prevent cores, but that would let more errors slide
         * through.
         *
         * First check for entries that do not use the value parameter, then
         * validate that value is given and check for the remaining entries
         * that use the parameter.
         */

        if (!strcmp(key, "msg")) {
            char msgbuf[HUGE_BUF];
            int msgpos = 0;

            while (fgets(buf, sizeof(buf), fp) != NULL) {
                if (!strcmp(buf, "endmsg\n"))
                    break;
                else {
                    snprintf(msgbuf+msgpos, sizeof(msgbuf)-msgpos, "%s", buf);
                    msgpos += strlen(buf);
                }
            }
            /* There are lots of maps that have empty messages (eg, msg/endmsg
             * with nothing between).  There is no reason in those cases to
             * keep the empty message.  Also, msgbuf contains garbage data
             * when msgpos is zero, so copying it results in crashes
             */
            if (msgpos != 0) {
                /* When loading eg an overlay, message is already set, so free() current one. */
                free(m->msg);
                m->msg = strdup_local(msgbuf);
            }
        } else if (!strcmp(key, "maplore")) {
            char maplorebuf[HUGE_BUF];
            int maplorepos = 0;

            while (fgets(buf, HUGE_BUF-1, fp) != NULL) {
                if (!strcmp(buf, "endmaplore\n"))
                    break;
                else {
                    snprintf(maplorebuf+maplorepos, sizeof(maplorebuf)-maplorepos, "%s", buf);
                    maplorepos += strlen(buf);
                }
            }
            if (maplorepos != 0)
                m->maplore = strdup_local(maplorebuf);
        } else if (!strcmp(key, "end")) {
            break;
        } else if (value == NULL) {
            LOG(llevError, "Got '%s' line without parameter in map header\n", key);
        } else if (!strcmp(key, "arch")) {
            /* This is an oddity, but not something we care about much. */
            if (strcmp(value, "map"))
                LOG(llevError, "loading map and got a non 'arch map' line(%s %s)?\n", key, value);
        } else if (!strcmp(key, "name")) {
            /* When loading eg an overlay, the name is already set, so free() current one. */
            free(m->name);
            m->name = strdup_local(value);
        /* first strcmp value on these are old names supported
         * for compatibility reasons.  The new values (second) are
         * what really should be used.
         */
        } else if (!strcmp(key, "enter_x")) {
            m->enter_x = atoi(value);
        } else if (!strcmp(key, "enter_y")) {
            m->enter_y = atoi(value);
        } else if (!strcmp(key, "width")) {
            m->width = atoi(value);
        } else if (!strcmp(key, "height")) {
            m->height = atoi(value);
        } else if (!strcmp(key, "reset_timeout")) {
            m->reset_timeout = atoi(value);
        } else if (!strcmp(key, "swap_time")) {
            m->timeout = atoi(value);
        } else if (!strcmp(key, "difficulty")) {
            m->difficulty = atoi(value);
        } else if (!strcmp(key, "darkness")) {
            m->darkness = atoi(value);
        } else if (!strcmp(key, "fixed_resettime")) {
            m->fixed_resettime = atoi(value);
        } else if (!strcmp(key, "unique")) {
            m->unique = atoi(value);
        } else if (!strcmp(key, "template")) {
            m->is_template = atoi(value);
        } else if (!strcmp(key, "region")) {
            m->region = get_region_by_name(value);
        } else if (!strcmp(key, "shopitems")) {
            m->shopitems = parse_shop_string(value, m);
        } else if (!strcmp(key, "shopgreed")) {
            m->shopgreed = atof(value);
        } else if (!strcmp(key, "shopmin")) {
            m->shopmin = atol(value);
        } else if (!strcmp(key, "shopmax")) {
            m->shopmax = atol(value);
        } else if (!strcmp(key, "shoprace")) {
            m->shoprace = strdup_local(value);
        } else if (!strcmp(key, "outdoor")) {
            m->outdoor = atoi(value);
        } else if (!strcmp(key, "nosmooth")) {
            m->nosmooth = atoi(value);
        } else if (!strcmp(key, "first_load")) {
            m->last_reset_time.tv_sec = atoi(value);
        } else if (!strncmp(key, "tile_path_", 10)) {
            int tile = atoi(key+10);

            if (tile < 1 || tile > 4) {
                LOG(llevError, "load_map_header: tile location %d out of bounds (%s)\n", tile, m->path);
            } else {
                char path[HUGE_BUF];

                if (m->tile_path[tile-1]) {
                    LOG(llevError, "load_map_header: tile location %d duplicated (%s)\n", tile, m->path);
                    free(m->tile_path[tile-1]);
                    m->tile_path[tile-1] = NULL;
                }

                if (check_path(value, 1) != -1) {
                    /* The unadorned path works. */
                    snprintf(path, sizeof(path), "%s", value);
                } else {
                    /* Try again; it could be a relative exit. */
                    path_combine_and_normalize(m->path, value, path, sizeof(path));

                    if (check_path(path, 1) == -1) {
                        LOG(llevError, "get_map_header: Bad tile path %s %s\n", m->path, value);
                        path[0] = '\0';
                    }
                }

                if (*path != '\0') {
                    /* Use the normalized value. */
                    m->tile_path[tile-1] = strdup_local(path);
                }
            } /* end if tile direction (in)valid */
        } else  if (!strcmp(key, "background_music")) {
            m->background_music = strdup_local(value);
        } else {
            LOG(llevError, "Got unknown value in map header: %s %s\n", key, value);
        }
    }
    if ((m->width == 0) || (m->height == 0)) {
        LOG(llevError, "Map width or height not specified\n");
        return 1;
    }
    if (!key || strcmp(key, "end")) {
        LOG(llevError, "Got premature eof on map header!\n");
        return 1;
    }
    return 0;
}

/**
 * Opens the file "filename" and reads information about the map
 * from the given file, and stores it in a newly allocated
 * mapstruct.  A pointer to this structure is returned, or NULL on failure.
 * flags correspond to those in map.h.  Main ones used are
 * ::MAP_PLAYER_UNIQUE, in which case we don't do any name changes, and
 *
 * @param filename
 * map path.
 * @param flags
 * how to interpret the path, and misc information (can be combined):
 * \li ::MAP_PLAYER_UNIQUE: this is a unique map, path isn't changed
 * \li ::MAP_OVERLAY: map is an overlay
 * \li ::MAP_BLOCK: we block on this load.  This happens in all cases, no matter if this flag is set or not.
 * \li ::MAP_STYLE: style map - don't add active objects, don't add to server managed map list.
 * @return
 * loaded map, or NULL if failure.
 */
mapstruct *load_original_map(const char *filename, int flags) {
    FILE *fp;
    mapstruct *m;
    char pathname[MAX_BUF];

    LOG(llevDebug, "load_original_map: %s (%x)\n", filename, flags);
    if (flags&MAP_PLAYER_UNIQUE)
        snprintf(pathname, sizeof(pathname), "%s", filename);
    else if (flags&MAP_OVERLAY)
        create_overlay_pathname(filename, pathname, MAX_BUF);
    else
        create_pathname(filename, pathname, MAX_BUF);

    if ((fp = fopen(pathname, "r")) == NULL) {
        char err[MAX_BUF];

        LOG((flags&MAP_PLAYER_UNIQUE) ? llevDebug : llevError, "Can't open %s: %s\n", pathname, strerror_local(errno, err, sizeof(err)));
        return (NULL);
    }

    m = get_linked_map();

    strcpy(m->path, filename);
    if (load_map_header(fp, m)) {
        LOG(llevError, "Error loading map header for %s, flags=%d\n", filename, flags);
        delete_map(m);
        return NULL;
    }

    allocate_map(m);

    m->in_memory = MAP_LOADING;
    load_objects(m, fp, flags&(MAP_BLOCK|MAP_STYLE));
    fclose(fp);
    m->in_memory = MAP_IN_MEMORY;
    if (!MAP_DIFFICULTY(m))
        MAP_DIFFICULTY(m) = calculate_difficulty(m);
    set_map_reset_time(m);

    /* In case other objects press some buttons down */
    update_buttons(m);

    /* Handle for map load event */
    execute_global_event(EVENT_MAPLOAD, m);

    if (!(flags & MAP_STYLE))
        apply_auto_fix(m); /* Chests which open as default */

    return (m);
}

/**
 * Loads a map, which has been loaded earlier, from file.
 *
 * @param m
 * map we want to reload.
 * @return
 * 0 if success, non zero if failure, in which case error was LOG'ed.
 */
static int load_temporary_map(mapstruct *m) {
    FILE *fp;
    char buf[MAX_BUF];

    if (!m->tmpname) {
        LOG(llevError, "No temporary filename for map %s\n", m->path);
        return 1;
    }

    if ((fp = fopen(m->tmpname, "r")) == NULL) {
        LOG(llevError, "Cannot open %s: %s\n", m->tmpname, strerror_local(errno, buf, sizeof(buf)));
        return 2;
    }

    if (load_map_header(fp, m)) {
        LOG(llevError, "Error loading map header for %s (%s)\n", m->path, m->tmpname);
        return 3;
    }
    allocate_map(m);

    m->in_memory = MAP_LOADING;
    load_objects(m, fp, 0);
    fclose(fp);
    m->in_memory = MAP_IN_MEMORY;
    return 0;
}

/**
 * Loads an overlay for a map, which has been loaded earlier, from file.
 * @param filename
 * filename for overlay.
 * @param m
 * map we want to load.
 * @return
 * 0 on success, non zero in case of error, which is LOG'ed.
 */
static int load_overlay_map(const char *filename, mapstruct *m) {
    FILE *fp;
    char pathname[MAX_BUF];

    create_overlay_pathname(filename, pathname, MAX_BUF);

    if ((fp = fopen(pathname, "r")) == NULL) {
        /* nothing bad to not having an overlay */
        return 0;
    }

    if (load_map_header(fp, m)) {
        LOG(llevError, "Error loading map header for overlay %s (%s)\n", m->path, pathname);
        return 1;
    }
    /*allocate_map(m);*/

    m->in_memory = MAP_LOADING;
    load_objects(m, fp, MAP_OVERLAY);
    fclose(fp);
    m->in_memory = MAP_IN_MEMORY;
    return 0;
}

/******************************************************************************
 * This is the start of unique map handling code
 *****************************************************************************/

/**
 * This goes through map 'm' and removes any unique items on the map.
 *
 * @param m
 * map to check.
 */
static void delete_unique_items(mapstruct *m) {
    int i, j, unique = 0;

    for (i = 0; i < MAP_WIDTH(m); i++)
        for (j = 0; j < MAP_HEIGHT(m); j++) {
            unique = 0;
            FOR_MAP_PREPARE(m, i, j, op) {
                if (QUERY_FLAG(op, FLAG_IS_FLOOR) && QUERY_FLAG(op, FLAG_UNIQUE))
                    unique = 1;
                if (op->head == NULL && (QUERY_FLAG(op, FLAG_UNIQUE) || unique)) {
                    clean_object(op);
                    if (QUERY_FLAG(op, FLAG_IS_LINKED))
                        remove_button_link(op);
                    object_remove(op);
                    object_free_drop_inventory(op);
                }
            } FOR_MAP_FINISH();
        }
}

/**
 * Loads unique objects from file(s) into the map which is in memory
 * @param m
 * map to load unique items into.
 */
static void load_unique_objects(mapstruct *m) {
    FILE *fp;
    int count;
    char firstname[MAX_BUF], name[MAX_BUF];

    create_items_path(m->path, name, MAX_BUF);
    for (count = 0; count < 10; count++) {
        snprintf(firstname, sizeof(firstname), "%s.v%02d", name, count);
        if (!access(firstname, R_OK))
            break;
    }
    /* If we get here, we did not find any map */
    if (count == 10)
        return;

    if ((fp = fopen(firstname, "r")) == NULL) {
        /* There is no expectation that every map will have unique items, but this
        * is debug output, so leave it in.
        */
        LOG(llevDebug, "Can't open unique items file for %s\n", name);
        return;
    }

    m->in_memory = MAP_LOADING;
    if (m->tmpname == NULL)    /* if we have loaded unique items from */
        delete_unique_items(m); /* original map before, don't duplicate them */
    load_object(fp, NULL, LO_NOREAD, 0);
    load_objects(m, fp, 0);
    fclose(fp);
    m->in_memory = MAP_IN_MEMORY;
}

/**
 * Saves a map to file.  If flag is SAVE_MODE_INPLACE, it is saved into the same
 * file it was (originally) loaded from.  Otherwise a temporary
 * filename will be genarated, and the file will be stored there.
 * The temporary filename will be stored in the mapstructure.
 * If the map is unique, we also save to the filename in the map
 * (this should have been updated when first loaded)
 *
 * @param m
 * map to save.
 * @param flag
 * One of @ref SAVE_MODE_xxx "SAVE_MODE_xxx" values.
 * @return
 * one of @ref SAVE_ERROR_xxx "SAVE_ERROR_xxx" values.
 */
int save_map(mapstruct *m, int flag) {
#define TEMP_EXT ".savefile"
    FILE *fp, *fp2;
    char filename[MAX_BUF], buf[MAX_BUF], shop[MAX_BUF], final[MAX_BUF];
    int i, res;

    if (flag && !*m->path) {
        LOG(llevError, "Tried to save map without path.\n");
        return SAVE_ERROR_NO_PATH;
    }

    if (flag != SAVE_MODE_NORMAL || (m->unique) || (m->is_template)) {
        if (!m->unique && !m->is_template) { /* flag is set */
            if (flag == SAVE_MODE_OVERLAY)
                create_overlay_pathname(m->path, filename, MAX_BUF);
            else
                create_pathname(m->path, filename, MAX_BUF);
        } else
            snprintf(filename, sizeof(filename), "%s", m->path);

        make_path_to_file(filename);
    } else {
        if (!m->tmpname)
            m->tmpname = tempnam_local(settings.tmpdir, NULL);
        snprintf(filename, sizeof(filename), "%s", m->tmpname);
    }
    LOG(llevDebug, "Saving map %s\n", m->path);
    m->in_memory = MAP_SAVING;

    snprintf(final, sizeof(final), "%s", filename);
    snprintf(filename, sizeof(filename), "%s%s", final, TEMP_EXT);
    fp = fopen(filename, "w");

    if (fp == NULL) {
        LOG(llevError, "Cannot open regular objects file %s: %s\n", filename, strerror_local(errno, buf, sizeof(buf)));
        return SAVE_ERROR_RCREATION;
    }

    /* legacy */
    fprintf(fp, "arch map\n");
    if (m->name)
        fprintf(fp, "name %s\n", m->name);
    if (!flag)
        fprintf(fp, "swap_time %d\n", m->swap_time);
    if (m->reset_timeout)
        fprintf(fp, "reset_timeout %u\n", m->reset_timeout);
    if (m->fixed_resettime)
        fprintf(fp, "fixed_resettime %d\n", m->fixed_resettime);
    /* we unfortunately have no idea if this is a value the creator set
     * or a difficulty value we generated when the map was first loaded
     */
    if (m->difficulty)
        fprintf(fp, "difficulty %d\n", m->difficulty);
    if (m->region)
        fprintf(fp, "region %s\n", m->region->name);
    if (m->shopitems) {
        print_shop_string(m, shop, sizeof(shop));
        fprintf(fp, "shopitems %s\n", shop);
    }
    if (m->shopgreed)
        fprintf(fp, "shopgreed %f\n", m->shopgreed);
    if (m->shopmin)
        fprintf(fp, "shopmin %"FMT64U"\n", m->shopmin);
    if (m->shopmax)
        fprintf(fp, "shopmax %"FMT64U"\n", m->shopmax);
    if (m->shoprace)
        fprintf(fp, "shoprace %s\n", m->shoprace);
    if (m->darkness)
        fprintf(fp, "darkness %d\n", m->darkness);
    if (m->width)
        fprintf(fp, "width %d\n", m->width);
    if (m->height)
        fprintf(fp, "height %d\n", m->height);
    if (m->enter_x)
        fprintf(fp, "enter_x %d\n", m->enter_x);
    if (m->enter_y)
        fprintf(fp, "enter_y %d\n", m->enter_y);
    if (m->msg)
        fprintf(fp, "msg\n%sendmsg\n", m->msg);
    if (m->maplore)
        fprintf(fp, "maplore\n%sendmaplore\n", m->maplore);
    if (m->unique)
        fprintf(fp, "unique %d\n", m->unique);
    if (m->is_template)
        fprintf(fp, "template %d\n", m->is_template);
    if (m->outdoor)
        fprintf(fp, "outdoor %d\n", m->outdoor);
    if (m->nosmooth)
        fprintf(fp, "nosmooth %d\n", m->nosmooth);
    if (m->last_reset_time.tv_sec)
        fprintf(fp, "first_load %d\n", (int)m->last_reset_time.tv_sec);
    if (m->background_music)
        fprintf(fp, "background_music %s\n", m->background_music);

    /* Save any tiling information, except on overlays */
    if (flag != SAVE_MODE_OVERLAY)
        for (i = 0; i < 4; i++)
            if (m->tile_path[i])
                fprintf(fp, "tile_path_%d %s\n", i+1, m->tile_path[i]);

    fprintf(fp, "end\n");

    /* In the game save unique items in the different file, but
     * in the editor save them to the normal map file.
     * If unique map, save files in the proper destination (set by
     * player)
     */
    if ((flag == SAVE_MODE_NORMAL || flag == SAVE_MODE_OVERLAY) && !m->unique && !m->is_template) {
        char name[MAX_BUF], final_unique[MAX_BUF];

        create_items_path(m->path, name, MAX_BUF);
        snprintf(final_unique, sizeof(final_unique), "%s.v00", name);
        snprintf(buf, sizeof(buf), "%s%s", final_unique, TEMP_EXT);
        if ((fp2 = fopen(buf, "w")) == NULL) {
            LOG(llevError, "Can't open unique items file %s\n", buf);
            fclose(fp);
            return SAVE_ERROR_UCREATION;
        }
        if (flag == SAVE_MODE_OVERLAY) {
            /* SO_FLAG_NO_REMOVE is non destructive save, so map is still valid. */
            res = save_objects(m, fp, fp2, SAVE_FLAG_NO_REMOVE);
            if (res < 0) {
                LOG(llevError, "Save error during object save: %d\n", res);
                fclose(fp);
                fclose(fp2);
                return res;
            }
            m->in_memory = MAP_IN_MEMORY;
        } else {
            res = save_objects(m, fp, fp2, 0);
            if (res < 0) {
                LOG(llevError, "Save error during object save: %d\n", res);
                fclose(fp);
                fclose(fp2);
                return res;
            }
            free_all_objects(m);
        }
        if (ftell(fp2) == 0) {
            fclose(fp2);
            unlink(buf);
            /* If there are no unique items left on the map, we need to
             * unlink the original unique map so that the unique
             * items don't show up again.
             */
            unlink(final_unique);
        } else {
            fflush(fp2);
            fclose(fp2);
            unlink(final_unique); /* failure isn't too bad, maybe the file doesn't exist. */
            if (rename(buf, final_unique) == -1) {
                LOG(llevError, "Couldn't rename unique file %s to %s\n", buf, final_unique);
                fclose(fp);
                return SAVE_ERROR_URENAME;
            }
            chmod(final_unique, SAVE_MODE);
        }
    } else { /* save same file when not playing, like in editor */
        res = save_objects(m, fp, fp, 0);
        if (res < 0) {
            LOG(llevError, "Save error during object save: %d\n", res);
            fclose(fp);
            return res;
        }
        free_all_objects(m);
    }

    fflush(fp);
    if (fclose(fp) != 0) {
        LOG(llevError, "fclose error!\n");
        return SAVE_ERROR_CLOSE;
    }
    unlink(final); /* failure isn't too bad, maybe the file doesn't exist. */
    if (rename(filename, final) == -1) {
        LOG(llevError, "Couldn't rename regular file %s to %s\n", filename, final);
        return SAVE_ERROR_RRENAME;
    }

    chmod(final, SAVE_MODE);
    return SAVE_ERROR_OK;
}

/**
 * Remove and free all objects in the inventory of the given object.
 *
 * @param op
 * object to clean.
 *
 * @todo
 * move to common/object.c ?
 */
void clean_object(object *op) {
    FOR_INV_PREPARE(op, tmp) {
        clean_object(tmp);
        if (QUERY_FLAG(tmp, FLAG_IS_LINKED))
            remove_button_link(tmp);
        object_remove(tmp);
        object_free_drop_inventory(tmp);
    } FOR_INV_FINISH();
}

/**
 * Remove and free all objects in the given map.
 *
 * @param m
 * map to free.
 */
static void free_all_objects(mapstruct *m) {
    int i, j;
    object *op;

    for (i = 0; i < MAP_WIDTH(m); i++)
        for (j = 0; j < MAP_HEIGHT(m); j++) {
            object *previous_obj = NULL;

            while ((op = GET_MAP_OB(m, i, j)) != NULL) {
                if (op == previous_obj) {
                    LOG(llevDebug, "free_all_objects: Link error, bailing out.\n");
                    break;
                }
                previous_obj = op;
                op = HEAD(op);

                /* If the map isn't in memory, object_free_drop_inventory() will remove and
                * free objects in op's inventory.  So let it do the job.
                */
                if (m->in_memory == MAP_IN_MEMORY)
                    clean_object(op);
                object_remove(op);
                object_free_drop_inventory(op);
            }
        }
#ifdef MANY_CORES
    /* I see periodic cores on metalforge where a map has been swapped out, but apparantly
     * an item on that map was not saved - look for that condition and die as appropriate -
     * this leaves more of the map data intact for better debugging.
     */
    for (op = objects; op != NULL; op = op->next) {
        if (!QUERY_FLAG(op, FLAG_REMOVED) && op->map == m) {
            LOG(llevDebug, "free_all_objects: object %s still on map after it should have been freed\n", op->name);
            abort();
        }
    }
#endif
}

/**
 * Frees everything allocated by the given mapstructure.
 * Don't free tmpname - our caller is left to do that.
 * Mapstructure itself is not freed.
 *
 * @param m
 * map to free.
 */
void free_map(mapstruct *m) {
    int i;

    if (!m->in_memory) {
        LOG(llevError, "Trying to free freed map.\n");
        return;
    }

    /* Handle for plugin map unload event. */
    execute_global_event(EVENT_MAPUNLOAD, m);

    if (m->spaces)
        free_all_objects(m);
    if (m->name)
        FREE_AND_CLEAR(m->name);
    if (m->spaces)
        FREE_AND_CLEAR(m->spaces);
    if (m->msg)
        FREE_AND_CLEAR(m->msg);
    if (m->maplore)
        FREE_AND_CLEAR(m->maplore);
    if (m->shopitems)
        FREE_AND_CLEAR(m->shopitems);
    if (m->shoprace)
        FREE_AND_CLEAR(m->shoprace);
    if (m->background_music)
        FREE_AND_CLEAR(m->background_music);
    if (m->buttons)
        free_objectlinkpt(m->buttons);
    m->buttons = NULL;
    for (i = 0; i < 4; i++) {
        if (m->tile_path[i])
            FREE_AND_CLEAR(m->tile_path[i]);
        m->tile_map[i] = NULL;
    }
    m->in_memory = MAP_SWAPPED;
}

/**
 * Frees the map, including the mapstruct.
 *
 * This deletes all the data on the map (freeing pointers)
 * and then removes this map from the global linked list of maps.
 *
 * @param m
 * pointer to mapstruct, if NULL no action. Will be invalid after this function.
 */
void delete_map(mapstruct *m) {
    mapstruct *tmp, *last;
    int i;

    if (!m)
        return;
    if (m->in_memory == MAP_IN_MEMORY) {
        /* change to MAP_SAVING, even though we are not,
         * so that object_remove() doesn't do as much work.
         */
        m->in_memory = MAP_SAVING;
        free_map(m);
    }
    /* move this out of free_map, since tmpname can still be needed if
     * the map is swapped out.
     */
    free(m->tmpname);
    m->tmpname = NULL;
    last = NULL;
    /* We need to look through all the maps and see if any maps
     * are pointing at this one for tiling information.  Since
     * tiling can be assymetric, we just can not look to see which
     * maps this map tiles with and clears those.
     */
    for (tmp = first_map; tmp != NULL; tmp = tmp->next) {
        if (tmp->next == m)
            last = tmp;

        /* This should hopefully get unrolled on a decent compiler */
        for (i = 0; i < 4; i++)
            if (tmp->tile_map[i] == m)
                tmp->tile_map[i] = NULL;
    }

    /* If last is null, then this should be the first map in the list */
    if (!last) {
        if (m == first_map)
            first_map = m->next;
        else
            /* m->path is a static char, so should hopefully still have
            * some useful data in it.
            */
            LOG(llevError, "delete_map: Unable to find map %s in list\n", m->path);
    } else
        last->next = m->next;

    free(m);
}

/**
 * Makes sure the given map is loaded and swapped in.
 * @param name
 * path name of the map.
 * @param flags
 * @li 0x1 (::MAP_FLUSH): flush the map - always load from the map directory,
 *  and don't do unique items or the like.
 * @li 0x2 (::MAP_PLAYER_UNIQUE) - this is a unique map for each player.
 *  dont do any more name translation on it.
 *
 * @return
 * pointer to the given map, NULL on failure.
 */
mapstruct *ready_map_name(const char *name, int flags) {
    mapstruct *m;

    if (!name)
        return (NULL);

    /* Have we been at this level before? */
    m = has_been_loaded(name);

    /* Map is good to go, so just return it */
    if (m && (m->in_memory == MAP_LOADING || m->in_memory == MAP_IN_MEMORY)) {
        return m;
    }

    /* unique maps always get loaded from their original location, and never
     * a temp location.  Likewise, if map_flush is set, or we have never loaded
     * this map, load it now.  I removed the reset checking from here -
     * it seems the probability of a player trying to enter a map that should
     * reset but hasn't yet is quite low, and removing that makes this function
     * a bit cleaner (and players probably shouldn't rely on exact timing for
     * resets in any case - if they really care, they should use the 'maps command.
     */
    if ((flags&(MAP_FLUSH|MAP_PLAYER_UNIQUE)) || !m) {
        /* first visit or time to reset */
        if (m) {
            clean_tmp_map(m); /* Doesn't make much difference */
            delete_map(m);
        }

        /* create and load a map */
        if (flags&MAP_PLAYER_UNIQUE)
            LOG(llevDebug, "Trying to load map %s.\n", name);
        else {
            char fullpath[MAX_BUF];

            create_pathname(name, fullpath, MAX_BUF);
            LOG(llevDebug, "Trying to load map %s.\n", fullpath);
        }

        if (!(m = load_original_map(name, (flags&MAP_PLAYER_UNIQUE))))
            return (NULL);

        /* If a player unique map, no extra unique object file to load.
         * if from the editor, likewise.
         */
        if (!(flags&(MAP_FLUSH|MAP_PLAYER_UNIQUE)))
            load_unique_objects(m);

        if (!(flags&(MAP_FLUSH|MAP_PLAYER_UNIQUE|MAP_OVERLAY))) {
            if (load_overlay_map(name, m) != 0) {
                delete_map(m);
                m = load_original_map(name, 0);
                if (m == NULL) {
                    /* Really, this map is bad :( */
                    return NULL;
                }
            }
        }
    } else {
        /* If in this loop, we found a temporary map, so load it up. */

        if (load_temporary_map(m) != 0) {
            /*
             * There was a failure loading the temporary map, fall back to original one.
             * load_temporary_map() already logged the error.
             */
            delete_map(m);
            m = load_original_map(name, 0);
            if (m == NULL) {
                /* Really, this map is bad :( */
                return NULL;
            }
        }
        load_unique_objects(m);

        clean_tmp_map(m);
        m->in_memory = MAP_IN_MEMORY;
        /* tempnam() on sun systems (probably others) uses malloc
        * to allocated space for the string.  Free it here.
        * In some cases, load_temporary_map above won't find the
        * temporary map, and so has reloaded a new map.  If that
        * is the case, tmpname is now null
        */
        free(m->tmpname);
        m->tmpname = NULL;
        /* It's going to be saved anew anyway */
    }

    /* Below here is stuff common to both first time loaded maps and
     * temp maps.
     */

    decay_objects(m); /* start the decay */

    if (m->outdoor)
        set_darkness_map(m);
    if (!(flags&(MAP_FLUSH))) {
        if (m->last_reset_time.tv_sec == 0)
            gettimeofday(&(m->last_reset_time), NULL);
    }
    return m;
}

/**
 * This routine is supposed to find out the difficulty of the map.
 * Difficulty does not have a lot to do with character level,
 * but does have a lot to do with treasure on the map.
 *
 * Difficulty can now be set by the map creature.  If the value stored
 * in the map is zero, then use this routine.  Maps should really
 * have a difficulty set than using this function - human calculation
 * is much better than this functions guesswork.
 *
 * @param m
 * map for which to compute difficulty.
 * @return
 * difficulty of the map.
 */
int calculate_difficulty(mapstruct *m) {
    archetype *at;
    int x, y;
    int diff = 0;
    int i;
    sint64 exp_pr_sq, total_exp = 0;

    if (MAP_DIFFICULTY(m)) {
        return MAP_DIFFICULTY(m);
    }

    for (x = 0; x < MAP_WIDTH(m); x++)
        for (y = 0; y < MAP_HEIGHT(m); y++)
            FOR_MAP_PREPARE(m, x, y, op) {
                if (QUERY_FLAG(op, FLAG_MONSTER))
                    total_exp += op->stats.exp;
                if (QUERY_FLAG(op, FLAG_GENERATOR)) {
                    total_exp += op->stats.exp;
                    at = get_archetype_by_type_subtype(GENERATE_TYPE(op), -1);
                    if (at != NULL)
                        total_exp += at->clone.stats.exp*8;
                }
            } FOR_MAP_FINISH();

    exp_pr_sq = ((double)1000*total_exp)/(MAP_WIDTH(m)*MAP_HEIGHT(m)+1);
    diff = 20;
    for (i = 1; i < 20; i++)
        if (exp_pr_sq <= level_exp(i, 1.0)) {
            diff = i;
            break;
        }

    return diff;
}

/**
 * Removse the temporary file used by the map.
 *
 * @param m
 * map, which mustn't be NULL but can have no temporary file set.
 */
void clean_tmp_map(mapstruct *m) {
    if (m->tmpname == NULL)
        return;
    (void)unlink(m->tmpname);
}

/**
 * Frees all allocated maps.
 */
void free_all_maps(void) {
    int real_maps = 0;

    while (first_map) {
        /* I think some of the callers above before it gets here set this to be
         * saving, but we still want to free this data
         */
        if (first_map->in_memory == MAP_SAVING)
            first_map->in_memory = MAP_IN_MEMORY;
        delete_map(first_map);
        real_maps++;
    }
    LOG(llevDebug, "free_all_maps: Freed %d maps\n", real_maps);
}

/**
 * Used to change map light level (darkness)
 * up or down. It should now be possible to change a value by more than 1.
 *
 * Move this from los.c to map.c since this is more related
 * to maps than los.
 * postive values make it darker, negative make it brighter
 *
 * Will inform players on the map.
 *
 * @param m
 * map to change.
 * @param change
 * delta of light.
 * @return
 * TRUE if light changed, FALSE else.
 */
int change_map_light(mapstruct *m, int change) {
    int new_level = m->darkness+change;

    /* Nothing to do */
    if (!change
    || (new_level <= 0 && m->darkness == 0)
    || (new_level >= MAX_DARKNESS && m->darkness >= MAX_DARKNESS)) {
        return 0;
    }

    /* inform all players on the map */
    if (change > 0)
        ext_info_map(NDI_BLACK, m, MSG_TYPE_MISC, MSG_SUBTYPE_NONE, "It becomes darker.");
    else
        ext_info_map(NDI_BLACK, m, MSG_TYPE_MISC, MSG_SUBTYPE_NONE, "It becomes brighter.");

    /* Do extra checking.  since m->darkness is a unsigned value,
     * we need to be extra careful about negative values.
     * In general, the checks below are only needed if change
     * is not +/-1
     */
    if (new_level < 0)
        m->darkness = 0;
    else if (new_level >= MAX_DARKNESS)
        m->darkness = MAX_DARKNESS;
    else
        m->darkness = new_level;

    /* All clients need to get re-updated for the change */
    update_all_map_los(m);
    return 1;
}

/**
 * This function is used for things that can have multiple
 * layers - NO_PICK, ITEM, LIVING, FLYING.
 * Basically, we want to store in the empty spot,
 * and if both full, store highest visiblity objects.
 * Since update_position() goes from bottom to top order,
 * if the new object is equal to existing we take the new
 * object since it is higher in the stack.
 * @param low_layer
 * lower bounds to check (inclusive).
 * @param high_layer
 * upper bounds to check (inclusive).
 * @param ob
 * object to add to the layer.
 * @param layers
 * layers to change.
 * @param honor_visibility
 * if it is set to 0,then we do a pure stacking logic - this is used
 * for the no pick layer, since stacking ordering there
 * is basically fixed - don't want to re-order walls,
 * pentagrams, etc.
 */
static inline void add_face_layer(int low_layer, int high_layer, object *ob, object *layers[], int honor_visibility) {
    int l, l1;
    object *tmp;

    for (l = low_layer; l <= high_layer; l++) {
        if (!layers[l]) {
            /* found an empty spot.  now, we want to make sure
             * highest visibility at top, etc.
             */
            layers[l] = ob;
            if (!honor_visibility)
                return;

            /* This is basically a mini bubble sort.  Only swap
             * position if the lower face has greater (not equal)
             * visibility - map stacking is secondary consideration here.
             */
            for (l1 = (l-1); l1 >= low_layer; l1--) {
                if (layers[l1]->face->visibility > layers[l1+1]->face->visibility) {
                    tmp = layers[l1+1];
                    layers[l1+1] = layers[l1];
                    layers[l1] = tmp;
                }
            }
            /* Nothing more to do - face inserted */
            return;
        }
    }
    /* If we get here, all the layers have an object..
     */
    if (!honor_visibility) {
        /* Basically, in this case, it is pure stacking logic, so
        * new object goes on the top.
        */
        for (l = low_layer; l < high_layer; l++)
            layers[l] = layers[l+1];
        layers[high_layer] = ob;
    /* If this object doesn't have higher visibility than
     * the lowest object, no reason to go further.
     */
    } else if (ob->face->visibility >= layers[low_layer]->face->visibility) {
        /*
         * Start at the top (highest visibility) layer and work down.
         * once this face exceed that of the layer, push down those
         * other layers, and then replace the layer with our object.
         */
        for (l = high_layer; l >= low_layer; l--) {
            if (ob->face->visibility >= layers[l]->face->visibility) {
                for (l1 = low_layer; l1 < l; l1++)
                    layers[l1] = layers[l1+1];
                layers[l] = ob;
                break;
            }
        }
    }
}

/**
 * This function updates various attributes about a specific space
 * on the map (what it looks like, whether it blocks magic,
 * has a living creatures, prevents people from passing
 * through, etc)
 *
 * @param m
 * map considered
 * @param x
 * @param y
 * coordinates to update
 */
void update_position(mapstruct *m, int x, int y) {
    object *player = NULL;
    uint8 flags = 0, oldflags, light = 0;
    object *layers[MAP_LAYERS];

    MoveType move_block = 0, move_slow = 0, move_on = 0, move_off = 0, move_allow = 0;

    oldflags = GET_MAP_FLAGS(m, x, y);
    if (!(oldflags&P_NEED_UPDATE)) {
        LOG(llevDebug, "update_position called with P_NEED_UPDATE not set: %s (%d, %d)\n", m->path, x, y);
        return;
    }

    memset(layers, 0, MAP_LAYERS*sizeof(object *));

    FOR_MAP_PREPARE(m, x, y, tmp) {
        /* DMs just don't do anything when hidden, including no light. */
        if (QUERY_FLAG(tmp, FLAG_WIZ) && tmp->contr->hidden)
            continue;

        if (tmp->type == PLAYER)
            player = tmp;

        /* This could be made additive I guess (two lights better than
         * one).  But if so, it shouldn't be a simple additive - 2
         * light bulbs do not illuminate twice as far as once since
         * it is a dissipation factor that is squared (or is it cubed?)
         */
        if (tmp->glow_radius > light)
            light = tmp->glow_radius;

        /* if this object is visible and not a blank face,
        * update the objects that show how this space
        * looks.
        */
        if (!tmp->invisible && tmp->face != blank_face) {
            if (tmp->map_layer) {
                add_face_layer(tmp->map_layer, map_layer_info[tmp->map_layer].high_layer,
                               tmp, layers, map_layer_info[tmp->map_layer].honor_visibility);
            } else if (tmp->move_type&MOVE_FLYING) {
                add_face_layer(MAP_LAYER_FLY1, MAP_LAYER_FLY2, tmp, layers, 1);
            } else if ((tmp->type == PLAYER || QUERY_FLAG(tmp, FLAG_MONSTER))) {
                add_face_layer(MAP_LAYER_LIVING1, MAP_LAYER_LIVING2, tmp, layers, 1);
            } else if (QUERY_FLAG(tmp, FLAG_IS_FLOOR)) {
                layers[MAP_LAYER_FLOOR] = tmp;
                /* floors hide everything else */
                memset(layers+1, 0, (MAP_LAYERS-1)*sizeof(object *));
            /* Check for FLAG_SEE_ANYWHERE is removed - objects
             * with that flag should just have a high visibility
             * set - we shouldn't need special code here.
             */
            } else if (QUERY_FLAG(tmp, FLAG_NO_PICK)) {
                add_face_layer(MAP_LAYER_NO_PICK1, MAP_LAYER_NO_PICK2, tmp, layers, 0);
            } else {
                add_face_layer(MAP_LAYER_ITEM1, MAP_LAYER_ITEM3, tmp, layers, 1);
            }
        }
        if (tmp == tmp->above) {
            LOG(llevError, "Error in structure of map\n");
            exit(-1);
        }

        move_slow |= tmp->move_slow;
        move_block |= tmp->move_block;
        move_on |= tmp->move_on;
        move_off |= tmp->move_off;
        move_allow |= tmp->move_allow;

        if (QUERY_FLAG(tmp, FLAG_ALIVE))
            flags |= P_IS_ALIVE;
        if (QUERY_FLAG(tmp, FLAG_NO_MAGIC))
            flags |= P_NO_MAGIC;
        if (QUERY_FLAG(tmp, FLAG_DAMNED))
            flags |= P_NO_CLERIC;

        if (QUERY_FLAG(tmp, FLAG_BLOCKSVIEW))
            flags |= P_BLOCKSVIEW;
    } FOR_MAP_FINISH(); /* for stack of objects */

    if (player)
        flags |= P_PLAYER;

    /* we don't want to rely on this function to have accurate flags, but
     * since we're already doing the work, we calculate them here.
     * if they don't match, logic is broken someplace.
     */
    if (((oldflags&~(P_NEED_UPDATE|P_NO_ERROR)) != flags)
    && (!(oldflags&P_NO_ERROR))) {
        LOG(llevDebug, "update_position: updated flags do not match old flags: %s (x=%d,y=%d) %x != %x\n",
            m->path, x, y, (oldflags&~P_NEED_UPDATE), flags);
    }

    SET_MAP_FLAGS(m, x, y, flags);
    SET_MAP_MOVE_BLOCK(m, x, y, move_block&~move_allow);
    SET_MAP_MOVE_ON(m, x, y, move_on);
    SET_MAP_MOVE_OFF(m, x, y, move_off);
    SET_MAP_MOVE_SLOW(m, x, y, move_slow);
    SET_MAP_LIGHT(m, x, y, light);

    /* Note that player may be NULL here, which is fine - if no player, need
     * to clear any value that may be set.
     */
    SET_MAP_PLAYER(m, x, y, player);

    /* Note it is intentional we copy everything, including NULL values. */
    memcpy(GET_MAP_FACE_OBJS(m, x, y), layers, sizeof(object *)*MAP_LAYERS);
}

/**
 * Updates the map's timeout.
 *
 * @param map
 * map to update.
 */
void set_map_reset_time(mapstruct *map) {
    int timeout;

    timeout = MAP_RESET_TIMEOUT(map);
    if (timeout <= 0)
        timeout = MAP_DEFAULTRESET;
    if (timeout >= MAP_MAXRESET)
        timeout = MAP_MAXRESET;
    MAP_WHEN_RESET(map) = seconds()+timeout;
}

/**
 * This updates the orig_map->tile_map[tile_num] value after loading
 * the map.  It also takes care of linking back the freshly loaded
 * maps tile_map values if it tiles back to this one.  It returns
 * the value of orig_map->tile_map[tile_num].  It really only does this
 * so that it is easier for calling functions to verify success.
 *
 * @param orig_map
 * map for which we load the tiled map.
 * @param tile_num
 * tile to load. Must be between 0 and 3 inclusive.
 * @return
 * linked map, or NULL on failure.
 *
 * @todo
 * check ready_map_name() 's return value, which can be NULL?
 */
static mapstruct *load_and_link_tiled_map(mapstruct *orig_map, int tile_num) {
    int dest_tile = (tile_num+2)%4;
    char path[HUGE_BUF];

    path_combine_and_normalize(orig_map->path, orig_map->tile_path[tile_num], path, sizeof(path));

    orig_map->tile_map[tile_num] = ready_map_name(path, 0);

    /* need to do a strcmp here as the orig_map->path is not a shared string */
    if (orig_map->tile_map[tile_num]->tile_path[dest_tile]
    && !strcmp(orig_map->tile_map[tile_num]->tile_path[dest_tile], orig_map->path))
        orig_map->tile_map[tile_num]->tile_map[dest_tile] = orig_map;

    return orig_map->tile_map[tile_num];
}

/**
 * this returns TRUE if the coordinates (x,y) are out of
 * map m.  This function also takes into account any
 * tiling considerations, loading adjacant maps as needed.
 * This is the function should always be used when it
 * necessary to check for valid coordinates.
 * This function will recursively call itself for the
 * tiled maps.
 *
 * @param m
 * map to consider.
 * @param x
 * @param y
 * coordinates.
 * @return
 * 1 if out of map, 0 else
 */
int out_of_map(mapstruct *m, int x, int y) {
    /* If we get passed a null map, this is obviously the
     * case.  This generally shouldn't happen, but if the
     * map loads fail below, it could happen.
     */
    if (!m)
        return 0;

    /* Simple case - coordinates are within this local
     * map.
     */
    if (x >= 0 && x < MAP_WIDTH(m) && y >= 0 && y < MAP_HEIGHT(m))
        return 0;

    if (x < 0) {
        if (!m->tile_path[3])
            return 1;
        if (!m->tile_map[3] || m->tile_map[3]->in_memory != MAP_IN_MEMORY) {
            load_and_link_tiled_map(m, 3);
        }
        return (out_of_map(m->tile_map[3], x+MAP_WIDTH(m->tile_map[3]), y));
    }
    if (x >= MAP_WIDTH(m)) {
        if (!m->tile_path[1])
            return 1;
        if (!m->tile_map[1] || m->tile_map[1]->in_memory != MAP_IN_MEMORY) {
            load_and_link_tiled_map(m, 1);
        }
        return (out_of_map(m->tile_map[1], x-MAP_WIDTH(m), y));
    }
    if (y < 0) {
        if (!m->tile_path[0])
            return 1;
        if (!m->tile_map[0] || m->tile_map[0]->in_memory != MAP_IN_MEMORY) {
            load_and_link_tiled_map(m, 0);
        }
        return (out_of_map(m->tile_map[0], x, y+MAP_HEIGHT(m->tile_map[0])));
    }
    if (y >= MAP_HEIGHT(m)) {
        if (!m->tile_path[2])
            return 1;
        if (!m->tile_map[2] || m->tile_map[2]->in_memory != MAP_IN_MEMORY) {
            load_and_link_tiled_map(m, 2);
        }
        return (out_of_map(m->tile_map[2], x, y-MAP_HEIGHT(m)));
    }
    return 1;
}

/**
 * This is basically the same as out_of_map above(), but
 * instead we return NULL if no map is valid (coordinates
 * out of bounds and no tiled map), otherwise it returns
 * the map as that the coordinates are really on, and
 * updates x and y to be the localized coordinates.
 * Using this is more efficient than calling out_of_map
 * and then figuring out what the real map is
 *
 * @param m
 * map we want to look at.
 * @param x
 * @param y
 * coordinates, which will contain the real position that was checked.
 * @return
 * map that is at specified location. Will be NULL if not on any map.
 */
mapstruct *get_map_from_coord(mapstruct *m, sint16 *x, sint16 *y) {
    /* m should never be null, but if a tiled map fails to load below, it could
     * happen.
     */
    if (!m)
        return NULL;

    /* Simple case - coordinates are within this local
     * map.
     */

    if (*x >= 0 && *x < MAP_WIDTH(m) && *y >= 0 && *y < MAP_HEIGHT(m))
        return m;

    if (*x < 0) {
        if (!m->tile_path[3])
            return NULL;
        if (!m->tile_map[3] || m->tile_map[3]->in_memory != MAP_IN_MEMORY)
            load_and_link_tiled_map(m, 3);

        *x += MAP_WIDTH(m->tile_map[3]);
        return (get_map_from_coord(m->tile_map[3], x, y));
    }
    if (*x >= MAP_WIDTH(m)) {
        if (!m->tile_path[1])
            return NULL;
        if (!m->tile_map[1] || m->tile_map[1]->in_memory != MAP_IN_MEMORY)
            load_and_link_tiled_map(m, 1);

        *x -= MAP_WIDTH(m);
        return (get_map_from_coord(m->tile_map[1], x, y));
    }
    if (*y < 0) {
        if (!m->tile_path[0])
            return NULL;
        if (!m->tile_map[0] || m->tile_map[0]->in_memory != MAP_IN_MEMORY)
            load_and_link_tiled_map(m, 0);

        *y += MAP_HEIGHT(m->tile_map[0]);
        return (get_map_from_coord(m->tile_map[0], x, y));
    }
    if (*y >= MAP_HEIGHT(m)) {
        if (!m->tile_path[2])
            return NULL;
        if (!m->tile_map[2] || m->tile_map[2]->in_memory != MAP_IN_MEMORY)
            load_and_link_tiled_map(m, 2);

        *y -= MAP_HEIGHT(m);
        return (get_map_from_coord(m->tile_map[2], x, y));
    }
    return NULL;    /* Coordinates aren't valid if we got here */
}

/**
 * Return whether map2 is adjacent to map1. If so, store the distance from
 * map1 to map2 in dx/dy.
 *
 * @param map1
 * @param map2
 * maps to consider.
 * @param dx
 * @param dy
 * distance. Must not be NULL. Not altered if returns 0.
 * @return
 * 1 if maps are adjacent, 0 else.
 */
static int adjacent_map(const mapstruct *map1, const mapstruct *map2, int *dx, int *dy) {
    if (!map1 || !map2)
        return 0;

    if (map1 == map2) {
        *dx = 0;
        *dy = 0;
    } else if (map1->tile_map[0] == map2) { /* up */
        *dx = 0;
        *dy = -MAP_HEIGHT(map2);
    } else if (map1->tile_map[1] == map2) { /* right */
        *dx = MAP_WIDTH(map1);
        *dy = 0;
    } else if (map1->tile_map[2] == map2) { /* down */
        *dx = 0;
        *dy = MAP_HEIGHT(map1);
    } else if (map1->tile_map[3] == map2) { /* left */
        *dx = -MAP_WIDTH(map2);
        *dy = 0;
    } else if (map1->tile_map[0] && map1->tile_map[0]->tile_map[1] == map2) { /* up right */
        *dx = MAP_WIDTH(map1->tile_map[0]);
        *dy = -MAP_HEIGHT(map1->tile_map[0]);
    } else if (map1->tile_map[0] && map1->tile_map[0]->tile_map[3] == map2) { /* up left */
        *dx = -MAP_WIDTH(map2);
        *dy = -MAP_HEIGHT(map1->tile_map[0]);
    } else if (map1->tile_map[1] && map1->tile_map[1]->tile_map[0] == map2) { /* right up */
        *dx = MAP_WIDTH(map1);
        *dy = -MAP_HEIGHT(map2);
    } else if (map1->tile_map[1] && map1->tile_map[1]->tile_map[2] == map2) { /* right down */
        *dx = MAP_WIDTH(map1);
        *dy = MAP_HEIGHT(map1->tile_map[1]);
    } else if (map1->tile_map[2] && map1->tile_map[2]->tile_map[1] == map2) { /* down right */
        *dx = MAP_WIDTH(map1->tile_map[2]);
        *dy = MAP_HEIGHT(map1);
    } else if (map1->tile_map[2] && map1->tile_map[2]->tile_map[3] == map2) { /* down left */
        *dx = -MAP_WIDTH(map2);
        *dy = MAP_HEIGHT(map1);
    } else if (map1->tile_map[3] && map1->tile_map[3]->tile_map[0] == map2) { /* left up */
        *dx = -MAP_WIDTH(map1->tile_map[3]);
        *dy = -MAP_HEIGHT(map2);
    } else if (map1->tile_map[3] && map1->tile_map[3]->tile_map[2] == map2) { /* left down */
        *dx = -MAP_WIDTH(map1->tile_map[3]);
        *dy = MAP_HEIGHT(map1->tile_map[3]);
    } else { /* not "adjacent" enough */
        return 0;
    }

    return 1;
}

/**
 * From map.c
 * This is used by get_player to determine where the other
 * creature is.  get_rangevector takes into account map tiling,
 * so you just can not look the the map coordinates and get the
 * right value.  distance_x/y are distance away, which
 * can be negative.  direction is the crossfire direction scheme
 * that the creature should head.  part is the part of the
 * monster that is closest.
 *
 * get_rangevector looks at op1 and op2, and fills in the
 * structure for op1 to get to op2.
 * We already trust that the caller has verified that the
 * two objects are at least on adjacent maps.  If not,
 * results are not likely to be what is desired.
 *
 * @param op1
 * object which wants to go to op2's location.
 * @param op2
 * target of op1.
 * @param retval
 * vector for op1 to go to op2.
 * @param flags
 * if 1, don't translate for closest body part of 'op1'
 * @return
 * 1=ok; 0=the objects are not on the same map
 */
int get_rangevector(object *op1, const object *op2, rv_vector *retval, int flags) {
    if (!adjacent_map(op1->map, op2->map, &retval->distance_x, &retval->distance_y)) {
        /* be conservative and fill in _some_ data */
        retval->distance = 100000;
        retval->distance_x = 32767;
        retval->distance_y = 32767;
        retval->direction = 0;
        retval->part = NULL;
        return 0;
    } else {
        object *best;

        retval->distance_x += op2->x-op1->x;
        retval->distance_y += op2->y-op1->y;

        best = op1;
        /* If this is multipart, find the closest part now */
        if (!(flags&0x1) && op1->more) {
            object *tmp;
            int best_distance = retval->distance_x*retval->distance_x+
                                retval->distance_y*retval->distance_y, tmpi;

            /* we just take the offset of the piece to head to figure
             * distance instead of doing all that work above again
             * since the distance fields we set above are positive in the
             * same axis as is used for multipart objects, the simply arithmetic
             * below works.
             */
            for (tmp = op1->more; tmp != NULL; tmp = tmp->more) {
                tmpi = (op1->x-tmp->x+retval->distance_x)*(op1->x-tmp->x+retval->distance_x)+
                       (op1->y-tmp->y+retval->distance_y)*(op1->y-tmp->y+retval->distance_y);
                if (tmpi < best_distance) {
                    best_distance = tmpi;
                    best = tmp;
                }
            }
            if (best != op1) {
                retval->distance_x += op1->x-best->x;
                retval->distance_y += op1->y-best->y;
            }
        }
        retval->part = best;
        retval->distance = isqrt(retval->distance_x*retval->distance_x+retval->distance_y*retval->distance_y);
        retval->direction = find_dir_2(-retval->distance_x, -retval->distance_y);
        return 1;
    }
}

/**
 * This is basically the same as get_rangevector() above, but instead of
 * the first parameter being an object, it instead is the map
 * and x,y coordinates - this is used for path to player -
 * since the object is not infact moving but we are trying to traverse
 * the path, we need this.
 * flags has no meaning for this function at this time - I kept it in to
 * be more consistant with the above function and also in case they are needed
 * for something in the future.  Also, since no object is pasted, the best
 * field of the rv_vector is set to NULL.
 *
 * @param m
 * map to consider.
 * @param x
 * @param y
 * origin coordinates.
 * @param op2
 * target object.
 * @param retval
 * vector to get to op2.
 * @param flags
 * unused.
 * @return
 * 1=ok; 0=the objects are not on the same map
 */
int get_rangevector_from_mapcoord(const mapstruct *m, int x, int y, const object *op2, rv_vector *retval, int flags) {
    if (!adjacent_map(m, op2->map, &retval->distance_x, &retval->distance_y)) {
        /* be conservative and fill in _some_ data */
        retval->distance = 100000;
        retval->distance_x = 32767;
        retval->distance_y = 32767;
        retval->direction = 0;
        retval->part = NULL;
        return 0;
    } else {
        retval->distance_x += op2->x-x;
        retval->distance_y += op2->y-y;

        retval->part = NULL;
        retval->distance = isqrt(retval->distance_x*retval->distance_x+retval->distance_y*retval->distance_y);
        retval->direction = find_dir_2(-retval->distance_x, -retval->distance_y);
        return 1;
    }
}

/**
 * Checks whether 2 objects are on the same map or not.
 *
 * Note we only look one map out to keep the processing simple
 * and efficient.  This could probably be a macro.
 * MSW 2001-08-05
 *
 * @param op1
 * first object.
 * @param op2
 * second object.
 * @return
 * TRUE if op1 and op2 are effectively on the same map (as related to map tiling).
 *
 * @note
 * This looks for a path from op1 to op2, so if the tiled maps are assymetric and op2 has a path
 * to op1, this will still return false.
 */
int on_same_map(const object *op1, const object *op2) {
    int dx, dy;

    return adjacent_map(op1->map, op2->map, &dx, &dy);
}

/**
 * Finds an object in a map tile by flag number. Checks the objects' heads.
 *
 * @param map
 * the map to search.
 * @param x
 * the x-coordiate to search.
 * @param y
 * the y-coordiate to search.
 * @param flag
 * the flag to seacrh for
 * @return
 * first object's head in the tile that has the flag set. NULL if no match.
 *
 * @note
 * will not search in inventory of objects.
 */
object *map_find_by_flag(mapstruct *map, int x, int y, int flag) {
    object *tmp;

    for (tmp = GET_MAP_OB(map, x, y); tmp != NULL; tmp = tmp->above) {
        object *head;

        head = HEAD(tmp);
        if (QUERY_FLAG(head, flag))
            return head;
    }
    return NULL;
}

/**
 * Remove files containing the map's unique items.
 * @param map
 */
void map_remove_unique_files(const mapstruct *map) {
    char base[HUGE_BUF], path[HUGE_BUF];
    int count;

    if (map->unique) {
        /* Unique maps have their full path already set. */
        unlink(map->path);
        return;
    }

    create_items_path(map->path, base, sizeof(base));

    for (count = 0; count < 10; count++) {
        snprintf(path, sizeof(path), "%s.v%02d", base, count);
        unlink(path);
    }
}

/**
 * Return the map path on which the specified item is.
 * @param item what to return the map path for.
 * @return path, map's name, or error string, never NULL.
 */
const char *map_get_path(const object *item) {
    if (item->map != NULL) {
        if (item->map->path != NULL)
            return item->map->path;
        return item->map->name ? item->map->name : "(empty path and name)";
    }

    if (item->env != NULL)
        return map_get_path(item->env);

    return "(no map and no env!)";
}
