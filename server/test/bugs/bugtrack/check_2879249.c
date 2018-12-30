/*
 * CrossFire, A Multiplayer game for X-windows
 *
 * Copyright (C) 2009 Crossfire Development Team
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

/**
 * @file
 * This is the unit tests file for the bug #2879249: SIGSEGV on transport
 * moving across some edges of tiled maps.
 */

#include <stdlib.h>
#include <check.h>
#include <global.h>
#include <sproto.h>

static void setup(void) {
}

static void teardown(void) {
}

START_TEST(test_insert) {
    mapstruct *map1;
    archetype *big_galleon_arch;
    object *big_galleon;
    object *ob;

    map1 = ready_map_name("/world/world_103_128", 0);
    fail_unless(map1 != NULL, "cannot load map /world/world_103_128");

    big_galleon_arch = find_archetype("big_galleon");
    fail_unless(big_galleon_arch != NULL, "cannot find archetype big_galleon");
    big_galleon = object_create_arch(big_galleon_arch);
    fail_unless(big_galleon != NULL, "cannot create object big_galleon");

    for (ob = big_galleon; ob != NULL; ob = ob->more)
        if (ob->arch->clone.x > 0)
            break;
    fail_unless(ob != NULL, "big_galleon's height must be at least two");
    object_insert_in_map_at(big_galleon, map1, NULL, 0, map1->width/2, map1->height-1);
    for (ob = big_galleon; ob != NULL; ob = ob->more)
        fail_unless(ob->map != NULL);
}
END_TEST

static Suite *bug_suite(void) {
    Suite *s = suite_create("bug");
    TCase *tc_core = tcase_create("Core");

    tcase_add_checked_fixture(tc_core, setup, teardown);

    suite_add_tcase(s, tc_core);
    tcase_add_test(tc_core, test_insert);
    tcase_set_timeout(tc_core, 0);

    return s;
}

int main(void) {
    int nf;
    Suite *s = bug_suite();
    SRunner *sr = srunner_create(s);

    srunner_set_fork_status(sr, CK_NOFORK);
    init(0, NULL);

    srunner_set_xml(sr, LOGDIR "/bugs/bugtrack/2879249.xml");
    srunner_set_log(sr, LOGDIR "/bugs/bugtrack/2879249.out");
    srunner_run_all(sr, CK_ENV); /*verbosity from env variable*/
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return nf == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
