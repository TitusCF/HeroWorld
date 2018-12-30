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
 * @file los.c
 * This handles the "line of sight" for players and monsters.
 */

/* Nov 95 - inserted USE_LIGHTING code stuff in here - b.t. */

#include <global.h>
#include <math.h>

/**
 * Distance must be less than this for the object to be blocked.
 * An object is 1.0 wide, so if set to 0.5, it means the object
 * that blocks half the view (0.0 is complete block) will
 * block view in our tables.
 * .4 or less lets you see through walls.  .5 is about right.
 */
#define SPACE_BLOCK 0.5

typedef struct blstr {
    int x[4], y[4];
    int index;
} blocks;

static blocks block[MAP_CLIENT_X][MAP_CLIENT_Y];

static void expand_lighted_sight(object *op);

/**
 * What this sets is that x,y blocks the view of bx,by
 * This then sets up a relation - for example, something
 * at 5,4 blocks view at 5,3 which blocks view at 5,2
 * etc.  So when we check 5,4 and find it blocks, we have
 * the data to know that 5,3 and 5,2 and 5,1 should also
 * be blocked.
 * Used to initialise the array used by the LOS routines.
 *
 * @param x
 * @param y
 * coordinates that block.
 * @param bx
 * @param by
 * coordinates that get blocked.
 * @todo
 * check index for overflow?
 */
static void set_block(int x, int y, int bx, int by) {
    int index = block[x][y].index, i;

    /* Due to flipping, we may get duplicates - better safe than sorry.
     */
    for (i = 0; i < index; i++) {
        if (block[x][y].x[i] == bx
        && block[x][y].y[i] == by)
            return;
    }

    block[x][y].x[index] = bx;
    block[x][y].y[index] = by;
    block[x][y].index++;
#ifdef LOS_DEBUG
    LOG(llevDebug, "setblock: added %d %d -> %d %d (%d)\n", x, y, bx, by, block[x][y].index);
#endif
}

/**
 * initialises the array used by the LOS routines.
 *
 * Since we are only doing the upper left quadrant, only
 * these spaces could possibly get blocked, since these
 * are the only ones further out that are still possibly in the
 * sightline.
 */
void init_block(void) {
    int x, y, dx, dy, i;
    static const int block_x[3] = {
        -1, -1, 0
    }, block_y[3] = {
        -1, 0, -1
    };

    for (x = 0; x < MAP_CLIENT_X; x++)
        for (y = 0; y < MAP_CLIENT_Y; y++) {
            block[x][y].index = 0;
        }

    /* The table should be symmetric, so only do the upper left
     * quadrant - makes the processing easier.
     */
    for (x = 1; x <= MAP_CLIENT_X/2; x++) {
        for (y = 1; y <= MAP_CLIENT_Y/2; y++) {
            for (i = 0; i < 3; i++) {
                dx = x+block_x[i];
                dy = y+block_y[i];

                /* center space never blocks */
                if (x == MAP_CLIENT_X/2 && y == MAP_CLIENT_Y/2)
                    continue;

                /* If its a straight line, its blocked */
                if ((dx == x && x == MAP_CLIENT_X/2)
                || (dy == y && y == MAP_CLIENT_Y/2)) {
                    /* For simplicity, we mirror the coordinates to block the other
                    * quadrants.
                    */
                    set_block(x, y, dx, dy);
                    if (x == MAP_CLIENT_X/2) {
                        set_block(x, MAP_CLIENT_Y-y-1, dx, MAP_CLIENT_Y-dy-1);
                    } else if (y == MAP_CLIENT_Y/2) {
                        set_block(MAP_CLIENT_X-x-1, y, MAP_CLIENT_X-dx-1, dy);
                    }
                } else {
                    float d1, s, l;

                    /* We use the algorihm that found out how close the point
                     * (x,y) is to the line from dx,dy to the center of the viewable
                     * area.  l is the distance from x,y to the line.
                     * r is more a curiosity - it lets us know what direction (left/right)
                     * the line is off
                     */

                    d1 = (float)(pow(MAP_CLIENT_X/2-dx, 2)+pow(MAP_CLIENT_Y/2-dy, 2));
                    s = (float)((dy-y)*(MAP_CLIENT_X/2-dx)-(dx-x)*(MAP_CLIENT_Y/2-dy))/d1;
                    l = FABS(sqrt(d1)*s);

                    if (l <= SPACE_BLOCK) {
                        /* For simplicity, we mirror the coordinates to block the other
                        * quadrants.
                        */
                        set_block(x, y, dx, dy);
                        set_block(MAP_CLIENT_X-x-1, y, MAP_CLIENT_X-dx-1, dy);
                        set_block(x, MAP_CLIENT_Y-y-1, dx, MAP_CLIENT_Y-dy-1);
                        set_block(MAP_CLIENT_X-x-1, MAP_CLIENT_Y-y-1, MAP_CLIENT_X-dx-1, MAP_CLIENT_Y-dy-1);
                    }
                }
            }
        }
    }
}

/**
 * This recursively sets the blocked line of sight view.
 * From the blocked[][] array, we know for example
 * that if some particular space is blocked, it blocks
 * the view of the spaces 'behind' it, and those blocked
 * spaces behind it may block other spaces, etc.
 * In this way, the chain of visibility is set.
 * Used to initialise the array used by the LOS routines.
 *
 * @param op
 * player for which we're computing.
 * @param x
 * @param y
 * indexes into the blocked[][] array.
 */
static void set_wall(object *op, int x, int y) {
    int i;

    for (i = 0; i < block[x][y].index; i++) {
        int dx = block[x][y].x[i], dy = block[x][y].y[i], ax, ay;

        /* ax, ay are the values as adjusted to be in the
        * socket look structure.
        */
        ax = dx-(MAP_CLIENT_X-op->contr->socket.mapx)/2;
        ay = dy-(MAP_CLIENT_Y-op->contr->socket.mapy)/2;

        if (ax < 0 || ax >= op->contr->socket.mapx
        || ay < 0 || ay >= op->contr->socket.mapy)
            continue;
        /* we need to adjust to the fact that the socket
         * code wants the los to start from the 0,0
         * and not be relative to middle of los array.
         */
        op->contr->blocked_los[ax][ay] = 100;
        set_wall(op, dx, dy);
    }
}

/**
 * Used to initialise the array used by the LOS routines.
 * @param op
 * player's object
 * @param x
 * @param y
 * Values based on MAP_CLIENT_X and Y that index the blocked[][] arrays.
 *
 * @todo
 * use player *instead of object *to show it must be a player?
 */
static void check_wall(object *op, int x, int y) {
    int ax, ay;

    if (!block[x][y].index)
        return;

    /* ax, ay are coordinates as indexed into the look window */
    ax = x-(MAP_CLIENT_X-op->contr->socket.mapx)/2;
    ay = y-(MAP_CLIENT_Y-op->contr->socket.mapy)/2;

    /* If the converted coordinates are outside the viewable
     * area for the client, return now.
     */
    if (ax < 0 || ay < 0 || ax >= op->contr->socket.mapx || ay >= op->contr->socket.mapy)
        return;

    /* If this space is already blocked, prune the processing - presumably
     * whatever has set this space to be blocked has done the work and already
     * done the dependency chain.
     */
    if (op->contr->blocked_los[ax][ay] == 100)
        return;


    if (get_map_flags(op->map, NULL, op->x+x-MAP_CLIENT_X/2, op->y+y-MAP_CLIENT_Y/2, NULL, NULL)&(P_BLOCKSVIEW|P_OUT_OF_MAP))
        set_wall(op, x, y);
}

/**
 * Clears/initialises the los-array associated to the player
 * controlling the object.
 *
 * @param op
 * player's object.
 *
 * @todo
 * use player *instead of object *to show it must be a player?
 */
void clear_los(object *op) {
    /* This is safer than using the socket->mapx, mapy because
     * we index the blocked_los as a 2 way array, so clearing
     * the first z spaces may not not cover the spaces we are
     * actually going to use
     */
    (void)memset((void *)op->contr->blocked_los, 0, MAP_CLIENT_X*MAP_CLIENT_Y);
}

/**
 * Goes through the array of what the given player is
 * able to see, and expands the visible area a bit, so the player will,
 * to a certain degree, be able to see into corners.
 * This is somewhat suboptimal, would be better to improve the formula.
 *
 * @param op
 * player's object to process.
 *
 * @todo
 * use player *instead of object *to show it must be a player? */
static void expand_sight(object *op) {
    int i, x, y, dx, dy;

    for (x = 1; x < op->contr->socket.mapx-1; x++) /* loop over inner squares */
        for (y = 1; y < op->contr->socket.mapy-1; y++) {
            if (!op->contr->blocked_los[x][y]
            && !(get_map_flags(op->map, NULL,
                    op->x-op->contr->socket.mapx/2+x,
                    op->y-op->contr->socket.mapy/2+y,
                    NULL, NULL)&(P_BLOCKSVIEW|P_OUT_OF_MAP))) {
                for (i = 1; i <= 8; i += 1) { /* mark all directions */
                    dx = x+freearr_x[i];
                    dy = y+freearr_y[i];
                    if (op->contr->blocked_los[dx][dy] > 0) /* for any square blocked */
                        op->contr->blocked_los[dx][dy] = -1;
                }
            }
        }

    if (MAP_DARKNESS(op->map) > 0) /* player is on a dark map */
        expand_lighted_sight(op);


    /* clear mark squares */
    for (x = 0; x < op->contr->socket.mapx; x++)
        for (y = 0; y < op->contr->socket.mapy; y++)
            if (op->contr->blocked_los[x][y] < 0)
                op->contr->blocked_los[x][y] = 0;
}

/**
 * Checks if op has a light source.
 *
 * @param op
 * object being checked.
 * @return
 * true if op carries one or more lights
 *
 * @note
 * This is a trivial function now days, but it used to
 * be a bit longer.  Probably better for callers to just
 * check the op->glow_radius instead of calling this.
 */
int has_carried_lights(const object *op) {
    /* op may glow! */
    if (op->glow_radius > 0)
        return 1;

    return 0;
}

/**
 * Propagate the light information.
 *
 * @param op
 * player's object for which to compute the light values.
 */
static void expand_lighted_sight(object *op) {
    int x, y, darklevel, ax, ay, basex, basey, mflags, light, x1, y1;
    mapstruct *m = op->map;
    sint16 nx, ny;

    darklevel = MAP_DARKNESS(m);

    /* If the player can see in the dark, lower the darklevel for him */
    if (QUERY_FLAG(op, FLAG_SEE_IN_DARK))
        darklevel -= 2;

    /* add light, by finding all (non-null) nearby light sources, then
     * mark those squares specially. If the darklevel<1, there is no
     * reason to do this, so we skip this function
     */
    if (darklevel < 1)
        return;

    /* Do a sanity check.  If not valid, some code below may do odd
     * things.
     */
    if (darklevel > MAX_DARKNESS) {
        LOG(llevError, "Map darkness for %s on %s is too high (%d)\n", op->name, op->map->path, darklevel);
        darklevel = MAX_DARKNESS;
    }

    /* First, limit player furthest (unlighted) vision */
    for (x = 0; x < op->contr->socket.mapx; x++)
        for (y = 0; y < op->contr->socket.mapy; y++)
            if (op->contr->blocked_los[x][y] != 100)
                op->contr->blocked_los[x][y] = MAX_LIGHT_RADII;

    /* the spaces[] darkness value contains the information we need.
     * Only process the area of interest.
     * the basex, basey values represent the position in the op->contr->blocked_los
     * array.  Its easier to just increment them here (and start with the right
     * value) than to recalculate them down below.
     */
    for (x = (op->x-op->contr->socket.mapx/2-MAX_LIGHT_RADII), basex = -MAX_LIGHT_RADII;
            x <= (op->x+op->contr->socket.mapx/2+MAX_LIGHT_RADII); x++, basex++) {
        for (y = (op->y-op->contr->socket.mapy/2-MAX_LIGHT_RADII), basey = -MAX_LIGHT_RADII;
                y <= (op->y+op->contr->socket.mapy/2+MAX_LIGHT_RADII); y++, basey++) {
            m = op->map;
            nx = x;
            ny = y;

            mflags = get_map_flags(m, &m, nx, ny, &nx, &ny);

            if (mflags&P_OUT_OF_MAP)
                continue;

            /* This space is providing light, so we need to brighten up the
            * spaces around here.
            */
            light = GET_MAP_LIGHT(m, nx, ny);
            if (light != 0) {
                for (ax = basex-light; ax <= basex+light; ax++) {
                    if (ax < 0 || ax >= op->contr->socket.mapx)
                        continue;
                    for (ay = basey-light; ay <= basey+light; ay++) {
                        if (ay < 0 || ay >= op->contr->socket.mapy)
                            continue;

                        /* If the space is fully blocked, do nothing.  Otherwise, we
                         * brighten the space.  The further the light is away from the
                         * source (basex-x), the less effect it has.  Though light used
                         * to dim in a square manner, it now dims in a circular manner
                         * using the the pythagorean theorem. glow_radius still
                         * represents the radius
                         */
                        if (op->contr->blocked_los[ax][ay] != 100) {
                            x1 = abs(basex-ax)*abs(basex-ax);
                            y1 = abs(basey-ay)*abs(basey-ay);
                            if (light > 0)
                                op->contr->blocked_los[ax][ay] -= MAX((light-isqrt(x1+y1)), 0);
                            if (light < 0)
                                op->contr->blocked_los[ax][ay] -= MIN((light+isqrt(x1+y1)), 0);
                        }
                    } /* for ay */
                } /* for ax */
            } /* if this space is providing light */
        } /* for y */
    } /* for x */

    /* Outdoor should never really be completely pitch black dark like
     * a dungeon, so let the player at least see a little around themselves
     */
    if (op->map->outdoor && darklevel > (MAX_DARKNESS-3)) {
        if (op->contr->blocked_los[op->contr->socket.mapx/2][op->contr->socket.mapy/2] > (MAX_DARKNESS-3))
            op->contr->blocked_los[op->contr->socket.mapx/2][op->contr->socket.mapy/2] = MAX_DARKNESS-3;

        for (x = -1; x <= 1; x++)
            for (y = -1; y <= 1; y++) {
                if (op->contr->blocked_los[x+op->contr->socket.mapx/2][y+op->contr->socket.mapy/2] > (MAX_DARKNESS-2))
                    op->contr->blocked_los[x+op->contr->socket.mapx/2][y+op->contr->socket.mapy/2] = MAX_DARKNESS-2;
            }
    }
    /*  grant some vision to the player, based on the darklevel */
    for (x = darklevel-MAX_DARKNESS; x < MAX_DARKNESS+1-darklevel; x++)
        for (y = darklevel-MAX_DARKNESS; y < MAX_DARKNESS+1-darklevel; y++)
            if (!(op->contr->blocked_los[x+op->contr->socket.mapx/2][y+op->contr->socket.mapy/2] == 100))
                op->contr->blocked_los[x+op->contr->socket.mapx/2][y+op->contr->socket.mapy/2] -= MAX(0, 6-darklevel-MAX(abs(x), abs(y)));
}

/**
 * Sets all veiwable squares to blocked except
 * for the one the central one that the player occupies.  A little
 * odd that you can see yourself (and what your standing on), but
 * really need for any reasonable game play.
 *
 * @param op
 * player's object for which to reset los. Must have a valid contr.
 */
static void blinded_sight(object *op) {
    int x, y;

    for (x = 0; x < op->contr->socket.mapx; x++)
        for (y = 0; y <  op->contr->socket.mapy; y++)
            op->contr->blocked_los[x][y] = 100;

    op->contr->blocked_los[op->contr->socket.mapx/2][op->contr->socket.mapy/2] = 0;
}

/**
 * Recalculates the array which specifies what is
 * visible for the given player-object.
 *
 * @param op
 * player's object for which to compute.
 */
void update_los(object *op) {
    int dx = op->contr->socket.mapx/2, dy = op->contr->socket.mapy/2, x, y;

    if (QUERY_FLAG(op, FLAG_REMOVED))
        return;

    clear_los(op);
    if (QUERY_FLAG(op, FLAG_WIZ) /* || XRAYS(op) */)
        return;

    /* For larger maps, this is more efficient than the old way which
     * used the chaining of the block array.  Since many space views could
     * be blocked by different spaces in front, this mean that a lot of spaces
     * could be examined multile times, as each path would be looked at.
     */
    for (x = (MAP_CLIENT_X-op->contr->socket.mapx)/2+1; x < (MAP_CLIENT_X+op->contr->socket.mapx)/2-1; x++)
        for (y = (MAP_CLIENT_Y-op->contr->socket.mapy)/2+1; y < (MAP_CLIENT_Y+op->contr->socket.mapy)/2-1; y++)
            check_wall(op, x, y);


    /* do the los of the player. 3 (potential) cases */
    if (QUERY_FLAG(op, FLAG_BLIND)) /* player is blind */
        blinded_sight(op);
    else
        expand_sight(op);

    if (QUERY_FLAG(op, FLAG_XRAYS)) {
        int x, y;
        for (x = -2; x <= 2; x++)
            for (y = -2; y <= 2; y++)
                op->contr->blocked_los[dx+x][dy+y] = 0;
    }
}

/**
 * update all_map_los is like update_all_los() below,
 * but updates everyone on the map, no matter where they
 * are.  This generally should not be used, as a per
 * specific map change doesn't make much sense when tiling
 * is considered (lowering darkness would certainly be a
 * strange effect if done on a tile map, as it makes
 * the distinction between maps much more obvious to the
 * players, which is should not be.
 * Currently, this function is called from the
 * change_map_light function
 *
 * @param map
 * map on which affected players are.
 */
void update_all_map_los(mapstruct *map) {
    player *pl;

    for (pl = first_player; pl != NULL; pl = pl->next) {
        if (pl->ob->map == map)
            pl->do_los = 1;
    }
}

/**
 * This function makes sure that update_los() will be called for all
 * players on the given map within the next frame.
 * It is triggered by removal or inserting of objects which blocks
 * the sight in the map.
 * Modified by MSW 2001-07-12 to take a coordinate of the changed
 * position, and to also take map tiling into account.  This change
 * means that just being on the same map is not sufficient - the
 * space that changes must be withing your viewable area.
 *
 * @param map
 * map that changed.
 * @param x
 * @param y
 * coordinates of the change.
 *
 * @todo
 * check if this couldn't be simplified, especially tiling (isn't there a function somewhere that could help?)
 */
void update_all_los(const mapstruct *map, int x, int y) {
    player *pl;

    for (pl = first_player; pl != NULL; pl = pl->next) {
        /* Player should not have a null map, but do this
         * check as a safety
         */
        if (!pl->ob->map)
            continue;

        /* Same map is simple case - see if pl is close enough.
         * Note in all cases, we did the check for same map first,
         * and then see if the player is close enough and update
         * los if that is the case.  If the player is on the
         * corresponding map, but not close enough, then the
         * player can't be on another map that may be closer,
         * so by setting it up this way, we trim processing
         * some.
         */
        if (pl->ob->map == map) {
            if ((abs(pl->ob->x-x) <= pl->socket.mapx/2)
            && (abs(pl->ob->y-y) <= pl->socket.mapy/2))
                pl->do_los = 1;
        }
        /* Now we check to see if player is on adjacent
         * maps to the one that changed and also within
         * view.  The tile_maps[] could be null, but in that
         * case it should never match the pl->ob->map, so
         * we want ever try to dereference any of the data in it.
         */

        /* The logic for 0 and 3 is to see how far the player is
         * from the edge of the map (height/width) - pl->ob->(x,y)
         * and to add current position on this map - that gives a
         * distance.
         * For 1 and 2, we check to see how far the given
         * coordinate (x,y) is from the corresponding edge,
         * and then add the players location, which gives
         * a distance.
         */
        else if (pl->ob->map == map->tile_map[0]) {
            if ((abs(pl->ob->x-x) <= pl->socket.mapx/2)
            && (abs(y+MAP_HEIGHT(map->tile_map[0])-pl->ob->y) <= pl->socket.mapy/2))
                pl->do_los = 1;
        } else if (pl->ob->map == map->tile_map[2]) {
            if ((abs(pl->ob->x-x) <= pl->socket.mapx/2)
            && (abs(pl->ob->y+MAP_HEIGHT(map)-y) <= pl->socket.mapy/2))
                pl->do_los = 1;
        } else if (pl->ob->map == map->tile_map[1]) {
            if ((abs(pl->ob->x+MAP_WIDTH(map)-x) <= pl->socket.mapx/2)
            && (abs(pl->ob->y-y) <= pl->socket.mapy/2))
                pl->do_los = 1;
        } else if (pl->ob->map == map->tile_map[3]) {
            if ((abs(x+MAP_WIDTH(map->tile_map[3])-pl->ob->x) <= pl->socket.mapx/2)
            && (abs(pl->ob->y-y) <= pl->socket.mapy/2))
                pl->do_los = 1;
        }
    }
}

/**
 * Debug-routine which dumps the array which specifies the visible
 * area of a player.  Triggered by the DM command printlos.
 *
 * @param op
 * DM asking for information.
 *
 * @todo
 * change the command to view another player's LOS?
 */
void print_los(object *op) {
    int x, y;
    char buf[MAP_CLIENT_X*2+20], buf2[10];

    snprintf(buf, sizeof(buf), "[fixed]   ");
    for (x = 0; x < op->contr->socket.mapx; x++) {
        snprintf(buf2, sizeof(buf2), "%2d", x);
        strncat(buf, buf2, sizeof(buf)-strlen(buf)-1);
    }
    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DEBUG, buf);
    for (y = 0; y < op->contr->socket.mapy; y++) {
        snprintf(buf, sizeof(buf), "[fixed]%2d:", y);
        for (x = 0; x < op->contr->socket.mapx; x++) {
            snprintf(buf2, sizeof(buf2), " %1d", op->contr->blocked_los[x][y] == 100 ? 1 : 0);
            strncat(buf, buf2, sizeof(buf)-strlen(buf)-1);
        }
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DEBUG, buf);
    }
}

/**
 * The object is supposed to be visible through walls, thus
 * check if any players are nearby, and edit their LOS array.
 *
 * @param op
 * object that should be visible.
 *
 * @todo
 * what about tiled maps?
 */
void make_sure_seen(const object *op) {
    player *pl;

    for (pl = first_player; pl; pl = pl->next)
        if (pl->ob->map == op->map
        && pl->ob->y-pl->socket.mapy/2 <= op->y
        && pl->ob->y+pl->socket.mapy/2 >= op->y
        && pl->ob->x-pl->socket.mapx/2 <= op->x
        && pl->ob->x+pl->socket.mapx/2 >= op->x)
            pl->blocked_los[pl->socket.mapx/2+op->x-pl->ob->x][pl->socket.mapy/2+op->y-pl->ob->y] = 0;
}

/**
 * The object which is supposed to be visible through
 * walls has just been removed from the map, so update the los of any
 * players within its range
 *
 * @param op
 * object that disappears.
 *
 * @todo
 * what about tiled maps?
 */
void make_sure_not_seen(const object *op) {
    player *pl;

    for (pl = first_player; pl; pl = pl->next)
        if (pl->ob->map == op->map
        && pl->ob->y-pl->socket.mapy/2 <= op->y
        && pl->ob->y+pl->socket.mapy/2 >= op->y
        && pl->ob->x-pl->socket.mapx/2 <= op->x
        && pl->ob->x+pl->socket.mapx/2 >= op->x)
            pl->do_los = 1;
}
