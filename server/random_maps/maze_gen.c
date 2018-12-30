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
 * General maze generator.
 * @author
 * peterm@langmuir.eecs.berkeley.edu
 */

/* we need to maintain a list of wall points to generate
 * reasonable mazes:  a straightforward recursive random walk maze
 * generator would generate a map with a trivial circle-the-outer-wall solution
 */

#include <stdlib.h>
#include <stdio.h>
#include <global.h>
#include <maze_gen.h>
#include <time.h>

/** Contains free walls in the map. */
typedef struct free_walls_struct {
    int *wall_x_list;       /**< X coordinates of free spots for walls. */
    int *wall_y_list;       /**< Y coordinates of free spots for walls. */
    int wall_free_size;     /**< Number of items in wall_x_list and wall_y_list. */
} free_walls_struct;

static void fill_maze_full(char **maze, int x, int y, int xsize, int ysize, free_walls_struct *);
static void fill_maze_sparse(char **maze, int x, int y, int xsize, int ysize, free_walls_struct *);
static void make_wall_free_list(int xsize, int ysize, free_walls_struct *);
static void pop_wall_point(int *x, int *y, free_walls_struct *);
static int find_free_point(char **maze, int *x, int *y, int xc, int yc, int xsize, int ysize);

/**
 * This function generates a random blocked maze with the property that there is only one path from one spot
 * to any other, and there is always a path from one spot to any other.
 * @param xsize
 * @param ysize
 * wanted map size.
 * @param option
 * if 0, maze will be sparse (sizeable rooms), else totally filled.
 * @return
 * a char** array with # and . for closed and open respectively.
 * a char value of 0 represents a blank space:  a '#' is
 * a wall.
*/
char **maze_gen(int xsize, int ysize, int option)
{
    int i, j;
    struct free_walls_struct free_walls;

    /* allocate that array, set it up */
    char **maze = (char **)calloc(sizeof(char *), xsize);
    for (i = 0; i < xsize; i++) {
        maze[i] = (char *)calloc(sizeof(char), ysize);
    }

    /* write the outer walls */
    for (i = 0; i < xsize; i++) {
        maze[i][0] = maze[i][ysize-1] = '#';
    }
    for (j = 0; j < ysize; j++) {
        maze[0][j] = maze[xsize-1][j] = '#';
    }


    /* find how many free wall spots there are */
    free_walls.wall_free_size = 2*(xsize-4)+2*(ysize-4);
    free_walls.wall_x_list = NULL;
    free_walls.wall_y_list = NULL;

    make_wall_free_list(xsize, ysize, &free_walls);

    /* return the empty maze */
    if (free_walls.wall_free_size <= 0) {
        return maze;
    }

    /* recursively generate the walls of the maze */
    /* first pop a random starting point */
    while (free_walls.wall_free_size > 0) {
        pop_wall_point(&i, &j, &free_walls);
        if (option) {
            fill_maze_full(maze, i, j, xsize, ysize, &free_walls);
        } else {
            fill_maze_sparse(maze, i, j, xsize, ysize, &free_walls);
        }
    }

    /* clean up our intermediate data structures. */
    free(free_walls.wall_x_list);
    free(free_walls.wall_y_list);

    return maze;
}

/**
 * Inits the list of points where we can put walls on.
 * The free wall points are those outer points which aren't corners or
 * near corners, and don't have a maze wall growing out of them already.
 * @param xsize
 * @param ysize
 * size of the map.
 * @param free_walls
 * structure to initialise. free_walls_struct::wall_free_size must be initialised.
 */
static void make_wall_free_list(int xsize, int ysize, free_walls_struct *free_walls)
{
    int i, j, count;

    count = 0;  /* entries already placed in the free list */
    /*allocate it*/
    if (free_walls->wall_free_size < 0) {
        return;
    }
    free_walls->wall_x_list = (int *)calloc(sizeof(int), free_walls->wall_free_size);
    free_walls->wall_y_list = (int *)calloc(sizeof(int), free_walls->wall_free_size);


    /* top and bottom wall */
    for (i = 2; i < xsize-2; i++) {
        free_walls->wall_x_list[count] = i;
        free_walls->wall_y_list[count] = 0;
        count++;
        free_walls->wall_x_list[count] = i;
        free_walls->wall_y_list[count] = ysize-1;
        count++;
    }

    /* left and right wall */
    for (j = 2; j < ysize-2; j++) {
        free_walls->wall_x_list[count] = 0;
        free_walls->wall_y_list[count] = j;
        count++;
        free_walls->wall_x_list[count] = xsize-1;
        free_walls->wall_y_list[count] = j;
        count++;
    }
}

/**
 * Randomly returns one of the elements from the wall point list.
 * @param[out] x
 * @param[out] y
 * coordinates of the point.
 * @param free_walls
 * free walls list.
 */
static void pop_wall_point(int *x, int *y, free_walls_struct *free_walls)
{
    int index = RANDOM()%free_walls->wall_free_size;
    *x = free_walls->wall_x_list[index];
    *y = free_walls->wall_y_list[index];
    /* write the last array point here */
    free_walls->wall_x_list[index] = free_walls->wall_x_list[free_walls->wall_free_size-1];
    free_walls->wall_y_list[index] = free_walls->wall_y_list[free_walls->wall_free_size-1];
    free_walls->wall_free_size--;
}

/**
 * Randomly look for a square adjacent to this one where
 * we can place a new block without closing a path.  We may only look
 * up, down, right, or left.
 * @param maze
 * current maze.
 * @param[out] x
 * @param[out] y
 * coordinates of the found point.
 * @param xc
 * @param yc
 * coordinates from where to look.
 * @param xsize
 * @param ysize
 * maze size.
 * @return
 * -1 if no free spot, 0 else.
 */
static int find_free_point(char **maze, int *x, int *y, int xc, int yc, int xsize, int ysize)
{
    /* we will randomly pick from this list, 1=up, 2=down, 3=right, 4=left */
    int dirlist[4];
    int count = 0;  /* # elements in dirlist */

    /* look up */
    if (yc < ysize-2 && xc > 2 && xc < xsize-2) {
        int cleartest = (int)maze[xc][yc+1]+(int)maze[xc-1][yc+1]+(int)maze[xc+1][yc+1];

        cleartest += (int)maze[xc][yc+2]+(int)maze[xc-1][yc+2]+(int)maze[xc+1][yc+2];
        if (cleartest == 0) {
            dirlist[count] = 1;
            count++;
        }
    }

    /* look down */
    if (yc > 2 && xc > 2 && xc < xsize-2) {
        int cleartest = (int)maze[xc][yc-1]+(int)maze[xc-1][yc-1]+(int)maze[xc+1][yc-1];

        cleartest += (int)maze[xc][yc-2]+(int)maze[xc-1][yc-2]+(int)maze[xc+1][yc-2];
        if (cleartest == 0) {
            dirlist[count] = 2;
            count++;
        }
    }


    /* look right */
    if (xc < xsize-2 && yc > 2 && yc < ysize-2) {
        int cleartest = (int)maze[xc+1][yc]+(int)maze[xc+1][yc-1]+(int)maze[xc+1][yc+1];

        cleartest += (int)maze[xc+2][yc]+(int)maze[xc+2][yc-1]+(int)maze[xc+2][yc+1];
        if (cleartest == 0) {
            dirlist[count] = 3;
            count++;
        }
    }


    /* look left */
    if (xc > 2 && yc > 2 && yc < ysize-2) {
        int cleartest = (int)maze[xc-1][yc]+(int)maze[xc-1][yc-1]+(int)maze[xc-1][yc+1];

        cleartest += (int)maze[xc-2][yc]+(int)maze[xc-2][yc-1]+(int)maze[xc-2][yc+1];
        if (cleartest == 0) {
            dirlist[count] = 4;
            count++;
        }
    }

    if (count == 0) {
        return -1;    /* failed to find any clear points */
    }

    /* choose a random direction */
    if (count > 1) {
        count = RANDOM()%count;
    } else {
        count = 0;
    }
    switch (dirlist[count]) {
    case 1: { /* up */
        *y = yc+1;
        *x = xc;
        break;
    };

    case 2: { /* down */
        *y = yc-1;
        *x = xc;
        break;
    };

    case 3: { /* right */
        *y = yc;
        *x = xc+1;
        break;
    }

    case 4: { /* left */
        *x = xc-1;
        *y = yc;
        break;
    }

    default: { /* ??? */
        return -1;
    }
    }
    return 1;
}

/**
 * Recursive routine which will fill every available space in the maze
 * with walls.
 *
 * @param maze
 * maze.
 * @param x
 * @param y
 * where to put a wall.
 * @param xsize
 * @param ysize
 * maze size.
 * @param free_walls
 * free walls list.
 */
static void fill_maze_full(char **maze, int x, int y, int xsize, int ysize, free_walls_struct *free_walls)
{
    int xc, yc;

    /* write a wall here */
    maze[x][y] = '#';

    /* decide if we're going to pick from the wall_free_list */
    if (RANDOM()%4 && free_walls->wall_free_size > 0) {
        pop_wall_point(&xc, &yc, free_walls);
        fill_maze_full(maze, xc, yc, xsize, ysize, free_walls);
    }

    /* change the if to a while for a complete maze.  */
    while (find_free_point(maze, &xc, &yc, x, y, xsize, ysize) != -1) {
        fill_maze_full(maze, xc, yc, xsize, ysize, free_walls);
    }
}

/**
 * Recursive routine which will fill much of the maze, but will leave
 * some free spots (possibly large) toward the center.
 * @param maze
 * maze.
 * @param x
 * @param y
 * where to put a wall.
 * @param xsize
 * @param ysize
 * maze size.
 * @param free_walls
 * free walls list.
 */
static void fill_maze_sparse(char **maze, int x, int y, int xsize, int ysize, free_walls_struct *free_walls)
{
    int xc, yc;

    /* write a wall here */
    maze[x][y] = '#';

    /* decide if we're going to pick from the wall_free_list */
    if (RANDOM()%4 && free_walls->wall_free_size > 0) {
        pop_wall_point(&xc, &yc, free_walls);
        fill_maze_sparse(maze, xc, yc, xsize, ysize, free_walls);
    }

    /* change the if to a while for a complete maze.  */
    if (find_free_point(maze, &xc, &yc, x, y, xsize, ysize) != -1) {
        fill_maze_sparse(maze, xc, yc, xsize, ysize, free_walls);
    }
}
