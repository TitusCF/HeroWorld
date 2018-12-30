/*
 * static char *rcsid_check_loader_c =
 *   "$Id: check_loader.c 15379 2011-11-01 19:40:45Z ryo_saeba $";
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
 * This is the unit tests file for common/loader.c
 */

#include <global.h>
#include <stdlib.h>
#include <check.h>
#include <loader.h>
#include <toolkit_common.h>
#include <object.h>
#include <stringbuffer.h>

static void setup(void) {
    cctk_setdatadir(BUILD_ROOT "lib");
    cctk_setlog(LOGDIR "/unit/common/loader.out");
    cctk_init_std_archetypes();
}

static void teardown(void) {
    /* put any cleanup steps here, they will be run after each testcase */
}

START_TEST(test_get_ob_diff) {
    StringBuffer *buf;
    object *orc;
    archetype *arch;
    char *result;

    arch = find_archetype("orc");
    fail_unless(arch != NULL, "Can't find 'orc' archetype!");
    orc = arch_to_object(arch);
    fail_unless(orc != NULL, "Couldn't create first orc!");

    buf = stringbuffer_new();
    get_ob_diff(buf, orc, &arch->clone);
    result = stringbuffer_finish(buf);
    fail_unless(result && result[0] == '\0', "diff obj/clone was %s!", result);
    free(result);

    orc->speed = 0.5;
    orc->speed_left = arch->clone.speed_left;
    FREE_AND_COPY(orc->name, "Orc chief");

    buf = stringbuffer_new();
    get_ob_diff(buf, orc, &arch->clone);
    result = stringbuffer_finish(buf);
    fail_unless(result && strcmp(result, "name Orc chief\nspeed 0.500000\n") == 0, "diff modified obj/clone was %s!", result);
    free(result);

    orc->stats.hp = 50;
    orc->stats.Wis = 59;
    orc->expmul = 8.5;
    orc->stats.dam = 168;

    buf = stringbuffer_new();
    get_ob_diff(buf, orc, &arch->clone);
    result = stringbuffer_finish(buf);
    fail_unless(result && strcmp(result, "name Orc chief\nWis 59\nhp 50\nexpmul 8.500000\ndam 168\nspeed 0.500000\n") == 0, "2n diff modified obj/clone was %s!", result);
    free(result);
}
END_TEST

START_TEST(test_object_dump) {
    /** we only test specific things like env/more/head/..., the rest is in test_get_ob_diff(). */
    StringBuffer *buf;
    object *empty;
    char *result;
    char expect[10000];

    /* Basic */
    empty = arch_to_object(empty_archetype);
    fail_unless(empty != NULL, "Couldn't create empty archetype!");

    snprintf(expect, sizeof(expect), "arch empty_archetype\nend\n");
    buf = stringbuffer_new();
    object_dump(empty, buf);
    result = stringbuffer_finish(buf);
    fail_unless(result && strcmp(result, expect) == 0, "object_dump was \"%s\" instead of \"%s\"!", result, expect);
    free(result);

    /* With more things */
    empty->head = arch_to_object(empty_archetype);
    empty->inv = arch_to_object(empty_archetype);
    empty->more = arch_to_object(empty_archetype);
    empty->env = arch_to_object(empty_archetype);
    fail_unless(empty->head != NULL, "Couldn't create empty archetype as head!");
    fail_unless(empty->inv != NULL, "Couldn't create empty archetype as inv!");
    fail_unless(empty->more != NULL, "Couldn't create empty archetype as more!");
    fail_unless(empty->env != NULL, "Couldn't create empty archetype as env!");

    snprintf(expect, sizeof(expect), "arch empty_archetype\nmore %d\nhead %d\nenv %d\ninv %d\nend\n", empty->more->count, empty->head->count, empty->env->count, empty->inv->count);
    buf = stringbuffer_new();
    object_dump(empty, buf);
    result = stringbuffer_finish(buf);
    fail_unless(result && strcmp(result, expect) == 0, "object_dump was \"%s\" instead of \"%s\"!", result, expect);
    free(result);
}
END_TEST

static Suite *loader_suite(void) {
    Suite *s = suite_create("loader");
    TCase *tc_core = tcase_create("Core");

    /*setup and teardown will be called before each test in testcase 'tc_core' */
    tcase_add_checked_fixture(tc_core, setup, teardown);

    suite_add_tcase(s, tc_core);
    tcase_add_test(tc_core, test_get_ob_diff);
    tcase_add_test(tc_core, test_object_dump);

    return s;
}

int main(void) {
    int nf;
    Suite *s = loader_suite();
    SRunner *sr = srunner_create(s);

    srunner_set_xml(sr, LOGDIR "/unit/common/loader.xml");
    srunner_set_log(sr, LOGDIR "/unit/common/loader.out");
    srunner_run_all(sr, CK_ENV); /*verbosity from env variable*/
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
