/*
 * static char *rcsid_check_c_party_c =
 *   "$Id: check_c_party.c 15379 2011-11-01 19:40:45Z ryo_saeba $";
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
 * This is the unit tests file for server/c_party.c
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

START_TEST(test_party) {
    partylist *p1, *p2, *p3;
    object *pl1;
    object *pl2;
    object *pl3;

    fail_unless(party_get_first() == NULL, "firstparty should be NULL!");

    pl1 = calloc(1, sizeof(object));
    pl1->name = "player1";
    fail_unless(pl1 != NULL, "memory allocation failure");
    pl1->contr = calloc(1, sizeof(player));
    fail_unless(pl1->contr != NULL, "memory allocation failure");
    first_player = pl1->contr; /* needed because obsolete parties uses this. */
    pl1->contr->ob = pl1;

    pl2 = calloc(1, sizeof(object));
    pl2->name = "player2";
    fail_unless(pl2 != NULL, "memory allocation failure");
    pl2->contr = calloc(1, sizeof(player));
    fail_unless(pl2->contr != NULL, "memory allocation failure");
    first_player = pl2->contr; /* needed because obsolete parties uses this. */
    pl2->contr->ob = pl2;

    pl3 = calloc(1, sizeof(object));
    pl3->name = "player2";
    fail_unless(pl3 != NULL, "memory allocation failure");
    pl3->contr = calloc(1, sizeof(player));
    fail_unless(pl3->contr != NULL, "memory allocation failure");
    first_player = pl3->contr; /* needed because obsolete parties uses this. */
    pl3->contr->ob = pl3;

    p1 = party_form(pl1, "test1");
    fail_unless(p1 != NULL, "party_form failed.");
    fail_unless(party_get_first() == p1, "firstparty wasn't updated");
    fail_unless(strcmp(p1->partyname, "test1") == 0, "wrong party name");
    fail_unless(p1 == pl1->contr->party, "player wasn't added to party");
    fail_unless(strcmp(party_get_leader(p1), "player1") == 0, "wrong party leader");

    p2 = party_form(pl2, "test2");
    fail_unless(p2 != NULL, "party_form failed.");
    fail_unless(party_get_next(party_get_first()) == p2, "party incorrectly linked");

    party_remove(p1);

    fail_unless(party_get_first() == p2, "party incorrectly removed");

    p3 = party_form(pl3, "test3");
    fail_unless(p3 != NULL, "party_form failed");
    fail_unless(party_get_next(party_get_first()) == p3, "party p3 incorrectly linked");
    fail_unless(pl3->contr->party == p3, "p3 incorrectly assigned to pl3");

    party_obsolete_parties();
    fail_unless(party_get_first() == p3, "party p2 wasn't removed by obsolete_parties(), party %s still there", party_get_first() ? party_get_first()->partyname : "NULL party?");
}
END_TEST

static Suite *c_party_suite(void) {
    Suite *s = suite_create("c_party");
    TCase *tc_core = tcase_create("Core");

    /*setup and teardown will be called before each test in testcase 'tc_core' */
    tcase_add_checked_fixture(tc_core, setup, teardown);

    suite_add_tcase(s, tc_core);
    tcase_add_test(tc_core, test_party);

    return s;
}

int main(void) {
    int nf;
    Suite *s = c_party_suite();
    SRunner *sr = srunner_create(s);

    srunner_set_xml(sr, LOGDIR "/unit/server/c_party.xml");
    srunner_set_log(sr, LOGDIR "/unit/server/c_party.out");
    srunner_run_all(sr, CK_ENV); /*verbosity from env variable*/
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
