/*
    CrossFire, A Multiplayer game for X-windows

    Copyright (C) 2008 Crossfire Development Team

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
 * @file
 * The implementation of @ref page_type_83 "duplicator" objects.
 */

#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static method_ret duplicator_type_trigger(ob_methods *context, object *op, object *cause, int state);

/**
 * Initializer for the @ref page_type_83 "duplicator" object type.
 */
void init_type_duplicator(void) {
    register_trigger(DUPLICATOR, duplicator_type_trigger);
}

/**
 * Trigger for @ref page_type_83 "duplicator".
 *
 * Will duplicate a specified object placed on top of it.
 * - connected: what will trigger it.
 * - level: multiplier.  0 to destroy.
 * - other_arch: the object to look for and duplicate.
 *
 * @param op
 * duplicator.
 */
static void move_duplicator(object *op) {
    object *tmp;

    if (!op->other_arch) {
        LOG(llevInfo, "Duplicator with no other_arch! %d %d %s\n", op->x, op->y, op->map ? op->map->path : "nullmap");
        return;
    }

    if (op->above == NULL)
        return;
    for (tmp = op->above; tmp != NULL; tmp = tmp->above) {
        if (strcmp(op->other_arch->name, tmp->arch->name) == 0) {
            if (op->level <= 0) {
                object_remove(tmp);
                object_free_drop_inventory(tmp);
            } else {
                uint64 new_nrof = (uint64)tmp->nrof*op->level;

                if (new_nrof >= 1UL<<31)
                    new_nrof = 1UL<<31;
                tmp->nrof = new_nrof;
            }
            break;
        }
    }
}

/**
 * A @ref page_type_83 "duplicator" is triggered.
 * @param context Ignored.
 * @param op The duplicator being triggered
 * @param cause Ignored.
 * @param state Ignored.
 * @retval METHOD_OK
 */
static method_ret duplicator_type_trigger(ob_methods *context, object *op, object *cause, int state) {
    move_duplicator(op);
    return METHOD_OK;
}
