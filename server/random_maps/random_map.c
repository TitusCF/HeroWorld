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
 * @file random_maps/random_map.c
 * Routines for random map generation.
 *
 * @todo
 * Explain process, layout signs (#, C, <, >) and such. Use constants for
 * common spot types.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "global.h"
#include "maze_gen.h"
#include "random_map.h"
#include "room_gen.h"
#include "rproto.h"
#include "sproto.h"

/**
 * Dumps specified layout using printf().
 * @param layout
 * layout to dump.
 * @param RP
 * layout parameters.
 */
void dump_layout(char **layout, RMParms *RP)
{
    int i, j;

    for (i = 0; i < RP->Xsize; i++) {
        for (j = 0; j < RP->Ysize; j++) {
            if (layout[i][j] == 0) {
                layout[i][j] = ' ';
            }
            printf("%c", layout[i][j]);
            if (layout[i][j] == ' ') {
                layout[i][j] = 0;
            }
        }
        printf("\n");
    }
    printf("\n");
}

/**
 * Main random map routine. Generates a random map based on specified parameters.
 * @param OutFileName
 * the path the map should have.
 * @param RP
 * parameters for generation.
 * @param use_layout
 * if not NULL, this should be a suitable layout.
 * @return
 * Crossfire map, which should be free()d by caller.
 */
mapstruct *generate_random_map(const char *OutFileName, RMParms *RP, char **use_layout)
{
    char **layout, *buf;
    mapstruct *theMap;
    int i;

    /* pick a random seed, or use the one from the input file */
    if (RP->random_seed == 0) {
        RP->random_seed = time(NULL);
    }

    SRANDOM(RP->random_seed);

    buf = stringbuffer_finish(write_map_parameters_to_string(RP));

    if (RP->difficulty == 0) {
        RP->difficulty = RP->dungeon_level; /* use this instead of a map difficulty  */
        if (RP->difficulty_increase > 0.001) {
            RP->difficulty = (int)((float)RP->dungeon_level*RP->difficulty_increase);
            if (RP->difficulty < 1) {
                RP->difficulty = 1;
            }
        }
    } else {
        RP->difficulty_given = 1;
    }

    if (!use_layout) {
        if (RP->Xsize < MIN_RANDOM_MAP_SIZE) {
            RP->Xsize = MIN_RANDOM_MAP_SIZE+RANDOM()%25+5;
        }
        if (RP->Ysize < MIN_RANDOM_MAP_SIZE) {
            RP->Ysize = MIN_RANDOM_MAP_SIZE+RANDOM()%25+5;
        }

        if (RP->expand2x > 0) {
            RP->Xsize /= 2;
            RP->Ysize /= 2;
        }

        layout = layoutgen(RP);

#ifdef RMAP_DEBUG
        dump_layout(layout, RP);
#endif

        /*  rotate the layout randomly */
        layout = rotate_layout(layout, RANDOM()%4, RP);
#ifdef RMAP_DEBUG
        dump_layout(layout, RP);
#endif
    } else {
        layout = use_layout;
    }

    /* increment these for the current map */
    RP->dungeon_level += 1;

    /* allocate the map and set the floor */
    theMap = make_map_floor(layout, RP->floorstyle, RP);

    /* set the name of the map. */
    strncpy(theMap->path, OutFileName, sizeof(theMap->path));

    /* set region */
    theMap->region = RP->region;

    /* create walls unless the wallstyle is "none" */
    if (strcmp(RP->wallstyle, "none")) {
        make_map_walls(theMap, layout, RP->wallstyle, RP);

        /* place doors unless doorstyle or wallstyle is "none"*/
        if (strcmp(RP->doorstyle, "none")) {
            put_doors(theMap, layout, RP->doorstyle, RP);
        }
    }

    /* create exits unless the exitstyle is "none" */
    if (strcmp(RP->exitstyle, "none")) {
        place_exits(theMap, layout, RP->exitstyle, RP->orientation, RP);
    }

    place_specials_in_map(theMap, layout, RP);

    /* create monsters unless the monsterstyle is "none" */
    if (strcmp(RP->monsterstyle, "none")) {
        place_monsters(theMap, RP->monsterstyle, RP->difficulty, RP);
    }

    /* treasures needs to have a proper difficulty set for the map. */
    theMap->difficulty = calculate_difficulty(theMap);

    /* create treasure unless the treasurestyle is "none" */
    if (strcmp(RP->treasurestyle, "none")) {
        place_treasure(theMap, layout, RP->treasurestyle, RP->treasureoptions, RP);
    }

    /* create decor unless the decorstyle is "none" */
    if (strcmp(RP->decorstyle, "none")) {
        put_decor(theMap, layout, RP->decorstyle, RP->decoroptions, RP);
    }

    /* generate treasures, etc. */
    apply_auto_fix(theMap);

    unblock_exits(theMap, layout, RP);

    /* free the layout unless it was given by caller */
    if (!use_layout) {
        for (i = 0; i < RP->Xsize; i++) {
            free(layout[i]);
        }
        free(layout);
    }

    theMap->msg = buf;

    /* We set the reset time at this, so town portal works on the map. */
    gettimeofday(&(theMap->last_reset_time), NULL);

    return theMap;
}

/**
 * This function builds the actual layout.
 * Selects the layout based on parameters and gives it whatever
 * arguments it needs.
 * @param RP
 * random map parameters.
 * @return
 * layout, must be free()d be caller.
 * @todo
 * use an array for name/style mapping. Refactor to call only one function for each
 * (will make it easier to override later on).
 */
char **layoutgen(RMParms *RP)
{
    char **maze = NULL;
    int oxsize = RP->Xsize, oysize = RP->Ysize;

    if (RP->symmetry == RANDOM_SYM) {
        RP->symmetry_used = (RANDOM()%(XY_SYM))+1;
    } else {
        RP->symmetry_used = RP->symmetry;
    }

    if (RP->symmetry_used == Y_SYM || RP->symmetry_used == XY_SYM) {
        RP->Ysize = RP->Ysize/2+1;
    }
    if (RP->symmetry_used == X_SYM || RP->symmetry_used == XY_SYM) {
        RP->Xsize = RP->Xsize/2+1;
    }

    if (RP->Xsize < MIN_RANDOM_MAP_SIZE) {
        RP->Xsize = MIN_RANDOM_MAP_SIZE+RANDOM()%5;
    }
    if (RP->Ysize < MIN_RANDOM_MAP_SIZE) {
        RP->Ysize = MIN_RANDOM_MAP_SIZE+RANDOM()%5;
    }
    RP->map_layout_style = 0;

    /* Redo this - there was a lot of redundant code of checking for preset
     * layout style and then random layout style.  Instead, figure out
     * the numeric layoutstyle, so there is only one area that actually
     * calls the code to make the maps.
     */
    if (strstr(RP->layoutstyle, "onion")) {
        RP->map_layout_style = ONION_LAYOUT;
    }

    if (strstr(RP->layoutstyle, "maze")) {
        RP->map_layout_style = MAZE_LAYOUT;
    }

    if (strstr(RP->layoutstyle, "spiral")) {
        RP->map_layout_style = SPIRAL_LAYOUT;
    }

    if (strstr(RP->layoutstyle, "rogue")) {
        RP->map_layout_style = ROGUELIKE_LAYOUT;
    }

    if (strstr(RP->layoutstyle, "snake")) {
        RP->map_layout_style = SNAKE_LAYOUT;
    }

    if (strstr(RP->layoutstyle, "squarespiral")) {
        RP->map_layout_style = SQUARE_SPIRAL_LAYOUT;
    }
    /* No style found - choose one ranomdly */
    if (RP->map_layout_style == 0) {
        RP->map_layout_style = (RANDOM()%NROFLAYOUTS)+1;
    }

    switch (RP->map_layout_style) {
    case ONION_LAYOUT:
        maze = map_gen_onion(RP->Xsize, RP->Ysize, RP->layoutoptions1, RP->layoutoptions2);
        if (!(RANDOM()%3) && !(RP->layoutoptions1&OPT_WALLS_ONLY)) {
            roomify_layout(maze, RP);
        }
        break;

    case MAZE_LAYOUT:
        maze = maze_gen(RP->Xsize, RP->Ysize, RANDOM()%2);
        if (!(RANDOM()%2)) {
            doorify_layout(maze, RP);
        }
        break;

    case SPIRAL_LAYOUT:
        maze = map_gen_spiral(RP->Xsize, RP->Ysize, RP->layoutoptions1);
        if (!(RANDOM()%2)) {
            doorify_layout(maze, RP);
        }
        break;

    case ROGUELIKE_LAYOUT:
        /* Don't put symmetry in rogue maps.  There isn't much reason to
         * do so in the first place (doesn't make it any more interesting),
         * but more importantly, the symmetry code presumes we are symmetrizing
         * spirals, or maps with lots of passages - making a symmetric rogue
         * map fails because its likely that the passages the symmetry process
         * creates may not connect the rooms.
         */
        RP->symmetry_used = NO_SYM;
        RP->Ysize = oysize;
        RP->Xsize = oxsize;
        maze = roguelike_layout_gen(RP->Xsize, RP->Ysize, RP->layoutoptions1);
        /* no doorifying...  done already */
        break;

    case SNAKE_LAYOUT:
        maze = make_snake_layout(RP->Xsize, RP->Ysize);
        if (RANDOM()%2) {
            roomify_layout(maze, RP);
        }
        break;

    case SQUARE_SPIRAL_LAYOUT:
        maze = make_square_spiral_layout(RP->Xsize, RP->Ysize);
        if (RANDOM()%2) {
            roomify_layout(maze, RP);
        }
        break;
    }

    maze = symmetrize_layout(maze, RP->symmetry_used, RP);
#ifdef RMAP_DEBUG
    dump_layout(maze, RP);
#endif
    if (RP->expand2x) {
        maze = expand2x(maze, RP->Xsize, RP->Ysize);
        RP->Xsize = RP->Xsize*2-1;
        RP->Ysize = RP->Ysize*2-1;
    }
    return maze;
}

/**
 * Takes a map and makes it symmetric:  adjusts Xsize and
 * Ysize to produce a symmetric map.
 * @param maze
 * layout to symmetrize. Will be free()d by this function.
 * @param sym
 * how to make symetric, a @ref SYM_xxx value.
 * @param RP
 * map parameters.
 * @return
 * new layout, must be free()d by caller.
 */
char **symmetrize_layout(char **maze, int sym, RMParms *RP)
{
    int i, j;
    char **sym_maze;
    int Xsize_orig, Ysize_orig;

    Xsize_orig = RP->Xsize;
    Ysize_orig = RP->Ysize;
    RP->symmetry_used = sym;  /* tell everyone else what sort of symmetry is used.*/
    if (sym == NO_SYM) {
        RP->Xsize = Xsize_orig;
        RP->Ysize = Ysize_orig;
        return maze;
    }
    /* pick new sizes */
    RP->Xsize = ((sym == X_SYM || sym == XY_SYM) ? RP->Xsize*2-3 : RP->Xsize);
    RP->Ysize = ((sym == Y_SYM || sym == XY_SYM) ? RP->Ysize*2-3 : RP->Ysize);

    sym_maze = (char **)calloc(sizeof(char *), RP->Xsize);
    for (i = 0; i < RP->Xsize; i++) {
        sym_maze[i] = (char *)calloc(sizeof(char), RP->Ysize);
    }

    if (sym == X_SYM)
        for (i = 0; i < RP->Xsize/2+1; i++)
            for (j = 0; j < RP->Ysize; j++) {
                sym_maze[i][j] = maze[i][j];
                sym_maze[RP->Xsize-i-1][j] = maze[i][j];
            };
    if (sym == Y_SYM)
        for (i = 0; i < RP->Xsize; i++)
            for (j = 0; j < RP->Ysize/2+1; j++) {
                sym_maze[i][j] = maze[i][j];
                sym_maze[i][RP->Ysize-j-1] = maze[i][j];
            }
    if (sym == XY_SYM)
        for (i = 0; i < RP->Xsize/2+1; i++)
            for (j = 0; j < RP->Ysize/2+1; j++) {
                sym_maze[i][j] = maze[i][j];
                sym_maze[i][RP->Ysize-j-1] = maze[i][j];
                sym_maze[RP->Xsize-i-1][j] = maze[i][j];
                sym_maze[RP->Xsize-i-1][RP->Ysize-j-1] = maze[i][j];
            }

    /* delete the old maze */
    for (i = 0; i < Xsize_orig; i++) {
        free(maze[i]);
    }
    free(maze);

    /* reconnect disjointed spirals */
    if (RP->map_layout_style == SPIRAL_LAYOUT) {
        connect_spirals(RP->Xsize, RP->Ysize, sym, sym_maze);
    }
    /* reconnect disjointed nethackmazes:  the routine for
     * spirals will do the trick?
     */
    if (RP->map_layout_style == ROGUELIKE_LAYOUT) {
        connect_spirals(RP->Xsize, RP->Ysize, sym, sym_maze);
    }

    return sym_maze;
}

/**
 * Takes  a map and rotates it. This completes the
 * onion layouts, making them possibly centered on any wall.
 * It'll modify Xsize and Ysize if they're swapped.
 * @param maze
 * layout to rotate, will be free()d by this function.
 * @param rotation
 * how to rotate:
 * - 0: don't do anything.
 * - 1: rotate 90 deg clockwise.
 * - 2: rotate 180 deg.
 * - 3: rotate 90 deg counter-clockwise.
 * @param RP
 * random map parameters.
 * @return
 * new layout, must be free()d be caller. NULL if invalid rotation.
 */
char **rotate_layout(char **maze, int rotation, RMParms *RP)
{
    char **new_maze;
    int i, j;

    switch (rotation) {
    case 0:
        return maze;
        break;

    case 2: { /* a reflection */
        char *new = malloc(sizeof(char)*RP->Xsize*RP->Ysize);

        for (i = 0; i < RP->Xsize; i++) { /* make a copy */
            for (j = 0; j < RP->Ysize; j++) {
                new[i*RP->Ysize+j] = maze[i][j];
            }
        }
        for (i = 0; i < RP->Xsize; i++) { /* copy a reflection back */
            for (j = 0; j < RP->Ysize; j++) {
                maze[i][j] = new[(RP->Xsize-i-1)*RP->Ysize+RP->Ysize-j-1];
            }
        }
        free(new);
        return maze;
        break;
    }

    case 1:
    case 3: {
        int swap;

        new_maze = (char **)calloc(sizeof(char *), RP->Ysize);
        for (i = 0; i < RP->Ysize; i++) {
            new_maze[i] = (char *)calloc(sizeof(char), RP->Xsize);
        }
        if (rotation == 1) /* swap x and y */
            for (i = 0; i < RP->Xsize; i++)
                for (j = 0; j < RP->Ysize; j++) {
                    new_maze[j][i] = maze[i][j];
                }

        if (rotation == 3) { /* swap x and y */
            for (i = 0; i < RP->Xsize; i++)
                for (j = 0; j < RP->Ysize; j++) {
                    new_maze[j][i] = maze[RP->Xsize-i-1][RP->Ysize-j-1];
                }
        }

        /* delete the old layout */
        for (i = 0; i < RP->Xsize; i++) {
            free(maze[i]);
        }
        free(maze);

        swap = RP->Ysize;
        RP->Ysize = RP->Xsize;
        RP->Xsize = swap;
        return new_maze;
        break;
    }
    }
    return NULL;
}

/**
 * Take a layout and make some rooms in it. Works best on onions.
 * @param maze
 * layout to alter.
 * @param RP
 * map parameters.
 */
void roomify_layout(char **maze, RMParms *RP)
{
    int tries = RP->Xsize*RP->Ysize/30;
    int ti;

    for (ti = 0; ti < tries; ti++) {
        int dx, dy;  /* starting location for looking at creating a door */
        int cx, cy;  /* results of checking on creating walls. */

        dx = RANDOM()%RP->Xsize;
        dy = RANDOM()%RP->Ysize;
        cx = can_make_wall(maze, dx, dy, 0, RP);  /* horizontal */
        cy = can_make_wall(maze, dx, dy, 1, RP);  /* vertical */
        if (cx == -1) {
            if (cy != -1) {
                make_wall(maze, dx, dy, 1);
            }
            continue;
        }
        if (cy == -1) {
            make_wall(maze, dx, dy, 0);
            continue;
        }
        if (cx < cy) {
            make_wall(maze, dx, dy, 0);
        } else {
            make_wall(maze, dx, dy, 1);
        }
    }
}

/**
 * Checks the layout to see if we can stick a horizontal (dir = 0) wall
 * (or vertical, dir == 1) here which ends up on other walls sensibly.
 * @param maze
 * layout.
 * @param dx
 * @param dy
 * coordinates to check
 * @param dir
 * direction:
 * - 0: horizontally.
 * - 1: vertically.
 * @param RP
 * random map parameters.
 * @return
 * -1 if wall can't be made, possibly wall length else.
 */
int can_make_wall(char **maze, int dx, int dy, int dir, RMParms *RP)
{
    int i1;
    int length = 0;

    /* dont make walls if we're on the edge. */
    if (dx == 0 || dx == (RP->Xsize-1) || dy == 0 || dy == (RP->Ysize-1)) {
        return -1;
    }

    /* don't make walls if we're ON a wall. */
    if (maze[dx][dy] != 0) {
        return -1;
    }

    if (dir == 0) {
        /* horizontal */
        int y = dy;

        for (i1 = dx-1; i1 > 0; i1--) {
            int sindex = surround_flag2(maze, i1, y, RP);

            if (sindex == 1) {
                break;
            }
            if (sindex != 0) {
                return -1;    /* can't make horiz.  wall here */
            }
            if (maze[i1][y] != 0) {
                return -1;    /* can't make horiz.  wall here */
            }
            length++;
        }

        for (i1 = dx+1; i1 < RP->Xsize-1; i1++) {
            int sindex = surround_flag2(maze, i1, y, RP);

            if (sindex == 2) {
                break;
            }
            if (sindex != 0) {
                return -1;    /* can't make horiz.  wall here */
            }
            if (maze[i1][y] != 0) {
                return -1;    /* can't make horiz.  wall here */
            }
            length++;
        }
        return length;
    } else {
        /* vertical */
        int x = dx;

        for (i1 = dy-1; i1 > 0; i1--) {
            int sindex = surround_flag2(maze, x, i1, RP);

            if (sindex == 4) {
                break;
            }
            if (sindex != 0) {
                return -1;    /* can't make vert. wall here */
            }
            if (maze[x][i1] != 0) {
                return -1;    /* can't make horiz.  wall here */
            }
            length++;
        }

        for (i1 = dy+1; i1 < RP->Ysize-1; i1++) {
            int sindex = surround_flag2(maze, x, i1, RP);

            if (sindex == 8) {
                break;
            }
            if (sindex != 0) {
                return -1;    /* can't make verti. wall here */
            }
            if (maze[x][i1] != 0) {
                return -1;    /* can't make horiz.  wall here */
            }
            length++;
        }
        return length;
    }
}

/**
 * Cuts the layout horizontally or vertically by a wall with a door.
 * @param maze
 * layout.
 * @param x
 * @param y
 * where to put the door.
 * @param dir
 * wall direction:
 * - 0: horizontally.
 * - 1: vertically.
 * @return
 * 0
 */
int make_wall(char **maze, int x, int y, int dir)
{
    maze[x][y] = 'D'; /* mark a door */
    switch (dir) {
    case 0: { /* horizontal */
        int i1;

        for (i1 = x-1; maze[i1][y] == 0; i1--) {
            maze[i1][y] = '#';
        }
        for (i1 = x+1; maze[i1][y] == 0; i1++) {
            maze[i1][y] = '#';
        }
        break;
    }

    case 1: { /* vertical */
        int i1;

        for (i1 = y-1; maze[x][i1] == 0; i1--) {
            maze[x][i1] = '#';
        }
        for (i1 = y+1; maze[x][i1] == 0; i1++) {
            maze[x][i1] = '#';
        }
        break;
    }
    }

    return 0;
}

/**
 * Puts doors at appropriate locations in a layout.
 * @param maze
 * layout.
 * @param RP
 * map parameters.
 */
void doorify_layout(char **maze, RMParms *RP)
{
    int ndoors = RP->Xsize*RP->Ysize/60;  /* reasonable number of doors. */
    int *doorlist_x;
    int *doorlist_y;
    int doorlocs = 0;  /* # of available doorlocations */
    int i, j;

    doorlist_x = malloc(sizeof(int)*RP->Xsize*RP->Ysize);
    doorlist_y = malloc(sizeof(int)*RP->Xsize*RP->Ysize);


    /* make a list of possible door locations */
    for (i = 1; i < RP->Xsize-1; i++)
        for (j = 1; j < RP->Ysize-1; j++) {
            int sindex = surround_flag(maze, i, j, RP);
            if (sindex == 3 || sindex == 12) {
                /* these are possible door sindex*/
                doorlist_x[doorlocs] = i;
                doorlist_y[doorlocs] = j;
                doorlocs++;
            }
        }

    while (ndoors > 0 && doorlocs > 0) {
        int di;
        int sindex;

        di = RANDOM()%doorlocs;
        i = doorlist_x[di];
        j = doorlist_y[di];
        sindex = surround_flag(maze, i, j, RP);
        if (sindex == 3 || sindex == 12) { /* these are possible door sindex*/
            maze[i][j] = 'D';
            ndoors--;
        }
        /* reduce the size of the list */
        doorlocs--;
        doorlist_x[di] = doorlist_x[doorlocs];
        doorlist_y[di] = doorlist_y[doorlocs];
    }
    free(doorlist_x);
    free(doorlist_y);
}

/**
 * Creates a suitable message for exit from RP.
 * @param RP
 * parameters to convert to message.
 * @return
 * new StringBuffer containing the message.
 */
StringBuffer *write_map_parameters_to_string(RMParms *RP)
{
    StringBuffer *buf;

    buf = stringbuffer_new();
    stringbuffer_append_printf(buf, "xsize %d\nysize %d\n", RP->Xsize, RP->Ysize);

    if (RP->wallstyle[0]) {
        stringbuffer_append_printf(buf, "wallstyle %s\n", RP->wallstyle);
    }

    if (RP->floorstyle[0]) {
        stringbuffer_append_printf(buf, "floorstyle %s\n", RP->floorstyle);
    }

    if (RP->monsterstyle[0]) {
        stringbuffer_append_printf(buf, "monsterstyle %s\n", RP->monsterstyle);
    }

    if (RP->treasurestyle[0]) {
        stringbuffer_append_printf(buf, "treasurestyle %s\n", RP->treasurestyle);
    }

    if (RP->layoutstyle[0]) {
        stringbuffer_append_printf(buf, "layoutstyle %s\n", RP->layoutstyle);
    }

    if (RP->decorstyle[0]) {
        stringbuffer_append_printf(buf, "decorstyle %s\n", RP->decorstyle);
    }

    if (RP->doorstyle[0]) {
        stringbuffer_append_printf(buf, "doorstyle %s\n", RP->doorstyle);
    }

    if (RP->exitstyle[0]) {
        stringbuffer_append_printf(buf, "exitstyle %s\n", RP->exitstyle);
    }

    if (RP->final_map[0]) {
        stringbuffer_append_printf(buf, "final_map %s\n", RP->final_map);
    }

    if (RP->final_exit_archetype[0]) {
        stringbuffer_append_printf(buf, "final_exit_archetype %s\n", RP->final_exit_archetype);
    }

    if (RP->exit_on_final_map[0]) {
        stringbuffer_append_printf(buf, "exit_on_final_map %s\n", RP->exit_on_final_map);
    }

    if (RP->this_map[0]) {
        stringbuffer_append_printf(buf, "origin_map %s\n", RP->this_map);
    }

    if (RP->expand2x) {
        stringbuffer_append_printf(buf, "expand2x %d\n", RP->expand2x);
    }

    if (RP->layoutoptions1) {
        stringbuffer_append_printf(buf, "layoutoptions1 %d\n", RP->layoutoptions1);
    }

    if (RP->layoutoptions2) {
        stringbuffer_append_printf(buf, "layoutoptions2 %d\n", RP->layoutoptions2);
    }

    if (RP->symmetry) {
        stringbuffer_append_printf(buf, "symmetry %d\n", RP->symmetry);
    }

    if (RP->difficulty && RP->difficulty_given) {
        stringbuffer_append_printf(buf, "difficulty %d\n", RP->difficulty);
    }

    if (RP->difficulty_increase != 1.0) {
        stringbuffer_append_printf(buf, "difficulty_increase %f\n", RP->difficulty_increase);
    }

    stringbuffer_append_printf(buf, "dungeon_level %d\n", RP->dungeon_level);

    if (RP->dungeon_depth) {
        stringbuffer_append_printf(buf, "dungeon_depth %d\n", RP->dungeon_depth);
    }

    if (RP->dungeon_name[0]) {
        stringbuffer_append_printf(buf, "dungeon_name %s\n", RP->dungeon_name);
    }

    if (RP->decoroptions) {
        stringbuffer_append_printf(buf, "decoroptions %d\n", RP->decoroptions);
    }

    if (RP->orientation) {
        stringbuffer_append_printf(buf, "orientation %d\n", RP->orientation);
    }

    if (RP->origin_x) {
        stringbuffer_append_printf(buf, "origin_x %d\n", RP->origin_x);
    }

    if (RP->origin_y) {
        stringbuffer_append_printf(buf, "origin_y %d\n", RP->origin_y);
    }
    if (RP->random_seed) {
        /* Add one so that the next map is a bit different */
        stringbuffer_append_printf(buf, "random_seed %d\n", RP->random_seed+1);
    }

    if (RP->treasureoptions) {
        stringbuffer_append_printf(buf, "treasureoptions %d\n", RP->treasureoptions);
    }

    if (RP->multiple_floors) {
        stringbuffer_append_printf(buf, "multiple_floors %d\n", RP->multiple_floors);
    }

    return buf;
}
