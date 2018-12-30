/*****************************************************************************/
/* Logger plugin version 1.0 alpha.                                          */
/* Contact:                                                                  */
/*****************************************************************************/
/* That code is placed under the GNU General Public Licence (GPL)            */
/* (C)2007 by Weeger Nicolas (Feel free to deliver your complaints)          */
/*****************************************************************************/
/*  CrossFire, A Multiplayer game for X-windows                              */
/*                                                                           */
/*  Copyright (C) 2000 Mark Wedel                                            */
/*  Copyright (C) 1992 Frank Tore Johansen                                   */
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
 * @file cflogger.c
 * This plugin will log events to an sqlite3 database named cflogger.db in the
 * var directory.
 *
 * Log includes:
 * @li players join/leave/creation/quit
 * @li map load/unload/reset/enter/leave
 * @li kills, whenever a player is concerned
 * @li ingame/real time links
 *
 * @warning
 * The plugin will not check the database size, which can grow a lot.
 *
 * @note
 * Thanks to sqlite's locking, it's possible to access the database through the command
 * line even while the server is running.
 */

#include <cflogger.h>
#ifndef __CEXTRACT__
#include <cflogger_proto.h>
#endif
/*#include <stdarg.h>*/

#include <sqlite3.h>
#include <svnversion.h>

CF_PLUGIN char SvnRevPlugin[] = SVN_REV;

/** Current database format */
#define CFLOGGER_CURRENT_FORMAT 3

/** Pointer to the logging database. */
static sqlite3 *database;

/** To keep track of stored ingame/real time matching. */
static int last_stored_day = -1;

/**
 * Simple callback to get an integer from a query.
 *
 * @param param
 * user-supplied data.
 * @param argc
 * number of items.
 * @param argv
 * values.
 * @param azColName
 * column names.
 *
 * @return
 * always returns 0 to continue the execution.
 */
static int check_tables_callback(void *param, int argc, char **argv, char **azColName) {
    int *format = (int *)param;

    *format = atoi(argv[0]);
    return 0;
}

/**
 * Helper function to run a SQL query.
 *
 * Will LOG() an error if the query fails.
 *
 * @param sql
 * query to run.
 *
 * @return
 * SQLITE_OK if no error, other value if error.
 *
 * @note
 * There is most likely no need to check return value unless you need to
 * rollback a transaction or similar.
 */
static int do_sql(const char *sql) {
    int err;
    char *msg;

    if (!database)
        return -1;

    err = sqlite3_exec(database, sql, NULL, NULL, &msg);
    if (err != SQLITE_OK) {
        cf_log(llevError, " [%s] error: %d [%s] for sql = %s\n", PLUGIN_NAME, err, msg, sql);
        sqlite3_free(msg);
    }
    return err;
}

/**
 * Updates a table to a new schema, used for when ALTER TABLE doesn't work.
 * (Such as when changing column constraints.)
 *
 * @param table
 * Name of table.
 * @param newschema
 * This is the new table format. Will be inserted into the parantheses of
 * "create table table_name()".
 * @param select_columns
 * This is inserted into "INSERT INTO table_name SELECT _ FROM ..." to allow
 * changing order of columns, or skipping some. Normally it should be "*".
 *
 * @warning
 * This function should only be used in check_tables() below.
 *
 * No error checking is done. Also it is expected that appending an _old
 * on the table name won't collide with anything.
 *
 * Note that columns are expected to have same (or compatible) type, and be in
 * the same order. Further both tables should have the same number of columns.
 *
 * @return
 * SQLITE_OK if no error, non-zero if error. This SHOULD be rollback any
 * transaction this function is called in.
 */
static int update_table_format(const char *table, const char *newschema,
                               const char *select_columns) {
    char *sql;
    int err;

    sql = sqlite3_mprintf("ALTER TABLE %s RENAME TO %s_old;", table, table);
    err = do_sql(sql);
    sqlite3_free(sql);
    if (err != SQLITE_OK)
        return err;

    sql = sqlite3_mprintf("CREATE TABLE %s(%s);", table, newschema);
    err = do_sql(sql);
    sqlite3_free(sql);
    if (err != SQLITE_OK)
        return err;

    sql = sqlite3_mprintf("INSERT INTO %s SELECT %s FROM %s_old;",
                          table, select_columns, table);
    err = do_sql(sql);
    sqlite3_free(sql);
    if (err != SQLITE_OK)
        return err;

    sql = sqlite3_mprintf("DROP TABLE %s_old;", table, table);
    err = do_sql(sql);
    sqlite3_free(sql);
    /* Final return. */
    return err;
}

/**
 * Helper macros for rolling back and returning if query failed.
 * Used in check_tables().
 *
 * Yes they are quite messy. The alternatives seemed worse.
 */
#define DO_OR_ROLLBACK(sqlstring) \
    if (do_sql(sqlstring) != SQLITE_OK) { \
        do_sql("rollback transaction;"); \
        cf_log(llevError, " [%s] Logger database format update failed! Couldn't upgrade from format %d to fromat %d!. Won't log.\n", PLUGIN_NAME, format, CFLOGGER_CURRENT_FORMAT);\
        sqlite3_close(database); \
        database = NULL; \
        return; \
    }

#define UPDATE_OR_ROLLBACK(tbl, newschema, select_columns) \
    if (update_table_format((tbl), (newschema), (select_columns)) != SQLITE_OK) { \
        do_sql("rollback transaction;"); \
        cf_log(llevError, " [%s] Logger database format update failed! Couldn't upgrade from format %d to fromat %d!. Won't log.\n", PLUGIN_NAME, format, CFLOGGER_CURRENT_FORMAT);\
        sqlite3_close(database); \
        database = NULL; \
        return; \
    }

/**
 * Checks the database format, and applies changes if old version.
 */
static void check_tables(void) {
    int format;
    /*int err;*/
    format = 0;

    /*err =*/ sqlite3_exec(database, "select param_value from parameters where param_name = 'version';", check_tables_callback, &format, NULL);

    /* Safety check. */
    if (format > CFLOGGER_CURRENT_FORMAT) {
        cf_log(llevError, " [%s] Logger database format (%d) is newer than supported (%d) by this binary!. Won't log.\n", PLUGIN_NAME, format, CFLOGGER_CURRENT_FORMAT);
        /* This will disable using the db since do_sql() checks if database is
         * NULL.
         */
        sqlite3_close(database);
        database = NULL;
    }

    /* Check if we need to upgrade/create database. */
    if (format < 1) {
        cf_log(llevDebug, " [%s] Creating logger database schema (format 1).\n", PLUGIN_NAME);
        if (do_sql("BEGIN EXCLUSIVE TRANSACTION;") != SQLITE_OK) {
            cf_log(llevError, " [%s] Logger database format update failed! Couldn't acquire exclusive lock to database when upgrading from format %d to fromat %d!. Won't log.\n", PLUGIN_NAME, format, CFLOGGER_CURRENT_FORMAT);
            sqlite3_close(database);
            database = NULL;
            return;
        }
        DO_OR_ROLLBACK("create table living(liv_id integer primary key autoincrement, liv_name text, liv_is_player integer, liv_level integer);");
        DO_OR_ROLLBACK("create table region(reg_id integer primary key autoincrement, reg_name text);");
        DO_OR_ROLLBACK("create table map(map_id integer primary key autoincrement, map_path text, map_reg_id integer);");
        DO_OR_ROLLBACK("create table time(time_real integer, time_ingame text);");

        DO_OR_ROLLBACK("create table living_event(le_liv_id integer, le_time integer, le_code integer, le_map_id integer);");
        DO_OR_ROLLBACK("create table map_event(me_map_id integer, me_time integer, me_code integer, me_living_id integer);");
        DO_OR_ROLLBACK("create table kill_event(ke_time integer, ke_victim_id integer, ke_victim_level integer, ke_map_id integer , ke_killer_id integer, ke_killer_level integer);");

        DO_OR_ROLLBACK("create table parameters(param_name text, param_value text);");
        DO_OR_ROLLBACK("insert into parameters values( 'version', '1' );");
        do_sql("COMMIT TRANSACTION;");
    }

    /* Must be able to handle update from format 1. If we are creating a new
     * database, format 1 is still created first, then updated.
     *
     * This way is simpler than having to create two ways to make a format 2 db.
     */
    if (format < 2) {
        cf_log(llevDebug, " [%s] Upgrading logger database schema (to format 2).\n", PLUGIN_NAME);
        if (do_sql("BEGIN EXCLUSIVE TRANSACTION;") != SQLITE_OK) {
            cf_log(llevError, " [%s] Logger database format update failed! Couldn't acquire exclusive lock to database when upgrading from format %d to fromat %d!. Won't log.\n", PLUGIN_NAME, format, CFLOGGER_CURRENT_FORMAT);
            sqlite3_close(database);
            database = NULL;
            return;
        }
        /* Update schema for various tables. Why so complex? Because ALTER TABLE
         * can't add the "primary key" bit or other constraints...
         */
        UPDATE_OR_ROLLBACK("living", "liv_id INTEGER PRIMARY KEY AUTOINCREMENT, liv_name TEXT NOT NULL, liv_is_player INTEGER NOT NULL, liv_level INTEGER NOT NULL", "*");
        UPDATE_OR_ROLLBACK("region", "reg_id INTEGER PRIMARY KEY AUTOINCREMENT, reg_name TEXT UNIQUE NOT NULL", "*");
        UPDATE_OR_ROLLBACK("map",    "map_id INTEGER PRIMARY KEY AUTOINCREMENT, map_path TEXT NOT NULL, map_reg_id INTEGER NOT NULL, CONSTRAINT map_path_reg_id UNIQUE(map_path, map_reg_id)", "*");
#if 0
        /* Turned out this was incorrect. And version 1 -> 3 directly works for this. */
        UPDATE_OR_ROLLBACK("time",   "time_real INTEGER PRIMARY KEY, time_ingame TEXT UNIQUE NOT NULL");
#endif
        UPDATE_OR_ROLLBACK("living_event", "le_liv_id INTEGER NOT NULL, le_time INTEGER NOT NULL, le_code INTEGER NOT NULL, le_map_id INTEGER NOT NULL", "*");
        UPDATE_OR_ROLLBACK("map_event",    "me_map_id INTEGER NOT NULL, me_time INTEGER NOT NULL, me_code INTEGER NOT NULL, me_living_id INTEGER NOT NULL", "*");
        UPDATE_OR_ROLLBACK("kill_event",   "ke_time INTEGER NOT NULL, ke_victim_id INTEGER NOT NULL, ke_victim_level INTEGER NOT NULL, ke_map_id INTEGER NOT NULL, ke_killer_id INTEGER NOT NULL, ke_killer_level INTEGER NOT NULL", "*");

        /* Handle changed parameters table format: */
        /* Due to backward compatiblity "primary key" in SQLite doesn't imply
         * "not null" (http://www.sqlite.org/lang_createtable.html), unless it
         * is "integer primary key".
         *
         * We don't need to save anything stored in this in format 1, it was
         * only used for storing what format was used.
         */
        DO_OR_ROLLBACK("DROP TABLE parameters;");
        DO_OR_ROLLBACK("CREATE TABLE parameters(param_name TEXT NOT NULL PRIMARY KEY, param_value TEXT);");
        DO_OR_ROLLBACK("INSERT INTO parameters (param_name, param_value) VALUES( 'version', '2' );");

        /* Create various indexes. */
        DO_OR_ROLLBACK("CREATE INDEX living_name_player_level ON living(liv_name,liv_is_player,liv_level);");

        /* Newspaper module could make use of some indexes too: */
        DO_OR_ROLLBACK("CREATE INDEX kill_event_time ON kill_event(ke_time);");
        DO_OR_ROLLBACK("CREATE INDEX map_reg_id ON map(map_reg_id);");

        /* Finally commit the transaction. */
        do_sql("COMMIT TRANSACTION;");
    }

    if (format < 3) {
        cf_log(llevDebug, " [%s] Upgrading logger database schema (to format 3).\n", PLUGIN_NAME);
        if (do_sql("BEGIN EXCLUSIVE TRANSACTION;") != SQLITE_OK) {
            cf_log(llevError, " [%s] Logger database format update failed! Couldn't acquire exclusive lock to database when upgrading from format %d to fromat %d!. Won't log.\n", PLUGIN_NAME, format, CFLOGGER_CURRENT_FORMAT);
            sqlite3_close(database);
            database = NULL;
            return;
        }
        UPDATE_OR_ROLLBACK("time",   "time_ingame TEXT NOT NULL PRIMARY KEY, time_real INTEGER NOT NULL", "time_ingame, time_real");
        DO_OR_ROLLBACK("UPDATE parameters SET param_value = '3' WHERE param_name = 'version';");
        do_sql("COMMIT TRANSACTION;");
        /* After all these changes better vacuum... The tables could have been
         * huge, and since we recreated several of them above there could be a
         * lot of wasted space.
         */
        do_sql("VACUUM;");
    }
}

/**
 * Returns a unique identifier for specified object.
 *
 * Will insert an item in the table if required.
 *
 * If the object is a player, only name is taken into account to generate an id.
 *
 * Else, the object's level is taken into account, to distinguish monsters with
 * the same name and different levels (special monsters, and such).
 *
 * @param living
 * object to get identifier for.
 * @return
 * unique identifier in the 'living' table.
 */
static int get_living_id(object *living) {
    char **line;
    char *sql;
    int nrow, ncolumn, id;

    if (living->type == PLAYER)
        sql = sqlite3_mprintf("select liv_id from living where liv_name='%q' and liv_is_player = 1", living->name);
    else
        sql = sqlite3_mprintf("select liv_id from living where liv_name='%q' and liv_is_player = 0 and liv_level = %d", living->name, living->level);
    sqlite3_get_table(database, sql, &line, &nrow, &ncolumn, NULL);

    if (nrow > 0)
        id = atoi(line[ncolumn]);
    else {
        sqlite3_free(sql);
        sql = sqlite3_mprintf("insert into living(liv_name, liv_is_player, liv_level) values('%q', %d, %d)", living->name, living->type == PLAYER ? 1 : 0, living->level);
        do_sql(sql);
        id = sqlite3_last_insert_rowid(database);
    }
    sqlite3_free(sql);
    sqlite3_free_table(line);
    return id;
}

/**
 * Gets the unique identifier for a region.
 *
 * Will generate one if required.
 *
 * @param reg
 * region for which an id is wanted
 * @return
 * unique region identifier, or 0 if reg is NULL.
 */
static int get_region_id(region *reg) {
    char **line;
    char *sql;
    int nrow, ncolumn, id;

    if (!reg)
        return 0;

    sql = sqlite3_mprintf("select reg_id from region where reg_name='%q'", reg->name);
    sqlite3_get_table(database, sql, &line, &nrow, &ncolumn, NULL);

    if (nrow > 0)
        id = atoi(line[ncolumn]);
    else {
        sqlite3_free(sql);
        sql = sqlite3_mprintf("insert into region(reg_name) values( '%q' )", reg->name);
        do_sql(sql);
        id = sqlite3_last_insert_rowid(database);
    }
    sqlite3_free(sql);
    sqlite3_free_table(line);
    return id;
}

/**
 * Gets the unique identifier for a map.
 *
 * Will generate one if required.
 *
 * Maps starting with '/random/' will all share the same identifier for the same region.
 *
 * @param map
 * map for which an id is wanted. Must not be NULL.
 * @return
 * unique map identifier.
 */
static int get_map_id(mapstruct *map) {
    char **line;
    char *sql;
    int nrow, ncolumn, id, reg_id;
    const char *path = map->path;

    if (strncmp(path, "/random/", 7) == 0)
        path = "/random/";

    reg_id = get_region_id(map->region);
    sql = sqlite3_mprintf("select map_id from map where map_path='%q' and map_reg_id = %d", path, reg_id);
    sqlite3_get_table(database, sql, &line, &nrow, &ncolumn, NULL);

    if (nrow > 0)
        id = atoi(line[ncolumn]);
    else {
        sqlite3_free(sql);
        sql = sqlite3_mprintf("insert into map(map_path, map_reg_id) values( '%q', %d)", path, reg_id);
        do_sql(sql);
        id = sqlite3_last_insert_rowid(database);
    }
    sqlite3_free(sql);
    sqlite3_free_table(line);

    return id;
}

/**
 * Stores a line to match current ingame and real time.
 *
 * @return
 * 1 if a line was inserted, 0 if the current ingame time was already logged.
 */
static int store_time(void) {
    char **line;
    char *sql;
    int nrow, ncolumn;
    char date[50];
    time_t now;
    timeofday_t tod;

    cf_get_time(&tod);
    now = time(NULL);

    if (tod.day == last_stored_day)
        return 0;
    last_stored_day = tod.day;

    snprintf(date, 50, "%10d-%2d-%2d %2d:%2d", tod.year, tod.month, tod.day, tod.hour, tod.minute);

    sql = sqlite3_mprintf("select * from time where time_ingame='%q'", date);
    sqlite3_get_table(database, sql, &line, &nrow, &ncolumn, NULL);
    sqlite3_free(sql);
    sqlite3_free_table(line);
    if (nrow > 0)
        return 0;

    sql = sqlite3_mprintf("insert into time (time_ingame, time_real) values( '%s', %d )", date, now);
    do_sql(sql);
    sqlite3_free(sql);
    return 1;
}

/**
 * Logs an event for a living object.
 *
 * @param pl
 * object for which to log an event.
 * @param event_code
 * arbitrary event code.
 */
static void add_player_event(object *pl, int event_code) {
    int id = get_living_id(pl);
    int map_id = 0;
    char *sql;

    if (pl == NULL)
        return;

    if (pl->map)
        map_id = get_map_id(pl->map);

    sql = sqlite3_mprintf("insert into living_event values( %d, %d, %d, %d)", id, time(NULL), event_code, map_id);
    do_sql(sql);
    sqlite3_free(sql);
}

/**
 * Logs an event for a map.
 *
 * @param map
 * map for which to log an event.
 * @param event_code
 * arbitrary event code.
 * @param pl
 * object causing the event. Can be NULL.
 */
static void add_map_event(mapstruct *map, int event_code, object *pl) {
    int mapid;
    int playerid = 0;
    char *sql;

    if (pl && pl->type == PLAYER)
        playerid = get_living_id(pl);

    mapid = get_map_id(map);
    sql = sqlite3_mprintf("insert into map_event values( %d, %d, %d, %d)", mapid, time(NULL), event_code, playerid);
    do_sql(sql);
    sqlite3_free(sql);
}

/**
 * Logs a death.
 *
 * If either of the parameters is NULL, or if neither is a PLAYER, nothing is logged.
 *
 * @param victim
 * who died.
 * @param killer
 * who killed.
 */
static void add_death(object *victim, object *killer) {
    int vid, kid, map_id;
    char *sql;

    if (!victim || !killer)
        return;
    if (victim->type != PLAYER && killer->type != PLAYER) {
        /* Killer might be a bullet, which might be owned by the player. */
        object *owner = cf_object_get_object_property(killer, CFAPI_OBJECT_PROP_OWNER);
        if (owner != NULL && owner->type == PLAYER)
            killer = owner;
        else
            return;
    }

    vid = get_living_id(victim);
    kid = get_living_id(killer);
    map_id = get_map_id(victim->map);
    sql = sqlite3_mprintf("insert into kill_event values( %d, %d, %d, %d, %d, %d)", time(NULL), vid, victim->level, map_id, kid, killer->level);
    do_sql(sql);
    sqlite3_free(sql);
}

/**
 * Main plugin entry point.
 *
 * @param iversion
 * server version.
 * @param gethooksptr
 * function to get hooks from.
 * @return
 * always 0.
 */
CF_PLUGIN int initPlugin(const char *iversion, f_plug_api gethooksptr) {
    cf_init_plugin(gethooksptr);
    cf_log(llevInfo, "%s init\n", PLUGIN_VERSION);
    return 0;
}

/**
 * Gets a plugin property.
 *
 * @param type
 * ignored.
 * @return
 * @li the name, if asked for 'Identification'.
 * @li the version, if asked for 'FullName'.
 * @li NULL else.
 */
CF_PLUGIN void *getPluginProperty(int *type, ...) {
    va_list args;
    const char *propname;
    char *buf;
    int size;

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
 * Runs a plugin command. Doesn't do anything.
 *
 * @param op
 * ignored.
 * @param params
 * ignored.
 * @return
 * -1.
 */
CF_PLUGIN int cflogger_runPluginCommand(object *op, char *params) {
    return -1;
}

/**
 * Handles an object-related event. Doesn't do anything.
 *
 * @param type
 * ignored.
 * @return
 * 0.
 */
CF_PLUGIN int eventListener(int *type, ...) {
    return 0;
}

/**
 * Handles a global event.
 *
 * @param type
 * ignored.
 * @return
 * 0
 */
CF_PLUGIN int cflogger_globalEventListener(int *type, ...) {
    va_list args;
    int rv = 0;
    player *pl;
    object *op/*, *op2*/;
    int event_code;
    mapstruct *map;

    va_start(args, type);
    event_code = va_arg(args, int);

    switch (event_code) {
    case EVENT_BORN:
    case EVENT_REMOVE:
    case EVENT_MUZZLE:
    case EVENT_KICK:
        op = va_arg(args, object *);
        add_player_event(op, event_code);
        break;

    case EVENT_PLAYER_DEATH:
        op = va_arg(args, object *);
        /*op2 =*/ va_arg(args, object *);
        add_player_event(op, event_code);
        break;

    case EVENT_LOGIN:
    case EVENT_LOGOUT:
        pl = va_arg(args, player *);
        add_player_event(pl->ob, event_code);
        break;

    case EVENT_MAPENTER:
    case EVENT_MAPLEAVE:
        op = va_arg(args, object *);
        map = va_arg(args, mapstruct *);
        add_map_event(map, event_code, op);
        break;

    case EVENT_MAPLOAD:
    case EVENT_MAPUNLOAD:
    case EVENT_MAPRESET:
        map = va_arg(args, mapstruct *);
        add_map_event(map, event_code, NULL);
        break;

    case EVENT_GKILL: {
            object *killer;
            op = va_arg(args, object *);
            killer = va_arg(args, object *);
            add_death(op, killer);
        }
        break;

    case EVENT_CLOCK:
        store_time();
        break;
    }
    va_end(args);

    return rv;
}

/**
 * Plugin was initialized, now to finish.
 *
 * Registers events, initializes the database.
 *
 * @return
 * 0.
 */
CF_PLUGIN int postInitPlugin(void) {
    char path[500];
    const char *dir;

    cf_log(llevInfo, "%s post init\n", PLUGIN_VERSION);

    dir = cf_get_directory(4);
    snprintf(path, sizeof(path), "%s/cflogger.db", dir);
    cf_log(llevDebug, " [%s] database file: %s\n", PLUGIN_NAME, path);

    if (sqlite3_open(path, &database) != SQLITE_OK) {
        cf_log(llevError, " [%s] database error!\n", PLUGIN_NAME);
        sqlite3_close(database);
        database = NULL;
        return 0;
    }

    check_tables();

    store_time();

    cf_system_register_global_event(EVENT_BORN, PLUGIN_NAME, cflogger_globalEventListener);
    cf_system_register_global_event(EVENT_REMOVE, PLUGIN_NAME, cflogger_globalEventListener);
    cf_system_register_global_event(EVENT_GKILL, PLUGIN_NAME, cflogger_globalEventListener);
    cf_system_register_global_event(EVENT_LOGIN, PLUGIN_NAME, cflogger_globalEventListener);
    cf_system_register_global_event(EVENT_LOGOUT, PLUGIN_NAME, cflogger_globalEventListener);

    cf_system_register_global_event(EVENT_PLAYER_DEATH, PLUGIN_NAME, cflogger_globalEventListener);

    cf_system_register_global_event(EVENT_MAPENTER, PLUGIN_NAME, cflogger_globalEventListener);
    cf_system_register_global_event(EVENT_MAPLEAVE, PLUGIN_NAME, cflogger_globalEventListener);
    cf_system_register_global_event(EVENT_MAPRESET, PLUGIN_NAME, cflogger_globalEventListener);
    cf_system_register_global_event(EVENT_MAPLOAD, PLUGIN_NAME, cflogger_globalEventListener);
    cf_system_register_global_event(EVENT_MAPUNLOAD, PLUGIN_NAME, cflogger_globalEventListener);

    cf_system_register_global_event(EVENT_MUZZLE, PLUGIN_NAME, cflogger_globalEventListener);
    cf_system_register_global_event(EVENT_KICK, PLUGIN_NAME, cflogger_globalEventListener);

    cf_system_register_global_event(EVENT_CLOCK, PLUGIN_NAME, cflogger_globalEventListener);

    return 0;
}

/**
 * Close the plugin.
 *
 * Closes the sqlite database.
 *
 * @return
 * 0.
 */
CF_PLUGIN int closePlugin(void) {
    cf_log(llevInfo, "%s closing.\n", PLUGIN_VERSION);
    if (database) {
        sqlite3_close(database);
        database = NULL;
    }
    return 0;
}
