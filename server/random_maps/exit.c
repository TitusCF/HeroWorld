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

#include <global.h>
#include <random_map.h>
#include <sproto.h>
#include <rproto.h>

/**
 * @file
 * Handle exit placement in map.
 */

/**
 * Find a character in the layout.
 * @param mode
 * how to look:
 * - 1: from top/left to bottom/right.
 * - 2: from top/right to bottom/left.
 * - 3: from bottom/left to top/right.
 * - 4: from bottom/right to top/left.
 * - other value: one random order is chosen.
 * @param target
 * character to search.
 * @param fx
 * @param fy
 * position of character, or -1 if not found.
 * @param layout
 * maze.
 * @param RP
 * maze parameters.
 */
void find_in_layout(int mode, char target, int *fx, int *fy, char **layout, RMParms *RP)
{
    int M;
    int i, j;

    *fx = -1;
    *fy = -1;

    /* if a starting mode isn't given, pick one */
    if (mode < 1 || mode > 4) {
        M = RANDOM()%4+1;
    } else {
        M = mode;
    }

    /* four different search starting points and methods so that
       we can do something different for symmetrical maps instead of
       the same damned thing every time. */
    switch (M) {
    case 1: {  /* search from top left down/right */
        for (i = 1; i < RP->Xsize; i++)
            for (j = 1; j < RP->Ysize; j++) {
                if (layout[i][j] == target) {
                    *fx = i;
                    *fy = j;
                    return;
                }
            }
        break;
    }

    case 2: { /* Search from top right down/left */
        for (i = RP->Xsize-2; i > 0; i--)
            for (j = 1; j < RP->Ysize-1; j++) {
                if (layout[i][j] == target) {
                    *fx = i;
                    *fy = j;
                    return;
                }
            }
        break;
    }

    case 3: { /* search from bottom-left up-right */
        for (i = 1; i < RP->Xsize-1; i++)
            for (j = RP->Ysize-2; j > 0; j--) {
                if (layout[i][j] == target) {
                    *fx = i;
                    *fy = j;
                    return;
                }
            }
        break;
    }

    case 4: { /* search from bottom-right up-left */
        for (i = RP->Xsize-2; i > 0; i--)
            for (j = RP->Ysize-2; j > 0; j--) {
                if (layout[i][j] == target) {
                    *fx = i;
                    *fy = j;
                    return;
                }
            }
        break;
    }
    }
}

/* orientation:
*/

/**
 * Place exits in the map.
 * @param map
 * map to put exits into.
 * @param maze
 * map layout.
 * @param exitstyle
 * what style to use. If NULL uses a random one.
 * @param orientation
 * how exits should be oriented:
 * - 0 means random.
 * - 1 means descending dungeon.
 * - 2 means ascending dungeon.
 * - 3 means rightward.
 * - 4 means leftward.
 * - 5 means northward.
 * - 6 means southward.
 * @param RP
 * map parameters.
 * @note
 * unblock_exits() should be called at some point, as exits will be blocking everything
 * to avoid putting other objects on them.
 * @todo
 * add orientations 3-6 or fix previous comment.
 */
void place_exits(mapstruct *map, char **maze, char *exitstyle, int orientation, RMParms *RP)
{
    char styledirname[256];
    mapstruct *style_map_down = NULL; /* harder maze */
    mapstruct *style_map_up = NULL;   /* easier maze */
    object *the_exit_down;            /* harder maze */
    object *the_exit_up;              /* easier maze */
    object *random_sign;              /* magic mouth saying this is a random map. */
    char buf[512];
    int cx = -1, cy = -1;             /* location of a map center */
    int upx = -1, upy = -1;           /* location of up exit */
    int downx = -1, downy = -1;
    int final_map_exit = 1;
    int i, j;

    if (RP->exit_on_final_map) {
        if (strstr(RP->exit_on_final_map, "no")) {
            final_map_exit = 0;
        }
    }

    if (orientation == 0) {
        orientation = RANDOM()%6+1;
    }

    switch (orientation) {
    case 1: {
        snprintf(styledirname, sizeof(styledirname), "/styles/exitstyles/up");
        style_map_up = find_style(styledirname, exitstyle, -1);
        snprintf(styledirname, sizeof(styledirname), "/styles/exitstyles/down");
        style_map_down = find_style(styledirname, exitstyle, -1);
        break;
    }

    case 2: {
        snprintf(styledirname, sizeof(styledirname), "/styles/exitstyles/down");
        style_map_up = find_style(styledirname, exitstyle, -1);
        snprintf(styledirname, sizeof(styledirname), "/styles/exitstyles/up");
        style_map_down = find_style(styledirname, exitstyle, -1);
        break;
    }

    default: {
        snprintf(styledirname, sizeof(styledirname), "/styles/exitstyles/generic");
        style_map_up = find_style(styledirname, exitstyle, -1);
        style_map_down = style_map_up;
        break;
    }
    }

    if (style_map_up == NULL) {
        the_exit_up = arch_to_object(find_archetype("exit"));
    } else {
        object *tmp;

        tmp = pick_random_object(style_map_up);
        the_exit_up = arch_to_object(tmp->arch);
    }

    /* we need a down exit only if we're recursing. */
    if (RP->dungeon_level < RP->dungeon_depth || RP->final_map[0] != 0)
        if (RP->dungeon_level >= RP->dungeon_depth && RP->final_exit_archetype[0] != 0) {
            the_exit_down = arch_to_object(find_archetype(RP->final_exit_archetype));
        } else if (style_map_down == NULL) {
            the_exit_down = arch_to_object(find_archetype("exit"));
        } else {
            object *tmp;

            tmp = pick_random_object(style_map_down);
            the_exit_down = arch_to_object(tmp->arch);
        }
    else {
        the_exit_down = NULL;
    }

    /* set up the up exit */
    the_exit_up->stats.hp = RP->origin_x;
    the_exit_up->stats.sp = RP->origin_y;
    the_exit_up->slaying = add_string(RP->origin_map);

    /* figure out where to put the entrance */
    /* First, look for a '<' char */
    find_in_layout(0, '<', &upx, &upy, maze, RP);

    /* next, look for a C, the map center.  */
    find_in_layout(0, 'C', &cx, &cy, maze, RP);

    /* if we didn't find an up, find an empty place far from the center */
    if (upx == -1 && cx != -1) {
        if (cx > RP->Xsize/2) {
            upx = 1;
        } else {
            upx = RP->Xsize-2;
        }
        if (cy > RP->Ysize/2) {
            upy = 1;
        } else {
            upy = RP->Ysize-2;
        }

        /* find an empty place far from the center */
        if (upx == 1 && upy == 1) {
            find_in_layout(1, 0, &upx, &upy, maze, RP);
        } else if (upx == 1 && upy > 1) {
            find_in_layout(3, 0, &upx, &upy, maze, RP);
        } else if (upx > 1 && upy == 1) {
            find_in_layout(2, 0, &upx, &upy, maze, RP);
        } else if (upx > 1 && upy > 1) {
            find_in_layout(4, 0, &upx, &upy, maze, RP);
        }
    }

    /* no indication of where to place the exit, so just place it at any empty spot. */
    if (upx == -1) {
        find_in_layout(0, 0, &upx, &upy, maze, RP);
    }

    /* surround the exits with notices that this is a random map. */
    for (j = 1; j < 9; j++) {
        if (!wall_blocked(map, upx+freearr_x[j], upy+freearr_y[j])) {
            random_sign = create_archetype("sign");

            snprintf(buf, sizeof(buf), "This is a random map.\nLevel: %d\n", (RP->dungeon_level)-1);
            object_set_msg(random_sign, buf);
            object_insert_in_map_at(random_sign, map, NULL, 0, upx+freearr_x[j], upy+freearr_y[j]);
        }
    }
    /* Block the exit so things don't get dumped on top of it. */
    the_exit_up->move_block = MOVE_ALL;

    object_insert_in_map_at(the_exit_up, map, NULL, 0, upx, upy);
    maze[upx][upy] = '<';

    /* set the starting x,y for this map */
    MAP_ENTER_X(map) = upx;
    MAP_ENTER_Y(map) = upy;

    /* first, look for a '>' character */
    find_in_layout(0, '>', &downx, &downy, maze, RP);
    /* if no > is found use C */
    if (downx == -1) {
        downx = cx;
        downy = cy;
    }

    /* make the other exit far away from this one if
       there's no center. */
    if (downx == -1) {
        if (upx > RP->Xsize/2) {
            downx = 1;
        } else {
            downx = RP->Xsize-2;
        }
        if (upy > RP->Ysize/2) {
            downy = 1;
        } else {
            downy = RP->Ysize-2;
        }

        /* find an empty place far from the entrance */
        if (downx == 1 && downy == 1) {
            find_in_layout(1, 0, &downx, &downy, maze, RP);
        } else if (downx == 1 && downy > 1) {
            find_in_layout(3, 0, &downx, &downy, maze, RP);
        } else if (downx > 1 && downy == 1) {
            find_in_layout(2, 0, &downx, &downy, maze, RP);
        } else if (downx > 1 && downy > 1) {
            find_in_layout(4, 0, &downx, &downy, maze, RP);
        }
    }
    /* no indication of where to place the down exit, so just place it on an empty spot. */
    if (downx == -1) {
        find_in_layout(0, 0, &downx, &downy, maze, RP);
    }
    if (the_exit_down) {
        char *buf;

        i = object_find_first_free_spot(the_exit_down, map, downx, downy);
        RP->origin_x = downx+freearr_x[i];
        RP->origin_y = downy+freearr_y[i];
        buf = stringbuffer_finish(write_map_parameters_to_string(RP));
        object_set_msg(the_exit_down, buf);
        free(buf);
        /* the identifier for making a random map. */
        if (RP->dungeon_level >= RP->dungeon_depth && RP->final_map[0] != 0) {
            /* Next map is the final map, special case. */
            mapstruct *new_map;
            object *the_exit_back = arch_to_object(the_exit_up->arch);

            /* load it */
            if ((new_map = ready_map_name(RP->final_map, 0)) == NULL) {
                return;
            }

            the_exit_down->slaying = add_string(RP->final_map);
            EXIT_X(the_exit_down) = MAP_ENTER_X(new_map);
            EXIT_Y(the_exit_down) = MAP_ENTER_Y(new_map);
            strncpy(new_map->path, RP->final_map, sizeof(new_map->path));

            FOR_MAP_PREPARE(new_map, MAP_ENTER_X(new_map), MAP_ENTER_Y(new_map), tmp)
            /* Remove exit back to previous random map.  There should only be one
             * which is why we break out.  To try to process more than one
             * would require keeping a 'next' pointer, as object_free_drop_inventory() kills tmp, which
             * breaks the for loop.
             */
            if (tmp->type == EXIT && EXIT_PATH(tmp) && !strncmp(EXIT_PATH(tmp), "/random/", 8)) {
                object_remove(tmp);
                object_free_drop_inventory(tmp);
                break;
            }
            FOR_MAP_FINISH();

            if (final_map_exit == 1) {
                /* setup the exit back */
                the_exit_back->slaying = add_string(map->path);
                the_exit_back->stats.hp = downx+freearr_x[i];
                the_exit_back->stats.sp = downy+freearr_y[i];
                object_insert_in_map_at(the_exit_back, new_map, NULL, 0, MAP_ENTER_X(new_map), MAP_ENTER_Y(new_map));
            }

            set_map_timeout(new_map);   /* So it gets swapped out */
        } else {
            the_exit_down->slaying = add_string("/!");
        }

        /* Block the exit so things don't get dumped on top of it. */
        the_exit_down->move_block = MOVE_ALL;
        object_insert_in_map_at(the_exit_down, map, NULL, 0, downx+freearr_x[i], downy+freearr_y[i]);
        maze[the_exit_down->x][the_exit_down->y] = '>';
    }
}

/**
 * This function unblocks the exits.  We blocked them to
 * keep things from being dumped on them during the other
 * phases of random map generation.
 * @param map
 * map to alter.
 * @param maze
 * map layout.
 * @param RP
 * map generation parameters.
 */
void unblock_exits(mapstruct *map, char **maze, RMParms *RP)
{
    int i = 0, j = 0;

    for (i = 0; i < RP->Xsize; i++)
        for (j = 0; j < RP->Ysize; j++)
            if (maze[i][j] == '>' || maze[i][j] == '<') {
                FOR_MAP_PREPARE(map, i, j, walk) {
                    if (walk->move_block == MOVE_ALL && walk->type != LOCKED_DOOR) {
                        walk->move_block = MOVE_BLOCK_DEFAULT;
                        object_update(walk, UP_OBJ_CHANGE);
                    }
                }
                FOR_MAP_FINISH();
            }
}
