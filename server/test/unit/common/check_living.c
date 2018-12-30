/*
 * static char *rcsid_check_living_c =
 *   "$Id: check_living.c 18988 2013-09-06 09:48:16Z akirschbaum $";
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
 * This is the unit tests file for common/living.c
 */

#include <global.h>
#include <stdlib.h>
#include <check.h>
#include <toolkit_common.h>
#include <malloc.h>

static void setup(void) {
    cctk_setdatadir(BUILD_ROOT "lib");
    cctk_setconfdir(BUILD_ROOT "lib");
    cctk_setlog(LOGDIR "/unit/common/living.out");
    cctk_init_std_archetypes();
    init_experience();
}

static void teardown(void) {
    /* put any cleanup steps here, they will be run after each testcase */
}

/**
 * Note that the test and results here make assumptions about certain
 * values that can change.  For example, if maximum level (in the exp
 * file) is lower, the results may not match expected results.  Likewise,
 * if any of the stat bonuses are changed, these results also will
 * not match.
 */

START_TEST(test_fix_object) {
#define TESTS   50
#define ARCHS   3
    static const char *archs[ARCHS] = { "pl_dragon", "pl_half_orc", "human_player" };

    object *ob, *grace, *mana;
    player *pl;
    int test,i;

    /** It could be nice to put all these into a structure, so the actual
     * values for each test are in the same structure element.
     */
    /** First group of arrays here are the values that should be returned
     * from the test - basically the output values.
     */
    int wc[3][50] = {
        { 21, 3, 3, 22, 2, 10, 18, 10, 20, 14, 1, 14, 15, 9, 19, 4, 2, 3, 16, 17, 9, 2, 11, 14, 4, 11, 14, 12, 8, 20, 16, 21, 19, 3, 12, 6, 14, 8, 10, 14, 15, 17, 6, 21, 16, 9, 3, 19, 2, 20},
        { 6, 23, 1, 2, 6, 14, 7, 6, 23, 13, 17, 9, 9, 19, 11, 18, 5, 15, 2, 3, 15, 9, 13, 13, 6, 19, 5, 21, 2, 12, 15, 6, 23, 19, 7, 20, 19, 3, 22, 7, 7, 12, 14, 15, 7, 15, 8, 10, 23, 11},
        { 1, 8, 5, 16, 16, 15, 9, 21, 4, 16, 20, 17, 12, 19, 1, 3, 23, 4, 2, 10, 17, 3, 20, 6, 19, 21, 1, 7, 12, 20, 8, 2, 23, 17, 17, 18, 22, 17, 6, 3, 17, 3, 18, 13, 6, 3, 5, 17, 23, 1}
    };

    int maxgr[3][50] = {
        { 333, 236, 182, 225, 160, 36, 53, 78, 221, 130, 204, 246, 242, 256, 38, 67, 419, 145, 282, 245, 315, 208, 340, 81, 120, 232, 176, 260, 354, 111, 332, 251, 249, 90, 215, 62, 42, 372, 241, 196, 216, 46, 94, 47, 264, 102, 86, 126, 289, 107},
        { 62, 119, 164, 309, 223, 236, 124, 98, 38, 302, 209, 155, 148, 134, 90, 220, 96, 262, 111, 126, 92, 236, 206, 131, 269, 255, 222, 88, 58, 102, 126, 111, 215, 170, 132, 156, 46, 153, 203, 88, 236, 43, 69, 167, 123, 36, 56, 168, 359, 52},
        { 42, 192, 232, 110, 164, 287, 90, 238, 160, 242, 225, 258, 66, 226, 165, 114, 189, 208, 88, 255, 97, 83, 40, 254, 130, 88, 109, 238, 238, 226, 46, 231, 172, 206, 42, 148, 126, 82, 97, 162, 161, 232, 108, 152, 205, 190, 212, 80, 54, 152}
    };

    int maxsp[3][50] = {
        { 64, 403, 72, 106, 156, 234, 214, 194, 183, 432, 219, 70, 208, 108, 140, 224, 263, 218, 211, 178, 194, 160, 48, 106, 165, 100, 349, 245, 218, 239, 336, 247, 323, 243, 148, 37, 160, 148, 132, 362, 156, 238, 286, 217, 229, 166, 156, 178, 245, 220},
        { 159, 218, 84, 115, 154, 159, 70, 78, 46, 243, 167, 142, 70, 130, 158, 164, 66, 180, 297, 252, 188, 142, 284, 80, 251, 61, 78, 207, 188, 210, 228, 236, 124, 32, 224, 228, 39, 246, 69, 188, 200, 210, 242, 158, 112, 184, 278, 229, 346, 220},
        { 74, 64, 156, 240, 217, 297, 114, 276, 82, 265, 273, 212, 207, 253, 172, 146, 110, 88, 72, 68, 220, 251, 224, 94, 135, 53, 151, 181, 150, 214, 92, 218, 184, 126, 172, 170, 58, 224, 90, 50, 152, 181, 208, 79, 79, 163, 106, 31, 62, 231}
    };

    int maxhp[3][50] = {
        { 79, 466, 246, 36, 238, 170, 80, 218, 196, 358, 279, 112, 110, 164, 62, 300, 380, 224, 119, 117, 358, 300, 218, 162, 212, 153, 306, 136, 178, 90, 186, 73, 304, 226, 188, 260, 114, 293, 267, 352, 106, 90, 244, 150, 164, 182, 228, 90, 476, 132},
        { 210, 34, 246, 351, 198, 254, 211, 196, 34, 188, 162, 196, 162, 140, 146, 72, 210, 158, 349, 264, 138, 164, 154, 136, 192, 91, 258, 82, 256, 278, 102, 238, 93, 64, 188, 171, 64, 420, 39, 184, 303, 202, 258, 104, 206, 121, 274, 346, 130, 142},
        { 259, 176, 219, 100, 92, 344, 164, 100, 218, 152, 248, 90, 182, 64, 242, 226, 110, 214, 239, 152, 150, 364, 54, 194, 112, 50, 250, 268, 138, 85, 174, 240, 32, 90, 82, 74, 45, 82, 284, 226, 109, 246, 78, 154, 222, 254, 202, 82, 33, 248}
    };

    int ac[3][50] = {
        { 4, -7, -7, 6, -7, -7, -2, -7, 2, -7, -7, -7, -7, -7, 1, -7, -7, -7, -4, -2, -7, -7, -7, -7, -7, -7, -7, -7, -7, 2, -4, 4, 1, -7, -7, -7, -7, -7, -7, -7, -6, -3, -7, 3, -4, -7, -7, 0, -7, 3},
        { 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13},
        { 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13}
    };


    /**
     * Follow group of arrays are values to use for the different attributes -
     * in other word, basically the input values.
     */
    int wis[ARCHS][TESTS] = {
        {
            28, 26, 25, 25, 5, 16, 21, 7, 11, 25, 12, 16, 22, 23, 20, 7, 30, 20, 30, 28,
            26, 16, 29, 24, 27, 19, 12, 26, 29, 23, 30, 14, 15, 2, 21, 13, 9, 30, 17, 19,
            2, 13, 9, 24, 26, 21, 8, 28, 28, 7
        },{
            15, 19, 16, 26, 24, 19, 2, 16, 1, 27, 19, 15, 28, 21, 20, 3, 11, 23, 8, 4, 22,
            12, 7, 17, 27, 21, 6, 9, 12, 7, 9, 28, 9, 4, 12, 1, 15, 2, 27, 2, 13, 20, 1, 24, 15, 7,
            3, 11, 27, 1

        }, {
            3, 11, 14, 13, 7, 11, 3, 5, 6, 13, 15, 23, 4, 13, 29, 10, 15, 10, 7, 19, 19,
            14, 9, 25, 9, 11, 23, 29, 27, 30, 23, 30, 7, 16, 10, 9, 18, 26, 9, 11, 28, 8, 14, 25,
            17, 5, 19, 14, 21, 12
        }
    };

    int intelligence[ARCHS][TESTS] = {
        {
            2, 7, 21, 10, 22, 2, 21, 16, 23, 27, 8, 24, 11, 27, 3, 26, 28, 1, 11, 3, 11, 27,
            3, 15, 25, 13, 8, 25, 14, 12, 29, 11, 30, 30, 24, 5, 23, 3, 8, 10, 22, 21, 29,
            5, 26, 11, 7, 4, 24, 30
        },{
            20, 18, 16, 15, 27, 2, 3, 9, 19, 21, 10, 4, 19, 8, 14, 13, 26, 7, 28, 26, 30, 27,
            30, 2, 27, 16, 13, 22, 5, 5, 24, 20, 19, 18, 3, 17, 23, 11, 26, 11, 19, 3, 22, 29,
            5, 10, 22, 12, 26, 8
        },{
            2, 20, 17, 5, 30, 9, 28, 30, 16, 19, 4, 6, 12, 28, 2, 2, 30, 24, 13, 10, 1, 15, 27,
            16, 3, 23, 28, 10, 27, 4, 26, 4, 13, 9, 19, 23, 19, 19, 18, 13, 1, 24, 7, 12, 22,
            18, 14, 11, 6, 26
        }
    };
    int pow[ARCHS][TESTS] = {
        {
            19, 30, 16, 11, 5, 14, 6, 22, 28, 30, 19, 4, 11, 1, 4, 24, 28, 5, 17, 19, 29, 22,
            23, 21, 6, 13, 29, 6, 5, 18, 25, 17, 30, 2, 21, 22, 1, 27, 27, 30, 9, 2, 21, 26,
            23, 16, 11, 16, 30, 24
        }, {
            14, 7, 12, 27, 1, 28, 17, 12, 5, 22, 24, 18, 12, 23, 3, 3, 1, 21, 27, 20, 18, 2, 18,
            14, 7, 17, 21, 20, 16, 28, 8, 20, 27, 3, 8, 27, 9, 29, 13, 12, 27, 23, 28, 7, 16, 15,
            26, 29, 30, 8
        }, {
            15, 8, 15, 12, 12, 30, 7, 21, 2, 22, 29, 5, 21, 1, 1, 12, 29, 1, 13, 12, 22, 28, 11,
            1, 21, 12, 11, 24, 10, 17, 6, 9, 10, 8, 6, 8, 15, 4, 25, 3, 17, 16, 7, 18, 18, 18, 2,
            11, 9, 12
        }
    };
    int con[ARCHS][TESTS] = {
        {
            19, 30, 16, 11, 5, 14, 6, 22, 28, 30, 19, 4, 11, 1, 4, 24, 28, 5, 17, 19, 29, 22,
            23, 21, 6, 13, 29, 6, 5, 18, 25, 17, 30, 2, 21, 22, 1, 27, 27, 30, 9, 2, 21, 26,
            23, 16, 11, 16, 30, 24
        }, {
            14, 7, 12, 27, 1, 28, 17, 12, 5, 22, 24, 18, 12, 23, 3, 3, 1, 21, 27, 20, 18, 2, 18,
            14, 7, 17, 21, 20, 16, 28, 8, 20, 27, 3, 8, 27, 9, 29, 13, 12, 27, 23, 28, 7, 16, 15,
            26, 29, 30, 8
        }, {
            15, 8, 15, 12, 12, 30, 7, 21, 2, 22, 29, 5, 21, 1, 1, 12, 29, 1, 13, 12, 22, 28, 11,
            1, 21, 12, 11, 24, 10, 17, 6, 9, 10, 8, 6, 8, 15, 4, 25, 3, 17, 16, 7, 18, 18, 18, 2,
            11, 9, 12
        }
    };
    int ob_level[ARCHS][TESTS] = {
        {
            12, 103, 103, 6, 109, 70, 30, 69, 18, 49, 112, 46, 45, 72, 21, 100, 110, 102,
            37, 31, 74, 110, 64, 46, 96, 64, 48, 58, 79, 20, 38, 14, 22, 103, 59, 90, 47,
            79, 66, 46, 43, 35, 87, 15, 37, 71, 104, 25, 108, 16
        }, {
            90, 4, 113, 108, 89, 47, 83, 88, 4, 54, 31, 73, 71, 25, 63, 26, 95, 44, 107,
            102, 44, 72, 52, 53, 86, 23, 94, 11, 108, 59, 41, 89, 5, 22, 84, 18, 22,
            105, 6, 82, 84, 56, 49, 42, 83, 43, 77, 68, 4, 61
        }, {
            112, 78, 92, 40, 36, 42, 72, 15, 99, 36, 19, 35, 56, 22, 111, 103, 4,
            97, 107, 66, 35, 102, 17, 87, 21, 15, 115, 84, 59, 20, 77,
            110, 2, 35, 31, 27, 6, 31, 87, 103, 32, 103, 29, 52, 86, 102, 91, 31, 3, 114
        }
    };
    int grace_level[ARCHS][TESTS] = {
        {
            112, 40, 61, 85, 70, 5, 15, 29, 83, 7, 92, 113, 105, 114, 8, 23, 90, 62, 42, 68, 93, 89, 94,
            13, 21, 103, 50, 100, 111, 34, 58, 115, 70, 35, 89, 19, 11, 71, 92, 40, 98, 13, 37, 2, 93,
            35, 33, 10, 48, 43
        }, {
            21, 49, 72, 104, 93, 83, 52, 39, 8, 104, 83, 66, 23, 46, 35, 100, 38, 108,
            35, 53, 27, 108, 93, 54, 95, 111, 101, 34, 19, 27, 53, 8, 86, 75, 56, 64, 13, 48,
            59, 34, 93, 4, 16, 62, 51, 6, 17, 47, 92, 16
        }, {
            11, 86, 106, 45, 72, 93, 35, 109, 70, 109, 72, 113, 23, 103, 18, 47, 54,
            94, 34, 115, 31, 11, 10, 104, 55, 34, 36, 41, 78, 14, 7, 20, 76, 93, 11, 64, 50,
            12, 34, 71, 27, 106, 44, 45, 89, 85, 96, 30, 14, 66
        }
    };

    int mana_level[ARCHS][TESTS] = {
        {
            22, 107, 22, 43, 68, 107, 97, 79, 34, 101, 99, 25, 94, 40, 60, 75, 58, 99, 95, 79, 28, 46, 9,
            37, 71, 40, 107, 111, 99, 108, 108, 113, 19, 79, 51, 4, 70, 36, 26, 85, 68, 109, 97, 79, 82,
            73, 68, 79, 17, 41
        }, {
            68, 99, 32, 14, 63, 33, 25, 29, 13, 101, 61, 61, 25, 48, 69, 72, 23, 78, 84,
            99, 37, 56, 85, 30, 106, 18, 24, 85, 84, 57, 103, 101, 17, 2, 102, 70, 9, 54, 14, 84, 55, 90,
            65, 46, 46, 82, 101, 45, 63, 100
        }, {
            27, 22, 67, 110, 57, 53, 33, 78, 31, 113, 71, 96, 89, 108, 76, 63, 3, 34,
            26, 24, 99, 73, 89, 37, 57, 14, 48, 68, 53, 97, 32, 99, 82, 53, 76, 75, 17, 102, 14, 15, 66,
            72, 94, 28, 23, 67, 43, 1, 21, 96
        }
    };


    /** if you want to generate the test results, change 0 to 1 and run this test. */
#define GENERATE    0

#if GENERATE
    StringBuffer *swc = stringbuffer_new();
    StringBuffer *smaxgr = stringbuffer_new();
    StringBuffer *smaxsp = stringbuffer_new();
    StringBuffer *smaxhp = stringbuffer_new();
    StringBuffer *sac = stringbuffer_new();
    const char *sep = "";

    stringbuffer_append_printf(swc, "\tint wc[%d][%d] = {\n\t", ARCHS, TESTS);
    stringbuffer_append_printf(smaxgr, "\tint maxgr[%d][%d] = {\n\t", ARCHS, TESTS);
    stringbuffer_append_printf(smaxsp, "\tint maxsp[%d][%d] = {\n\t", ARCHS, TESTS);
    stringbuffer_append_printf(smaxhp, "\tint maxhp[%d][%d] = {\n\t", ARCHS, TESTS);
    stringbuffer_append_printf(sac, "\tint ac[%d][%d] = {\n\t", ARCHS, TESTS);
#endif

    pl = calloc(1, sizeof(player));

    for (i = 0; i < ARCHS; i++)
    {
#if GENERATE
        stringbuffer_append_printf(swc, "%s{ ", sep);
        stringbuffer_append_printf(smaxgr, "%s{ ", sep);
        stringbuffer_append_printf(smaxsp, "%s{ ", sep);
        stringbuffer_append_printf(smaxhp, "%s{ ", sep);
        stringbuffer_append_printf(sac, "%s{ ", sep);
        sep = "";
#endif

        ob = create_archetype(archs[i]);
        fail_unless(ob != NULL, "invalid archetype %s", archs[i]);
        if (ob->type == PLAYER)
            ob->contr = pl;

        grace = create_archetype("skill_praying");
        object_insert_in_ob(grace, ob);
        mana = create_archetype("skill_pyromancy");
        object_insert_in_ob(mana, ob);

        for (test = 0; test < TESTS; test++)
        {

            ob->contr->orig_stats.Wis = wis[i][test];
            ob->contr->orig_stats.Int = intelligence[i][test];
            ob->contr->orig_stats.Pow = pow[i][test];
            ob->contr->orig_stats.Con = con[i][test];

            ob->level = ob_level[i][test];
            grace->level = grace_level[i][test];
            mana->level = mana_level[i][test];

            fix_object(ob);

#if GENERATE
            stringbuffer_append_printf(swc, "%s%d", sep, ob->stats.wc);
            stringbuffer_append_printf(smaxgr, "%s%d", sep, ob->stats.maxgrace);
            stringbuffer_append_printf(smaxsp, "%s%d", sep, ob->stats.maxsp);
            stringbuffer_append_printf(smaxhp, "%s%d", sep, ob->stats.maxhp);
            stringbuffer_append_printf(sac, "%s%d", sep, ob->stats.ac);
            sep = ", ";
#else
            fail_unless(ob->stats.wc == wc[i][test], "wc [test %d, arch %d]: got %d instead of %d", test, i, ob->stats.wc, wc[i][test]);
            fail_unless(ob->stats.maxgrace == maxgr[i][test], "gr: got %d instead of %d", ob->stats.maxgrace, maxgr[i][test]);
            fail_unless(ob->stats.maxsp == maxsp[i][test], "sp: got %d instead of %d", ob->stats.maxsp, maxsp[i][test]);
            fail_unless(ob->stats.maxhp == maxhp[i][test], "hp: [test %d, arch %d] [con %d level %d] got %d instead of %d", test, i, con[i][test], ob->level, ob->stats.maxhp, maxhp[i][test]);
            fail_unless(ob->stats.ac == ac[i][test], "ac: got %d instead of %d", ob->stats.ac, ac[i][test]);
#endif
        }
        object_free2(ob, FREE_OBJ_FREE_INVENTORY);

#if GENERATE
        stringbuffer_append_string(swc, "}\n\t");
        stringbuffer_append_string(smaxgr, "}\n\t");
        stringbuffer_append_string(smaxsp, "}\n\t");
        stringbuffer_append_string(smaxhp, "}\n\t");
        stringbuffer_append_string(sac, "}\n\t");
#endif
    }

    free(pl);

#if GENERATE
    stringbuffer_append_string(swc, "\n}; \n");
    stringbuffer_append_string(smaxgr, "\n}; \n");
    stringbuffer_append_string(smaxsp, "\n}; \n");
    stringbuffer_append_string(smaxhp, "\n}; \n");
    stringbuffer_append_string(sac, "\n}; \n");
    {
        char *pwc = stringbuffer_finish(swc);
        char *pmaxgr = stringbuffer_finish(smaxgr);
        char *pmaxsp = stringbuffer_finish(smaxsp);
        char *pmaxhp = stringbuffer_finish(smaxhp);
        char *pac = stringbuffer_finish(sac);
        printf("%s\n", pwc);
        printf("%s\n", pmaxgr);
        printf("%s\n", pmaxsp);
        printf("%s\n", pmaxhp);
        printf("%s\n", pac);
        free(pwc);
        free(pmaxgr);
        free(pmaxsp);
        free(pmaxhp);
        free(pac);
    }
    fail("Here are the results.");
#endif
}
END_TEST

static Suite *living_suite(void) {
    Suite *s = suite_create("living");
    TCase *tc_core = tcase_create("Core");

    /*setup and teardown will be called before each test in testcase 'tc_core' */
    tcase_add_checked_fixture(tc_core, setup, teardown);

    suite_add_tcase(s, tc_core);
    tcase_add_test(tc_core, test_fix_object);

    return s;
}

int main(void) {
    int nf;
    Suite *s = living_suite();
    SRunner *sr = srunner_create(s);

    /* to debug, uncomment this line */
    /*srunner_set_fork_status(sr, CK_NOFORK);*/

    srunner_set_xml(sr, LOGDIR "/unit/common/living.xml");
    srunner_set_log(sr, LOGDIR "/unit/common/living.out");
    srunner_run_all(sr, CK_ENV); /*verbosity from env variable*/
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
