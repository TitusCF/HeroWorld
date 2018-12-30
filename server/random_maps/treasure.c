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

/*  placing treasure in maps, where appropriate.  */

/**
 * @file
 * This deals with inserting treasures in random maps.
 */

#include <global.h>
#include <random_map.h>
#include <rproto.h>

/**
 * @defgroup TREASURE_OPTIONS Treasure options
 *
 * Some defines for various options which can be set for random map treasures.
 */
/*@{*/
#define CONCENTRATED 1 /* all the treasure is at the C's for onions. */
#define HIDDEN 2   /* doors to treasure are hidden. */
#define KEYREQUIRED 4 /* chest has a key, which is placed randomly in the map. */
#define DOORED 8   /* treasure has doors around it. */
#define TRAPPED 16 /* trap dropped in same location as chest. */
#define SPARSE 32  /* 1/2 as much treasure as default */
#define RICH 64   /* 2x as much treasure as default */
#define FILLED 128  /* Fill/tile the entire map with treasure */
#define LAST_OPTION 64  /* set this to the last real option, for random */
/*@}*/

#define NO_PASS_DOORS 0
#define PASS_DOORS 1

static object **surround_by_doors(mapstruct *map, char **layout, int x, int y, int opts);

/**
 * Returns true if square x,y has P_NO_PASS set, which is true for walls
 * and doors but not monsters.
 * This function is not map tile aware.
 * @param m
 * @param x
 * @param y
 * map and coordinates to check for.
 * @return
 * non zero if blocked, 0 else.
 */
int wall_blocked(mapstruct *m, int x, int y)
{
    int r;

    if (OUT_OF_REAL_MAP(m, x, y)) {
        return 1;
    }
    r = GET_MAP_MOVE_BLOCK(m, x, y)&~MOVE_BLOCK_DEFAULT;
    return r;
}

/**
 * Place treasures in the map.
 * map,         (required)
 * layout,      (required)
 * treasure style    (may be empty or NULL, or "none" to cause no treasure.)
 * treasureoptions   (may be 0 for random choices or positive)
 * @param map
 * where to insert to.
 * @param layout
 * layout the map was generated from.
 * @param treasure_style
 * treasure style. May be empty or NULL for random style, or "none" for no treasures.
 * @param treasureoptions
 * treasure options.
 * @param RP
 * random map parameters.
 * @todo
 * flags for treasureoptions.
 */
void place_treasure(mapstruct *map, char **layout, char *treasure_style, int treasureoptions, RMParms *RP)
{
    char styledirname[256];
    char stylefilepath[256];
    mapstruct *style_map = NULL;
    int num_treasures;

    /* bail out if treasure isn't wanted. */
    if (treasure_style)
        if (!strcmp(treasure_style, "none")) {
            return;
        }
    if (treasureoptions <= 0) {
        treasureoptions = RANDOM()%(2*LAST_OPTION);
    }

    /* filter out the mutually exclusive options */
    if ((treasureoptions&RICH) && (treasureoptions&SPARSE)) {
        if (RANDOM()%2) {
            treasureoptions -= 1;
        } else {
            treasureoptions -= 2;
        }
    }

    /* pick the number of treasures */
    if (treasureoptions&SPARSE) {
        num_treasures = BC_RANDOM(RP->total_map_hp/600+RP->difficulty/2+1);
    } else if (treasureoptions&RICH) {
        num_treasures = BC_RANDOM(RP->total_map_hp/150+2*RP->difficulty+1);
    } else {
        num_treasures = BC_RANDOM(RP->total_map_hp/300+RP->difficulty+1);
    }

    if (num_treasures <= 0) {
        return;
    }

    /* get the style map */
    snprintf(styledirname, sizeof(styledirname), "%s", "/styles/treasurestyles");
    snprintf(stylefilepath, sizeof(stylefilepath), "%s/%s", styledirname, treasure_style);
    style_map = find_style(styledirname, treasure_style, -1);

    /* all the treasure at one spot in the map. */
    if (treasureoptions&CONCENTRATED) {
        /* map_layout_style global, and is previously set */
        switch (RP->map_layout_style) {
        case ONION_LAYOUT:
        case SPIRAL_LAYOUT:
        case SQUARE_SPIRAL_LAYOUT: {
            int i, j;

            /* search the onion for C's or '>', and put treasure there. */
            for (i = 0; i < RP->Xsize; i++) {
                for (j = 0; j < RP->Ysize; j++) {
                    if (layout[i][j] == 'C' || layout[i][j] == '>') {
                        int tdiv = RP->symmetry_used;
                        object **doorlist;
                        object *chest;

                        if (tdiv == 3) {
                            tdiv = 2;    /* this symmetry uses a divisor of 2*/
                        }
                        /* don't put a chest on an exit. */
                        chest = place_chest(treasureoptions, i, j, map, style_map, num_treasures/tdiv, RP);
                        if (!chest) {
                            continue;    /* if no chest was placed NEXT */
                        }
                        if (treasureoptions&(DOORED|HIDDEN)) {
                            doorlist = find_doors_in_room(map, i, j, RP);
                            lock_and_hide_doors(doorlist, map, treasureoptions, RP);
                            free(doorlist);
                        }
                    }
                }
            }
            break;
        }
        default: {
            int i, j, tries;
            object *chest;
            object **doorlist;

            i = j = -1;
            tries = 0;
            while (i == -1 && tries < 100) {
                i = RANDOM()%(RP->Xsize-2)+1;
                j = RANDOM()%(RP->Ysize-2)+1;
                find_enclosed_spot(map, &i, &j, RP);
                if (wall_blocked(map, i, j)) {
                    i = -1;
                }
                tries++;
            }
            chest = place_chest(treasureoptions, i, j, map, style_map, num_treasures, RP);
            if (!chest) {
                return;
            }
            i = chest->x;
            j = chest->y;
            if (treasureoptions&(DOORED|HIDDEN)) {
                doorlist = surround_by_doors(map, layout, i, j, treasureoptions);
                lock_and_hide_doors(doorlist, map, treasureoptions, RP);
                free(doorlist);
            }
        }
        }
    } else { /* DIFFUSE treasure layout */
        int ti, i, j;

        for (ti = 0; ti < num_treasures; ti++) {
            i = RANDOM()%(RP->Xsize-2)+1;
            j = RANDOM()%(RP->Ysize-2)+1;
            place_chest(treasureoptions, i, j, map, style_map, 1, RP);
        }
    }
}

/**
 * Put a chest into the map.
 * near x and y, with the treasure style
 * determined (may be null, or may be a treasure list from lib/treasures,
 * if the global variable "treasurestyle" is set to that treasure list's name
 * @param treasureoptions
 * options.
 * @param x
 * @param y
 * around which spot to put treasure.
 * @param map
 * map to put on.
 * @param style_map
 * unused.
 * @param n_treasures
 * ?
 * @param RP
 * parameters the map was generated from.
 * @return
 * inserted chest, NULL for failure.
 * @todo
 * document treasureoptions. Clean parameters. Check meaning of chest hp's field.
 */
object *place_chest(int treasureoptions, int x, int y, mapstruct *map, mapstruct *style_map, int n_treasures, RMParms *RP)
{
    object *the_chest;
    int i, xl, yl;
    treasurelist *tlist;
    const char *chests[] = { "chest", "chest_green", "chest_red", "chest_yellow", "chest_blue", "chest_pink" };

    the_chest = create_archetype(chests[RANDOM() % (sizeof(chests)/sizeof(*chests))]);
    if (the_chest == NULL) {
        return NULL;
    }

    /* first, find a place to put the chest. */
    i = object_find_first_free_spot(the_chest, map, x, y);
    if (i == -1) {
        object_free_drop_inventory(the_chest);
        return NULL;
    }
    xl = x+freearr_x[i];
    yl = y+freearr_y[i];

    /* if the placement is blocked, return a fail. */
    if (wall_blocked(map, xl, yl)) {
        object_free_drop_inventory(the_chest);
        return NULL;
    }

    tlist = find_treasurelist("chest");
    the_chest->randomitems = tlist;
    the_chest->stats.hp = n_treasures;

    /* stick a trap in the chest if required  */
    if (treasureoptions&TRAPPED) {
        mapstruct *trap_map = find_style("/styles/trapstyles", "traps", -1);
        object *the_trap;

        if (trap_map) {
            the_trap = pick_random_object(trap_map);
            if (the_trap) {
                object *new_trap;

                new_trap = object_new();
                object_copy_with_inv(the_trap, new_trap);
                new_trap->stats.Cha = 10+RP->difficulty;
                new_trap->level = BC_RANDOM((3*RP->difficulty)/2);
                if (new_trap->level == 0) {
                    new_trap->level = 1;
                }
                new_trap->x = x;
                new_trap->y = y;
                object_insert_in_ob(new_trap, the_chest);
            }
        }
    }

    /* set the chest lock code, and call the keyplacer routine with
     the lockcode.  It's not worth bothering to lock the chest if
     there's only 1 treasure....*/
    if ((treasureoptions&KEYREQUIRED) && n_treasures > 1) {
        char keybuf[256];

        snprintf(keybuf, sizeof(keybuf), "%d", (int)RANDOM());
        if (keyplace(map, x, y, keybuf, PASS_DOORS, 1, RP)) {
            the_chest->slaying = add_string(keybuf);
        }
    }

    /* actually place the chest. */
    object_insert_in_map_at(the_chest, map, NULL, 0, xl, yl);
    return the_chest;
}

/**
 * finds the closest monster and returns him, regardless of doors
 * or walls
 * @param map
 * @param x
 * @param y
 * where to look from.
 * @param RP
 * parameters for random map.
 * @return
 * monster, or NULL if none found.
 * @todo
 * shouldn't it search further away?
 */
object *find_closest_monster(mapstruct *map, int x, int y, RMParms *RP)
{
    int i;

    for (i = 0; i < SIZEOFFREE; i++) {
        int lx, ly;

        lx = x+freearr_x[i];
        ly = y+freearr_y[i];
        /* boundscheck */
        if (lx >= 0 && ly >= 0 && lx < RP->Xsize && ly < RP->Ysize)
            /* don't bother searching this square unless the map says life exists.*/
            if (GET_MAP_FLAGS(map, lx, ly)&P_IS_ALIVE) {
                FOR_MAP_PREPARE(map, lx, ly, the_monster)
                if (QUERY_FLAG(the_monster, FLAG_MONSTER)) {
                    return the_monster;
                }
                FOR_MAP_FINISH();
            }
    }
    return NULL;
}

/**
 * Places keys in the map, preferably in something alive.
 *
 * The idea is that you call keyplace on x,y where a door is, and it'll make
 * sure a key is placed on both sides of the door.
 *
 * @param map
 * @param x
 * @param y
 * where to put a key.
 * @param keycode
 * keycode is the key's code.
 * @param door_flag
 * if NO_PASS_DOORS won't cross doors or walls to keyplace, PASS_DOORS will.
 * if PASS_DOORS is set, the x & y values that are passed in are basically
 * meaningless - IMO, it is a bit of misnomer, as when it is set, it just
 * randomly chooses spaces on the map, ideally finding a close monster, to put
 * the key in.  In fact, if PASS_DOORS is set, there is no guarantee that
 * the keys will be on both sides of the door - it may happen by randomness,
 * but the code doesn't work to make sure it happens.
 * @param n_keys
 * number of keys to place. If 1, it will place 1 key. Else, it will place 2-4 keys.
 * @param RP
 * random map parameters.
 * @return
 * 1 if key was successfully placed, 0 else.
 */
int keyplace(mapstruct *map, int x, int y, char *keycode, int door_flag, int n_keys, RMParms *RP)
{
    int i, j;
    int kx, ky;
    object *the_keymaster; /* the monster that gets the key. */
    object *the_key;
    char keybuf[256];
    const char *keys[] = { "key2", "blue_key", "brown_key", "darkgray_key", "darkgreen_key", "gray_key", "green_key", "magenta_key", "red_key", "white_key" };

    /* get a key and set its keycode */
    the_key = create_archetype(keys[RANDOM() % (sizeof(keys)/sizeof(*keys))]);
    the_key->slaying = add_string(keycode);
    free_string(the_key->name);
    snprintf(keybuf, 256, "key from level %d of %s", RP->dungeon_level-1, RP->dungeon_name[0] != '\0' ? RP->dungeon_name : "a random map");
    the_key->name = add_string(keybuf);

    if (door_flag == PASS_DOORS) {
        int tries = 0;

        the_keymaster = NULL;
        while (tries < 15 && the_keymaster == NULL) {
            i = (RANDOM()%(RP->Xsize-2))+1;
            j = (RANDOM()%(RP->Ysize-2))+1;
            tries++;
            the_keymaster = find_closest_monster(map, i, j, RP);
        }
        /* if we don't find a good keymaster, drop the key on the ground. */
        if (the_keymaster == NULL) {
            int freeindex;

            freeindex = -1;
            for (tries = 0; tries < 15 && freeindex == -1; tries++) {
                kx = (RANDOM()%(RP->Xsize-2))+1;
                ky = (RANDOM()%(RP->Ysize-2))+1;
                freeindex = object_find_first_free_spot(the_key, map, kx, ky);
            }
            if (freeindex != -1) {
                kx += freearr_x[freeindex];
                ky += freearr_y[freeindex];
            }
        }
    } else { /* NO_PASS_DOORS --we have to work harder.*/
        /* don't try to keyplace if we're sitting on a blocked square and
         * NO_PASS_DOORS is set.
        */
        if (n_keys == 1) {
            if (wall_blocked(map, x, y)) {
                return 0;
            }
            the_keymaster = find_monster_in_room(map, x, y, RP);
            if (the_keymaster == NULL) /* if fail, find a spot to drop the key. */
                if (!find_spot_in_room(map, x, y, &kx, &ky, RP)) {
                    return 0;
                }
        } else {
            /* It can happen that spots around that point are all blocked, so
            * try to look farther away if needed
            */
            int sum = 0; /* count how many keys we actually place */
            int distance = 1;

            while (distance < 5) {
                /* I'm lazy, so just try to place in all 4 directions. */
                sum += keyplace(map, x+distance, y, keycode, NO_PASS_DOORS, 1, RP);
                sum += keyplace(map, x, y+distance, keycode, NO_PASS_DOORS, 1, RP);
                sum += keyplace(map, x-distance, y, keycode, NO_PASS_DOORS, 1, RP);
                sum += keyplace(map, x, y-distance, keycode, NO_PASS_DOORS, 1, RP);
                if (sum < 2) {  /* we might have made a disconnected map-place more keys. */
                    /* diagonally this time. */
                    keyplace(map, x+distance, y+distance, keycode, NO_PASS_DOORS, 1, RP);
                    keyplace(map, x+distance, y-distance, keycode, NO_PASS_DOORS, 1, RP);
                    keyplace(map, x-distance, y+distance, keycode, NO_PASS_DOORS, 1, RP);
                    keyplace(map, x-distance, y-distance, keycode, NO_PASS_DOORS, 1, RP);
                }
                if (sum > 0) {
                    return 1;
                }
                distance++;
            }
            return 0;
        }
    }

    if (the_keymaster == NULL) {
        object_insert_in_map_at(the_key, map, NULL, 0, kx, ky);
        return 1;
    }

    object_insert_in_ob(the_key, the_keymaster);
    return 1;
}

/**
 * A recursive routine which will return a monster, eventually, if there is one.
 * One should really call find_monster_in_room().
 * @param layout
 * map layout.
 * @param map
 * generated map.
 * @param x
 * @param y
 * where to look from.
 * @param RP
 * random map parameters.
 * @return
 * monster, or NULL if none found.
 */
object *find_monster_in_room_recursive(char **layout, mapstruct *map, int x, int y, RMParms *RP)
{
    int i, j;

    /* bounds check x and y */
    if (!(x >= 0 && y >= 0 && x < RP->Xsize && y < RP->Ysize)) {
        return NULL;
    }

    /* if the square is blocked or searched already, leave */
    if (layout[x][y] != 0) {
        return NULL;
    }

    /* check the current square for a monster.  If there is one,
       set theMonsterToFind and return it. */
    layout[x][y] = 1;
    if (GET_MAP_FLAGS(map, x, y)&P_IS_ALIVE) {
        FOR_MAP_PREPARE(map, x, y, the_monster)
        if (QUERY_FLAG(the_monster, FLAG_ALIVE)) {
            return HEAD(the_monster);
        }
        FOR_MAP_FINISH();
    }

    /* now search all the 8 squares around recursively for a monster, in random order */
    for (i = RANDOM()%8, j = 0; j < 8; i++, j++) {
        object *the_monster;

        the_monster = find_monster_in_room_recursive(layout, map, x+freearr_x[i%8+1], y+freearr_y[i%8+1], RP);
        if (the_monster != NULL) {
            return the_monster;
        }
    }
    return NULL;
}

/**
 * Find a monster in a room. Real work is done by find_monster_in_room_recursive().
 * @param map
 * generated map.
 * @param x
 * @param y
 * where to look from.
 * @param RP
 * random map parameters.
 * @return
 * monster, or NULL if none found.
 * @todo
 * couldn't the layout be given instead of being calculated?
 */
object *find_monster_in_room(mapstruct *map, int x, int y, RMParms *RP)
{
    char **layout2;
    int i, j;
    object *theMonsterToFind;

    layout2 = (char **)calloc(sizeof(char *), RP->Xsize);
    /* allocate and copy the layout, converting C to 0. */
    for (i = 0; i < RP->Xsize; i++) {
        layout2[i] = (char *)calloc(sizeof(char), RP->Ysize);
        for (j = 0; j < RP->Ysize; j++) {
            if (wall_blocked(map, i, j)) {
                layout2[i][j] = '#';
            }
        }
    }
    theMonsterToFind = find_monster_in_room_recursive(layout2, map, x, y, RP);

    /* deallocate the temp. layout */
    for (i = 0; i < RP->Xsize; i++) {
        free(layout2[i]);
    }
    free(layout2);

    return theMonsterToFind;
}

/** Datastructure needed by find_spot_in_room() and find_spot_in_room_recursive() */
typedef struct free_spots_struct {
    int *room_free_spots_x;             /**< Positions. */
    int *room_free_spots_y;             /**< Positions. */
    int number_of_free_spots_in_room;   /**< Number of positions. */
} free_spots_struct;

/**
 * the workhorse routine, which finds the free spots in a room:
 * a datastructure of free points is set up, and a position chosen from
 * that datastructure.
 * @param layout
 * map layout.
 * @param x
 * @param y
 * where to look from.
 * @param RP
 * random map parameters.
 * @param spots
 * currently found free spots.
 */
static void find_spot_in_room_recursive(char **layout, int x, int y, RMParms *RP, free_spots_struct *spots)
{
    int i, j;

    /* bounds check x and y */
    if (!(x >= 0 && y >= 0 && x < RP->Xsize && y < RP->Ysize)) {
        return;
    }

    /* if the square is blocked or searched already, leave */
    if (layout[x][y] != 0) {
        return;
    }

    /* set the current square as checked, and add it to the list.
      check off this point */
    layout[x][y] = 1;
    spots->room_free_spots_x[spots->number_of_free_spots_in_room] = x;
    spots->room_free_spots_y[spots->number_of_free_spots_in_room] = y;
    spots->number_of_free_spots_in_room++;
    /* now search all the 8 squares around recursively for free spots, in random order */
    for (i = RANDOM()%8, j = 0; j < 8; i++, j++) {
        find_spot_in_room_recursive(layout, x+freearr_x[i%8+1], y+freearr_y[i%8+1], RP, spots);
    }
}

/**
 * Find a random non-blocked spot in this room to drop a key.
 * Returns 1 if success, 0 else.
 * @param map
 * map to look into.
 * @param x
 * @param y
 * where to look from.
 * @param[out] kx
 * @param[out] ky
 * found spot if 1 is returned.
 * @param RP
 * random map parameters.
 * @return
 * 1 if spot found, 0 else.
 * @todo
 * couldn't layout be given instead of being computed?
 */
int find_spot_in_room(mapstruct *map, int x, int y, int *kx, int *ky, RMParms *RP)
{
    char **layout2;
    int i, j;
    free_spots_struct spots;

    spots.number_of_free_spots_in_room = 0;
    spots.room_free_spots_x = (int *)calloc(sizeof(int), RP->Xsize*RP->Ysize);
    spots.room_free_spots_y = (int *)calloc(sizeof(int), RP->Xsize*RP->Ysize);

    layout2 = (char **)calloc(sizeof(char *), RP->Xsize);
    /* allocate and copy the layout, converting C to 0. */
    for (i = 0; i < RP->Xsize; i++) {
        layout2[i] = (char *)calloc(sizeof(char), RP->Ysize);
        for (j = 0; j < RP->Ysize; j++) {
            if (wall_blocked(map, i, j)) {
                layout2[i][j] = '#';
            }
        }
    }

    /* setup num_free_spots and room_free_spots */
    find_spot_in_room_recursive(layout2, x, y, RP, &spots);

    if (spots.number_of_free_spots_in_room > 0) {
        i = RANDOM()%spots.number_of_free_spots_in_room;
        *kx = spots.room_free_spots_x[i];
        *ky = spots.room_free_spots_y[i];
    }

    /* deallocate the temp. layout */
    for (i = 0; i < RP->Xsize; i++) {
        free(layout2[i]);
    }
    free(layout2);
    free(spots.room_free_spots_x);
    free(spots.room_free_spots_y);

    if (spots.number_of_free_spots_in_room > 0) {
        return 1;
    }
    return 0;
}

/**
 * Searches the map for a spot with walls around it.  The more
 * walls the better, but it'll settle for 1 wall, or even 0, but
 * it'll return 0 if no FREE spots are found.
 * @param map
 * where to look.
 * @param cx
 * @param cy
 * where to look, and coordinates of found spot. -1 if no spot found.
 * @param RP
 * parameters of the random map.
 */
void find_enclosed_spot(mapstruct *map, int *cx, int *cy, RMParms *RP)
{
    int x, y;
    int i;

    x = *cx;
    y = *cy;

    for (i = 0; i <= SIZEOFFREE1; i++) {
        int lx, ly, sindex;
        lx = x+freearr_x[i];
        ly = y+freearr_y[i];
        sindex = surround_flag3(map, lx, ly, RP);
        /* if it's blocked on 3 sides, it's enclosed */
        if (sindex == 7 || sindex == 11 || sindex == 13 || sindex == 14) {
            *cx = lx;
            *cy = ly;
            return;
        }
    }

    /* OK, if we got here, we're obviously someplace where there's no enclosed
       spots--try to find someplace which is 2x enclosed.  */
    for (i = 0; i <= SIZEOFFREE1; i++) {
        int lx, ly, sindex;

        lx = x+freearr_x[i];
        ly = y+freearr_y[i];
        sindex = surround_flag3(map, lx, ly, RP);
        /* if it's blocked on 3 sides, it's enclosed */
        if (sindex == 3 || sindex == 5 || sindex == 9 || sindex == 6 || sindex == 10 || sindex == 12) {
            *cx = lx;
            *cy = ly;
            return;
        }
    }

    /* settle for one surround point */
    for (i = 0; i <= SIZEOFFREE1; i++) {
        int lx, ly, sindex;

        lx = x+freearr_x[i];
        ly = y+freearr_y[i];
        sindex = surround_flag3(map, lx, ly, RP);
        /* if it's blocked on 3 sides, it's enclosed */
        if (sindex) {
            *cx = lx;
            *cy = ly;
            return;
        }
    }

    /* give up and return the closest free spot. */
    i = object_find_first_free_spot(&find_archetype("chest")->clone, map, x, y);
    if (i != -1 && i <= SIZEOFFREE1) {
        *cx = x+freearr_x[i];
        *cy = y+freearr_y[i];
        return;
    }
    /* indicate failure */
    *cx = *cy = -1;
}

/**
 * Remove living things on specified spot.
 * @param x
 * @param y
 * @param map
 * where to remove.
 */
void remove_monsters(int x, int y, mapstruct *map)
{
    FOR_MAP_PREPARE(map, x, y, tmp) {
        if (QUERY_FLAG(tmp, FLAG_ALIVE)) {
            tmp = HEAD(tmp);
            object_remove(tmp);
            object_free_drop_inventory(tmp);
            tmp = GET_MAP_OB(map, x, y);
            if (tmp == NULL) {
                break;
            }
        }
    }
    FOR_MAP_FINISH();
}

/**
 * Surrounds the point x,y by doors, so as to enclose something, like
 * a chest.  It only goes as far as the 8 squares surrounding, and
 * it'll remove any monsters it finds.
 * @param map
 * map to work on.
 * @param layout
 * map's layout.
 * @param x
 * @param y
 * point to surround.
 * @param opts
 * flags.
 * @return
 * array of generated doors, NULL-terminated. Should be freed by caller.
 * @todo
 * document opts.
 */
static object **surround_by_doors(mapstruct *map, char **layout, int x, int y, int opts)
{
    int i;
    const char *doors[2];
    object **doorlist;
    int ndoors_made = 0;
    doorlist = (object **)calloc(9, sizeof(object *)); /* 9 doors so we can hold termination null */

    /* this is a list we pick from, for horizontal and vertical doors */
    if (opts&DOORED) {
        doors[0] = "locked_door2";
        doors[1] = "locked_door1";
    } else {
        doors[0] = "door_1";
        doors[1] = "door_2";
    }

    /* place doors in all the 8 adjacent unblocked squares. */
    for (i = 1; i < 9; i++) {
        int x1 = x+freearr_x[i], y1 = y+freearr_y[i];

        if (!wall_blocked(map, x1, y1)
                || layout[x1][y1] == '>') {/* place a door */
            object *new_door = create_archetype((freearr_x[i] == 0) ? doors[1] : doors[0]);
            sint16 nx, ny;

            nx = x+freearr_x[i];
            ny = y+freearr_y[i];
            remove_monsters(nx, ny, map);
            object_insert_in_map_at(new_door, map, NULL, 0, nx, ny);
            doorlist[ndoors_made] = new_door;
            ndoors_made++;
        }
    }
    return doorlist;
}

/**
 * Returns the first door in this square, or NULL if there isn't a door.
 * @param map
 * @param x
 * @param y
 * where to look.
 * @return
 * door, or NULL if none found.
 * @todo
 * isn't there a function for that in map.c?
 */
static object *door_in_square(mapstruct *map, int x, int y)
{
    FOR_MAP_PREPARE(map, x, y, tmp)
    if (tmp->type == DOOR || tmp->type == LOCKED_DOOR) {
        return tmp;
    }
    FOR_MAP_FINISH();
    return NULL;
}

/**
 * The workhorse routine, which finds the doors in a room
 * @param layout
 * @param map
 * @param x
 * @param y
 * random map to look into.
 * @param doorlist
 * list of doors.
 * @param ndoors
 * number of found doors.
 * @param RP
 * map parameters.
 */
void find_doors_in_room_recursive(char **layout, mapstruct *map, int x, int y, object **doorlist, int *ndoors, RMParms *RP)
{
    int i, j;
    object *door;

    /* bounds check x and y */
    if (!(x >= 0 && y >= 0 && x < RP->Xsize && y < RP->Ysize)) {
        return;
    }

    /* if the square is blocked or searched already, leave */
    if (layout[x][y] == 1) {
        return;
    }

    /* check off this point */
    if (layout[x][y] == '#') { /* there could be a door here */
        layout[x][y] = 1;
        door = door_in_square(map, x, y);
        if (door != NULL) {
            doorlist[*ndoors] = door;
            if (*ndoors > 254) { /* eek!  out of memory */
                LOG(llevError, "find_doors_in_room_recursive:Too many doors for memory allocated!\n");
                return;
            }
            *ndoors = *ndoors+1;
        }
    } else {
        layout[x][y] = 1;
        /* now search all the 8 squares around recursively for free spots, in random order */
        for (i = RANDOM()%8, j = 0; j < 8; i++, j++) {
            find_doors_in_room_recursive(layout, map, x+freearr_x[i%8+1], y+freearr_y[i%8+1], doorlist, ndoors, RP);
        }
    }
}

/**
 * Gets all doors in a room.
 * @param map
 * map to look into.
 * @param x
 * @param y
 * point of a room to find door for.
 * @param RP
 * map parameters.
 * @return
 * door list. Should be free()d be caller. NULL-terminated.
 * @todo
 * couldn't layout be given instead of being computed?
 */
object **find_doors_in_room(mapstruct *map, int x, int y, RMParms *RP)
{
    char **layout2;
    object **doorlist;
    int i, j;
    int ndoors = 0;

    doorlist = (object **)calloc(sizeof(*doorlist), 256);


    layout2 = (char **)calloc(sizeof(char *), RP->Xsize);
    /* allocate and copy the layout, converting C to 0. */
    for (i = 0; i < RP->Xsize; i++) {
        layout2[i] = (char *)calloc(sizeof(char), RP->Ysize);
        for (j = 0; j < RP->Ysize; j++) {
            if (wall_blocked(map, i, j)) {
                layout2[i][j] = '#';
            }
        }
    }

    /* setup num_free_spots and room_free_spots */
    find_doors_in_room_recursive(layout2, map, x, y, doorlist, &ndoors, RP);

    /* deallocate the temp. layout */
    for (i = 0; i < RP->Xsize; i++) {
        free(layout2[i]);
    }
    free(layout2);
    return doorlist;
}

/**
 * This removes any 'normal' doors around the specified door.
 * This is used for lock_and_hide_doors() below - it doesn't make sense
 * to have a locked door right behind a normal door, so lets
 * remove the normal ones.  It also fixes key placement issues.
 *
 * @param door
 * door around which to remove unlocked doors.
 */
static void remove_adjacent_doors(object *door)
{
    mapstruct *m = door->map;
    int x = door->x;
    int y = door->y;
    int i, flags;

    for (i = 1; i <= 8; i++) {
        flags = get_map_flags(m, NULL, x+freearr_x[i], y+freearr_y[i], NULL, NULL);
        if (flags&P_OUT_OF_MAP) {
            continue;
        }

        /* Old style doors are living objects.  So if P_IS_ALIVE is not
         * set, can not be a door on this space.
         */
        if (flags&P_IS_ALIVE) {
            FOR_MAP_PREPARE(m, x+freearr_x[i], y+freearr_y[i], tmp) {
                if (tmp->type == DOOR) {
                    object_remove(tmp);
                    object_free_drop_inventory(tmp);
                    break;
                }
            }
            FOR_MAP_FINISH();
        }
    }
}

/**
 * Locks and/or hides all the doors in doorlist, or does nothing if
 * opts doesn't say to lock/hide doors.
 * Note that some doors can be not locked if no good spot to put a key was found.
 * @param doorlist
 * doors to list. NULL-terminated.
 * @param map
 * map we're working on.
 * @param opts
 * options.
 * @param RP
 * map parameters.
 * @todo
 * document opts. Isn't it part of RP?
 */
void lock_and_hide_doors(object **doorlist, mapstruct *map, int opts, RMParms *RP)
{
    object *door;
    int i;

    /* lock the doors and hide the keys. */
    if (opts&DOORED) {
        for (i = 0, door = doorlist[0]; doorlist[i] != NULL; i++) {
            object *new_door = create_archetype("locked_door1");
            char keybuf[256];

            door = doorlist[i];
            new_door->face = door->face;
            object_remove(door);
            object_free_drop_inventory(door);
            doorlist[i] = new_door;
            object_insert_in_map_at(new_door, map, NULL, 0, door->x, door->y);

            snprintf(keybuf, 256, "%d", (int)RANDOM());
            if (keyplace(map, new_door->x, new_door->y, keybuf, NO_PASS_DOORS, 2, RP)) {
                new_door->slaying = add_string(keybuf);
            }
        }
        for (i = 0; doorlist[i] != NULL; i++) {
            remove_adjacent_doors(doorlist[i]);
        }
    }

    /* change the faces of the doors and surrounding walls to hide them. */
    if (opts&HIDDEN) {
        for (i = 0, door = doorlist[0]; doorlist[i] != NULL; i++) {
            object *wallface;

            door = doorlist[i];
            wallface = retrofit_joined_wall(map, door->x, door->y, 1, RP);
            if (wallface != NULL) {
                retrofit_joined_wall(map, door->x-1, door->y, 0, RP);
                retrofit_joined_wall(map, door->x+1, door->y, 0, RP);
                retrofit_joined_wall(map, door->x, door->y-1, 0, RP);
                retrofit_joined_wall(map, door->x, door->y+1, 0, RP);
                door->face = wallface->face;
                if (!QUERY_FLAG(wallface, FLAG_REMOVED)) {
                    object_remove(wallface);
                }
                object_free_drop_inventory(wallface);
            }
        }
    }
}
