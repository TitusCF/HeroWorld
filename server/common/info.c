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

#include <global.h>

/**
 * @file
 * The functions in this file are purely mean to generate information
 * in differently formatted output, mainly about monsters.
 */

/**
 * Writes num ones and zeros to the given string based on the
 * bits variable.
 *
 * @param bits
 * variable to convert to binary string
 * @param num
 * number of bits to dump. Values above 32 will be ignored.
 * @param str
 * string to write to. Must be long enough.
 *
 * @note
 * no check is done whether str has enough space to write or not.
 * Final \\0 is appended to str.
 */
static void bitstostring(long bits, int num, char *str) {
    int i, j = 0;

    if (num > 32)
        num = 32;

    for (i = 0; i < num; i++) {
        if (i && (i%3) == 0) {
            str[i+j] = ' ';
            j++;
        }
        if (bits&1)
            str[i+j] = '1';
        else
            str[i+j] = '0';
        bits >>= 1;
    }
    str[i+j] = '\0';
    return;
}

/**
 * Dump to standard out the abilities of all monsters.
 */
void dump_abilities(void) {
    archetype *at;
    char *name;

    for (at = first_archetype; at; at = at->next) {
        const char *gen_name = "";
        archetype *gen;

        if (!QUERY_FLAG(&at->clone, FLAG_MONSTER))
            continue;

        /* Get rid of e.g. multiple black puddings */
        if (QUERY_FLAG(&at->clone, FLAG_CHANGING))
            continue;

        for (gen = first_archetype; gen; gen = gen->next) {
            if (gen->clone.other_arch && gen->clone.other_arch == at) {
                gen_name = gen->name;
                break;
            }
        }

        name = stringbuffer_finish(describe_item(&at->clone, NULL, NULL));
        printf("%-16s|%6"FMT64"|%4d|%3d|%s|%s|%s\n", at->clone.name, at->clone.stats.exp,
               at->clone.stats.hp, at->clone.stats.ac, name, at->name, gen_name);
        free(name);
    }
}

/**
 * As dump_abilities(), but with an alternative way of output.
 */
void print_monsters(void) {
    archetype *at;
    object *op;
    char attbuf[34];
    int i;

    printf("               |     |   |    |    |      attack       |                        resistances                                                                       |\n");
    printf("monster        | hp  |dam| ac | wc |pmf ecw adw gpd ptf|phy mag fir ele cld cfs acd drn wmg ght poi slo par tud fer cnc dep dth chs csp gpw hwd bln int |  exp   | new exp |\n");
    printf("---------------------------------------------------------------------------------------------------------------------------------------------------\n");
    for (at = first_archetype; at != NULL; at = at->next) {
        op = arch_to_object(at);
        if (QUERY_FLAG(op, FLAG_MONSTER)) {
            bitstostring((long)op->attacktype, NROFATTACKS, attbuf);
            printf("%-15s|%5d|%3d|%4d|%4d|%s|",
                   op->arch->name, op->stats.maxhp, op->stats.dam, op->stats.ac,
                   op->stats.wc, attbuf);
            for (i = 0; i < NROFATTACKS; i++)
                printf("%4d", op->resist[i]);
            printf("|%8"FMT64"|%9"FMT64"|\n", op->stats.exp, new_exp(op));
        }
        object_free_drop_inventory(op);
    }
}
