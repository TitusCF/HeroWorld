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
 * Makes sure that inventory objects are freed correctly.
 */

#include <check.h>
#include <global.h>
#include <sproto.h>
#include <stdlib.h>
#include <string.h>

static object *find_arch_at(mapstruct *map, int x, int y, const char *arch_name) {
    FOR_MAP_PREPARE(map, x, y, op) {
	if (strcmp(op->arch->name, arch_name) == 0) {
	    return op;
	}
    } FOR_MAP_FINISH();
    return NULL;
}

static void setup(void) {
}

static void teardown(void) {
}

START_TEST(test_merge) {
    mapstruct *map1;
    archetype *scroll_arch;
    archetype *food_arch;
    object *scroll1;
    object *scroll2;
    object *food1;
    object *food2;

    map1 = ready_map_name("/world/world_103_128", 0);
    fail_unless(map1 != NULL, "cannot load map /world/world_103_128");

    scroll_arch = find_archetype("scroll_new");
    fail_unless(scroll_arch != NULL, "cannot find archetype scroll_new");

    food_arch = find_archetype("food");
    fail_unless(food_arch != NULL, "cannot find archetype food");

    scroll1 = object_create_arch(scroll_arch);
    fail_unless(scroll1 != NULL, "cannot create object scroll_new");

    scroll2 = object_create_arch(scroll_arch);
    fail_unless(scroll2 != NULL, "cannot create object scroll_new");

    food1 = object_create_arch(food_arch);
    fail_unless(food1 != NULL, "cannot create object food");

    food2 = object_create_arch(food_arch);
    fail_unless(food2 != NULL, "cannot create object food");

    food1 = object_insert_in_ob(food1, scroll1);

    food2 = object_insert_in_ob(food2, scroll2);

    fail_unless(find_arch_at(map1, 4, 3, "scroll_new") == NULL, "map initially contains a scroll");
    fail_unless(find_arch_at(map1, 4, 3, "food") == NULL, "map initially contains a food");

    scroll1 = object_insert_in_map_at(scroll1, map1, NULL, 0, 4, 3);
    fail_unless(scroll1 != NULL, "scroll could not be added to the map");

    fail_unless(find_arch_at(map1, 4, 3, "scroll_new") == scroll1, "scroll disappeared");
    fail_unless(find_arch_at(map1, 4, 3, "food") == NULL, "map contains a food");

    scroll2 = object_insert_in_map_at(scroll2, map1, NULL, 0, 4, 3);
    fail_unless(scroll2 != NULL, "scroll could not be added to the map");

    fail_unless(find_arch_at(map1, 4, 3, "scroll_new") == scroll2, "scroll disappeared");
    fail_unless(find_arch_at(map1, 4, 3, "food") == NULL, "map contains a food");

    fail_unless(scroll2->nrof == 2, "scrolls didn't merge");
    fail_unless(QUERY_FLAG(scroll1, FLAG_FREED), "scroll wasn't freed");
}
END_TEST

static Suite *bug_suite(void) {
    Suite *s = suite_create("bug");
    TCase *tc_core = tcase_create("Core");

    tcase_add_checked_fixture(tc_core, setup, teardown);

    suite_add_tcase(s, tc_core);
    tcase_add_test(tc_core, test_merge);
    tcase_set_timeout(tc_core, 0);

    return s;
}

int main(void) {
    int nf;
    Suite *s = bug_suite();
    SRunner *sr = srunner_create(s);

    srunner_set_fork_status(sr, CK_NOFORK);
    init(0, NULL);

    srunner_set_xml(sr, LOGDIR "/bugs/bugtrack/free_objects.xml");
    srunner_set_log(sr, LOGDIR "/bugs/bugtrack/free_objects.out");
    srunner_run_all(sr, CK_ENV); /*verbosity from env variable*/
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return nf == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
