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
@file

Handling of player knowledge of various things.

Right now, the following items are considered:
- alchemy formulea
- monster information
- god information (various types, like enemy, resistances, and such)
- messages in books, from the lib/messages file

Knowledge is specific for a player, and organized in 'items'.
One item is a specific formulae or monster, whatever is coded.

Note that some things are done through functions in readable.c, especially item marking.

Knowledge is persistent and without errors. Maybe that could be changed in the future?



Only use non static functions when using this file.

Loading is done on a per-demand basis, writing at each change.

Knowledge is stored in a "player.knowledge" file in the player's directory.

Knowledge relies on key/values. Key is "knowledge_marker", value is specific to the
item class considered and must have a left-side part, delimited by a double dot, specifying a type.

The value should be enough to determine a unique formulae or monster etc.

A type is linked to various functions for displaying, checking the key value.

During loading, obsolete items (eg formula changed) are discarded.

If the client has notifications support of at least 2, then knowledge information
is sent as needed, incrementally as to not freeze the server.


@todo
- make knowledge shareable between players
- more things to keep trace of
*/

#include <global.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#endif
#include <assert.h>

struct knowledge_player;
struct knowledge_type;

/**
 * Function to fill the StringBuffer with the short description of an item.
 * @param code knowledge internal code.
 * @param buf where to write the summary, must not be NULL.
 */
typedef void (*knowledge_summary)(const char *code, StringBuffer *buf);

/**
 * Function to fill the StringBuffer with a detailed description of an item.
 * @param code knowledge internal code.
 * @param buf where to write the description, must not be NULL.
 */
typedef void (*knowledge_detail)(const char *code, StringBuffer *buf);

/**
 * Function to check if the specified item is valid.
 * @param code knowledge internal code.
 * @return 0 if invalid, non 0 if valid.
 */
typedef int (*knowledge_is_valid_item)(const char *code);

/**
 * Add knowledge information to the player's knowledge.
 * @param current where to add the information to.
 * @param item what to add, format specific to the type.
 * @param type pointer of the handler type.
 * @param pl who we're adding the information for.
 * @return count of actually added items.
 */
typedef int (*knowledge_add_item)(struct knowledge_player *current, const char *item, const struct knowledge_type *type, player *pl);

/**
 * Check if an item can be used for a recipe, and fill the return buffer if it's the case.
 * @param code knowledge internal code.
 * @param item item's name, including title if there is one.
 * @param buf where to put the results. If NULL a new one can be allocated.
 * @param index the knowledge index for this item.
 * @return buf, if it was NULL and the recipe uses the item, a new one is allocated.
 */
typedef StringBuffer* (*knowledge_can_use_alchemy)(sstring code, const char *item, StringBuffer *buf, int index);

/**
 * Get the face for a knowledge item.
 * @param code knowledge internal code.
 * @return face number, -1 (as unsigned) for invalid.
 */
typedef unsigned (*knowledge_face)(sstring code);

struct knowledge_item;

/**
 * Attempt an alchemy based on the specified knowledge.
 * @param pl who attempts the recipe.
 * @param item item to attempt, must not be NULL.
 */
typedef void (*knowledge_attempt)(player *pl, const struct knowledge_item *item);

/** One item type that may be known to the player. */
typedef struct knowledge_type {
    const char *type;                   /**< Type internal code, musn't have a double dot, must be unique ingame. */
    knowledge_summary summary;          /**< Display the short description. */
    knowledge_detail detail;            /**< Display the detailed description. */
    knowledge_is_valid_item validate;   /**< Validate the specific value. */
    knowledge_add_item add;             /**< Add the item to the knowledge store. */
    const char *name;                   /**< Type name for player, to use with 'list'. */
    knowledge_can_use_alchemy use_alchemy; /**< If not null, checks if an item can be used in alchemy. */
    knowledge_attempt attempt_alchemy;
    const char *face;                   /**< Face for the type, as a basename. */
    knowledge_face item_face;           /**< Face for an item, if not defined the face it used. */
} knowledge_type;


/** One known item for a player. */
typedef struct knowledge_item {
    sstring item;                   /**< Internal item code. */
    const knowledge_type* handler;  /**< How to handle this item. */
} knowledge_item;

/** Information about a player. */
typedef struct knowledge_player {
    sstring player_name;            /**< Player's name. */
    struct knowledge_item **items;  /**< Known knowledge. */
    int item_count;                 /**< How many items this players knows. */
    int item_allocated;             /**< How many items are allocated for items. */
    int sent_up_to;                 /**< Largest index that was sent to the client with notifications,
                                     * -1 means the client doesn't want this information. */
    struct knowledge_player *next;  /**< Next player on the list. */
} knowledge_player;

/** All known loaded knowledge for a player. */
static knowledge_player *knowledge_global = NULL;


/**
 * Get a recipe from the internal code.
 * @param value code to get value for.
 * @return recipe, NULL if not found.
 */
static const recipe* knowledge_alchemy_get_recipe(const char *value) {
    const recipe *rec;
    recipelist *rl;
    int count, index;
    const char *dot;

    dot = strchr(value, ':');
    if (!dot) {
        /* warn? */
        return NULL;
    }

    count = atoi(value);
    index = atoi(dot + 1);
    dot = strchr(dot + 1, ':');
    if (!dot)
        /* warn? */
        return NULL;
    dot++;

    rl = get_formulalist(count);
    rec = rl->items;
    while (rec) {
        if (rec->index == index && strcmp(rec->title, dot) == 0)
            return rec;

        rec = rec->next;
    }

    return NULL;
}

/**
 * Give the title of the alchemy recpie.
 * @param value recipe internal value.
 * @param buf where to put the information.
 * @todo merge with stuff in readable.c
 */
static void knowledge_alchemy_summary(const char *value, StringBuffer *buf) {
    const recipe *rec = knowledge_alchemy_get_recipe(value);
    const archetype *arch;

    if (!rec)
        /* warn? */
        return;

    arch = find_archetype(rec->arch_name[RANDOM()%rec->arch_names]);
    if (!arch)
        /* not supposed to happen */
        return;

    if (strcmp(rec->title, "NONE"))
        stringbuffer_append_printf(buf, "%s of %s", arch->clone.name, rec->title);
    else {
        if (arch->clone.title != NULL) {
            stringbuffer_append_printf(buf, "%s %s", arch->clone.name, arch->clone.title);
        }
        else
            stringbuffer_append_printf(buf, "%s", arch->clone.name);
    }
}

/**
 * Give the full description of the alchemy recpie.
 * @param value recipe internal value.
 * @param buf where to store the detail.
 * @todo merge with stuff in readable.c
 */
static void knowledge_alchemy_detail(const char *value, StringBuffer *buf) {
    const recipe *rec = knowledge_alchemy_get_recipe(value);
    const linked_char *next;
    const archetype *arch;
    char name[MAX_BUF];


    if (!rec)
        /* warn? */
        return;

    arch = find_archetype(rec->arch_name[RANDOM()%rec->arch_names]);
    if (!arch)
        /* not supposed to happen */
        return;

    if (strcmp(rec->title, "NONE"))
        stringbuffer_append_printf(buf, "The %s of %s", arch->clone.name, rec->title);
    else {
        if (arch->clone.title != NULL) {
            stringbuffer_append_printf(buf, "The %s %s", arch->clone.name, arch->clone.title);
        }
        else
            stringbuffer_append_printf(buf, "The %s", arch->clone.name);
    }

    arch = find_archetype(rec->cauldron);
    if (arch)
        query_name(&arch->clone, name, MAX_BUF);
    else
        snprintf(name, sizeof(name), "an unknown place");

    stringbuffer_append_printf(buf, " is made at %s and uses the following ingredients:", name);

    for (next = rec->ingred; next != NULL; next = next->next) {
        stringbuffer_append_printf(buf, "\n - %s", next->name);
    }

}

/**
 * Check if an alchemy recipe is still ok.
 * @param item what to check for
 * @return 0 if non valid, 1 else.
 */
static int knowledge_alchemy_validate(const char *item) {
    return knowledge_alchemy_get_recipe(item) != NULL;
}

/**
 * Check if an item can be used for a recipe, and fill the return buffer if it's the case.
 * @param code recipe internal code.
 * @param item item's name, including title if there is one.
 * @param buf where to put the results. If NULL a new one can be allocated.
 * @param index the knowledge index for this item.
 * @return buf, if it was null and the recipe uses the item, a new one is allocated.
 */
static StringBuffer* knowledge_alchemy_can_use_item(sstring code, const char *item, StringBuffer *buf, int index) {
    const recipe *rec = knowledge_alchemy_get_recipe(code);
    const linked_char *next;
    const archetype *arch;
    const char *name;

    if (!rec)
        /* warn? */
        return buf;

    arch = find_archetype(rec->arch_name[RANDOM()%rec->arch_names]);
    if (!arch)
        /* not supposed to happen */
        return buf;

    for (next = rec->ingred; next != NULL; next = next->next) {
        name = next->name;
        while ((*name) != '\0' && (isdigit(*name) || (*name) == ' '))
            name++;

        if (strcmp(item, name) == 0) {
            if (buf == NULL) {
                buf = stringbuffer_new();
                stringbuffer_append_string(buf, "It can be used in the following recipes: ");
            } else
                stringbuffer_append_string(buf, ", ");

            if (strcmp(rec->title, "NONE"))
                stringbuffer_append_printf(buf, "%s of %s", arch->clone.name, rec->title);
            else {
                if (arch->clone.title != NULL) {
                    stringbuffer_append_printf(buf, "%s %s", arch->clone.name, arch->clone.title);
                }
                else
                    stringbuffer_append_printf(buf, "%s", arch->clone.name);
            }
            stringbuffer_append_printf(buf, " (%d)", index);

            break;
        }
    }

    return buf;
}

/**
 * Attempt an alchemy recipe through the knowledge system.
 * @param pl who is attempting the recipe.
 * @param item knowledge item, should be of type recipe.
 */
static void knowledge_alchemy_attempt(player *pl, const knowledge_item *item) {
    const recipe *rp = knowledge_alchemy_get_recipe(item->item);
    object *cauldron = NULL, *inv;
    object *ingredients[50];
    int index, x, y;
    uint32 count, counts[50];
    char name[MAX_BUF];
    const char *ingname;
    linked_char *ing;
    tag_t cauldron_tag;
    mapstruct *map;

    if (!rp) {
        LOG(llevError, "knowledge: couldn't find recipe for %s", item->item);
        return;
    }

    if (rp->ingred_count > 50) {
        LOG(llevError, "knowledge: recipe %s has more than 50 ingredients!", item->item);
        draw_ext_info(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_INFO, "This recipe is too complicated.");
        return;
    }

    /* first, check for a cauldron */
    for (cauldron = pl->ob->below; cauldron; cauldron = cauldron->below) {
        if (strcmp(rp->cauldron, cauldron->arch->name) == 0)
            break;
    }

    if (cauldron == NULL) {
        draw_ext_info_format(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_INFO, "You are not on a %s", rp->cauldron);
        return;
    }

    cauldron_tag = cauldron->count;

    /* find ingredients in player's inventory */
    for (index = 0; index < 50; index++) {
        ingredients[index] = NULL;
    }
    for (inv = pl->ob->inv; inv != NULL; inv = inv->below) {

        if (QUERY_FLAG(inv, FLAG_INV_LOCKED) || QUERY_FLAG(inv, FLAG_STARTEQUIP) /*|| (need_identify(inv) && !QUERY_FLAG(inv, FLAG_IDENTIFIED))*/)
            continue;

        if (inv->title == NULL)
            strncpy(name, inv->name, sizeof(name));
        else
            snprintf(name, sizeof(name), "%s %s", inv->name, inv->title);

        index = 0;
        for (ing = rp->ingred; ing != NULL; ing = ing->next) {

            if (ingredients[index] != NULL) {
                index++;
                continue;
            }

            ingname = ing->name;
            count = 0;
            while (isdigit(*ingname)) {
                count = 10 * count + (*ingname - '0');
                ingname++;
            }
            if (count == 0)
                count = 1;
            while (*ingname == ' ')
                ingname++;

            if (strcmp(name, ingname) == 0 && ((inv->nrof == 0 && count == 1) || (inv->nrof >= count))) {
                ingredients[index] = inv;
                counts[index] = count;
                break;
            }

            index++;
        }
    }

    index = 0;
    for (ing = rp->ingred; ing != NULL; ing = ing->next) {
        if (ingredients[index] == NULL) {
            draw_ext_info_format(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_INFO, "You don't have %s.", ing->name);
            return;
        }

        index++;
    }

    /* ensure cauldron is applied */
    if (pl->ob->container != cauldron) {
        apply_by_living_below(pl->ob);
        if (pl->ob->container != cauldron) {
            draw_ext_info_format(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_INFO, "Couldn't activate the %s.", rp->cauldron);
            return;
        }
    }

    /* ensure cauldron is empty */
    while (cauldron->inv != NULL) {
        inv = cauldron->inv;
        command_take(pl->ob, "");
        if (cauldron->inv == inv) {
            draw_ext_info_format(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_INFO, "Couldn't empty the %s", rp->cauldron);
            return;
        }
    }

    /* drop ingredients */
    for (index = 0; index < rp->ingred_count; index++) {
        tag_t tag = ingredients[index]->count;
        count = ingredients[index]->nrof;
        put_object_in_sack(pl->ob, cauldron, ingredients[index], counts[index]);
        if (object_was_destroyed(ingredients[index], tag)) {
            draw_ext_info(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_INFO, "Hum, some item disappeared, stopping the attempt.");
            return;

        }
        if (count == ingredients[index]->nrof && ingredients[index]->env == pl->ob) {
            draw_ext_info(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_INFO, "Hum, couldn't drop some items, stopping the attempt.");
            return;
        }
    }

    map = pl->ob->map;
    x = pl->ob->x;
    y = pl->ob->y;

    /* do alchemy */
    use_alchemy(pl->ob);

    /* don't forget to slow the player: 1 for alchemy, 1 for each ingredient put,
     * ingrediants taken are handler when picking */
    pl->ob->speed_left -= 1.0 * (rp->ingred_count + 1);

    /* safety: ensure cauldron is still there, and player is still above */
    if (object_was_destroyed(cauldron, cauldron_tag) || map != pl->ob->map || x != pl->ob->x || y != pl->ob->y) {
        return;
    }

    /* get back the result */
    while (cauldron->inv) {
        inv = cauldron->inv;
        examine(pl->ob, inv);
        command_take(pl->ob, "");
        pl->ob->speed_left -= 1.0;
        if (inv == cauldron->inv)
            break;
    }
}

/**
 * Try to get a face for an alchemy recipe.
 * @param code alchemy code.
 * @return face, -1 as unsigned if none could be found.
 */
static unsigned knowledge_alchemy_face(sstring code) {
    const recipe *rp = knowledge_alchemy_get_recipe(code);
    const artifact *art;
    archetype *arch;
    object *item;
    unsigned face;

    if (!rp) {
        LOG(llevError, "knowledge: couldn't find recipe for %s", code);
        return (unsigned)-1;
    }

    if (rp->arch_names == 0)
        return (unsigned)-1;

    arch = find_archetype(rp->arch_name[0]);
    if (strcmp(rp->title, "NONE") == 0) {
        return arch->clone.face->number;
    }

    art = locate_recipe_artifact(rp, 0);
    if (art == NULL)
        return arch->clone.face->number;

    face = arch->clone.face->number;
    item = arch_to_object(arch);
    give_artifact_abilities(item, art->item);
    object_give_identified_properties(item);
    if (item->face != NULL && item->face != blank_face)
        face = item->face->number;
    object_free2(item, FREE_OBJ_FREE_INVENTORY | FREE_OBJ_NO_DESTROY_CALLBACK);

    return face;
}

/**
 * Check whether a player already knows a knowledge item or not.
 * @param current player to check.
 * @param item what to check for.
 * @param kt the knowledge type for item.
 * @return 0 if item is known, 1 else.
 */
static int knowledge_known(const knowledge_player *current, const char *item, const knowledge_type *kt) {
    int i;
    for (i = 0; i < current->item_count; i++) {
        if (strcmp(kt->type, current->items[i]->handler->type) == 0 && strcmp(item, current->items[i]->item) == 0) {
            /* already known, bailout */
            return 1;
        }
    }
    return 0;
}

/**
 * Add a knowledge item to a player's store if not found yet.
 * @param current where to look for the knowledge.
 * @param item internal value of the item to give, considered atomic.
 * @param kt how to handle the knowledge type.
 * @param pl who we're adding the information to.
 * @return 0 if item was already known, 1 else.
 */
static int knowledge_add(knowledge_player *current, const char *item, const knowledge_type *kt, player *pl) {
    knowledge_item *check;

    if (knowledge_known(current, item, kt)) {
        return 0;
    }

    /* keep the knowledge */
    check = calloc(1, sizeof(knowledge_item));
    check->item = add_string(item);
    check->handler = kt;
    if (current->item_count >= current->item_allocated) {
        current->item_allocated += 10;
        current->items = realloc(current->items, current->item_allocated * sizeof(knowledge_item*));
    }
    current->items[current->item_count] = check;
    current->item_count++;

    return 1;
}

/**
 * Monster information summary.
 * @param item knowledge item.
 * @param buf where to put the information.
 * @todo merge with stuff in readable.c
 */
static void knowledge_monster_summary(const char *item, StringBuffer *buf) {
    archetype *monster = find_archetype(item);
    if (!monster)
        return;

    stringbuffer_append_printf(buf, "%s", monster->clone.name);
}

/**
 * Describe in detail a monster.
 * @param item knowledge item for the monster (archetype name).
 * @param buf where to put the description.
 * @todo merge with stuff in readable.c
 */
static void knowledge_monster_detail(const char *item, StringBuffer *buf) {
    archetype *monster = find_archetype(item);

    if (!monster)
        return;

    stringbuffer_append_printf(buf, " *** %s ***\n", monster->clone.name);
    describe_item(&monster->clone, NULL, buf);
}

/**
 * Check if a monster knowledge item is still valid.
 * @param item monster to check
 * @return 0 if invalid, 1 if valid.
 */
static int knowledge_monster_validate(const char *item) {
    const archetype *monster = try_find_archetype(item);
    if (monster == NULL || monster->clone.type != 0)
        return 0;
    return 1;
}

/**
 * Add monster information to the player's knowledge, handling the multiple monster case.
 * @param current where to add the information to.
 * @param item monster to add, can be separated by dots for multiple ones.
 * @param type pointer of the monster type.
 * @param pl who we're adding the information to.
 * @return count of actually added items.
 */
static int knowledge_monster_add(struct knowledge_player *current, const char *item, const struct knowledge_type *type, player *pl) {
    char *dup = strdup_local(item);
    char *pos, *first = dup;
    int added = 0;

    while (first) {
        pos = strchr(first, ':');
        if (pos)
            *pos = '\0';
        added += knowledge_add(current, first, type, pl);
        first = pos ? pos + 1 : NULL;
    }

    free(dup);
    return added;
}

/**
 * Get the face for a monster.
 * @param code monster's code.
 * @return face, -1 as unsigned if invalid.
 */
static unsigned knowledge_monster_face(sstring code) {
    const archetype *monster = find_archetype(code);

    if (!monster || monster->clone.face == blank_face || monster->clone.face == NULL)
        return (unsigned)-1;

    return monster->clone.face->number;
}

/**
 * God information summary.
 * @param item knowledge item.
 * @param buf where to put the information.
 * @todo merge with stuff in readable.c
 */
static void knowledge_god_summary(const char *item, StringBuffer *buf) {
    char *dup = strdup_local(item), *pos = strchr(dup, ':');

    if (pos)
        *pos = '\0';

    stringbuffer_append_printf(buf, "%s [god]", dup);
    free(dup);
}

/**
 * Describe in detail a god.
 * @param item knowledge item for the god (object name and what is known).
 * @param buf where to put the description.
 * @todo merge with stuff in readable.c
 */
static void knowledge_god_detail(const char *item, StringBuffer *buf) {
    char *dup = strdup_local(item), *pos = strchr(dup, ':');
    const archetype *god;
    int what;

    if (!pos) {
        LOG(llevError, "knowledge_god_detail: invalid god item %s\n", item);
        free(dup);
        return;
    }

    *pos = '\0';
    what = atoi(pos + 1);
    god = find_archetype_by_object_name(dup);
    free(dup);

    if (!god) {
        LOG(llevError, "knowledge_god_detail: couldn't find god %s?\n", item);
        return;
    }

    describe_god(&god->clone, what, buf, 0);
}

/**
 * Check if a god knowledge item is still valid.
 * @param item monster to check
 * @return 0 if invalid, 1 if valid.
 */
static int knowledge_god_validate(const char *item) {
    char *dup = strdup_local(item), *pos = strchr(dup, ':');
    int valid;

    if (!pos) {
        LOG(llevError, "knowledge_god_validate: invalid god item %s\n", item);
        free(dup);
        return 0;
    }
    *pos = '\0';
    valid = find_archetype_by_object_name(dup) != NULL;
    free(dup);
    return valid;
}

/**
 * Add god information to the player's knowledge, handling the multiple monster case.
 * @param current where to add the information to.
 * @param item god to add, a dot separating the exact knowledge.
 * @param type pointer of the monster type.
 * @param pl who we're adding the information to.
 * @return count of actually added items.
 */
static int knowledge_god_add(struct knowledge_player *current, const char *item, const struct knowledge_type *type, player *pl) {
    char *dup = strdup_local(item), *pos = strchr(dup, ':');
    StringBuffer *buf;
    int what, i;
    knowledge_item* check;

    if (!pos) {
        LOG(llevError, "knowledge_god_add: invalid god item %s\n", item);
        free(dup);
        return 0;
    }

    *pos = '\0';
    what = atoi(pos + 1);

    for (i = 0; i < current->item_count; i++) {
        check = current->items[i];
        if (check->handler != type)
            /* Only consider our own type. */
            continue;
        if (strncmp(check->item, dup, strlen(dup)) == 0) {
            /* Already known, update information. */
            int known, result;
            pos = strchr(check->item, ':');
            known = atoi(pos + 1);
            result = known | what;
            buf = stringbuffer_new();
            stringbuffer_append_printf(buf, "%s:%d", dup, result);
            free_string(check->item);
            check->item = stringbuffer_finish_shared(buf);
            free(dup);
            return (result != known);
        }
    }

    free(dup);

    /* Not known, so just add it regularly. */
    return knowledge_add(current, item, type, pl);
}

/**
 * Get the face for a god.
 * @param code god's code.
 * @return face, -1 as unsigned if invalid.
 */
static unsigned knowledge_god_face(sstring code) {
    char buf[MAX_BUF];
    size_t letter;
    const archetype *altar_arch;

    snprintf(buf, MAX_BUF, "altar_");
    letter = strlen(buf);
    strncpy(buf+letter, code, MAX_BUF-letter);
    for (; letter < strlen(buf); letter++) {
        if (buf[letter] == ':') {
            buf[letter] = '\0';
            break;
        }
        buf[letter] = tolower(buf[letter]);
    }
    altar_arch = find_archetype(buf);
    if (altar_arch == NULL)
        return (unsigned)-1;

    return altar_arch->clone.face->number;
}

/**
 * Give the title of a message.
 * @param value message internal value.
 * @param buf where to put the information.
 * @todo merge with stuff in readable.c
 */
static void knowledge_message_summary(const char *value, StringBuffer *buf) {
    const GeneralMessage *msg = get_message_from_identifier(value);

    if (!msg)
        /* warn? */
        return;

    stringbuffer_append_printf(buf, "%s", get_message_title(msg));
}

/**
 * Give the full description of a message.
 * @param value message internal value.
 * @param buf where to store the detail.
 * @todo merge with stuff in readable.c
 */
static void knowledge_message_detail(const char *value, StringBuffer *buf) {
    const GeneralMessage *msg = get_message_from_identifier(value);

    if (!msg)
        /* warn? */
        return;

    stringbuffer_append_printf(buf, "%s", get_message_body(msg));
}

/**
 * Check if a message is still valid.
 * @param item what to check for
 * @return 0 if non valid, 1 else.
 */
static int knowledge_message_validate(const char *item) {
    return get_message_from_identifier(item) != NULL;
}


/** All handled knowledge items. */
static const knowledge_type knowledges[] = {
    { "alchemy", knowledge_alchemy_summary, knowledge_alchemy_detail, knowledge_alchemy_validate, knowledge_add, "recipes", knowledge_alchemy_can_use_item, knowledge_alchemy_attempt, "knowledge_recipes.111", knowledge_alchemy_face },
    { "monster", knowledge_monster_summary, knowledge_monster_detail, knowledge_monster_validate, knowledge_monster_add, "monsters", NULL, NULL, "knowledge_monsters.111", knowledge_monster_face },
    { "god", knowledge_god_summary, knowledge_god_detail, knowledge_god_validate, knowledge_god_add, "gods", NULL, NULL, "knowledge_gods.111", knowledge_god_face },
    { "message", knowledge_message_summary, knowledge_message_detail, knowledge_message_validate, knowledge_add, "messages", NULL, NULL, "knowledge_messages.111", NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};


/**
 * Find a knowledge handler from its type.
 * @param type internal type name.
 * @return handler, NULL if not found.
 */
static const knowledge_type *knowledge_find(const char *type) {
    int i = 0;
    while (knowledges[i].type != NULL) {
        if (strcmp(knowledges[i].type, type) == 0)
            return &knowledges[i];
        i++;
    }

    return NULL;
}

/**
 * Store all knowledge data for a player to disk.
 * @param kp player data to store.
 */
static void knowledge_write_player_data(const knowledge_player *kp) {
    FILE *file;
    char write[MAX_BUF], final[MAX_BUF];
    const knowledge_item *item;
    int i;

    snprintf(final, sizeof(final), "%s/%s/%s/%s.knowledge", settings.localdir, settings.playerdir, kp->player_name, kp->player_name);
    snprintf(write, sizeof(write), "%s.new", final);

    file = fopen(write, "w+");
    if (!file) {
        LOG(llevError, "knowledge: couldn't open player knowledge file %s!", write);
        draw_ext_info(NDI_UNIQUE | NDI_ALL_DMS, 0, NULL, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_LOADSAVE, "File write error on server!");
        return;
    }

    for (i = 0; i < kp->item_count; i++) {
        item = kp->items[i];
        fprintf(file, "%s:%s\n", item->handler->type, item->item);
    }

    fclose(file);
    /** @todo rename/backup, stuff like that */
    unlink(final);
    rename(write, final);
}

/**
 * Read all knowledge data for a player from disk, discarding invalid data.
 * @param kp player structure to load data into.
 */
static void knowledge_read_player_data(knowledge_player *kp) {
    FILE *file;
    char final[MAX_BUF], read[MAX_BUF], *dot;
    knowledge_item *item;
    const knowledge_type *type;

    snprintf(final, sizeof(final), "%s/%s/%s/%s.knowledge", settings.localdir, settings.playerdir, kp->player_name, kp->player_name);

    file = fopen(final, "r");
    if (!file) {
        /* no knowledge yet, no big deal */
        return;
    }

    while (fgets(read, sizeof(read), file) != NULL) {
        if (strlen(read) > 0)
            read[strlen(read) - 1] = '\0';

        dot = strchr(read, ':');
        if (!dot) {
            LOG(llevError, "knowledge: invalid line in file %s\n", final);
            continue;
        }

        *dot = '\0';

        type = knowledge_find(read);
        if (!type) {
            LOG(llevError, "knowledge: invalid type %s in file %s\n", read, final);
            continue;
        }
        if (!type->validate(dot + 1)) {
            LOG(llevDebug, "knowledge: ignoring now invalid %s in file %s\n", read, final);
            continue;
        }

        item = calloc(1, sizeof(knowledge_item));
        item->item = add_string(dot + 1);
        item->handler = type;
        if (kp->item_count == kp->item_allocated) {
            kp->item_allocated += 10;
            kp->items = realloc(kp->items, kp->item_allocated * sizeof(knowledge_item*));
        }
        kp->items[kp->item_count] = item;
        kp->item_count++;
    }
    fclose(file);
}

/**
 * Find or create the knowledge store for a player. Will load data if required.
 * fatal() will be called if memory failure.
 * @param pl who to find data for.
 * @return data store, never NULL.
 */
static knowledge_player *knowledge_get_or_create(const player *pl) {
    knowledge_player *cur = knowledge_global;

    while (cur) {
        if (cur->player_name == pl->ob->name)
            return cur;
        cur = cur->next;
    }

    cur = calloc(1, sizeof(knowledge_player));
    if (!cur)
        fatal(OUT_OF_MEMORY);
    cur->player_name = add_refcount(pl->ob->name);
    cur->next = knowledge_global;
    knowledge_global = cur;
    /* read knowledge for this player */
    knowledge_read_player_data(cur);
    if (pl->socket.notifications < 2)
        cur->sent_up_to = -1;
    return cur;
}

/**
 * Give a knowledge item from its code.
 * @param pl who to give the knowldge to.
 * @param marker knowledge's code.
 * @param book optional item containing the knowledge code.
 */
void knowledge_give(player *pl, const char *marker, const object *book) {
    char *dot, *copy;
    const knowledge_type *type;
    int none, added = 0;
    knowledge_player *current = knowledge_get_or_create(pl);

    /* process marker, find if already known */
    dot = strchr(marker, ':');
    if (dot == NULL)
        return;

    copy = calloc(1, strlen(marker) + 1);
    strncpy(copy, marker, strlen(marker) + 1);

    dot = strchr(copy, ':');
    *dot = '\0';
    dot++;

    type = knowledge_find(copy);
    if (!type) {
        LOG(llevError, "knowledge: invalid marker type %s in %s\n", copy, book == NULL ? "(null)" : book->name);
        free(copy);
        return;
    }

    none = (current->items == NULL);
    if (type->validate(dot)) {
        added = type->add(current, dot, type, pl);
    }
    free(copy);

    if (added) {
        draw_ext_info(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_MISC, MSG_TYPE_CLIENT_NOTICE, "You write that down for future reference.");
        if (none) {
            /* first information ever written down, be nice and give hint to recover it. */
            draw_ext_info(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_MISC, MSG_TYPE_CLIENT_NOTICE, "Use the 'knowledge' command to browse what you write down (this message will not appear anymore).");
        }
    }

    if (pl->has_directory)
        knowledge_write_player_data(current);
}

/**
 * Player is reading a book, give knowledge if needed, warn player, and such.
 * @param pl who is reading.
 * @param book what is read.
 */
void knowledge_read(player *pl, object *book) {
    sstring marker = object_get_value(book, "knowledge_marker");
    if (marker != NULL)
        knowledge_give(pl, marker, book);
}

/**
 * Actually display knowledge list.
 * @param pl who to display to.
 * @param show_only if not NULL, only display this knowledge type.
 * @param search if not NULL, only display items having the summary or detail contain the string. Must be either NULL or a non empty string.
 */
static void knowledge_do_display(object *pl, const knowledge_type *show_only, const char *search) {
    knowledge_player *kp;
    knowledge_item *item;
    int header = 0, show, i;
    StringBuffer *summary;
    char *final;

    assert(search == NULL || search[0] != '\0');

    kp = knowledge_get_or_create(pl->contr);
    for (i = 0; i < kp->item_count; i++) {
        item = kp->items[i];
        show = 1;

        summary = stringbuffer_new();
        item->handler->summary(item->item, summary);
        final = stringbuffer_finish(summary);

        if (show_only != NULL && item->handler != show_only) {
            show = 0;
        }
        if (search != NULL && search[0] != '\0') {
            if (strstr(final, search) == NULL) {
                char *fd;
                StringBuffer *detail = stringbuffer_new();
                item->handler->detail(item->item, detail);
                fd = stringbuffer_finish(detail);
                if (strstr(fd, search) == NULL)
                    show = 0;
                free(fd);
            }
        }

        if (show == 1) {
            if (header == 0) {
                if (search != NULL)
                    draw_ext_info_format(NDI_UNIQUE, 0, pl, MSG_TYPE_MISC, MSG_TYPE_CLIENT_NOTICE, "You have knowledge of the following %s concerning '%s':", show_only == NULL ? "things" : show_only->type, search);
                else if (show_only != NULL)
                    draw_ext_info_format(NDI_UNIQUE, 0, pl, MSG_TYPE_MISC, MSG_TYPE_CLIENT_NOTICE, "You have knowledge of those %s:", show_only->name);
                else
                    draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_MISC, MSG_TYPE_CLIENT_NOTICE, "You have knowledge of:");
                header = 1;
            }

            draw_ext_info_format(NDI_UNIQUE, 0, pl, MSG_TYPE_MISC, MSG_TYPE_CLIENT_NOTICE, "(%3d) %s", i + 1, final);
        }

        free(final);
    }

    if (header == 0) {
        if (search != NULL)
            draw_ext_info_format(NDI_UNIQUE, 0, pl, MSG_TYPE_MISC, MSG_TYPE_CLIENT_NOTICE, "You don't know yet any relevant information about '%s'.", search);
        else if (show_only != NULL)
            draw_ext_info_format(NDI_UNIQUE, 0, pl, MSG_TYPE_MISC, MSG_TYPE_CLIENT_NOTICE, "You don't know yet any relevant information about %s.", show_only->name);
        else
            draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_MISC, MSG_TYPE_CLIENT_NOTICE, "You don't know yet any relevant information.");
    }
}

/**
 * Display all a player's knowledge.
 * @param pl who to display knowledge of.
 * @param params additionnal parameters.
 */
static void knowledge_display(object *pl, const char *params) {
    const knowledge_type *show_only = NULL;

    if (params && params[0] == ' ') {
        const char *type = params + 1;
        int idx = 0;
        for (; knowledges[idx].type != NULL; idx++) {
            if (strcmp(knowledges[idx].name, type) == 0) {
                show_only = &knowledges[idx];
                break;
            }
        }

        if (show_only == NULL) {
            draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_MISC, MSG_TYPE_CLIENT_NOTICE, "Invalid type, valid types are:");
            for (idx = 0; knowledges[idx].type != NULL; idx++) {
                draw_ext_info_format(NDI_UNIQUE, 0, pl, MSG_TYPE_MISC, MSG_TYPE_CLIENT_NOTICE, "- %s", knowledges[idx].name);
            }
            return;
        }
    }

    knowledge_do_display(pl, show_only, NULL);
}

/**
 * Show the details of a knowledge item.
 * @param pl who is asking for details.
 * @param params additional parameters, should contain the knowledge item number.
 */
static void knowledge_show(object *pl, const char *params) {
    knowledge_player *kp;
    knowledge_item *item;
    int count = atoi(params) - 1;
    StringBuffer *buf;
    char *final;

    kp = knowledge_get_or_create(pl->contr);
    if (count < 0 || count >= kp->item_count) {
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_INFO, "Invalid knowledge number");
        return;
    }

    item = kp->items[count];
    buf = stringbuffer_new();
    item->handler->detail(item->item, buf);
    final = stringbuffer_finish(buf);
    draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_MISC, MSG_TYPE_CLIENT_NOTICE, final);
    free(final);
}

/**
 * Player attempts something on a knowledge, get item and try it.
 * @param pl who is trying.
 * @param params command parameters.
 */
static void knowledge_do_attempt(object *pl, const char *params) {
    knowledge_player *kp;
    knowledge_item *item;
    int count = atoi(params) - 1;

    kp = knowledge_get_or_create(pl->contr);
    if (count < 0 || count >= kp->item_count) {
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_INFO, "Invalid knowledge number");
        return;
    }

    item = kp->items[count];
    if (item->handler->attempt_alchemy == NULL) {
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_INFO, "You can't do anything with that knowledge.");
    } else {
        item->handler->attempt_alchemy(pl->contr, item);
    }
}

/**
 * Handle the 'knowledge' for a player.
 * @param pl who is using the command.
 * @param params additional parameters.
 */
void command_knowledge(object *pl, const char *params) {

    if (!pl->contr) {
        LOG(llevError, "command_knowledge: called for %s not a player!\n", pl->name);
        return;
    }

    if (!params || *params == '\0') {
        command_help(pl, "knowledge");
        return;
    }

    if (strncmp(params, "list", 4) == 0) {
        knowledge_display(pl, params + 4);
        return;
    }

    if (strncmp(params, "search ", 7) == 0) {
        knowledge_do_display(pl, NULL, params + 7);
        return;
    }

    if (strncmp(params, "show ", 5) == 0) {
        knowledge_show(pl, params + 5);
        return;
    }

    if (strncmp(params, "attempt ", 8) == 0) {
        knowledge_do_attempt(pl, params + 8);
        return;
    }

    command_help(pl, "knowledge");
}

/**
 * Free all knowledge items for a player.
 * @param kp what to free.
 */
static void free_knowledge_items(knowledge_player *kp) {
    knowledge_item *item;
    int i;

    for (i = 0; i < kp->item_count; i++) {
        item = kp->items[i];
        free_string(item->item);
        free(item);
    }
    free(kp->items);
    kp->items = NULL;
    kp->item_count = 0;
    kp->item_allocated = 0;
}

/**
 * Totally free a knowledge structure, and its items.
 * @param kp structure to free, becomes invalid.
 */
static void free_knowledge_player(knowledge_player *kp) {
    free_knowledge_items(kp);
    free_string(kp->player_name);
    free(kp);
}

/**
 * Free all knowledge data.
 */
void free_knowledge(void) {
    knowledge_player *kp, *next;

    kp = knowledge_global;
    while (kp) {
        next = kp->next;
        free_knowledge_player(kp);
        kp = next;
    }
    knowledge_global = NULL;
}

/**
 * Determines whether a player knows a specific knowledge or not.
 * @param pl who to check knowledge for.
 * @param knowledge what to check for, in format "type:(type specific value)".
 * @return 0 if item is known, 1 else.
 */
int knowledge_player_knows(const player *pl, const char *knowledge) {
    const knowledge_type *type;
    char copy[MAX_BUF], *pos;
    const knowledge_player *current;

    if (strlen(knowledge) >= MAX_BUF - 1) {
        LOG(llevError, "knowledge_player_knows: too long knowledge %s\n", knowledge);
        return 0;
    }

    snprintf(copy, sizeof(copy), "%s", knowledge);
    pos = strchr(copy, ':');
    if (pos == NULL) {
        LOG(llevError, "knowledge_player_knows: invalid knowledge item %s\n", knowledge);
        return 0;
    }

    *pos = '\0';
    pos++;

    type = knowledge_find(copy);
    if (type == NULL) {
        LOG(llevError, "knowledge_player_knows: invalid knowledge type %s\n", knowledge);
        return 0;
    }

    current = knowledge_get_or_create(pl);

    return knowledge_known(current, pos, type);
}

/**
 * Displays known alchemy recipes an item can be used in.
 * @param op who to display recipes for.
 * @param item what to check formulae for.
 */
void knowledge_item_can_be_used_alchemy(object *op, const object *item) {
    knowledge_player *cur;
    knowledge_item *ki;
    char item_name[MAX_BUF], *result;
    const char *name;
    StringBuffer *buf = NULL;
    int i;

    if (op->type != PLAYER || op->contr == NULL)
        return;

    cur = knowledge_get_or_create(op->contr);

    if (item->title != NULL) {
        snprintf(item_name, sizeof(item_name), "%s %s", item->name, item->title);
        name = item_name;
    } else
        name = item->name;

    for (i = 0; i < cur->item_count; i++) {
        ki = cur->items[i];
        if (ki->handler->use_alchemy != NULL) {
            buf = ki->handler->use_alchemy(ki->item, name, buf, i + 1);
        }
    }

    if (buf == NULL)
        return;

    stringbuffer_append_string(buf, ".");
    result = stringbuffer_finish(buf);
    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_EXAMINE,
        result);
    free(result);
}

/**
 * Send the reply_info for 'knowledge_info'.
 * @param ns socket to send information to.
 */
void knowledge_send_info(socket_struct *ns) {
    int i;
    unsigned face;
    SockList sl;

    face = find_face("knowledge_generic.111", (unsigned)-1);
    if (face != (unsigned)-1 && (!ns->faces_sent[face] & NS_FACESENT_FACE))
        esrv_send_face(ns, face, 0);

    SockList_Init(&sl);
    SockList_AddString(&sl, "replyinfo knowledge_info\n");
    SockList_AddPrintf(&sl, "::%u:0\n", face);

    for (i = 0; knowledges[i].type != NULL; i++) {
        face = find_face(knowledges[i].face, (unsigned)-1);
        if (face != (unsigned)-1 && (!ns->faces_sent[face] & NS_FACESENT_FACE))
            esrv_send_face(ns, face, 0);

        SockList_AddPrintf(&sl, "%s:%s:%u:%s\n", knowledges[i].type, knowledges[i].name, face, knowledges[i].attempt_alchemy != NULL ? "1" : "0");
    }

    Send_With_Handling(ns, &sl);
    SockList_Term(&sl);
}

/**
 * Send initial known knowledge to player, if requested.
 * @param pl who to send knowledge for.
 */
void knowledge_send_known(player *pl) {
    knowledge_player *kp;

    if (pl->socket.notifications < 2)
        return;

    /* merely loading the knowledge will mark it as to be sent through knowledge_process_incremental(),
     * but we need to reset the sent_up_to field if eg the player was the last one to leave
     * then joins again - no knowledge processing is done at that point. */
    kp = knowledge_get_or_create(pl);
    kp->sent_up_to = 0;
}

/**
 * Ensure the knowledge state is correctly saved for the player.
 * This function should only be called once, when the player's save directory
 * is created. All other knowledge functions save the state automatically, but save
 * can only happen when the player directory exists.
 * @param pl who to save the state for.
 */
void knowledge_first_player_save(player *pl) {
    knowledge_player *cur = knowledge_global;

    while (cur) {
        if (cur->player_name == pl->ob->name) {
            knowledge_write_player_data(cur);
        }
        cur = cur->next;
    }

}

/**
 * Incrementally send knowledge information to players, and remove
 * information for players who left.
 */
void knowledge_process_incremental(void) {
    int i, last;
    SockList sl;
    size_t size;
    const knowledge_item *item;
    StringBuffer *buf;
    char *title;
    unsigned face;
    knowledge_player *cur = knowledge_global, *prev = NULL;
    player *pl;

    while (cur) {

        for (pl = first_player; pl != NULL; pl = pl->next) {
            if (pl->ob->name == cur->player_name) {
                break;
            }
        }

        /* player left, remove knowledge */
        if (pl == NULL) {
            if (prev == NULL) {
                knowledge_global = cur->next;
            } else {
                prev->next = cur->next;
            }

            free_knowledge_player(cur);

            /* wait until next tick to do something else */
            return;
        }

        if (cur->sent_up_to == -1 || cur->sent_up_to == cur->item_count) {
            prev = cur;
            cur = cur->next;
            continue;
        }

        last = MIN(cur->sent_up_to + 50, cur->item_count);
        SockList_Init(&sl);
        SockList_AddString(&sl, "addknowledge ");
        for (i = cur->sent_up_to; i < last; i++) {
            item = cur->items[i];

            buf = stringbuffer_new();
            item->handler->summary(item->item, buf);
            title = stringbuffer_finish(buf);

            face = (unsigned)-1;
            if (item->handler->item_face != NULL)
                face = item->handler->item_face(item->item);

            if (face == (unsigned)-1)
                face = find_face(item->handler->face, (unsigned)-1);

            size = 4 + (2 + strlen(item->handler->type)) + (2 + strlen(title)) + 4;

            if (SockList_Avail(&sl) < size) {
                Send_With_Handling(&pl->socket, &sl);
                SockList_Reset(&sl);
                SockList_AddString(&sl, "addknowledge ");
            }

            SockList_AddInt(&sl, i + 1);
            SockList_AddLen16Data(&sl, item->handler->type, strlen(item->handler->type));
            if ((face != (unsigned)-1) && !(pl->socket.faces_sent[face]&NS_FACESENT_FACE))
                esrv_send_face(&pl->socket, face, 0);
            SockList_AddLen16Data(&sl, title, strlen(title));
            SockList_AddInt(&sl, face);

            free(title);
        }

        cur->sent_up_to = last;

        Send_With_Handling(&pl->socket, &sl);
        SockList_Term(&sl);

        /* don't send more this tick */
        break;
    }
}

/**
 * Display the details of a monster if the player knows them.
 * @param op player asking for details.
 * @param name monster's archetype name.
 */
void knowledge_show_monster_detail(object *op, const char *name) {
    knowledge_player *kp;
    StringBuffer *buf = NULL;
    char *result;

    if (op->contr == NULL)
        return;

    kp = knowledge_get_or_create(op->contr);

    if (!knowledge_known(kp, name, &knowledges[1]))
        return;

    buf = stringbuffer_new();
    knowledge_monster_detail(name, buf);
    result = stringbuffer_finish(buf);
    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_EXAMINE, result);
    free(result);
}
