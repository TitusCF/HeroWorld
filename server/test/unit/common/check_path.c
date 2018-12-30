/*
 * static char *rcsid_check_path_c =
 *   "$Id: check_path.c 15379 2011-11-01 19:40:45Z ryo_saeba $";
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
 * This is the unit tests file for common/path.c
 */

#include <stdlib.h>
#include <string.h>
#include <check.h>
#include <global.h>

#include "path.h"

static void setup(void) {
    /* put any initialisation steps here, they will be run before each testcase */
}

static void teardown(void) {
    /* put any cleanup steps here, they will be run after each testcase */
}

static void check_combine(const char *src, const char *dst, const char *exp) {
    char res[HUGE_BUF];

    path_combine(src, dst, res, HUGE_BUF);
    fail_unless(strcmp(res, exp) == 0, "path_combine(%s, %s) = %s but should be %s", src, dst, res, exp);
}

static void check_normalize(const char *path, const char *exp) {
    char tmp[HUGE_BUF];

    /* This is needed as path_normalize modifies in place. */
    strncpy(tmp, path, sizeof(tmp));
    tmp[HUGE_BUF-1] = '\0';
    path_normalize(tmp);
    fail_unless(strcmp(tmp, exp) == 0, "path_normalize(%s) = %s but should be %s", path, tmp, exp);
}

static void check_combine_and_normalize(const char *src, const char *dst, const char *exp) {
    char res[HUGE_BUF];

    path_combine_and_normalize(src, dst, res, sizeof(res));
    fail_unless(strcmp(res, exp) == 0, "path_combine_and_normalize(%s, %s) = %s but should be %s", src, dst, res, exp);
}

START_TEST(test_path_combine) {
    check_combine("/path1/file1", "/path2/file2", "/path2/file2");
    check_combine("path1/file1", "/path2/file2", "/path2/file2");
    check_combine("/path1/file1", "path2/file2", "/path1/path2/file2");
    check_combine("path1/file1", "path2/file2", "path1/path2/file2");
    check_combine("/path1", "/path2", "/path2");
    check_combine("path1", "/path2", "/path2");
    check_combine("/path1", "path2", "/path2");
    check_combine("path1", "path2", "path2");
}
END_TEST

START_TEST(test_path_normalize) {
    check_normalize("", "");
    check_normalize("/", "/");
    check_normalize("path1/file1", "path1/file1");
    check_normalize("/path1/file1", "/path1/file1");
    check_normalize("/path1//file1", "/path1/file1");
    check_normalize("//path1/file1", "/path1/file1");
    check_normalize("///////x////////y///////z////////", "/x/y/z");
    check_normalize("/a/b/../c/d/../e/../../f/g/../h", "/a/f/h");
    check_normalize("//a//b//..//c//d//..//e//..//..//f//g//..//h", "/a/f/h");
    check_normalize("../a", "../a");
    check_normalize("a/../../b", "../b");
    check_normalize("/../a", "/a");
    check_normalize("/a/../../b", "/b");
    check_normalize("./b/./c/.d/..e/./f", "b/c/.d/..e/f");
    check_normalize("/b/././././e", "/b/e");
    check_normalize(".", ""); /* maybe the result should be "."? */
    check_normalize("/.", "/");
    check_normalize("./", ""); /* maybe the result should be "."? */
    check_normalize("/a/b/..", "/a");
    check_normalize("/a/b/../..", "/");
    check_normalize("/a/b/../../..", "/");
    check_normalize("a/b/..", "a");
    check_normalize("a/b/../..", "");
    check_normalize("a/b/../../..", "..");
}
END_TEST

START_TEST(test_path_combine_and_normalize) {
    check_combine_and_normalize("/path1/file1", "/path2/file2", "/path2/file2");
    check_combine_and_normalize("path1/file1", "/path2/file2", "/path2/file2");
    check_combine_and_normalize("/path1/file1", "path2/file2", "/path1/path2/file2");
    check_combine_and_normalize("/path1", "/path2", "/path2");
    check_combine_and_normalize("path1", "/path2", "/path2");
    check_combine_and_normalize("/path1", "path2", "/path2");
    check_combine_and_normalize("/path1/file1/../u", "path2/x/../y/z/../a/b/..", "/path1/path2/y/a");
    check_combine_and_normalize("/path1/file1", "/path2//file2", "/path2/file2");
    check_combine_and_normalize("/path1/file1", "/..", "/");
    check_combine_and_normalize("/path1/file1", "../x", "/x");
    check_combine_and_normalize("/path1/file1", "../../../x", "/x");
    check_combine_and_normalize("/path1/file1", "/.x/..x/...x/x", "/.x/..x/...x/x");
}
END_TEST

static Suite *path_suite(void) {
    Suite *s = suite_create("path");
    TCase *tc_core = tcase_create("Core");

    /*setup and teardown will be called before each test in testcase 'tc_core' */
    tcase_add_checked_fixture(tc_core, setup, teardown);

    suite_add_tcase(s, tc_core);
    tcase_add_test(tc_core, test_path_combine);
    tcase_add_test(tc_core, test_path_normalize);
    tcase_add_test(tc_core, test_path_combine_and_normalize);

    return s;
}

int main(void) {
    int nf;
    Suite *s = path_suite();
    SRunner *sr = srunner_create(s);

    srunner_set_xml(sr, LOGDIR "/unit/common/path.xml");
    srunner_set_log(sr, LOGDIR "/unit/common/path.out");
    srunner_run_all(sr, CK_ENV); /*verbosity from env variable*/
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
