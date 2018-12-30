/*
 * static char *rcsid_check_arch_c =
 *   "$Id: check_arch.c 15379 2011-11-01 19:40:45Z ryo_saeba $";
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
 * This is the unit tests file for common/arch.c
 */

#include <global.h>
#include <stdlib.h>
#include <check.h>
#include <loader.h>
#include <toolkit_common.h>

static void setup(void) {
    cctk_setdatadir(BUILD_ROOT"lib");
    cctk_setlog(LOGDIR"/unit/common/arch.out");
    cctk_init_std_archetypes();
}

static void teardown(void) {
    /* put any cleanup steps here, they will be run after each testcase */
}

START_TEST(test_find_archetype_by_object_name) {
    archetype *arch;

    arch = find_archetype_by_object_name("large city");
    fail_unless(arch != NULL, "Searching for an existing arch name (large city) should work");
    fail_unless(!strcmp(arch->clone.name, "large city"), "Searching for an existing arch name shoud have returned us large city but returned %s", arch->clone.name);
    arch = find_archetype_by_object_name("Cloak of Magic Resistance");
    fail_unless(arch != NULL, "Searching for an existing arch name (Cloak of Magic Resistance) should work");
    fail_unless(!strcmp(arch->clone.name, "Cloak of Magic Resistance"), "Searching for an existing arch name shoud have returned us Cloak of Magic Resistance but returned %s", arch->clone.name);
    arch = find_archetype_by_object_name("Cloak of Magic Resistanc");
    fail_unless(arch == NULL, "Searching for an inexistant arch name (Cloak of Magic Resistanc) should return NULL");
    arch = find_archetype_by_object_name("some really non existant archetype");
    fail_unless(arch == NULL, "Searching for an inexistant arch name (some really non existant archetype) should return NULL");
    arch = find_archetype_by_object_name("");
    fail_unless(arch == NULL, "Searching for empty arch name should return NULL");
    arch = find_archetype_by_object_name(NULL);
    fail_unless(arch == NULL, "Searching for NULL arch name should return NULL");
}
END_TEST

START_TEST(test_find_archetype_by_object_type_name) {
    archetype *arch;

    arch = find_archetype_by_object_type_name(66, "large city");
    fail_unless(arch != NULL, "Searching for an existing arch name (large city) + type (66) should work");
    fail_unless(arch->clone.type == 66, "Requested type 66 but got %d", arch->clone.type);
    fail_unless(!strcmp(arch->clone.name, "large city"), "Searching for an existing arch name shoud have returned us large city but returned %s", arch->clone.name);
    arch = find_archetype_by_object_type_name(87, "Cloak of Magic Resistance");
    fail_unless(arch != NULL, "Searching for an existing arch name (Cloak of Magic Resistance) + type (87) should work");
    fail_unless(arch->clone.type == 87, "Requested type 87 but got %d", arch->clone.type);
    fail_unless(!strcmp(arch->clone.name, "Cloak of Magic Resistance"), "Searching for an existing arch name shoud have returned us Cloak of Magic Resistance but returned %s", arch->clone.name);
    arch = find_archetype_by_object_type_name(87, "Cloak of Magic Resistanc");
    fail_unless(arch == NULL, "Searching for an inexistant arch name (Cloak of Magic Resistanc) should return NULL");
    arch = find_archetype_by_object_type_name(88, "Cloak of Magic Resistance");
    fail_unless(arch == NULL, "Searching for an existing arch name (Cloak of Magic Resistance) but with wrong type (88) should return NULL");
}
END_TEST


START_TEST(test_get_archetype_by_skill_name) {
    archetype *arch;

    arch = get_archetype_by_skill_name("alchemy", SKILL);
    fail_unless(arch != NULL, "Should be able to discover the alchemy skill");
    fail_unless(!strcmp(arch->name, "skill_alchemy"), "should have returned skill_alchemy but returned %s", arch->name);
    arch = get_archetype_by_skill_name("alchemy", SKILLSCROLL);
    fail_unless(arch != NULL, "Should be able to discover the scroll of alchemy skill or something similar");
    arch = get_archetype_by_skill_name("one handed weapons", -1);
    fail_unless(arch != NULL, "Should be able to discover something related to the 'one handed weapons' skill");
    arch = get_archetype_by_skill_name(NULL, -1);
    fail_unless(arch == NULL, "Asking for null skill should return null");
}
END_TEST

START_TEST(test_get_archetype_by_type_subtype) {
    archetype *arch;

    arch = get_archetype_by_type_subtype(SKILL, SK_LITERACY);
    fail_unless(arch != NULL, "Should be able to find an arch of type SKILL, subtype SK_LITERACY");
    fail_unless(arch->clone.type == SKILL, "Arch of type SKILL, subtype SK_LITERACY shoud have type %d but has type %d", SKILL, arch->clone.type);
    fail_unless(arch->clone.subtype == SK_LITERACY, "Arch of type SKILL, subtype SK_LITERACY shoud have subtype %d but has subtype %d", SK_LITERACY, arch->clone.subtype);
    fail_unless(arch != NULL, "Should be able to find an arch of type quest, subtype SK_LITERACY");
    arch = get_archetype_by_type_subtype(SKILL, -1);
    fail_unless(arch != NULL, "Should be able to find an arch of type SKILL, no subtype");
    fail_unless(arch->clone.type == SKILL, "arch of type SKILL, no subtype should have type %d but has %d", SKILL, arch->clone.type);
    arch = get_archetype_by_type_subtype(-1, SK_LITERACY);
    fail_unless(arch != NULL, "Should be able to find an arch of type unknown, SK_LITERACY");
    fail_unless(arch->clone.subtype == SK_LITERACY, "arch of type unknown, subtype quest in progress shoud have subtype %d but has subtype %d", SK_LITERACY, arch->clone.subtype);
    arch = get_archetype_by_type_subtype(-1, -1);
    fail_unless(arch != NULL, "Should be able to find arch of type unknown, subtype unknown, despite this being useless");
    arch = get_archetype_by_type_subtype(OBJECT_TYPE_MAX+1, -1);
    if (arch != NULL)
        fail("Should be not able to find arch of inexistant type but got %p (%s)", arch, arch->name);
}
END_TEST


START_TEST(test_create_archetype_by_object_name) {
    object *ob;

    ob = create_archetype_by_object_name("writing pen");
    fail_unless(ob != NULL, "Should never return null");
    fail_unless(strncmp(ob->name, ARCH_SINGULARITY, strlen(ARCH_SINGULARITY)), "Searching for writing pen should NOT have returned a singularity");
    fail_unless(!strncmp(ob->name, "writing pen", strlen(ob->name)), "Searching for writing pen should have returned something with same base name but returned '%s'", ob->name);
    ob = create_archetype_by_object_name("writing pen of hell raiser +3");
    fail_unless(ob != NULL, "Should never return null");
    fail_unless(strncmp(ob->name, ARCH_SINGULARITY, strlen(ARCH_SINGULARITY)), "Searching for writing pen of hell raiser +3 should NOT have returned a singularity");
    fail_unless(!strncmp(ob->name, "writing pen of hell raiser +3", strlen(ob->name)), "Searching for writing pen of hell raiser +3 should have returned something with same base name but returned %s", ob->name);
    ob = create_archetype_by_object_name("%*");
    fail_unless(ob != NULL, "Inexistent item shuold return a singularity");
    fail_unless(!strncmp(ob->name, ARCH_SINGULARITY, strlen(ARCH_SINGULARITY)), "Searching for %* should have returned a singularity");
    ob = create_archetype_by_object_name("");
    fail_unless(ob != NULL, "Inexistent item shuold return a singularity");
    fail_unless(!strncmp(ob->name, ARCH_SINGULARITY, strlen(ARCH_SINGULARITY)), "Searching for \"\" should have returned a singularity");
}
END_TEST

START_TEST(test_init_archetypes) {
    /*init_archetypes is used by setup, just check it created the empty_archetype*/
    archetype *arch = find_archetype("empty_archetype");

    fail_unless(arch != NULL, "init_archetype should have an 'empty_archetype' loaded");
}
END_TEST

START_TEST(test_clear_archetable) {
    clear_archetable(); /*should just not fail :p*/
}
END_TEST

START_TEST(test_free_all_archs) {
    archetype *arch;

    free_all_archs();
    arch = find_archetype("empty_archetype");
    fail_unless(arch == NULL, "init_archetype should not have an 'empty_archetype' loaded after call to free_all_archs");
    init_archetypes();
    arch = find_archetype("empty_archetype");
    fail_unless(arch != NULL, "init_archetype should have an 'empty_archetype' loaded");
}
END_TEST

START_TEST(test_get_archetype_struct) {
    archetype *arch = get_archetype_struct();

    fail_unless(arch != NULL, "get_archetype_struct should not return NULL");
    fail_unless(arch->name == NULL, "arch->name of get_archetype_struct should be inited to NULL");
    fail_unless(arch->head == NULL, "arch->head of get_archetype_struct should be inited to NULL");
    fail_unless(arch->next == NULL, "arch->next of get_archetype_struct should be inited to NULL");
    fail_unless(arch->more == NULL, "arch->more of get_archetype_struct should be inited to NULL");
    fail_unless(arch->clone.other_arch == NULL, "arch->clone.other_arch of get_archetype_struct should be inited to NULL");
    fail_unless(arch->clone.contr == NULL, "arch->clone.contr of get_archetype_struct should be inited to NULL");
    fail_unless(arch->clone.next == NULL, "arch->clone.next of get_archetype_struct should be inited to NULL");
    fail_unless(arch->clone.prev == NULL, "arch->clone.prev of get_archetype_struct should be inited to NULL");
    fail_unless(arch->clone.active_next == NULL, "arch->clone.active_next of get_archetype_struct should be inited to NULL");
    fail_unless(arch->clone.active_prev == NULL, "arch->clone.active_prev of get_archetype_struct should be inited to NULL");
    fail_unless(arch->clone.below == NULL, "arch->clone.below of get_archetype_struct should be inited to NULL");
    fail_unless(arch->clone.above == NULL, "arch->clone.above of get_archetype_struct should be inited to NULL");
    fail_unless(arch->clone.inv == NULL, "arch->clone.inv of get_archetype_struct should be inited to NULL");
    fail_unless(arch->clone.container == NULL, "arch->clone.container of get_archetype_struct should be inited to NULL");
    fail_unless(arch->clone.env == NULL, "arch->clone.env of get_archetype_struct should be inited to NULL");
    fail_unless(arch->clone.more == NULL, "arch->clone.more of get_archetype_struct should be inited to NULL");
    fail_unless(arch->clone.head == NULL, "arch->clone.head of get_archetype_struct should be inited to NULL");
    fail_unless(arch->clone.map == NULL, "arch->clone.map of get_archetype_struct should be inited to NULL");

    fail_unless(arch->clone.name == NULL, "arch->clone.name of get_archetype_struct should be inited to NULL");
    fail_unless(arch->clone.name_pl == NULL, "arch->clone.name_pl of get_archetype_struct should be inited to NULL");
    fail_unless(arch->clone.title == NULL, "arch->clone.title of get_archetype_struct should be inited to NULL");
    fail_unless(arch->clone.race == NULL, "arch->clone.race of get_archetype_struct should be inited to NULL");
    fail_unless(arch->clone.slaying == NULL, "arch->clone.slaying of get_archetype_struct should be inited to NULL");
    fail_unless(arch->clone.msg == NULL, "arch->clone.msg of get_archetype_struct should be inited to NULL");
    fail_unless(arch->clone.skill == NULL, "arch->clone.skill of get_archetype_struct should be inited to NULL");
    fail_unless(arch->clone.lore == NULL, "arch->clone.lore of get_archetype_struct should be inited to NULL");

    fail_unless(arch->clone.current_weapon == NULL, "arch->clone.current_weapon of get_archetype_struct should be inited to NULL");
    fail_unless(arch->clone.enemy == NULL, "arch->clone.enemy of get_archetype_struct should be inited to NULL");
    fail_unless(arch->clone.attacked_by == NULL, "arch->clone.attacked_by of get_archetype_struct should be inited to NULL");
    fail_unless(arch->clone.randomitems == NULL, "arch->clone.randomitems of get_archetype_struct should be inited to NULL");
    fail_unless(arch->clone.chosen_skill == NULL, "arch->clone.chosen_skill of get_archetype_struct should be inited to NULL");
    fail_unless(arch->clone.spellitem == NULL, "arch->clone.spellitem of get_archetype_struct should be inited to NULL");
    fail_unless(arch->clone.spell == NULL, "arch->clone.spell of get_archetype_struct should be inited to NULL");
    fail_unless(arch->clone.spellarg == NULL, "arch->clone.spellarg of get_archetype_struct should be inited to NULL");
    fail_unless(arch->clone.arch == arch, "arch->clone.arch of get_archetype_struct should be inited to arch");
    fail_unless(arch->clone.other_arch == NULL, "arch->clone.other_arch of get_archetype_struct should be inited to NULL");
    fail_unless(arch->clone.custom_name == NULL, "arch->clone.custom_name of get_archetype_struct should be inited to NULL");
    fail_unless(arch->clone.key_values == NULL, "arch->clone.key_values of get_archetype_struct should be inited to NULL");
}
END_TEST

START_TEST(test_arch_to_object) {
    archetype *arch;
    object *obj;

    arch = find_archetype("empty_archetype");
    obj = arch_to_object(arch);
    fail_unless(obj != NULL, "instanciating an arch should not return null");
}
END_TEST

START_TEST(test_create_singularity) {
    object *obj;

    obj = create_singularity("XYZABCD");
    fail_unless(obj != NULL, "create_singularity should not return null");
    fail_unless(strstr(obj->name, "XYZABCD") != NULL, "create_singularity(\"XYZABCD\") should put XYZABCD somewhere in singularity name");
}
END_TEST

START_TEST(test_create_archetype) {
    object *obj;

    obj = create_archetype("empty_archetype");
    fail_unless(obj != NULL, "create_archetype(\"empty_archetype\") should not return null");
}
END_TEST

START_TEST(test_find_archetype) {
    archetype *arch;

    arch = find_archetype("empty_archetype");
    fail_unless(arch != NULL, "find_archetype(\"empty_archetype\") should not return null");
    arch = find_archetype("elvenboots");
    fail_unless(arch != NULL, "find_archetype(\"elvenboots\") should not return null");
    arch = find_archetype("AA1234567890");
    fail_unless(arch == NULL, "find_archetype(\"AA1234567890\") should return null");
}
END_TEST

START_TEST(test_object_create_arch) {
    archetype *arch;
    object *obj;

    arch = find_archetype("dark_palace_4");
    obj = object_create_arch(arch);
    fail_unless(obj != NULL, "Should be able to fully instanciate the dark_palace");
    fail_unless(obj->head == NULL, "The object is full, so we should have got it's head. So head should be null but was %p for object %p", obj->head, obj);
    fail_unless(obj->more != NULL, "The object is full and multisquare, so more should not return null");
}
END_TEST

static Suite *arch_suite(void) {
    Suite *s = suite_create("arch");
    TCase *tc_core = tcase_create("Core");

    /*setup and teardown will be called before each test in testcase 'tc_core' */
    tcase_add_checked_fixture(tc_core, setup, teardown);

    suite_add_tcase(s, tc_core);
    tcase_add_test(tc_core, test_find_archetype_by_object_name);
    tcase_add_test(tc_core, test_find_archetype_by_object_type_name);
    tcase_add_test(tc_core, test_get_archetype_by_skill_name);
    tcase_add_test(tc_core, test_get_archetype_by_type_subtype);
    tcase_add_test(tc_core, test_create_archetype_by_object_name);
    tcase_add_test(tc_core, test_init_archetypes);
    tcase_add_test(tc_core, test_clear_archetable);
    tcase_add_test(tc_core, test_free_all_archs);
    tcase_add_test(tc_core, test_get_archetype_struct);
    tcase_add_test(tc_core, test_arch_to_object);
    tcase_add_test(tc_core, test_create_singularity);
    tcase_add_test(tc_core, test_create_archetype);
    tcase_add_test(tc_core, test_find_archetype);
    tcase_add_test(tc_core, test_object_create_arch);

    return s;
}

int main(void) {
    int nf;
    Suite *s = arch_suite();
    SRunner *sr = srunner_create(s);

    srunner_set_xml(sr, LOGDIR "/unit/common/arch.xml");
    srunner_set_log(sr, LOGDIR "/unit/common/arch.out");
    srunner_run_all(sr, CK_ENV); /*verbosity from env variable*/
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
