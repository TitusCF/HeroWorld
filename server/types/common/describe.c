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
/** @file describe.c
 * This file contains description-related methods that are common to many
 * classes of objects.
 */
#include <global.h>
#include <ob_methods.h>
#include <ob_types.h>

/**
 * Describes an object, seen by a given observer.
 * @param context The method context
 * @param op The object to describe
 * @param observer The object to make the description to
 * @param buf Buffer that will contain the description
 * @param size buf's size.
 */
void common_ob_describe(const ob_methods *context, const object *op, const object *observer, char *buf, size_t size) {
    char *desc;

    buf[0] = '\0';

    if (op == NULL)
        return;

    desc = stringbuffer_finish(describe_item(op, observer, NULL));
    if (desc[0] != '\0') {
        size_t len;

        query_name(op, buf, size-1);
        buf[size-1] = 0;
        len = strlen(buf);
        if ((len+5) < size) {
            /* Since we know the length, we save a few cpu cycles by using
             * it instead of calling strcat */
            strcpy(buf+len, " ");
            len++;
            strncpy(buf+len, desc, size-len-1);
            buf[size-1] = 0;
        }
    }
    free(desc);
    if (buf[0] == '\0') {
        query_name(op, buf, size-1);
        buf[size-1] = 0;
    }
}
