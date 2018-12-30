/*
 * static char *rcsid_check_account_c =
 *   "$Id: check_account.c 11817 2009-06-12 15:46:56Z akirschbaum $";
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
#include <unistd.h>
#include <toolkit_server.h>

static void setup(void) {
}

static void teardown(void) {
    /* put any cleanup steps here, they will be run after each testcase */
}


/* This test makes sure account_check_string() behaves as expected -
 * rejects strings with bad characters in them, etc.
 */
START_TEST(test_account_check_string) {
    char longname[MAX_NAME+3];
    int i;

    i=account_check_string(" abcd");
    fail_unless(i != 0, "string started with spaces should not be considered valid");
    /* We test one string with colon at end, other starting with semicolon -
     * in that way, we also do some basic testing to make sure entire string is being checked.
     */
    i=account_check_string("abcd:");
    fail_unless(i != 0, "string with colons should not be considered valid");
    i=account_check_string("a;bcd");
    fail_unless(i != 0, "string with semicolons should not be considered valid");
    i=account_check_string("a/bcd");
    fail_unless(i != 0, "string with slash should not be considered valid");
    i=account_check_string("a'bcd");
    fail_unless(i != 0, "string with quote should not be considered valid");

    i=account_check_string("abc\n\nefg");
    fail_unless(i != 0, "string with non printable characters should not be considered valid");

    i=account_check_string("$abc");
    fail_unless(i != 0, "string starting with non alphanumeric should not be considered valid");

    i=account_check_string("abc ");
    fail_unless(i != 0, "string ending in space should not be considered valid");

    for (i=0; i<=MAX_NAME; i++) {
        longname[i]='a';
    }
    longname[i]='\0';
    i=account_check_string(longname);

    fail_unless(i != 0, "Too long string should not be considered valid");

    i=account_check_string("Some Body");
    fail_unless(i == 0, "Valid string not considered valid");

}
END_TEST

/* This test tries to add some accounts.  As a final action, we try
 * to save the account file out.
 */
START_TEST(test_account_add_account) {
    int i,j;
    char names[50];
    char **char_names;

    /* Note that we run explicit checks for valid account names/passwords.
     * As such, the focus here is not checking valid strings again, but checking
     * duplicate account names, saving them out, etc.
     */
    i=account_add_account("Some Body", "mypassword");
    fail_unless(i == 0, "Could not add valid account, got code %d", i);

    i=account_add_account("Some Body", "mypassword");
    fail_unless(i != 0, "Duplicate account successfully added");

    /* This is mainly here to have 2 valid accounts */
    i=account_add_account("No Body", "mypassword");
    fail_unless(i == 0, "Could not add valid account, got code %d", i);

    /* This third account is to have one with no players associated to it */
    i=account_add_account("Every Body", "hispassword");
    fail_unless(i == 0, "Could not add valid account");

    i=account_add_player_to_account("foobar", "foobar");
    fail_unless(i != 0, "Added player to non existent character name");

    i=account_add_player_to_account("Some Body", "foobar");
    fail_unless(i == 0, "Failed to add player to valid account");

    /* The precise number of players per account is not exposed,
     * but at least as of now, it should be less than 30.  So we try to add
     * 30 players.  We use |= so that we can just try to add them all.
     */
    for (j=0; j<30; j++) {
        sprintf(names,"char-%02d", j);
        i |= account_add_player_to_account("No Body", names);
    }
    fail_unless(i != 0, "Too many players added to account");

    char_names = account_get_players_for_account("foobar");
    fail_unless(char_names == NULL, "Got non null value for players on account with non existant accoun");

    char_names = account_get_players_for_account("No Body");
    fail_unless(char_names != NULL, "Got null value for players on account");

    /* No return value here */
    accounts_save();
}
END_TEST

/* This tests the load logic.  Since the only data were are loading is the data from above,
 * we use that knowledge to check for proper existence accounts, etc.
 */
START_TEST(test_account_load_entries) {
    int i,j;
    char **char_names;
    const char *ae;

    clear_accounts();
    account_load_entries();

    ae = account_exists("Every Body");
    fail_unless(ae != NULL, "Could not find valid account");
    ae = account_exists("Some Body");
    fail_unless(ae != NULL, "Could not find valid account");

    char_names =  account_get_players_for_account("No Body");
    fail_unless(char_names != NULL, "Got null value for players on account");

    /* This is sort of data integrity testing - if char_names is not properly
     * null terminated, with will fail.  if char_names[] is corrupt, it should also
     * core.
     */
    i=0;
    while (char_names[i] != NULL) {
        j=strlen(char_names[i]);
        i++;
    }
    char_names =  account_get_players_for_account("Some Body");
    fail_unless(char_names != NULL, "Got null value for players on account");
    fail_unless(strcmp(char_names[0],"foobar") == 0, "Can not match on name we put in");

    /* while we do this same check in test_account_add_account, doing it
     * again here makes sure the code is properly detecting the end of the accounts
     * list after loading it up.
     */
    char_names = account_get_players_for_account("foobar");
    fail_unless(char_names == NULL, "Got non null value for players on account with non existant account");

    i = account_remove_player_from_account("foobar", "1foobar");
    fail_unless(i!=0, "Got successsful return code on account_remove_player_from_account with non existent account");

    i = account_remove_player_from_account("No Body", "1foobar");
    fail_unless(i!=0, "Got successsful return code on account_remove_player_from_account with non existent player");

    char_names =  account_get_players_for_account("No Body");
    j=0;
    while (char_names[j] != NULL) {
        j++;
    }

    i = account_remove_player_from_account("No Body", "char-01");
    fail_unless(i==0, "Got error removing player when it should have worked");

    char_names =  account_get_players_for_account("No Body");

    i=0;
    while (char_names[i] != NULL) {
        i++;
    }
    fail_unless((i+1) == j, "Player removal get unexpected result - %d != %d", i+1, j);
}
END_TEST

static Suite *account_suite(void) {
    Suite *s = suite_create("account");
    TCase *tc_core = tcase_create("Core");

    /*setup and teardown will be called before each test in testcase 'tc_core' */
    tcase_add_checked_fixture(tc_core, setup, teardown);

    suite_add_tcase(s, tc_core);
    tcase_add_test(tc_core, test_account_check_string);
    tcase_add_test(tc_core, test_account_add_account);
    tcase_add_test(tc_core, test_account_load_entries);

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

    srunner_set_xml(sr, LOGDIR "/unit/server/account.xml");
    srunner_set_log(sr, LOGDIR "/unit/server/account.out");
    srunner_run_all(sr, CK_ENV); /*verbosity from env variable*/
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    clean_test_account_data();
    return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
