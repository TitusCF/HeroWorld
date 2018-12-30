/*
 * static char *rcsid_check_treasure_c =
 *   "$Id: check_treasure.c 18988 2013-09-06 09:48:16Z akirschbaum $";
 */

/*
 * CrossFire, A Multiplayer game for X-windows
 *
 * Copyright (C) 2002,2011 Mark Wedel & Crossfire Development Team
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
 * This is the unit tests file for common/treasure.c
 */

#include <stdlib.h>
#include <check.h>
#include <global.h>
#include <toolkit_common.h>

static void setup(void) {
    cctk_setdatadir(BUILD_ROOT "lib");
    cctk_setlog(LOGDIR "/unit/common/object.out");
    printf("set log to %s\n", LOGDIR"/unit/common/object.out");
    cctk_init_std_archetypes();
}

static void teardown(void) {
    /* put any cleanup steps here, they will be run after each testcase */
}

/* Note: This file is far from a complete test of all treasurelists.
 * but it tests at least one function from treasure.c.
 */

/*
 * This tests that the treasurelist_find_matching_type()
 * works as expected.
 * Unfortunately, this requires on the contents of treasurelists,
 * which sit in the arch tree and can change over time.
 *
 * TODO: Add more checks.  Started out with basic check
 * that was needed for skill system.
 *
 * The setup is this - treasurelists[] contain the list to look
 * at. items[][] is the types that we should find.
 * types[] is what type we are checking for.
 *
 * In the initial setup, we check the same treasurelist, but
 * for different types - in particular, to make sure we
 * properly handle case with nothing matching.
 */
START_TEST(test_treasurelist_find_matching_type) {
#define NUM_TREASURE_LISTS 2

    /* We want to use the top level treasure list - in that way,
     * we can test that the recursion works properly
     */
    const char *treasurelists[NUM_TREASURE_LISTS] = {"monk_class_items",
        "monk_class_items"};
    const char *items[NUM_TREASURE_LISTS][100] = { {
#if 0
        /* Until new skill system is commited, these do not
         * exist.
         */
        "skill_missile_weapon_9",
        "skill_sword_9", "skill_axe_9", "skill_blunt_weapon_9",
        "skill_spear_9", "skill_unarmed_combat_3", "skill_armor_3",
        "skill_dodge_3", "skill_air_magic_8", "skill_fire_magic_8",
        "skill_water_magic_8", "skill_earth_magic_8", "skill_divine_power_5",
        "skill_trap_5", "skill_thievery_5", "skill_persuasion_6",
        "skill_acrobatics_3", "skill_literacy_4", "skill_weaponsmith_9",
        "skill_armorer_4", "skill_jeweler_8", "skill_alchemy_8",
        "skill_bowyer_9", "skill_thaumaturgy_8", "skill_woodsman_5",
#endif
        /* Following are legacy skills */
        "skill_sense_curse", "skill_sense_magic", "skill_meditation",
        "skill_karate","skill_praying","skill_missile_weapon",
        "skill_punching","skill_literacy","skill_use_magic_item",
        "skill_remove_trap","skill_find_traps","skill_throwing",
         NULL
        },
       { NULL }
    };
    int types[NUM_TREASURE_LISTS] = {SKILL, HOLY_ALTAR };
    int tr;

    for (tr=0; tr < NUM_TREASURE_LISTS; tr++) {
        struct treasureliststruct *randomitems;
        int *results=NULL, list_size;
        objectlink *ol;

        randomitems = find_treasurelist(treasurelists[tr]);

        fail_unless(randomitems != NULL, "Could not find treasurelist %s",
                    treasurelists[tr]);

        /* Figure out number of items that this one should
         * return.
         */
        for (list_size=0; items[tr][list_size] != NULL; list_size++) ;

        if (list_size)
            results = calloc (list_size, sizeof(*results));

        /* Last parameter is if we traverse treasurelists - that
         * should probably be another test case, but I suspect
         * that might be something that is always set.
         */
        ol = treasurelist_find_matching_type(randomitems, types[tr], TRUE);

        fail_if(ol && (list_size == 0),
                "On test %d, should have returned null but did not", tr);

        fail_if((ol == NULL) && (list_size != 0),
                "On test %d, should have returned results but returned NULL",
                tr);

        /* Based on above, this could be simplified to be just
         * ol != NULL, but this is clearer and performance
         * is not important.
         */
        if ((ol !=NULL) && (list_size != 0)) {
            int i;
            objectlink *oltmp;

            for (oltmp = ol; oltmp != NULL; oltmp = oltmp->next) {
                fail_if(oltmp->ob == NULL,
                        "On test %d, found NULL object on linked list", tr);

                /* We do this loop because treasurelist_find_matching_type()
                 * specifically says that the order of results
                 * is not deterministic.
                 */
                for (i=0; i < list_size; i++) {
                    if (!strcmp(items[tr][i], oltmp->ob->arch->name)) {
                        results[i]++;
                        break;
                    }
                }

                /* Did not find match */
                fail_if(i == list_size,
                        "On test %d, did not find match for %s", tr,
                        oltmp->ob->arch->name);
            }
            /* treasurelist_find_matching_type() does state multiple
             * objects of the same type may be returned - to properly
             * check for this, we would need to have a result table
             * for each test.  What I am more concerned is that it
             * finds all the objects it should, and does not find
             * any it should not.
             */
            for (i=0; i < list_size; i++) {
                fail_if(results[i] == 0, "On test %d, got 0 matches for %s",
                        tr, items[tr][i]);
            }
        }
        free(results);
        if (ol) free_objectlink(ol);
    } /* for loop */
}



END_TEST

static Suite *treasure_suite(void) {
    Suite *s = suite_create("treasure");
    TCase *tc_core = tcase_create("Core");

    /*setup and teardown will be called before each test in testcase 'tc_core' */
    tcase_add_checked_fixture(tc_core, setup, teardown);

    suite_add_tcase(s, tc_core);
    tcase_add_test(tc_core, test_treasurelist_find_matching_type);

    return s;
}

int main(void) {
    int nf;
    Suite *s = treasure_suite();
    SRunner *sr = srunner_create(s);


    /* to debug, uncomment this line */
    srunner_set_fork_status(sr, CK_NOFORK);

    srunner_set_xml(sr, LOGDIR "/unit/common/treasure.xml");
    srunner_set_log(sr, LOGDIR "/unit/common/treasure.out");
    srunner_run_all(sr, CK_ENV); /*verbosity from env variable*/
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
