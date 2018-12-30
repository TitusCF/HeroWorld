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
 * @file exp.c
 * Experience management. reading data from files and such.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <global.h>

sint64 *levels; /**< Number of levels for which we have experience. */

#define TRUE 1
#define FALSE 0

static const float exp_att_mult[NROFATTACKS] = {
    0.0,    /* AT_PHYSICAL */
    0.0,    /* AT_MAGIC */
    0.0,    /* AT_FIRE */
    0.0,    /* AT_ELECTRICITY */
    0.0,    /* AT_COLD */
    0.0,    /* AT_CONFUSION */
    0.4,    /* AT_ACID */
    1.5,    /* AT_DRAIN */
    0.0,    /* AT_WEAPONMAGIC */
    0.1,    /* AT_GHOSTHIT */
    0.3,    /* AT_POISON */
    0.1,    /* AT_SLOW */
    0.3,    /* AT_PARALYZE */
    0.0,    /* AT_TURN_UNDEAD */
    0.0,    /* AT_FEAR */
    0.0,    /* AT_CANCELLATION */
    0.0,    /* AT_DEPLETE */
    0.0,    /* AT_DEATH */
    0.0,    /* AT_CHAOS */
    0.0,    /* AT_COUNTERSPELL */
    0.5,    /* AT_GODPOWER */
    0.1,    /* AT_HOLYWORD */
    0.2,    /* AT_BLIND */
    0.0,    /* AT_INTERNAL */
    0.5,    /* AT_LIFE_STEALING */
    0.2,    /* AT_DISEASE */
};

static const float exp_prot_mult[NROFATTACKS] = {
    0.4,    /* AT_PHYSICAL */
    0.5,    /* AT_MAGIC */
    0.1,    /* AT_FIRE */
    0.1,    /* AT_ELECTRICITY */
    0.1,    /* AT_COLD */
    0.1,    /* AT_CONFUSION */
    0.1,    /* AT_ACID */
    0.1,    /* AT_DRAIN */
    0.1,    /* AT_WEAPONMAGIC */
    0.1,    /* AT_GHOSTHIT */
    0.1,    /* AT_POISON */
    0.0,    /* AT_SLOW */
    0.1,    /* AT_PARALYZE */
    0.1,    /* AT_TURN_UNDEAD */
    0.1,    /* AT_FEAR */
    0.0,    /* AT_CANCELLATION */
    0.0,    /* AT_DEPLETE */
    0.0,    /* AT_DEATH */
    0.0,    /* AT_CHAOS */
    0.0,    /* AT_COUNTERSPELL */
    0.0,    /* AT_GODPOWER */
    0.1,    /* AT_HOLYWORD */
    0.0,    /* AT_BLIND */
    0.0,    /* AT_INTERNAL */
    0.0,    /* AT_LIFE_STEALING */
    0.1,    /* AT_DISEASE */
};

/**
 * Alternative way to calculate experience based
 * on the ability of a monster.
 *
 * It's far from perfect, and doesn't consider everything which
 * can be considered, thus it's only used in debugging.
 * this is only used with one of the dumpflags,
 * and not anyplace in the code.
 *
 * @param ob
 * object for which to return experience
 * @return
 * experience computed from object's properties.
 */
sint64 new_exp(const object *ob) {
    double att_mult, prot_mult, spec_mult;
    double exp;
    int i;
    long mask = 1;

    att_mult = prot_mult = spec_mult = 1.0;
    for (i = 0; i < NROFATTACKS; i++) {
        mask = 1<<i;
        att_mult += (exp_att_mult[i]*((ob->attacktype&mask) != FALSE));
        /* We multiply & then divide to prevent roundoffs on the floats.
         * the doubling is to take into account the table and resistances
         * are lower than they once were.
         */
        /* prot_mult should increase by fairly minor amounts -
         * for example, if a creature has resist physical 30,
         * and exp mult on that is 0.4, then prot_mult should really
         * go up by 1.2 - still a considerable increase.
         */
        prot_mult += (exp_prot_mult[i]*ob->resist[i])/10.0;
    }

    if (prot_mult < 0)
        prot_mult = 1;

    spec_mult += (0.3*(QUERY_FLAG(ob, FLAG_SEE_INVISIBLE) != FALSE))+
                 (0.5*(QUERY_FLAG(ob, FLAG_SPLITTING) != FALSE))+
                 (0.3*(QUERY_FLAG(ob, FLAG_HITBACK) != FALSE))+
                 (0.1*(QUERY_FLAG(ob, FLAG_REFL_MISSILE) != FALSE))+
                 (0.3*(QUERY_FLAG(ob, FLAG_REFL_SPELL) != FALSE))+
                 (1.0*(QUERY_FLAG(ob, FLAG_NO_MAGIC) != FALSE))+
                 (0.1*(QUERY_FLAG(ob, FLAG_USE_SCROLL) != FALSE))+
                 (0.2*(QUERY_FLAG(ob, FLAG_USE_RANGE) != FALSE))+
                 (0.1*(QUERY_FLAG(ob, FLAG_USE_BOW) != FALSE));

    exp = MAX(ob->stats.maxhp, 5);
    exp *= (QUERY_FLAG(ob, FLAG_CAST_SPELL) && has_ability(ob)) ? (40+MIN(ob->stats.maxsp, 80))/40 : 1;
    exp *= (80.0/(70.0+ob->stats.wc))*(80.0/(70.0+ob->stats.ac))*(50.0+ob->stats.dam)/50.0;
    exp *= att_mult*prot_mult*spec_mult;
/*  exp *= 2.0/(2.0-(MIN(FABS(ob->speed), 0.95)));*/
    exp *= 2.0/(2.0-FABS(ob->speed));
    exp *= (20.0+ob->stats.Con)/20.0;
    if (QUERY_FLAG(ob, FLAG_STAND_STILL))
        exp /= 2;

    return (sint64)exp;
}

/**
 * Checks whether object has innate abilities (spell/spellbook in inventory).
 * @return
 * 1 if monster has any innate abilities, 0 else
 */
int has_ability(const object *ob) {
    return object_find_by_type(ob, SPELL) != NULL || object_find_by_type(ob, SPELLBOOK) != NULL;
}

/**
 * This loads the experience table from the exp_table
 * file.  This tends to exit on any errors, since it
 * populates the table as it goes along, so if there
 * are errors, the table is likely in an inconsistent
 * state.
 *
 * @note
 * will call exit() if file is invalid or not found.
 */
void init_experience(void) {
    char buf[MAX_BUF], *cp;
    int lastlevel = 0;
    sint64 lastexp = -1, tmpexp;
    FILE *fp;

    snprintf(buf, sizeof(buf), "%s/exp_table", settings.confdir);

    if ((fp = fopen(buf, "r")) == NULL) {
        LOG(llevError, "Fatal error: could not open experience table (%s)\n", buf);
        exit(1);
    }
    while (fgets(buf, MAX_BUF-1, fp) != NULL) {
        if (buf[0] == '#')
            continue;

        /* eliminate newline */
        if ((cp = strrchr(buf, '\n')) != NULL)
            *cp = '\0';

        /* Skip over empty lines */
        if (buf[0] == 0)
            continue;
        cp = buf;
        while (isspace(*cp) && *cp != 0)
            cp++;
        if (!strncasecmp(cp, "max_level", 9)) {
            if (settings.max_level) {
                LOG(llevDebug, "Got more than one max_level value from exp_table file?\n");
                free(levels);
            }
            settings.max_level = atoi(cp+9);
            if (!settings.max_level) {
                LOG(llevDebug, "Got invalid max_level from exp_table file? %s\n", buf);
            } else {
                levels = calloc(settings.max_level+1, sizeof(sint64));
            }
        }
        while (isdigit(*cp) && *cp != 0) {
            if (!settings.max_level) {
                LOG(llevError, "max_level is not set in exp_table file.  Did you remember to update it?\n");
                exit(1);
            }

            tmpexp = atoll(cp);
            /* Do some sanity checking - if value is bogus, just exit because
             * the table otherwise is probably in an inconsistent state
             */
            if (tmpexp <= lastexp) {
                LOG(llevError, "Experience for level %d is lower than previous level (%"FMT64" <= %"FMT64")\n", lastlevel+1, tmpexp, lastexp);
                exit(1);
            }
            lastlevel++;
            if (lastlevel > settings.max_level) {
                LOG(llevError, "Too many levels specified in table (%d > %d)\n", lastlevel, settings.max_level);
                exit(1);
            }
            levels[lastlevel] = tmpexp;
            lastexp = tmpexp;
            /* First, skip over the number we just processed. Then skip over
             * any spaces, commas, etc.
             */
            while (isdigit(*cp) && *cp != 0)
                cp++;
            while (!isdigit(*cp) && *cp != 0)
                cp++;
        }
    }
    fclose(fp);
    if (settings.max_level == 0 || lastlevel != settings.max_level) {
        LOG(llevError, "Fatal: exp_table does not have any level definition or not %d as defined, found %d.\n", settings.max_level, lastlevel);
        exit(1);
    }
    if (lastlevel != settings.max_level && lastlevel != 0) {
        LOG(llevError, "Warning: exp_table does not have %d entries (%d)\n", settings.max_level, lastlevel);
        exit(1);
    }
}

/**
 * Dump the experience table, then calls exit() - useful in terms of debugging to make sure the
 * format of the exp_table is correct.
 */
void dump_experience(void) {
    int i;

    for (i = 1; i <= settings.max_level; i++) {
        fprintf(logfile, "%4d %20"FMT64"\n", i, levels[i]);
    }
    exit(0);
}

/**
 * Frees experience-related memory.
 */
void free_experience(void) {
    FREE_AND_CLEAR(levels);
}
