/*
    CrossFire, A Multiplayer game for X-windows

    Copyright (C) 2007 Crossfire Development Team
    Copyright (C) 1992 Frank Tore Johansen

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    The authors can be reached via e-mail at crossfire-devel@real-time.com
*/

/**
 * @file spellbook.c
 * Implimentation of spellbooks.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

#include "living.h"

static method_ret spellbook_type_apply(ob_methods *context, object *lighter, object *applier, int aflags);

/**
 * Initializer for the SPELLBOOK object type.
 */
void init_type_spellbook(void) {
    register_apply(SPELLBOOK, spellbook_type_apply);
}

/**
 * Applies a spellbook.
 * Checks whether player has knowledge of required skill, doesn't
 * already know the spell, stuff like that. Random learning failure too.

 * @param context
 * method context.
 * @param book
 * Spellbook to apply.
 * @param applier
 * object attempting to apply the spellbook. Should be a player.
 * @param aflags
 * special flags (always apply/unapply).
 * @return
 * METHOD_OK always.
 *
 * @todo
 * handle failure differently for praying/magic.
 * @todo
 * split into multiple functions
 */
static method_ret spellbook_type_apply(ob_methods *context, object *book, object *applier, int aflags) {
    object *skapplier, *spell, *spell_skill;
    int read_level;
    char level[100];

    /* Must be applied by a player. */
    if (applier->type == PLAYER) {
        if (QUERY_FLAG(applier, FLAG_BLIND) && !QUERY_FLAG(applier, FLAG_WIZ)) {
            draw_ext_info(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
                "You are unable to read while blind.");
            return METHOD_OK;
        }

        skapplier = find_skill_by_name(applier, book->skill);

        /* need a literacy skill to learn spells. Also, having a literacy level
         * lower than the spell will make learning the spell more difficult */
        if (!skapplier) {
            draw_ext_info(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
                "You can't read! Your attempt fails.");
            return METHOD_OK;
        }
        read_level = skapplier->level;

        spell = book->inv;
        if (!spell) {
            LOG(llevError, "apply_spellbook: Book %s has no spell in it!\n", book->name);
            draw_ext_info(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_FAILURE,
                "The spellbook symbols make no sense.");
            return METHOD_OK;
        }

        if (QUERY_FLAG(book, FLAG_CURSED) || QUERY_FLAG(book, FLAG_DAMNED)) {
            char name[MAX_BUF];
            /* Player made a mistake, let's shake her/him :) */
            int failure = -35;

            if (settings.spell_failure_effects == TRUE)
                failure = -rndm(35, 100);
            query_name(book, name, MAX_BUF);
            draw_ext_info_format(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
                "The %s was %s!",
                name, QUERY_FLAG(book, FLAG_DAMNED) ? "damned" : "cursed");
            scroll_failure(applier, failure, (spell->level+4)*7);
            if (QUERY_FLAG(book, FLAG_DAMNED)
            && check_spell_known(applier, spell->name)
            && die_roll(1, 10, applier, 1) < 2)
                /* Really unlucky player, better luck next time */
                do_forget_spell(applier, spell->name);
            book = object_decrease_nrof_by_one(book);
            if (book && (!QUERY_FLAG(book, FLAG_IDENTIFIED))) {
                /* Well, not everything is lost, player now knows the
                 * book is cursed/damned. */
                book = identify(book);
                if (book->env)
                    esrv_update_item(UPD_FLAGS|UPD_NAME, applier, book);
                else
                    applier->contr->socket.update_look = 1;
            }
            return METHOD_OK;
        }

        if (QUERY_FLAG(book, FLAG_BLESSED))
            read_level += 5;

        if (spell->level > (read_level+10)) {
            draw_ext_info(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_FAILURE,
                "You are unable to decipher the strange symbols.");
            return METHOD_OK;
        }

        get_levelnumber(spell->level, level, sizeof(level));
        draw_ext_info_format(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
            "The spellbook contains the %s level spell %s.",
            level, spell->name);

        if (!QUERY_FLAG(book, FLAG_IDENTIFIED)) {
            book = identify(book);
            if (book->env)
                esrv_update_item(UPD_FLAGS|UPD_NAME, applier, book);
            else
                applier->contr->socket.update_look = 1;
            spell = book->inv;
        }

        /* I removed the check for special_prayer_mark here - it didn't make
         * a lot of sense - special prayers are not found in spellbooks, and
         * if the player doesn't know the spell, doesn't make a lot of sense
         * that they would have a special prayer mark.
         */
        if (check_spell_known(applier, spell->name)) {
            draw_ext_info(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_FAILURE,
                "You already know that spell.\n");
            return METHOD_OK;
        }

        if (spell->skill) {
            spell_skill = find_skill_by_name(applier, spell->skill);
            if (!spell_skill) {
                draw_ext_info_format(NDI_UNIQUE, 0, applier,
                    MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
                    "You lack the skill %s to use this spell",
                    spell->skill);
                return METHOD_OK;
            }
            if (spell_skill->level < spell->level) {
                draw_ext_info_format(NDI_UNIQUE, 0, applier,
                    MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
                    "You need to be level %d in %s to learn this spell.",
                    spell->level, spell->skill);
                return METHOD_OK;
            }
        }

        /* Logic as follows
         *
         *  1- MU spells use Int to learn, Cleric spells use Wisdom
         *
         *  2- The learner's skill level in literacy adjusts the chance
         *        to learn a spell.
         *
         *  3 -Automatically fail to learn if you read while confused
         *
         * Overall, chances are the same but a player will find having a high
         * literacy rate very useful!  -b.t.
         */
        if (QUERY_FLAG(applier, FLAG_CONFUSED)) {
            draw_ext_info(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_FAILURE,
                "In your confused state you flub the wording of the text!");
            scroll_failure(applier, 0-random_roll(0, spell->level, applier, PREFER_LOW), MAX(spell->stats.sp, spell->stats.grace));
        } else if (QUERY_FLAG(book, FLAG_STARTEQUIP)
        || (random_roll(0, 100, applier, PREFER_LOW)-(5*read_level)) < get_learn_spell(spell->stats.grace ? applier->stats.Wis : applier->stats.Int)) {
            draw_ext_info(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
                "You succeed in learning the spell!");
            do_learn_spell(applier, spell, 0);

            /* xp gain to literacy for spell learning */
            if (!QUERY_FLAG(book, FLAG_STARTEQUIP))
                change_exp(applier, calc_skill_exp(applier, book, skapplier), skapplier->skill, 0);
        } else {
            play_sound_player_only(applier->contr, SOUND_TYPE_SPELL, book, 0, "fumble");
            draw_ext_info(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_FAILURE,
                "You fail to learn the spell.\n");
        }
        object_decrease_nrof_by_one(book);
    }
    return METHOD_OK;
}
