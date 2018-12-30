/*
 * static char *rcsid_check_alchemy_c =
 *   "$Id: check_alchemy.c 15379 2011-11-01 19:40:45Z ryo_saeba $";
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
 * This is the unit tests file for server/alchemy.c
 */

#include <stdlib.h>
#include <check.h>
#include <global.h>
#include <assert.h>

static void setup(void) {
    /* put any initialisation steps here, they will be run before each testcase */
}

static void teardown(void) {
    /* put any cleanup steps here, they will be run after each testcase */
}

/* copied from alchemy.c */
static float recipe_chance(const recipe *rp, const object *skill) {
    assert(rp);
    assert(skill);

    if (skill->level < rp->diff - 10)
        return MAX(.01, .3 - (rp->diff - 10 - skill->level) * .03);

    if (skill->level <= rp->diff + 10)
        return .5 + .02 * (float)(skill->level - rp->diff);

    return MIN(.95, .70 + (skill->level - rp->diff - 10) * .01);
}


START_TEST(test_recipe_chance) {
    recipe rp;
    object skill;
    float chance;

    for (rp.diff = 0; rp.diff < 150; rp.diff++) {
        for (skill.level = 0; skill.level < 150; skill.level++) {
            chance = recipe_chance(&rp, &skill);
            /* use .009 because of floating point issues */
            fail_unless(chance >= .00999, "success can't be less than .01 but got %f for %d rp, %d skill", chance, rp.diff, skill.level);
            fail_unless(chance <= .95, "success can't be more than .95 but got %f for %d rp, %d skill", chance, rp.diff, skill.level);
            /*printf("%d %d => %f\n", rp.diff, skill.level, chance);*/
        }
        /*printf("\n");*/
    }
}
END_TEST

static Suite *alchemy_suite(void) {
    Suite *s = suite_create("alchemy");
    TCase *tc_core = tcase_create("Core");

    /*setup and teardown will be called before each test in testcase 'tc_core' */
    tcase_add_checked_fixture(tc_core, setup, teardown);

    suite_add_tcase(s, tc_core);
    tcase_add_test(tc_core, test_recipe_chance);

    return s;
}

int main(void) {
    int nf;
    Suite *s = alchemy_suite();
    SRunner *sr = srunner_create(s);

    srunner_set_xml(sr, LOGDIR "/unit/server/alchemy.xml");
    srunner_set_log(sr, LOGDIR "/unit/server/alchemy.out");
    srunner_run_all(sr, CK_ENV); /*verbosity from env variable*/
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
