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
 * Expands a layout by 2x in each dimension.
 *
 * H. S. Teoh
 */

#include <stdlib.h>   /* just in case */
#include <expand2x.h>   /* use compiler to do sanity check */


/* PROTOTYPES */

static void expand_misc(char **newlayout, int i, int j, char **layout);
static void expand_wall(char **newlayout, int i, int j, char **layout, int xsize, int ysize);
static void expand_door(char **newlayout, int i, int j, char **layout, int xsize, int ysize);

/* FUNCTIONS */

/**
 * Expands the layout be a factor 2. Doors and walls are taken care of.
 * @param layout
 * layout to expand. Memory is free()d at the end, so pointer becomes invalid.
 * @param xsize
 * @param ysize
 * layout size.
 * @return
 * new layout. Must be free()d by caller.
 */
char **expand2x(char **layout, int xsize, int ysize)
{
    int i, j;
    int nxsize = xsize*2-1;
    int nysize = ysize*2-1;

    /* Allocate new layout */
    char **newlayout = (char **)calloc(sizeof(char *), nxsize);
    for (i = 0; i < nxsize; i++) {
        newlayout[i] = (char *)calloc(sizeof(char), nysize);
    }

    for (i = 0; i < xsize; i++) {
        for (j = 0; j < ysize; j++) {
            switch (layout[i][j]) {
            case '#':
                expand_wall(newlayout, i, j, layout, xsize, ysize);
                break;

            case 'D':
                expand_door(newlayout, i, j, layout, xsize, ysize);
                break;

            default:
                expand_misc(newlayout, i, j, layout);
            }
        }
    }

    /* Dump old layout */
    for (i = 0; i < xsize; i++) {
        free(layout[i]);
    }
    free(layout);

    return newlayout;
}

/**
 * Copy the old tile X into the new one at location (i*2, j*2) and
 * fill up the rest of the 2x2 result with 0:
 * X ---> X  0
 *        0  0
 * @param newlayout
 * map layout.
 * @param i
 * @param j
 * spot to expand.
 * @param layout
 * map layout.
 */
static void expand_misc(char **newlayout, int i, int j, char **layout)
{
    newlayout[i*2][j*2] = layout[i][j];
    /* (Note: no need to reset rest of 2x2 area to \0 because calloc does that
     * for us.) */
}

/**
 * Returns a bitmap that represents which squares on the right and bottom
 * edges of a square (i,j) match the given character.
 *
 * @param ch
 * character to look for.
 * @param layout
 * map.
 * @param i
 * @param j
 * spot where to look.
 * @param xsize
 * @param ysize
 * layout size.
 * @return
 * combination of the following values:
 * - 1 means match on (i+1, j).
 * - 2 means match on (i, j+1).
 * - 4 means match on (i+1, j+1).
 */
static int calc_pattern(char ch, char **layout, int i, int j, int xsize, int ysize)
{
    int pattern = 0;

    if (i+1 < xsize && layout[i+1][j] == ch) {
        pattern |= 1;
    }

    if (j+1 < ysize) {
        if (layout[i][j+1] == ch) {
            pattern |= 2;
        }
        if (i+1 < xsize && layout[i+1][j+1] == ch) {
            pattern |= 4;
        }
    }

    return pattern;
}

/**
 * Expand a wall. This function will try to sensibly connect the resulting
 * wall to adjacent wall squares, so that the result won't have disconnected
 * walls.
 * @param newlayout
 * map layout.
 * @param i
 * @param j
 * coordinates of wall to expand in non expanded layout.
 * @param layout
 * current (non expanded) layout.
 * @param xsize
 * @param ysize
 * sizes of layout.
 */
static void expand_wall(char **newlayout, int i, int j, char **layout, int xsize, int ysize)
{
    int wall_pattern = calc_pattern('#', layout, i, j, xsize, ysize);
    int door_pattern = calc_pattern('D', layout, i, j, xsize, ysize);
    int both_pattern = wall_pattern|door_pattern;

    newlayout[i*2][j*2] = '#';
    if (i+1 < xsize) {
        if (both_pattern&1) {
            /* join walls/doors to the right */
            newlayout[i*2+1][j*2] = layout[i+1][j];
        }
    }

    if (j+1 < ysize) {
        if (both_pattern&2) {
            /* join walls/doors to the bottom */
            newlayout[i*2][j*2+1] = layout[i][j+1];
        }

        if (wall_pattern == 7) {
            /* if orig layout is a 2x2 wall block,
             * we fill the result with walls. */
            newlayout[i*2+1][j*2+1] = '#';
        }
    }
}

/**
 * Expand a door. This function will try to sensibly connect doors so that they meet up with
 * adjacent walls. Note that it will also presumptuously delete (ignore) doors
 * that it doesn't know how to correctly expand.
 * @param newlayout
 * expanded layout.
 * @param i
 * @param j
 * coordinates of door to expand in non expanded layout.
 * @param layout
 * non expanded layout.
 * @param xsize
 * @param ysize
 * size of non expanded layout.
 */
static void expand_door(char **newlayout, int i, int j, char **layout, int xsize, int ysize)
{
    int wall_pattern = calc_pattern('#', layout, i, j, xsize, ysize);
    int door_pattern = calc_pattern('D', layout, i, j, xsize, ysize);
    int join_pattern;

    /* Doors "like" to connect to walls more than other doors. If there is
     * a wall and another door, this door will connect to the wall and
     * disconnect from the other door. */
    if (wall_pattern&3) {
        join_pattern = wall_pattern;
    } else {
        join_pattern = door_pattern;
    }

    newlayout[i*2][j*2] = 'D';
    if (i+1 < xsize) {
        if (join_pattern&1) {
            /* there is a door/wall to the right */
            newlayout[i*2+1][j*2] = 'D';
        }
    }

    if (j+1 < ysize) {
        if (join_pattern&2) {
            /* there is a door/wall below */
            newlayout[i*2][j*2+1] = 'D';
        }
    }
}
