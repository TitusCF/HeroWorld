/*
 * CrossFire, A Multiplayer game for X-windows
 *
 * Copyright (C) 2007 Crossfire Development Team
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

/**
 * @file
 * This is the unit tests file for the bug #1727944: The horn of plenty
 * location: http://sourceforge.net/tracker/index.php?func=detail&aid=1727944&group_id=13833&atid=113833.
 *
 * I did try different combos to generate an empty horn, so far no success...
 * @author Nicolas Weeger
 * @date 2007-06-04
 */

#include <stdlib.h>
#include <check.h>
#include <global.h>
#include <sproto.h>

static void setup(void) {
    /* put any initialisation steps here, they will be run before each testcase */
}

static void teardown(void) {
    /* put any cleanup steps here, they will be run after each testcase */
}

#if 0
static mapstruct *get_random_map(mapstruct *map) {
    object *exit_ob;
    mapstruct *random;
    RMParms rp;
    char newmap_name[HUGE_BUF], *cp;
    static int reference_number = 0;
    int x, y;

    exit_ob = NULL;
    for (x = 0; x < MAP_WIDTH(map) && exit_ob == NULL; x++) {
        for (y = 0; y < MAP_HEIGHT(map) && exit_ob == NULL; y++) {
            for (exit_ob = GET_MAP_OB(map, x, y); exit_ob != NULL; exit_ob = exit_ob->above)
                if (exit_ob->type == EXIT && exit_ob->msg != NULL)
                    break;
        }
    }

    if (!exit_ob)
        /* this means we reached the end of the random part. */
        return NULL;

    /* copied from server/server.c:enter_random_map(). */
    memset(&rp, 0, sizeof(RMParms));
    rp.Xsize = -1;
    rp.Ysize = -1;
    rp.region = get_region_by_map(exit_ob->map);
    if (exit_ob->msg)
        set_random_map_variable(&rp, exit_ob->msg);
    rp.origin_x = exit_ob->x;
    rp.origin_y = exit_ob->y;
    strcpy(rp.origin_map, map->path);

    /* If we have a final_map, use it as a base name to give some clue
     * as where the player is.  Otherwise, use the origin map.
     * Take the last component (after the last slash) to give
     * shorter names without bogus slashes.
     */
    if (rp.final_map[0]) {
        cp = strrchr(rp.final_map, '/');
        if (!cp)
            cp = rp.final_map;
    } else {
        char buf[HUGE_BUF];

        cp = strrchr(rp.origin_map, '/');
        if (!cp)
            cp = rp.origin_map;
        /* Need to strip of any trailing digits, if it has them */
        snprintf(buf, sizeof(buf), "%s", cp);
        while (isdigit(buf[strlen(buf)-1]))
            buf[strlen(buf)-1] = 0;
        cp = buf;
    }
    snprintf(newmap_name, sizeof(newmap_name), "/random/%s%04d", cp+1, reference_number++);
    /* now to generate the actual map. */
    return generate_random_map(newmap_name, &rp, NULL);
}

static void do_run(void) {
    mapstruct *worldmap;
    mapstruct *random;
    mapstruct *old;
    int iteration, x, y, map;
    object *check;
    char path[150];

    for (map = 1; map <= 3; map++) {
        snprintf(path, sizeof(path), "/whalingoutpost/underwaterdungeon/level%d", map);
        worldmap = ready_map_name(path, 0);
        fail_unless(worldmap != NULL, "Can't load %s", path);

        random = worldmap;
        old = NULL;
        iteration = 0;
        while (random != NULL) {
            random = get_random_map(random);
            if (!random)
                break;
            if (old)
                delete_map(old);
            old = random;
            iteration++;
            for (x = 0; x < MAP_WIDTH(random); x++) {
                for (y = 0; y < MAP_HEIGHT(random); y++) {
                    for (check = GET_MAP_OB(random, x, y); check; check = check->above) {
                        if (check->type == ROD && check->title && strcmp(check->title, "of Plenty") == 0)
                            fail_unless(check->inv != NULL, "Horn has empty inventory!");
                    }
                }
            }
        }
        fail_unless(iteration != 0, "did %d iterations", iteration);
        if (old)
            delete_map(old);
    }
}
#endif

#if 0
static void do_run(void) {
    mapstruct *map, *overlay;
    int x, y, found = 0, test = 0;
    object *check;

    overlay = ready_map_name("../../rsc/bug_1727944_unique", MAP_PLAYER_UNIQUE);
    fail_unless(overlay != NULL, "Couldn't load unique map ../../rsc/bug_1727944_unique");

    while (found == 0 && test < 10) {
        map = ready_map_name("../../rsc/bug_1727944", MAP_PLAYER_UNIQUE);
        fail_unless(map != NULL, "couldn't load map ../../rsc/bug_1727944");

        for (x = 0; x < MAP_WIDTH(map); x++) {
            for (y = 0; y < MAP_HEIGHT(map); y++) {
                for (check = GET_MAP_OB(map, x, y); check; check = check->above) {
                    if (check->type == ROD) {
                        fail_unless(check->inv != NULL, "Horn has empty inventory!");
                        fail_unless(check->inv->below == NULL, "Horn has 2 items in inventory!");
                        if (check->title && strcmp(check->title, "of Plenty") == 0) {
                            object_remove(check);
                            object_insert_in_map_at(check, overlay, NULL, 0, 2, 3);
                            found++;
                            break;
                        }
                    }
                }
            }
        }
        delete_map(map);
        test++;
    }
    save_map(overlay, SAVE_MODE_OVERLAY);
    delete_map(overlay);
}
#endif

extern int artifact_init;
extern int arch_init;

/* Copied from loader.l */
extern const char *const spell_mapping[];

static void local_check_loaded_object(object *op) {
    int ip;

    if (artifact_init)
        /* Artifacts are special beasts, let's not check them. */
        return;

    /* We do some specialized handling to handle legacy cases of name_pl.
     * If the object doesn't have a name_pl, we just use the object name -
     * this isn't perfect (things won't be properly pluralized), but works to
     * that degree (5 heart is still quite understandable).  But the case we
     * also have to catch is if this object is not using the normal name for
     * the object.  In that case, we also want to use the loaded name.
     * Otherwise, what happens is that the the plural name will lose
     * information (appear as just 'hearts' and not 'goblins heart')
     */
    if (op->arch && op->name != op->arch->clone.name && op->name_pl == op->arch->clone.name_pl) {
        if (op->name_pl)
            free_string(op->name_pl);
        op->name_pl = NULL;
    }
    if (!op->name_pl && op->name)
        op->name_pl = add_string(op->name);

    /* objects now have a materialname.  try to patch it in */
    if (!(IS_WEAPON(op) && op->level > 0)) {
        set_materialname(op);
    }
    /* only do these when program is first run - a bit
     * excessive to do this at every run - most of this is
     * really just to catch any errors - program will still run, but
     * not in the ideal fashion.
     */
    if ((op->type == WEAPON || op->type == BOW) && arch_init) {
        if (!op->skill) {
            LOG(llevError, "Weapon %s lacks a skill.\n", op->name);
        } else if ((!strcmp(op->skill, "one handed weapons") && op->body_info[1] != -1)
        || (!strcmp(op->skill, "two handed weapons") && op->body_info[1] != -2)) {
            LOG(llevError, "weapon %s arm usage does not match skill: %d, %s\n",
            op->name, op->body_info[1], op->skill);
        }
    }

    /* We changed last_heal to gen_sp_armour, which is what it
     * really does for many objects.  Need to catch any in maps
     * that may have an old value.
     */
    if ((op->type == WEAPON)
    || (op->type == ARMOUR)
    || (op->type == HELMET)
    || (op->type == SHIELD)
    || (op->type == RING)
    || (op->type == BOOTS)
    || (op->type == GLOVES)
    || (op->type == AMULET)
    || (op->type == GIRDLE)
    || (op->type == BRACERS)
    || (op->type == CLOAK)) {
        if (op->last_heal) {
            LOG(llevDebug, "Object %s still has last_heal set, not gen_sp_armour\n", op->name ? op->name : "NULL");
            op->gen_sp_armour = op->last_heal;
            op->last_heal = 0;
        }
        ip = calc_item_power(op);
        /* Legacy objects from before item power was in the game */
        if (!op->item_power && ip) {
            if (ip > 3) {
                LOG(llevDebug, "Object %s had no item power, using %d\n", op->name ? op->name : "NULL", ip);
            }
            op->item_power = ip;
        }
        /* Check for possibly bogus values.  Has to meet both these criteria -
        * something that has item_power 1 is probably just fine if our calculated
        * value is 1 or 2 - these values are small enough that hard to be precise.
        * similarly, it item_power is 0, the first check will always pass,
        * but not the second one.
        */
        if (ip > 2*op->item_power && ip > (op->item_power+3)) {
            LOG(llevDebug, "Object %s seems to have too low item power? %d > %d\n", op->name ? op->name : "NULL", ip, op->item_power);
        }
    }
    /* Old spellcasting object - need to load in the appropiate object */
    if ((op->type == ROD || op->type == WAND || op->type == SCROLL || op->type == FIREWALL || /* POTIONS and ALTARS don't always cast spells, but if they do, update them */ ((op->type == POTION || op->type == ALTAR) && op->stats.sp))
    && !op->inv
    && !arch_init)  {
        object *tmp;

    /* Fireall is bizarre in that spell type was stored in dam.  Rest are 'normal'
     * in that spell was stored in sp.
     */
        tmp = create_archetype(spell_mapping[op->type == FIREWALL ? op->stats.dam : op->stats.sp]);
        object_insert_in_ob(tmp, op);
        op->randomitems = NULL; /* So another spell isn't created for this object */
    }

    /* spellbooks & runes use slaying.  But not to arch name, but to spell name */
    if ((op->type == SPELLBOOK || op->type == RUNE) && op->slaying && !op->inv && !arch_init) {
        object *tmp;

        tmp = create_archetype_by_object_name(op->slaying);
        object_insert_in_ob(tmp, op);
        op->randomitems = NULL; /* So another spell isn't created for this object */
        /* without this, value is all screwed up */
        op->value = op->arch->clone.value*op->inv->value;
    }

    if (QUERY_FLAG(op, FLAG_MONSTER)) {
        if (op->stats.hp > op->stats.maxhp)
            LOG(llevDebug, "Monster %s has hp set higher than maxhp (%d>%d)\n", op->name, op->stats.hp, op->stats.maxhp);
        }
    if ((QUERY_FLAG(op, FLAG_GENERATOR) && QUERY_FLAG(op, FLAG_CONTENT_ON_GEN))
        || op->type == CREATOR
        || op->type == CONVERTER) {
        /* Object will duplicate it's content as part of the
         * generation process. To do this, we must flag inventory
         * so it remains unevaluated concerning the randomitems and
         * the living (a demonlord shouldn't cast from inside generator!)
         */
        object_set_flag_inv(op, FLAG_IS_A_TEMPLATE);
    }

    /* Here we'll handle custom monsters. In order to handle them correctly, especially in the fix_object
     * method, we'll create a new temporary archetype containing defined values.
     * Of course this doesn't apply when loading archetypes or artifacts.
     */
    if (arch_init == 0 && artifact_init == 0 && QUERY_FLAG(op, FLAG_MONSTER) && op->arch && !object_can_merge(op, &op->arch->clone)) {
        archetype *temp = get_archetype_struct();

        temp->reference_count++;
        temp->name = add_string(op->arch->name);
        temp->tail_x = op->arch->tail_x;
        temp->tail_y = op->arch->tail_y;
        object_copy(op, &temp->clone);
        temp->clone.inv = NULL;
        temp->clone.env = NULL;
        temp->clone.x = 0;
        temp->clone.y = 0;
        temp->clone.map = NULL;
        if (FABS(temp->clone.speed) > MIN_ACTIVE_SPEED) {
            /* Clone has a speed, so need to clear that because it isn't on a map.
             * But we need to keep the value, because otherwise the customized object
             * will have no speed (fix_player() will use the 0 value).  So set it
             * to zero, call object_update_speed() to remove it from active list, then
             * set its speed back to the original.
             */
            temp->clone.speed = 0;
            object_update_speed(&temp->clone);
            temp->clone.speed = op->speed;
        }

        temp->more = op->arch->more;
        op->arch = temp;
        /* LOG(llevDebug, "created temporary archetype for %s at %d,%d\n", op->name, op->x, op->y); */
    }
}

START_TEST(test_randommaps) {
#if 0
    int test;
    mapstruct *overlay;
    object *check;

    for (test = 0; test < 50; test++)
        do_run();

    for (test = 0; test < 50; test++) {
        overlay = ready_map_name("../../rsc/bug_1727944_unique", MAP_PLAYER_UNIQUE);
        fail_unless(overlay != NULL, "Couldn't load unique map ../../rsc/bug_1727944_unique");
        fail_unless(GET_MAP_OB(overlay, 2, 3) != NULL, "No item on spot 2,3?");

        for (check = GET_MAP_OB(overlay, 2, 3)->above; check != NULL; check = check->above) {
            fail_unless(check->type == ROD, "Found a non horn?");
            fail_unless(check->inv != NULL, "Horn without a spell!");
            fail_unless(check->inv->below == NULL, "Horn with 2 items in inventory.");
        }
        save_map(overlay, SAVE_MODE_OVERLAY);
        delete_map(overlay);
    }
#endif

#if 0
    int test;
    archetype *horn = find_archetype("horn");
    fail_unless(horn != NULL, "couldn't find archetype horn.");
    archetype *horn2 = find_archetype("horn2");
    fail_unless(horn2 != NULL, "couldn't find archetype horn2.");

    for (test = 0; test < 100000; test++) {
        object *check = arch_to_object(RANDOM()%2 ? horn : horn2);

        generate_artifact(check, RANDOM()%100);
        fail_unless(check->inv != NULL, "horn without inventory!");
    }
#endif

    int test, level, found = 0;
    object *the_chest, *check;
    mapstruct *map;
    treasurelist *tlist = find_treasurelist("uncommon_items");
    fail_unless(tlist != NULL, "couldn't find treasure list uncommon_items");

    for (test = 0; test < 10; test++) {
        for (level = 1; level < 120; level++) {
            map = get_empty_map(1, 1);
            fail_unless(map != NULL, "failed to get empty map");
            map->difficulty = level;

            the_chest = create_archetype("chest");  /* was "chest_2" */
            fail_unless(the_chest != NULL, "failed to get chest");
            the_chest->randomitems = tlist;
            the_chest->stats.hp = RANDOM()%100;
            object_insert_in_map_at(the_chest, map, NULL, 0, 0, 0);
            apply_auto_fix(map);
            the_chest = GET_MAP_OB(map, 0, 0);
            fail_unless(the_chest != NULL, "failed to recover chest?");
            for (check = the_chest->inv; check; check = check->below) {
                if (check->type != ROD)
                    continue;
                local_check_loaded_object(check);
                fail_unless(check->inv != NULL, "horn without inventory");
                fail_unless(check->inv->below == NULL, "horn with 2 items");
                fail_unless(check->randomitems == NULL, "horn with randomitems set");
                found++;
            }
            delete_map(map);
        }
    }
    fail_unless(found > 100, "didn't find 100 horn but %d??", found);

}
END_TEST

static Suite *bug_suite(void) {
    Suite *s = suite_create("bug");
    TCase *tc_core = tcase_create("Core");

    /*setup and teardown will be called before each test in testcase 'tc_core' */
    tcase_add_checked_fixture(tc_core, setup, teardown);

    suite_add_tcase(s, tc_core);
    tcase_add_test(tc_core, test_randommaps);
    tcase_set_timeout(tc_core, 0);

    return s;
}

int main(void) {
    int nf;
    Suite *s = bug_suite();
    SRunner *sr = srunner_create(s);

    srunner_set_fork_status(sr, CK_NOFORK);
    init(0, NULL);

    srunner_set_xml(sr, LOGDIR "/bugs/bugtrack/1727944.xml");
    srunner_set_log(sr, LOGDIR "/bugs/bugtrack/1727944.out");
    srunner_run_all(sr, CK_ENV); /*verbosity from env variable*/
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
