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
 * Makes sure that weight reduction in containers correctly updates 'carrying'.
 */

#include <check.h>
#include <global.h>
#include <sproto.h>
#include <stdlib.h>
#include <string.h>

static void setup(void) {
}

static void teardown(void) {
}

START_TEST(test_weight_reduction) {
    /* Weight reduction to apply, last value -1. */
    const int reduction[] = { 0, 30, 50, 70, -1 };
    /* Container archetypes, must be CONTAINER, last value NULL. */
    const char *archs[] = { "sack", "luggage", "luggage", "luggage", NULL };
    /* What to put into the containers, last value NULL. Must be mergeable items. */
    const char *items[] = { "scroll", "crown", "gem", "tooth", NULL };
    /* How many to put or remove from container. Sum should be 0, last value 0. */
    const int add[] = { 2, 5, 7, 3, 3, 9, -5, -7, 11, -8, -9, -11, 0 };
    int red, a, i, nrof;
    uint32 sum;
    signed long carrying;
    object *container, *item;

    for (red = 0; reduction[red] != -1; red++) {
        for (a = 0; archs[a] != NULL; a++) {
            container = create_archetype(archs[a]);
            fail_unless(container != NULL, "couldn't find container arch %s!", archs[a]);
            fail_unless(container->type == CONTAINER, "%s isn't a container, type %d instead of %d!", archs[a], container->type, CONTAINER);
            fail_unless(container->carrying == 0, "%s has already some carrying?", archs[a]);

            container->stats.Str = reduction[red];

            for (i = 0; items[i] != NULL; i++) {
                item = create_archetype(items[i]);
                fail_unless(item != NULL, "couldn't find item %s to put", items[i]);
                fail_unless(item->nrof == 1, "can't test item %s with nrof != 1", items[i]);
                sum = 1;

                object_insert_in_ob(item, container);
                carrying = (signed long)(item->weight * (100 - container->stats.Str) / 100);
                fail_unless(container->carrying == carrying, "invalid weight %d instead of %d for %s in %s reduction %d!", container->carrying, carrying, items[i], archs[a], reduction[red]);

                for (nrof = 0; add[nrof] != 0; nrof++) {
                    sum += add[nrof];
                    if (add[nrof] > 0) {
                        item = create_archetype(items[i]);
                        item->nrof = add[nrof];
                        item = object_insert_in_ob(item, container);
                    } else {
                        object_decrease_nrof(item, - add[nrof]);
                    }
                    fail_unless(item->nrof == sum, "invalid item count %d instead of %d", item->nrof, sum);
                    carrying = (signed long)(item->nrof * item->weight * (100 - container->stats.Str) / 100);
                    fail_unless(container->carrying == carrying, "invalid weight %d instead of %d for %s in %s reduction %d after adding %d!", container->carrying, carrying, items[i], archs[a], reduction[red], add[nrof]);
                }
                object_remove(item);
                object_free2(item, 0);
                fail_unless(container->carrying == 0, "container %s is still carrying %d from %s instead of 0!", archs[a], container->carrying, items[i]);
            }
            object_free2(container, 0);
        }
    }
}
END_TEST

static Suite *bug_suite(void) {
    Suite *s = suite_create("bug");
    TCase *tc_core = tcase_create("Core");

    tcase_add_checked_fixture(tc_core, setup, teardown);

    suite_add_tcase(s, tc_core);
    tcase_add_test(tc_core, test_weight_reduction);
    tcase_set_timeout(tc_core, 0);

    return s;
}

int main(void) {
    int nf;
    Suite *s = bug_suite();
    SRunner *sr = srunner_create(s);

    srunner_set_fork_status(sr, CK_NOFORK);
    init(0, NULL);

    srunner_set_xml(sr, LOGDIR "/bugs/bugtrack/weight_reduction.xml");
    srunner_set_log(sr, LOGDIR "/bugs/bugtrack/weight_reduction.out");
    srunner_run_all(sr, CK_ENV); /*verbosity from env variable*/
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return nf == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

