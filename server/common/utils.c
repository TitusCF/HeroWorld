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
 * @file utils.c
 * General convenience functions for crossfire.
 *
 * The random functions here take luck into account when rolling random
 * dice or numbers.  This function has less of an impact the larger the
 * difference becomes in the random numbers.  IE, the effect is lessened
 * on a 1-1000 roll, vs a 1-6 roll.  This can be used by crafty programmers,
 * to specifically disable luck in certain rolls, simply by making the
 * numbers larger (ie, 1d1000 > 500 vs 1d6 > 3)
 */

#include <stdlib.h>
#include <global.h>

/**
 * Roll a random number between min and max.  Uses op to determine luck,
 * and if goodbad is non-zero, luck increases the roll, if zero, it decreases.
 *
 * Generally, op should be the player/caster/hitter requesting the roll,
 * not the recipient (ie, the poor slob getting hit). [garbled 20010916]
 */
int random_roll(int min, int max, const object *op, int goodbad) {
    int omin, diff, luck, base, ran;

    omin = min;
    diff = max-min+1;
    ((diff > 2) ? (base = 20) : (base = 50)); /* d2 and d3 are corner cases */

    if (max < 1 || diff < 1) {
        LOG(llevError, "Calling random_roll with min=%d max=%d\n", min, max);
        return(min); /* avoids a float exception */
    }

    ran = RANDOM();

    if (op->type != PLAYER)
        return((ran%diff)+min);

    luck = op->stats.luck;
    if (RANDOM()%base < MIN(10, abs(luck))) {
        /* we have a winner */
        ((luck > 0) ? (luck = 1) : (luck = -1));
        diff -= luck;
        if (diff < 1)
            return(omin); /*check again*/
        ((goodbad) ? (min += luck) : (diff));

        return(MAX(omin, MIN(max, (ran%diff)+min)));
    }
    return((ran%diff)+min);
}

/**
 * This is a 64 bit version of random_roll() above.  This is needed
 * for exp loss calculations for players changing religions.
 */
sint64 random_roll64(sint64 min, sint64 max, const object *op, int goodbad) {
    sint64 omin, diff, luck, ran;
    int base;

    omin = min;
    diff = max-min+1;
    ((diff > 2) ? (base = 20) : (base = 50)); /* d2 and d3 are corner cases */

    if (max < 1 || diff < 1) {
        LOG(llevError, "Calling random_roll with min=%"FMT64" max=%"FMT64"\n", min, max);
        return(min); /* avoids a float exception */
    }

    /* Don't know of a portable call to get 64 bit random values.
     * So make a call to get two 32 bit random numbers, and just to
     * a little byteshifting.  Do make sure the first one is only
     * 32 bit, so we don't get skewed results
     */
    ran = (RANDOM()&0xffffffff)|((sint64)RANDOM()<<32);

    if (op->type != PLAYER)
        return((ran%diff)+min);

    luck = op->stats.luck;
    if (RANDOM()%base < MIN(10, abs(luck))) {
        /* we have a winner */
        ((luck > 0) ? (luck = 1) : (luck = -1));
        diff -= luck;
        if (diff < 1)
            return (omin); /*check again*/
        ((goodbad) ? (min += luck) : (diff));

        return (MAX(omin, MIN(max, (ran%diff)+min)));
    }
    return ((ran%diff)+min);
}

/**
 * Roll a number of dice (2d3, 4d6).  Uses op to determine luck,
 * If goodbad is non-zero, luck increases the roll, if zero, it decreases.
 * Generally, op should be the player/caster/hitter requesting the roll,
 * not the recipient (ie, the poor slob getting hit).
 * The args are num D size (ie 4d6)  [garbled 20010916]
 */
int die_roll(int num, int size, const object *op, int goodbad) {
    int min, diff, luck, total, i, gotlucky, base, ran;

    diff = size;
    min = 1;
    luck = total = gotlucky = 0;
    ((diff > 2) ? (base = 20) : (base = 50)); /* d2 and d3 are corner cases */
    if (size < 2 || diff < 1) {
        LOG(llevError, "Calling die_roll with num=%d size=%d\n", num, size);
        return(num); /* avoids a float exception */
    }

    if (op->type == PLAYER)
        luck = op->stats.luck;

    for (i = 0; i < num; i++) {
        if (RANDOM()%base < MIN(10, abs(luck)) && !gotlucky) {
            /* we have a winner */
            gotlucky++;
            ((luck > 0) ? (luck = 1) : (luck = -1));
            diff -= luck;
            if (diff < 1)
                return(num); /*check again*/
            ((goodbad) ? (min += luck) : (diff));
            ran = RANDOM();
            total += MAX(1, MIN(size, (ran%diff)+min));
        } else {
            total += RANDOM()%size+1;
        }
    }
    return(total);
}

/**
 * Returns a number between min and max.
 *
 * It is suggested one use these functions rather than RANDOM()%, as it
 * would appear that a number of off-by-one-errors exist due to improper
 * use of %.  This should also prevent SIGFPE.
 */
int rndm(int min, int max) {
    int diff;

    diff = max-min+1;
    if (max < 1 || diff < 1)
        return (min);

    return (RANDOM()%diff+min);
}

/**
 * Decay and destroy persihable items in a map
 */
void decay_objects(mapstruct *m) {
    int x, y, destroy;

    if (m->unique)
        return;

    for (x = 0; x < MAP_WIDTH(m); x++)
        for (y = 0; y < MAP_HEIGHT(m); y++)
            FOR_MAP_PREPARE(m, x, y, op) {
                destroy = 0;
                if (QUERY_FLAG(op, FLAG_IS_FLOOR) && QUERY_FLAG(op, FLAG_UNIQUE))
                    break;
                if (QUERY_FLAG(op, FLAG_IS_FLOOR)
                || QUERY_FLAG(op, FLAG_OBJ_ORIGINAL)
                || QUERY_FLAG(op, FLAG_UNIQUE)
                || QUERY_FLAG(op, FLAG_OVERLAY_FLOOR)
                || QUERY_FLAG(op, FLAG_UNPAID)
                || IS_LIVE(op))
                    continue;
                if (op->head)
                    /* Don't try to remove a non head part of a multipart object, object_remove() would abort(). */
                    continue;
                /* otherwise, we decay and destroy */
                if (IS_WEAPON(op)) {
                    op->stats.dam--;
                    if (op->stats.dam < 0)
                        destroy = 1;
                } else if (IS_ARMOR(op)
                || IS_SHIELD(op)
                || op->type == GIRDLE
                || op->type == GLOVES
                || op->type == CLOAK) {
                    op->stats.ac--;
                    if (op->stats.ac < 0)
                        destroy = 1;
                } else if (op->type == FOOD) {
                    op->stats.food -= rndm(5, 20);
                    if (op->stats.food < 0)
                        destroy = 1;
                } else {
                    if (op->material&M_PAPER
                    || op->material&M_LEATHER
                    || op->material&M_WOOD
                    || op->material&M_ORGANIC
                    || op->material&M_CLOTH
                    || op->material&M_LIQUID)
                        destroy = 1;
                    if (op->material&M_IRON && rndm(1, 5) == 1)
                        destroy = 1;
                    if (op->material&M_GLASS && rndm(1, 2) == 1)
                        destroy = 1;
                    if ((op->material&M_STONE || op->material&M_ADAMANT) && rndm(1, 10) == 1)
                        destroy = 1;
                    if ((op->material&M_SOFT_METAL || op->material&M_BONE)
                    && rndm(1, 3) == 1)
                        destroy = 1;
                    if (op->material&M_ICE && rndm(0, 100) > 70)
                        destroy = 1;
                }
                /* adjust overall chance below */
                if (destroy && rndm(0, 1)) {
                    object_remove(op);
                    object_free_drop_inventory(op);
                }
            } FOR_MAP_FINISH();
}

/**
 * Convert materialname to materialtype_t
 *
 * @todo
 * why use a break?
 */
materialtype_t *name_to_material(const char *name) {
    materialtype_t *mt, *nmt;

    mt = NULL;
    for (nmt = materialt; nmt != NULL && nmt->next != NULL; nmt = nmt->next) {
        if (strcmp(name, nmt->name) == 0) {
            mt = nmt;
            break;
        }
    }
    return mt;
}

/**
 * When doing transmutation of objects, we have to recheck the resistances,
 * as some that did not apply previously, may apply now.
 *
 * Only works on armors.
 */
void transmute_materialname(object *op, const object *change) {
    materialtype_t *mt;
    int j;

    if (op->materialname == NULL)
        return;

    if (change->materialname != NULL
    && strcmp(op->materialname, change->materialname))
        return;

    if (!(IS_ARMOR(op) || IS_SHIELD(op) || op->type == GIRDLE || op->type == GLOVES || op->type == CLOAK))
        return;

    mt = name_to_material(op->materialname);
    if (!mt) {
        LOG(llevError, "archetype '%s>%s' uses nonexistent material '%s'\n", op->arch->name, op->name, op->materialname);
        return;
    }

    for (j = 0; j < NROFATTACKS; j++)
        if (op->resist[j] == 0 && change->resist[j] != 0) {
            op->resist[j] += mt->mod[j];
            if (op->resist[j] > 100)
                op->resist[j] = 100;
            if (op->resist[j] < -100)
                op->resist[j] = -100;
        }
}

/**
 * Set the material name and type for an item, if not set.
 * @param op item to set the material for.
 */
void set_materialname(object *op) {
    materialtype_t *mt;

    if (op->materialname != NULL)
        return;

    for (mt = materialt; mt != NULL && mt->next != NULL; mt = mt->next) {
        if (op->material&mt->material) {
            break;
        }
    }

    if (mt != NULL) {
        op->materialname = add_string(mt->name);
        return;
    }
}

/**
 * Finds a string in a string.
 *
 * @todo
 * isn't there another function (porting.c?) for that?
 */
const char *strrstr(const char *haystack, const char *needle) {
    const char *lastneedle;

    lastneedle = NULL;
    while ((haystack = strstr(haystack, needle)) != NULL) {
        lastneedle = haystack;
        haystack++;
    }
    return lastneedle;
}

#define EOL_SIZE (sizeof("\n")-1)

/**
 * Removes endline from buffer (modified in place).
 */
void strip_endline(char *buf) {
    if (strlen(buf) < sizeof("\n")) {
        return;
    }
    if (!strcmp(buf+strlen(buf)-EOL_SIZE, "\n"))
        buf[strlen(buf)-EOL_SIZE] = '\0';
}

/**
 * Replace in string src all occurrences of key by replacement. The resulting
 * string is put into result; at most resultsize characters (including the
 * terminating null character) will be written to result.
 */
void replace(const char *src, const char *key, const char *replacement, char *result, size_t resultsize) {
    size_t resultlen;
    size_t keylen;

    /* special case to prevent infinite loop if key==replacement=="" */
    if (strcmp(key, replacement) == 0) {
        snprintf(result, resultsize, "%s", src);
        return;
    }

    keylen = strlen(key);

    resultlen = 0;
    while (*src != '\0' && resultlen+1 < resultsize) {
        if (strncmp(src, key, keylen) == 0) {
            snprintf(result+resultlen, resultsize-resultlen, "%s", replacement);
            resultlen += strlen(result+resultlen);
            src += keylen;
        } else {
            result[resultlen++] = *src++;
        }
    }
    result[resultlen] = '\0';
}

/**
 * Taking a string as an argument, mutate it into a string that looks like a list.
 *
 * A 'list' for the purposes here is a string of items, seperated by commas, except
 * for the last entry, which has an 'and' before it, and a full stop (period) after it.
 *
 * This function will also strip all trailing non alphanumeric characters.
 *
 * It does not insert an oxford comma.
 *
 * @param input
 * string to transform. Will be overwritten. Must be long enough to contain the modified string.
 *
 * @todo
 * use safe string functions.
 */
void make_list_like(char *input) {
    char *p, tmp[MAX_BUF];
    int i;
    if (!input || strlen(input) > MAX_BUF-5)
        return;
    /* bad stuff would happen if we continued here, the -5 is to make space for ' and ' */

    strncpy(tmp, input, MAX_BUF-5);
    /*trim all trailing commas, spaces etc.*/
    for (i = strlen(tmp); !isalnum(tmp[i]) && i >= 0; i--)
        tmp[i] = '\0';
    strcat(tmp, ".");

    p = strrchr(tmp, ',');
    if (p) {
        *p = '\0';
        strcpy(input, tmp);
        p++;
        strcat(input, " and");
        strcat(input, p);
    } else
        strcpy(input, tmp);
    return;
}

/**
 * Returns a random direction (1..8).
 *
 * @return
 * the random direction.
 */
int get_random_dir(void) {
    return rndm(1, 8);
}

/**
 * Returns a random direction (1..8) similar to a given direction.
 *
 * @param dir
 * the exact direction
 * @return
 * the randomized direction
 */
int get_randomized_dir(int dir) {
    return absdir(dir+RANDOM()%3+RANDOM()%3-2);
}

/**
 * Adjusts a given direction by +/-1 towards a destination direction.
 *
 * @param dir
 * the direction to adjust
 * @param destination_dir
 * the destination direction to adjust towards
 * @return
 * the adjusted direction
 */
int adjust_dir(int dir, int destination_dir) {
    int diff;

    diff = (destination_dir-dir)&7;
    if (1 <= diff && diff <= 3)
        dir++;
    else if (5 <= diff && diff <= 7)
        dir--;
    else if (rndm(0, 1) == 0)
        dir++;
    else
        dir--;
    return absdir(dir);
}

/**
 * Replaces any unprintable character in the given buffer with a space.
 *
 * @param buf
 * the buffer to modify
 */
void replace_unprintable_chars(char *buf) {
    char *p;

    for (p = buf; *p != '\0'; p++) {
        if (*p < ' ') {
            *p = ' ';
        }
    }
}

/**
 * Splits a string delimited by passed in sep value into characters into an array of strings.
 *
 * @param str
 * the string to be split; will be modified
 * @param array
 * the string array; will be filled with pointers into str
 * @param array_size
 * the number of elements in array; if <code>str</code> contains more fields
 * excess fields are not split but included into the last element
 * @param sep
 * seperator to use.
 * @return
 * the number of elements found; always less or equal to
 * <code>array_size</code>
 */
size_t split_string(char *str, char *array[], size_t array_size, char sep) {
    char *p;
    size_t pos;

    if (array_size <= 0)
        return 0;

    if (*str == '\0') {
        array[0] = str;
        return 1;
    }

    pos = 0;
    p = str;
    while (pos < array_size) {
        array[pos++] = p;
        while (*p != '\0' && *p != sep)
            p++;
        if (pos >= array_size)
            break;
        if (*p != sep)
            break;
        *p++ = '\0';
    }
    return pos;
}

/**
 * Describe the specified path attenuation.
 * @param attenuation string describing if "Attenued", "Denied", "Repelled".
 * @param value path value to describe.
 * @param buf where to describe, can be NULL.
 * @return buf, newly allocated StringBuffer the caller should free if buf was NULL.
 */
StringBuffer *describe_spellpath_attenuation(const char *attenuation, int value, StringBuffer *buf) {
    if (buf == NULL)
        buf = stringbuffer_new();

    if (value) {
        int i, j = 0;
        stringbuffer_append_printf(buf, "(%s: ", attenuation);
        for (i = 0; i < NRSPELLPATHS; i++)
            if (value&(1<<i)) {
                if (j)
                    stringbuffer_append_string(buf, ", ");
                else
                    j = 1;
                stringbuffer_append_string(buf, spellpathnames[i]);
            }
        stringbuffer_append_string(buf, ")");
    }

    return buf;
}

/**
 * Describe the specified attack type.
 * @param attack string describing the attack ("Clawing", and such).
 * @param value attack type to describe.
 * @param buf where to describe, can be NULL.
 * @return buf, newly allocated StringBuffer the caller should free if buf was NULL.
 */
StringBuffer *describe_attacktype(const char *attack, int value, StringBuffer *buf) {
    if (buf == NULL)
        buf = stringbuffer_new();

    if (value) {
        int i, j = 0;
        stringbuffer_append_printf(buf, "(%s: ", attack);
        for (i = 0; i < NROFATTACKS; i++)
            if (value&(1<<i)) {
                if (j)
                    stringbuffer_append_string(buf, ", ");
                else
                    j = 1;
                stringbuffer_append_string(buf, attacks[i]);
            }
        stringbuffer_append_string(buf, ")");
    }

    return buf;
}
