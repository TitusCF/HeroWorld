/*
 * static char *rcsid_check_shstr_c =
 *   "$Id: check_shstr.c 15379 2011-11-01 19:40:45Z ryo_saeba $";
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
 * This is the unit tests file for common/shstr.c
 */

#include <stdlib.h>
#include <string.h>
#include <check.h>
#include "global.h"

/*
extern const char *add_string(const char *str);
extern const char *add_refcount(const char *str);
extern int query_refcount(const char *str);
extern const char *find_string(const char *str);
extern void free_string(const char *str);
extern int buf_overflow(const char *buf1, const char *buf2, int bufsize);
*/

static void setup(void) {
    init_hash_table();
}

static void teardown(void) {
    /* nothing to do */
}

START_TEST(test_add_string) {
    const char *str1;
    const char *str2;
    const char *str3;
    char *temp;
#ifndef MANY_CORES
    fail_unless(add_string(NULL) == NULL, "add_string should null when receiving a null as parameter.");
#endif
    str1 = add_string("Hello world");
    fail_unless(str1 != NULL, "add_string should not return null when receiving content.");
    temp = malloc(strlen(str1)+1);
    strcpy(temp, str1);
    str2 = add_string(temp);
    fail_unless(str2 == str1, "add_string should return same pointer for 2 same strings but str1 (%p -> '%s') != str2 (%p -> '%s').", str1, str1, str2, str2);
    str3 = add_string("");
    fail_unless(str3 != NULL, "add_string should handle gracefully empty non null strings.");
    free(temp);
}
END_TEST

START_TEST(test_add_refcount) {
    const char *str1;
    const char *str2;

    str1 = add_string("Crossfire Rulez");
    str2 = add_refcount(str1);
    fail_unless(str1 == str2, "result of add_refcount (%p) should be the same as original pointer (%p).", str2, str1);
    fail_unless(query_refcount(str1) == 2, "add_refcount (%p) should have made refcount to value 2 but was %d", str1, query_refcount(str1));
}
END_TEST

START_TEST(test_query_refcount) {
    const char *str1;

    str1 = add_string("Hello World");
    fail_unless(query_refcount(str1) == 1, "After add_string, query_refcount should return 1 but returned %d(0x%X) for %s", query_refcount(str1), query_refcount(str1), str1);
    add_string("Hello World");
    fail_unless(query_refcount(str1) == 2, "After twice add_string with same string, query_refcount should return 2 but returned %d(0x%X) for %s", query_refcount(str1), query_refcount(str1), str1);
    add_refcount(str1);
    fail_unless(query_refcount(str1) == 3, "After call to add_refcount, query_refcount should now return 3 but returned %d(0x%X) for %s", query_refcount(str1), query_refcount(str1), str1);
}
END_TEST

START_TEST(test_find_string) {
    const char *str1;
    const char *str2;
    const char *result;

    str1 = add_string("Hello world");
    str2 = add_string("Bonjour le monde");
    result = find_string("Hello world");
    fail_unless(str1 == result, "find_string for %s should return %p but returned %p(%s).", str1, str1, result, result);
    result = find_string("Bonjour le monde");
    fail_unless(str2 == result, "find_string for %s should return %p but returned %p(%s).", str2, str2, result, result);
    result = find_string("Hola mundo");
    fail_unless(result == NULL, "Searching for an inexistant string should return NULL but returned %p(%s)", result, result);
    str1 = add_string("");
    result = find_string("");
    fail_unless(result == str1, "Search for empty string should return it(%p), but returned %p", str1, result);
    free_string(str2);
    result = find_string("Bonjour le monde");
    fail_unless(result == NULL, "add_string + free_string should mean i can't find the string anymore but find string returned %p(%s)", result, result);
}
END_TEST

START_TEST(test_free_string) {
    const char *str1;
    const char *str2;

    str1 = add_string("Cr0ssf1r3 r|_|1z");
    free_string(str1);
    str2 = find_string("Cr0ssf1r3 r|_|1z");
    fail_unless(str2 == NULL, "find_String should return null after a free_string but it returned %p (%s)", str2, str2);
    str1 = add_string("bleh");
    add_string("bleh");
    free_string(str1);
    str2 = find_string("bleh");
    fail_unless(str2 == str1, "find_string should return the string(%p) after a add_string, add_string, free_string but returned %p", str1, str2);
    free_string(str1);
    str2 = find_string("bleh");
    fail_unless(str2 == NULL, "find_string should return null after add_string, add_string, free_string, free_string but returned %p", str2);
}
END_TEST

START_TEST(test_buf_overflow) {
    int i;

    i = buf_overflow("1", "22", 3);
    fail_unless(i, "'1' +'22' can't fit in a 3 char buffer but buf_overflow told us there won't be any overflow");
    i = buf_overflow("1", NULL, 1);
    fail_unless(i, "'1' +NULL can't fit in a 1 char buffer but buf_overflow told us there won't be any overflow");
    i = buf_overflow("1", NULL, 2);
    fail_unless(!i, "'1' +NULL can fit in a 2 char buffer but buf_overflow told us it won't");
    i = buf_overflow("", NULL, 1);
    fail_unless(!i, "EMPTY +NULL can fit in a 1 char buffer but buf_overflow told us it won't");
    i = buf_overflow("", NULL, 0);
    fail_unless(i, "EMPTY +NULL can't fit in a 0 char buffer but buf_overflow told us there won't be any overflow");
}
END_TEST

static Suite *shstr_suite(void) {
    Suite *s = suite_create("shstr");
    TCase *tc_core = tcase_create("Core");

    /*setup and teardown will be called before each test in testcase 'tc_core' */
    tcase_add_checked_fixture(tc_core, setup, teardown);

    suite_add_tcase(s, tc_core);
    tcase_add_test(tc_core, test_add_string);
    tcase_add_test(tc_core, test_add_refcount);
    tcase_add_test(tc_core, test_query_refcount);
    tcase_add_test(tc_core, test_find_string);
    tcase_add_test(tc_core, test_free_string);
    tcase_add_test(tc_core, test_buf_overflow);

    return s;
}

int main(void) {
    int nf;
    Suite *s = shstr_suite();
    SRunner *sr = srunner_create(s);

    srunner_set_xml(sr, LOGDIR "/unit/common/shstr.xml");
    srunner_set_log(sr, LOGDIR "/unit/common/shstr.out");
    srunner_run_all(sr, CK_ENV); /*verbosity from env variable*/
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
