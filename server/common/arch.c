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
 * @file arch.c
 * All archetype-related functions.
 * @note
 * The naming of these functions is really poor - they are all
 * pretty much named '.._arch_...', but they may more may not
 * return archetypes.  Some make the arch_to_object call, and thus
 * return an object.  Perhaps those should be called 'archob' functions
 * to denote they return an object derived from the archetype.
 * MSW 2003-04-29
 * @todo
 * make the functions use the same order for parameters (type first, then name, or the opposite).
 */

#include <global.h>
#include <loader.h>

/** If set, does a little timing on the archetype load. */
#define TIME_ARCH_LOAD 0

static void add_arch(archetype *at);

static archetype *arch_table[ARCHTABLE];
static int arch_cmp = 0;  /**< How many strcmp's */
static int arch_search = 0; /**< How many searches */
int arch_init;  /**< True if doing arch initialization */

static void load_archetypes(void);

/**
 * This function retrieves an archetype given the name that appears
 * during the game (for example, "writing pen" instead of "stylus").
 * It does not use the hashtable system, but browse the whole archlist each time.
 * I suggest not to use it unless you really need it because of performance issue.
 * It is currently used by scripting extensions (create-object).
 * Params:
 * @param name
 * the name we're searching for (ex: "writing pen")
 * @return
 * the archetype found or NULL if nothing was found
 */
archetype *find_archetype_by_object_name(const char *name) {
    archetype *at;
    const char *tmp;

    if (name == NULL)
        return (archetype *)NULL;
    tmp = add_string(name);
    for (at = first_archetype; at != NULL; at = at->next) {
        if (at->clone.name == tmp) {
            free_string(tmp);
            return at;
        }
    }
    free_string(tmp);
    return NULL;
}

/**
 * This function retrieves an archetype by type and name that appears during
 * the game. It is basically the same as find_archetype_by_object_name()
 * except that it considers only items of the given type.
 * @param type
 * item type we're searching
 * @param name
 * the name we're searching for (ex: "writing pen")
 */
archetype *find_archetype_by_object_type_name(int type, const char *name) {
    archetype *at;

    if (name == NULL)
        return NULL;

    for (at = first_archetype; at != NULL; at = at->next) {
        if (at->clone.type == type && strcmp(at->clone.name, name) == 0)
            return at;
    }

    return NULL;
}

/**
 * Retrieves an archetype by skill name and type.
 * This is a lot like the other get_archetype_ functions, with different options.
 * @param skill
 * skill to search for. Must not be NULL.
 * @param type
 * item type to search for. -1 means that it doesn't matter.
 * @return
 * matching archetype, or NULL if none found.
 */
archetype *get_archetype_by_skill_name(const char *skill, int type) {
    archetype *at;

    if (skill == NULL)
        return NULL;

    for (at = first_archetype; at != NULL; at = at->next) {
        if (((type == -1) || (type == at->clone.type))
        && (at->clone.skill) && (!strcmp(at->clone.skill, skill)))
            return at;
    }
    return NULL;
}

/**
 * Retrieves an archetype by type and subtype.
 * Similiar to other get_archetype_ functions. This returns the first archetype
 * that matches both the type and subtype.  type and subtype
 * can be -1 to say ignore, but in this case, the match it does
 * may not be very useful.  This function is most useful when
 * subtypes are known to be unique for a particular type
 * (eg, skills)
 * @param type
 * object type to search for. -1 means any
 * @param subtype
 * object subtype to search for. -1 means any
 * @return
 * matching archetype, or NULL if none found.
 */
archetype *get_archetype_by_type_subtype(int type, int subtype) {
    archetype *at;

    for (at = first_archetype; at != NULL; at = at->next) {
        if (((type == -1) || (type == at->clone.type))
        && (subtype == -1 || subtype == at->clone.subtype))
            return at;
    }
    return NULL;
}

/**
 * Creates an object given the name that appears during the game
 * (for example, "writing pen" instead of "stylus").
 * @param name
 * the name we're searching for (ex: "writing pen"), must not be null
 * @return
 * a corresponding object if found; a singularity object if not found.
 * @note
 * Note by MSW - it appears that it takes the full name and keeps
 * shortening it until it finds a match.  I re-wrote this so that it
 * doesn't malloc it each time - not that this function is used much,
 * but it otherwise had a big memory leak.
 */
object *create_archetype_by_object_name(const char *name) {
    archetype *at;
    char tmpname[MAX_BUF];
    size_t i;

    strncpy(tmpname, name, MAX_BUF-1);
    tmpname[MAX_BUF-1] = 0;
    for (i = strlen(tmpname); i > 0; i--) {
        tmpname[i] = 0;
        at = find_archetype_by_object_name(tmpname);
        if (at != NULL) {
            return arch_to_object(at);
        }
    }
    return create_singularity(name);
}

/**
 * Initialises the internal linked list of archetypes (read from file).
 * Then the global ::empty_archetype pointer is initialised.
 * Can be called multiple times, will just return.
 */
void init_archetypes(void) {
    if (first_archetype != NULL) /* Only do this once */
        return;
    arch_init = 1;
    load_archetypes();
    arch_init = 0;
    empty_archetype = find_archetype("empty_archetype");
    /*  init_blocksview();*/
}

/**
 * Stores debug-information about how efficient the hashtable
 * used for archetypes has been in the static errmsg array.
 */
void arch_info(object *op) {
    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DEBUG,
        "%d searches and %d strcmp()'s",
        arch_search, arch_cmp);
}

/**
 * Initialise the hashtable used by the archetypes.
 */
void clear_archetable(void) {
    memset((void *)arch_table, 0, ARCHTABLE*sizeof(archetype *));
}

/**
 * An alternative way to init the hashtable which is slower, but _works_...
 */
static void init_archetable(void) {
    archetype *at;

    LOG(llevDebug, " Setting up archetable...\n");
    for (at = first_archetype; at != NULL; at = (at->more == NULL) ? at->next : at->more) {
        if (at->name == NULL) {
            LOG(llevError, "archetype without name? %s\n", at->clone.name ? at->clone.name : "(no clone name)");
            abort();
        }
        add_arch(at);
    }
    LOG(llevDebug, "done\n");
}

/**
 * Dumps an archetype to buffer.
 *
 * @param at
 * archetype to dump. Must not be NULL.
 * @param sb
 * buffer that will contain dumped information.
 */
void dump_arch(archetype *at, StringBuffer *sb) {
    object_dump(&at->clone, sb);
}

/**
 * Dumps _all_ archetypes to debug-level output.
 * If you run crossfire with debug, and enter DM-mode, you can trigger
 * this with the "dumpallarchetypes" command.
 */
void dump_all_archetypes(void) {
    archetype *at;

    for (at = first_archetype; at != NULL; at = (at->more == NULL) ? at->next : at->more) {
        StringBuffer *sb;
        char *diff;

        sb = stringbuffer_new();
        dump_arch(at, sb);
        diff = stringbuffer_finish(sb);
        LOG(llevDebug, "%s\n", diff);
        free(diff);
    }
}

/**
 * Frees archetype.
 *
 * @param at
 * archetype to free. Pointer becomes invalid after the call.
 */
void free_arch(archetype *at) {
    if (at->name)
        free_string(at->name);
    if (at->clone.name)
        free_string(at->clone.name);
    if (at->clone.name_pl)
        free_string(at->clone.name_pl);
    if (at->clone.title)
        free_string(at->clone.title);
    if (at->clone.race)
        free_string(at->clone.race);
    if (at->clone.slaying)
        free_string(at->clone.slaying);
    if (at->clone.msg)
        free_string(at->clone.msg);
    free(at->clone.discrete_damage);
    object_free_key_values(&at->clone);
    free(at);
}

/**
 * Frees all memory allocated to archetypes.
 * After calling this, it's possible to call again init_archetypes() to reload data.
 */
void free_all_archs(void) {
    archetype *at, *next;
    int i = 0;

    for (at = first_archetype; at != NULL; at = next) {
        if (at->more)
            next = at->more;
        else
            next = at->next;
        free_arch(at);
        i++;
    }
    first_archetype = NULL;
    /* Reset the hashtable */
    clear_archetable();
    LOG(llevDebug, "Freed %d archetypes\n", i);
}

/**
 * Allocates, initialises and returns the pointer to an archetype structure.
 * @return
 * new archetype structure, will never be NULL.
 * @note
 * this function will call fatal() if it can't allocate memory.
 */
archetype *get_archetype_struct(void) {
    archetype *new;

    new = (archetype *)CALLOC(1, sizeof(archetype));
    if (new == NULL)
        fatal(OUT_OF_MEMORY);
    new->next = NULL;
    new->name = NULL;
    new->clone.other_arch = NULL;
    new->clone.name = NULL;
    new->clone.name_pl = NULL;
    new->clone.title = NULL;
    new->clone.race = NULL;
    new->clone.slaying = NULL;
    new->clone.msg = NULL;
    object_clear(&new->clone);  /* to initial state other also */
    CLEAR_FLAG((&new->clone), FLAG_FREED); /* This shouldn't matter, since object_copy() */
    SET_FLAG((&new->clone), FLAG_REMOVED); /* doesn't copy these flags... */
    new->head = NULL;
    new->more = NULL;
    new->clone.arch = new;
    return new;
}

/**
 * Reads/parses the archetype-file, and copies into a linked list
 * of archetype-structures.
 * Called through load_archetypes()
 *
 * Will discard object in archetypes, those are handled by second_arch_pass().
 *
 * @param fp
 * opened file descriptor which will be used to read the archetypes.
 */
static void first_arch_pass(FILE *fp) {
    archetype *at, *head = NULL, *last_more = NULL;
    int i, first = LO_NEWFILE;

    at = get_archetype_struct();
    first_archetype = at;

    while ((i = load_object(fp, &at->clone, first, 0))) {
        first = 0;
        at->clone.speed_left = (float)(-0.1);

        switch (i) {
        case LL_NORMAL: /* A new archetype, just link it with the previous */
            if (last_more != NULL)
                last_more->next = at;
            if (head != NULL)
                head->next = at;
            head = last_more = at;
            at->tail_x = 0;
            at->tail_y = 0;
            break;

        case LL_MORE: /* Another part of the previous archetype, link it correctly */
            at->head = head;
            at->clone.head = &head->clone;
            if (last_more != NULL) {
                last_more->more = at;
                last_more->clone.more = &at->clone;
            }
            last_more = at;

            /* Set FLAG_MONSTER throughout parts if head has it */
            if (QUERY_FLAG(&head->clone, FLAG_MONSTER)) {
                SET_FLAG(&at->clone, FLAG_MONSTER);
            }

            /* If this multipart image is still composed of individual small
             * images, don't set the tail_.. values.  We can't use them anyways,
             * and setting these to zero makes the map sending to the client much
             * easier as just looking at the head, we know what to do.
             */
            if (at->clone.face != head->clone.face) {
                head->tail_x = 0;
                head->tail_y = 0;
            } else {
                if (at->clone.x > head->tail_x)
                    head->tail_x = at->clone.x;
                if (at->clone.y > head->tail_y)
                    head->tail_y = at->clone.y;
            }
            break;
        }

        at = get_archetype_struct();
    }
    at->clone.arch = NULL; /* arch is checked for temporary archetypes if not NULL. */
    free(at);
}

/**
 * Reads the archetype file once more, and links all pointers between
 * archetypes and treasure lists. Must be called after first_arch_pass().
 *
 * This also handles putting items in inventory when defined in archetype.
 *
 * @param fp
 * file fron which to read. Won't be rewinded.
 */
static void second_arch_pass(FILE *fp) {
    char buf[MAX_BUF], *variable = buf, *argument, *cp;
    archetype *at = NULL, *other;
    object *inv;

    while (fgets(buf, MAX_BUF, fp) != NULL) {
        if (*buf == '#')
            continue;
        if ((argument = strchr(buf, ' ')) != NULL) {
            *argument = '\0', argument++;
            cp = argument+strlen(argument)-1;
            while (isspace(*cp)) {
                *cp = '\0';
                cp--;
            }
        }
        if (!strcmp("Object", variable)) {
            if ((at = find_archetype(argument)) == NULL)
            {
                LOG(llevError, "Fatal: failed to find arch %s in second_arch_pass\n", argument);
                fatal(ARCHETYPE_ISSUE);
            }
        } else if (!strcmp("other_arch", variable)) {
            if (at != NULL && at->clone.other_arch == NULL) {
                if ((other = find_archetype(argument)) == NULL)
                    LOG(llevError, "Warning: failed to find other_arch %s\n", argument);
                else if (at != NULL)
                    at->clone.other_arch = other;
            }
        } else if (!strcmp("randomitems", variable)) {
            if (at != NULL) {
                treasurelist *tl = find_treasurelist(argument);
                if (tl == NULL)
                    LOG(llevError, "Failed to link treasure to arch (%s): %s\n", at->name, argument);
                else
                    at->clone.randomitems = tl;
            }
        } else if (!strcmp("arch", variable)) {
            inv = create_archetype(argument);
            load_object(fp, inv, LO_LINEMODE, 0);
            if (at) {
                object_insert_in_ob(inv, &at->clone);
                /*LOG(llevDebug, "Put %s in %s\n", inv->name, at->clone.name);*/
            } else {
                LOG(llevError, "Got an arch %s not inside an Object.\n", argument);
                object_free_drop_inventory(inv);
            }
        }
    }
}

/**
 * Check all generators have the other_arch set or something in inventory.
 */
void check_generators(void) {
    const archetype *at;
    int abort = 0;

    for (at = first_archetype; at != NULL; at = at->next) {
        if (!QUERY_FLAG(&at->clone, FLAG_GENERATOR))
            continue;

        if (!QUERY_FLAG(&at->clone, FLAG_CONTENT_ON_GEN) && at->clone.other_arch == NULL) {
            LOG(llevError, "Fatal: %s is generator without content_on_gen but lacks other_arch.\n", at->name);
            abort = 1;
            continue;
        }
        if (QUERY_FLAG(&at->clone, FLAG_CONTENT_ON_GEN) && at->clone.inv == NULL) {
            LOG(llevError, "Fatal: %s is generator with content_on_gen but lacks inventory.\n", at->name);
            abort = 1;
            continue;
        }
    }

    if (abort)
        fatal(SEE_LAST_ERROR);
}

/**
 * This checks all summonable items for move_type and other things.
 * Will call fatal() if an error is found.
 */
void check_summoned(void) {
    const archetype *at;

    for (at = first_archetype; at != NULL; at = at->next) {
        if (at->clone.type == SPELL && at->clone.subtype == SP_SUMMON_GOLEM && at->clone.other_arch) {
            if (at->clone.other_arch->clone.move_type == 0) {
                LOG(llevError, "Summonable archetype %s [%s] has no move_type defined!\n", at->clone.other_arch->name, at->clone.other_arch->clone.name);
                fatal(SEE_LAST_ERROR);
            }
        }
    }
}

/**
 * This ensures all spells have a skill defined, calling fatal() if any error was found.
 */
static void check_spells(void) {
    int abort = 0;
    const archetype *at;

    for (at = first_archetype; at != NULL; at = at->next) {
        if (at->clone.type == SPELL && at->clone.skill == NULL) {
            LOG(llevError, "Spell archetype %s [%s] has no skill defined!\n", at->name, at->clone.name);
            abort = 1;
        }
    }
    if (abort)
        fatal(SEE_LAST_ERROR);
}

/**
 * Loads all archetypes and treasures.
 * First initialises the archtype hash-table (init_archetable()).
 * Reads and parses the archetype file (with the first and second-pass
 * functions).
 * Then initialises treasures by calling load_treasures().
 */
static void load_archetypes(void) {
    FILE *fp;
    char filename[MAX_BUF];
#if TIME_ARCH_LOAD
    struct timeval tv1, tv2;
#endif

    snprintf(filename, sizeof(filename), "%s/%s", settings.datadir, settings.archetypes);
    LOG(llevDebug, "Reading archetypes from %s...\n", filename);
    if ((fp = fopen(filename, "r")) == NULL) {
        LOG(llevError, " Can't open archetype file.\n");
        return;
    }
    clear_archetable();
    LOG(llevDebug, " arch-pass 1...\n");
#if TIME_ARCH_LOAD
    GETTIMEOFDAY(&tv1);
#endif
    first_arch_pass(fp);
#if TIME_ARCH_LOAD
    {
        int sec, usec;

        GETTIMEOFDAY(&tv2);
        sec = tv2.tv_sec-tv1.tv_sec;
        usec = tv2.tv_usec-tv1.tv_usec;
        if (usec < 0) {
            usec += 1000000;
            sec--;
        }
        LOG(llevDebug, "Load took %d.%06d seconds\n", sec, usec);
    }
#endif

    LOG(llevDebug, " done\n");
    init_archetable();
    warn_archetypes = 1;

    rewind(fp);

    LOG(llevDebug, " loading treasure...\n");
    load_treasures();
    LOG(llevDebug, " done\n");
    LOG(llevDebug, "arch-pass 2...\n");
    second_arch_pass(fp);
    LOG(llevDebug, " done\n");
    check_generators();
    check_spells();
    check_summoned();
    fclose(fp);
    LOG(llevDebug, " done\n");
}

/**
 * Creates and returns a new object which is a copy of the given archetype.
 * This function returns NULL if given a NULL pointer, else an object.
 * @param at
 * archetype from which to get an object.
 * @return
 * object of specified type.
 * @note
 * object_new() will either allocate memory or call fatal(), so returned value
 * is never NULL.
 */
object *arch_to_object(archetype *at) {
    object *op;

    if (at == NULL) {
        if (warn_archetypes)
            LOG(llevError, "Couldn't find archetype.\n");
        return NULL;
    }
    op = object_new();
    object_copy_with_inv(&at->clone, op);
    op->arch = at;
    return op;
}

/**
 * Creates a dummy object. This function is called by get_archetype()
 * if it fails to find the appropriate archetype.
 * Thus get_archetype() will be guaranteed to always return
 * an object, and never NULL.
 * @param name
 * name to give the dummy object.
 * @return
 * object of specified name. It fill have the ::FLAG_NO_PICK flag set.
 * @note
 * object_new() will either allocate memory or call fatal(), so returned value
 * is never NULL.
 */
object *create_singularity(const char *name) {
    object *op;
    char buf[MAX_BUF];

    snprintf(buf, sizeof(buf), "%s (%s)", ARCH_SINGULARITY, name);
    op = object_new();
    op->name = add_string(buf);
    op->name_pl = add_string(buf);
    SET_FLAG(op, FLAG_NO_PICK);
    return op;
}

/**
 * Finds which archetype matches the given name, and returns a new
 * object containing a copy of the archetype.
 * @param name
 * archetype name
 * @return
 * object of specified archetype, or a singularity. Will never be NULL.
 * @todo
 * replace with object_create_arch() which is multi-part aware.
 */
object *create_archetype(const char *name) {
    archetype *at;

    at = find_archetype(name);
    if (at == NULL)
        return create_singularity(name);
    return arch_to_object(at);
}

/**
 * Hash-function used by the arch-hashtable.
 * @param str
 * archetype name
 * @param tablesize
 * size of the hash table
 * @return
 * hash of the archetype name
 */
static unsigned long
hasharch(const char *str, int tablesize) {
    unsigned long hash = 0;
    int i = 0;
    const char *p;

    /* use the one-at-a-time hash function, which supposedly is
     * better than the djb2-like one used by perl5.005, but
     * certainly is better then the bug used here before.
     * see http://burtleburtle.net/bob/hash/doobs.html
     */
    for (p = str; i < MAXSTRING && *p; p++, i++) {
        hash += *p;
        hash += hash<<10;
        hash ^= hash>>6;
    }
    hash += hash<<3;
    hash ^= hash>>11;
    hash += hash<<15;
    return hash%tablesize;
}

/**
 * Finds, using the hashtable, which archetype matches the given name. Will not LOG() if not found.
 * @return
 * pointer to the found archetype, otherwise NULL.
 * @see find_archetype()
 */
archetype *try_find_archetype(const char *name) {
    archetype *at;
    unsigned long index;

    if (name == NULL)
        return (archetype *)NULL;

    index = hasharch(name, ARCHTABLE);
    arch_search++;
    for (;;) {
        at = arch_table[index];
        if (at == NULL) {
            return NULL;
        }
        arch_cmp++;
        if (!strcmp(at->name, name))
            return at;
        if (++index >= ARCHTABLE)
            index = 0;
    }
}

/**
 * Finds, using the hashtable, which archetype matches the given name. Will LOG() if not found.
 * @return
 * pointer to the found archetype, otherwise NULL.
 * @see try_find_archetype
 * @todo replace by try_find_archetype() when suitable and trash ::warn_archetypes.
 */
archetype *find_archetype(const char *name) {
    archetype *at;

    if (name == NULL)
        return (archetype *)NULL;
    at = try_find_archetype(name);
    if (at == NULL && warn_archetypes)
        LOG(llevError, "Couldn't find archetype %s\n", name);
    return at;
}

/**
 * Adds an archetype to the hashtable.
 * Will call fatal() if archetype table is too small to contain archetypes.
 */
static void add_arch(archetype *at) {
    unsigned long index = hasharch(at->name, ARCHTABLE), org_index = index;

    for (;;) {
        if (arch_table[index] == NULL) {
            arch_table[index] = at;
            return;
        }
        if (++index == ARCHTABLE)
            index = 0;
        if (index == org_index)
            fatal(ARCHTABLE_TOO_SMALL);
    }
}

/**
 * Create a full object using the given archetype.
 * This instanciate not only the archetype but also
 * all linked archetypes in case of multisquare archetype.
 * @param at
 * archetype to instanciate. Must not be NULL.
 * @return
 * pointer to head of instance.
 * @note
 * will never return NULL.
 */
object *object_create_arch(archetype *at) {
    object *op, *prev = NULL, *head = NULL;

    while (at) {
        op = arch_to_object(at);
        op->x = at->clone.x;
        op->y = at->clone.y;
        if (head)
            op->head = head, prev->more = op;
        if (!head)
            head = op;
        prev = op;
        at = at->more;
    }
    return (head);
}

/**
 * Checks if the specified type is a valid one for a Crossfire object.
 * It would be better (not sure how) if it could automatically
 * extract this somehow from define.h .  Otherwise, it will complain
 * if a new type is added, but if a type is removed and this
 * function not updated, that will no be noticed.
 *
 * @param type value to check.
 * @return 1 if the type is valid, 0 else.
 */
int is_type_valid(uint8 type) {
    if (type >= OBJECT_TYPE_MAX)
        return 0;

    if (type == 11 || type == 12)
        return 0;
    if (type == 19 || type == 25 || type == 35 || type == 38 || type == 44 || type == 47)
        return 0;
    if (type == 61 || type == 63 || type == 76 || type == 78 || type == 81)
        return 0;
    if (type == 81 || type == 84 || type == 86 || type == 88 || type == 89)
        return 0;
    if (type == 96 || type == 97 || type == 107 || type == 108 || type == 110)
        return 0;
    if (type >= 117 && type <= 120)
        return 0;
    if (type >= 125 && type <= 129)
        return 0;
    if (type >= 131 && type <= 137)
        return 0;
    if (type >= 140 && type <= 149)
        return 0;
    if (type >= 151 && type <= 153)
        return 0;

    return 1;
}

/*** end of arch.c ***/
