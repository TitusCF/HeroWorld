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
#include <rproto.h>

/**
 * Given a layout and a coordinate, tell me which squares up/down/right/left
 * are occupied.
 * @param layout
 * @param i
 * @param j
 * spot to look.
 * @param RP
 * map parameters.
 * @return
 * combination of:
 * - 1 = something on left.
 * - 2 = something on right.
 * - 4 = something on above.
 * - 8 = something on below.
 * @todo
 * merge with surround_flag2() and friends, check if such a function doesn't exist in other files.
 */
int surround_flag(char **layout, int i, int j, RMParms *RP)
{
    int surround_index = 0;

    if ((i > 0) && layout[i-1][j] != 0) {
        surround_index |= 1;
    }
    if ((i < RP->Xsize-1) && layout[i+1][j] != 0) {
        surround_index |= 2;
    }
    if ((j > 0) && layout[i][j-1] != 0) {
        surround_index |= 4;
    }
    if ((j < RP->Ysize-1) && layout[i][j+1] != 0) {
        surround_index |= 8;
    }
    return surround_index;
}

/**
 * Given a layout and a coordinate, tell me which squares up/down/right/left
 * are occupied by walls.
 * @param layout
 * @param i
 * @param j
 * spot to look.
 * @param RP
 * map parameters.
 * @return
 * combination of:
 * - 1 = wall on left.
 * - 2 = wall on right.
 * - 4 = wall on above.
 * - 8 = wall on below.
 * @todo
 * merge with surround_flag() and friends, check if such a function doesn't exist in other files.
 */
int surround_flag2(char **layout, int i, int j, RMParms *RP)
{
    int surround_index = 0;

    if ((i > 0) && layout[i-1][j] == '#') {
        surround_index |= 1;
    }
    if ((i < RP->Xsize-1) && layout[i+1][j] == '#') {
        surround_index |= 2;
    }
    if ((j > 0) && layout[i][j-1] == '#') {
        surround_index |= 4;
    }
    if ((j < RP->Ysize-1) && layout[i][j+1] == '#') {
        surround_index |= 8;
    }
    return surround_index;
}

/**
 * Check a map for blocked spots.
 * Since this is part of the random map code, presumption
 * is that this is not a tiled map.
 * What is considered blocking and not is somewhat hard coded.
 * @param map
 * @param i
 * @param j
 * spot to look.
 * @param RP
 * map parameters.
 * @return
 * combination of:
 * - 1 = blocked on left.
 * - 2 = blocked on right.
 * - 4 = blocked on above.
 * - 8 = blocked on below.
 */
int surround_flag3(mapstruct *map, int i, int j, RMParms *RP)
{
    int surround_index = 0;

    if ((i > 0) && (GET_MAP_MOVE_BLOCK(map, i-1, j)&~MOVE_BLOCK_DEFAULT)) {
        surround_index |= 1;
    }
    if ((i < RP->Xsize-1) && (GET_MAP_MOVE_BLOCK(map, i+1, j)&~MOVE_BLOCK_DEFAULT)) {
        surround_index |= 2;
    }
    if ((j > 0) && (GET_MAP_MOVE_BLOCK(map, i, j-1)&~MOVE_BLOCK_DEFAULT)) {
        surround_index |= 4;
    }
    if ((j < RP->Ysize-1) && (GET_MAP_MOVE_BLOCK(map, i, j+1)&~MOVE_BLOCK_DEFAULT)) {
        surround_index |= 8;
    }

    return surround_index;
}

/**
 * Check a map for spots with walls.
 * Since this is part of the random map code, presumption
 * is that this is not a tiled map.
 * What is considered blocking and not is somewhat hard coded.
 * @param map
 * @param i
 * @param j
 * spot to look.
 * @param RP
 * map parameters.
 * @return
 * combination of:
 * - 1 = blocked on left.
 * - 2 = blocked on right.
 * - 4 = blocked on above.
 * - 8 = blocked on below.
 */
int surround_flag4(mapstruct *map, int i, int j, RMParms *RP)
{
    int surround_index = 0;

    if ((i > 0) && wall_blocked(map, i-1, j)) {
        surround_index |= 1;
    }
    if ((i < RP->Xsize-1) && wall_blocked(map, i+1, j)) {
        surround_index |= 2;
    }
    if ((j > 0) && wall_blocked(map, i, j-1)) {
        surround_index |= 4;
    }
    if ((j < RP->Ysize-1) && wall_blocked(map, i, j+1)) {
        surround_index |= 8;
    }

    return surround_index;
}

/**
 * takes a map and a layout, and puts walls in the map (picked from
 * w_style) at '#' marks.
 * @param map
 * map where walls will be put.
 * @param layout
 * layout containing walls and such.
 * @param w_style
 * wall style. Must not be NULL, can be "none".
 * @param RP
 * map parameters.
 */
void make_map_walls(mapstruct *map, char **layout, char *w_style, RMParms *RP)
{
    char styledirname[256];
    mapstruct *style_map = NULL;
    object *the_wall;

    /* get the style map */
    if (!strcmp(w_style, "none")) {
        return;
    }
    snprintf(styledirname, sizeof(styledirname), "%s", "/styles/wallstyles");
    style_map = find_style(styledirname, w_style, -1);
    if (style_map == NULL) {
        return;
    }

    /* fill up the map with the given floor style */
    if ((the_wall = pick_random_object(style_map)) != NULL) {
        int i, j;
        char *cp;
        int joinedwalls = 0;
        object *thiswall;

        snprintf(RP->wall_name, sizeof(RP->wall_name), "%s", the_wall->arch->name);
        if ((cp = strchr(RP->wall_name, '_')) != NULL) {
            *cp = 0;
            joinedwalls = 1;
        }

        for (i = 0; i < RP->Xsize; i++)
            for (j = 0; j < RP->Ysize; j++) {
                if (layout[i][j] == '#') {
                    if (joinedwalls) {
                        thiswall = pick_joined_wall(the_wall, layout, i, j, RP);
                    } else {
                        thiswall = arch_to_object(the_wall->arch);
                    }
                    thiswall->move_block = MOVE_ALL;
                    thiswall->move_allow = 0;
                    object_insert_in_map_at(thiswall, map, thiswall, INS_NO_MERGE|INS_NO_WALK_ON, i, j);
                }
            }
    }
}

/**
 * Picks the right wall type for this square, to make it look nice,
 * and have everything nicely joined.  It uses the layout.
 * @param the_wall
 * wall we want to insert.
 * @param layout
 * @param i
 * @param j
 * where to insert.
 * @param RP
 * map parameters.
 * @return
 * correct wall archetype to fit on the square.
 * @todo
 * check if there isn't an equivalent function in the building code, merge?
 */
object *pick_joined_wall(object *the_wall, char **layout, int i, int j, RMParms *RP)
{
    /* 1 = wall to left,
       2 = wall to right,
       4 = wall above
       8 = wall below */
    int surround_index = 0;
    int l;
    char wall_name[64];
    archetype *wall_arch = NULL;

    strncpy(wall_name, the_wall->arch->name, sizeof(wall_name));

    /* conventionally, walls are named like this:
     wallname_wallcode, where wallcode indicates
     a joinedness, and wallname is the wall.
     this code depends on the convention for
     finding the right wall. */

    /* extract the wall name, which is the text up to the leading _ */
    for (l = 0; l < 64; l++) {
        if (wall_name[l] == '_') {
            wall_name[l] = 0;
            break;
        }
    }

    surround_index = surround_flag2(layout, i, j, RP);

    switch (surround_index) {
    case 0:
        strcat(wall_name, "_0");
        break;

    case 1:
        strcat(wall_name, "_1_3");
        break;

    case 2:
        strcat(wall_name, "_1_4");
        break;

    case 3:
        strcat(wall_name, "_2_1_2");
        break;

    case 4:
        strcat(wall_name, "_1_2");
        break;

    case 5:
        strcat(wall_name, "_2_2_4");
        break;

    case 6:
        strcat(wall_name, "_2_2_1");
        break;

    case 7:
        strcat(wall_name, "_3_1");
        break;

    case 8:
        strcat(wall_name, "_1_1");
        break;

    case 9:
        strcat(wall_name, "_2_2_3");
        break;

    case 10:
        strcat(wall_name, "_2_2_2");
        break;

    case 11:
        strcat(wall_name, "_3_3");
        break;

    case 12:
        strcat(wall_name, "_2_1_1");
        break;

    case 13:
        strcat(wall_name, "_3_4");
        break;

    case 14:
        strcat(wall_name, "_3_2");
        break;

    case 15:
        strcat(wall_name, "_4");
        break;
    }
    wall_arch = try_find_archetype(wall_name);
    if (wall_arch) {
        return arch_to_object(wall_arch);
    } else {
        return arch_to_object(the_wall->arch);
    }
}

/**
 * this takes a map, and changes an existing wall to match what's blocked
 * around it, counting only doors and walls as blocked.  If insert_flag is
 * 1, .  If not, it
 * will only return the wall which would belong there, and doesn't
 * remove anything.  It depends on the
 * global, previously-set variable, "wall_name"
 * @param the_map
 * @param i
 * @param j
 * where to look.
 * @param insert_flag
 * if 1, insert the correct wall into the map, else don't insert.
 * @param RP
 * map parameters.
 * @return
 * correct wall for spot.
 * @todo
 * merge with pick_joined_wall()?
 */
object *retrofit_joined_wall(mapstruct *the_map, int i, int j, int insert_flag, RMParms *RP)
{
    /* 1 = wall to left,
     * 2 = wall to right,
     * 4 = wall above
     * 8 = wall below
     */
    int surround_index = 0;
    int l;
    object *the_wall = NULL;
    object *new_wall = NULL;
    archetype *wall_arch = NULL;

    /* first find the wall */
    FOR_MAP_PREPARE(the_map, i, j, tmp)
    if ((tmp->move_type&MOVE_WALK) && tmp->type != EXIT && tmp->type != TELEPORTER) {
        the_wall = tmp;
        break;
    }
    FOR_MAP_FINISH();

    /* if what we found is a door, don't remove it, set the_wall to NULL to
     * signal that later.
     */
    if (the_wall && (the_wall->type == DOOR || the_wall->type == LOCKED_DOOR)) {
        the_wall = NULL;
        /* if we're not supposed to insert a new wall where there wasn't one,
         * we've gotta leave.
         */
        if (insert_flag == 0) {
            return NULL;
        }
    } else if (the_wall == NULL) {
        return NULL;
    }

    /* canonicalize the wall name */
    for (l = 0; l < 64; l++) {
        if (RP->wall_name[l] == '_') {
            RP->wall_name[l] = 0;
            break;
        }
    }

    surround_index = surround_flag4(the_map, i, j, RP);
    /* This would be a lot cleaner to just us a lookup table,
     * eg, wall_suffix[surround_index]
     */
    switch (surround_index) {
    case 0:
        strcat(RP->wall_name, "_0");
        break;

    case 1:
        strcat(RP->wall_name, "_1_3");
        break;

    case 2:
        strcat(RP->wall_name, "_1_4");
        break;

    case 3:
        strcat(RP->wall_name, "_2_1_2");
        break;

    case 4:
        strcat(RP->wall_name, "_1_2");
        break;

    case 5:
        strcat(RP->wall_name, "_2_2_4");
        break;

    case 6:
        strcat(RP->wall_name, "_2_2_1");
        break;

    case 7:
        strcat(RP->wall_name, "_3_1");
        break;

    case 8:
        strcat(RP->wall_name, "_1_1");
        break;

    case 9:
        strcat(RP->wall_name, "_2_2_3");
        break;

    case 10:
        strcat(RP->wall_name, "_2_2_2");
        break;

    case 11:
        strcat(RP->wall_name, "_3_3");
        break;

    case 12:
        strcat(RP->wall_name, "_2_1_1");
        break;

    case 13:
        strcat(RP->wall_name, "_3_4");
        break;

    case 14:
        strcat(RP->wall_name, "_3_2");
        break;

    case 15:
        strcat(RP->wall_name, "_4");
        break;
    }
    wall_arch = try_find_archetype(RP->wall_name);
    if (wall_arch != NULL) {
        new_wall = arch_to_object(wall_arch);
        if (the_wall && the_wall->map) {
            object_remove(the_wall);
            object_free_drop_inventory(the_wall);
        }
        the_wall->move_block = MOVE_ALL;
        object_insert_in_map_at(new_wall, the_map, new_wall, INS_NO_MERGE|INS_NO_WALK_ON, i, j);
    }
    return new_wall;
}
