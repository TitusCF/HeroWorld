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

/** @file converter.c
 * The implementation of the Converter class of objects.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

/*
 * convert_item() returns 1 if anything was converted, 0 if the item was not
 * what the converter wants, -1 if the converter is broken.
 */
#define CONV_FROM(xyz)  xyz->slaying
#define CONV_TO(xyz)    xyz->other_arch
#define CONV_NR(xyz)    (unsigned char)xyz->stats.sp
#define CONV_NEED(xyz)  (unsigned long)xyz->stats.food

static int convert_item(object *item, object *converter);

static method_ret converter_type_move_on(ob_methods *context, object *trap, object *victim, object *originator);

/**
 * Initializer for the CONVERTER object type.
 */
void init_type_converter(void) {
    register_move_on(CONVERTER, converter_type_move_on);
}

/**
 * Transforms an item into another item.
 * @param item The object that triggered the converter - if it isn't of a type
 * accepted by the converter, nothing will happen
 * @param converter The object that is doing the conversion
 * @retval -1 If something went wrong when attempting the conversion
 * @retval 0 If the item was not converted
 * @retval 1 If the item got converted
 */
static int convert_item(object *item, object *converter) {
    int nr = 0;
    uint32 price_in;

    /* We make some assumptions - we assume if it takes money as it type,
     * it wants some amount.  We don't make change (ie, if something costs
     * 3 gp and player drops a platinum, tough luck)
     */
    if (!strcmp(CONV_FROM(converter), "money")) {
        int cost;

        if (item->type != MONEY)
            return 0;

        nr = (item->nrof*item->value)/CONV_NEED(converter);
        if (!nr)
            return 0;

        cost = nr*CONV_NEED(converter)/item->value;
        /* take into account rounding errors */
        if (nr*CONV_NEED(converter)%item->value)
            cost++;
        object_decrease_nrof(item, cost);

        price_in = cost*item->value;
    } else {
        if (item->type == PLAYER
        || CONV_FROM(converter) != item->arch->name
        || (CONV_NEED(converter) && CONV_NEED(converter) > item->nrof))
            return 0;

        /* silently burn unpaid items (only if they match what we want) */
        if (QUERY_FLAG(item, FLAG_UNPAID)) {
            object_remove(item);
            object_free_drop_inventory(item);
            item = create_archetype("burnout");
            if (item != NULL)
                object_insert_in_map_at(item, converter->map, converter, 0, converter->x, converter->y);
            return 1;
        }

        if (CONV_NEED(converter)) {
            nr = item->nrof/CONV_NEED(converter);
            object_decrease_nrof(item, nr*CONV_NEED(converter));
            price_in = nr*CONV_NEED(converter)*item->value;
        } else {
            price_in = item->value;
            object_remove(item);
            object_free_drop_inventory(item);
        }
    }

    if (converter->inv != NULL) {
        int i;
        object *ob_to_copy;

        /* select random object from inventory to copy */
        ob_to_copy = converter->inv;
        i = 1;
        FOR_BELOW_PREPARE(converter->inv, ob) {
            if (rndm(0, i) == 0)
                ob_to_copy = ob;
            i++;
        } FOR_BELOW_FINISH();
        item = object_create_clone(ob_to_copy);
        CLEAR_FLAG(item, FLAG_IS_A_TEMPLATE);
        object_unset_flag_inv(item, FLAG_IS_A_TEMPLATE);
    } else {
        if (converter->other_arch == NULL) {
            LOG(llevError, "move_creator: Converter doesn't have other arch set: %s (%s, %d, %d)\n", converter->name ? converter->name : "(null)", converter->map->path, converter->x, converter->y);
            return -1;
        }
        item = object_create_arch(converter->other_arch);
        fix_generated_item(item, converter, 0, 0, GT_MINIMAL);
    }

    if (CONV_NR(converter))
        item->nrof = CONV_NR(converter);
    if (nr)
        item->nrof *= nr;
    if (item->type != MONEY && is_in_shop(converter))
        SET_FLAG(item, FLAG_UNPAID);
    else if (price_in < item->nrof*item->value && settings.allow_broken_converters == FALSE) {
        LOG(llevError, "Broken converter %s at %s (%d, %d) in value %d, out value %d for %s\n", converter->name, converter->map->path, converter->x, converter->y, price_in, item->nrof*item->value, item->name);
        object_free_drop_inventory(item);
        return -1;
    }
    object_insert_in_map_at(item, converter->map, converter, 0, converter->x, converter->y);
    return 1;
}

/**
 * Move on this Converter object.
 * @param context The method context
 * @param trap The Converter we're moving on
 * @param victim The object moving over this one
 * @param originator The object that caused the move_on event
 * @return METHOD_OK
 */
static method_ret converter_type_move_on(ob_methods *context, object *trap, object *victim, object *originator) {
    if (common_pre_ob_move_on(trap, victim, originator) == METHOD_ERROR)
        return METHOD_OK;
    if (convert_item(victim, trap) < 0) {
        object *op;
        char name[MAX_BUF];

        query_name(trap, name, MAX_BUF);
        draw_ext_info_format(NDI_UNIQUE, 0, originator, MSG_TYPE_APPLY, MSG_TYPE_APPLY_FAILURE,
            "The %s seems to be broken!", name);

        op = create_archetype("burnout");
        if (op != NULL)
            object_insert_in_map_at(op, trap->map, trap, 0, trap->x, trap->y);
    }
    common_post_ob_move_on(trap, victim, originator);
    return METHOD_OK;
}
