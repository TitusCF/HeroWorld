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
 * @file region.c
 * Region management.
 *
 * A region is a group of maps. It includes a "parent" region.
 */

#include <global.h>

#ifndef WIN32 /* ---win32 exclude header */
#include <unistd.h>
#endif /* win32 */

static void parse_regions(FILE *fp);
static void assign_region_parents(void);

/**
 * Gets a region by name.
 *
 * Used by the map parsing code.
 *
 * @param region_name
 * name of region.
 * @return
 * @li matching region
 * @li if no match, returns the first region with the 'fallback' property set and LOG()s to debug.
 * @li if it can't find a matching name and a fallback region it LOG()s an info message and returns NULL.
 */
region *get_region_by_name(const char *region_name) {
    region *reg;

    for (reg = first_region; reg != NULL; reg = reg->next)
        if (!strcmp(reg->name, region_name))
            return reg;

    for (reg = first_region; reg != NULL; reg = reg->next) {
        if (reg->fallback) {
            LOG(llevDebug, "region called %s requested, but not found, fallback used.\n", region_name);
            return reg;
        }
    }
    LOG(llevInfo, "Got no region or fallback for region %s.\n", region_name);
    return NULL;
}

/**
 * Gets a region from a map.
 *
 * @param m
 * map we want the region of.
 * @return
 * region.
 *
 * @todo
 * This might need optimising at some point.
 */
region *get_region_by_map(mapstruct *m) {
    return get_region_by_name(get_name_of_region_for_map(m));
}

/**
 * Gets the name of a region for a map.
 *
 * Since we won't assume all maps have a region set properly, we need an
 * explicit check that it is, this is much nicer here than scattered throughout
 * the map code.
 *
 * @param m
 * map
 * @return
 * @li region's name if map has a region
 * @li if no region is set for the map, returns the first region with the 'fallback' property
 * @li if no fallback region, LOG()s an info message and returns "unknown".
 */
const char *get_name_of_region_for_map(const mapstruct  *m) {
    region *reg;

    if (m->region != NULL)
        return m->region->name;
    for (reg = first_region; reg != NULL; reg = reg->next) {
        if (reg->fallback)
            return reg->name;
    }
    LOG(llevInfo, "map %s had no region and I couldn't find a fallback to use.\n", m->name);
    return "unknown";
}

/**
 * Tries to find a region that 'name' corresponds to.
 * It looks, in order, for:
 * @li an exact match to region name (case insensitive)
 * @li an exact match to longname (case insensitive)
 * @li a substring that matches to the longname (eg Kingdom)
 * @li a substring that matches to the region name (eg nav)
 * @li if it can find none of these it returns the first parentless region
 * (there should be only one of these - the top level one)
 * An empty region name returns the top level region
 *
 * @param name
 * region we're searching.
 * @return
 * matching region.
 */
region *get_region_from_string(const char *name) {
    region *reg;
    char *substr;
    char *p;

    if (first_region == NULL) {
        return NULL;
    }

    if (*name == '\0') {
        for (reg = first_region; reg->parent != NULL; reg = reg->parent)
            ;
        return reg;
    }
    p = strchr(name, '\n');
    if (p)
        *p = '\0';
    for (reg = first_region; reg != NULL; reg = reg->next)
        if (!strcasecmp(reg->name, name))
            return reg;

    for (reg = first_region; reg != NULL; reg = reg->next)
        if (reg->longname != NULL) {
            if (!strcasecmp(reg->longname, name))
                return reg;
        }

    substr = NULL;
    for (reg = first_region; reg != NULL; reg = reg->next)
        if (reg->longname != NULL) {
            substr = strstr(reg->longname, name);
            if (substr != NULL)
                return reg;
        }
    for (reg = first_region; reg != NULL; reg = reg->next)
        if (reg->longname != NULL) {
            /*
             * This is not a bug, we want the region that is  most identifiably a discrete
             * area in the game, eg if we have 'scor', we want to return 'scorn' and not
             * 'scornarena', regardless of their order on the list so we only look at those
             * regions with a longname set.
             */
            substr = strstr(reg->name, name);
            if (substr != NULL)
                return reg;
        }
    for (reg = first_region; reg != NULL; reg = reg->next) {
        substr = strstr(reg->name, name);
        if (substr != NULL)
            return reg;
    }
    /* if we are still here, we are going to have to give up, and give the top level region */
    for (reg = first_region; reg->parent != NULL; reg = reg->parent)
        ;
    return reg;
}

/**
 * Checks if a region is a child of another.
 *
 * @param child
 * region we want to test.
 * @param r
 * potential parent.
 * @return
 * @li 1 if child has r as parent somewhere.
 * @li -1 if passed a NULL region
 * @li 0 else
 */
int region_is_child_of_region(const region *child, const region *r) {
    if (r == NULL)
        return -1;
    if (child == NULL)
        return 0;
    if (!strcmp(child->name, r->name))
        return 1;
    else if (child->parent != NULL)
        return region_is_child_of_region(child->parent, r);
    else
        return 0;
}

/**
 * Gets the longname of a region.
 *
 * The longname of a region is not a required field, any given region
 * may want to not set it and use the parent's one instead.
 *
 * @param r
 * region we're searching the longname.
 * @return
 * @li if a longname is set return it.
 * @li if there is a parent, call the function against that
 * @li if all fails, return a obviously wrong string if we can't get a longname, this should
 *   never happen. We also LOG() a debug message.
 */
const char *get_region_longname(const region *r) {
    if (r->longname != NULL)
        return r->longname;
    else if (r->parent != NULL)
        return get_region_longname(r->parent);
    else {
        LOG(llevDebug, "NOTICE region %s has no parent and no longname.\n", r->name);
        return "no name can be found for the current region";
    }
}

/**
 * Gets a message for a region.
 *
 * @param r
 * region. Can't be NULL.
 * @return
 * @li region's message if set
 * @li message of parent else
 * @li obviously wrong message if no parent, and we LOG() a debug message.
 */
const char *get_region_msg(const region *r) {
    if (r->msg != NULL)
        return r->msg;
    else if (r->parent != NULL)
        return get_region_msg(r->parent);
    else {
        LOG(llevDebug, "NOTICE region %s has no parent and no msg.\n", r->name);
        return "no description can be found for the current region";
    }
}

/**
 * Returns an object which is an exit through which the player represented by op should be
 * sent in order to be imprisoned. If there is no suitable place to which an exit can be
 * constructed, then NULL will be returned. The caller is responsible for freeing the object
 * created by this function.
 *
 * @param op
 * Object we want to jail. Must be a player.
 * @return
 * exit to jail, or NULL, in which case a message is LOG()ged .
 */
object *get_jail_exit(object *op) {
    region *reg;
    object *exit;

    if (op->type != PLAYER) {
        LOG(llevError, "region.c: get_jail_exit called against non-player object.\n");
        return NULL;
    }

    reg = get_region_by_map(op->map);
    while (reg != NULL) {
        if (reg->jailmap) {
            exit = object_new();
            EXIT_PATH(exit) = add_string(reg->jailmap);
            /* damned exits reset savebed and remove teleports, so the prisoner can't escape */
            SET_FLAG(exit, FLAG_DAMNED);
            EXIT_X(exit) = reg->jailx;
            EXIT_Y(exit) = reg->jaily;
            return exit;
        } else
            reg = reg->parent;
    }
    LOG(llevDebug, "No suitable jailmap for region %s was found.\n", reg->name);
    return NULL;
}

/**
 * Initialises regions from the regions file.
 *
 * @return
 * @li Returns '0' on success, anything else if failed.
 */
int init_regions(void) {
    FILE *fp;
    char filename[MAX_BUF];

    if (first_region != NULL) /* Only do this once */
        return 0;

    snprintf(filename, sizeof(filename), "%s/%s/%s", settings.datadir, settings.mapdir, settings.regions);
    LOG(llevDebug, "Reading regions from %s...\n", filename);
    if ((fp = fopen(filename, "r")) == NULL) {
        LOG(llevError, "Couldn't read regions file from \"%s\".\n", filename);
        return 1;
    }
    parse_regions(fp);
    assign_region_parents();
    LOG(llevDebug, " done\n");

    fclose(fp);
    return 0;
}

/**
 * Allocates and zeros a region struct, this isn't free()'d anywhere, so might
 * be a memory leak, but it shouldn't matter too much since it isn't called that
 * often....
 *
 * @return
 * initialised region structure.
 *
 * @note
 * will never fail, as a memory allocation error calls fatal().
 *
 * @todo
 * free those pointers someday? :)
 */
region *get_region_struct(void) {
    region *new;

    new = (region *)CALLOC(1, sizeof(region));
    if (new == NULL)
        fatal(OUT_OF_MEMORY);

    memset(new, '\0', sizeof(region));

    return new;
}

/**
 * Reads/parses the region file, and copies into a linked list
 * of region structs.
 *
 * @param fp
 * opened file to read from.
 */
static void parse_regions(FILE *fp) {
    region *new;
    region *reg;

    char buf[HUGE_BUF], msgbuf[HUGE_BUF], *key = NULL, *value, *end;
    int msgpos = 0;

    new = NULL;
    while (fgets(buf, HUGE_BUF-1, fp) != NULL) {
        buf[HUGE_BUF-1] = 0;
        key = buf;
        while (isspace(*key))
            key++;
        if (*key == 0)
            continue;    /* empty line */
        value = strchr(key, ' ');
        if (!value) {
            end = strchr(key, '\n');
            *end = 0;
        } else {
            *value = 0;
            value++;
            /* isspace() includes newline. To avoid crash on empty line further
             * down we must check for it here.
             */
            while (isspace(*value) && *value != '\n')
                value++;
            end = strchr(value, '\n');
        }

        /*
         * This is a bizzare mutated form of the map and archetype parser
         * rolled into one. Key is the field name, value is what it should
         * be set to.
         * We've already done the work to null terminate key,
         * and strip off any leading spaces for both of these.
         * We have not touched the newline at the end of the line -
         * these might be needed for some values. the end pointer
         * points to the first of the newlines.
         * value could be NULL!  It would be easy enough to just point
         * this to "" to prevent cores, but that would let more errors slide
         * through.
         */
        if (!strcmp(key, "region")) {
            *end = 0;
            new = get_region_struct();
            new->name = strdup_local(value);
        } else if (!strcmp(key, "parent")) {
            /*
             * Note that this is in the initialisation code, so we don't actually
             * assign the pointer to the parent yet, because it might not have been
             * parsed.
             */
            *end = 0;

            if (!new) {
                LOG(llevError, "region.c: malformated regions file: \"parent\" before \"region\".\n");
                fatal(SEE_LAST_ERROR);
            }
            if (!value) {
                LOG(llevError, "region.c: malformated regions file: No value given for \"parent\" key.\n");
                fatal(SEE_LAST_ERROR);
            }
            new->parent_name = strdup_local(value);
        } else if (!strcmp(key, "longname")) {
            *end = 0;
            if (!new) {
                LOG(llevError, "region.c: malformated regions file: \"longname\" before \"region\".\n");
                fatal(SEE_LAST_ERROR);
            }
            if (!value) {
                LOG(llevError, "region.c: malformated regions file: No value given for \"longname\" key.\n");
                fatal(SEE_LAST_ERROR);
            }
            new->longname = strdup_local(value);
        } else if (!strcmp(key, "jail")) {
            /* jail entries are of the form: /path/to/map x y */
            char path[MAX_BUF];
            int x, y;

            if (!new) {
                LOG(llevError, "region.c: malformated regions file: \"jail\" before \"region\".\n");
                fatal(SEE_LAST_ERROR);
            }
            if (!value) {
                LOG(llevError, "region.c: malformated regions file: No value given for \"jail\" key.\n");
                fatal(SEE_LAST_ERROR);
            }

            if (sscanf(value, "%[^ ] %d %d\n", path, &x, &y) != 3) {
                LOG(llevError, "region.c: malformated regions entry: jail %s\n", value);
                continue;
            }
            new->jailmap = strdup_local(path);
            new->jailx = x;
            new->jaily = y;
        } else if (!strcmp(key, "msg")) {
            if (!new) {
                LOG(llevError, "region.c: malformated regions file: \"msg\" before \"region\".\n");
                fatal(SEE_LAST_ERROR);
            }
            while (fgets(buf, HUGE_BUF-1, fp) != NULL) {
                key = buf;
                while (isspace(*key))
                    key++;
                if (key && strcmp(key, "endmsg\n") == 0)
                    break;
                else {
                    strcpy(msgbuf+msgpos, key);
                    msgpos += strlen(key);
                }
            }
            /*
             * There may be regions with empty messages (eg, msg/endmsg
             * with nothing between). When maps are loaded, this is done
             * so better do it here too...
             */
            if (msgpos != 0)
                new->msg = strdup_local(msgbuf);

            /* we have to reset msgpos, or the next region will store both msg blocks.*/
            msgpos = 0;
        } else if (!strcmp(key, "fallback")) {
            *end = 0;
            if (!new) {
                LOG(llevError, "region.c: malformated regions file: \"fallback\" before \"region\".\n");
                fatal(SEE_LAST_ERROR);
            }
            if (!value) {
                LOG(llevError, "region.c: malformated regions file: No value given for \"fallback\" key.\n");
                fatal(SEE_LAST_ERROR);
            }
            new->fallback = atoi(value);
        } else if (!strcmp(key, "end")) {
            if (!new) {
                LOG(llevError, "region.c: Ignoring spurious \"end\" between regions.\n");
                continue;
            }
            /* Place this new region last on the list, if the list is empty put it first */
            for (reg = first_region; reg != NULL && reg->next != NULL; reg = reg->next)
                ;

            if (reg == NULL)
                first_region = new;
            else
                reg->next = new;
            new = NULL;
        } else if (!strcmp(key, "nomore")) {
            if (new) {
                LOG(llevError, "region.c: Last region not properly closed.\n");
            }
            /* we have reached the end of the region specs....*/
            break;
        } else {
            /* we should never get here, if we have, then something is wrong */
            LOG(llevError, "Got unknown value in region file: %s %s\n", key, value);
        }
    }
    if (!key || strcmp(key, "nomore"))
        LOG(llevError, "Got premature eof on regions file!\n");
}

/**
 * Links child with their parent from the parent_name field.
 */
static void assign_region_parents(void) {
    region *reg;
    uint32 parent_count = 0;
    uint32 region_count = 0;

    for (reg = first_region; reg != NULL; reg = reg->next) {
        if (reg->parent_name != NULL) {
            reg->parent = get_region_by_name(reg->parent_name);
            if (reg->parent == NULL) {
                LOG(llevError, "Couldn't find parent %s for region %s\n", reg->name, reg->parent_name);
            }
            parent_count++;
        }
        region_count++;
    }
    LOG(llevDebug, "Assigned %u regions with %u parents.\n", region_count, parent_count);
}
