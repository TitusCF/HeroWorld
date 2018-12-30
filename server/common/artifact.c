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
 * @file
 * Everything concerning artifacts.
 * @see page_treasure_list
 */

#include <stdlib.h>
#include "global.h"
#include "loader.h"

int artifact_init;  /**< 1 if doing archetypes initialization */

/**
 * Allocate and return the pointer to an empty artifactlist structure.
 *
 * @return
 * new structure blanked, never NULL.
 *
 * @note
 * will fatal() if memory error.
 */
static artifactlist *get_empty_artifactlist(void) {
    artifactlist *tl = (artifactlist *)malloc(sizeof(artifactlist));
    if (tl == NULL)
        fatal(OUT_OF_MEMORY);
    tl->next = NULL;
    tl->items = NULL;
    tl->total_chance = 0;
    return tl;
}

/**
 * Allocate and return the pointer to an empty artifact structure.
 *
 * @return
 * new structure blanked, never NULL.
 *
 * @note
 * will fatal() if memory error.
 */
static artifact *get_empty_artifact(void) {
    artifact *t = (artifact *)malloc(sizeof(artifact));
    if (t == NULL)
        fatal(OUT_OF_MEMORY);
    t->item = NULL;
    t->next = NULL;
    t->chance = 0;
    t->difficulty = 0;
    t->allowed = NULL;
    t->allowed_size = 0;
    return t;
}

/**
 * Frees a link structure and its next items.
 *
 * @param lc
 * item to free. Pointer is free()d too, so becomes invalid.
 */
static void free_charlinks(linked_char *lc) {
    if (lc->next)
        free_charlinks(lc->next);
    free(lc);
}

/**
 * Totally frees an artifact, its next items, and such.
 *
 * @param at
 * artifact to free. Pointer is free()d too, so becomes invalid.
 *
 * @note
 * Objects at->item are malloc()ed by init_artifacts(), so can simply be free()d.
 *
 * But artifact inventory is a 'real' object, that may be created for 'old' objects. So should be
 * destroyed through object_free_drop_inventory(). Note that it isn't on the usual item list, so some tweaking is required.
 */
static void free_artifact(artifact *at) {
    object *next;

    if (at->next)
        free_artifact(at->next);
    if (at->allowed)
        free_charlinks(at->allowed);
    while (at->item) {
        next = at->item->next;
        if (at->item->name)
            free_string(at->item->name);
        if (at->item->name_pl)
            free_string(at->item->name_pl);
        if (at->item->msg)
            free_string(at->item->msg);
        if (at->item->title)
            free_string(at->item->title);
        object_free_key_values(at->item);
        free(at->item);
        at->item = next;
    }
    free(at);
}

/**
 * Free specified list and its items.
 *
 * @param al
 * list to free. Pointer is free()d too, so becomes invalid.
 */
static void free_artifactlist(artifactlist *al) {
    artifactlist *nextal;

    for (; al != NULL; al = nextal) {
        nextal = al->next;
        if (al->items) {
            free_artifact(al->items);
        }
        free(al);
    }
}

/**
 * Free all artifact-related information.
 */
void free_all_artifacts(void) {
    free_artifactlist(first_artifactlist);
    first_artifactlist = NULL;
}

/** Give 1 re-roll attempt per artifact */
#define ARTIFACT_TRIES 2

/**
 * Decides randomly which artifact the object should be
 * turned into.  Makes sure that the item can become that
 * artifact (means magic, difficulty, and Allowed fields properly).
 * Then calls give_artifact_abilities in order to actually create
 * the artifact.
 */
void generate_artifact(object *op, int difficulty) {
    const artifactlist *al;
    const artifact *art;
    int i;

    al = find_artifactlist(op->type);

    if (al == NULL) {
        return;
    }

    for (i = 0; i < ARTIFACT_TRIES; i++) {
        int roll = RANDOM()%al->total_chance;

        for (art = al->items; art != NULL; art = art->next) {
            roll -= art->chance;
            if (roll < 0)
                break;
        }

        if (art == NULL || roll >= 0) {
            LOG(llevError, "Got null entry and non zero roll in generate_artifact, type %d\n", op->type);
            return;
        }
        if (!strcmp(art->item->name, "NONE"))
            return;
        if (FABS(op->magic) < art->item->magic)
            continue; /* Not magic enough to be this item */

        /* Map difficulty not high enough */
        if (difficulty < art->difficulty)
            continue;

        if (!legal_artifact_combination(op, art)) {
#ifdef TREASURE_VERBOSE
            LOG(llevDebug, "%s of %s was not a legal combination.\n", op->name, art->item->name);
#endif
            continue;
        }
        give_artifact_abilities(op, art->item);
        return;
    }
}

/**
 * Fixes the given object, giving it the abilities and titles
 * it should have due to the second artifact-template.
 */
void give_artifact_abilities(object *op, const object *artifact) {
    char new_name[MAX_BUF];

    snprintf(new_name, sizeof(new_name), "of %s", artifact->name);
    if (op->title)
        free_string(op->title);
    op->title = add_string(new_name);
    if (op->artifact)
        free_string(op->artifact);
    op->artifact = add_refcount(artifact->name);
    add_abilities(op, artifact); /* Give out the bonuses */

    return;
}

/**
 * Checks if op can be combined with art.
 */
int legal_artifact_combination(const object *op, const artifact *art) {
    int neg, success = 0;
    linked_char *tmp;
    const char *name;

    if (art->allowed == (linked_char *)NULL)
        return 1; /* Ie, "all" */
    for (tmp = art->allowed; tmp; tmp = tmp->next) {
#ifdef TREASURE_VERBOSE
        LOG(llevDebug, "legal_art: %s\n", tmp->name);
#endif
        if (*tmp->name == '!') {
            name = tmp->name+1;
            neg = 1;
        } else {
            name = tmp->name;
            neg = 0;
        }

        /* If we match name, then return the opposite of 'neg' */
        if (!strcmp(name, op->name) || (op->arch && !strcmp(name, op->arch->name)))
            return !neg;

        /* Set success as true, since if the match was an inverse, it means
         * everything is allowed except what we match
         */
        else if (neg)
            success = 1;
    }
    return success;
}

/**
 * Used in artifact generation.  The bonuses of the first object
 * is modified by the bonuses of the second object.
 */
void add_abilities(object *op, const object *change) {
    int i, tmp;
    char buf[MAX_BUF];
    sstring key;

    if (change->face != blank_face) {
#ifdef TREASURE_VERBOSE
        LOG(llevDebug, "FACE: %d\n", change->face->number);
#endif

        object_set_value(op, "identified_face", change->face->name, 1);
    }
    if (QUERY_FLAG(change, FLAG_CLIENT_ANIM_RANDOM)) {
        object_set_value(op, "identified_anim_random", "1", 1);
    }
    if (change->anim_speed > 0) {
        snprintf(buf, sizeof(buf), "%d", change->anim_speed);
        object_set_value(op, "identified_anim_speed", buf, 1);
    }
    if (change->animation_id != 0 && op->arch != NULL) {
        /* op->arch can be NULL when called from artifact_msg(). */
        snprintf(buf, sizeof(buf),"%d", change->animation_id);
        object_set_value(op, "identified_animation", buf, 1);
    } else if (op->animation_id != 0 && (key = object_get_value(change, "animation_suffix")) != NULL) {
        snprintf(buf, sizeof(buf), "%s_%s", animations[op->animation_id].name, key);
        i = try_find_animation(buf);
        if (i != 0) {
            snprintf(buf, sizeof(buf),"%d", i);
            object_set_value(op, "identified_animation", buf, 1);
        }
    }

    for (i = 0; i < NUM_STATS; i++)
        change_attr_value(&(op->stats), i, get_attr_value(&(change->stats), i));

    op->attacktype |= change->attacktype;
    op->path_attuned |= change->path_attuned;
    op->path_repelled |= change->path_repelled;
    op->path_denied |= change->path_denied;
    op->move_type |= change->move_type;
    op->stats.luck += change->stats.luck;

    if (QUERY_FLAG(change, FLAG_CURSED))
        SET_FLAG(op, FLAG_CURSED);
    if (QUERY_FLAG(change, FLAG_DAMNED))
        SET_FLAG(op, FLAG_DAMNED);
    if ((QUERY_FLAG(change, FLAG_CURSED) || QUERY_FLAG(change, FLAG_DAMNED))
    && op->magic > 0)
        set_abs_magic(op, -op->magic);

    if (QUERY_FLAG(change, FLAG_LIFESAVE))
        SET_FLAG(op, FLAG_LIFESAVE);
    if (QUERY_FLAG(change, FLAG_REFL_SPELL))
        SET_FLAG(op, FLAG_REFL_SPELL);
    if (QUERY_FLAG(change, FLAG_STEALTH))
        SET_FLAG(op, FLAG_STEALTH);
    if (QUERY_FLAG(change, FLAG_XRAYS))
        SET_FLAG(op, FLAG_XRAYS);
    if (QUERY_FLAG(change, FLAG_BLIND))
        SET_FLAG(op, FLAG_BLIND);
    if (QUERY_FLAG(change, FLAG_SEE_IN_DARK))
        SET_FLAG(op, FLAG_SEE_IN_DARK);
    if (QUERY_FLAG(change, FLAG_REFL_MISSILE))
        SET_FLAG(op, FLAG_REFL_MISSILE);
    if (QUERY_FLAG(change, FLAG_MAKE_INVIS))
        SET_FLAG(op, FLAG_MAKE_INVIS);

    if (QUERY_FLAG(change, FLAG_STAND_STILL)) {
        CLEAR_FLAG(op, FLAG_ANIMATE);
        /* so artifacts will join */
        if (!QUERY_FLAG(op, FLAG_ALIVE))
            op->speed = 0.0;
        object_update_speed(op);
    }
    if (change->nrof)
        op->nrof = RANDOM()%((int)change->nrof)+1;
    op->stats.exp += change->stats.exp; /* Speed modifier */
    op->stats.wc += change->stats.wc;
    op->stats.ac += change->stats.ac;

    if (change->other_arch) {
        /* Basically, for horns & potions, the other_arch field is the spell
         * to cast.  So convert that to into a spell and put it into
         * this object.
         */
        if (op->type == ROD || op->type == POTION) {
            object *tmp_obj;

            /* Remove any spells this object currently has in it */
            while (op->inv) {
                tmp_obj = op->inv;
                object_remove(tmp_obj);
                object_free_drop_inventory(tmp_obj);
            }
            tmp_obj = arch_to_object(change->other_arch);
            /* This is an artifact, so this function will be called at load time,
             * thus we don't need to keep the inventory */
            SET_FLAG(tmp_obj, FLAG_NO_SAVE);
            object_insert_in_ob(tmp_obj, op);
        }
        /* No harm setting this for potions/horns */
        op->other_arch = change->other_arch;
    }

    if (change->stats.hp < 0)
        op->stats.hp = -change->stats.hp;
    else
        op->stats.hp += change->stats.hp;
    if (change->stats.maxhp < 0)
        op->stats.maxhp = -change->stats.maxhp;
    else
        op->stats.maxhp += change->stats.maxhp;
    if (change->stats.sp < 0)
        op->stats.sp = -change->stats.sp;
    else
        op->stats.sp += change->stats.sp;
    if (change->stats.maxsp < 0)
        op->stats.maxsp = -change->stats.maxsp;
    else
        op->stats.maxsp += change->stats.maxsp;
    if (change->stats.food < 0)
        op->stats.food = -(change->stats.food);
    else
        op->stats.food += change->stats.food;
    if (change->level < 0)
        op->level = -(change->level);
    else
        op->level += change->level;

    op->item_power = change->item_power;

    for (i = 0; i < NROFATTACKS; i++) {
        if (change->resist[i]) {
            op->resist[i] += change->resist[i];
        }
    }
    if (change->stats.dam) {
        if (change->stats.dam < 0)
            op->stats.dam = (-change->stats.dam);
        else if (op->stats.dam) {
            tmp = (int)(((int)op->stats.dam*(int)change->stats.dam)/10);
            if (tmp == op->stats.dam) {
                if (change->stats.dam < 10)
                    op->stats.dam--;
                else
                    op->stats.dam++;
            } else
                op->stats.dam = tmp;
        }
    }
    if (change->weight) {
        if (change->weight < 0)
            op->weight = (-change->weight);
        else
            op->weight = (op->weight*(change->weight))/100;
    }
    if (change->last_sp) {
        if (change->last_sp < 0)
            op->last_sp = (-change->last_sp);
        else
            op->last_sp = (signed char)(((int)op->last_sp*(int)change->last_sp)/(int)100);
    }
    if (change->gen_sp_armour) {
        if (change->gen_sp_armour < 0)
            op->gen_sp_armour = (-change->gen_sp_armour);
        else
            op->gen_sp_armour = (signed char)(((int)op->gen_sp_armour*((int)change->gen_sp_armour))/(int)100);
    }
    op->value *= change->value;

    if (change->material)
        op->material = change->material;

    if (change->materialname) {
        if (op->materialname)
            free_string(op->materialname);
        op->materialname = add_refcount(change->materialname);
    }

    if (change->slaying) {
        if (op->slaying)
            free_string(op->slaying);
        op->slaying = add_refcount(change->slaying);
    }
    if (change->race) {
        if (op->race)
            free_string(op->race);
        op->race = add_refcount(change->race);
    }
    if (change->msg)
        object_set_msg(op, change->msg);

    if (change->inv) {
        object *copy;

        FOR_INV_PREPARE(change, inv) {
            copy = object_new();
            object_copy(inv, copy);
            object_insert_in_ob(copy, op);
        } FOR_INV_FINISH();
    }
}

/**
 * Searches the artifact lists and returns one that has the same type
 * of objects on it, non-const version of find_artifactlist() used only
 * during artifact loading.
 *
 * @return
 * NULL if no suitable list found.
 */
static artifactlist *find_artifactlist_internal(int type) {
    artifactlist *al;

    for (al = first_artifactlist; al != NULL; al = al->next)
        if (al->type == type)
            return al;
    return NULL;
}

/**
 * Builds up the lists of artifacts from the file in the libdir.
 * Can be called multiple times without ill effects.
 */
void init_artifacts(void) {
    static int has_been_inited = 0;
    FILE *fp;
    char filename[MAX_BUF], buf[HUGE_BUF], *cp, *next;
    artifact *art = NULL;
    linked_char *tmp;
    int value;
    artifactlist *al;
    archetype dummy_archetype;

    memset(&dummy_archetype, 0, sizeof(archetype));

    if (has_been_inited)
        return;
    else
        has_been_inited = 1;

    artifact_init = 1;

    snprintf(filename, sizeof(filename), "%s/artifacts", settings.datadir);
    LOG(llevDebug, "Reading artifacts from %s...\n", filename);
    if ((fp = fopen(filename, "r")) == NULL) {
        LOG(llevError, "Can't open %s.\n", filename);
        return;
    }

    while (fgets(buf, HUGE_BUF, fp) != NULL) {
        if (*buf == '#')
            continue;
        if ((cp = strchr(buf, '\n')) != NULL)
            *cp = '\0';
        cp = buf;
        while (*cp == ' ') /* Skip blanks */
            cp++;
        if (*cp == '\0')
            continue;

        if (!strncmp(cp, "Allowed", 7)) {
            if (art == NULL) {
                art = get_empty_artifact();
                nrofartifacts++;
            }

            cp = strchr(cp, ' ')+1;
            while (*(cp+strlen(cp)-1) == ' ')
                cp[strlen(cp)-1] = '\0';

            if (!strcmp(cp, "all"))
                continue;

            do {
                while (*cp == ' ')
                    cp++;
                nrofallowedstr++;
                if ((next = strchr(cp, ',')) != NULL)
                    *(next++) = '\0';
                tmp = (linked_char *)malloc(sizeof(linked_char));
                tmp->name = add_string(cp);
                tmp->next = art->allowed;
                art->allowed = tmp;
                art->allowed_size++;
            } while ((cp = next) != NULL);
        } else if (sscanf(cp, "chance %d", &value))
            art->chance = (uint16)value;
        else if (sscanf(cp, "difficulty %d", &value))
            art->difficulty = (uint8)value;
        else if (!strncmp(cp, "Object", 6)) {
            art->item = (object *)calloc(1, sizeof(object));
            if (art->item == NULL) {
                LOG(llevError, "init_artifacts: memory allocation failure.\n");
                abort();
            }
            object_reset(art->item);
            art->item->arch = &dummy_archetype;
            if (!load_object(fp, art->item, LO_LINEMODE, MAP_STYLE))
                LOG(llevError, "Init_Artifacts: Could not load object.\n");
            art->item->arch = NULL;
            art->item->name = add_string((strchr(cp, ' ')+1));
            al = find_artifactlist_internal(art->item->type);
            if (al == NULL) {
                al = get_empty_artifactlist();
                al->type = art->item->type;
                al->next = first_artifactlist;
                first_artifactlist = al;
            }
            art->next = al->items;
            al->items = art;
            art = NULL;
        } else
            LOG(llevError, "Unknown input in artifact file: %s\n", buf);
    }

    fclose(fp);

    for (al = first_artifactlist; al != NULL; al = al->next) {
        for (art = al->items; art != NULL; art = art->next) {
            if (!art->chance)
                LOG(llevError, "Warning: artifact with no chance: %s\n", art->item->name);
            else
                al->total_chance += art->chance;
        }
#if 0
        LOG(llevDebug, "Artifact list type %d has %d total chance\n", al->type, al->total_chance);
#endif
    }

    LOG(llevDebug, "done artifacts.\n");
    artifact_init = 0;
}

/**
 * Searches the artifact lists and returns one that has the same type
 * of objects on it.
 *
 * @return
 * NULL if no suitable list found.
 */
const artifactlist *find_artifactlist(int type) {
    return find_artifactlist_internal(type);
}

/**
 * Searches and returns a specific artifact, NULL if not found.
 * @param op item to search for.
 * @param name artifact name.
 * @return matching artifact, NULL if none matched.
 */
const artifact *find_artifact(const object *op, const char *name) {
    artifactlist *list;
    artifact *at;
    sstring sname = find_string(name);

    if (sname == NULL)
        return NULL;

    list = find_artifactlist_internal(op->type);
    if (list == NULL)
        return NULL;

    for (at = list->items; at != NULL; at = at->next) {
        if (at->item->name == sname && legal_artifact_combination(op, at))
            return at;
    }

    return NULL;
}

/**
 * For debugging purposes.  Dumps all tables.
 *
 * @todo
 * use LOG() instead of fprintf.
 */
void dump_artifacts(void) {
    artifactlist *al;
    artifact *art;
    linked_char *next;

    fprintf(logfile, "\n");
    for (al = first_artifactlist; al != NULL; al = al->next) {
        fprintf(logfile, "Artifact has type %d, total_chance=%d\n", al->type, al->total_chance);
        for (art = al->items; art != NULL; art = art->next) {
            fprintf(logfile, "Artifact %-30s Difficulty %3d Chance %5d\n", art->item->name, art->difficulty, art->chance);
            if (art->allowed != NULL) {
                fprintf(logfile, "\tAllowed combinations:");
                for (next = art->allowed; next != NULL; next = next->next)
                    fprintf(logfile, "%s,", next->name);
                fprintf(logfile, "\n");
            }
        }
    }
    fprintf(logfile, "\n");
}

/**
 * Get a suitable face number for representing an artifact.
 * @param art what to get the face of.
 * @return face, -1 as unsigned if none could be found.
 */
unsigned artifact_get_face(const artifact *art) {
    const archetype *arch = first_archetype;

    if (art->item->face != blank_face && art->item->face != NULL)
        return art->item->face->number;

    if (art->allowed_size > 0) {
        if (art->allowed->name[0] == '!') {
            linked_char *allowed;
            while (arch) {
                if (arch->clone.type != art->item->type)
                    arch = arch->next;

                for (allowed = art->allowed; allowed != NULL; allowed = allowed->next) {
                    if (strcmp(arch->name, allowed->name + 1) == 0) {
                        break;
                    }
                }
                if (allowed != NULL)
                    continue;

                if (arch->clone.face == NULL)
                    continue;
                return arch->clone.face->number;
            }
            return (unsigned)-1;
        } else {
            const archetype *arch = find_archetype(art->allowed->name);
            if (arch != NULL)
                return arch->clone.face->number;
            return (unsigned)-1;
        }
    }

    while (arch != NULL) {
        if (arch->clone.type == art->item->type && arch->clone.face != NULL)
            return arch->clone.face->number;

        arch = arch->next;
    }
    return (unsigned)-1;
}
