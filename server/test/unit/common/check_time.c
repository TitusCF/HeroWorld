/*
 * static char *rcsid_check_time_c =
 *   "$Id: check_time.c 15379 2011-11-01 19:40:45Z ryo_saeba $";
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
 * This is the unit tests file for common/time.c
 */

#include <stdlib.h>
#include <check.h>
#include <global.h>
#include <libproto.h>
#include "tod.h"

static void setup(void) {
    reset_sleep();
}

static void teardown(void) {
    /* put any cleanup steps here, they will be run after each testcase */
}

START_TEST(test_get_month_name) {
    fail_unless(get_month_name(-1) == NULL, "getting month name for negative value should bring a NULL");
    fail_unless(get_month_name(MONTHS_PER_YEAR) == NULL, "getting month name for too high value should bring a NULL");
    fail_unless(get_month_name(0) != NULL, "getting month name for correct value should bring a non NULL season name");
}
END_TEST

START_TEST(test_get_weekday) {
    fail_unless(get_weekday(-1) == NULL, "getting week day name for negative value should bring a NULL");
    fail_unless(get_weekday(DAYS_PER_WEEK) == NULL, "getting weekday name for too high value should bring a NULL");
    fail_unless(get_weekday(0) != NULL, "getting weekday name for correct value should bring a non NULL season name");
}
END_TEST

START_TEST(test_get_season_name) {
    fail_unless(get_season_name(-1) == NULL, "getting season name for negative value should bring a NULL");
    fail_unless(get_season_name(SEASONS_PER_YEAR+2) == NULL, "getting season name for too high value should bring a NULL");
    fail_unless(get_season_name(SEASONS_PER_YEAR) != NULL, "getting season name for limit value should bring a '\\n'");
    fail_unless(strcmp(get_season_name(SEASONS_PER_YEAR), "\n") == 0, "getting season name for limit value should bring a '\n'");
    fail_unless(get_season_name(0) != NULL, "getting season name for correct value should bring a non NULL season name");
}
END_TEST

static Suite *time_suite(void) {
    Suite *s = suite_create("time");
    TCase *tc_core = tcase_create("Core");

    /*setup and teardown will be called before each test in testcase 'tc_core' */
    tcase_add_checked_fixture(tc_core, setup, teardown);

    suite_add_tcase(s, tc_core);
    tcase_add_test(tc_core, test_get_month_name);
    tcase_add_test(tc_core, test_get_season_name);
    tcase_add_test(tc_core, test_get_weekday);

    return s;
}

int main(void) {
    int nf;
    Suite *s = time_suite();
    SRunner *sr = srunner_create(s);

    srunner_set_xml(sr, LOGDIR "/unit/common/time.xml");
    srunner_set_log(sr, LOGDIR "/unit/common/time.out");
    srunner_run_all(sr, CK_ENV); /*verbosity from env variable*/
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
