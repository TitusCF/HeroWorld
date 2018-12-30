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
 * @file random_maps/rogue_layout.c
 * Rogue/NetHack style room generation.
 */

#include <math.h>

#include "global.h"
#include "random_map.h"

typedef struct {
    /** Coordinates of room centers */
    int x, y;

    /** Room size */
    int sx, sy;

    /** Coordinates of the extrema of the rectangle */
    int ax, ay, zx, zy;

    /* Room type (circular or rectangular) */
    int rtype;
} Room;

/**
 * Place a room in the layout.
 * @param Rooms
 * list of existing rooms, new room will be added to it.
 * @param xsize
 * @param ysize
 * layout size.
 * @param nrooms
 * wanted number of room, used to determine size.
 * @return
 * 0 if no room could be generated, 1 else.
 */
static int roguelike_place_room(Room *Rooms, int xsize, int ysize, int nrooms) {
    int tx, ty;  /* trial center locations */
    int sx, sy;  /* trial sizes */
    int ax, ay;  /* min coords of rect */
    int zx, zy;  /* max coords of rect */
    int x_basesize;
    int y_basesize;
    Room *walk;

    /* decide on the base x and y sizes */

    x_basesize = xsize/isqrt(nrooms);
    y_basesize = ysize/isqrt(nrooms);


    tx = RANDOM()%xsize;
    ty = RANDOM()%ysize;

    /* generate a distribution of sizes centered about basesize */
    sx = (RANDOM()%x_basesize)+(RANDOM()%x_basesize)+(RANDOM()%x_basesize);
    sy = (RANDOM()%y_basesize)+(RANDOM()%y_basesize)+(RANDOM()%y_basesize);
    sy = (int)(sy*.5);   /* renormalize */

    /* find the corners */
    ax = tx-sx/2;
    zx = tx+sx/2+sx%2;

    ay = ty-sy/2;
    zy = ty+sy/2+sy%2;

    /* check to see if it's in the map */
    if (zx > xsize-1 || ax < 1) {
        return 0;
    }
    if (zy > ysize-1 || ay < 1) {
        return 0;
    }

    /* no small fish */
    if (sx < 3 || sy < 3) {
        return 0;
    }

    /* check overlap with existing rooms */
    for (walk = Rooms; walk->x != 0; walk++) {
        int dx = abs(tx-walk->x);
        int dy = abs(ty-walk->y);

        if ((dx < (walk->sx+sx)/2+2) && (dy < (walk->sy+sy)/2+2)) {
            return 0;
        }
    }

    /* if we've got here, presumably the room is OK. */

    /* get a pointer to the first free room */
    for (walk = Rooms; walk->x != 0; walk++)
        ;
    walk->x = tx;
    walk->y = ty;
    walk->sx = sx;
    walk->sy = sy;
    walk->ax = ax;
    walk->ay = ay;
    walk->zx = zx;
    walk->zy = zy;
    return 1;  /* success */
}

/**
 * Write all the rooms into the maze.
 * @param Rooms
 * list of rooms to write.
 * @param maze
 * where to write to.
 * @param options
 * 2 to have circular rooms, 1 for rectanglar ones, another value for random choice.
 */
static void roguelike_make_rooms(Room *Rooms, char **maze, int options) {
    int making_circle = 0;
    int i, j;
    int R;
    Room *walk;

    for (walk = Rooms; walk->x != 0; walk++) {
        /* first decide what shape to make */
        switch (options) {
        case 1:
            making_circle = 0;
            break;

        case 2:
            making_circle = 1;
            break;

        default:
            making_circle = ((RANDOM()%3 == 0) ? 1 : 0);
            break;
        }

        if (walk->sx < walk->sy) {
            R = walk->sx/2;
        } else {
            R = walk->sy/2;
        }

        /* enscribe a rectangle or a circle */
        for (i = walk->ax; i < walk->zx; i++)
            for (j = walk->ay; j < walk->zy; j++) {
                if (!making_circle || ((int)(0.5+hypot(walk->x-i, walk->y-j))) <= R) {
                    maze[i][j] = '.';
                }
            }
    }
}

/**
 * Link generated rooms with corridors.
 * @param Rooms
 * room list.
 * @param maze
 * maze.
 * @param xsize
 * @param ysize
 * maze size.
 */
static void roguelike_link_rooms(Room *Rooms, char **maze, int xsize, int ysize) {
    Room *walk;
    int i, j;

    /* link each room to the previous room */
    if (Rooms[1].x == 0) {
        return;    /* only 1 room */
    }

    for (walk = Rooms+1; walk->x != 0; walk++) {
        int x1 = walk->x;
        int y1 = walk->y;
        int x2 = (walk-1)->x;
        int y2 = (walk-1)->y;
        int in_wall = 0;

        if (RANDOM()%2) {     /* connect in x direction first */
            /* horizontal connect */
            /* swap (x1,y1) (x2,y2) if necessary */

            if (x2 < x1) {
                int tx = x2, ty = y2;
                x2 = x1;
                y2 = y1;
                x1 = tx;
                y1 = ty;
            }

            j = y1;
            for (i = x1; i < x2; i++) {
                if (in_wall == 0 && maze[i][j] == '#') {
                    in_wall = 1;
                    maze[i][j] = 'D';
                } else if (in_wall && maze[i][j] == '.') {
                    in_wall = 0;
                    maze[i-1][j] = 'D';
                } else if (maze[i][j] != 'D' && maze[i][j] != '.') {
                    maze[i][j] = 0;
                }
            }
            j = MIN(y1, y2);
            if (maze[i][j] == '.') {
                in_wall = 0;
            }
            if (maze[i][j] == 0 || maze[i][j] == '#') {
                in_wall = 1;
            }
            for (/* j set already */; j < MAX(y1, y2); j++) {
                if (in_wall == 0 && maze[i][j] == '#') {
                    in_wall = 1;
                    maze[i][j] = 'D';
                } else if (in_wall && maze[i][j] == '.') {
                    in_wall = 0;
                    maze[i][j-1] = 'D';
                } else if (maze[i][j] != 'D' && maze[i][j] != '.') {
                    maze[i][j] = 0;
                }
            }
        } else { /* connect in y direction first */
            in_wall = 0;
            /* swap if necessary */
            if (y2 < y1) {
                int tx = x2, ty = y2;
                x2 = x1;
                y2 = y1;
                x1 = tx;
                y1 = ty;
            }
            i = x1;
            /* vertical connect */
            for (j = y1; j < y2; j++) {
                if (in_wall == 0 && maze[i][j] == '#') {
                    in_wall = 1;
                    maze[i][j] = 'D';
                } else if (in_wall && maze[i][j] == '.') {
                    in_wall = 0;
                    maze[i][j-1] = 'D';
                } else if (maze[i][j] != 'D' && maze[i][j] != '.') {
                    maze[i][j] = 0;
                }
            }

            i = MIN(x1, x2);
            if (maze[i][j] == '.') {
                in_wall = 0;
            }
            if (maze[i][j] == 0 || maze[i][j] == '#') {
                in_wall = 1;
            }
            for (/* i set already */; i < MAX(x1, x2); i++) {
                if (in_wall == 0 && maze[i][j] == '#') {
                    in_wall = 1;
                    maze[i][j] = 'D';
                } else if (in_wall && maze[i][j] == '.') {
                    in_wall = 0;
                    maze[i-1][j] = 'D';
                } else if (maze[i][j] != 'D' && maze[i][j] != '.') {
                    maze[i][j] = 0;
                }
            }
        }
    }
}

/**
 * Checks free spots around a spot.
 * @param layout
 * map layout.
 * @param i
 * @param j
 * coordinates to check.
 * @param Xsize
 * @param Ysize
 * size of the layout.
 * @return
 * combination of the following values:
 * - 1 = free space to left,
 * - 2 = free space to right,
 * - 4 = free space above
 * - 8 = free space below
 * @todo
 * there is an equivalent function in another layout, merge them together.
 */
int surround_check(char **layout, int i, int j, int Xsize, int Ysize) {
    int surround_index = 0;

    if ((i > 0) && (layout[i-1][j] != 0 && layout[i-1][j] != '.')) {
        surround_index += 1;
    }
    if ((i < Xsize-1) && (layout[i+1][j] != 0 && layout[i+1][j] != '.')) {
        surround_index += 2;
    }
    if ((j > 0) && (layout[i][j-1] != 0 && layout[i][j-1] != '.')) {
        surround_index += 4;
    }
    if ((j < Ysize-1) && (layout[i][j+1] != 0 && layout[i][j+1] != '.')) {
        surround_index += 8;
    }
    return surround_index;
}

/**
 * Actually make the rogue layout. We work by a reduction process:
 * first we make everything a wall, then we remove areas to make rooms
 * @param xsize
 * @param ysize
 * wanted layout size.
 * @param options
 * 2 to have circular rooms, 1 for rectanglar ones, another value for random choice.
 * @return
 * generated layout.
 */
char **roguelike_layout_gen(int xsize, int ysize, int options) {
    int i, j;
    Room *Rooms = NULL;
    Room *walk;
    int nrooms = 0;
    int tries = 0;

    /* allocate that array, write walls everywhere up */
    char **maze = (char **)malloc(sizeof(char *)*xsize);
    for (i = 0; i < xsize; i++) {
        maze[i] = (char *)malloc(sizeof(char)*ysize);
        for (j = 0; j < ysize; j++) {
            maze[i][j] = '#';
        }
    }

    /* minimum room size is basically 5x5:  if xsize/ysize is
     less than 3x that then hollow things out, stick in
     a stairsup and stairs down, and exit */

    if (xsize < 11 || ysize < 11) {
        for (i = 1; i < xsize-1; i++)
            for (j = 1; j < ysize-1; j++) {
                maze[i][j] = 0;
            }
        maze[(xsize-1)/2][(ysize-1)/2] = '>';
        maze[(xsize-1)/2][(ysize-1)/2+1] = '<';
        return maze;
    }

    /* decide on the number of rooms */
    nrooms = RANDOM()%10+6;
    Rooms = (Room *)calloc(nrooms+1, sizeof(Room));

    /* actually place the rooms */
    i = 0;
    while (tries < 450 && i < nrooms) {
        /* try to place the room */
        if (!roguelike_place_room(Rooms, xsize, ysize, nrooms)) {
            tries++;
        } else {
            i++;
        }
    }

    if (i == 0) { /* no can do! */
        for (i = 1; i < xsize-1; i++)
            for (j = 1; j < ysize-1; j++) {
                maze[i][j] = 0;
            }
        maze[(xsize-1)/2][(ysize-1)/2] = '>';
        maze[(xsize-1)/2][(ysize-1)/2+1] = '<';
        free(Rooms);
        return maze;
    }

    /* erase the areas occupied by the rooms */
    roguelike_make_rooms(Rooms, maze, options);

    roguelike_link_rooms(Rooms, maze, xsize, ysize);

    /* put in the stairs */

    maze[Rooms->x][Rooms->y] = '<';
    /* get the last one */
    for (walk = Rooms; walk->x != 0; walk++)
        ;
    /* back up one */
    walk--;
    if (walk == Rooms) {
        /* In this case, there is only a single room.  We don't want to
         * clobber are up exit (above) with a down exit, so put the
         * other exit one space up/down, depending which is a space
         * and not a wall.
         */
        if (maze[walk->x][walk->y+1] == '.') {
            maze[walk->x][walk->y+1] = '>';
        } else {
            maze[walk->x][walk->y-1] = '>';
        }
    } else {
        maze[walk->x][walk->y] = '>';
    }

    /* convert all the '.' to 0, we're through with the '.' */
    for (i = 0; i < xsize; i++)
        for (j = 0; j < ysize; j++) {
            if (maze[i][j] == '.') {
                maze[i][j] = 0;
            }
            if (maze[i][j] == 'D') { /* remove bad door. */
                int si = surround_check(maze, i, j, xsize, ysize);

                if (si != 3 && si != 12) {
                    maze[i][j] = 0;
                    /* back up and recheck any nearby doors */
                    i = 0;
                    j = 0;
                }
            }
        }

    free(Rooms);
    return maze;
}
