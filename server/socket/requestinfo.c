/*
 * Crossfire -- cooperative multi-player graphical RPG and adventure game
 *
 * Copyright (c) 1999-2014 Mark Wedel and the Crossfire Development Team
 * Copyright (c) 1992 Frank Tore Johansen
 *
 * Crossfire is free software and comes with ABSOLUTELY NO WARRANTY. You are
 * welcome to redistribute it under certain conditions. For details, please
 * see COPYING and LICENSE.
 *
 * The authors can be reached via e-mail at <crossfire@metalforge.org>.
 */

/**
 * \file
 * requestinfo protocol handling.
 *
 * \date 2010-07-04
 *
 * This file implements all of requestinfo protocol commands/responses.
 * This was broken from request.c as that file as getting quite large
 * and throwing everything into that file was making it fairly unmanageable.
 *
 */

#include <assert.h>
#include <global.h>
#include <sproto.h>

#include <shared/newclient.h>
#include <newserver.h>
#include <living.h>
#include <commands.h>

/* This block is basically taken from socket.c - I assume if it works there,
 * it should work here.
 */
#ifndef WIN32 /* ---win32 exclude unix headers */
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#endif /* win32 */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include "sounds.h"

/* Note that following protocol commands (was corresponding function)
 * are in image.c and not this file, even though they are requestinfo
 * commands:
 * image_sums -> send_image_sums()
 * image_info -> send_image_info()
 *
 * The order of the functions here is basically the order they
 * are called in from request_info_cmd() from loop.c
 */

/**
 * This sends the skill number to name mapping. We ignore
 * the params - we always send the same info no matter what.
 */
void send_skill_info(socket_struct *ns, char *params) {
    SockList sl;
    int i;

    SockList_Init(&sl);
    SockList_AddString(&sl, "replyinfo skill_info\n");
    for (i = 1; i < NUM_SKILLS; i++) {
        size_t len;

        len = 16+strlen(skill_names[i]); /* upper bound for length */
        if (SockList_Avail(&sl) < len) {
            LOG(llevError, "Buffer overflow in send_skill_info, not sending all skill information\n");
            break;
        }

        if (params != NULL && *params == '1') {
            if ((skill_faces[i] != -1) && !(ns->faces_sent[skill_faces[i]]&NS_FACESENT_FACE))
                esrv_send_face(ns, skill_faces[i], 0);
            SockList_AddPrintf(&sl, "%d:%s:%d\n", i+CS_STAT_SKILLINFO, skill_names[i], skill_faces[i]);
        } else
            SockList_AddPrintf(&sl, "%d:%s\n", i+CS_STAT_SKILLINFO, skill_names[i]);
    }
    Send_With_Handling(ns, &sl);
    SockList_Term(&sl);
}

/**
 * This sends the spell path to name mapping. We ignore
 * the params - we always send the same info no matter what.
 */
void send_spell_paths(socket_struct *ns, char *params) {
    SockList sl;
    int i;

    SockList_Init(&sl);
    SockList_AddString(&sl, "replyinfo spell_paths\n");
    for (i = 0; i < NRSPELLPATHS; i++) {
        size_t len;

        len = 16+strlen(spellpathnames[i]); /* upper bound for length */
        if (SockList_Avail(&sl) < len) {
            LOG(llevError, "Buffer overflow in send_spell_paths, not sending all spell information\n");
            break;
        }

        SockList_AddPrintf(&sl, "%d:%s\n", 1<<i, spellpathnames[i]);
    }
    Send_With_Handling(ns, &sl);
    SockList_Term(&sl);
}

/**
 * This sends the experience table the sever is using
 */
void send_exp_table(socket_struct *ns, char *params) {
    SockList sl;
    int i;
    extern sint64 *levels;

    SockList_Init(&sl);
    SockList_AddString(&sl, "replyinfo exp_table\n");
    SockList_AddShort(&sl, settings.max_level+1);
    for (i = 1; i <= settings.max_level; i++) {
        if (SockList_Avail(&sl) < 8) {
            LOG(llevError, "Buffer overflow in send_exp_table, not sending all information\n");
            break;
        }
        SockList_AddInt64(&sl, levels[i]);
    }
    Send_With_Handling(ns, &sl);
    SockList_Term(&sl);
}

/* This is like the AddIf... routines, but instead of
 * checking against another value which we overwrite,
 * we just check against 0.
 */
#define AddShortAttr(New, Type) \
    if (New) { \
        SockList_AddChar(sl, Type); \
        SockList_AddShort(sl, New); \
        }

/** This sends information about object op to client - used
 * in response to requestinfo.  This function is used by
 * both race & class transmissions, and could perhaps be used
 * by future requestinfo types.
 *
 * @param sl
 * socketlist to add data to
 * @param op
 * Object to extract data from.
 */
static void send_arch_info(SockList *sl, const object *op)
{
    if (op->name) {
        SockList_AddString(sl, "name ");
        SockList_AddLen8Data(sl, op->name, strlen(op->name));
    }

    /* It is conceivable some may lack messages */
    if (op->msg) {
        SockList_AddString(sl, "msg ");
        SockList_AddShort(sl, strlen(op->msg));
        SockList_AddData(sl, op->msg, strlen(op->msg));
    }

    SockList_AddString(sl, "stats ");
    /* Only send non zero stats.  More stats could be added here,
     * but ideally, the text description (op->msg) should give information
     * about resistances and other abilities.
     * Send stats last - if the client gets a stat it does not understand,
     * it stops processing this replyinfo.
     */
    AddShortAttr(op->stats.Str, CS_STAT_STR);
    AddShortAttr(op->stats.Int, CS_STAT_INT);
    AddShortAttr(op->stats.Pow, CS_STAT_POW);
    AddShortAttr(op->stats.Wis, CS_STAT_WIS);
    AddShortAttr(op->stats.Dex, CS_STAT_DEX);
    AddShortAttr(op->stats.Con, CS_STAT_CON);
    AddShortAttr(op->stats.Cha, CS_STAT_CHA);

    /* Terminator for the stats line */
    SockList_AddChar(sl, 0);

    /* Handle any race/class_choice options -
     * the code is exactly the same, except for
     * name of field we are looking for.
     */
    if (op->type == CLASS || op->type == PLAYER) {
        int i=1;
        char buf[MAX_BUF];
        const char *value, *value1;
        char *lasts, *mychoices, *token;

        while (1) {
            if (op->type == PLAYER) {
                snprintf(buf, MAX_BUF, "race_choice_description_%d", i);
                value = object_get_value(op, buf);
                snprintf(buf, MAX_BUF, "race_choice_%d", i);
                value1 = object_get_value(op, buf);
            } else { /* Must be class */
                snprintf(buf, MAX_BUF, "class_choice_description_%d", i);
                value = object_get_value(op, buf);
                snprintf(buf, MAX_BUF, "class_choice_%d", i);
                value1 = object_get_value(op, buf);
            }

            if (value && value1) {
                SockList_AddString(sl, "choice ");
                SockList_AddLen8Data(sl, buf, strlen(buf));
                SockList_AddLen8Data(sl, value, strlen(value));
                i++;
                /* value1 now contains a list of archetypes */
                /* Following operations modify string */
                mychoices = strdup_local(value1);

                /* split_string() requires we have some
                 * idea on number of fields - in this case,
                 * we really have no idea - one could conceive
                 * of a choice of 50 weapons - using strtok_r
                 * is just as safe and will scale to any amount.
                 */
                token = strtok_r(mychoices, " ", &lasts);
                while (token) {
                    archetype *arch;

                    arch = try_find_archetype(token);
                    if (arch) {
                        SockList_AddLen8Data(sl, token, strlen(token));
                        SockList_AddLen8Data(sl, arch->clone.name,
                                             strlen(arch->clone.name));
                    } else {
                        LOG(llevError, "send_arch_info: Unable to find archetype %s\n", token);
                    }
                    token = strtok_r(NULL, " ", &lasts);
                }
                free(mychoices);
                /* Terminator byte */
                SockList_AddChar(sl, 0);
            } else {
                break;
            }
        }
    }


    /* Any addition to data to send should be at the end of the
     * function - in other words, the order of data sent here should
     * match order of addition.  In that way, the newest additions are
     * sent last, so client can process this data until it gets
     * something it does not understand - if new data (subfields in the
     * replyinfo) are sent first, the client basically has to stop
     * processing once it gets something it does not understand.
     */

}

/**
 * Creates the appropriate reply to the 'race_list' request info.
 *
 * @param sl
 * suitable reply.
 */
static void build_race_list_reply(SockList *sl) {
    archetype *race;

    SockList_AddString(sl, "replyinfo race_list ");

    for (race = first_archetype; race; race = race->next) {
        if (race->clone.type == PLAYER) {
            SockList_AddPrintf(sl, "|%s", race->name);
        }
    }
}

/**
 * Send the list of player races to the client.
 * The reply is kept in a static buffer, as it won't change during server run.
 *
 * @param ns
 * where to send.
 * @param params
 * ignored.
 */
void send_race_list(socket_struct *ns, char *params) {
    static SockList sl;
    static int sl_initialized = 0;

    if (!sl_initialized) {
        sl_initialized = 1;
        SockList_Init(&sl);
        build_race_list_reply(&sl);
    }

    Send_With_Handling(ns, &sl);
}

/**
 * Sends information on specified race to the client.
 *
 * @param ns
 * where to send.
 * @param params
 * race name to send.
 */
void send_race_info(socket_struct *ns, char *params) {
    archetype *race = try_find_archetype(params);
    SockList sl;

    SockList_Init(&sl);
    SockList_AddPrintf(&sl, "replyinfo race_info %s\n", params);

    /* do not let the client arbitrarily request information about
     * any archetype, so put a check in here for the right clone type.
     */
    if (race && race->clone.type == PLAYER) {
        send_arch_info(&sl, &race->clone);
    }

    Send_With_Handling(ns, &sl);
    SockList_Term(&sl);
}

/**
 * Creates the appropriate reply to the 'class_list' request info.
 *
 * @param sl
 * reply.
 */
static void build_class_list_reply(SockList *sl) {
    archetype *cl;

    SockList_Reset(sl);
    SockList_AddString(sl, "replyinfo class_list ");

    for (cl = first_archetype; cl; cl = cl->next) {
        if (cl->clone.type == CLASS) {
            SockList_AddPrintf(sl, "|%s", cl->name);
        }
    }
}

/**
 * Sends the list of classes to the client.
 * The reply is kept in a static buffer, as it won't change during server run.
 *
 * @param ns
 * client to send to.
 * @param params
 * ignored.
 */
void send_class_list(socket_struct *ns, char *params) {
    static SockList sl;
    static int sl_initialized = 0;

    if (!sl_initialized) {
        sl_initialized = 1;
        SockList_Init(&sl);
        build_class_list_reply(&sl);
    }

    Send_With_Handling(ns, &sl);
}

/**
 * Send information on the specified class.
 *
 * @param ns
 * where to send.
 * @param params
 * class name to send.
 */
void send_class_info(socket_struct *ns, char *params) {
    archetype *class = try_find_archetype(params);
    SockList sl;

    SockList_Init(&sl);
    SockList_AddPrintf(&sl, "replyinfo class_info %s\n", params);

    /* do not let the client arbitrarily request information about
     * any archetype, so put a check in here for the right clone type.
     */
    if (class && class->clone.type == CLASS) {
        send_arch_info(&sl, &class->clone);
    }

    Send_With_Handling(ns, &sl);
    SockList_Term(&sl);
}

/**
 * Send information on the specified class.
 *
 * @param ns
 * where to send.
 */
void send_map_info(socket_struct *ns) {
    archetype *m;
    SockList sl;

    SockList_Init(&sl);
    SockList_AddPrintf(&sl, "replyinfo startingmap\n");

    for (m = first_archetype; m; m = m->next) {
        if (m->clone.type == MAP && m->clone.subtype == MAP_TYPE_CHOICE) {
            SockList_AddChar(&sl, INFO_MAP_ARCH_NAME);
            SockList_AddLen16Data(&sl, m->name, strlen(m->name));

            SockList_AddChar(&sl, INFO_MAP_NAME);
            SockList_AddLen16Data(&sl, m->clone.name, strlen(m->clone.name));

            /* In theory, this should always be set, but better not to crash
             * if it is not.
             */
            if (m->clone.msg) {
                SockList_AddChar(&sl, INFO_MAP_DESCRIPTION);
                SockList_AddLen16Data(&sl, m->clone.msg, strlen(m->clone.msg));
            }
        }
    }

    Send_With_Handling(ns, &sl);
    SockList_Term(&sl);
}

/**
 * Sends the desired file to the client.  In all
 * three cases, we are basically just dumping file
 * contents to the client - nothing more.
 *
 * @param ns
 * socket to send to
 * @param file
 * Which file to send - string of either motd, news, rules
 */
void send_file(socket_struct *ns, const char *file) {
    char buf[MAX_BUF];
    FILE *fp;
    SockList sl;

    if (!strcmp(file,"motd"))
        snprintf(buf, sizeof(buf), "%s/%s", settings.confdir, settings.motd);
    else if (!strcmp(file,"rules"))
        snprintf(buf, sizeof(buf), "%s/%s", settings.confdir, settings.rules);
    else if (!strcmp(file,"news"))
        snprintf(buf, sizeof(buf), "%s/%s", settings.confdir, settings.news);
    else {
        LOG(llevError,"send_file requested to send unknown file: %s\n", file);
        return;
    }
    fp = fopen(buf, "r");
    if (fp == NULL)
        return;
    SockList_Init(&sl);
    SockList_AddString(&sl, "replyinfo ");
    SockList_AddString(&sl, file);
    SockList_AddString(&sl, "\n");

    while (fgets(buf, MAX_BUF, fp) != NULL) {
        if (*buf == '#')
            continue;
        SockList_AddString(&sl, buf);
    }
    fclose(fp);
    SockList_AddChar(&sl, 0);   /* Null terminate it */
    Send_With_Handling(ns, &sl);
    SockList_Term(&sl);
}

/**
 * Sends information related to creating a new character
 * to the client.
 *
 * @param ns
 * socket to send to
 */
void send_new_char_info(socket_struct *ns) {
    char buf[MAX_BUF];
    int i;
    size_t len;
    SockList sl;

    SockList_Init(&sl);

    SockList_AddString(&sl, "replyinfo newcharinfo\n");
    snprintf(buf, MAX_BUF, "V points %d", settings.starting_stat_points);
    /* We add +1 to the length so that the null (terminator) byte
     * gets included - this make processing on the client side easier.
     */
    SockList_AddLen8Data(&sl, buf, strlen(buf) + 1);

    snprintf(buf, MAX_BUF, "V statrange %d %d",
             settings.starting_stat_min, settings.starting_stat_max);
    SockList_AddLen8Data(&sl, buf, strlen(buf) + 1);

    snprintf(buf, MAX_BUF, "V statname");
    len = strlen(buf);
    for (i=0; i<NUM_STATS; i++) {
        safe_strcat(buf, " ", &len, MAX_BUF);
        safe_strcat(buf, short_stat_name[i], &len, MAX_BUF);
    }

    SockList_AddLen8Data(&sl, buf, strlen(buf) + 1);

    snprintf(buf, MAX_BUF, "R race requestinfo");
    SockList_AddLen8Data(&sl, buf, strlen(buf) + 1);

    snprintf(buf, MAX_BUF, "R class requestinfo");
    SockList_AddLen8Data(&sl, buf, strlen(buf) + 1);

    snprintf(buf, MAX_BUF, "O startingmap requestinfo");
    SockList_AddLen8Data(&sl, buf, strlen(buf) + 1);

    Send_With_Handling(ns, &sl);
    SockList_Term(&sl);
}
