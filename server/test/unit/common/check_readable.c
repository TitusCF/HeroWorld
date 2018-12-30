/*
 * static char *rcsid_check_readable_c =
 *   "$Id: check_readable.c 15379 2011-11-01 19:40:45Z ryo_saeba $";
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
 * This is the unit tests file for common/readable.c
 */

#include <stdlib.h>
#include <check.h>
#include <global.h>
#include <assert.h>
#include <spells.h>
#include <toolkit_common.h>

static void setup(void) {
    cctk_setdatadir(BUILD_ROOT "lib");
    cctk_setlog(LOGDIR "/unit/common/readable.out");
/*    printf("set log to %s\n", LOGDIR"/unit/common/readable.out");*/
    cctk_init_std_archetypes();
    init_gods();
}

static void teardown(void) {
    /* put any cleanup steps here, they will be run after each testcase */
}

static Suite *readable_suite(void) {
    Suite *s = suite_create("readable");
    TCase *tc_core = tcase_create("Core");

    /*setup and teardown will be called before each test in testcase 'tc_core' */
    tcase_add_checked_fixture(tc_core, setup, teardown);

    suite_add_tcase(s, tc_core);

    return s;
}

int main(void) {
    int nf;
    Suite *s = readable_suite();
    SRunner *sr = srunner_create(s);

    /* to debug, uncomment this line */
    srunner_set_fork_status(sr, CK_NOFORK);
    srunner_set_xml(sr, LOGDIR "/unit/common/readable.xml");
    srunner_set_log(sr, LOGDIR "/unit/common/readable.out");
    srunner_run_all(sr, CK_ENV); /*verbosity from env variable*/
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
