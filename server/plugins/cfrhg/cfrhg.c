/*****************************************************************************/
/* Template for version 2.0 plugins.                                         */
/* Contact: yann.chachkoff@myrealbox.com                                     */
/*****************************************************************************/
/* That code is placed under the GNU General Public Licence (GPL)            */
/* (C)2001-2005 by Chachkoff Yann (Feel free to deliver your complaints)     */
/*****************************************************************************/
/*  CrossFire, A Multiplayer game for X-windows                              */
/*                                                                           */
/*  Copyright (C) 2008 the Crossfire development team                        */
/*                                                                           */
/*  This program is free software; you can redistribute it and/or modify     */
/*  it under the terms of the GNU General Public License as published by     */
/*  the Free Software Foundation; either version 2 of the License, or        */
/*  (at your option) any later version.                                      */
/*                                                                           */
/*  This program is distributed in the hope that it will be useful,          */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of           */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            */
/*  GNU General Public License for more details.                             */
/*                                                                           */
/*  You should have received a copy of the GNU General Public License        */
/*  along with this program; if not, write to the Free Software              */
/*  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                */
/*                                                                           */
/*****************************************************************************/

/**
 * @defgroup plugin_rhg Random house generation plugin
 * This plugin links unused exits to random maps.
 * The random map parameters vary based on the map it is in, but each exit will always use
 * the same random seed to have the same layout and content.
 *
 * Exits will only point to a random map if both their @ref obj::slaying "slaying" and
 * @ref obj::msg "msg" fields are empty.
 *
 * @todo
 * - make more parameters vary based on maps
 * - add exits to all towns
 *
 * @{
 */

/**
 * @file
 * This file is part of the @ref plugin_rhg "random house generation plugin".
 * See this page for more information.
 */

#include <stdarg.h>
#include <assert.h>

#include <cfrhg.h>
#ifndef __CEXTRACT__
#include <cfrhg_proto.h>
#endif
#include <svnversion.h>

CF_PLUGIN char SvnRevPlugin[] = SVN_REV;

/** Link between a map and the exits to generate for it. */
typedef struct house_zone_struct {
    const char *mappath;        /**< Full map path. */
    const char *monsterstyle;   /**< Style of monsters. */
} house_zone_struct;

/** Maps we work on. */
static const house_zone_struct zones[] = {
    /* Scorn */
    { "/world/world_104_115", "city" },
    { "/world/world_105_115", "city" },
    { "/world/world_104_116", "city" },
    { "/world/world_105_116", "city" },
    /* Navar */
    { "/world/world_122_116", "city" },
    { "/world/world_121_116", "city" },
    { "/world/world_122_117", "city" },
    { "/world/world_121_117", "city" },
    { NULL, NULL }
};

/**
 * Get the random map parameters of a map.
 * @param map
 * map to get the zone of.
 * @return
 * NULL if the map shouldn't be processed, else its parameters.
 */
static const house_zone_struct *get_map_zone(const mapstruct *map) {
    int zone;

    for (zone = 0; zones[zone].mappath != NULL; zone++) {
        if (strcmp(zones[zone].mappath, map->path) == 0)
            return &zones[zone];
    }

    return NULL;
}

/**
 * Should we add a random map to this exit?
 * @param exit
 * exit to check.
 * @return
 * 1 if a map should be set, 0 else.
 */
static int is_suitable_exit(object *exit) {
    assert(exit);

    if (cf_object_get_int_property(exit, CFAPI_OBJECT_PROP_TYPE) != EXIT)
        return 0;
    if (cf_object_get_sstring_property(exit, CFAPI_OBJECT_PROP_SLAYING) || cf_object_get_sstring_property(exit, CFAPI_OBJECT_PROP_MESSAGE))
        return 0;

    return 1;
}

/**
 * Get the random map seed. Will always yield the same value for the same (position included) exit.
 * @param exit
 * exit to get the seed of.
 * @param map
 * map the exit is on.
 * @return
 * random seed.
 */
static int get_exit_seed(const object *exit, const mapstruct *map) {
    char r[500];
    int seed = 0, len, w = 0;

    snprintf(r, sizeof(r), "%s!%d,%d*%s", exit->arch->name, exit->x, exit->y, map->path);

    len = strlen(r)-1;
    while (len >= 0) {
        seed ^= ((int)r[len])<<w;
        w += 8;
        w = w%32;
        len--;
    }

    return seed;
}

/**
 * Change an empty exit to point to a random map.
 * @param exit
 * exit to alter.
 * @param zone
 * zone we're part of, to know the random map parameters.
 * @param map
 * map the exit is on.
 */
static void add_exit_to_item(object *exit, const house_zone_struct *zone, const mapstruct *map) {
    char params[MAX_BUF];

    assert(exit);
    assert(zone);

    snprintf(params, sizeof(params), "layoutstyle onion\n"
        "floorstyle indoor\n"
        "wallstyle wooden\n"
        "monsterstyle %s\n"
        "dungeon_level 1\n"
        "dungeon_depth 1\n"
        "decorstyle furniture\n"
        "random_seed %d\n",
        zone->monsterstyle,
        get_exit_seed(exit, map));

    cf_object_set_string_property(exit, CFAPI_OBJECT_PROP_SLAYING, "/!");
    cf_object_set_string_property(exit, CFAPI_OBJECT_PROP_MESSAGE, params);
}

/**
 * Checks if the map should be processed, and if so process it.
 * @param map
 * map to work on.
 */
static void add_exits_to_map(const mapstruct *map) {
    int x, y;
    const house_zone_struct *zone = get_map_zone(map);

    if (!zone)
        return;

    for (x = 0; x < MAP_WIDTH(map); x++) {
        for (y = 0; y < MAP_HEIGHT(map); y++) {
            FOR_MAP_PREPARE(map, x, y, item) {
                if (is_suitable_exit(item))
                    add_exit_to_item(item, zone, map);
            } FOR_MAP_FINISH();
        }
    }
}

/**
 * Global server event handling. Only uses EVENT_MAPLOAD.
 * @param type
 * unused.
 * @return
 * 0.
 */
CF_PLUGIN int cfrhg_globalEventListener(int *type, ...) {
    va_list args;
    int rv = 0;
    mapstruct *map;
    int code;

    va_start(args, type);
    code = va_arg(args, int);

    switch (code) {
    case EVENT_MAPLOAD:
        map = va_arg(args, mapstruct *);
        add_exits_to_map(map);
        break;
    }
    va_end(args);

    return rv;
}

/**
 * Unused.
 * @param type
 * unused.
 * @return
 * 0.
 */
CF_PLUGIN int eventListener(int *type, ...) {
    return 0;
}

/**
 * Plugin initialization.
 * @param iversion
 * server version.
 * @param gethooksptr
 * function to get hooks.
 * @return
 * 0.
 */
CF_PLUGIN int initPlugin(const char *iversion, f_plug_api gethooksptr) {
    cf_init_plugin(gethooksptr);

    cf_log(llevDebug, PLUGIN_VERSION " init\n");

    return 0;
}

/**
 * Get the plugin identification or full name.
 * @param type
 * unused.
 * @return
 * NULL.
 */
CF_PLUGIN void *getPluginProperty(int *type, ...) {
    va_list args;
    const char *propname;
    int size;
    char *buf;

    va_start(args, type);
    propname = va_arg(args, const char *);

    if (!strcmp(propname, "Identification")) {
        buf = va_arg(args, char *);
        size = va_arg(args, int);
        va_end(args);
        snprintf(buf, size, PLUGIN_NAME);
        return NULL;
    } else if (!strcmp(propname, "FullName")) {
        buf = va_arg(args, char *);
        size = va_arg(args, int);
        va_end(args);
        snprintf(buf, size, PLUGIN_VERSION);
        return NULL;
    }
    va_end(args);
    return NULL;
}

/**
 * Unused.
 * @param op
 * unused.
 * @param params
 * unused.
 * @return
 * -1.
 */
CF_PLUGIN int cfrhg_runPluginCommand(object *op, char *params) {
    return -1;
}

/**
 * Plugin initialisation.
 * @return
 * 0.
 */
CF_PLUGIN int postInitPlugin(void) {
    cf_log(llevDebug, PLUGIN_VERSION " post init\n");

    cf_system_register_global_event(EVENT_MAPLOAD, PLUGIN_NAME, cfrhg_globalEventListener);

    return 0;
}

/**
 * Unloading of plugin.
 * @return
 * 0.
 */
CF_PLUGIN int closePlugin(void) {
    cf_log(llevDebug, PLUGIN_VERSION " closing\n");
    return 0;
}
/*@}*/
