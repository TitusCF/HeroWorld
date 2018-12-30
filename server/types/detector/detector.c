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
 * The implementation of @ref page_type_51 "detector" objects.
 */

#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>
#include <sounds.h>
#include <sproto.h>

static method_ret detector_type_process(ob_methods *context, object *op);

/**
 * Initializer for the @ref page_type_51 "detector" object type.
 */
void init_type_detector(void) {
    register_process(DETECTOR, detector_type_process);
}

/**
 * Move a @ref page_type_51 "detector".
 *
 * - slaying:    name of the thing the detector is to look for
 * - speed:      frequency of 'glances'
 * - connected:  connected value of detector
 * - sp:         1 if detection sets buttons
 *              -1 if detection unsets buttons
 *
 * @param op
 * detector to move.
 */
static void move_detector(object *op) {
    object *tmp;
    int last = op->value;
    int detected;
    detected = 0;

    if (!op->slaying) {
        if (op->map)
            LOG(llevError, "Detector with no slaying set at %s (%d,%d)\n", op->map->path, op->x, op->y);
        else if (op->env)
            LOG(llevError, "Detector with no slaying in %s\n", op->env->name);
        else
            LOG(llevError, "Detector with no slaying nowhere?\n");
        op->speed = 0;
        object_update_speed(op);
        return;
    }

    for (tmp = GET_MAP_OB(op->map, op->x, op->y); tmp != NULL; tmp = tmp->above) {
        if (op->stats.hp) {
            if (object_find_by_name(tmp, op->slaying) != NULL
            || object_find_by_type_and_slaying(tmp, FORCE, op->slaying)) {
                detected = 1;
                break;
            }
        }
        if (op->slaying == tmp->name) {
            detected = 1;
            break;
        }
        if (tmp->type == PLAYER && !strcmp(op->slaying, "player")) {
            detected = 1;
            break;
        }
        if (tmp->type == SPECIAL_KEY && tmp->slaying == op->slaying) {
            detected = 1;
            break;
        }
    }

    /* the detector sets the button if detection is found */
    if (op->stats.sp == 1)  {
        if (detected && last == 0) {
            op->value = 1;
            push_button(op);
        }
        if (!detected && last == 1) {
            op->value = 0;
            push_button(op);
        }
    } else { /* in this case, we unset buttons */
        if (detected && last == 1) {
            op->value = 0;
            push_button(op);
        }
        if (!detected && last == 0) {
            op->value = 1;
            push_button(op);
        }
    }
}

/**
 * Processes a @ref page_type_51 "detector".
 * @param context The method context, ignored
 * @param op The detector to process
 * @retval METHOD_OK
 */
static method_ret detector_type_process(ob_methods *context, object *op) {
    move_detector(op);
    return METHOD_OK;
}
