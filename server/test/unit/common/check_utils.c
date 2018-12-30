/*
 * static char *rcsid_check_utils_c =
 *   "$Id: check_utils.c 15993 2011-12-01 19:34:01Z ryo_saeba $";
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
 * This is the unit tests file for common/utils.c
 */

#include <check.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "global.h"


static void setup(void) {
    /* put any initialisation steps here, they will be run before each testcase */
}

static void teardown(void) {
    /* put any cleanup steps here, they will be run after each testcase */
}

static void check_split_string(const char *str, size_t array_size, ...) {
    char tmp[256];
    char *array[64];
    size_t result;
    size_t i;
    va_list arg;

    snprintf(tmp, sizeof(tmp), "%s", str);
    for(i = 0; i < sizeof(array)/sizeof(*array); i++)
        array[i] = NULL;
    result = split_string(tmp, array, array_size, ':');
    fail_if(result > array_size, "result == %zu > %zu == array_size", result, array_size);
    va_start(arg, array_size);
    for(i = 0; i < sizeof(array)/sizeof(*array); i++) {
        const char *expected_result;

        expected_result = va_arg(arg, const char *);
        if (expected_result == NULL)
            break;

        if (i >= array_size)
            fail("internal error: too many arguments passed to check_split_string()");
        if (i < result)
            fail_if(strcmp(array[i], expected_result) != 0, "strcmp(array[%zu] == %s, %s) != 0", i, array[i], expected_result);
        else
            fail_if(array[i] != NULL, "array[%zu] == NULL", i);
    }
    va_end(arg);
    fail_if(result != i, "%zu != %zu", result, i);
}

START_TEST(test_split_string) {
    check_split_string("", 0, NULL);
    check_split_string("", 5, "", NULL);
    check_split_string(":", 5, "", "", NULL);
    check_split_string("::", 5, "", "", "", NULL);
    check_split_string("abc:def:ghi", 0, NULL);
    check_split_string("abc:def:ghi", 1, "abc:def:ghi", NULL);
    check_split_string("abc:def:ghi", 2, "abc", "def:ghi", NULL);
    check_split_string("abc:def:ghi", 3, "abc", "def", "ghi", NULL);
    check_split_string("abc:def:ghi", 4, "abc", "def", "ghi", NULL);
    check_split_string("::abc::def::", 0, NULL);
    check_split_string("::abc::def::", 1, "::abc::def::", NULL);
    check_split_string("::abc::def::", 2, "", ":abc::def::", NULL);
    check_split_string("::abc::def::", 3, "", "", "abc::def::", NULL);
    check_split_string("::abc::def::", 4, "", "", "abc", ":def::", NULL);
    check_split_string("::abc::def::", 5, "", "", "abc", "", "def::", NULL);
    check_split_string("::abc::def::", 6, "", "", "abc", "", "def", ":", NULL);
    check_split_string("::abc::def::", 7, "", "", "abc", "", "def", "", "", NULL);
    check_split_string("::abc::def::", 8, "", "", "abc", "", "def", "", "", NULL);
}
END_TEST

START_TEST(test_replace) {
    char init[MAX_BUF], replaced[MAX_BUF];

    snprintf(init, sizeof(init), "first test");
    replace(init, "first", "other", replaced, sizeof(replaced));
    fail_if(strcmp(replaced, "other test") != 0, "%s != other test", replaced);

    snprintf(init, sizeof(init), "second test");
    replace(init, "e", "a", replaced, sizeof(replaced));
    fail_if(strcmp(replaced, "sacond tast") != 0, "%s != sacond tast", replaced);

    /* corner case for length */
    snprintf(init, sizeof(init), "long test");
    replace(init, "long", "really long", replaced, strlen(init) + 1);
    fail_if(strcmp(replaced, "really lo") != 0, "%s != really lo", replaced);
}
END_TEST

static Suite *utils_suite(void) {
    Suite *s = suite_create("utils");
    TCase *tc_core = tcase_create("Core");

    /*setup and teardown will be called before each test in testcase 'tc_core' */
    tcase_add_checked_fixture(tc_core, setup, teardown);

    suite_add_tcase(s, tc_core);
    tcase_add_test(tc_core, test_split_string);
    tcase_add_test(tc_core, test_replace);

    return s;
}

int main(void) {
    int nf;
    Suite *s = utils_suite();
    SRunner *sr = srunner_create(s);

    srunner_set_fork_status(sr, CK_NOFORK);
    srunner_set_xml(sr, LOGDIR "/unit/common/utils.xml");
    srunner_set_log(sr, LOGDIR "/unit/common/utils.out");
    srunner_run_all(sr, CK_ENV); /*verbosity from env variable*/
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
