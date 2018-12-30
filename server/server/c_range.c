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
 * Range related commands (casting, shooting, throwing, etc.).
 */

#include <global.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#endif
#include <spells.h>
#include <skills.h>
#include <shared/newclient.h>
#include <commands.h>

/**
 * 'invoke' command, fires a spell immediately.
 *
 * @param op
 * player.
 * @param params
 * spell.
 */
void command_invoke(object *op, const char *params) {
    command_cast_spell(op, params, 'i');
}

/**
 * 'cast' command, prepares a spell for laster casting.
 *
 * @param op
 * player.
 * @param params
 * spell.
 */
void command_cast(object *op, const char *params) {
    command_cast_spell(op, params, 'c');
}

/**
 * Equivalent to command_cast().
 *
 * @param op
 * player.
 * @param params
 * spell.
 * @todo remove.
 */
void command_prepare(object *op, const char *params) {
    command_cast_spell(op, params, 'p');
}

/**
 * Shows all spells that op knows.
 *
 * Given there is more than one skill, we can't supply break
 * them down to cleric/wizardry.
 *
 * @param op
 * player wanting to knows her spells.
 * @param params
 * if supplied, the spell name must match that.
 */
static void show_matching_spells(object *op, const char *params) {
    char spell_sort[NROFREALSPELLS][MAX_BUF], tmp[MAX_BUF], *cp;
    int num_found = 0, i;

    /* We go and see what spells the player has.  We put them
     * into the spell_sort array so that we can sort them -
     * we prefix the skill in the name so that the sorting
     * works better.
     */
    FOR_INV_PREPARE(op, spell) {
        /* If it is a spell, and no params are passed, or they
         * match the name, process this spell.
         */
        if (spell->type == SPELL
        && (*params == '\0' || !strncmp(params, spell->name, strlen(params)))) {
            if (spell->path_attuned&op->path_denied) {
                snprintf(spell_sort[num_found++], sizeof(spell_sort[0]),
                         "%s:%-22s %3s %3s", spell->skill ? spell->skill : "generic",
                         spell->name, "den", "den");
            } else {
                snprintf(spell_sort[num_found++], sizeof(spell_sort[0]),
                         "%s:%-22s %3d %3d", spell->skill ? spell->skill : "generic",
                         spell->name, spell->level,
                         SP_level_spellpoint_cost(op, spell, SPELL_HIGHEST));
            }
        }
    } FOR_INV_FINISH();
    if (!num_found) {
        if (*params != '\0')
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                          "You know no spells like '%s'.", params);
        else
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                          "You know no spells.");

        return;
    }

    if (*params != '\0')
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                      "You know the following '%s' spells:", params);
    else
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                      "You know the following spells:");

    /* Note in the code below that we make some
     * presumptions that there will be a colon in the
     * string.  given the code above, this is always
     * the case.
     */
    qsort(spell_sort, num_found, MAX_BUF, (int (*)(const void *, const void *))strcmp);
    strcpy(tmp, "asdfg"); /* Dummy string so initial compare fails */
    for (i = 0; i < num_found; i++) {
        /* Different skill name, so print banner */
        if (strncmp(tmp, spell_sort[i], strlen(tmp))) {
            strcpy(tmp, spell_sort[i]);
            cp = strchr(tmp, ':');
            *cp = '\0';

            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                                 "\n[fixed]%s spells %.*s <lvl> <sp>",
                                 tmp, (int)(12-strlen(tmp)), "              ");
        }
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                             "[fixed]%s",
                             strchr(spell_sort[i], ':')+1);
    }
}

/**
 * Sets up to cast a spell.
 *
 * Invoke casts a spell immediately, whereas cast just set up the range type.
 *
 * @param op
 * caster.
 * @param params
 * spell name.
 * @param command
 * first letter of the spell type (c=cast, i=invoke, p=prepare).
 */
void command_cast_spell(object *op, const char *params, char command) {
    int castnow = 0;
    char *cp, cpy[MAX_BUF];
    object *spob;

    strncpy(cpy, params, sizeof(cpy));

    if (command == 'i')
        castnow = 1;

    if (*cpy != '\0') {
        tag_t spellnumber = 0;
        if ((spellnumber = atoi(cpy)) != 0)
            spob = object_find_by_tag(op, spellnumber);
        else
            spob = lookup_spell_by_name(op, cpy);

        if (spob && spob->type == SPELL) {
            /* Now grab any extra data, if there is any.  Forward pass
             * any 'of' delimiter
             */
            if (spellnumber) {
                /* if we passed a number, the options start at the second word */
                cp = strchr(cpy, ' ');
                if (cp) {
                    cp++;
                    if (!strncmp(cp, "of ", 3))
                        cp += 3;
                }
            } else if (strlen(cpy) > strlen(spob->name)) {
                cp = cpy+strlen(spob->name);
                *cp = 0;
                cp++;
                if (!strncmp(cp, "of ", 3))
                    cp += 3;
            } else
                cp = NULL;

            if (spob->skill && !find_skill_by_name(op, spob->skill)) {
                draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_MISSING,
                                     "You need the skill %s to cast %s!",
                                     spob->skill, spob->name);
                return;
            }

            /* Remove control of the golem */
            if (op->contr->ranges[range_golem] != NULL) {
                if (op->contr->golem_count == op->contr->ranges[range_golem]->count) {
                    remove_friendly_object(op->contr->ranges[range_golem]);
                    object_remove(op->contr->ranges[range_golem]);
                    object_free_drop_inventory(op->contr->ranges[range_golem]);
                }
                op->contr->ranges[range_golem] = NULL;
                op->contr->golem_count = 0;
            }

            /* This assignment is need for casting_time logic */
            op->spell = spob;
            if (castnow) {
                cast_spell(op, op, op->facing, spob, cp);
            } else {
                /** @todo present the list nicely instead of comma-separated simply */
                sstring required = object_get_value(spob, "casting_requirements");
                op->contr->ranges[range_magic] = spob;
                op->contr->shoottype = range_magic;

                if (cp != NULL) {
                    strncpy(op->contr->spellparam, cp, MAX_BUF);
                    op->contr->spellparam[MAX_BUF-1] = '\0';
                } else {
                    op->contr->spellparam[0] = '\0';
                }
                draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                                     "You ready the spell %s%s%s",
                                     spob->name, required ? " which consumes for each invocation " : "", required ? required : "");
            }
            return;
        } /* else fall through to below and print spells */
    } /* params supplied */

    /* We get here if cast was given without options or we could not find
     * the requested spell.  List all the spells the player knows.
     */
    show_matching_spells(op, cpy);
}

/**************************************************************************/

/**
 * Check for the validity of a player range.
 *
 * This function could probably be simplified, eg, everything
 * should index into the ranges[] array.
 *
 * @param op
 * player to check.
 * @param r
 * range to check.
 *
 * @retval 1
 * range specified is legal - that is, the character has an item that is equipped for that range type.
 * @retval 0
 * no item of that range type that is usable.
 */

int legal_range(object *op, int r) {
    switch (r) {
    case range_none: /* "Nothing" is always legal */
        return 1;

    case range_bow:
    case range_misc:
    case range_magic: /* cast spells */
        if (op->contr->ranges[r])
            return 1;
        else
            return 0;

    case range_golem: /* Use scrolls */
        if (op->contr->ranges[range_golem]
        && op->contr->ranges[range_golem]->count == op->contr->golem_count)
            return 1;
        else
            return 0;

    case range_skill:
        if (op->chosen_skill)
            return 1;
        else
            return 0;
    }
    /* No match above, must not be valid */
    return 0;
}

/**
 * Rotate the selected range attack.
 *
 * @param op
 * player.
 * @param k
 * '+' selects next range, other values previous range.
 */
static void change_spell(object *op, char k) {
    char name[MAX_BUF];

    do {
        op->contr->shoottype += ((k == '+') ? 1 : -1);
        if (op->contr->shoottype >= range_size)
            op->contr->shoottype = range_none;
        else if (op->contr->shoottype <= range_bottom)
            op->contr->shoottype = (rangetype)(range_size-1);
    } while (!legal_range(op, op->contr->shoottype));

    /* Legal range has already checked that we have an appropriate item
     * that uses the slot, so we don't need to be too careful about
     * checking the status of the object.
     */
    switch (op->contr->shoottype) {
    case range_none:
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "No ranged attack chosen.");
        break;

    case range_golem:
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                      "You regain control of your golem.");
        break;

    case range_bow:
        query_name(op->contr->ranges[range_bow], name, MAX_BUF);
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                             "Switched to %s and %s.",
                             name,
                             op->contr->ranges[range_bow]->race ? op->contr->ranges[range_bow]->race : "nothing");
        break;

    case range_magic:
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                             "Switched to spells (%s).",
                             op->contr->ranges[range_magic]->name);
        break;

    case range_misc:
        query_base_name(op->contr->ranges[range_misc], 0, name, MAX_BUF);
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                             "Switched to %s.",
                             name);
        break;

    case range_skill:
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                             "Switched to skill: %s",
                             op->chosen_skill ? op->chosen_skill->name : "none");
        break;

    default:
        break;
    }
}

/**
 * 'rotateshoottype' command, switch range attack.
 *
 * @param op
 * player.
 * @param params
 * arguments to the command.
 */
void command_rotateshoottype(object *op, const char *params) {
    if (*params == '\0')
        change_spell(op, '+');
    else
        change_spell(op, params[0]);
}
