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
 * Those functions handle the decor in the random maps.
 */

#include <global.h>
#include <random_map.h>
#include <rproto.h>

/** Number of decor styles that can be chosen if none specified. */
#define NR_DECOR_OPTIONS 1

/**
 * Count objects at a spot.
 * @param map
 * map we want to check
 * @param x
 * @param y
 * coordinates
 * @return
 * count of objects in the map at x,y.
 */
int obj_count_in_map(mapstruct *map, int x, int y)
{
    int count = 0;

    FOR_MAP_PREPARE(map, x, y, tmp)
    count++;
    FOR_MAP_FINISH();
    return count;
}

/**
 * Put the decor into the map.  Right now, it's very primitive.
 * @param map
 * map to add decor to.
 * @param maze
 * layout of the map, as was generated.
 * @param decorstyle
 * style to use. Can be NULL.
 * @param decor_option
 * how to place decor:
 * - 0 means no decor.
 * - 1 means to place randomly decor.
 * - other means to fill the map with decor.
 * @param RP
 * parameters of the random map.
 */
void put_decor(mapstruct *map, char **maze, char *decorstyle, int decor_option, RMParms *RP)
{
    mapstruct *decor_map;
    char style_name[256];

    snprintf(style_name, sizeof(style_name), "/styles/decorstyles");

    decor_map = find_style(style_name, decorstyle, -1);
    if (decor_map == NULL) {
        return;
    }

    /* pick a random option, only 1 option right now. */
    if (decor_option == 0) {
        decor_option = RANDOM()%NR_DECOR_OPTIONS+1;
    }

    switch (decor_option) {
    case 0:
        break;

    case 1: { /* random placement of decor objects. */
        int number_to_place = RANDOM()%((RP->Xsize*RP->Ysize)/5);
        int failures = 0;
        object *new_decor_object;

        while (failures < 100 && number_to_place > 0) {
            int x, y;

            x = RANDOM()%(RP->Xsize-2)+1;
            y = RANDOM()%(RP->Ysize-2)+1;
            if (maze[x][y] == 0 && obj_count_in_map(map, x, y) < 2) { /* empty */
                object *this_object;

                new_decor_object = pick_random_object(decor_map);
                this_object = arch_to_object(new_decor_object->arch);
                object_copy(new_decor_object, this_object);
                /*
                 * Don't change move_block, this prevents item merging.
                 * Instead, fix the item on the style map if blocking
                 * is bad.
                 */
                /*this_object->move_block = MOVE_BLOCK_DEFAULT;*/
                object_insert_in_map_at(this_object, map, NULL, 0, x, y);
                number_to_place--;
            } else {
                failures++;
            }
        }
        break;
    }

    default: { /* place decor objects everywhere: tile the map. */
        int i, j;

        for (i = 1; i < RP->Xsize-1; i++)
            for (j = 1; j < RP->Ysize-1; j++) {
                if (maze[i][j] == 0) {
                    object *new_decor_object, *this_object;

                    new_decor_object = pick_random_object(decor_map);
                    this_object = arch_to_object(new_decor_object->arch);
                    object_copy(new_decor_object, this_object);
                    /*
                     * Don't change move_block, this prevents item merging.
                     * Instead, fix the item on the style map if blocking
                     * is bad.
                     */
                    /*this_object->move_block = MOVE_BLOCK_DEFAULT;*/
                    object_insert_in_map_at(this_object, map, NULL, 0, i, j);
                }
            }
        break;
    }
    }
}
