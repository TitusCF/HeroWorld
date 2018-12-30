/*
 * static char *rcsid_check_alchemy_c =
 *   "$Id: check_alchemy.c 4640 2006-06-07 21:44:18Z tchize $";
 */

/*
 * CrossFire, A Multiplayer game for X-windows
 *
 * Copyright (C) 2007 Mark Wedel & Crossfire Development Team
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
 * This tests the comet spell.  My main motivation for writing this
 * was to have a consistent test I could use for performance testing.
 * But I also wanted to make sure that the results were close before and
 * after the performance changes - make the spell use less cpu time
 * but having drastically different results probably isn't a good thing
 * either.
 * To really be useful, everything needs to be compiled with profiling
 * (-pg).  This can be done like 'setenv CFLAGS -pg; ./configure;
 * make clean; make'.  The make clean is necessary because it won't
 * recompile the objects based just on changes the the CFLAGS.
 *
 * Note that this test, even after performance changes are done, still
 * isn't bad - it checks several things - map creation, spell casting,
 * etc.  It wouldn't be hard to use this as a template to test things
 * like resistance code, etc.
 */

#include <stdlib.h>
#include <check.h>

#include <global.h>
#include <sproto.h>

#define TEST_MAP_SIZE    40
#define NUM_TICKS_TO_RUN    500
#define NUM_COMETS_TO_CAST  30
#define STARTING_HP     25000

/* The percentage, either plus or minus, that the results can
 * vary from the baseline and still be considered OK.
 * Note a good sanity check to make sure you put in the correct
 * values is to set this to 0.0 - in that case, checks should
 * pass.
 */
#define HP_DELTA    10

/* The first time you set up a test, you want to dump the
 * initial values to put into the hp_row/hp_diag arrays.
 * If this is set, it prints those values instead of doing
 * a comparision.
 */
/*#define PRINT_DEBUG_HP */

mapstruct *test_map;
object *mon;

static void setup(void) {
    object *mon1;
    int x, i;

    test_map = get_empty_map(TEST_MAP_SIZE, TEST_MAP_SIZE);

    mon = create_archetype("orc");
    fail_unless(mon != NULL, "Unable to find orc object");

    /* We customize the monster a bit so it is consistent -
     * give it a bunch of HP so it can survive the attacks,
     * set it speed to 0 so it doesn't do anything, etc.
     */
    for (i = 0; i < NROFATTACKS; i++)
        mon->resist[i] = 95;
    mon->stats.hp = STARTING_HP;
    mon->stats.maxhp = STARTING_HP;
    mon->speed = 0.0;
    mon->speed_left = 0.0;
    SET_FLAG(mon, FLAG_STAND_STILL);
    object_update_speed(mon);

    /* We now make copies of this custom monster and put it into the
     * map.  We make a diagonal from one corner to another,
     * as well as a line across the middle of the map (\ overlayed with -)
     * We could fill most of the map with monsters, but I think the
     * diagonal + horizontal should give a pretty representative
     * value of creatures being hit.
     */
    for (x = 0; x < TEST_MAP_SIZE; x++) {
        mon1 = object_new();
        object_copy(mon, mon1);
        object_insert_in_map_at(mon1, test_map, NULL, 0, x, TEST_MAP_SIZE/2);

        if (x != TEST_MAP_SIZE/2) {
            mon1 = object_new();
            object_copy(mon, mon1);
            object_insert_in_map_at(mon1, test_map, NULL, 0, x, x);
        }
    }

}

static void teardown(void) {
    free_map(test_map);
}

static void check_hp(const char *test, int hp_row[TEST_MAP_SIZE], int hp_diag[TEST_MAP_SIZE]) {
    object *our_mon;
    int x, diff;

#ifdef PRINT_DEBUG_HP
    printf("\nDumping debug hp for test %s\n ", test);
#endif

    /* Dump the HP of the monsters now.  We do it in 2 passes,
     * as I think it is easier to read that way.
     */
    for (x = 0; x < TEST_MAP_SIZE; x++) {
        our_mon = GET_MAP_OB(test_map, x, TEST_MAP_SIZE/2);
        if (!our_mon) {
            fail("Monster destroyed at %d, %d\n", x, TEST_MAP_SIZE/2);
            continue;
        }
        fail_unless(mon->name == our_mon->name, "Could not find our monster on the space?");

#ifdef PRINT_DEBUG_HP
        printf("%d, ", our_mon->stats.hp);
#else

        if (our_mon->stats.hp == hp_row[x]) {
            diff = 0;
        } else  if (our_mon->stats.hp < hp_row[x]) {
            diff = 100-(STARTING_HP-hp_row[x])*100/((STARTING_HP-our_mon->stats.hp) ? (STARTING_HP-our_mon->stats.hp) : 1);
        } else {
            diff = -(100-(STARTING_HP-our_mon->stats.hp)*100/((STARTING_HP-hp_row[x]) ? (STARTING_HP-hp_row[x]) : 1));
        }

        if (FABS(diff) > HP_DELTA) {
            fail("Mon (%d, %d) has hp out of range (%d != %d +/- %d, diff %d)\n", our_mon->x, our_mon->y, our_mon->stats.hp, hp_row[x], HP_DELTA, diff);
        }
#endif
    }

#ifdef PRINT_DEBUG_HP
    printf("\n\n");
#endif

    for (x = 0; x < TEST_MAP_SIZE; x++) {
        our_mon = GET_MAP_OB(test_map, x, x);
        if (!our_mon) {
            fprintf(stderr, "Monster destroyed at %d, %d\n", x, x);
            continue;
        }

        fail_unless(mon->name == our_mon->name, "Could not find our monster on the space?");

#ifdef PRINT_DEBUG_HP
        printf("%d, ", our_mon->stats.hp);
#else
        if (our_mon->stats.hp == hp_diag[x]) {
            diff = 0;
        } else if (our_mon->stats.hp < hp_diag[x]) {
            diff = 100-(STARTING_HP-hp_diag[x])*100/((STARTING_HP-our_mon->stats.hp) ? (STARTING_HP-our_mon->stats.hp) : 1);
        } else {
            diff = -(100-(STARTING_HP-our_mon->stats.hp)*100/((STARTING_HP-hp_diag[x]) ? (STARTING_HP-hp_diag[x]) : 1));
        }

        if (FABS(diff) > HP_DELTA) {
            fail("Mon (%d, %d) has hp out of range (%d != %d +/- %d, diff %d)\n", our_mon->x, our_mon->y, our_mon->stats.hp, hp_diag[x], HP_DELTA, diff);
        }
#endif
    }
}

START_TEST(cast_one_comet) {
    int hp_row[TEST_MAP_SIZE] = {25000, 25000, 25000, 25000, 25000, 25000, 25000, 25000, 24924, 24920, 24916, 24912, 24908, 24904, 24900, 24896, 24892, 24888, 24884, 24880, 24869, 24880, 24884, 24888, 24892, 24896, 24900, 24904, 24908, 24912, 24916, 24920, 24924, 25000, 25000, 25000, 25000, 25000, 25000, 25000 },
        hp_diag[TEST_MAP_SIZE] = {25000, 25000, 25000, 25000, 25000, 25000, 25000, 25000, 24924, 24920, 24916, 24912, 24908, 24904, 24900, 24896, 24892, 24888, 24884, 24880, 24869, 24880, 24884, 24888, 24892, 24896, 24900, 24904, 24908, 24912, 24916, 24920, 24924, 25000, 25000, 25000, 25000, 25000, 25000, 25000 };
    object *comet, *rod;
    int tick;

    rod = create_archetype("rod_heavy");
    rod->level = 100;
    comet = create_archetype("spell_comet");
    object_insert_in_ob(comet, rod);

    object_insert_in_map_at(rod, test_map, NULL, 0, TEST_MAP_SIZE/2, TEST_MAP_SIZE-1);

    cast_spell(rod, rod, 1, rod->inv, NULL);
    for (tick = 0; tick < NUM_TICKS_TO_RUN; tick++) {
        process_events();
    }

    check_hp("cast_one_comet", hp_row, hp_diag);
}
END_TEST

START_TEST(cast_random_comet) {
    object *comet, *rod;
    int tick, num_cast = 0;
    int hp_row[TEST_MAP_SIZE] = {23756, 23617, 23516, 23428, 23397, 23291, 23203, 23097, 23014, 22875, 22801, 22782, 22706, 22707, 22620, 22645, 22646, 22595, 22705, 22773, 22809, 22835, 22975, 23098, 23239, 23346, 23462, 23597, 23627, 23675, 23786, 23888, 24001, 24119, 24206, 24306, 24336, 24455, 24565, 24649 },
        hp_diag[TEST_MAP_SIZE] = {25000, 25000, 25000, 25000, 25000, 25000, 25000, 25000, 23515, 23351, 23177, 23097, 22946, 22931, 22763, 22706, 22678, 22658, 22728, 22812, 22809, 22712, 22728, 22741, 22726, 22833, 22862, 22967, 23014, 23009, 23167, 23267, 23367, 23459, 23596, 23713, 23750, 23879, 24026, 24160 };

    rod = create_archetype("rod_heavy");
    rod->level = 100;
    comet = create_archetype("spell_comet");
    object_insert_in_ob(comet, rod);

    object_insert_in_map_at(rod, test_map, NULL, 0, TEST_MAP_SIZE/2, TEST_MAP_SIZE-1);

    for (tick = 0; tick < NUM_TICKS_TO_RUN; tick++) {
        if (num_cast < NUM_COMETS_TO_CAST && (tick%1) == 0) {
            object_remove(rod);

            /* The idea here on the x is to shuffle the spaces around
             * a little, as a more typical case is comets
             * blowing up on different spaces.
             */
            object_insert_in_map_at(rod, test_map, NULL, 0, (tick*59)%37, TEST_MAP_SIZE-1);

            cast_spell(rod, rod, 1, rod->inv, NULL);
            num_cast++;
        }
        process_events();
    }
    check_hp("cast_random_comet", hp_row, hp_diag);

}
END_TEST

START_TEST(cast_bunch_comet) {
    object *comet, *rod;
    int tick, num_cast = 0;
    int hp_row[TEST_MAP_SIZE] = {25000, 25000, 25000, 25000, 25000, 25000, 25000, 25000, 22355, 22262, 22115, 21966, 21837, 21684, 21554, 21424, 21268, 21137, 21006, 20875, 20534, 20875, 21006, 21137, 21268, 21424, 21554, 21684, 21837, 21966, 22115, 22262, 22355, 25000, 25000, 25000, 25000, 25000, 25000, 25000 },
        hp_diag[TEST_MAP_SIZE] = {25000, 25000, 25000, 25000, 25000, 25000, 25000, 25000, 22355, 22262, 22115, 21966, 21837, 21684, 21554, 21424, 21268, 21137, 21006, 20875, 20534, 20875, 21006, 21137, 21268, 21424, 21554, 21684, 21837, 21966, 22115, 22262, 22355, 25000, 25000, 25000, 25000, 25000, 25000, 25000 };

    rod = create_archetype("rod_heavy");
    rod->level = 100;
    comet = create_archetype("spell_comet");
    object_insert_in_ob(comet, rod);

    object_insert_in_map_at(rod, test_map, NULL, 0, TEST_MAP_SIZE/2, TEST_MAP_SIZE-1);

    for (tick = 0; tick < NUM_TICKS_TO_RUN; tick++) {
        if (num_cast < NUM_COMETS_TO_CAST && (tick%1) == 0) {
            cast_spell(rod, rod, 1, rod->inv, NULL);
            num_cast++;
        }
        process_events();
    }
    check_hp("cast_bunch_comet", hp_row, hp_diag);

}
END_TEST

static Suite *comet_suite(void) {
    Suite *s = suite_create("comet");
    TCase *tc_core = tcase_create("Core");

    /* check by defaults has a 2 second timeout - that isn't
     * fast enough on my system - a run of 30 comets takes about
     * 7 seconds.  Setting this to 20 is enough, but on a slower
     * system may not be.
     */
    tcase_set_timeout(tc_core, 20);

    /*setup and teardown will be called before each test in testcase 'tc_core' */
    tcase_add_checked_fixture(tc_core, setup, teardown);

    suite_add_tcase(s, tc_core);
    tcase_add_test(tc_core, cast_one_comet);
    tcase_add_test(tc_core, cast_bunch_comet);
    tcase_add_test(tc_core, cast_random_comet);

    return s;
}

int main(void) {
    int nf;

    Suite *s = comet_suite();
    SRunner *sr = srunner_create(s);

    /* Don't want to fork - if we do, we lose the profile (-pg)
     * compiled information, which is what I used to determine if
     * things are more efficient.
     */
    srunner_set_fork_status(sr, CK_NOFORK);

    /* Only want to run this once, so don't put it in setup() */
    init(0, NULL);

    srunner_set_xml(sr, LOGDIR "/unit/server/comet.xml");
    srunner_set_log(sr, LOGDIR "/unit/server/comet.out");
    srunner_run_all(sr, CK_ENV); /*verbosity from env variable*/
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    fprintf(stderr, "Got %"FMT64U" supressions, %"FMT64U" spell merges, %"FMT64U" full tables\n", statistics.spell_suppressions, statistics.spell_merges, statistics.spell_hash_full);
    return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
