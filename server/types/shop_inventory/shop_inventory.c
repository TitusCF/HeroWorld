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

/** @file shop_inventory.c
 * Implementation of the shop inventory class of objects.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static method_ret shop_inventory_type_apply(ob_methods *context, object *lighter, object *applier, int aflags);

/**
 * Initializer for the SHOP_INVENTORY object type.
 */
void init_type_shop_inventory(void) {
    register_apply(SHOP_INVENTORY, shop_inventory_type_apply);
}

/**
 * One item to list on the sign.
 */
typedef struct shopinv {
    char        *item_sort;     /**< Singular name. */
    char        *item_real;     /**< Plural name. */
    uint16      type;           /**< Item type. */
    uint32      nrof;           /**< Count of this items. */
} shopinv;

/**
 * Sort routine for shopinv.
 * There are a lot of extra casts in here just to suppress warnings - it
 * makes it look uglier than it really it.
 * The format of the strings we get is type:name.  So we first want to
 * sort by type (numerical) - if the same type, then sort by name.
 *
 * @param a1
 * @param a2
 * items to compare.
 * @return
 * -1 is a1 is less than a2, 1 if the opposite, 0 if equals.
 */
static int shop_sort(const void *a1, const void *a2) {
    const shopinv *s1 = (const shopinv *)a1, *s2 = (const shopinv *)a2;

    if (s1->type < s2->type)
        return -1;
    if (s1->type > s2->type)
        return 1;
    /* the type is the same (what atoi gets), so do a strcasecmp to sort
     * via alphabetical order
     */
    return strcasecmp(s1->item_sort, s2->item_sort);
}

/**
 * Insert the item in the list.
 * @param tmp
 * object to insert. Must have FLAG_UNPAID set.
 * @param items
 * array of items, should have (*numitems)+1 items allocated.
 * @param numitems
 * how many items items contains.
 */
static void add_shop_item(object *tmp, shopinv *items, size_t *numitems) {
    /* clear unpaid flag so that doesn't come up in query
     * string.  We clear nrof so that we can better sort
     * the object names.
     */
    char name[MAX_BUF];

    CLEAR_FLAG(tmp, FLAG_UNPAID);
    items[*numitems].nrof = tmp->nrof;
    /* Non mergable items have nrof of 0, but count them as one
     * so the display is properly.
     */
    if (tmp->nrof == 0)
        items[*numitems].nrof++;
    items[*numitems].type = tmp->type;
    query_base_name(tmp, 0, name, MAX_BUF);
    items[*numitems].item_sort = strdup_local(name);
    query_base_name(tmp, 1, name, MAX_BUF);
    items[*numitems].item_real = strdup_local(name);
    (*numitems)++;

    SET_FLAG(tmp, FLAG_UNPAID);
}

/**
 * Apply a shop inventory.
 *
 * @param context
 * method context.
 * @param lighter
 * applied object to apply.
 * @param applier
 * object attempting to apply the inventory. Should be a player.
 * @param aflags
 * special flags (always apply/unapply).
 * @return
 * METHOD_OK if applier is a player, METHOD_UNHANDLED else.
 */
static method_ret shop_inventory_type_apply(ob_methods *context, object *lighter, object *applier, int aflags) {
    size_t i, j, numitems = 0, numallocated = 0;
    object *stack;
    shopinv *items;

    if (applier->type != PLAYER)
        return METHOD_UNHANDLED;

    draw_ext_info(NDI_UNIQUE, 0, applier, MSG_TYPE_SHOP, MSG_TYPE_SHOP_LISTING,
        "\nThe shop contains:");

    items = malloc(40*sizeof(shopinv));
    numallocated = 40;

    /* Find all the appropriate items */
    for (i = 0; i < MAP_WIDTH(applier->map); i++) {
        for (j = 0; j < MAP_HEIGHT(applier->map); j++) {
            stack = GET_MAP_OB(applier->map, i, j);

            while (stack) {
                if (QUERY_FLAG(stack, FLAG_UNPAID)) {
                    if (numitems == numallocated) {
                        items = realloc(items, sizeof(shopinv)*(numallocated+10));
                        numallocated += 10;
                    }
                    add_shop_item(stack, items, &numitems);
                }
                stack = stack->above;
            }
        }
    }
    if (numitems == 0) {
        draw_ext_info(NDI_UNIQUE, 0, applier, MSG_TYPE_SHOP, MSG_TYPE_SHOP_LISTING,
            "The shop is currently empty.\n");
        free(items);
        return METHOD_OK;
    }
    qsort(items, numitems, sizeof(shopinv), (int (*)(const void *, const void *))shop_sort);

    for (i = 0; i < numitems; i++) {
        /* Collapse items of the same name together */
        if ((i+1) < numitems && !strcmp(items[i].item_real, items[i+1].item_real)) {
            items[i+1].nrof += items[i].nrof;
            free(items[i].item_sort);
            free(items[i].item_real);
        } else {
            draw_ext_info_format(NDI_UNIQUE, 0, applier, MSG_TYPE_SHOP, MSG_TYPE_SHOP_LISTING,
                "%d %s",
                items[i].nrof ? items[i].nrof : 1,
                items[i].nrof == 1 ? items[i].item_sort : items[i].item_real);
            free(items[i].item_sort);
            free(items[i].item_real);
        }
    }
    free(items);
    return METHOD_OK;
}
