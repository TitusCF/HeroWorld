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
 * @file random_maps/main.c
 * Command-line interface to the map generator used to test layouts.
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

#define LO_NEWFILE 2

static void generate_map(char *OutFileName) {
    RMParms rp;
    mapstruct *newMap;

    fprintf(stderr, "Reading parameters from stdin...\n");

    /* Initialize parameters and set initial settings. */
    memset(&rp, 0, sizeof(RMParms));
    rp.Xsize = -1;
    rp.Ysize = -1;

    /* Initialize Crossfire library. */
    init_globals();
    init_library();
    init_archetypes();
    init_artifacts();
    init_formulae();
    init_readable();
    init_gods();

    load_parameters(stdin, LO_NEWFILE, &rp);
    fclose(stdin);

    newMap = generate_random_map(OutFileName, &rp, NULL);
    save_map(newMap, SAVE_MODE_INPLACE);
}

/**
 * Print the human-readable layout of a map.
 */
static void print_map(char **layout, int width, int height) {
    int i, j;

    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            char display_char;
            display_char = layout[i][j];

            switch (display_char) {
                case 0:
                    display_char = '.';
                    break;
                case 'D':
                    display_char = '+';
                    break;
            }

            putchar(display_char);
        }

        putchar('\n');
    }
}

/**
 * Test the map layout produced by the specified generator.
 * TODO: Select generator based on command-line arguments.
 */
static void test_layout(int width, int height) {
    char **layout;
    SRANDOM(time(0));

    layout = roguelike_layout_gen(width, height, 0);
    /*layout = make_snake_layout(width, height, 0); */
    /*layout = make_square_spiral_layout(width, height, 0); */
    /*layout = gen_corridor_rooms(width, height, 1); */
    /*layout = maze_gen(width, height, 0); */
    /*layout = map_gen_onion(width, height, 0, 0);*/

    print_map(layout, width, height);
}

/** Print a message stating how to get help. */
static void print_quickhelp() {
    fprintf(stderr, "Type 'random_map -h' for usage instructions.\n");
}

/** Print out usage information. */
static void print_usage() {
    printf(
        "Usage: random_map [options]\n"
        "\n"
        "Options:\n"
        "  -h             display this help message\n"
        "  -g <file>      randomly generate the specified map file\n"
        "  -t             test map layout (overriden by -g)\n"
        "  -x <width>     specify map width\n"
        "  -y <height>    specify map height\n"
    );
}

int main(int argc, char *argv[]) {
    int flag, mode = 0, width = 80, height = 23;
    char *filename_out;

    /* Parse command-line arguments. */
    while ((flag = getopt(argc, argv, "g:htx:y:")) != -1) {
        switch (flag) {
            case 'g':
                mode = 2;
                filename_out = optarg;
                break;
            case 'h':
                print_usage();
                exit(EXIT_SUCCESS);
                break;
            case 't':
                mode = 1;
                break;
            case 'x':
                width = atoi(optarg);
                break;
            case 'y':
                height = atoi(optarg);
                break;
            case '?':
                print_quickhelp();
                exit(EXIT_FAILURE);
                break;
        }
    }

    if (mode == 1) {
        test_layout(width, height);
        exit(EXIT_SUCCESS);
    } else if (mode == 2) {
        generate_map(filename_out);
        exit(EXIT_SUCCESS);
    } else {
        print_quickhelp();
        exit(EXIT_FAILURE);
    }
}

/* some plagarized code from apply.c--I needed just these two functions
without all the rest of the junk, so.... */
int apply_auto(object *op)
{
    object *tmp = NULL;
    int i;

    switch (op->type) {
    case SHOP_FLOOR:
        if (!HAS_RANDOM_ITEMS(op)) {
            return 0;
        }
        do {
            i = 10; /* let's give it 10 tries */
            while ((tmp = generate_treasure(op->randomitems, op->stats.exp ? op->stats.exp : 5)) == NULL && --i)
                ;
            if (tmp == NULL) {
                return 0;
            }
            if (QUERY_FLAG(tmp, FLAG_CURSED) || QUERY_FLAG(tmp, FLAG_DAMNED)) {
                object_free_drop_inventory(tmp);
                tmp = NULL;
            }
        } while (!tmp);

        SET_FLAG(tmp, FLAG_UNPAID);
        object_insert_in_map_at(tmp, op->map, NULL, 0, op->x, op->y);
        CLEAR_FLAG(op, FLAG_AUTO_APPLY);
        tmp = identify(tmp);
        break;

    case TREASURE:
        if (HAS_RANDOM_ITEMS(op))
            while ((op->stats.hp--) > 0) {
                create_treasure(op->randomitems, op, GT_ENVIRONMENT, op->stats.exp ? op->stats.exp : op->map == NULL ? 14 : op->map->difficulty, 0);
            }
        object_remove(op);
        object_free_drop_inventory(op);
        break;
    }

    return tmp ? 1 : 0;
}

/* apply_auto_fix goes through the entire map (only the first time
 * when an original map is loaded) and performs special actions for
 * certain objects (most initialization of chests and creation of
 * treasures and stuff).  Calls apply_auto if appropriate.
 */
void apply_auto_fix(mapstruct *m)
{
    int x, y;

    for (x = 0; x < MAP_WIDTH(m); x++)
        for (y = 0; y < MAP_HEIGHT(m); y++)
            FOR_MAP_PREPARE(m, x, y, tmp) {
            if (QUERY_FLAG(tmp, FLAG_AUTO_APPLY)) {
                apply_auto(tmp);
            } else if (tmp->type == TREASURE) {
                if (HAS_RANDOM_ITEMS(tmp))
                    while ((tmp->stats.hp--) > 0) {
                        create_treasure(tmp->randomitems, tmp, 0, m->difficulty, 0);
                    }
            }
            if (tmp && tmp->arch
                    && tmp->type != PLAYER
                    && tmp->type != TREASURE
                    && tmp->randomitems) {
                if (tmp->type == CONTAINER) {
                    if (HAS_RANDOM_ITEMS(tmp))
                        while ((tmp->stats.hp--) > 0) {
                            create_treasure(tmp->randomitems, tmp, 0, m->difficulty, 0);
                        }
                } else if (HAS_RANDOM_ITEMS(tmp)) {
                    create_treasure(tmp->randomitems, tmp, 0, m->difficulty, 0);
                }
            }
        }
    FOR_MAP_FINISH();
    for (x = 0; x < MAP_WIDTH(m); x++)
        for (y = 0; y < MAP_HEIGHT(m); y++)
            FOR_MAP_PREPARE(m, x, y, tmp) {
            if (tmp->above
                    && (tmp->type == TRIGGER_BUTTON || tmp->type == TRIGGER_PEDESTAL)) {
                check_trigger(tmp, tmp->above);
            }
        }
    FOR_MAP_FINISH();
}

/*
 * The following dummy variables are only used to resolve symbols at compile
 * time. They don't actually do anything useful.
 */

void set_map_timeout(mapstruct *oldmap) {
}

void draw_ext_info(int flags, int pri, const object *pl, uint8 type,
        uint8 subtype, const char *message) {
    fprintf(logfile, "%s\n", message);
}

void draw_ext_info_format(int flags, int pri, const object *pl, uint8 type,
        uint8 subtype, const char *format, ...) {
    va_list ap;

    va_start(ap, format);
    vfprintf(logfile, format, ap);
    va_end(ap);
}


void ext_info_map(int color, const mapstruct *map, uint8 type, uint8 subtype,
        const char *str1) {
    fprintf(logfile, "ext_info_map: %s\n", str1);
}

void move_firewall(object *ob) {
}

void emergency_save(int x) {
}

void clean_tmp_files() {
}

void esrv_send_item(object *ob, object *obx) {
}

void esrv_update_item(int flags, object *pl, object *op) {
}

void dragon_ability_gain(object *ob, int x, int y) {
}

void set_darkness_map(mapstruct *m) {
}

object *find_skill_by_number(object *who, int skillno) {
    return NULL;
}

void esrv_del_item(player *pl, object *ob) {
}

void esrv_update_spells(player *pl) {
}

void rod_adjust(object *rod) {
}

int execute_event(object *op, int eventcode, object *activator, object *third,
        const char *message, int fix) {
    return 0;
}

int execute_global_event(int eventcode, ...) {
    return 0;
}
