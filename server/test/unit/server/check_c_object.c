/*
 * static char *rcsid_check_c_object_c =
 *   "$Id: check_c_object.c 15379 2011-11-01 19:40:45Z ryo_saeba $";
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
 * This is the unit tests file for server/c_object.c
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

static object *find_best_apply_object_match(object *start, object *pl, const char *params, int aflag) {
    object *tmp, *best = NULL;
    int match_val = 0, tmpmatch;

    for (tmp = start; tmp; tmp = tmp->below) {
        if (tmp->invisible)
            continue;
        if ((tmpmatch = object_matches_string(pl, tmp, params)) > match_val) {
            if ((aflag == AP_APPLY) && (QUERY_FLAG(tmp, FLAG_APPLIED)))
                continue;
            if ((aflag == AP_UNAPPLY) && (!QUERY_FLAG(tmp, FLAG_APPLIED)))
                continue;
            match_val = tmpmatch;
            best = tmp;
        }
    }
    return best;
}

START_TEST(test_find_best_apply_object_match) {
    object *pl, *found;
    object *gorokh, *cloak, *other;

    pl = create_archetype("kobold");
    fail_unless(pl != NULL, "can't find kobold archetype.");

    gorokh = create_archetype("cloak");
    gorokh->title = add_string("of Gorokh");
    CLEAR_FLAG(gorokh, FLAG_IDENTIFIED);
    object_insert_in_ob(gorokh, pl);

    cloak = create_archetype("cloak");
    object_insert_in_ob(cloak, pl);

    other = create_archetype("gem");
    object_insert_in_ob(other, pl);

    found = find_best_apply_object_match(pl->inv, pl, "all", 0);
    fail_unless(found == other, "not found gem but %s", found ? found->name : "nothing");

    found = find_best_apply_object_match(pl->inv, pl, "cloak", 0);
    fail_unless(found == cloak, "didn't find cloak but %s", found ? found->name : "nothing");

    found = find_best_apply_object_match(pl->inv, pl, "Gorokh", 0);
    fail_unless(found == NULL, "Gorokh found %s instead of nothing", found ? found->name : "nothing??");
}
END_TEST

START_TEST(test_put_object_in_sack) {
    mapstruct *test_map;
    object *sack, *obj, *sack2, *dummy;

    dummy = create_archetype("orc");

    test_map = get_empty_map(5, 5);
    fail_unless(test_map != NULL, "can't create test map");

    sack = create_archetype("gem");
    object_insert_in_map_at(sack, test_map, NULL, 0, 0, 0);
    fail_unless(GET_MAP_OB(test_map, 0, 0) == sack);

    obj = create_archetype("gem");
    obj->nrof = 1;
    object_insert_in_map_at(obj, test_map, NULL, 0, 1, 0);
    put_object_in_sack(dummy, sack, obj, 1);
    fail_unless(GET_MAP_OB(test_map, 1, 0) == obj, "object was removed from map?");
    fail_unless(sack->inv == NULL, "sack's inventory isn't null?");

    object_remove(sack);
    object_free_drop_inventory(sack);

    /* basic insertion */
    sack = create_archetype("sack");
    sack->nrof = 1;
    fail_unless(sack->type == CONTAINER, "sack isn't a container?");
    object_insert_in_map_at(sack, test_map, NULL, 0, 0, 0);
    fail_unless(GET_MAP_OB(test_map, 0, 0) == sack, "sack not put on map?");

    SET_FLAG(sack, FLAG_APPLIED);
    put_object_in_sack(dummy, sack, obj, 1);
    fail_unless(sack->inv == obj, "object not inserted into sack?");
    fail_unless(GET_MAP_OB(test_map, 1, 0) == NULL, "object wasn't removed from map?");

    object_remove(obj);
    object_insert_in_map_at(obj, test_map, NULL, 0, 1, 0);
    sack->weight_limit = 1;
    obj->weight = 5;

    put_object_in_sack(dummy, sack, obj, 1);
    fail_unless(sack->inv == NULL, "item was put in sack even if too heavy?");
    fail_unless(GET_MAP_OB(test_map, 1, 0) == obj, "object was removed from map?");

    /* now for sack splitting */
    sack->nrof = 2;
    obj->weight = 1;

    put_object_in_sack(dummy, sack, obj, 1);
    fail_unless(sack->nrof == 1, "sack wasn't split?");
    fail_unless(sack->above != NULL, "no new sack created?");
    fail_unless(sack->inv == obj, "object not inserted in old sack?");
    fail_unless(sack == obj->env, "object's env not updated?");

    /* now moving to/from containers */
    obj->nrof = 2;
    sack2 = sack->above;
    SET_FLAG(sack2, FLAG_APPLIED);
    dummy->container = sack;
    put_object_in_sack(dummy, sack, sack2, 1);
    fail_unless(sack2->inv == NULL, "sack2's not empty?");
    fail_unless(sack->inv == obj, "obj wasn't transferred?");

    /* move between containers and split containers */
    object_remove(sack2);
    object_insert_in_map_at(sack2, test_map, NULL, 0, 2, 0);
    SET_FLAG(sack2, FLAG_APPLIED);
    sack2->nrof = 2;
    dummy->container = sack2;
    put_object_in_sack(dummy, sack2, sack, 0);
    fail_unless(sack->inv == NULL, "sack wasn't put into sack2?");
    fail_unless(sack2->inv != NULL, "sack2 wasn't filled?");
    fail_unless(sack2->above != NULL, "sack2 wasn't split?");
    fail_unless(sack2->above->inv == NULL, "sack2's split was filled?");

    free_map(test_map);
}
END_TEST

static Suite *c_object_suite(void) {
    Suite *s = suite_create("c_object");
    TCase *tc_core = tcase_create("Core");

    /*setup and teardown will be called before each test in testcase 'tc_core' */
    tcase_add_checked_fixture(tc_core, setup, teardown);

    suite_add_tcase(s, tc_core);
    tcase_add_test(tc_core, test_find_best_apply_object_match);
    tcase_add_test(tc_core, test_put_object_in_sack);

    return s;
}

int main(void) {
    int nf;
    Suite *s = c_object_suite();
    SRunner *sr = srunner_create(s);

    settings.debug = 0;
    settings.logfilename = "c_object.out";
    init(0, NULL);

    /* srunner_set_fork_status (sr, CK_NOFORK); */

    srunner_set_xml(sr, LOGDIR "/unit/server/c_object.xml");
    srunner_set_log(sr, LOGDIR "/unit/server/c_object.out");
    srunner_run_all(sr, CK_ENV); /*verbosity from env variable*/
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
