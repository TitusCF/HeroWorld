/*
    CrossFire, A Multiplayer game for X-windows

    Copyright (C) 2007 Mark Wedel & Crossfire Development Team
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
/** @file scroll.c
 * The implementation of the Scroll class of objects.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static method_ret scroll_type_apply(ob_methods *context, object *op, object *applier, int aflags);

/**
 * Initializer for the scroll object type.
 */
void init_type_scroll(void) {
    register_apply(SCROLL, scroll_type_apply);
}

/**
 * Handles reading a spell scroll.
 * @param context
 * The method context
 * @param scroll
 * The scroll to apply
 * @param applier
 * The object attempting to apply the scroll
 * @param aflags
 * Special flags (always apply/unapply)
 * @return METHOD_OK
 * @todo
 * should handle scroll failure differently if god-like scroll.
 * @todo
 * Tweak failure parameters.
 */
static method_ret scroll_type_apply(ob_methods *context, object *scroll,
    object *applier, int aflags) {
    object *skapplier;
    object *head;
    sstring name;

    head = HEAD(applier);

    if (QUERY_FLAG(applier, FLAG_BLIND) && !QUERY_FLAG(applier, FLAG_WIZ)) {
        draw_ext_info(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_ERROR,
            "You are unable to read while blind.");
        return METHOD_OK;
    }

    if (!scroll->inv || scroll->inv->type != SPELL) {
        draw_ext_info(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_FAILURE,
            "The scroll just doesn't make sense!");
        return METHOD_OK;
    }

    if (applier->type == PLAYER) {
        /* players need a literacy skill to read stuff! */
        int exp_gain;

        /* hard code literacy - scroll->skill points to where the exp
         * should go for anything killed by the spell.
         */
        skapplier = find_skill_by_name(applier, skill_names[SK_LITERACY]);

        if (!skapplier) {
            draw_ext_info(NDI_UNIQUE, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_FAILURE,
                "You are unable to decipher the strange symbols.");
            return METHOD_OK;
        }

        if (!QUERY_FLAG(scroll, FLAG_IDENTIFIED))
            scroll = identify(scroll);

        if (QUERY_FLAG(scroll, FLAG_CURSED) || QUERY_FLAG(scroll, FLAG_DAMNED)) {
            /* Player made a mistake, let's shake her/him :)
             * a failure of -35 means merely mana drain, -80 means
             * mana blast. But if server settings say 'no failure effect',
             * we still want to drain mana.
             * As for power, hey, better take care what you apply :)
             */
            int failure = -35;

            if (settings.spell_failure_effects == TRUE)
                failure = -rndm(35, 100);
            scroll_failure(applier, failure, MAX(20, (scroll->level-skapplier->level)*5));
            object_decrease_nrof_by_one(scroll);
            return METHOD_OK;
        }

        if ((exp_gain = calc_skill_exp(applier, scroll, skapplier)))
            change_exp(applier, exp_gain, skapplier->skill, 0);
    }

    /* need to keep the name, as the scroll may be destroyed when on the ground (reading a scroll of alchemy for instance) */
    name = scroll->inv->name;
    cast_spell(applier, scroll, head->facing, scroll->inv, NULL);

    if (QUERY_FLAG(scroll, FLAG_BLESSED) && die_roll(1, 100, applier, 1) < 10) {
        draw_ext_info_format(NDI_BLACK, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
            "Your scroll of %s glows for a second!",
            name);
    } else {
        draw_ext_info_format(NDI_BLACK, 0, applier, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
            "The scroll of %s turns to dust.",
            name);

        object_decrease_nrof_by_one(scroll);
    }
    return METHOD_OK;
}
