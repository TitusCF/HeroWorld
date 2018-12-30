/*
 * static char *rcsid_check_attack_c =
 *   "$Id: check_attack.c 15379 2011-11-01 19:40:45Z ryo_saeba $";
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
 * This is the unit tests file for server/attack.c
 */

#include <global.h>
#include <stdlib.h>
#include <check.h>
#include <loader.h>
#include <toolkit_common.h>
#include <sproto.h>

static void setup(void) {
}

static void teardown(void) {
    /* put any cleanup steps here, they will be run after each testcase */
}

START_TEST(test_hit_player) {
    object *victim = NULL;
    object *hitter = NULL;
    object *floor = NULL;
    mapstruct *map = NULL;
    archetype *deplete = NULL;
    int test;

    map = get_empty_map(5, 5);
    floor = create_archetype("battleground");
    fail_unless(floor != NULL, "can't find archetype battleground");
    object_insert_in_map_at(floor, map, NULL, 0, 0, 0);
    floor = create_archetype("battleground");
    fail_unless(floor != NULL, "can't find archetype battleground");
    object_insert_in_map_at(floor, map, NULL, 0, 1, 0);

    deplete = find_archetype(ARCH_DEPLETION);
    fail_unless(deplete != NULL, "can't find archetype %s", ARCH_DEPLETION);

    victim = create_archetype("kobold");
    fail_unless(victim != NULL, "couldn't create kobold");
    fail_unless(victim->inv == NULL, "kobold shouldn't have an inventory");
    victim->stats.hp = 5000;
    victim->stats.maxhp = 5000;
    victim->resist[ATNR_DEPLETE] = 100;
    victim->resist[ATNR_FIRE] = 100;
    object_insert_in_map_at(victim, map, NULL, 0, 0, 0);
    hitter = create_archetype("sword");
    fail_unless(hitter != NULL, "couldn't create sword");
    hitter->attacktype = AT_DEPLETE|AT_FIRE;
    hitter->stats.dam = 100;
    hitter->map = map;
    object_insert_in_map_at(hitter, map, NULL, 0, 1, 0);

    fail_unless(arch_present_in_ob(deplete, victim) == NULL, "victim shouldn't be depleted before being attacked");

    for (test = 0; test < 100; test++) {
        hit_player(victim, hitter->stats.dam, hitter, hitter->attacktype, 0);
        fail_unless(victim->stats.hp == victim->stats.maxhp, "victim should have %d hp and not %d.", victim->stats.maxhp, victim->stats.hp);
    }
    fail_unless(victim->inv == NULL, "kobold shouldn't have an inventory after attacked");
    fail_unless(arch_present_in_ob(deplete, victim) == NULL, "victim shouldn't be depleted when slaying not set");

    hitter->slaying = add_string(victim->race);
    victim->resist[ATNR_FIRE] = 95;
    for (test = 0; test < 100 && arch_present_in_ob(deplete, victim) == NULL; test++) {
        hit_player(victim, hitter->stats.dam, hitter, hitter->attacktype, 0);
    }
    fail_unless(arch_present_in_ob(deplete, victim) != NULL, "victim should be depleted when slaying is set");
    fail_unless(victim->stats.hp != victim->stats.maxhp, "victim shouldn't have %d hp", victim->stats.hp);
}
END_TEST

static Suite *attack_suite(void) {
    Suite *s = suite_create("attack");
    TCase *tc_core = tcase_create("Core");

    /*setup and teardown will be called before each test in testcase 'tc_core' */
    tcase_add_checked_fixture(tc_core, setup, teardown);

    suite_add_tcase(s, tc_core);
    tcase_add_test(tc_core, test_hit_player);

    return s;
}

int main(void) {
    int nf;
    Suite *s = attack_suite();
    SRunner *sr = srunner_create(s);

    /* If you wish to debug the program, uncomment this line. */
    /*srunner_set_fork_status (sr, CK_NOFORK); */

    settings.debug = 0;
    init(0, NULL);

    srunner_set_xml(sr, LOGDIR "/unit/server/attack.xml");
    srunner_set_log(sr, LOGDIR "/unit/server/attack.out");
    srunner_run_all(sr, CK_ENV); /*verbosity from env variable*/
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
