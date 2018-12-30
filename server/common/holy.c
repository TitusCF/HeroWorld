/*
 * Crossfire -- cooperative multi-player graphical RPG and adventure game
 *
 * Copyright (c) 1999-2014 Mark Wedel and the Crossfire Development Team
 * Copyright (c) 1992 Frank Tore Johansen
 *
 * Crossfire is free software and comes with ABSOLUTELY NO WARRANTY. You are
 * welcome to redistribute it under certain conditions. For details, please
 * see COPYING and LICENSE.
 *
 * The authors can be reached via e-mail at <crossfire@metalforge.org>.
 */

/**
 * @file holy.c
 * God-related common functions.
 */

#include <stdlib.h>
#include <global.h>
#include <living.h>
#include <spells.h>
#include <libproto.h>
#include <assert.h>

static void add_god_to_list(archetype *god_arch);

/**
 * Initializes a god structure.
 *
 * @note
 * Will never return NULL.
 */
static godlink *init_godslist(void) {
    godlink *gl = (godlink *)malloc(sizeof(godlink));
    if (gl == NULL)
        fatal(OUT_OF_MEMORY);
    gl->name = NULL;    /* how to describe the god to the player */
    gl->arch = NULL;    /* pointer to the archetype of this god */
    gl->id = 0;         /* id of the god */
    gl->next = NULL;    /* next god in this linked list */

    return gl;
}

/**
 * This takes a look at all of the archetypes to find
 * the objects which correspond to the GODS (type GOD)
 */
void init_gods(void) {
    archetype *at = NULL;

    LOG(llevDebug, "Initializing gods...\n");
    for (at = first_archetype; at != NULL; at = at->next)
        if (at->clone.type == GOD)
            add_god_to_list(at);

    LOG(llevDebug, "done.\n");
}

/**
 * Adds specified god to linked list, gives it an id.
 *
 * @param god_arch
 * God to add. If NULL, will log an error.
 */
static void add_god_to_list(archetype *god_arch) {
    godlink *god;

    if (!god_arch) {
        LOG(llevError, "ERROR: Tried to add null god to list!\n");
        return;
    }

    god = init_godslist();

    god->arch = god_arch;
    god->name = add_string(god_arch->clone.name);
    if (!first_god)
        god->id = 1;
    else {
        god->id = first_god->id+1;
        god->next = first_god;
    }
    first_god = god;

#ifdef DEBUG_GODS
    LOG(llevDebug, "Adding god %s (%d) to list\n", god->name, god->id);
#endif
}

/**
 * Returns a random god.
 *
 * @return
 * a random god, or NULL if no god was found.
 */
godlink *get_rand_god(void) {
    godlink *god = first_god;
    int i;

    if (god)
        for (i = RANDOM()%(god->id)+1; god; god = god->next)
            if (god->id == i)
                break;

    if (!god)
        LOG(llevError, "get_rand_god(): can't find a random god!\n");
    return god;
}

/**
 * Returns a pointer to the object
 * We need to be VERY careful about using this, as we
 * are returning a pointer to the CLONE object. -b.t.
 *
 * @param godlnk
 * god to get object.
 */
const object *pntr_to_god_obj(godlink *godlnk) {
    if (godlnk && godlnk->arch)
        return &godlnk->arch->clone;
    return NULL;
}

/**
 * Describe a god. The reason we return a combination is to know what exactly was written for knowledge management.
 * @param god which god to describe.
 * @param what information to describe, combination of @ref GOD_xxx "GOD_xxx" flags.
 * @param buf where to describe, must not be NULL.
 * @param maxlen maximum wanted length of the description, if 0 no maximum length.
 * @return information actually written, combination of @ref GOD_xxx "GOD_xxx", based on maxlen.
 */
int describe_god(const object *god, int what, StringBuffer *buf, size_t maxlen) {
    int hassomething = 0, real = 0;
    char temp[HUGE_BUF];
    StringBuffer *add;
    const char *name;

    name = god->name;

    /* preamble.. */
    assert(buf);
    stringbuffer_append_printf(buf, "This document contains knowledge concerning the diety %s", name);

    /* Always have as default information the god's descriptive terms. */
    if (nstrtok(god->msg, ",") > 0) {
        stringbuffer_append_string(buf, ", known as");
        stringbuffer_append_string(buf, strtoktolin(god->msg, ",", temp, sizeof(temp)));
    } else
        stringbuffer_append_string(buf, "...");

    stringbuffer_append_string(buf, "\n ---\n");

    if (what & GOD_PATHS) {
        /* spell paths */
        int has_effect = 0;

        add = stringbuffer_new();
        stringbuffer_append_printf(add, "It is rarely known fact that the priests of %s are mystically transformed. Effects of this include:\n", name);

        if (god->path_attuned) {
            has_effect = 1;
            describe_spellpath_attenuation("Attuned", god->path_attuned, add);
        }
        if (god->path_repelled) {
            has_effect = 1;
            describe_spellpath_attenuation("Repelled", god->path_repelled, add);
        }
        if (god->path_denied) {
            has_effect = 1;
            describe_spellpath_attenuation("Denied", god->path_denied, add);
        }
        if (has_effect) {
            stringbuffer_append_string(add, "\n ---\n");

            if ((maxlen == 0) || (stringbuffer_length(add) + stringbuffer_length(buf) < maxlen)) {
                stringbuffer_append_stringbuffer(buf, add);
                free(stringbuffer_finish(add));
                hassomething = 1;
            } else {
                free(stringbuffer_finish(add));
                return real;
            }
        } else {
            free(stringbuffer_finish(add));
        }
        what |= GOD_PATHS;
    }

    if (what & GOD_IMMUNITIES) {
        int has_effect = 0, tmpvar;
        char tmpbuf[MAX_BUF];

        add = stringbuffer_new();
        stringbuffer_append_printf(add, "\nThe priests of %s are known to make cast a mighty prayer of possession which gives the recipient ", name);

        for (tmpvar = 0; tmpvar < NROFATTACKS; tmpvar++) {
            if (god->resist[tmpvar] == 100) {
                has_effect = 1;
                snprintf(tmpbuf, MAX_BUF, "Immunity to %s", attacktype_desc[tmpvar]);
            }
        }
        if (has_effect) {
            stringbuffer_append_printf(add, "%s\n ---\n", tmpbuf);
            if ((maxlen == 0) || (stringbuffer_length(add) + stringbuffer_length(buf) < maxlen)) {
                stringbuffer_append_stringbuffer(buf, add);
                free(stringbuffer_finish(add));
                hassomething = 1;
            } else {
                free(stringbuffer_finish(add));
                return real;
            }
        } else {
            free(stringbuffer_finish(add));
        }
        real |= GOD_IMMUNITIES;
    }

    if (what & GOD_BLESSED) {
        char *cp = stringbuffer_finish(describe_resistance(god, 1, NULL));

        if (*cp) {  /* This god does have protections */
            add = stringbuffer_new();
            stringbuffer_append_printf(add, "\nThe priests of %s are known to be able to bestow a blessing which makes the recipient %s\n ---\n", name, cp);

            if ((maxlen == 0) || (stringbuffer_length(add) + stringbuffer_length(buf) < maxlen)) {
                stringbuffer_append_stringbuffer(buf, add);
                free(stringbuffer_finish(add));
                hassomething = 1;
            } else {
                free(stringbuffer_finish(add));
                return real;
            }
        }
        free(cp);
        real |= GOD_BLESSED;
    }

    if (what & GOD_SACRED) {
        if (god->race && !(god->path_denied&PATH_SUMMON)) {
            int i = nstrtok(god->race, ",");
            add = stringbuffer_new();
            stringbuffer_append_printf(add, "Creatures sacred to %s include the\n", name);

            if (i > 1)
                stringbuffer_append_printf(add, "following races:%s\n ---\n", strtoktolin(god->race, ",", temp, sizeof(temp)));
            else
                stringbuffer_append_printf(add, "race of %s\n ---\n", strtoktolin(god->race, ",", temp, sizeof(temp)));

            if ((maxlen == 0) || (stringbuffer_length(add) + stringbuffer_length(buf) < maxlen)) {
                stringbuffer_append_stringbuffer(buf, add);
                free(stringbuffer_finish(add));
                hassomething = 1;
            } else {
                free(stringbuffer_finish(add));
                return real;
            }
        }
        real |= GOD_SACRED;
    }

    if (what & GOD_RESISTANCES) {
        char *cp;
        cp = stringbuffer_finish(describe_resistance(god, 1, NULL));

        if (*cp) {  /* This god does have protections */
            add = stringbuffer_new();
            stringbuffer_append_printf(add, "%s has a potent aura which is extended to faithful priests. The effects of this aura include:\n%s\n ---\n", name, cp);

            if ((maxlen == 0) || (stringbuffer_length(add) + stringbuffer_length(buf) < maxlen)) {
                stringbuffer_append_stringbuffer(buf, add);
                free(stringbuffer_finish(add));
                hassomething = 1;
            } else {
                free(stringbuffer_finish(add));
                return real;
            }
        }
        free(cp);
        real |= GOD_RESISTANCES;
    }

    if (what & GOD_HOLYWORD) {
        if ((god->slaying) && !(god->path_denied&PATH_TURNING)) {
            add = stringbuffer_new();
            stringbuffer_append_printf(add, "The holy words of %s have the power to slay creatures belonging to the ", name);

            if (nstrtok(god->slaying, ",") > 1)
                stringbuffer_append_printf(add, "following races:%s\n ---\n", strtoktolin(god->slaying, ",", temp, sizeof(temp)));
            else
                stringbuffer_append_printf(add, "race of%s\n ---\n", strtoktolin(god->slaying, ",", temp, sizeof(temp)));

            if ((maxlen == 0) || (stringbuffer_length(add) + stringbuffer_length(buf) < maxlen)) {
                stringbuffer_append_stringbuffer(buf, add);
                free(stringbuffer_finish(add));
                hassomething = 1;
            } else {
                free(stringbuffer_finish(add));
                return real;
            }
        }
        real |= GOD_HOLYWORD;
    }

    if (what & GOD_ENEMY) {
        if (god->title) {
            add = stringbuffer_new();
            stringbuffer_append_printf(add, "The gods %s and %s are enemies.\n ---\n", name, god->title);

            if ((maxlen == 0) || (stringbuffer_length(add) + stringbuffer_length(buf) < maxlen)) {
                stringbuffer_append_stringbuffer(buf, add);
                free(stringbuffer_finish(add));
                hassomething = 1;
            } else {
                free(stringbuffer_finish(add));
                return real;
            }
        }
        real |= GOD_ENEMY;
    }

    if (hassomething == 0) {
        /* we got no information beyond the preamble! */
        stringbuffer_append_string(buf, " Unfortunately the rest of the information is hopelessly garbled!\n ---\n");
    }

    return real;
}

/**
 * Frees all god information.
 */
void free_all_god(void) {
    godlink *god, *godnext;

    LOG(llevDebug, "Freeing god information\n");
    for (god = first_god; god; god = godnext) {
        godnext = god->next;
        if (god->name)
            free_string(god->name);
        free(god);
    }
    first_god = NULL;
}

#define DESCRIBE_ABILITY(retbuf, variable, name)                   \
    if (variable) {                                                \
        int i, j = 0;                                              \
        strcat(retbuf, "(" name ": ");                             \
        for (i = 0; i < NROFATTACKS; i++)                          \
            if (variable&(1<<i)) {                                 \
                if (j)                                             \
                    strcat(retbuf, ", ");                          \
                else                                               \
                    j = 1;                                         \
                strcat(retbuf, attacks[i]);                        \
            }                                                      \
        strcat(retbuf, ")");                                       \
    }

#define DESCRIBE_PATH(retbuf, variable, name)                      \
    if (variable) {                                                \
        int i, j = 0;                                              \
        strcat(retbuf, "(" name ": ");                             \
        for (i = 0; i < NRSPELLPATHS; i++)                         \
            if (variable&(1<<i)) {                                 \
                if (j)                                             \
                    strcat(retbuf, ", ");                          \
                else                                               \
                    j = 1;                                         \
                strcat(retbuf, spellpathnames[i]);                 \
            }                                                      \
        strcat(retbuf, ")");                                       \
    }

/**
 * Prints all gods to stderr.
 *
 * @todo
 * use LOG instead of fprintf().
 */
void dump_gods(void) {
    godlink *glist;

    fprintf(stderr, "\n");
    for (glist = first_god; glist; glist = glist->next) {
        const object *god = pntr_to_god_obj(glist);
        char tmpbuf[HUGE_BUF], *final;
        int tmpvar, gifts = 0;

        fprintf(stderr, "GOD: %s\n", god->name);
        fprintf(stderr, " avatar stats:\n");
        fprintf(stderr, "  S:%d C:%d D:%d I:%d W:%d P:%d\n", god->stats.Str, god->stats.Con, god->stats.Dex, god->stats.Int, god->stats.Wis, god->stats.Pow);
        fprintf(stderr, "  lvl:%d speed:%4.2f\n", god->level, god->speed);
        fprintf(stderr, "  wc:%d ac:%d hp:%d dam:%d\n", god->stats.wc, god->stats.ac, god->stats.hp, god->stats.dam);
        fprintf(stderr, " enemy: %s\n", god->title ? god->title : "NONE");
        if (god->other_arch) {
            object *serv = &god->other_arch->clone;
            fprintf(stderr, " servant stats: (%s)\n", god->other_arch->name);
            fprintf(stderr, "  S:%d C:%d D:%d I:%d W:%d P:%d\n", serv->stats.Str, serv->stats.Con, serv->stats.Dex, serv->stats.Int, serv->stats.Wis, serv->stats.Pow);
            fprintf(stderr, "  lvl:%d speed:%4.2f\n", serv->level, serv->speed);
            fprintf(stderr, "  wc:%d ac:%d hp:%d dam:%d\n", serv->stats.wc, serv->stats.ac, serv->stats.hp, serv->stats.dam);
        } else
            fprintf(stderr, " servant: NONE\n");
        fprintf(stderr, " aligned_race(s): %s\n", god->race);
        fprintf(stderr, " enemy_race(s): %s\n", (god->slaying ? god->slaying : "none"));
        final = stringbuffer_finish(describe_resistance(god, 1, NULL));
        fprintf(stderr, "%s", final);
        free(final);
        snprintf(tmpbuf, sizeof(tmpbuf), " attacktype:");
        if ((tmpvar = god->attacktype)) {
            strcat(tmpbuf, "\n  ");
            DESCRIBE_ABILITY(tmpbuf, tmpvar, "Attacks");
        }
        strcat(tmpbuf, "\n aura:");

        strcat(tmpbuf, "\n paths:");
        if ((tmpvar = god->path_attuned)) {
            strcat(tmpbuf, "\n  ");
            DESCRIBE_PATH(tmpbuf, tmpvar, "Attuned");
        }
        if ((tmpvar = god->path_repelled)) {
            strcat(tmpbuf, "\n  ");
            DESCRIBE_PATH(tmpbuf, tmpvar, "Repelled");
        }
        if ((tmpvar = god->path_denied)) {
            strcat(tmpbuf, "\n  ");
            DESCRIBE_PATH(tmpbuf, tmpvar, "Denied");
        }
        fprintf(stderr, "%s\n", tmpbuf);
        fprintf(stderr, " Desc: %s", god->msg ? god->msg : "---\n");
        fprintf(stderr, " Priest gifts/limitations: ");
        if (!QUERY_FLAG(god, FLAG_USE_WEAPON)) { gifts = 1; fprintf(stderr, "\n  weapon use is forbidden"); }
        if (!QUERY_FLAG(god, FLAG_USE_ARMOUR)) { gifts = 1; fprintf(stderr, "\n  no armour may be worn"); }
        if (QUERY_FLAG(god, FLAG_UNDEAD)) { gifts = 1; fprintf(stderr, "\n  is undead"); }
        if (QUERY_FLAG(god, FLAG_SEE_IN_DARK)) { gifts = 1; fprintf(stderr, "\n  has infravision "); }
        if (QUERY_FLAG(god, FLAG_XRAYS)) { gifts = 1; fprintf(stderr, "\n  has X-ray vision"); }
        if (QUERY_FLAG(god, FLAG_REFL_MISSILE)) { gifts = 1; fprintf(stderr, "\n  reflect missiles"); }
        if (QUERY_FLAG(god, FLAG_REFL_SPELL)) { gifts = 1; fprintf(stderr, "\n  reflect spells"); }
        if (QUERY_FLAG(god, FLAG_STEALTH)) { gifts = 1; fprintf(stderr, "\n  is stealthy"); }
        if (QUERY_FLAG(god, FLAG_MAKE_INVIS)) { gifts = 1; fprintf(stderr, "\n  is (permanently) invisible"); }
        if (QUERY_FLAG(god, FLAG_BLIND)) { gifts = 1; fprintf(stderr, "\n  is blind"); }
        if (god->last_heal) { gifts = 1; fprintf(stderr, "\n  hp regenerate at %d", god->last_heal); }
        if (god->last_sp) { gifts = 1; fprintf(stderr, "\n  sp regenerate at %d", god->last_sp); }
        if (god->last_eat) { gifts = 1; fprintf(stderr, "\n  digestion is %s (%d)", god->last_eat < 0 ? "slowed" : "faster", god->last_eat); }
        if (god->last_grace) { gifts = 1; fprintf(stderr, "\n  grace regenerates at %d", god->last_grace); }
        if (god->stats.luck) { gifts = 1; fprintf(stderr, "\n  luck is %d", god->stats.luck); }
        if (!gifts) fprintf(stderr, "NONE");
        fprintf(stderr, "\n\n");
    }
}
