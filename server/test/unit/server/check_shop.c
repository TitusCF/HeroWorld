/*
 * static char *rcsid_check_shop_c =
 *   "$Id: check_shop.c 18523 2012-11-18 18:56:34Z ryo_saeba $";
 */

/*
 * CrossFire, A Multiplayer game for X-windows
 *
 * Copyright (C) 2002 Mark Wedel & Crossfire Development Team
 * Copyright (C) 1992 Frank Tore Johansen
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * The authors can be reached via e-mail at crossfire-devel@real-time.com
 */

/*
 * This is the unit tests file for server/shop.c
 */

#include <stdlib.h>
#include <check.h>
#include <global.h>
#include <sproto.h>

static void setup(void) {
    /* put any initialisation steps here, they will be run before each testcase */
}

static void teardown(void) {
    /* put any cleanup steps here, they will be run after each testcase */
}

START_TEST(test_query_cost) {
    object *tosell, *player;
    int map_reset_time, player_charisma, arch;
    mapstruct* map;
    uint64 cost;
    static const char *sell_archs[] = { "fl_corpse", "Pdragon_mail", NULL };

    player = arch_to_object(find_archetype("dwarf_player"));
    fail_unless(player != NULL, "can't find player?");
    fail_unless(player->type == PLAYER, "invalid type for player?");
    fail_unless(strcmp(player->name, "dwarf") == 0, "wrong name?");

    map = get_empty_map(5, 5);
    strncpy(map->path, "test", sizeof(map->path) - 1);
    player->map = map;

    for (arch = 0; sell_archs[arch] != NULL; arch++) {
        tosell = arch_to_object(find_archetype(sell_archs[arch]));
        fail_unless(tosell != NULL, "can't find %s", sell_archs[arch]);
        tosell->nrof = 6;
        CLEAR_FLAG(tosell, FLAG_IDENTIFIED);

        for (player_charisma = 1; player_charisma <= 30; player_charisma++) {
            player->stats.Cha = player_charisma;
            for (map_reset_time = 0; map_reset_time < 1000; map_reset_time++) {
                map->reset_time = map_reset_time;
                cost = query_cost(tosell, player, BS_SELL|BS_SHOP);
                fail_unless(cost < 18446744073710, "mega price %" FMT64U " for charisma %d reset_time %d!", cost, player_charisma, map_reset_time);
            }
        }
        object_free_drop_inventory(tosell);
    }

    object_free_drop_inventory(player);
}
END_TEST

static Suite *shop_suite(void) {
    Suite *s = suite_create("shop");
    TCase *tc_core = tcase_create("Core");

    /*setup and teardown will be called before each test in testcase 'tc_core' */
    tcase_add_checked_fixture(tc_core, setup, teardown);

    suite_add_tcase(s, tc_core);
    tcase_add_test(tc_core, test_query_cost);

    return s;
}

int main(void) {
    int nf;
    Suite *s = shop_suite();
    SRunner *sr = srunner_create(s);

    srunner_set_fork_status(sr, CK_NOFORK);

    /* Only want to run this once, so don't put it in setup() */
    init(0, NULL);

    srunner_set_xml(sr, LOGDIR "/unit/server/shop.xml");
    srunner_set_log(sr, LOGDIR "/unit/server/shop.out");
    srunner_run_all(sr, CK_ENV); /*verbosity from env variable*/
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
