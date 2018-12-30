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
 * Square-spiral layout generator.
 * @todo
 * what does that look like? :)
 */

/* peterm@langmuir.eecs.berkeley.edu:  this function generates a random
snake-type layout.
*/

#include <stdlib.h>
#include <stdio.h>
#include <global.h>
#include <time.h>

#include <maze_gen.h>
#include <room_gen.h>
#include <random_map.h>
#include <sproto.h>
#include <rproto.h>

char **map_gen_onion(int xsize, int ysize, int option, int layers);

/* These are some helper functions which help with
   manipulating a centered onion and turning it into
   a square spiral */

/**
 * This starts from within a centered onion layer (or between two layers),
 * and looks up until it finds a wall, and then looks right until it
 * finds a vertical wall, i.e., the corner.  It sets cx and cy to that.
 * it also starts from cx and cy.
 * @param maze
 * where to look.
 * @param cx
 * @param cy
 * where to start from, and detected corner.
 */
void find_top_left_corner(char **maze, int *cx, int *cy)
{
    (*cy)--;
    /* find the top wall. */
    while (maze[*cx][*cy] == 0) {
        (*cy)--;
    }
    /* proceed right until a corner is detected */
    while (maze[*cx][*cy+1] == 0) {
        (*cx)++;
    }

    /* cx and cy should now be the top-right corner of the onion layer */
}

/**
 * Generates a square-spiral layout.
 * @param xsize
 * @param ysize
 * size of the layout.
 * @return
 * generated layout.
 * @todo
 * use function in another file for character searching.
 */
char **make_square_spiral_layout(int xsize, int ysize)
{
    int i, j;
    int cx, cy;
    int tx, ty;

    /* generate and allocate a doorless, centered onion */
    char **maze = map_gen_onion(xsize, ysize, OPT_CENTERED|OPT_NO_DOORS, 0);

    /* find the layout center.  */
    cx = 0;
    cy = 0;
    for (i = 0; i < xsize; i++)
        for (j = 0; j < ysize; j++) {
            if (maze[i][j] == 'C') {
                cx = i;
                cy = j;
            }
        }
    tx = cx;
    ty = cy;
    while (1) {
        find_top_left_corner(maze, &tx, &ty);

        if (ty < 2 || tx < 2 || tx > xsize-2 || ty > ysize-2) {
            break;
        }
        make_wall(maze, tx, ty-1, 1);  /* make a vertical wall with a door */

        maze[tx][ty-1] = '#'; /* convert the door that make_wall puts here to a wall */
        maze[tx-1][ty] = 'D';/* make a doorway out of this layer */

        /* walk left until we find the top-left corner */
        while ((tx > 2) && maze[tx-1][ty]) {
            tx--;
        }

        make_wall(maze, tx-1, ty, 0);     /* make a horizontal wall with a door */

        /* walk down until we find the bottom-left corner */
        while (((ty+1) < ysize) && maze[tx][ty+1]) {
            ty++;
        }

        make_wall(maze, tx, ty+1, 1);    /* make a vertical wall with a door */

        /* walk rightuntil we find the bottom-right corner */
        while (((tx+1) < xsize) && maze[tx+1][ty]) {
            tx++;
        }

        make_wall(maze, tx+1, ty, 0);   /* make a horizontal wall with a door */
        tx++;  /* set up for next layer. */
    }

    /* place the exits.  */
    if (RANDOM()%2) {
        maze[cx][cy] = '>';
        maze[xsize-2][1] = '<';
    } else {
        maze[cx][cy] = '<';
        maze[xsize-2][1] = '>';
    }

    return maze;
}
