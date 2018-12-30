/*
 * static char *rcsid_check_account_c =
 *   "$Id: check_account_char.c 18934 2013-08-15 10:24:00Z ryo_saeba $";
 */

/*
 * CrossFire, A Multiplayer game for X-windows
 *
 * Copyright (C) 2010 Mark Wedel & Crossfire Development Team
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
 * This is the unit tests file for server/account.c
 */

#include <global.h>
#include <stdlib.h>
#include <check.h>
#include <loader.h>
#include <toolkit_common.h>
#include <sproto.h>
#include <account_char.h>

static void setup(void) {
}

static void teardown(void) {
    /* put any cleanup steps here, they will be run after each testcase */
}



/* This test tries to add some accounts.  As a final action, we try
 * to save the account file out.
 */
START_TEST(test_account_char_add) {
    player *pl;
    Account_Char *chars;
    char path[MAX_BUF];

    pl =  calloc(1, sizeof(player));

    /* The account_character code takes a player structure to
     * fill in the values, so we create a fake one here -
     * we just fill in the fields that are used.
     */
    pl->ob = create_archetype("human_player");
    pl->ob->level = 1;
    pl->ob->name = add_string("test character");
    pl->ob->contr = pl;
    strcpy(pl->maplevel, "test map");

    chars = account_char_add(NULL, pl);
    fail_unless(chars != NULL, "account_char_add returned NULL on initial character");

    pl->ob->level = 2;
    chars = account_char_add(chars, pl);
    fail_unless(chars != NULL, "account_char_add returned NULL on update character");
    fail_unless(chars->next == NULL, "account_char_add added to list, not updated existing entry");

    chars = account_char_remove(chars, pl->ob->name);
    fail_unless(chars == NULL, "account_char_remove returned non NULL on final character removal");

    chars = account_char_add(NULL, pl);
    fail_unless(chars != NULL, "account_char_add returned NULL on initial character");

    pl->ob->name = add_string("char 2");
    pl->party = party_form(pl->ob, "rockon");

    chars = account_char_add(chars, pl);
    fail_unless(chars != NULL, "account_char_add returned NULL on initial character");
    fail_unless(chars->next != NULL, "account_char_add did not set next pointer!");

    sprintf(path,"%s/account", settings.localdir);
    mkdir(path, S_IRWXU);

    /* This does not return anything, but this at least checks for
     * core dumps, etc
     */
    account_char_save("testaccount", chars);

    /* Like above, this returns nothing but does check for core dumps */
    account_char_free(chars);
}
END_TEST

/* This tests the load logic.  Since the only data were are loading is the data from above,
 * we use that knowledge to check for proper existence accounts, etc.
 */
START_TEST(test_account_char_load) {
    Account_Char *chars;
    object *ob = create_archetype("human_player");

    chars = account_char_load("testaccount");
    fail_unless(chars != NULL, "account_char_load returned NULL");

    /* As of now, the account order is in FIFO order */

    fail_unless(!strcmp(chars->name, "test character"),
                "Name for first character is not test char");

    fail_unless(!strcmp(chars->race, ob->race),
                "Race for first character does not match");

    fail_unless(chars->level == 2,
                "Level for first character is not 2");

    fail_unless(!strcmp(chars->face, ob->face->name),
                "Face for first character does not match");

    fail_unless(chars->party[0] == 0,
                "Party for first character is not blank");

    fail_unless(!strcmp(chars->map, "test map"),
                "Map for first character does not match");

    fail_unless(chars->next != NULL, "account_char_load only loaded one character");

    /* The presumption here is that if it loaded the first entry
     * successfully, so it should the second, but we do check for the fields
     * which are different.
     */
    chars = chars->next;

    fail_unless(!strcmp(chars->name, "char 2"),
                "Name for second character does not match");

    fail_unless(!strcmp(chars->party, "rockon"),
                "Party for second character does not match");
}

END_TEST

static Suite *account_suite(void) {
    Suite *s = suite_create("account_char");
    TCase *tc_core = tcase_create("Core");

    /*setup and teardown will be called before each test in testcase 'tc_core' */
    tcase_add_checked_fixture(tc_core, setup, teardown);

    suite_add_tcase(s, tc_core);
    tcase_add_test(tc_core, test_account_char_add);
    tcase_add_test(tc_core, test_account_char_load);

    return s;
}

int main(void) {
    int nf;
    Suite *s = account_suite();
    SRunner *sr = srunner_create(s);

    /* If you wish to debug the program, uncomment this line. */
    /*srunner_set_fork_status (sr, CK_NOFORK);*/

    settings.debug = 0;
    /* Not sure if there is a better place to put this file - basically,
     * the account code usings the localdir to determine where to read/write
     * the accounts file from - we don't want to be altering the real version of
     * that file.
     */
    settings.localdir = strdup_local("/tmp/");
    clean_test_account_data();
    init(0, NULL);

    srunner_set_xml(sr, LOGDIR "/unit/server/account_char.xml");
    srunner_set_log(sr, LOGDIR "/unit/server/account_char.out");
    srunner_run_all(sr, CK_ENV); /*verbosity from env variable*/
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);

    clean_test_account_data();
    return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
