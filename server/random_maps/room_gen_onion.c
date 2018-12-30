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
 * The onion room generator:
Onion rooms are like this:

char **map_gen_onion(int xsize, int ysize, int option, int layers);

like this:
regular                       random
centered, linear onion        bottom/right centered, nonlinear

#########################     #########################
#                       #     #                       #
# ########  ##########  #     #   #####################
# #                  #  #     #   #                   #
# # ######  ######## #  #     #   #                   #
# # #              # #  #     #   #   ######## ########
# # # ####  ###### # #  #     #   #   #               #
# # # #          # # #  #     #   #   #               #
# # # ############ # #  #     #   #   #  ########### ##
# # #              # #  #     #   #   #  #            #
# # ################ #  #     #   #   #  #    #########
# #                  #  #     #       #  #    #       #
# ####################  #     #   #   #  #            #
#                       #     #   #   #  #    #       #
#########################     #########################

*/

#include <stdlib.h>
#include <global.h>
#include <random_map.h>

#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

void centered_onion(char **maze, int xsize, int ysize, int option, int layers);
void bottom_centered_onion(char **maze, int xsize, int ysize, int option, int layers);
void bottom_right_centered_onion(char **maze, int xsize, int ysize, int option, int layers);

void draw_onion(char **maze, float *xlocations, float *ylocations, int layers);
void make_doors(char **maze, float *xlocations, float *ylocations, int layers, int options);

/**
 * Generates an onion layout.
 * @param xsize
 * @param ysize
 * layout size.
 * @param option
 * combination of @ref OPT_xxx "OPT_xxx" values.
 * @param layers
 * number of layers the onion should have.
 * @return
 * layout.
 */
char **map_gen_onion(int xsize, int ysize, int option, int layers)
{
    int i, j;

    /* allocate that array, set it up */
    char **maze = (char **)calloc(sizeof(char *), xsize);
    for (i = 0; i < xsize; i++) {
        maze[i] = (char *)calloc(sizeof(char), ysize);
    }

    /* pick some random options if option = 0 */
    if (option == 0) {
        switch (RANDOM()%3) {
        case 0:
            option |= OPT_CENTERED;
            break;

        case 1:
            option |= OPT_BOTTOM_C;
            break;

        case 2:
            option |= OPT_BOTTOM_R;
            break;
        }
        if (RANDOM()%2) {
            option |= OPT_LINEAR;
        }
        if (RANDOM()%2) {
            option |= OPT_IRR_SPACE;
        }
    }

    /* write the outer walls, if appropriate. */
    if (!(option&OPT_WALL_OFF)) {
        for (i = 0; i < xsize; i++) {
            maze[i][0] = maze[i][ysize-1] = '#';
        }
        for (j = 0; j < ysize; j++) {
            maze[0][j] = maze[xsize-1][j] = '#';
        }
    };

    if (option&OPT_WALLS_ONLY) {
        return maze;
    }

    /* pick off the mutually exclusive options */
    if (option&OPT_BOTTOM_R) {
        bottom_right_centered_onion(maze, xsize, ysize, option, layers);
    } else if (option&OPT_BOTTOM_C) {
        bottom_centered_onion(maze, xsize, ysize, option, layers);
    } else if (option&OPT_CENTERED) {
        centered_onion(maze, xsize, ysize, option, layers);
    }

    return maze;
}

/**
 * Creates a centered onion layout.
 * @param maze
 * layout.
 * @param xsize
 * @param ysize
 * layout size.
 * @param option
 * combination of @ref OPT_xxx "OPT_xxx" values.
 * @param layers
 * number of layers to create.
 */
void centered_onion(char **maze, int xsize, int ysize, int option, int layers)
{
    int i, maxlayers;
    float *xlocations;
    float *ylocations;

    maxlayers = (MIN(xsize, ysize)-2)/5;
    if (!maxlayers) {
        return;    /* map too small to onionize */
    }
    if (layers > maxlayers) {
        layers = maxlayers;
    }
    if (layers == 0) {
        layers = (RANDOM()%maxlayers)+1;
    }
    xlocations = (float *)calloc(sizeof(float), 2*layers);
    ylocations = (float *)calloc(sizeof(float), 2*layers);

    /* place all the walls */
    if (option&OPT_IRR_SPACE) { /* randomly spaced */
        int x_spaces_available, y_spaces_available;
        /* the "extra" spaces available for spacing between layers */
        x_spaces_available = (xsize-2)-6*layers+1;
        y_spaces_available = (ysize-2)-6*layers+1;

        /* pick an initial random pitch */
        for (i = 0; i < 2*layers; i++) {
            float xpitch = 2, ypitch = 2;

            if (x_spaces_available > 0)
                xpitch = 2
                         +(RANDOM()%x_spaces_available
                           +RANDOM()%x_spaces_available
                           +RANDOM()%x_spaces_available)/3;

            if (y_spaces_available > 0)
                ypitch = 2
                         +(RANDOM()%y_spaces_available
                           +RANDOM()%y_spaces_available
                           +RANDOM()%y_spaces_available)/3;
            xlocations[i] = ((i > 0) ? xlocations[i-1] : 0)+xpitch;
            ylocations[i] = ((i > 0) ? ylocations[i-1] : 0)+ypitch;
            x_spaces_available -= xpitch-2;
            y_spaces_available -= ypitch-2;
        }
    }
    if (!(option&OPT_IRR_SPACE)) {
        /* evenly spaced */
        float xpitch, ypitch;  /* pitch of the onion layers */

        xpitch = (xsize-2.0)/(2.0*layers+1);
        ypitch = (ysize-2.0)/(2.0*layers+1);
        xlocations[0] = xpitch;
        ylocations[0] = ypitch;
        for (i = 1; i < 2*layers; i++) {
            xlocations[i] = xlocations[i-1]+xpitch;
            ylocations[i] = ylocations[i-1]+ypitch;
        }
    }

    /* draw all the onion boxes.  */
    draw_onion(maze, xlocations, ylocations, layers);
    make_doors(maze, xlocations, ylocations, layers, option);
}

/**
 * Create a bottom-centered layout.
 * @param maze
 * layout.
 * @param xsize
 * @param ysize
 * layout size.
 * @param option
 * combination of @ref OPT_xxx "OPT_xxx" values.
 * @param layers
 * number of layers to create.
 */
void bottom_centered_onion(char **maze, int xsize, int ysize, int option, int layers)
{
    int i, maxlayers;
    float *xlocations;
    float *ylocations;

    maxlayers = (MIN(xsize, ysize)-2)/5;
    if (!maxlayers) {
        return;    /* map too small to onionize */
    }
    if (layers > maxlayers) {
        layers = maxlayers;
    }
    if (layers == 0) {
        layers = (RANDOM()%maxlayers)+1;
    }
    xlocations = (float *)calloc(sizeof(float), 2*layers);
    ylocations = (float *)calloc(sizeof(float), 2*layers);

    /* place all the walls */
    if (option&OPT_IRR_SPACE) { /* randomly spaced */
        int x_spaces_available, y_spaces_available;
        /* the "extra" spaces available for spacing between layers */
        x_spaces_available = (xsize-2)-6*layers+1;
        y_spaces_available = (ysize-2)-3*layers+1;

        /* pick an initial random pitch */
        for (i = 0; i < 2*layers; i++) {
            float xpitch = 2, ypitch = 2;

            if (x_spaces_available > 0)
                xpitch = 2
                         +(RANDOM()%x_spaces_available
                           +RANDOM()%x_spaces_available
                           +RANDOM()%x_spaces_available)/3;

            if (y_spaces_available > 0)
                ypitch = 2
                         +(RANDOM()%y_spaces_available
                           +RANDOM()%y_spaces_available
                           +RANDOM()%y_spaces_available)/3;
            xlocations[i] = ((i > 0) ? xlocations[i-1] : 0)+xpitch;
            if (i < layers) {
                ylocations[i] = ((i > 0) ? ylocations[i-1] : 0)+ypitch;
            } else {
                ylocations[i] = ysize-1;
            }
            x_spaces_available -= xpitch-2;
            y_spaces_available -= ypitch-2;
        }
    }

    if (!(option&OPT_IRR_SPACE)) {
        /* evenly spaced */
        float xpitch, ypitch;  /* pitch of the onion layers */

        xpitch = (xsize-2.0)/(2.0*layers+1);
        ypitch = (ysize-2.0)/(layers+1);
        xlocations[0] = xpitch;
        ylocations[0] = ypitch;
        for (i = 1; i < 2*layers; i++) {
            xlocations[i] = xlocations[i-1]+xpitch;
            if (i < layers) {
                ylocations[i] = ylocations[i-1]+ypitch;
            } else {
                ylocations[i] = ysize-1;
            }
        }
    }

    /* draw all the onion boxes.  */
    draw_onion(maze, xlocations, ylocations, layers);
    make_doors(maze, xlocations, ylocations, layers, option);
}

/**
 * Draws the lines in the maze defining the onion layers.
 * @param maze
 * map to draw to.
 * @param xlocations
 * @param ylocations
 * array of locations.
 * @param layers
 * number of layers.
 * @todo
 * explain what locations arrays should be, and the meaning of layers.
 */
void draw_onion(char **maze, float *xlocations, float *ylocations, int layers)
{
    int i, j, l;

    for (l = 0; l < layers; l++) {
        int x1, x2, y1, y2;

        /* horizontal segments */
        y1 = (int)ylocations[l];
        y2 = (int)ylocations[2*layers-l-1];
        for (i = (int)xlocations[l]; i <= (int)xlocations[2*layers-l-1]; i++) {
            maze[i][y1] = '#';
            maze[i][y2] = '#';
        }

        /* vertical segments */
        x1 = (int)xlocations[l];
        x2 = (int)xlocations[2*layers-l-1];
        for (j = (int)ylocations[l]; j <= (int)ylocations[2*layers-l-1]; j++) {
            maze[x1][j] = '#';
            maze[x2][j] = '#';
        }
    }
}

/**
 * Add doors to the layout.
 * @param maze
 * map to draw to.
 * @param xlocations
 * @param ylocations
 * array of locations. Will be free()d.
 * @param layers
 * number of layers.
 * @param options
 * combination of @ref OPT_xxx "OPT_xxx" values.
 * @todo
 * explain what locations arrays should be, and the meaning of layers.
 */
void make_doors(char **maze, float *xlocations, float *ylocations, int layers, int options)
{
    int freedoms;  /* number of different walls on which we could place a door */
    int which_wall; /* left, 1, top, 2, right, 3, bottom 4 */
    int l, x1 = 0, x2, y1 = 0, y2;

    freedoms = 4;  /* centered */
    if (options&OPT_BOTTOM_C) {
        freedoms = 3;
    }
    if (options&OPT_BOTTOM_R) {
        freedoms = 2;
    }
    if (layers <= 0) {
        return;
    }

    /* pick which wall will have a door. */
    which_wall = RANDOM()%freedoms+1;
    for (l = 0; l < layers; l++) {
        if (options&OPT_LINEAR) { /* linear door placement. */
            switch (which_wall) {
            case 1: { /* left hand wall */
                x1 = (int)xlocations[l];
                y1 = (int)((ylocations[l]+ylocations[2*layers-l-1])/2);
                break;
            }

            case 2: { /* top wall placement */
                x1 = (int)((xlocations[l]+xlocations[2*layers-l-1])/2);
                y1 = (int)ylocations[l];
                break;
            }

            case 3: { /* right wall placement */
                x1 = (int)xlocations[2*layers-l-1];
                y1 = (int)((ylocations[l]+ylocations[2*layers-l-1])/2);
                break;
            }

            case 4: { /* bottom wall placement */
                x1 = (int)((xlocations[l]+xlocations[2*layers-l-1])/2);
                y1 = (int)ylocations[2*layers-l-1];
                break;
            }
            }
        } else { /* random door placement. */
            which_wall = RANDOM()%freedoms+1;
            switch (which_wall) {
            case 1: {  /* left hand wall */
                x1 = (int)xlocations[l];
                y2 = ylocations[2*layers-l-1]-ylocations[l]-1;
                if (y2 > 0) {
                    y1 = ylocations[l]+RANDOM()%y2+1;
                } else {
                    y1 = ylocations[l]+1;
                }
                break;
            }

            case 2: { /* top wall placement */
                x2 = (int)((-xlocations[l]+xlocations[2*layers-l-1]))-1;
                if (x2 > 0) {
                    x1 = xlocations[l]+RANDOM()%x2+1;
                } else {
                    x1 = xlocations[l]+1;
                }
                y1 = (int)ylocations[l];
                break;
            }

            case 3: { /* right wall placement */
                x1 = (int)xlocations[2*layers-l-1];
                y2 = (int)((-ylocations[l]+ylocations[2*layers-l-1]))-1;
                if (y2 > 0) {
                    y1 = ylocations[l]+RANDOM()%y2+1;
                } else {
                    y1 = ylocations[l]+1;
                }
                break;
            }

            case 4: { /* bottom wall placement */
                x2 = (int)((-xlocations[l]+xlocations[2*layers-l-1]))-1;
                if (x2 > 0) {
                    x1 = xlocations[l]+RANDOM()%x2+1;
                } else {
                    x1 = xlocations[l]+1;
                }
                y1 = (int)ylocations[2*layers-l-1];
                break;
            }
            }
        }

        if (options&OPT_NO_DOORS) {
            maze[x1][y1] = '#';    /* no door. */
        } else {
            maze[x1][y1] = 'D';    /* write the door */
        }
    }

    /* mark the center of the maze with a C */
    l = layers-1;
    x1 = (int)(xlocations[l]+xlocations[2*layers-l-1])/2;
    y1 = (int)(ylocations[l]+ylocations[2*layers-l-1])/2;
    maze[x1][y1] = 'C';

    /* not needed anymore */
    free(xlocations);
    free(ylocations);
}

/**
 * Create a bottom-right-centered layout.
 * @param maze
 * layout.
 * @param xsize
 * @param ysize
 * layout size.
 * @param option
 * combination of @ref OPT_xxx "OPT_xxx" values.
 * @param layers
 * number of layers to create.
 */
void bottom_right_centered_onion(char **maze, int xsize, int ysize, int option, int layers)
{
    int i, maxlayers;
    float *xlocations;
    float *ylocations;

    maxlayers = (MIN(xsize, ysize)-2)/5;
    if (!maxlayers) {
        return;    /* map too small to onionize */
    }
    if (layers > maxlayers) {
        layers = maxlayers;
    }
    if (layers == 0) {
        layers = (RANDOM()%maxlayers)+1;
    }
    xlocations = (float *)calloc(sizeof(float), 2*layers);
    ylocations = (float *)calloc(sizeof(float), 2*layers);

    /* place all the walls */
    if (option&OPT_IRR_SPACE) { /* randomly spaced */
        int x_spaces_available, y_spaces_available;
        /* the "extra" spaces available for spacing between layers */
        x_spaces_available = (xsize-2)-3*layers+1;
        y_spaces_available = (ysize-2)-3*layers+1;

        /* pick an initial random pitch */
        for (i = 0; i < 2*layers; i++) {
            float xpitch = 2, ypitch = 2;

            if (x_spaces_available > 0)
                xpitch = 2
                         +(RANDOM()%x_spaces_available
                           +RANDOM()%x_spaces_available
                           +RANDOM()%x_spaces_available)/3;

            if (y_spaces_available > 0)
                ypitch = 2
                         +(RANDOM()%y_spaces_available
                           +RANDOM()%y_spaces_available
                           +RANDOM()%y_spaces_available)/3;
            if (i < layers) {
                xlocations[i] = ((i > 0) ? xlocations[i-1] : 0)+xpitch;
            } else {
                xlocations[i] = xsize-1;
            }

            if (i < layers) {
                ylocations[i] = ((i > 0) ? ylocations[i-1] : 0)+ypitch;
            } else {
                ylocations[i] = ysize-1;
            }
            x_spaces_available -= xpitch-2;
            y_spaces_available -= ypitch-2;
        }
    }

    if (!(option&OPT_IRR_SPACE)) { /* evenly spaced */
        float xpitch, ypitch;  /* pitch of the onion layers */

        xpitch = (xsize-2.0)/(2.0*layers+1);
        ypitch = (ysize-2.0)/(layers+1);
        xlocations[0] = xpitch;
        ylocations[0] = ypitch;
        for (i = 1; i < 2*layers; i++) {
            if (i < layers) {
                xlocations[i] = xlocations[i-1]+xpitch;
            } else {
                xlocations[i] = xsize-1;
            }
            if (i < layers) {
                ylocations[i] = ylocations[i-1]+ypitch;
            } else {
                ylocations[i] = ysize-1;
            }
        }
    }

    /* draw all the onion boxes.  */
    draw_onion(maze, xlocations, ylocations, layers);
    make_doors(maze, xlocations, ylocations, layers, option);
}
