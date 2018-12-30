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
 * This file handles all ingame construction functions: builders, destroyers, building materials.
 *
 * Basically, those enable a player to alter in real-time a map.
 *
 * @todo document building, forces used to store connection values, ...
 */

#include <global.h>
#include <living.h>
#include <spells.h>
#include <skills.h>
#include <tod.h>
#include <sproto.h>

/**
 * Check if objects on a square interfere with building.
 *
 * @param map
 * map we're building on.
 * @param new_item
 * item the player is trying to build.
 * @param x
 * @param y
 * coordinates where to build.
 * @return
 * 0 if tmp can't be built on the spot, 1 if it can be built.
 */
static int can_build_over(struct mapdef *map, object *new_item, short x, short y) {
    FOR_MAP_PREPARE(map, x, y, tmp) {
        object *ob;

        ob = HEAD(tmp);
        if (strcmp(ob->arch->name, "rune_mark") == 0)
            /* you can always build on marking runes, used for connected building things. */
            continue;

        if (QUERY_FLAG(ob, FLAG_IS_BUILDABLE))
            /* Check for the flag is required, as this function
             * can be called recursively on different spots.
             */
            continue;

        switch (new_item->type) {
        case SIGN:
        case MAGIC_EAR:
            /* Allow signs and magic ears to be built on books */
            if (ob->type != BOOK)
                return 0;
            break;

        case BUTTON:
        case DETECTOR:
        case PEDESTAL:
        case CF_HANDLE:
            /* Allow buttons and levers to be built under gates */
            if (ob->type != GATE && ob->type != DOOR)
                return 0;
            break;

        default:
            return 0;
        }
    } FOR_MAP_FINISH();

    /* If item being built is multi-tile, need to check other parts too. */
    if (new_item->more)
        return can_build_over(map, new_item->more, x+new_item->more->arch->clone.x-new_item->arch->clone.x, y+new_item->more->arch->clone.y-new_item->arch->clone.y);

    return 1;
}

/**
 * Returns the marking rune on the square, for purposes of building connections.
 *
 * @param pl
 * player trying to build.
 * @param x
 * @param y
 * coordinates to search.
 * @return
 * marking rune, NULL if none found.
 */
static object *get_connection_rune(object *pl, short x, short y) {
    FOR_MAP_PREPARE(pl->map, x, y, rune)
        if (rune->type == SIGN && strcmp(rune->arch->name, "rune_mark") == 0)
            return rune;
    FOR_MAP_FINISH();
    return NULL;
}

/**
 * Returns the book/scroll on the current square, for purposes of building
 *
 * @param pl
 * player trying to build.
 * @param x
 * @param y
 * coordinates to search.
 * @return
 * book, NULL if none found.
 */
static object *get_msg_book(object *pl, short x, short y) {
    FOR_MAP_PREPARE(pl->map, x, y, book)
        if (book->type == BOOK)
            return book;
    FOR_MAP_FINISH();
    return NULL;
}

/**
 * Returns first item of type WALL.
 *
 * @param map
 * @param x
 * @param y
 * where to search.
 * @return
 * wall, or NULL if none found.
 * @todo isn't there a similar function somewhere? put this in a common library?
 * investigate possible merge with retrofit_joined_wall() used for random maps
 */
static object *get_wall(struct mapdef *map, int x, int y) {
    FOR_MAP_PREPARE(map, x, y, wall)
        if (wall->type == WALL)
            return wall;
    FOR_MAP_FINISH();
    return NULL;
}

/**
 * Erases all marking runes at specified location (before building a wall)
 *
 * @param map
 * @param x
 * @param y
 * coordinates to erase runes at.
 */
static void remove_marking_runes(struct mapdef *map, short x, short y) {
    FOR_MAP_PREPARE(map, x, y, rune) {
        if ((rune->type == SIGN) && (!strcmp(rune->arch->name, "rune_mark"))) {
            object_remove(rune);
            object_free_drop_inventory(rune);
        }
    } FOR_MAP_FINISH();
}

/**
 * Make the built object inherit the msg of books that are used with it.
 * For objects already invisible (i.e. magic mouths & ears), also make it
 * it inherit the face and the name with "talking " prepended.
 *
 * The book is removed during the operation.
 *
 * @param pl
 * player building.
 * @param x
 * @param y
 * building coordinates.
 * @param tmp
 * object that is being built.
 * @return
 * -1 if no text found, 0 if ok to build.
 */
static int adjust_sign_msg(object *pl, short x, short y, object *tmp) {
    object *book;
    char buf[MAX_BUF];
    char buf2[MAX_BUF];

    book = get_msg_book(pl, x, y);
    if (!book) {
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_BUILD,
                      "You need to put a book or scroll with the message.");
        return -1;
    }

    object_set_msg(tmp, book->msg);

    if (tmp->invisible) {
        if (book->custom_name != NULL) {
            snprintf(buf, sizeof(buf), "talking %s", book->custom_name);
        } else {
            snprintf(buf, sizeof(buf), "talking %s", book->name);
        }
        if (tmp->name)
            free_string(tmp->name);
        tmp->name = add_string(buf);

        if (book->name_pl != NULL) {
            snprintf(buf2, sizeof(buf2), "talking %s", book->name_pl);
            if (tmp->name_pl)
                free_string(tmp->name_pl);
            tmp->name_pl = add_string(buf2);
        }

        tmp->face = book->face;
        tmp->invisible = 0;
    }
    object_remove(book);
    object_free_drop_inventory(book);
    return 0;
}

/**
 * Returns an unused value for 'connected'.
 *
 * Tries 1000 random values, then returns -1.
 *
 * @param map
 * map for which to find a value
 * @return
 * 'connected' value with no item, or -1 if failure.
 */
static int find_unused_connected_value(struct mapdef *map) {
    int connected = 0;
    int itest = 0;
    oblinkpt *obp;

    while (itest++ < 1000) {
        connected = 1+rand()%20000;
        for (obp = map->buttons; obp && (obp->value != connected); obp = obp->next)
            ;

        if (!obp)
            return connected;
    }

    return -1;
}


/**
 * Helper function for door/button/connected item building.
 *
 * Will search the specified spot for a marking rune.
 * If not found, returns -1
 * Else, searches a force in op's inventory matching the map's name
 * and the rune's text.
 * If found, returns the connection value associated
 * else searches a new connection value, and adds the force to the player.
 *
 * @param pl
 * player building.
 * @param x
 * @param y
 * coordinates where to build.
 * @param rune
 * rune used to indicate the connection value. If NULL, building is searched for one.
 * @return
 * -1 for failure, else connection value.
 */
static int find_or_create_connection_for_map(object *pl, short x, short y, object *rune) {
    object *force;
    int connected;

    if (!rune)
        rune = get_connection_rune(pl, x, y);

    if (!rune || !rune->msg) {
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_BUILD,
                      "You need to put a marking rune with the group name.");
        return -1;
    }

    /* Now, find force in player's inventory */
    force = NULL;
    FOR_INV_PREPARE(pl, tmp) {
        if (tmp->type == FORCE
        && tmp->slaying != NULL && strcmp(tmp->slaying, pl->map->path) == 0
        && tmp->msg != NULL && tmp->msg == rune->msg) {
            force = tmp;
            break;
        }
    } FOR_INV_FINISH();

    if (!force) {
        /* No force, need to create & insert one */
        /* Find unused value */
        connected = find_unused_connected_value(pl->map);
        if (connected == -1) {
            draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_BUILD,
                          "Could not create more groups.");
            return -1;
        }

        force = create_archetype(FORCE_NAME);
        force->speed = 0;
        object_update_speed(force);
        force->slaying = add_string(pl->map->path);
        object_set_msg(force, rune->msg);
        force->path_attuned = connected;
        object_insert_in_ob(force, pl);

        return connected;
    }

    /* Found the force, everything's easy. */
    return force->path_attuned;
}

/**
 * Fixes walls around specified spot
 *
 * Basically it ensures the correct wall is put where needed.
 *
 * @note
 * x & y must be valid map coordinates.
 *
 * @param map
 * @param x
 * @param y
 * position to fix.
 * @todo
 * investigate possible merge with retrofit_joined_wall() used for random maps
 */
static void fix_walls(struct mapdef *map, int x, int y) {
    int connect;
    object *wall;
    char archetype[MAX_BUF];
    char *underscore;
    uint32 old_flags[4];
    struct archt *new_arch;
    int flag;
    int len;
    int has_window;

    /* First, find the wall on that spot */
    wall = get_wall(map, x, y);
    if (!wall)
        /* Nothing -> bail out */
        return;

    /* Find base name */
    strncpy(archetype, wall->arch->name, sizeof(archetype));
    archetype[sizeof(archetype)-1] = '\0';
    underscore = strchr(archetype, '_');
    if (!underscore)
        /* Not in a format we can change, bail out */
        return;
    has_window = 0;
    if (!strcmp(underscore+1, "win1"))
        has_window = 1;
    else if (!strcmp(underscore+1, "win2"))
        has_window = 1;
    else if (!isdigit(*(underscore+1)))
        return;

    underscore++;
    *underscore = '\0';
    len = sizeof(archetype)-strlen(archetype)-2;

    connect = 0;

    if ((x > 0) && get_wall(map, x-1, y))
        connect |= 1;
    if ((x < MAP_WIDTH(map)-1) && get_wall(map, x+1, y))
        connect |= 2;
    if ((y > 0) && get_wall(map, x, y-1))
        connect |= 4;
    if ((y < MAP_HEIGHT(map)-1) && get_wall(map, x, y+1))
        connect |= 8;

    switch (connect) {
    case 0:
        strncat(archetype, "0", len);
        break;

    case 1:
        strncat(archetype, "1_3", len);
        break;

    case 2:
        strncat(archetype, "1_4", len);
        break;

    case 3:
        if (has_window) {
            strncat(archetype, "win2", len);
        } else {
            strncat(archetype, "2_1_2", len);
        }
        break;

    case 4:
        strncat(archetype, "1_2", len);
        break;

    case 5:
        strncat(archetype, "2_2_4", len);
        break;

    case 6:
        strncat(archetype, "2_2_1", len);
        break;

    case 7:
        strncat(archetype, "3_1", len);
        break;

    case 8:
        strncat(archetype, "1_1", len);
        break;

    case 9:
        strncat(archetype, "2_2_3", len);
        break;

    case 10:
        strncat(archetype, "2_2_2", len);
        break;

    case 11:
        strncat(archetype, "3_3", len);
        break;

    case 12:
        if (has_window) {
            strncat(archetype, "win1", len);
        } else {
            strncat(archetype, "2_1_1", len);
        }
        break;

    case 13:
        strncat(archetype, "3_4", len);
        break;

    case 14:
        strncat(archetype, "3_2", len);
        break;

    case 15:
        strncat(archetype, "4", len);
        break;
    }

    /*
     * No need to change anything if the old and new names are identical.
     */
    if (!strncmp(archetype, wall->arch->name, sizeof(archetype)))
        return;

    /*
     * Before anything, make sure the archetype does exist...
     * If not, prolly an error...
     */
    new_arch = find_archetype(archetype);
    if (!new_arch)
        return;

    /* Now delete current wall, and insert new one
     * We save flags to avoid any trouble with buildable/non buildable, and so on
     */
    for (flag = 0; flag < 4; flag++)
        old_flags[flag] = wall->flags[flag];
    object_remove(wall);
    object_free_drop_inventory(wall);

    wall = arch_to_object(new_arch);
    wall->type = WALL;
    object_insert_in_map_at(wall, map, NULL, INS_ABOVE_FLOOR_ONLY, x, y);
    for (flag = 0; flag < 4; flag++)
        wall->flags[flag] = old_flags[flag];
}

/**
 * Floor building function.
 *
 * Floors can be built:
 *  - on existing floors, with or without a detector/button
 *  - on an existing wall, with or without a floor under it
 *
 * @note
 * this function will inconditionally change squares around (x, y)
 * so don't call it with x == 0 for instance!
 *
 * @param pl
 * player building.
 * @param new_floor
 * new floor object
 * @param x
 * @param y
 * where to build.
 * @return
 * 1 if the floor was built
 */
static int apply_builder_floor(object *pl, object *new_floor, short x, short y) {
    object *above_floor; /* Item above floor, if any */
    object *floor;       /* Floor which would be removed if required */
    struct archt *new_wall;
    int i, xt, yt, wall_removed;
    char message[MAX_BUF];

    snprintf(message, sizeof(message), "You change the floor to better suit your tastes.");

    /*
     * Now the building part...
     * First, remove wall(s) and floor(s) at position x, y
     */
    above_floor = NULL;
    floor = NULL;
    new_wall = NULL;
    wall_removed = 0;
    FOR_MAP_PREPARE(pl->map, x, y, tmp) {
        if (WALL == tmp->type) {
            /* There was a wall, remove it & keep its archetype to make new walls */
            new_wall = tmp->arch;
            object_remove(tmp);
            object_free_drop_inventory(tmp);
            snprintf(message, sizeof(message), "You destroy the wall and redo the floor.");
            wall_removed = 1;
            if (floor != NULL) {
                object_remove(floor);
                object_free_drop_inventory(floor);
                floor = NULL;
            }
        } else if ((FLOOR == tmp->type) || (QUERY_FLAG(tmp, FLAG_IS_FLOOR))) {
            floor = tmp;
        } else {
            if (floor != NULL)
                above_floor = tmp;
        }
    } FOR_MAP_FINISH();

    if (wall_removed == 0 && floor != NULL) {
        if (floor->arch == new_floor->arch) {
            draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_BUILD, "You feel too lazy to redo the exact same floor.");
            object_free_drop_inventory(new_floor);
            return 0;
        }
    }

    SET_FLAG(new_floor, FLAG_UNIQUE);
    SET_FLAG(new_floor, FLAG_IS_FLOOR);
    new_floor->type = FLOOR;
    object_insert_in_map_at(new_floor, pl->map, above_floor, above_floor ? INS_BELOW_ORIGINATOR : INS_ON_TOP, x, y);

    /* if there was a floor, remove it */
    if (floor) {
        object_remove(floor);
        object_free_drop_inventory(floor);
        floor = NULL;
    }

    /*
     * Next step: make sure there are either walls or floors around the new square
     * Since building, you can have: blocking view / floor / wall / nothing
     */
    for (i = 1; i <= 8; i++) {
        object *tmp;

        xt = x+freearr_x[i];
        yt = y+freearr_y[i];
        tmp = GET_MAP_OB(pl->map, xt, yt);
        if (!tmp) {
            /* Must insert floor & wall */

            tmp = arch_to_object(new_floor->arch);
            /* Better make the floor unique */
            SET_FLAG(tmp, FLAG_UNIQUE);
            SET_FLAG(tmp, FLAG_IS_BUILDABLE);
            tmp->type = FLOOR;
            object_insert_in_map_at(tmp, pl->map, NULL, 0, xt, yt);
            /* Insert wall if exists. Note: if it doesn't, the map is weird... */
            if (new_wall) {
                tmp = arch_to_object(new_wall);
                SET_FLAG(tmp, FLAG_IS_BUILDABLE);
                tmp->type = WALL;
                object_insert_in_map_at(tmp, pl->map, NULL, 0, xt, yt);
            }
        }
    }

    /* Finally fixing walls to ensure nice continuous walls
     * Note: 2 squares around are checked, because potentially we added walls
     * around the building spot, so need to check that those new walls connect
     * correctly
     */
    for (xt = x-2; xt <= x+2; xt++)
        for (yt = y-2; yt <= y+2; yt++) {
            if (!OUT_OF_REAL_MAP(pl->map, xt, yt))
                fix_walls(pl->map, xt, yt);
        }

    /* Tell player about the fix */
    draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_BUILD, message);
    return 1;
}

/**
 * Wall building function
 *
 * Walls can be built:
 *  - on a floor without anything else
 *  - on an existing wall, with or without a floor
 *
 * @param pl
 * player building.
 * @param new_wall
 * new wall object
 * @param x
 * @param y
 * where to build.
 * @return
 * 1 if the wall was built
 */
static int apply_builder_wall(object *pl, object *new_wall, short x, short y) {
    object *current_wall;
    char message[MAX_BUF];

    remove_marking_runes(pl->map, x, y);

    current_wall = get_wall(pl->map, x, y);

    if (current_wall) {
        char current_basename[MAX_BUF];
        char new_basename[MAX_BUF];
        char *underscore;

        /* Check if the old and new archetypes have the same prefix */
        strncpy(current_basename, current_wall->arch->name, sizeof(current_basename));
        current_basename[sizeof(current_basename)-1] = '\0';
        underscore = strchr(current_basename, '_');
        if (underscore && isdigit(*(underscore+1))) {
            underscore++;
            *underscore = '\0';
        }
        strncpy(new_basename, new_wall->arch->name, sizeof(new_basename));
        new_basename[sizeof(new_basename)-1] = '\0';
        underscore = strchr(new_basename, '_');
        if (underscore && isdigit(*(underscore+1))) {
            underscore++;
            *underscore = '\0';
        }
        if (!strncmp(current_basename, new_basename, sizeof(new_basename))) {
            draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_BUILD, "You feel too lazy to redo the exact same wall.");
            object_free_drop_inventory(new_wall);
            return 0;
        }
    }

    snprintf(message, sizeof(message), "You build a wall.");
    new_wall->type = WALL;

    if (current_wall) {
        /* If existing wall, replace it, no need to fix other walls */
        object_remove(current_wall);
        object_free_drop_inventory(current_wall);
        object_insert_in_map_at(new_wall, pl->map, NULL, INS_ABOVE_FLOOR_ONLY, x, y);
        fix_walls(pl->map, x, y);
        snprintf(message, sizeof(message), "You redecorate the wall to better suit your tastes.");
    } else {
        int xt, yt;

        /* Else insert new wall and fix all walls around */
        object_insert_in_map_at(new_wall, pl->map, NULL, INS_ABOVE_FLOOR_ONLY, x, y);
        for (xt = x-1; xt <= x+1; xt++)
            for (yt = y-1; yt <= y+1; yt++) {
                if (OUT_OF_REAL_MAP(pl->map, xt, yt))
                    continue;

                fix_walls(pl->map, xt, yt);
            }
    }

    /* Tell player what happened */
    draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_BUILD, message);
    return 1;
}

/**
 * Window building function
 *
 * Windows can be built only on top of existing vertical or horizontal walls
 * (*wall_2_1_1 or *wall_2_1_2).
 *
 * @param pl
 * player building.
 * @param new_wall_win
 * new windowed wall object
 * @param x
 * @param y
 * where to build.
 * @return
 * 1 if the window was built
 */
static int apply_builder_window(object *pl, object *new_wall_win, short x, short y) {
    object *current_wall;
    char archetype[MAX_BUF];
    struct archt *new_arch;
    object *window;
    uint32 old_flags[4];
    int flag;

    /* Too bad, we never use the window contained in the building material */
    object_free_drop_inventory(new_wall_win);

    current_wall = get_wall(pl->map, x, y);

    if (current_wall) {
        char *underscore;

        strncpy(archetype, current_wall->arch->name, sizeof(archetype));
        archetype[sizeof(archetype)-1] = '\0';
        underscore = strchr(archetype, '_');
        if (underscore) {
            underscore++;
            /* Check if the current wall has a window */
            if (!strcmp(underscore, "win1")
            || !strcmp(underscore, "win2")) {
                draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_BUILD, "You feel too lazy to redo the window.");
                return 0;
            }
            if (!strcmp(underscore, "2_1_1"))
                strcpy(underscore, "win1");
            else if (!strcmp(underscore, "2_1_2"))
                strcpy(underscore, "win2");
            else {
                /* Wrong wall orientation */
                draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_BUILD, "You cannot build a window in that wall.");
                return 0;
            }
        }
    } else {
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_BUILD,
                      "There is no wall there.");
        return 0;
    }

    new_arch = find_archetype(archetype);
    if (!new_arch) {
        /* That type of wall doesn't have corresponding window archetypes */
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_BUILD, "You cannot build a window in that wall.");
        return 0;
    }

    /* Now delete current wall, and insert new one with a window
     * We save flags to avoid any trouble with buildable/non buildable, and so on
     */
    for (flag = 0; flag < 4; flag++)
        old_flags[flag] = current_wall->flags[flag];
    object_remove(current_wall);
    object_free_drop_inventory(current_wall);

    window = arch_to_object(new_arch);
    window->type = WALL;
    object_insert_in_map_at(window, pl->map, NULL, INS_ABOVE_FLOOR_ONLY, x, y);
    for (flag = 0; flag < 4; flag++)
        window->flags[flag] = old_flags[flag];

    /* Tell player what happened */
    draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_BUILD, "You build a window in the wall.");
    return 1;
}

/**
 * Generic item builder.
 *
 * Item must be put on a square with a floor, you can have something under.
 *
 * Type of inserted item is tested for specific cases (doors & such).
 *
 * Item is inserted above the floor.
 *
 * Note that it is the responsability of the caller to check whether the space is buildable or not.
 *
 * @param pl
 * player building.
 * @param new_item
 * new item being built
 * @param x
 * @param y
 * where to build.
 * @return
 * 1 if the item was built
 */
static int apply_builder_item(object *pl, object *new_item, short x, short y) {
    int insert_flag;
    object *floor;
    object *con_rune;
    int connected;
    char name[MAX_BUF];

    /* Find floor */
    floor = GET_MAP_OB(pl->map, x, y);
    if (!floor) {
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_BUILD, "Invalid square.");
        object_free_drop_inventory(new_item);
        return 0;
    }

    FOR_OB_AND_ABOVE_PREPARE(floor)
        if (floor->type == FLOOR || QUERY_FLAG(floor, FLAG_IS_FLOOR))
            break;
    FOR_OB_AND_ABOVE_FINISH();
    if (!floor) {
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_BUILD,
                      "This square has no floor, you can't build here.");
        object_free_drop_inventory(new_item);
        return 0;
    }

    SET_FLAG(new_item, FLAG_NO_PICK);

    /*
     * This doesn't work on non unique maps. pedestals under floor will not be saved...
     * insert_flag = (material->stats.Str == 1) ? INS_BELOW_ORIGINATOR : INS_ABOVE_FLOOR_ONLY;
     */
    insert_flag = INS_ABOVE_FLOOR_ONLY;

    connected = 0;
    con_rune = NULL;
    switch (new_item->type) {
    case DOOR:
    case GATE:
    case BUTTON:
    case DETECTOR:
    case TIMED_GATE:
    case PEDESTAL:
    case CF_HANDLE:
    case MAGIC_EAR:
    case SIGN:
        /* Signs don't need a connection, but but magic mouths do. */
        if (new_item->type == SIGN && strcmp(new_item->arch->name, "magic_mouth"))
            break;
        con_rune = get_connection_rune(pl, x, y);
        connected = find_or_create_connection_for_map(pl, x, y, con_rune);
        if (connected == -1) {
            /* Player already informed of failure by the previous function */
            object_free_drop_inventory(new_item);
            return 0;
        }
    }

    /* For magic mouths/ears, and signs, take the msg from a book of scroll */
    if ((new_item->type == SIGN) || (new_item->type == MAGIC_EAR)) {
        if (adjust_sign_msg(pl, x, y, new_item) == -1) {
            object_free_drop_inventory(new_item);
            return 0;
        }
    }

    if (con_rune != NULL) {
        /* Remove marking rune */
        object_remove(con_rune);
        object_free_drop_inventory(con_rune);
    }

    object_insert_in_map_at(new_item, pl->map, floor, insert_flag, x, y);
    if (connected != 0)
        add_button_link(new_item, pl->map, connected);

    query_name(new_item, name, MAX_BUF);
    draw_ext_info_format(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_BUILD,
                         "You build the %s",
                         name);
    return 1;
}

/**
 * Item remover.
 *
 * Removes first buildable item, either under or above the floor
 *
 * @param pl
 * player removing an item.
 * @param dir
 * direction the player is trying to remove.
 */
void apply_builder_remove(object *pl, int dir) {
    object *item;
    short x, y;
    char name[MAX_BUF];

    x = pl->x+freearr_x[dir];
    y = pl->y+freearr_y[dir];

    /* Check square */
    item = GET_MAP_OB(pl->map, x, y);
    if (!item) {
        /* Should not happen with previous tests, but we never know */
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_BUILD,
                      "Invalid square.");
        LOG(llevError, "apply_builder_remove: (null) square at (%d, %d, %s)\n", x, y, pl->map->path);
        return;
    }

    if (item->type == FLOOR || QUERY_FLAG(item, FLAG_IS_FLOOR))
        item = item->above;

    if (!item) {
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_BUILD,
                      "Nothing to remove.");
        return;
    }

    /* Now remove object, with special cases (buttons & such) */
    switch (item->type) {
    case WALL:
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_BUILD,
                      "Can't remove a wall with that, build a floor.");
        return;

    case DOOR:
    case BUTTON:
    case GATE:
    case TIMED_GATE:
    case DETECTOR:
    case PEDESTAL:
    case CF_HANDLE:
    case MAGIC_EAR:
    case SIGN:
        /* Special case: must unconnect */
        if (QUERY_FLAG(item, FLAG_IS_LINKED))
            remove_button_link(item);

        /* Fall through */
    default:
        /* Remove generic item */
        query_name(item, name, MAX_BUF);
        draw_ext_info_format(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_BUILD,
                             "You remove the %s",
                             name);
        object_remove(item);
        object_free_drop_inventory(item);
    }
}

/**
 * Global building function
 *
 * This is the general map building function. Called when the player 'fires' a
 * builder or remover object.
 *
 * @param pl
 * player building or removing.
 * @param dir
 * building direction.
 */
void apply_map_builder(object *pl, int dir) {
    object *builder;
    object *tmp;
    short x, y;

    if (!pl->type == PLAYER)
        return;

    if (dir == 0) {
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_BUILD,
                      "You can't build or destroy under yourself.");
        return;
    }

    x = pl->x+freearr_x[dir];
    y = pl->y+freearr_y[dir];

    if ((1 > x) || (1 > y)
    || ((MAP_WIDTH(pl->map)-2) < x) || ((MAP_HEIGHT(pl->map)-2) < y)) {
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_BUILD,
                      "Can't build on map edge.");
        return;
    }

    /*
     * Check specified square
     * The square must have only buildable items
     * Exception: marking runes are all right,
     * since they are used for special things like connecting doors / buttons
     */

    tmp = GET_MAP_OB(pl->map, x, y);
    if (!tmp) {
        /* Nothing, meaning player is standing next to an undefined square. */
        LOG(llevError, "apply_map_builder: undefined square at (%d, %d, %s)\n", x, y, pl->map->path);
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_BUILD,
                      "You'd better not build here, it looks weird.");
        return;
    }

    builder = pl->contr->ranges[range_builder];

    if (builder->subtype != ST_BD_BUILD) {
        FOR_OB_AND_ABOVE_PREPARE(tmp)
            if (!QUERY_FLAG(tmp, FLAG_IS_BUILDABLE)
            && ((tmp->type != SIGN) || (strcmp(tmp->arch->name, "rune_mark")))) {
                draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_BUILD,
                              "You can't build here.");
                return;
            }
        FOR_OB_AND_ABOVE_FINISH();
    }

    /* Now we know the square is ok */

    if (builder->subtype == ST_BD_REMOVE) {
        /* Remover -> call specific function and bail out */
        apply_builder_remove(pl, dir);
        return;
    }

    if (builder->subtype == ST_BD_BUILD) {
        object *material;
        struct archt *new_arch;
        object *new_item;
        int built = 0;

        /* Builder -> find material, get new item, call specific function */
        /* find the marked item to buld */
        material = find_marked_object(pl);
        if (!material) {
            draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_BUILD,
                          "You need to mark raw materials to use.");
            return;
        }

        if (material->type != MATERIAL) {
            draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_BUILD,
                          "You can't use the marked item to build.");
            return;
        }

        /* create a new object from the raw materials */
        new_arch = find_archetype(material->slaying);
        if (!new_arch) {
            draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_BUILD,
                         "You can't use this strange material.");
            LOG(llevError, "apply_map_builder: unable to find archetype %s\n", material->slaying);
            return;
        }
        new_item = object_create_arch(new_arch);
        SET_FLAG(new_item, FLAG_IS_BUILDABLE);

        if (!can_build_over(pl->map, new_item, x, y)) {
            draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_BUILD,
                          "You can't build here.");
            return;
        }

        /* insert the new object in the map */
        switch (material->subtype) {
        case ST_MAT_FLOOR:
            built = apply_builder_floor(pl, new_item, x, y);
            break;

        case ST_MAT_WALL:
            built = apply_builder_wall(pl, new_item, x, y);
            break;

        case ST_MAT_ITEM:
            built = apply_builder_item(pl, new_item, x, y);
            break;

        case ST_MAT_WINDOW:
            built = apply_builder_window(pl, new_item, x, y);
            break;

        default:
            draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_BUILD,
                          "Don't know how to apply this material, sorry.");
            LOG(llevError, "apply_map_builder: invalid material subtype %d\n", material->subtype);
            break;
        }
        if (built)
            object_decrease_nrof_by_one(material);
        return;
    }

    /* Here, it means the builder has an invalid type */
    draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_APPLY, MSG_TYPE_APPLY_BUILD,
                  "Don't know how to apply this tool, sorry.");
    LOG(llevError, "apply_map_builder: invalid builder subtype %d\n", builder->subtype);
}
