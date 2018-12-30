/*
 * bwp - build wiki pages
 *
 * This program will sort out all monster archetypes and print wiki pages
 * for them, named 'a' through 'z'.  It uses some *_template subroutines taken
 * from Ryo's mapper.c.  It should compile if installed in server/trunk/utils.
 * Please direct all suggestions or corrections to aaron@baugher.biz (or
 * Mhoram on #crossfire).
 *
 * Compile command: gcc -g -O0 bwp.c -I../include ../common/libcross.a ../socket/libsocket.a -o bwp -lz -lcrypt -lm
 */

/*
 * CrossFire, A Multiplayer game for X-windows
 *
 * Copyright (C) 2002-2006 Mark Wedel & Crossfire Development Team
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

#define LO_NEWFILE 2
#define MAX_SIZE 64
#define NA "n/a"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <global.h>

char *monster_page_head;       /* Head of wiki page of monsters  */
char *monster_page_foot;       /* Foot of wiki page of monsters  */
char *monster_entry;           /* A single monster entry         */
char *monster_canuse_row;      /* Can_use table row              */
char *monster_protected_row;   /* Protected table row            */
char *monster_vulnerable_row;  /* Vulnerable table row           */
char *monster_special_row;     /* Special table row              */
char *monster_attack_row;      /* Attack types table row         */
char *monster_lore_row;        /* Lore table row                 */

typedef struct string_array {
    sint16 count;
    char **item;
} String_Array;

/**
 * This is a list of pointers that correspond to the FLAG_.. values.
 * This is a simple 1:1 mapping - if FLAG_FRIENDLY is 15, then
 * the 15'th element of this array should match that name.
 * If an entry is NULL, that is a flag not to loaded/saved.
 *
 * Copied from common/loader.c; perhaps should be defined elsewhere?
 *
 */
const char *const flag_names[NUM_FLAGS+1] = {
    "alive", "wiz", NULL, NULL, "was_wiz", "applied", "unpaid",
    "can_use_shield", "no_pick", "client_anim_sync", "client_anim_random", /* 10 */
    "is_animated", NULL /* slow_move */,
    NULL /* flying */, "monster", "friendly", "generator",
    "is_thrown", "auto_apply", "treasure", "player sold",   /* 20 */
    "see_invisible", "can_roll", "overlay_floor",
    "is_turnable", NULL /* walk_off */, NULL /* fly_on */,
    NULL /*fly_off*/, "is_used_up", "identified", "reflecting", /* 30 */
    "changing", "splitting", "hitback", "startequip",
    "blocksview", "undead", "scared", "unaggressive",
    "reflect_missile", "reflect_spell",                             /* 40 */
    "no_magic", "no_fix_player", "is_lightable", "tear_down",
    "run_away", NULL /*pass_thru */, NULL /*can_pass_thru*/,
    "pick_up", "unique", "no_drop",                                     /* 50 */
    NULL /* wizcast*/, "can_cast_spell", "can_use_scroll", "can_use_range",
    "can_use_bow",  "can_use_armour", "can_use_weapon",
    "can_use_ring", "has_ready_range", "has_ready_bow",             /* 60 */
    "xrays", NULL, "is_floor", "lifesave", "no_strength", "sleep",
    "stand_still", "random_movement", "only_attack", "confused",        /* 70 */
    "stealth", NULL, NULL, "cursed", "damned",
    "see_anywhere", "known_magical", "known_cursed",
    "can_use_skill", "been_applied",                                /* 80 */
    "has_ready_scroll", NULL, NULL,
    NULL, "make_invisible",  "inv_locked", "is_wooded",
    "is_hilly", "has_ready_skill", "has_ready_weapon",              /* 90 */
    "no_skill_ident", "is_blind", "can_see_in_dark", "is_cauldron",
    "is_dust", "no_steal", "one_hit", NULL, "berserk", "neutral",       /* 100 */
    "no_attack", "no_damage", NULL, NULL, "activate_on_push",
    "activate_on_release", "is_water", "use_content_on_gen", NULL, "is_buildable", /* 110 */
    NULL, "blessed", "known_blessed"
};

/**
 * Concatenates a string, and free concatenated string.
 *
 * @param source
 * string to append to. Can be NULL.
 * @param add
 * string that is appened. Will be free()d after. Must not be NULL.
 * @return
 * new string that should be free()d by caller.
 */
static char *cat_template(char *source, char *add) {
    if (!source)
        return add;
    source = realloc(source, strlen(source)+strlen(add)+1);
    strcat(source, add);
    free(add);
    return source;
}

/**
 * Reads a file in memory.
 *
 * @param name
 * file path to read.
 * @param buffer
 * where to store. Can be left uninitialized in case of errors.
 * @return
 * 1 if error, 0 else.
 */
static int read_template(const char *name, char **buffer) {
    FILE *file;
    size_t size;
    struct stat info;

    if (stat(name, &info)) {
        printf("Couldn't stat template %s!\n", name);
        return 1;
    }

    (*buffer) = calloc(1, info.st_size+1);
    if (!(*buffer)) {
        printf("Template %s calloc failed!\n", name);
        return 1;
    }

    if (info.st_size == 0) {
        (*buffer)[0] = '\0';
        return 0;
    }

    file = fopen(name, "rb");
    if (!file) {
        printf("Couldn't open template %s!\n", name);
        free(*buffer);
        return 1;
    }
    if (fread(*buffer, info.st_size, 1, file) != 1) {
        printf("Couldn't read template %s!\n", name);
        free(*buffer);
        fclose(file);
        return 1;
    }
    fclose(file);
    return 0;
}

/**
 * Processes a template.
 *
 * Variables in the form <code>\#VARIABLE#</code> will be substituted for specified values.
 *
 * @param template
 * template to process.
 * @param vars
 * variables to replace. Array must be NULL-terminated.
 * @param values
 * variables to replace by. Must be the same size as vars, no NULL element allowed.
 * @return
 * filled-in template, that must be free()d be caller. NULL if memory allocation error.
 *
 * @note
 * returned string will be a memory block larger than required, for performance reasons.
 */
static char *do_template(const char *template, const char **vars, const char **values) {
    int count = 0;
    const char *sharp = template;
    int maxlen = 0;
    int var = 0;
    char *result;
    char *current_result;
    const char *end;

    while ((sharp = strchr(sharp, '#')) != NULL) {
        sharp++;
        count++;
    }
    if (!count)
        return strdup(template);
    if (count%2) {
        printf("Malformed template, mismatched #!\n");
        return strdup(template);
    }

    while (vars[var] != NULL) {
        if (strlen(values[var]) > maxlen)
            maxlen = strlen(values[var]);
        var++;
    }
    result = calloc(1, strlen(template)+maxlen*(count/2)+1);
    if (!result)
        return NULL;
    current_result = result;

    sharp = template;
    while ((sharp = strchr(sharp, '#')) != NULL) {
        end = strchr(sharp+1, '#');
        strncpy(current_result, template, sharp-template);
        if (end == sharp+1) {
            strcat(current_result, "#");
        } else {
            current_result = current_result+strlen(current_result);
            var = 0;
            while (vars[var] != 0 && strncmp(vars[var], sharp+1, end-sharp-1))
                var++;
            if (vars[var] == 0)
                printf("Wrong tag: %s\n", sharp);
            else
                strcpy(current_result, values[var]);
        }
        current_result = current_result+strlen(current_result);
        sharp = end+1;
        template = sharp;
    }
    strcat(current_result, template);
    return result;
}

/****  Mhoram's code starts here *****/

/**
 * Frees memory if the pointer was ever given a string.
 *
 * There's probably a cleaner way to do this, but this frees the memory
 * given to a pointer if the pointer points to a string longer than zero
 * length.  It's to get rid of "in free(): warning: junk pointer, too high
 * to make sense" errors.
 *
 * @param p
 * Pointer to free memory from
 *
 */
static void free_if_used(char *p) {
    if (p && strlen(p) > 0) {
        free(p);
    }
}

/**
 * Sort values alphabetically
 *
 * Used by qsort to sort values alphabetically without regard to case
 *
 * @param a
 * First value
 *
 * @param b
 * Second value
 *
 */
static int sortbyname(const void *a, const void *b) {
    return (strcasecmp(*(const char **)a, *(const char **)b));
}

/**
 * Sort archetypes alphabetically
 *
 * Used by qsort to sort archetypes alphabetically
 * without regard to case
 *
 * @param a
 * First value
 *
 * @param b
 * Second value
 *
 */
static int sort_archetypes(const void *a, const void *b) {
    archetype *aa;
    archetype *bb;

    aa = *(archetype **)a;
    bb = *(archetype **)b;

    return (strcasecmp(aa->clone.name, bb->clone.name));
}

/**
 * Add a string to a String_Array struct
 *
 * Adds the new string to the struct's 'item' array, and updates the 'count'value.
 *
 * @param array
 * The array to be appended
 *
 * @param string
 * The new string to append
 *
 */
void push(String_Array *array, const char *string) {
    sint16 i = array->count;

    array->item[i] = strdup_local(string);
    array->count++;
}

/**
 * Frees the item's data of specified String_Array.
 *
 * Will free array->item and its fields.
 *
 * @param array
 * element we want to clean.
 */
void free_data(String_Array *array) {
    int item;

    for (item = 0; item < array->count; item++)
        free(array->item[item]);
    free(array->item);
    array->item = NULL;
}

/**
 * Joins strings with a comma and space.
 *
 * Takes an array of strings and joins them togther with a comma and a space
 * between each of them.
 *
 * @param array
 * Pointer to struct of type String_Array, containing strings to join
 *
 */
const char *join_with_comma(String_Array *array) {
    char *newtext;
    int i;

    newtext = calloc(1, 1);
    qsort(array->item, array->count, sizeof(char *), sortbyname);
    for (i = 0; i < array->count; i++) {
        if (i) {
            newtext = realloc(newtext, strlen(newtext)+strlen(", ")+1);
            newtext = strncat(newtext, ", ", 2);
        }
        newtext = realloc(newtext, strlen(newtext)+strlen(array->item[i])+1);
        newtext = strncat(newtext, array->item[i], strlen(array->item[i]));
    }
    return newtext;
}

int main(int argc, char *argv[]) {

    archetype *at;
    int archnum = 0;
    archetype *monster[4000];
    int i;
    char letter;
    char last_letter;
    char *wiki_page = NULL;
    char *monster_entries = NULL;

    FILE *fp = NULL;
    FILE *image_list;
    char image_list_path[128];
    char wikifile[128];
    char *template;

    const char *wikidir = "/tmp";  /* Should change this to come from command line? */

    init_globals();
    init_library();
    init_archetypes();
    init_artifacts();
    init_formulae();
    init_readable();

    init_gods();

        /* Initialize templates */
    if (read_template("templates/wiki/monster_page_head", &monster_page_head))
        return;
    if (read_template("templates/wiki/monster_page_foot", &monster_page_foot))
        return;
    if (read_template("templates/wiki/monster_entry", &monster_entry))
        return;
    if (read_template("templates/wiki/monster_canuse_row", &monster_canuse_row))
        return;
    if (read_template("templates/wiki/monster_protected_row", &monster_protected_row))
        return;
    if (read_template("templates/wiki/monster_vulnerable_row", &monster_vulnerable_row))
        return;
    if (read_template("templates/wiki/monster_special_row", &monster_special_row))
        return;
    if (read_template("templates/wiki/monster_attack_row", &monster_attack_row))
        return;
    if (read_template("templates/wiki/monster_lore_row", &monster_lore_row))
        return;
    sprintf(image_list_path, "%s/image_list", wikidir);
    image_list = fopen(image_list_path, "w");
    if (!image_list) {
        LOG(llevError, "Unable to open image list file!\n");
        exit(1);
    }

        /* Pick out the monster archetypes and sort them into an array */
    for (at = first_archetype; at != NULL; at = at->next) {
        if (QUERY_FLAG(&at->clone, FLAG_MONSTER)
        && QUERY_FLAG(&at->clone, FLAG_ALIVE)) {
            monster[archnum++] = at;
        }
    }
    printf("Sorting...");
    /* Calling qsort on monster, which is archetype**     */
    qsort(&monster[0], archnum, sizeof(archetype *), sort_archetypes);
    printf("done.  %i items found\n", archnum);

    last_letter = '\0';

    for (i = 0; i < archnum; i++) {
        at = monster[i];
        if (at) {
            const char *key[16] = { NULL, };
            const char *val[16] = { NULL, };
            char buf[16][MAX_BUF];
            int keycount = 0;
            int res;

            letter = tolower(at->clone.name[0]);

            LOG(llevInfo, "Doing archetype %s\n", at->name);

            if (letter != last_letter) {  /* New letter, new file */
                if (fp) {
                    keycount = 0;
                    key[keycount] = NULL;
                    template = do_template(monster_page_foot, key, val);
                    res = fprintf(fp, "%s", template);
                    free(template);
                    template = NULL;
                    if (res < 0) {
                        LOG(llevError, "Unable to write to file!\n");
                    }
                    fclose(fp);
                }

                snprintf(wikifile, sizeof(wikifile), "%s/%c", wikidir, letter);
                fp = fopen(wikifile, "w");
                if (!fp) {
                    fprintf(stderr, "Unable to write to wiki file!\n");
                    exit(1);
                }

                char letterindex[256] = "";
                char letterindexnext[7];
                char li;
                letterindexnext[0] = '\0';
                for (li = 'a'; li <= 'z'; li++) {
                    char *p;

                    if (li == letter) {
                        sprintf(letterindexnext, "%c ", toupper(li));
                    } else {
                        sprintf(letterindexnext, "[[%c]] ", toupper(li));
                    }
                    p = strchr(letterindex, '\0');
                    snprintf(p, letterindex+sizeof(letterindex)-p, "%s", letterindexnext);
                }

                keycount = 0;
                key[keycount] = "LETTER";
                sprintf(buf[keycount], "%c", toupper(letter));
                val[keycount++] = buf[keycount];
                key[keycount] = "LETTERINDEX";
                val[keycount++] = letterindex;
                key[keycount] = NULL;
                template = do_template(monster_page_head, key, val);
                res = fprintf(fp, template);
                free(template);
                if (res < 0) {
                    LOG(llevError, "Unable to write to file!");
                }
                last_letter = letter;
            }

            /* add a monster entry */
            char *canuse_row;
            char *protected_row;
            char *vulnerable_row;
            char *special_row;
            char *attack_row;
            char *lore_row;
            const int CANUSE_LENGTH = 16;
            String_Array canuse;
            String_Array resist;
            String_Array vulner;
            String_Array attack;
            String_Array special;
            /* Some flags that seemed useful; may need to add to this list.
             * *special_names[] is used because some of the names in
             * define.h are a bit awkward.  Last one is negative to mark end.
                 */
            const sint8 special_flags[] = { 21, 93, 52, 38, 13, 32, 61, -1 };
            const char *special_names[] = {
                "see invisible",
                "see in dark",
                "spellcaster",
                "unaggressive",
                "flying",
                "splitting",
                "x-ray vision"
            };
            int j;

            canuse.item = calloc(1, sizeof(const char *)*(CANUSE_LENGTH+1));
            resist.item = calloc(1, sizeof(const char *)*(NROFATTACKS+1));
            vulner.item = calloc(1, sizeof(const char *)*(NROFATTACKS+1));
            attack.item = calloc(1, sizeof(const char *)*(NROFATTACKS+1));
            special.item = calloc(1, sizeof(const char *)*(NROFATTACKS+1));

                /* Do lore row */
            if (at->clone.lore) {
                key[keycount] = "LORE";
                key[keycount+1] = NULL;
                val[keycount] = at->clone.lore;
                keycount++;
                lore_row = do_template(monster_lore_row, key, val);
            } else
                lore_row = strdup("");

                /* Do canuse row */
            canuse.count = 0;
            keycount = 0;
            for (j = 1; j <= NUM_FLAGS; j++) {
                if (QUERY_FLAG(&at->clone, j)
                && flag_names[j]
                && !strncmp(flag_names[j], "can_use_", 8)) {
                    push(&canuse, flag_names[j]+8);
                }
            }
            if (canuse.count) {
                key[keycount] = "CANUSE";
                key[keycount+1] = NULL;
                val[keycount] = join_with_comma(&canuse);
                canuse_row = do_template(monster_canuse_row, key, val);
                free(val[keycount]);
            } else
                canuse_row = strdup("");

                /* Do protected/vulnerable rows */
            resist.count = 0;
            vulner.count = 0;
            for (j = 0; j <= NROFATTACKS; j++) {
                if (at->clone.resist[j] && attacktype_desc[j]) {
                    char rowtext[32];

                    if (at->clone.resist[j] < 0) {
                        sprintf(rowtext, "%s %i", attacktype_desc[j], at->clone.resist[j]);
                        push(&vulner, rowtext);
                    } else {
                        sprintf(rowtext, "%s +%i", attacktype_desc[j], at->clone.resist[j]);
                        push(&resist, rowtext);
                    }
                }
            }
            keycount = 0;
            if (resist.count) {
                key[keycount] = "PROTECTED";
                key[keycount+1] = NULL;
                val[keycount] = join_with_comma(&resist);
                protected_row = do_template(monster_protected_row, key, val);
                free(val[keycount]);
            } else
                protected_row = strdup("");

            keycount = 0;
            if (vulner.count) {
                key[keycount] = "VULNERABLE";
                key[keycount+1] = NULL;
                val[keycount] = join_with_comma(&vulner);
                vulnerable_row = do_template(monster_vulnerable_row, key, val);
                free(val[keycount]);
            } else
                vulnerable_row = strdup("");

                /* Do attacktype row */
            attack.count = 0;
            keycount = 0;
            val[keycount] = NULL;
            for (j = 0; j <= NROFATTACKS; j++) {
                if (at->clone.attacktype&(1U<<j)) {
                    push(&attack, attacktype_desc[j]);
                }
            }
            if (attack.count) {
                key[keycount] = "ATTACKS";
                key[keycount+1] = NULL;
                val[keycount] = join_with_comma(&attack);
                attack_row = do_template(monster_attack_row, key, val);
                free(val[keycount]);
            } else
                attack_row = strdup("");

                /* Do special row */
            special.count = 0;
            keycount = 0;
            val[keycount] = NULL;
            for (j = 0; special_flags[j] >= 0; j++) {
                if (QUERY_FLAG(&at->clone, special_flags[j])) {
                    push(&special, special_names[j]);
                }
            }
            if (special.count) {
                key[keycount] = "SPECIAL";
                key[keycount+1] = NULL;
                val[keycount] = join_with_comma(&special);
                special_row = do_template(monster_special_row, key, val);
                free(val[keycount]);
            } else
                special_row = strdup("");

            keycount = 0;
            key[keycount] = "CANUSEROW";
            val[keycount++] = canuse_row;
            key[keycount] = "PROTECTEDROW";
            val[keycount++] = protected_row;
            key[keycount] = "VULNERABLEROW";
            val[keycount++] = vulnerable_row;
            key[keycount] = "SPECIALROW";
            val[keycount++] = attack_row;
            key[keycount] = "ATTACKROW";
            val[keycount++] = special_row;
            key[keycount] = "LOREROW";
            val[keycount++] = lore_row;
            key[keycount] = "XP";
            sprintf(buf[keycount], "%li", at->clone.stats.exp);
            val[keycount++] = buf[keycount];
            key[keycount] = "HP";
            sprintf(buf[keycount], "%i", at->clone.stats.hp);
            val[keycount++] = buf[keycount];
            key[keycount] = "AC";
            sprintf(buf[keycount], "%i", at->clone.stats.ac);
            val[keycount++] = buf[keycount];
            key[keycount] = "NAME";
            val[keycount++] = at->clone.name;
            key[keycount] = "RACE";
            if (at->clone.race) {
                val[keycount++] = at->clone.race;
            } else {
                val[keycount++] = NA;
            }
            if (at->clone.face->name) {
                key[keycount] = "FACE";
                sprintf(buf[keycount], "{{http://aaron.baugher.biz/images/cf/%s.png}}", at->clone.face->name);
                val[keycount++] = buf[keycount];
                sprintf(buf[keycount], "%s.png\n", at->clone.face->name);
                fprintf(image_list, buf[keycount]);
            }
/*  Plan to add generator face too, when I decide how    */
            key[keycount] = "GENFACE";
            val[keycount++] = "";
            key[keycount] = NULL;

            template = do_template(monster_entry, key, val);
            fprintf(fp, template);
            free(template);
            template = NULL;

            free_data(&canuse);
            free_data(&resist);
            free_data(&vulner);
            free_data(&attack);
            free_data(&special);
            free(canuse_row);
            free(protected_row);
            free(vulnerable_row);
            free(attack_row);
            free(special_row);
            free(lore_row);
        } else {
            LOG(llevError, "Something is very wrong.\n");
        }
    }
    fclose(image_list);
}

void set_map_timeout(void) {
    /* doesn't need to do anything */
}

#include <global.h>

/* some plagarized code from apply.c--I needed just these two functions
   without all the rest of the junk, so.... */
int apply_auto(object *op) {
    object *tmp = NULL;
    int i;

    switch (op->type) {
    case SHOP_FLOOR:
        if (!HAS_RANDOM_ITEMS(op))
            return 0;
        do {
            i = 10; /* let's give it 10 tries */
            while ((tmp = generate_treasure(op->randomitems, op->stats.exp ? op->stats.exp : 5)) == NULL && --i)
                ;
            if (tmp == NULL)
                return 0;
            if (QUERY_FLAG(tmp, FLAG_CURSED) || QUERY_FLAG(tmp, FLAG_DAMNED)) {
                object_free_drop_inventory(tmp);
                tmp = NULL;
            }
        } while (!tmp);

        SET_FLAG(tmp, FLAG_UNPAID);
        object_insert_in_map_at(tmp, op->map, NULL, 0, op->x, op->y);
        CLEAR_FLAG(op, FLAG_AUTO_APPLY);
        tmp = identify(tmp);
        break;

    case TREASURE:
        if (HAS_RANDOM_ITEMS(op))
            while ((op->stats.hp--) > 0)
                create_treasure(op->randomitems, op, GT_ENVIRONMENT, op->stats.exp ? op->stats.exp : op->map == NULL ? 14 : op->map->difficulty, 0);
        object_remove(op);
        object_free_drop_inventory(op);
        break;
    }

    return tmp ? 1 : 0;
}

/* apply_auto_fix goes through the entire map (only the first time
 * when an original map is loaded) and performs special actions for
 * certain objects (most initialization of chests and creation of
 * treasures and stuff).  Calls apply_auto if appropriate.
 */
void apply_auto_fix(mapstruct *m) {
    object *tmp, *above = NULL;
    int x, y;

    for (x = 0; x < MAP_WIDTH(m); x++)
        for (y = 0; y < MAP_HEIGHT(m); y++)
            for (tmp = GET_MAP_OB(m, x, y); tmp != NULL; tmp = above) {
                above = tmp->above;

                if (QUERY_FLAG(tmp, FLAG_AUTO_APPLY))
                    apply_auto(tmp);
                else if (tmp->type == TREASURE) {
                    if (HAS_RANDOM_ITEMS(tmp))
                        while ((tmp->stats.hp--) > 0)
                            create_treasure(tmp->randomitems, tmp, 0, m->difficulty, 0);
                }
                if (tmp
                && tmp->arch
                && tmp->type != PLAYER
                && tmp->type != TREASURE
                && tmp->randomitems) {
                    if (tmp->type == CONTAINER) {
                        if (HAS_RANDOM_ITEMS(tmp))
                            while ((tmp->stats.hp--) > 0)
                                create_treasure(tmp->randomitems, tmp, 0, m->difficulty, 0);
                    } else if (HAS_RANDOM_ITEMS(tmp)) {
                        create_treasure(tmp->randomitems, tmp, 0, m->difficulty, 0);
                        if (QUERY_FLAG(tmp, FLAG_MONSTER)) {
                            monster_check_apply_all(tmp);
                        }
                    }
                }
            }
    for (x = 0; x < MAP_WIDTH(m); x++)
        for (y = 0; y < MAP_HEIGHT(m); y++)
            for (tmp = GET_MAP_OB(m, x, y); tmp != NULL; tmp = tmp->above)
                if (tmp->above
                && (tmp->type == TRIGGER_BUTTON || tmp->type == TRIGGER_PEDESTAL))
                    check_trigger(tmp, tmp->above);
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * Those are dummy functions defined to resolve all symboles.
 * Added as part of glue cleaning.
 * Ryo 2005-07-15
 **/
void draw_ext_info(int flags, int pri, const object *pl, uint8 type, uint8 subtype, const char *txt) {
    fprintf(logfile, "%s\n", txt);
}

void draw_ext_info_format(int flags, int pri, const object *pl, uint8 type, uint8 subtype, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    vfprintf(logfile, format, ap);
    va_end(ap);
}

void ext_info_map(int color, const mapstruct *map, uint8 type, uint8 subtype, const char *str1) {
    fprintf(logfile, "ext_info_map: %s\n", str1);
}

void move_firewall(object *ob) {
}

void emergency_save(int x) {
}

void clean_tmp_files(void) {
}

void esrv_send_item(object *ob, object *obx) {
}

void dragon_ability_gain(object *ob, int x, int y) {
}

void set_darkness_map(mapstruct *m) {
}

object *find_skill_by_number(object *who, int skillno) {
    return NULL;
}

void esrv_del_item(player *pl, object *ob) {
}

void esrv_update_spells(player *pl) {
}

int execute_event(object *op, int eventcode, object *activator, object *third, const char *message, int fix) {
    return 0;
}

int execute_global_event(int eventcode, ...) {
    return 0;
}
#endif /* dummy DOXYGEN_SHOULD_SKIP_THIS */
