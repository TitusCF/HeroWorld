/*****************************************************************************/
/* Newspaper plugin version 1.0 alpha.                                       */
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

/* First let's include the header file needed                                */

#include <cfnewspaper.h>
#include <stdarg.h>
#ifndef __CEXTRACT__
#include <cfnewspaper_proto.h>
#endif
#include <sqlite3.h>
#include <svnversion.h>

CF_PLUGIN char SvnRevPlugin[] = SVN_REV;

f_plug_api gethook;

f_plug_api registerGlobalEvent;

f_plug_api unregisterGlobalEvent;

f_plug_api reCmp;

static sqlite3 *logger_database;

static sqlite3 *newspaper_database;

static void do_sql(const char *sql, sqlite3 *base) {
    int err;
    char *msg;

    if (!base)
        return;

    err = sqlite3_exec(base, sql, NULL, NULL, &msg);
    if (err != SQLITE_OK) {
        cf_log(llevError, " [%s] error: %d [%s] for sql = %s\n", PLUGIN_NAME, err, msg, sql);
        sqlite3_free(msg);
    }
}

static int get_region_id(region *reg) {
    char **line;
    char *sql;
    int nrow, ncolumn, id;

    if (!reg)
        return 0;

    sql = sqlite3_mprintf("select reg_id from region where reg_name='%q'", reg->name);
    sqlite3_get_table(logger_database, sql, &line, &nrow, &ncolumn, NULL);

    if (nrow > 0)
        id = atoi(line[ncolumn]);
    else {
        sqlite3_free(sql);
        sql = sqlite3_mprintf("insert into region(reg_name) values( '%q' )", reg->name);
        do_sql(sql, logger_database);
        id = sqlite3_last_insert_rowid(logger_database);
    }
    sqlite3_free(sql);
    sqlite3_free_table(line);
    return id;
}

static void format_time(timeofday_t *tod, char *buffer, int size) {
    snprintf(buffer, size, "%10d-%2d-%2d %2d:%2d", tod->year, tod->month, tod->day, tod->hour, tod->minute);
}

static void read_parameters(void) {
}

CF_PLUGIN int initPlugin(const char *iversion, f_plug_api gethooksptr) {
    cf_init_plugin(gethooksptr);
    cf_log(llevInfo, "%s init\n", PLUGIN_VERSION);
    return 0;
}

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
    }

    if (!strcmp(propname, "FullName")) {
        buf = va_arg(args, char *);
        size = va_arg(args, int);
        va_end(args);
        snprintf(buf, size, PLUGIN_VERSION);
        return NULL;
    }

    va_end(args);
    return NULL;
}

CF_PLUGIN int cfnewspaper_runPluginCommand(object *op, char *params) {
    return -1;
}

CF_PLUGIN int cfnewspaper_globalEventListener(int *type, ...) {
    va_list args;
    int rv = 0;
    int event_code;

    va_start(args, type);
    event_code = va_arg(args, int);

    switch (event_code) {
    }
    va_end(args);

    return rv;
}

CF_PLUGIN int postInitPlugin(void) {
    char path[500];
    const char *dir;

    cf_log(llevInfo, "%s post init\n", PLUGIN_VERSION);

    dir = cf_get_directory(4);
    snprintf(path, 500, "%s/cflogger.db", dir);

    if (sqlite3_open(path, &logger_database) != SQLITE_OK) {
        cf_log(llevError, " [%s] couldn't connect to logger database!\n", PLUGIN_NAME);
        sqlite3_close(logger_database);
        logger_database = NULL;
        return 0;
    }

    snprintf(path, 500, "%s/cfnewspaper.db", dir);
    if (sqlite3_open(path, &newspaper_database) != SQLITE_OK) {
        cf_log(llevError, " [%s] unable to open newspaper database!\n", PLUGIN_NAME);
        sqlite3_close(logger_database);
        sqlite3_close(newspaper_database);
        logger_database = NULL;
        newspaper_database = NULL;
        return 0;
    }

    read_parameters();

    return 0;
}

typedef struct paper_properties {
    const char *name;
    int info_region;
    int info_world;
} paper_properties;

static paper_properties default_properties = {
    "world newspaper",
    0,
    1
};

typedef struct kill_format {
    const char *no_player_death;
    const char *one_player_death;
    const char *many_player_death;
    const char *no_monster_death;
    const char *one_monster_death;
    const char *many_monster_death;
} kill_format;

static paper_properties *get_newspaper(const char *name) {
    return &default_properties;
}

static void news_cat(char *buffer, int size, const char *format, ...) {
    va_list args;

    size -= strlen(buffer)-1;
    buffer += strlen(buffer);

    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
}

static void do_kills(char *buffer, int size, time_t start, time_t end, const char *reg,  kill_format *format) {
    char *sql;
    char **results;
    int deaths = 0;
    int nrow, ncolumn;
    int err;
    char *msg;
    const char *raw_players = "select sum(1) as deaths from kill_event inner join living on liv_id = ke_victim_id where liv_is_player = %d and ke_time >= %d and ke_time < %d %s";
    const char *raw_monsters = "select sum(1) as deaths from kill_event inner join living on liv_id = ke_victim_id where liv_is_player = %d and ke_time >= %d and ke_time < %d";

    sql = sqlite3_mprintf(raw_players, 1, start, end, reg);
    err = sqlite3_get_table(logger_database, sql, &results, &nrow, &ncolumn, &msg);
    sqlite3_free(sql);
    if (err != SQLITE_OK) {
        cf_log(llevError, " [%s] error: %d [%s] for sql = %s\n", PLUGIN_NAME, err, msg, sql);
        sqlite3_free(msg);
    }
    if (nrow > 0 && results[ncolumn] != NULL)
        deaths = atoi(results[ncolumn]);
    sqlite3_free_table(results);

    if (deaths == 0)
        news_cat(buffer, size, format->no_player_death);
    else if (deaths == 1)
        news_cat(buffer, size, format->one_player_death);
    else
        news_cat(buffer, size, format->many_player_death, deaths);
    news_cat(buffer, size, "\n");

    sql = sqlite3_mprintf(raw_monsters, 0, start, end);
    err = sqlite3_get_table(logger_database, sql, &results, &nrow, &ncolumn, &msg);
    sqlite3_free(sql);
    if (err != SQLITE_OK) {
        cf_log(llevError, " [%s] error: %d [%s] for sql = %s\n", PLUGIN_NAME, err, msg, sql);
        sqlite3_free(msg);
    }
    if (nrow > 0 && results[ncolumn] != NULL)
        deaths = atoi(results[ncolumn]);
    sqlite3_free_table(results);

    if (deaths == 0)
        news_cat(buffer, size, format->no_monster_death);
    else if (deaths == 1)
        news_cat(buffer, size, format->one_monster_death);
    else
        news_cat(buffer, size, format->many_monster_death, deaths);
    news_cat(buffer, size, "\n");
}

static void do_region_kills(region *reg, char *buffer, int size, time_t start, time_t end) {
    kill_format f;
    char where[50];
    int region_id;

    f.no_player_death = "No player died.";
    f.one_player_death = "Only one player died, May Fido(tm) Have Mercy.";
    f.many_player_death = "Monsters were busy, %d players died.";
    f.no_monster_death = "No monster was killed, players were lazy around here.";
    f.one_monster_death = "One poor monster was killed.";
    f.many_monster_death = "Players tried hard to kill monsters, with %d victims.";

    region_id = get_region_id(reg);
    snprintf(where, 50, "and map_reg_id = %d", region_id);

    do_kills(buffer, size, start, end, where, &f);
}

static void do_region(region *reg, char *buffer, int size, time_t start, time_t end) {
    news_cat(buffer, size, "--- local %s news ---\n", reg->name);
    do_region_kills(reg, buffer, size, start, end);
    news_cat(buffer, size, "\n\n");
}

static void do_world_kills(char *buffer, int size, time_t start, time_t end) {
    kill_format f;

    f.no_player_death = "No player died at all.";
    f.one_player_death = "Only one player died in the whole world, May Fido(tm) Have Mercy.";
    f.many_player_death = "Monsters all around the world were busy, %d players died.";
    f.no_monster_death = "No monster was killed at all, players must be tired!";
    f.one_monster_death = "One poor monster was killed in the whole world, too bad for it.";
    f.many_monster_death = "Bad day for monsters, with %d dead in their ranks.";
    do_kills(buffer, size, start, end, "", &f);
}

static void do_world(char *buffer, int size, time_t start, time_t end) {
    news_cat(buffer, size, "--- worldnews section ---\n");
    do_world_kills(buffer, size, start, end);
    news_cat(buffer, size, "\n\n");
}

static void get_newspaper_content(object *paper, paper_properties *properties, region *reg) {
    char contents[5000];
    char *sql;
    char **results;
    char date[50];
    int nrow, ncolumn;
    time_t start, end;
    timeofday_t tod;
    int err;
    char *msg;

    start = 0;
    time(&end);

    cf_get_time(&tod);
    format_time(&tod, date, 50);

    sql = sqlite3_mprintf("select * from time where time_ingame < '%q' order by time_ingame desc", date);
    err = sqlite3_get_table(logger_database, sql, &results, &nrow, &ncolumn, &msg);
    sqlite3_free(sql);
    if (err != SQLITE_OK) {
        cf_log(llevError, " [%s] error: %d [%s] for sql = %s\n", PLUGIN_NAME, err, msg, sql);
        sqlite3_free(msg);
    }
    if (nrow > 1 && results[ncolumn+1] != NULL) {
        end = atol(results[ncolumn+1]);
        if (nrow > 1 && results[ncolumn+2] != NULL)
            start = atol(results[ncolumn+2]);
    }

    contents[0] = '\0';

    if (properties->info_region)
        do_region(reg, contents, 5000, start, end);

    if (properties->info_world)
        do_world(contents, 5000, start, end);

    cf_object_set_string_property(paper, CFAPI_OBJECT_PROP_MESSAGE, contents);
}

CF_PLUGIN int eventListener(int *type, ...) {
    int rv = 0;
    va_list args;
    object *who;
    int event_code;
    object *activator;
    /*object *third;*/
    object *event;
    /*char *buf;*/
    /*int fix;*/
    object *newspaper;
    paper_properties *paper;
    region *reg;

    va_start(args, type);
    who = va_arg(args, object *);
    /*event_code = va_arg(args, int);*/
    activator = va_arg(args, object *);
    /*third =*/ va_arg(args, object *);
    /*buf =*/ va_arg(args, char *);
    /*fix =*/ va_arg(args, int);
    /*buf = va_arg(args, char *);*/
    event = va_arg(args, object *);
    event_code = event->subtype;

    va_end(args);

    if (event_code != EVENT_APPLY)
        return rv;

    paper = get_newspaper(event->slaying);

    newspaper = cf_create_object_by_name("scroll");

    cf_object_set_string_property(newspaper, CFAPI_OBJECT_PROP_NAME, paper->name);
    cf_object_set_string_property(newspaper, CFAPI_OBJECT_PROP_NAME_PLURAL, paper->name);

    if (activator->map)
        reg = cf_map_get_region_property(activator->map, CFAPI_MAP_PROP_REGION);
    else
        reg = NULL;

    get_newspaper_content(newspaper, paper, reg);

    cf_object_insert_object(newspaper, who);

    return rv;
}

CF_PLUGIN int closePlugin(void) {
    cf_log(llevInfo, "%s closing.\n", PLUGIN_VERSION);
    if (logger_database) {
        sqlite3_close(logger_database);
        logger_database = NULL;
    }
    if (newspaper_database) {
        sqlite3_close(newspaper_database);
        newspaper_database = NULL;
    }
    return 0;
}
