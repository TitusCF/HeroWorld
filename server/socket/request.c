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
 * Client handling.
 *
 * \date 2003-12-02
 *
 * This file implements all of the goo on the server side for handling
 * clients. It's got a bunch of global variables for keeping track of
 * each of the clients.
 *
 * Note: All functions that are used to process data from the client
 * have the prototype of (char *data, int datalen, int client_num). This
 * way, we can use one dispatch table.
 *
 * esrv_map_scroll tells the client to scroll the map, and does similarily
 * for the locally cached copy.
 *
 * @todo
 * smoothing should be automatic for latest clients. Remove some stuff we can assume is always on.
 * fix comments for this file.
 *
 * This file should probably be broken up into smaller sections - having all request
 * handling from the client in one file makes this a very large file in which
 * it is hard to find data (and know how it is related).  In addition, a lot
 * of the function is not actually requests from the client, but push from
 * the server (stats, maps)
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

/**
 * This table translates the attack numbers as used within the
 * program to the value we use when sending STATS command to the
 * client. If a value is -1, then we don't send that to the
 * client.
 */
static const short atnr_cs_stat[NROFATTACKS] = {
    CS_STAT_RES_PHYS, CS_STAT_RES_MAG,
    CS_STAT_RES_FIRE, CS_STAT_RES_ELEC,
    CS_STAT_RES_COLD, CS_STAT_RES_CONF,
    CS_STAT_RES_ACID,
    CS_STAT_RES_DRAIN, -1 /* weaponmagic */,
    CS_STAT_RES_GHOSTHIT, CS_STAT_RES_POISON,
    CS_STAT_RES_SLOW, CS_STAT_RES_PARA,
    CS_STAT_TURN_UNDEAD,
    CS_STAT_RES_FEAR, -1 /* Cancellation */,
    CS_STAT_RES_DEPLETE, CS_STAT_RES_DEATH,
    -1 /* Chaos */, -1 /* Counterspell */,
    -1 /* Godpower */, CS_STAT_RES_HOLYWORD,
    CS_STAT_RES_BLIND,
    -1, /* Internal */
    -1, /* life stealing */
    -1 /* Disease - not fully done yet */
};

/** This is the Setup cmd - easy first implementation */
void set_up_cmd(char *buf, int len, socket_struct *ns) {
    int s;
    char *cmd, *param;
    SockList sl;

    /* run through the cmds of setup
     * syntax is setup <cmdname1> <parameter> <cmdname2> <parameter> ...
     *
     * we send the status of the cmd back, or a FALSE is the cmd
     * is the server unknown
     * The client then must sort this out
     */

    LOG(llevInfo, "Get SetupCmd:: %s\n", buf);
    SockList_Init(&sl);
    SockList_AddString(&sl, "setup");
    for (s = 0; s < len; ) {
        cmd = &buf[s];

        /* find the next space, and put a null there */
        for (; buf[s] && buf[s] != ' '; s++)
            ;
        if (s >= len)
            break;
        buf[s++] = 0;

        while (buf[s] == ' ')
            s++;
        if (s >= len)
            break;
        param = &buf[s];

        for (; buf[s] && buf[s] != ' '; s++)
            ;
        buf[s++] = 0;

        while (s < len && buf[s] == ' ')
            s++;

        SockList_AddPrintf(&sl, " %s ", cmd);

        if (!strcmp(cmd, "sound2")) {
            ns->sound = atoi(param)&(SND_EFFECTS|SND_MUSIC|SND_MUTE);
            SockList_AddString(&sl, param);
        } else if (!strcmp(cmd, "spellmon")) {
            int monitor_spells;

            monitor_spells = atoi(param);
            if (monitor_spells < 0 || monitor_spells > 2) {
                SockList_AddString(&sl, "FALSE");
            } else {
                ns->monitor_spells = monitor_spells;
                SockList_AddPrintf(&sl, "%d", monitor_spells);
            }
        } else if (!strcmp(cmd, "darkness")) {
            int darkness;

            darkness = atoi(param);
            if (darkness != 0 && darkness != 1) {
                SockList_AddString(&sl, "FALSE");
            } else {
                ns->darkness = darkness;
                SockList_AddPrintf(&sl, "%d", darkness);
            }
        } else if (!strcmp(cmd, "map2cmd")) {
            int map2cmd;

            map2cmd = atoi(param);
            if (map2cmd != 1) {
                SockList_AddString(&sl, "FALSE");
            } else {
                SockList_AddString(&sl, "1");
            }
        } else if (!strcmp(cmd, "facecache")) {
            int facecache;

            facecache = atoi(param);
            if (facecache != 0 && facecache != 1) {
                SockList_AddString(&sl, "FALSE");
            } else {
                ns->facecache = facecache;
                SockList_AddPrintf(&sl, "%d", facecache);
            }
        } else if (!strcmp(cmd, "faceset")) {
            int q = atoi(param);

            if (is_valid_faceset(q))
                ns->faceset = q;
            SockList_AddPrintf(&sl, "%d", ns->faceset);
        } else if (!strcmp(cmd, "mapsize")) {
            int x, y, n;

            if (sscanf(param, "%dx%d%n", &x, &y, &n) != 2 || n != (int)strlen(param)) {
                x = 0;
                y = 0;
            }
            if (x < 9 || y < 9 || x > MAP_CLIENT_X || y > MAP_CLIENT_Y) {
                SockList_AddPrintf(&sl, "%dx%d", MAP_CLIENT_X, MAP_CLIENT_Y);
            } else {
                player *pl;
                ns->mapx = x;
                ns->mapy = y;
                /* better to send back what we are really using and not the
                 * param as given to us in case it gets parsed differently.
                 */
                SockList_AddPrintf(&sl, "%dx%d", x, y);
                /* need to update the los, else the view jumps */
                pl = find_player_socket(ns);
                if (pl)
                  update_los(pl->ob);

                /* Client and server need to resynchronize on data - treating it as
                 * a new map is best way to go.
                 */
                map_newmap_cmd(ns);
            }
        } else if (!strcmp(cmd, "tick")) {
            int tick;

            tick = atoi(param);
            if (tick != 0 && tick != 1) {
                SockList_AddString(&sl, "FALSE");
            } else {
                ns->tick = tick;
                SockList_AddPrintf(&sl, "%d", tick);
            }
        } else if (!strcmp(cmd, "bot")) {
            int is_bot;

            is_bot = atoi(param);
            if (is_bot != 0 && is_bot != 1) {
                SockList_AddString(&sl, "FALSE");
            } else {
                ns->is_bot = is_bot;
                SockList_AddPrintf(&sl, "%d", is_bot);
            }
        } else if (!strcmp(cmd, "want_pickup")) {
            int want_pickup;

            want_pickup = atoi(param);
            if (want_pickup != 0 && want_pickup != 1) {
                SockList_AddString(&sl, "FALSE");
            } else {
                ns->want_pickup = want_pickup;
                SockList_AddPrintf(&sl, "%d", want_pickup);
            }
        } else if (!strcmp(cmd, "num_look_objects")) {
            int tmp;
            player *pl;

            tmp = atoi(param);
            if (tmp < MIN_NUM_LOOK_OBJECTS) {
                tmp = MIN_NUM_LOOK_OBJECTS;
            } else if (tmp > MAX_NUM_LOOK_OBJECTS) {
                tmp = MAX_NUM_LOOK_OBJECTS;
            }
            ns->num_look_objects = (uint8)tmp;
            SockList_AddPrintf(&sl, "%d", tmp);

            pl = find_player_socket(ns);
            if (pl && pl->ob) {
              ns->update_look = 1;
              esrv_draw_look(pl->ob);
            }
        } else if (!strcmp(cmd, "extended_stats")) {
            int extended_stats;

            extended_stats = atoi(param);
            if (extended_stats != 0 && extended_stats != 1) {
                SockList_AddString(&sl, "FALSE");
            } else {
                ns->extended_stats = extended_stats;
                SockList_AddPrintf(&sl, "%d", extended_stats);
            }

        } else if (!strcmp(cmd, "loginmethod")) {
            int loginmethod;

            loginmethod = atoi(param);

            /* Only support basic login right now */
            if (loginmethod > 2) loginmethod=2;

            ns->login_method = loginmethod;
            SockList_AddPrintf(&sl, "%d", loginmethod);

        } else if (!strcmp(cmd, "notifications")) {
            int notifications;

            notifications = atoi(param);

            ns->notifications = notifications;
            SockList_AddPrintf(&sl, "%d", notifications);

        } else if (!strcmp(cmd, "newmapcmd")) {
            /* newmapcmd is deprecated (now standard part), but some
             * clients still use this setup option, and if the server
             * doesn't respond, erroneously report that the client is
             * too old.  Since it is always on, regardless of what is
             * request, send back one.
             */
            SockList_AddString(&sl, "1");
        } else if (!strcmp(cmd, "extendedTextInfos")) {
            /* like newmapcmd above, extendedTextInfos is
             * obsolete, but we respond for the same reason as we do
             * in newmapcmd
             */
            SockList_AddString(&sl, "1");
        } else if (!strcmp(cmd, "itemcmd")) {
            /* like newmapcmd above, itemcmd is
             * obsolete, but we respond for the same reason as we do
             * in newmapcmd
             */
            SockList_AddString(&sl, "2");
        } else if (!strcmp(cmd, "exp64")) {
            /* like newmapcmd above, exp64 is
             * obsolete, but we respond for the same reason as we do
             * in newmapcmd
             */
            SockList_AddString(&sl, "1");
        } else {
            /* Didn't get a setup command we understood -
             * report a failure to the client.
             */
            SockList_AddString(&sl, "FALSE");
        }
    } /* for processing all the setup commands */
    Send_With_Handling(ns, &sl);
    SockList_Term(&sl);
}

/**
 * The client has requested to be added to the game.
 * This is what takes care of it. We tell the client how things worked out.
 * I am not sure if this file is the best place for this function. However,
 * it either has to be here or init_sockets needs to be exported.
 *
 * @todo can ns->status not be Ns_Add?
 */
void add_me_cmd(char *buf, int len, socket_struct *ns) {
    Settings oldsettings;
    SockList sl;

    oldsettings = settings;
    if (ns->status != Ns_Add) {
        SockList_Init(&sl);
        SockList_AddString(&sl, "addme_failed");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
    } else if (find_player_socket(ns) == NULL) {
        /* if there is already a player for this socket (add_me was already called),
         * just ignore, else weird issues. */

        add_player(ns, 0);
        /* Basically, the add_player copies the socket structure into
         * the player structure, so this one (which is from init_sockets)
         * is not needed anymore. The write below should still work,
         * as the stuff in ns is still relevant.
         */
        SockList_Init(&sl);
        SockList_AddString(&sl, "addme_success");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        if (ns->sc_version < 1027 || ns->cs_version < 1023) {
            /* The space in the link isn't correct, but in my
             * quick test with client 1.1.0, it didn't print it
             * out correctly when done as a single line.
             */
            print_ext_msg(ns, NDI_RED, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_VERSION,
                          "Warning: Your client is too old to receive map data. Please update to a new client at http://sourceforge.net/project/showfiles.php ?group_id=13833");
        }

        ns->status = Ns_Avail;
    }
    settings = oldsettings;
}

/**
 * A lot like the old AskSmooth (in fact, now called by AskSmooth).
 * Basically, it makes no sense to wait for the client to request a
 * a piece of data from us that we know the client wants. So
 * if we know the client wants it, might as well push it to the
 * client.
 */
static void send_smooth(socket_struct *ns, uint16 face) {
    uint16 smoothface;
    SockList sl;

    /* If we can't find a face, return and set it so we won't
     * try to send this again.
     */
    if (!find_smooth(face, &smoothface)
    && !find_smooth(smooth_face->number, &smoothface)) {
        LOG(llevError, "could not findsmooth for %d. Neither default (%s)\n", face, smooth_face->name);
        ns->faces_sent[face] |= NS_FACESENT_SMOOTH;
        return;
    }

    if (!(ns->faces_sent[smoothface]&NS_FACESENT_FACE))
        esrv_send_face(ns, smoothface, 0);

    ns->faces_sent[face] |= NS_FACESENT_SMOOTH;

    SockList_Init(&sl);
    SockList_AddString(&sl, "smooth ");
    SockList_AddShort(&sl, face);
    SockList_AddShort(&sl, smoothface);
    Send_With_Handling(ns, &sl);
    SockList_Term(&sl);
}

/**
 * Tells client the picture it has to use
 * to smooth a picture number given as argument.
 */
void ask_smooth_cmd(char *buf, int len, socket_struct *ns) {
    uint16 facenbr;

    facenbr = atoi(buf);
    send_smooth(ns, facenbr);
}

/**
 * This handles the commands issued by the player (ie, north, fire, cast,
 * etc.). This is called with the 'ncom' method which gives more information back
 * to the client so it can throttle.
 *
 * @param buf
 * data received.
 * @param len
 * length of buf.
 * @param pl
 * player who issued the command. Mustn't be NULL.
 */
void new_player_cmd(uint8 *buf, int len, player *pl) {
    int time, repeat;
    short packet;
    char command[MAX_BUF];
    SockList sl;

    if (len < 7) {
        LOG(llevDebug, "Corrupt ncom command - not long enough - discarding\n");
        return;
    }

    packet = GetShort_String(buf);
    repeat = GetInt_String(buf+2);
    /* -1 is special - no repeat, but don't update */
    if (repeat != -1) {
        pl->count = repeat;
    }
    if (len-4 >= MAX_BUF)
        len = MAX_BUF-5;

    strncpy(command, (char *)buf+6, len-4);
    command[len-4] = '\0';

    /* The following should never happen with a proper or honest client.
     * Therefore, the error message doesn't have to be too clear - if
     * someone is playing with a hacked/non working client, this gives them
     * an idea of the problem, but they deserve what they get
     */
    if (pl->state != ST_PLAYING) {
        draw_ext_info_format(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                             "You can not issue commands - state is not ST_PLAYING (%s)",
                             buf);
        return;
    }

    /* This should not happen anymore. */
    if (pl->ob->speed_left < -1.0) {
        LOG(llevError, "Player has negative time - shouldn't do command.\n");
    }
    /* In c_new.c */
    execute_newserver_command(pl->ob, command);
    /* Perhaps something better should be done with a left over count.
     * Cleaning up the input should probably be done first - all actions
     * for the command that issued the count should be done before
     * any other commands.
     */
    pl->count = 0;

    /* Send confirmation of command execution now */
    SockList_Init(&sl);
    SockList_AddString(&sl, "comc ");
    SockList_AddShort(&sl, packet);
    if (FABS(pl->ob->speed) < 0.001)
        time = MAX_TIME*100;
    else
        time = (int)(MAX_TIME/FABS(pl->ob->speed));
    SockList_AddInt(&sl, time);
    Send_With_Handling(&pl->socket, &sl);
    SockList_Term(&sl);
}

/** This is a reply to a previous query. */
void reply_cmd(char *buf, int len, player *pl) {
    /* This is to synthesize how the data would be stored if it
     * was normally entered. A bit of a hack, and should be cleaned up
     * once all the X11 code is removed from the server.
     *
     * We pass 13 to many of the functions because this way they
     * think it was the carriage return that was entered, and the
     * function then does not try to do additional input.
     */
    snprintf(pl->write_buf, sizeof(pl->write_buf), ":%s", buf);

    /* this avoids any hacking here */

    switch (pl->state) {
    case ST_PLAYING:
        LOG(llevError, "Got reply message with ST_PLAYING input state\n");
        break;

    case ST_PLAY_AGAIN:
        /* We can check this for return value (2==quit). Maybe we
         * should, and do something appropriate?
         */
        receive_play_again(pl->ob, buf[0]);
        break;

    case ST_ROLL_STAT:
        key_roll_stat(pl->ob, buf[0]);
        break;

    case ST_CHANGE_CLASS:

        key_change_class(pl->ob, buf[0]);
        break;

    case ST_CONFIRM_QUIT:
        key_confirm_quit(pl->ob, buf[0]);
        break;

    case ST_GET_NAME:
        receive_player_name(pl->ob);
        break;

    case ST_GET_PASSWORD:
    case ST_CONFIRM_PASSWORD:
    case ST_CHANGE_PASSWORD_OLD:
    case ST_CHANGE_PASSWORD_NEW:
    case ST_CHANGE_PASSWORD_CONFIRM:
        receive_player_password(pl->ob);
        break;

    case ST_GET_PARTY_PASSWORD:        /* Get password for party */
        receive_party_password(pl->ob);
        break;

    default:
        LOG(llevError, "Unknown input state: %d\n", pl->state);
    }
}

/**
 * Client tells its version. If there is a mismatch, we close the
 * socket. In real life, all we should care about is the client having
 * something older than the server. If we assume the client will be
 * backwards compatible, having it be a later version should not be a
 * problem.
 */
void version_cmd(char *buf, int len, socket_struct *ns) {
    char *cp;

    if (!buf) {
        LOG(llevError, "CS: received corrupted version command\n");
        return;
    }

    ns->cs_version = atoi(buf);
    ns->sc_version = ns->cs_version;
    if (VERSION_CS != ns->cs_version) {
#ifdef ESRV_DEBUG
        LOG(llevDebug, "CS: csversion mismatch (%d,%d)\n", VERSION_CS, ns->cs_version);
#endif
    }
    cp = strchr(buf+1, ' ');
    if (!cp)
        return;
    ns->sc_version = atoi(cp);
    if (VERSION_SC != ns->sc_version) {
#ifdef ESRV_DEBUG
        LOG(llevDebug, "CS: scversion mismatch (%d,%d)\n", VERSION_SC, ns->sc_version);
#endif
    }
    cp = strchr(cp+1, ' ');
    if (cp) {
        LOG(llevDebug, "CS: connection from client of type <%s>, ip %s\n", cp, ns->host);
    }
}

/**
 * Sound related function.
 * @todo remove once clients don't try to use this - server closes connection on invalid client.
 */

/** client wants the map resent
 * @todo remove
*/
void map_redraw_cmd(char *buf, int len, player *pl) {
    /* This function is currently disabled; just clearing the
     * map state results in display errors. It should clear the
     * cache and send a newmap command. Unfortunately this
     * solution does not work because some client versions send
     * a mapredraw command after receiving a newmap command.
     */
}

/** Newmap command */
void map_newmap_cmd(socket_struct *ns) {
    SockList sl;

    /* If getting a newmap command, this scroll information
     * is no longer relevant.
     */
    ns->map_scroll_x = 0;
    ns->map_scroll_y = 0;


    memset(&ns->lastmap, 0, sizeof(ns->lastmap));
    SockList_Init(&sl);
    SockList_AddString(&sl, "newmap");
    Send_With_Handling(ns, &sl);
    SockList_Term(&sl);
}

/**
 * Moves an object (typically, container to inventory).
 * syntax is: move (to) (tag) (nrof)
 */
void move_cmd(char *buf, int len, player *pl) {
    int vals[3], i;

    /* A little funky here. We only cycle for 2 records, because
     * we obviously am not going to find a space after the third
     * record. Perhaps we should just replace this with a
     * sscanf?
     */
    for (i = 0; i < 2; i++) {
        vals[i] = atoi(buf);
        if (!(buf = strchr(buf, ' '))) {
            LOG(llevError, "Incomplete move command: %s\n", buf);
            return;
        }
        buf++;
    }
    vals[2] = atoi(buf);

/*    LOG(llevDebug, "Move item %d (nrof=%d) to %d.\n", vals[1], vals[2], vals[0]);*/
    esrv_move_object(pl->ob, vals[0], vals[1], vals[2]);
}

/***************************************************************************
 *
 * Start of commands the server sends to the client.
 *
 ***************************************************************************
 */

/**
 * Asks the client to query the user. This way, the client knows
 * it needs to send something back (vs just printing out a message)
 */
void send_query(socket_struct *ns, uint8 flags, const char *text) {
    SockList sl;

    SockList_Init(&sl);
    SockList_AddPrintf(&sl, "query %d %s", flags, text ? text : "");
    Send_With_Handling(ns, &sl);
    SockList_Term(&sl);
}

#define AddIfInt64(Old, New, Type)                  \
    if (Old != New) {                               \
        Old = New;                                  \
        SockList_AddChar(&sl, Type);                \
        SockList_AddInt64(&sl, New);                \
    }

#define AddIfInt(Old, New, Type)                    \
    if (Old != New) {                               \
        Old = New;                                  \
        SockList_AddChar(&sl, Type);                \
        SockList_AddInt(&sl, New);                  \
    }

#define AddIfShort(Old, New, Type)                  \
    if (Old != New) {                               \
        Old = New;                                  \
        SockList_AddChar(&sl, Type);                \
        SockList_AddShort(&sl, New);                \
    }

#define AddIfFloat(Old, New, Type)                  \
    if (Old != New) {                               \
        Old = New;                                  \
        SockList_AddChar(&sl, Type);                \
        SockList_AddInt(&sl, (long)(New*FLOAT_MULTI));\
    }

#define AddIfString(Old, New, Type)                 \
    if (Old == NULL || strcmp(Old, New)) {          \
        free(Old);                                  \
        Old = strdup_local(New);                    \
        SockList_AddChar(&sl, Type);                \
        SockList_AddLen8Data(&sl, New, strlen(New));\
    }

/**
 * Sends a statistics update. We look at the old values,
 * and only send what has changed. Stat mapping values are in newclient.h
 * Since this gets sent a lot, this is actually one of the few binary
 * commands for now.
 */
void esrv_update_stats(player *pl) {
    SockList sl;
    char buf[MAX_BUF];
    uint16 flags;
    uint8 s;

    SockList_Init(&sl);
    SockList_AddString(&sl, "stats ");

    if (pl->ob != NULL) {
        AddIfShort(pl->last_stats.hp, pl->ob->stats.hp, CS_STAT_HP);
        AddIfShort(pl->last_stats.maxhp, pl->ob->stats.maxhp, CS_STAT_MAXHP);
        AddIfShort(pl->last_stats.sp, pl->ob->stats.sp, CS_STAT_SP);
        AddIfShort(pl->last_stats.maxsp, pl->ob->stats.maxsp, CS_STAT_MAXSP);
        AddIfShort(pl->last_stats.grace, pl->ob->stats.grace, CS_STAT_GRACE);
        AddIfShort(pl->last_stats.maxgrace, pl->ob->stats.maxgrace, CS_STAT_MAXGRACE);
        AddIfShort(pl->last_stats.Str, pl->ob->stats.Str, CS_STAT_STR);
        AddIfShort(pl->last_stats.Int, pl->ob->stats.Int, CS_STAT_INT);
        AddIfShort(pl->last_stats.Pow, pl->ob->stats.Pow, CS_STAT_POW);
        AddIfShort(pl->last_stats.Wis, pl->ob->stats.Wis, CS_STAT_WIS);
        AddIfShort(pl->last_stats.Dex, pl->ob->stats.Dex, CS_STAT_DEX);
        AddIfShort(pl->last_stats.Con, pl->ob->stats.Con, CS_STAT_CON);
        AddIfShort(pl->last_stats.Cha, pl->ob->stats.Cha, CS_STAT_CHA);
    }
    if (pl->socket.extended_stats) {
        sint16 golem_hp, golem_maxhp;
        AddIfShort(pl->last_orig_stats.Str, pl->orig_stats.Str, CS_STAT_BASE_STR);
        AddIfShort(pl->last_orig_stats.Int, pl->orig_stats.Int, CS_STAT_BASE_INT);
        AddIfShort(pl->last_orig_stats.Pow, pl->orig_stats.Pow, CS_STAT_BASE_POW);
        AddIfShort(pl->last_orig_stats.Wis, pl->orig_stats.Wis, CS_STAT_BASE_WIS);
        AddIfShort(pl->last_orig_stats.Dex, pl->orig_stats.Dex, CS_STAT_BASE_DEX);
        AddIfShort(pl->last_orig_stats.Con, pl->orig_stats.Con, CS_STAT_BASE_CON);
        AddIfShort(pl->last_orig_stats.Cha, pl->orig_stats.Cha, CS_STAT_BASE_CHA);
        if (pl->ob != NULL) {
            AddIfShort(pl->last_race_stats.Str, 20 + pl->ob->arch->clone.stats.Str, CS_STAT_RACE_STR);
            AddIfShort(pl->last_race_stats.Int, 20 + pl->ob->arch->clone.stats.Int, CS_STAT_RACE_INT);
            AddIfShort(pl->last_race_stats.Pow, 20 + pl->ob->arch->clone.stats.Pow, CS_STAT_RACE_POW);
            AddIfShort(pl->last_race_stats.Wis, 20 + pl->ob->arch->clone.stats.Wis, CS_STAT_RACE_WIS);
            AddIfShort(pl->last_race_stats.Dex, 20 + pl->ob->arch->clone.stats.Dex, CS_STAT_RACE_DEX);
            AddIfShort(pl->last_race_stats.Con, 20 + pl->ob->arch->clone.stats.Con, CS_STAT_RACE_CON);
            AddIfShort(pl->last_race_stats.Cha, 20 + pl->ob->arch->clone.stats.Cha, CS_STAT_RACE_CHA);
            AddIfShort(pl->last_applied_stats.Str, pl->applied_stats.Str, CS_STAT_APPLIED_STR);
            AddIfShort(pl->last_applied_stats.Int, pl->applied_stats.Int, CS_STAT_APPLIED_INT);
            AddIfShort(pl->last_applied_stats.Pow, pl->applied_stats.Pow, CS_STAT_APPLIED_POW);
            AddIfShort(pl->last_applied_stats.Wis, pl->applied_stats.Wis, CS_STAT_APPLIED_WIS);
            AddIfShort(pl->last_applied_stats.Dex, pl->applied_stats.Dex, CS_STAT_APPLIED_DEX);
            AddIfShort(pl->last_applied_stats.Con, pl->applied_stats.Con, CS_STAT_APPLIED_CON);
            AddIfShort(pl->last_applied_stats.Cha, pl->applied_stats.Cha, CS_STAT_APPLIED_CHA);
        }
        if (pl->ranges[range_golem]) {
            object *golem = pl->ranges[range_golem];
            if (QUERY_FLAG(golem, FLAG_REMOVED) || golem->count != pl->golem_count || QUERY_FLAG(golem, FLAG_FREED)) {
                golem_hp = 0;
                golem_maxhp = 0;
            } else {
                golem_hp = golem->stats.hp;
                golem_maxhp = golem->stats.maxhp;
            }
        } else {
            golem_hp = 0;
            golem_maxhp = 0;
        }
        /* send first the maxhp, so the client can set up the display */
        AddIfShort(pl->last_golem_maxhp, golem_maxhp, CS_STAT_GOLEM_MAXHP);
        AddIfShort(pl->last_golem_hp, golem_hp, CS_STAT_GOLEM_HP);
    }

    for (s = 0; s < NUM_SKILLS; s++) {
        if (pl->last_skill_ob[s]
        && pl->last_skill_exp[s] != pl->last_skill_ob[s]->stats.exp) {
            /* Always send along the level if exp changes. This
             * is only 1 extra byte, but keeps processing simpler.
             */
            SockList_AddChar(&sl, (char)(s+CS_STAT_SKILLINFO));
            SockList_AddChar(&sl, (char)pl->last_skill_ob[s]->level);
            SockList_AddInt64(&sl, pl->last_skill_ob[s]->stats.exp);
            pl->last_skill_exp[s] = pl->last_skill_ob[s]->stats.exp;
        }
    }
    AddIfInt64(pl->last_stats.exp, pl->ob->stats.exp, CS_STAT_EXP64);
    AddIfShort(pl->last_level, (char)pl->ob->level, CS_STAT_LEVEL);
    AddIfShort(pl->last_stats.wc, pl->ob->stats.wc, CS_STAT_WC);
    AddIfShort(pl->last_stats.ac, pl->ob->stats.ac, CS_STAT_AC);
    AddIfShort(pl->last_stats.dam, pl->ob->stats.dam, CS_STAT_DAM);
    AddIfFloat(pl->last_speed, pl->ob->speed, CS_STAT_SPEED);
    AddIfShort(pl->last_stats.food, pl->ob->stats.food, CS_STAT_FOOD);
    AddIfFloat(pl->last_weapon_sp, pl->ob->weapon_speed, CS_STAT_WEAP_SP);
    AddIfInt(pl->last_weight_limit, (sint32)get_weight_limit(pl->ob->stats.Str), CS_STAT_WEIGHT_LIM);
    flags = 0;
    if (pl->fire_on)
        flags |= SF_FIREON;
    if (pl->run_on)
        flags |= SF_RUNON;

    AddIfShort(pl->last_flags, flags, CS_STAT_FLAGS);
    if (pl->socket.sc_version < 1025) {
        AddIfShort(pl->last_resist[ATNR_PHYSICAL], pl->ob->resist[ATNR_PHYSICAL], CS_STAT_ARMOUR);
    } else {
        int i;

        for (i = 0; i < NROFATTACKS; i++) {
            /* Skip ones we won't send */
            if (atnr_cs_stat[i] == -1)
                continue;
            AddIfShort(pl->last_resist[i], pl->ob->resist[i], (char)atnr_cs_stat[i]);
        }
    }
    if (pl->socket.monitor_spells) {
        AddIfInt(pl->last_path_attuned, pl->ob->path_attuned, CS_STAT_SPELL_ATTUNE);
        AddIfInt(pl->last_path_repelled, pl->ob->path_repelled, CS_STAT_SPELL_REPEL);
        AddIfInt(pl->last_path_denied, pl->ob->path_denied, CS_STAT_SPELL_DENY);
    }
    /* we want to use the new fire & run system in new client */
    rangetostring(pl->ob, buf, sizeof(buf));
    AddIfString(pl->socket.stats.range, buf, CS_STAT_RANGE);
    set_title(pl->ob, buf, sizeof(buf));
    AddIfString(pl->socket.stats.title, buf, CS_STAT_TITLE);

    /* Only send it away if we have some actual data - 2 bytes for length, 6 for "stats ". */
    if (sl.len > 8) {
#ifdef ESRV_DEBUG
        LOG(llevDebug, "Sending stats command, %d bytes long.\n", sl.len);
#endif
        Send_With_Handling(&pl->socket, &sl);
    }
    SockList_Term(&sl);
}

/**
 * Tells the client that here is a player it should start using.
 */
void esrv_new_player(player *pl, uint32 weight) {
    SockList sl;

    pl->last_weight = weight;

    if (!(pl->socket.faces_sent[pl->ob->face->number]&NS_FACESENT_FACE))
        esrv_send_face(&pl->socket, pl->ob->face->number, 0);

    SockList_Init(&sl);
    SockList_AddString(&sl, "player ");
    SockList_AddInt(&sl, pl->ob->count);
    SockList_AddInt(&sl, weight);
    SockList_AddInt(&sl, pl->ob->face->number);
    SockList_AddLen8Data(&sl, pl->ob->name, strlen(pl->ob->name));

    Send_With_Handling(&pl->socket, &sl);
    SockList_Term(&sl);
    SET_FLAG(pl->ob, FLAG_CLIENT_SENT);
}

/**
 * Need to send an animation sequence to the client.
 * We will send appropriate face commands to the client if we haven't
 * sent them the face yet (this can become quite costly in terms of
 * how much we are sending - on the other hand, this should only happen
 * when the player logs in and picks stuff up.
 */
void esrv_send_animation(socket_struct *ns, short anim_num) {
    SockList sl;
    int i;

    /* Do some checking on the anim_num we got. Note that the animations
     * are added in contigous order, so if the number is in the valid
     * range, it must be a valid animation.
     */
    if (anim_num < 0 || anim_num > num_animations) {
        LOG(llevError, "esrv_send_anim (%d) out of bounds??\n", anim_num);
        return;
    }

    SockList_Init(&sl);
    SockList_AddString(&sl, "anim ");
    SockList_AddShort(&sl, anim_num);
    SockList_AddShort(&sl, 0); /* flags - not used right now */
    /* Build up the list of faces. Also, send any information (ie, the
     * the face itself) down to the client.
     */
    for (i = 0; i < animations[anim_num].num_animations; i++) {
        if (!(ns->faces_sent[animations[anim_num].faces[i]->number]&NS_FACESENT_FACE))
            esrv_send_face(ns, animations[anim_num].faces[i]->number, 0);
        /* flags - not used right now */
        SockList_AddShort(&sl, animations[anim_num].faces[i]->number);
    }
    Send_With_Handling(ns, &sl);
    SockList_Term(&sl);
    ns->anims_sent[anim_num] = 1;
}

/****************************************************************************
 *
 * Start of map related commands.
 *
 ****************************************************************************/

/** Clears a map cell */
static void map_clearcell(struct map_cell_struct *cell, int face, int count) {
    cell->darkness = count;
    memset(cell->faces, face, sizeof(cell->faces));
}

#define MAX_HEAD_POS MAX(MAX_CLIENT_X, MAX_CLIENT_Y)

/** Using a global really isn't a good approach, but saves the over head of
 * allocating and deallocating such a block of data each time run through,
 * and saves the space of allocating this in the socket object when we only
 * need it for this cycle. If the serve is ever threaded, this needs to be
 * re-examined.
 */

static const object *heads[MAX_HEAD_POS][MAX_HEAD_POS][MAP_LAYERS];

/****************************************************************************
 * This block is for map2 drawing related commands.
 * Note that the map2 still uses other functions.
 *
 ***************************************************************************/

/**
 * object 'ob' at 'ax,ay' on 'layer' is visible to the client.
 * This function does the following things:
 * If is_head head is set, this means this is from the heads[] array,
 * so don't try to store it away again - just send it and update
 * our look faces.
 *
 * 1) If a multipart object and we are not at the lower right corner,
 *    store this info away for later use.
 * 2) Check to see if this face has been sent to the client.  If not,
 *    we add data to the socklist, update the last map, and send any
 *    other data the client will need (smoothing table, image data, etc)
 * 3) Return 1 if function increased socket.
 * 4) has_obj is increased by one if there are visible objects on this
 *    this space, whether or not we sent them.  Basically, if has_obj
 *    is 0, we can clear info about this space.  It could be set to 1
 *    with the function returning zero - this means there are objects
 *    on the space we have already sent to the client.
 */
static int map2_add_ob(int ax, int ay, int layer, const object *ob, SockList *sl, socket_struct *ns, int *has_obj, int is_head) {
    uint16 face_num;
    uint8 nlayer, smoothlevel = 0;
    const object *head;

    assert(ob != NULL);

    head = HEAD(ob);
    face_num = ob->face->number;

    /* This is a multipart object, and we are not at the lower
     * right corner. So we need to store away the lower right corner.
     */
    if (!is_head && (head->arch->tail_x || head->arch->tail_y)
    && (head->arch->tail_x != ob->arch->clone.x || head->arch->tail_y != ob->arch->clone.y)) {
        int bx, by, l;

        /* Basically figure out where the offset is from where we
         * are right now. the ob->arch->clone.{x,y} values hold
         * the offset that this current piece is from the head,
         * and the tail is where the tail is from the head.
         * Note that bx and by will equal sx and sy if we are
         * already working on the bottom right corner. If ob is
         * the head, the clone values will be zero, so the right
         * thing will still happen.
         */
        bx = ax+head->arch->tail_x-ob->arch->clone.x;
        by = ay+head->arch->tail_y-ob->arch->clone.y;

        /* I don't think this can ever happen, but better to check
         * for it just in case.
         */
        if (bx < ax || by < ay) {
            LOG(llevError, "map2_add_ob: bx (%d) or by (%d) is less than ax (%d) or ay (%d)\n", bx, by, ax, ay);
            face_num = 0;
        }
        /* the target position must be within +/-1 of our current
         * layer as the layers are defined. We are basically checking
         * to see if we have already stored this object away.
         */
        for (l = layer-1; l <= layer+1; l++) {
            if (l < 0 || l >= MAP_LAYERS)
                continue;
            if (heads[by][bx][l] == head)
                break;
        }
        /* Didn't find it.  So we need to store it away. Try to store it
         * on our original layer, and then move up a layer.
         */
        if (l == layer+2) {
            if (!heads[by][bx][layer])
                heads[by][bx][layer] = head;
            else if (layer+1 < MAP_LAYERS && !heads[by][bx][layer+1])
                heads[by][bx][layer+1] = head;
        }
        return 0;
        /* Ok - All done storing away the head for future use */
    } else {
        (*has_obj)++;
        if (QUERY_FLAG(ob, FLAG_CLIENT_ANIM_SYNC)
        || QUERY_FLAG(ob, FLAG_CLIENT_ANIM_RANDOM)) {
            face_num = ob->animation_id|(1<<15);
            if (QUERY_FLAG(ob, FLAG_CLIENT_ANIM_SYNC))
                face_num |= ANIM_SYNC;
            else if (QUERY_FLAG(ob, FLAG_CLIENT_ANIM_RANDOM))
                face_num |= ANIM_RANDOM;
        }
        /* Since face_num includes the bits for the animation tag,
         * and we will store that away in the faces[] array, below
         * check works fine _except_ for the case where animation
         * speed chances.
         */
        if (ns->lastmap.cells[ax][ay].faces[layer] != face_num) {
            uint8 len, anim_speed = 0, i;

            /* This block takes care of sending the actual face
             * to the client. */
            ns->lastmap.cells[ax][ay].faces[layer] = face_num;

            /* Now form the data packet */
            nlayer = MAP2_LAYER_START+layer;

            len = 2;

            if (ob->map && !MAP_NOSMOOTH(ob->map)) {
                smoothlevel = ob->smoothlevel;
                if (smoothlevel)
                    len++;
            }

            if (QUERY_FLAG(ob, FLAG_CLIENT_ANIM_SYNC)
            || QUERY_FLAG(ob, FLAG_CLIENT_ANIM_RANDOM)) {
                len++;
                /* 1/0.004 == 250, so this is a good cap for an
                 * upper limit */
                if (ob->anim_speed)
                    anim_speed = ob->anim_speed;
                else if (FABS(ob->speed) < 0.004)
                    anim_speed = 255;
                else if (FABS(ob->speed) >= 1.0)
                    anim_speed = 1;
                else
                    anim_speed = (int)(1.0/FABS(ob->speed));

                if (!ns->anims_sent[ob->animation_id])
                    esrv_send_animation(ns, ob->animation_id);

                /* If smoothing, need to send smoothing information
                 * for all faces in the animation sequence. Since
                 * smoothlevel is an object attribute,
                 * it applies to all faces.
                 */
                if (smoothlevel) {
                    for (i = 0; i < NUM_ANIMATIONS(ob); i++) {
                        if (!(ns->faces_sent[animations[ob->animation_id].faces[i]->number]&NS_FACESENT_SMOOTH))
                            send_smooth(ns, animations[ob->animation_id].faces[i]->number);
                    }
                }
            } else if (!(ns->faces_sent[face_num]&NS_FACESENT_FACE)) {
                esrv_send_face(ns, face_num, 0);
            }

            if (smoothlevel
            && !(ns->faces_sent[ob->face->number]&NS_FACESENT_SMOOTH))
                send_smooth(ns, ob->face->number);

            /* Length of packet */
            nlayer |= len<<5;

            SockList_AddChar(sl, nlayer);
            SockList_AddShort(sl, face_num);
            if (anim_speed)
                SockList_AddChar(sl, anim_speed);
            if (smoothlevel)
                SockList_AddChar(sl, smoothlevel);
            return 1;
        } /* Face is different */
    }
    return 0;
}

/* This function is used see if a layer needs to be cleared.
 * It updates the socklist, and returns 1 if the update is
 * needed, 0 otherwise.
 */
static int map2_delete_layer(int ax, int ay, int layer, SockList *sl, socket_struct *ns) {
    int nlayer;

    if (ns->lastmap.cells[ax][ay].faces[layer] != 0) {
        /* Now form the data packet */
        nlayer = 0x10+layer+(2<<5);
        SockList_AddChar(sl, nlayer);
        SockList_AddShort(sl, 0);
        ns->lastmap.cells[ax][ay].faces[layer] = 0;
        return 1;
    }
    return 0;
}

/**
 * Check if a hp bar should be added to the map square.
 * @param ax x coordinate.
 * @param ay y coordinate.
 * @param ob object to check for hp bar.
 * @param sl where to write data.
 * @param ns to know if the face was sent or not.
 * @param has_obj number of objects, modified.
 * @param alive_layer will be filled with the layer containing the hp bar if applicable.
 * @return 1 if a bar was added, in which case alive_layer is modified, else 0.
 */
static int check_probe(int ax, int ay, const object *ob, SockList *sl, socket_struct *ns, int *has_obj, int *alive_layer) {
    int got_one = 0, poisoned = 0, diseased = 0;
    char name[60];
    int value, granularity;
    const object *probe;

    /* send hp bar if needed */
    if (!QUERY_FLAG(ob, FLAG_PROBE) || (*alive_layer) != -1 || ob->head)
        return 0;

    probe = object_find_by_type_and_name(ob, FORCE, "probe_force");
    if (probe == NULL || probe->level < 15) {
        /* if probe is not null, this is an error, but well */
        return 0;
    }

    granularity = (probe->level - 14) / 3;
    if (granularity <= 0)
        granularity = 1;
    else if (granularity > 30)
        granularity = 30;

    if (ob->stats.maxhp > 0) {
        value = (ob->stats.hp * granularity) / (ob->stats.maxhp);

        if (value < 0)
            value = 0;
        else if (value > granularity)
            value = granularity;
    } else
        value = 30;

    value = (value * 30) / granularity;

    if (object_present_in_ob(POISONING, ob) != NULL)
        poisoned = 1;
    if (object_present_in_ob(DISEASE, ob) != NULL)
        diseased = 1;

    if (value > 0) {
        archetype *dummy;

        snprintf(name, sizeof(name), "hpbar%s%s%s_%d",
            poisoned ? "_poisoned" : "",
            diseased ? "_diseased" : "",
            (!poisoned && !diseased) ? "_standard" : "",
            value);
        dummy = try_find_archetype(name);
        if (dummy != NULL) {
            got_one += map2_add_ob(ax, ay, MAP_LAYER_FLY2, &dummy->clone, sl, ns, has_obj, 0);
            (*alive_layer) = MAP_LAYER_FLY2;
        }
    }

    return got_one;
}

/*
 * This function is used to check a space (ax, ay) whose only
 * data we may care about are any heads. Basically, this
 * space is out of direct view. This is only used with the
 * Map2 protocol.
 *
 * @param ax
 * viewport relative x-coordinate
 * @param ay
 * viewport relative y-coordinate
 * @param sl
 * the reply to append to
 * @param ns
 * the client socket
 */
static void check_space_for_heads(int ax, int ay, SockList *sl, socket_struct *ns) {
    int layer, got_one = 0, del_one = 0, oldlen, has_obj = 0;
    uint16 coord;

    coord = MAP2_COORD_ENCODE(ax, ay, 0);
    oldlen = sl->len;
    SockList_AddShort(sl, coord);

    for (layer = 0; layer < MAP_LAYERS; layer++) {
        const object *head;

        head = heads[ay][ax][layer];
        if (head) {
            /* in this context, got_one should always increase
             * because heads should always point to data to really send.
             */
            got_one += map2_add_ob(ax, ay, layer, head, sl, ns, &has_obj, 1);
        } else {
            del_one += map2_delete_layer(ax, ay, layer, sl, ns);
        }
    }
    /* Note - if/when lighting information is added, some code is
     * needed here - lighting sources that are out of sight may still
     * extend into the viewable area.
     */

    /* If nothing to do for this space, we
     * can erase the coordinate bytes
     */
    if (!del_one && !got_one) {
        sl->len = oldlen;
    } else if (del_one && !has_obj) {
        /* If we're only deleting faces and not adding, and there
         * are not any faces on the space we care about,
         * more efficient
         * to send 0 as the type/len field.
         */
        sl->len = oldlen+2;         /* 2 bytes for coordinate */
        SockList_AddChar(sl, 0);    /* Clear byte */
        SockList_AddChar(sl, 255);  /* Termination byte */
        map_clearcell(&ns->lastmap.cells[ax][ay], 0, 0);
    } else {
        SockList_AddChar(sl, 255);  /* Termination byte */
    }
}

void draw_client_map2(object *pl) {
    int x, y, ax, ay, d, min_x, max_x, min_y, max_y, oldlen, layer;
    size_t startlen;
    sint16 nx, ny;
    SockList sl;
    uint16 coord;
    mapstruct *m;
    object *ob;

    SockList_Init(&sl);
    SockList_AddString(&sl, "map2 ");
    startlen = sl.len;

    /* Handle map scroll */
    if (pl->contr->socket.map_scroll_x || pl->contr->socket.map_scroll_y) {
        coord = MAP2_COORD_ENCODE(pl->contr->socket.map_scroll_x, pl->contr->socket.map_scroll_y, 1);
        pl->contr->socket.map_scroll_x = 0;
        pl->contr->socket.map_scroll_y = 0;
        SockList_AddShort(&sl, coord);
    }

    /* Init data to zero */
    memset(heads, 0, sizeof(heads));

    /* We could do this logic as conditionals in the if statement,
     * but that started to get a bit messy to look at.
     */
    min_x = pl->x-pl->contr->socket.mapx/2;
    min_y = pl->y-pl->contr->socket.mapy/2;
    max_x = pl->x+(pl->contr->socket.mapx+1)/2+MAX_HEAD_OFFSET;
    max_y = pl->y+(pl->contr->socket.mapy+1)/2+MAX_HEAD_OFFSET;

    /* x, y are the real map locations. ax, ay are viewport relative
     * locations.
     */
    ay = 0;
    for (y = min_y; y < max_y; y++, ay++) {
        ax = 0;
        for (x = min_x; x < max_x; x++, ax++) {
            /* If this space is out of the normal viewable area,
             * we only check the heads value. This is used to
             * handle big images - if they extend to a viewable
             * portion, we need to send just the lower right corner.
             */
            if (ax >= pl->contr->socket.mapx || ay >= pl->contr->socket.mapy) {
                check_space_for_heads(ax, ay, &sl, &pl->contr->socket);
            } else {
                /* This space is within the viewport of the client. Due
                 * to walls or darkness, it may still not be visible.
                 */

                /* Meaning of d:
                 * 0 - object is in plain sight, full brightness.
                 * 1 - MAX_DARKNESS - how dark the space is - higher
                 * value is darker space. If level is at max darkness,
                 * you can't see the space (too dark)
                 * 100 - space is blocked from sight.
                 */
                d = pl->contr->blocked_los[ax][ay];

                /* If the coordinates are not valid, or it is too
                 * dark to see, we tell the client as such
                 */
                nx = x;
                ny = y;
                m = get_map_from_coord(pl->map, &nx, &ny);
                coord = MAP2_COORD_ENCODE(ax, ay, 0);

                if (!m) {
                    /* space is out of map. Update space and clear
                     * values if this hasn't already been done.
                     * If the space is out of the map, it shouldn't
                     * have a head.
                     */
                    if (pl->contr->socket.lastmap.cells[ax][ay].darkness != 0) {
                        SockList_AddShort(&sl, coord);
                        SockList_AddChar(&sl, MAP2_TYPE_CLEAR);
                        SockList_AddChar(&sl, 255); /* Termination byte */
                        map_clearcell(&pl->contr->socket.lastmap.cells[ax][ay], 0, 0);
                    }
                } else if (d >= MAX_LIGHT_RADII) {
                    /* This block deals with spaces that are not
                     * visible due to darkness or walls. Still
                     * need to send the heads for this space.
                     */
                    check_space_for_heads(ax, ay, &sl, &pl->contr->socket);
                } else {
                    int have_darkness = 0, has_obj = 0, got_one = 0, del_one = 0, g1, alive_layer = -1, old_got;

                    /* In this block, the space is visible. */

                    /* Rather than try to figure out what everything
                     * that we might need to send is, then form the
                     * packet after that, we presume that we will in
                     * fact form a packet, and update the bits by what
                     * we do actually send. If we send nothing, we
                     * just back out sl.len to the old value, and no
                     * harm is done.
                     * I think this is simpler than doing a bunch of
                     * checks to see what if anything we need to send,
                     * setting the bits, then doing those checks again
                     * to add the real data.
                     */

                    oldlen = sl.len;
                    SockList_AddShort(&sl, coord);

                    /* Darkness changed */
                    if (pl->contr->socket.lastmap.cells[ax][ay].darkness != d
                    && pl->contr->socket.darkness) {
                        pl->contr->socket.lastmap.cells[ax][ay].darkness = d;
                        /* Darkness tag & length*/
                        SockList_AddChar(&sl, MAP2_TYPE_DARKNESS|1<<5);
                        SockList_AddChar(&sl, 255-d*(256/MAX_LIGHT_RADII));
                        have_darkness = 1;
                    }

                    for (layer = 0; layer < MAP_LAYERS; layer++) {
                        ob = GET_MAP_FACE_OBJ(m, nx, ny, layer);

                        /* Special case: send player itself if invisible */
                        if (!ob
                        && x == pl->x
                        && y == pl->y
                        && (pl->invisible&(pl->invisible < 50 ? 4 : 1))
                        && (layer == MAP_LAYER_LIVING1 || layer == MAP_LAYER_LIVING2))
                            ob = pl;

                        if (ob) {
                            g1 = has_obj;
                            old_got = got_one;
                            got_one += map2_add_ob(ax, ay, layer, ob, &sl, &pl->contr->socket, &has_obj, 0);

                            /* if we added the face, or it is a monster's head, check probe spell */
                            if (got_one != old_got || (ob->head == NULL && ob->more))
                                got_one += check_probe(ax, ay, ob, &sl, &pl->contr->socket, &has_obj, &alive_layer);

                            /* If we are just storing away the head
                             * for future use, then effectively this
                             * space/layer is blank, and we should clear
                             * it if needed.
                             */
                            if (g1 == has_obj) {
                                del_one += map2_delete_layer(ax, ay, layer, &sl, &pl->contr->socket);
                            } else if (ob->head == NULL) {
                                /* for single-part items */
                                got_one += check_probe(ax, ay, ob, &sl, &pl->contr->socket, &has_obj, &alive_layer);
                            }
                        } else {
                            if (layer != alive_layer)
                                del_one += map2_delete_layer(ax, ay, layer, &sl, &pl->contr->socket);
                        }
                    }
                    /* If nothing to do for this space, we
                     * can erase the coordinate bytes
                     */
                    if (!del_one && !got_one && !have_darkness) {
                        sl.len = oldlen;
                    } else if (del_one && !has_obj) {
                        /* If we're only deleting faces and don't
                         * have any objs we care about, just clear
                         * space. Note it is possible we may have
                         * darkness, but if there is nothing on the
                         * space, darkness isn't all that interesting
                         * - we can send it when an object shows up.
                         */
                        sl.len = oldlen+2;          /* 2 bytes for coordinate */
                        SockList_AddChar(&sl, MAP2_TYPE_CLEAR);
                        SockList_AddChar(&sl, 255); /* Termination byte */
                        map_clearcell(&pl->contr->socket.lastmap.cells[ax][ay], 0, 0);
                    } else {
                        SockList_AddChar(&sl, 255); /* Termination byte */
                    }
                }
            } /* else this is a viewable space */
        } /* for x loop */
    } /* for y loop */

    /* Only send this if there are in fact some differences. */
    if (sl.len > startlen) {
        Send_With_Handling(&pl->contr->socket, &sl);
    }
    SockList_Term(&sl);
}

/**
 * Draws client map.
 */
void draw_client_map(object *pl) {
    int i, j;
    sint16 ax, ay;
    int mflags;
    mapstruct *m, *pm;
    int min_x, min_y, max_x, max_y;

    if (pl->type != PLAYER) {
        LOG(llevError, "draw_client_map called with non player/non eric-server\n");
        return;
    }

    if (pl->contr->transport) {
        pm = pl->contr->transport->map;
    } else
        pm = pl->map;

    /* If player is just joining the game, he isn't here yet, so
     * the map can get swapped out. If so, don't try to send them
     * a map. All will be OK once they really log in.
     */
    if (pm == NULL || pm->in_memory != MAP_IN_MEMORY)
        return;

    /*
     * This block just makes sure all the spaces are properly
     * updated in terms of what they look like.
     */
    min_x = pl->x-pl->contr->socket.mapx/2;
    min_y = pl->y-pl->contr->socket.mapy/2;
    max_x = pl->x+(pl->contr->socket.mapx+1)/2;
    max_y = pl->y+(pl->contr->socket.mapy+1)/2;
    for (j = min_y; j < max_y; j++) {
        for (i = min_x; i < max_x; i++) {
            ax = i;
            ay = j;
            m = pm;
            mflags = get_map_flags(m, &m, ax, ay, &ax, &ay);
            if (mflags&P_OUT_OF_MAP)
                continue;
            if (mflags&P_NEED_UPDATE)
                update_position(m, ax, ay);
            /* If a map is visible to the player, we don't want
             * to swap it out just to reload it. This should
             * really call something like swap_map, but this is
             * much more efficient and 'good enough'
             */
            if (mflags&P_NEW_MAP)
                m->timeout = 50;
        }
    }

    /* do LOS after calls to update_position */
    if (pl->contr->do_los) {
        update_los(pl);
        pl->contr->do_los = 0;
    }

    draw_client_map2(pl);
}

void esrv_map_scroll(socket_struct *ns, int dx, int dy) {
    struct Map newmap;
    int x, y, mx, my;

    ns->map_scroll_x += dx;
    ns->map_scroll_y += dy;

    mx = ns->mapx+MAX_HEAD_OFFSET;
    my = ns->mapy+MAX_HEAD_OFFSET;

    /* the x and y here are coordinates for the new map, i.e. if we moved
     * (dx,dy), newmap[x][y] = oldmap[x-dx][y-dy]. For this reason,
     * if the destination x or y coordinate is outside the viewable
     * area, we clear the values - otherwise, the old values
     * are preserved, and the check_head thinks it needs to clear them.
     */
    for (x = 0; x < mx; x++) {
        for (y = 0; y < my; y++) {
            if (x >= ns->mapx || y >= ns->mapy) {
                /* clear cells outside the viewable area */
                memset(&newmap.cells[x][y], 0, sizeof(newmap.cells[x][y]));
            } else if (x+dx < 0 || x+dx >= ns->mapx || y+dy < 0 || y+dy >= ns->mapy) {
                /* clear newly visible tiles within the viewable area */
                memset(&newmap.cells[x][y], 0, sizeof(newmap.cells[x][y]));
            } else {
                memcpy(&newmap.cells[x][y], &ns->lastmap.cells[x+dx][y+dy], sizeof(newmap.cells[x][y]));
            }
        }
    }

    memcpy(&ns->lastmap, &newmap, sizeof(ns->lastmap));
}

/**
 * GROS: The following one is used to allow a plugin to send a generic cmd to
 * a player. Of course, the client need to know the command to be able to
 * manage it !
 */
void send_plugin_custom_message(object *pl, char *buf) {
    SockList sl;

    SockList_Init(&sl);
    SockList_AddString(&sl, buf);
    Send_With_Handling(&pl->contr->socket, &sl);
    SockList_Term(&sl);
}

/**
 * This looks for any spells the player may have that have changed
 * their stats. It then sends an updspell packet for each spell that
 * has changed in this way.
 */
void esrv_update_spells(player *pl) {
    SockList sl;
    int flags = 0;
    client_spell *spell_info;

    if (!pl->socket.monitor_spells)
        return;

    /* Handles problem at login, where this is called from fix_object
     * before we have had a chance to send spells to the player. It does seem
     * to me that there should never be a case where update_spells is called
     * before add_spells has been called. Add_spells() will update the
     * spell_state to non null.
     */
    if (!pl->spell_state)
        return;

    FOR_INV_PREPARE(pl->ob, spell) {
        if (spell->type == SPELL) {
            spell_info = get_client_spell_state(pl, spell);
            /* check if we need to update it*/
            if (spell_info->last_sp != SP_level_spellpoint_cost(pl->ob, spell, SPELL_MANA)) {
                spell_info->last_sp = SP_level_spellpoint_cost(pl->ob, spell, SPELL_MANA);
                flags |= UPD_SP_MANA;
            }
            if (spell_info->last_grace != SP_level_spellpoint_cost(pl->ob, spell, SPELL_GRACE)) {
                spell_info->last_grace = SP_level_spellpoint_cost(pl->ob, spell, SPELL_GRACE);
                flags |= UPD_SP_GRACE;
            }
            if (spell_info->last_dam != spell->stats.dam+SP_level_dam_adjust(pl->ob, spell)) {
                spell_info->last_dam = spell->stats.dam+SP_level_dam_adjust(pl->ob, spell);
                flags |= UPD_SP_DAMAGE;
            }
            if (flags != 0) {
                SockList_Init(&sl);
                SockList_AddString(&sl, "updspell ");
                SockList_AddChar(&sl, flags);
                SockList_AddInt(&sl, spell->count);
                if (flags&UPD_SP_MANA)
                    SockList_AddShort(&sl, spell_info->last_sp);
                if (flags&UPD_SP_GRACE)
                    SockList_AddShort(&sl, spell_info->last_grace);
                if (flags&UPD_SP_DAMAGE)
                    SockList_AddShort(&sl, spell_info->last_dam);
                flags = 0;
                Send_With_Handling(&pl->socket, &sl);
                SockList_Term(&sl);
            }
        }
    } FOR_INV_FINISH();
}

void esrv_remove_spell(player *pl, object *spell) {
    SockList sl;

    if (!pl->socket.monitor_spells)
        return;
    if (!pl || !spell || spell->env != pl->ob) {
        LOG(llevError, "Invalid call to esrv_remove_spell\n");
        return;
    }
    SockList_Init(&sl);
    SockList_AddString(&sl, "delspell ");
    SockList_AddInt(&sl, spell->count);
    Send_With_Handling(&pl->socket, &sl);
    SockList_Term(&sl);
}

/**
 * Sends the "pickup" state to pl if client wants it requested.
 *
 * @param pl
 * player that just logged in.
 */
void esrv_send_pickup(player *pl) {
    SockList sl;

    if (!pl->socket.want_pickup)
        return;
    SockList_Init(&sl);
    SockList_AddString(&sl, "pickup ");
    SockList_AddInt(&sl, pl->mode);
    Send_With_Handling(&pl->socket, &sl);
    SockList_Term(&sl);
}

/**
 * Give the client-side use information for a spell, to know how to use a spell.
 * @param spell what to check.
 * @retval 0 spell needs no argument.
 * @retval 1 spell needs the name of another spell.
 * @retval 2 spell can use a freeform string argument.
 * @retval 3 spell requires a freeform string argument.
 */
static int spell_client_use(const object *spell) {
    if (spell->type == SP_RAISE_DEAD)
        return 3;

    if (spell->type == SP_RUNE && !spell->other_arch)
        return 1;

    if (spell->type == SP_MAKE_MARK)
        return 3;

    if (spell->type == SP_CREATE_FOOD)
        return 2;

    if (spell->type == SP_SUMMON_MONSTER && spell->randomitems != NULL)
        return 2;

    if (spell->type == SP_CREATE_MISSILE)
        return 2;

    return 0;
}

/** appends the spell *spell to the Socklist we will send the data to. */
static void append_spell(player *pl, SockList *sl, object *spell) {
    client_spell *spell_info;
    int len, i, skill = 0;

    if (!spell->name) {
        LOG(llevError, "item number %d is a spell with no name.\n", spell->count);
        return;
    }

    if (spell->face && !(pl->socket.faces_sent[spell->face->number]&NS_FACESENT_FACE))
        esrv_send_face(&pl->socket, spell->face->number, 0);

    spell_info = get_client_spell_state(pl, spell);
    SockList_AddInt(sl, spell->count);
    SockList_AddShort(sl, spell->level);
    SockList_AddShort(sl, spell->casting_time);
    /* store costs and damage in the object struct, to compare to later */
    spell_info->last_sp = SP_level_spellpoint_cost(pl->ob, spell, SPELL_MANA);
    spell_info->last_grace = SP_level_spellpoint_cost(pl->ob, spell, SPELL_GRACE);
    spell_info->last_dam = spell->stats.dam+SP_level_dam_adjust(pl->ob, spell);
    /* send the current values */
    SockList_AddShort(sl, spell_info->last_sp);
    SockList_AddShort(sl, spell_info->last_grace);
    SockList_AddShort(sl, spell_info->last_dam);

    /* figure out which skill it uses, if it uses one */
    if (spell->skill) {
        for (i = 1; i < NUM_SKILLS; i++)
            if (!strcmp(spell->skill, skill_names[i])) {
                skill = i+CS_STAT_SKILLINFO;
                break;
            }
    }
    SockList_AddChar(sl, skill);

    SockList_AddInt(sl, spell->path_attuned);
    SockList_AddInt(sl, spell->face ? spell->face->number : 0);
    SockList_AddLen8Data(sl, spell->name, strlen(spell->name));

    if (!spell->msg) {
        SockList_AddShort(sl, 0);
    } else {
        len = strlen(spell->msg);
        SockList_AddShort(sl, len);
        SockList_AddData(sl, spell->msg, len);
    }

    /* Extended spell information available if the client wants it.
     */
    if (pl->socket.monitor_spells >= 2) {
        /* spellmon 2
         */
        sstring req = object_get_value(spell, "casting_requirements");

        SockList_AddChar(sl, spell_client_use(spell));  /* Usage code */

        if (req) {                                      /* Requirements */
            SockList_AddLen8Data(sl, req, strlen(req));
        } else {
            SockList_AddChar(sl, 0);
        }
        /* end spellmon 2
         */
    }
}

/**
 * This tells the client to add the spell *spell, if spell is NULL, then add
 * all spells in the player's inventory.
 */
void esrv_add_spells(player *pl, object *spell) {
    SockList sl;
    size_t size;
    sstring value;

    if (!pl) {
        LOG(llevError, "esrv_add_spells, tried to add a spell to a NULL player\n");
        return;
    }

    if (!pl->socket.monitor_spells)
        return;

    SockList_Init(&sl);
    SockList_AddString(&sl, "addspell ");
    if (!spell) {
        FOR_INV_PREPARE(pl->ob, spell) {
            if (spell->type != SPELL)
                continue;
            /* Were we to simply keep appending data here, we could
             * exceed the SockList buffer if the player has enough spells
             * to add. We know that append_spell will always append
             * 23 data bytes, plus 3 length bytes and 2 strings
             * (because that is the spec) so we need to check that
             * the length of those 2 strings, plus the 26 bytes,
             * won't take us over the length limit for the socket.
             * If it does, we need to send what we already have,
             * and restart packet formation.
             */
            size = 26+strlen(spell->name)+(spell->msg ? strlen(spell->msg) : 0);
            if (pl->socket.monitor_spells >= 2) {
                /** @todo casting_requirements should be a constant somewhere */
                value = object_get_value(spell, "casting_requirements");
                size += 2 + (value ? strlen(value) : 0);
            }
            if (SockList_Avail(&sl) < size) {
                Send_With_Handling(&pl->socket, &sl);
                SockList_Reset(&sl);
                SockList_AddString(&sl, "addspell ");
            }
            append_spell(pl, &sl, spell);
        } FOR_INV_FINISH();
    } else if (spell->type != SPELL) {
        LOG(llevError, "Asked to send a non-spell object as a spell\n");
        return;
    } else
        append_spell(pl, &sl, spell);
    /* finally, we can send the packet */
    Send_With_Handling(&pl->socket, &sl);
    SockList_Term(&sl);
}

/* sends a 'tick' information to the client.
 * We also take the opportunity to toggle TCP_NODELAY -
 * this forces the data in the socket to be flushed sooner to the
 * client - otherwise, the OS tries to wait for full packets
 * and will this hold sending the data for some amount of time,
 * which thus adds some additional latency.
 */
void send_tick(player *pl) {
    SockList sl;
    int tmp;

    SockList_Init(&sl);
    SockList_AddString(&sl, "tick ");
    SockList_AddInt(&sl, pticks);
    tmp = 1;
    if (setsockopt(pl->socket.fd, IPPROTO_TCP, TCP_NODELAY, &tmp, sizeof(tmp)))
        LOG(llevError, "send_tick: Unable to turn on TCP_NODELAY\n");

    Send_With_Handling(&pl->socket, &sl);
    tmp = 0;
    if (setsockopt(pl->socket.fd, IPPROTO_TCP, TCP_NODELAY, &tmp, sizeof(tmp)))
        LOG(llevError, "send_tick: Unable to turn off TCP_NODELAY\n");
    SockList_Term(&sl);
}

/**
 * Basic helper function which adds a piece of data for
 * the accountplayers protocol command.  Called from
 * send_account_players.  If data is empty, we don't add.
 *
 * @param sl
 * socklist to add data to.
 * @param type
 * type of data (ACL_.. value)
 * @param data
 * string data to add
 */
static void add_char_field(SockList *sl, int type, const char *data)
{
    int len;

    len = strlen(data);

    if (len) {
        /* one extra for length for the type byte */
        SockList_AddChar(sl, len+1);
        SockList_AddChar(sl, type);
        SockList_AddString(sl, data);
    }
}

/**
 * Upon successful login/account creation, we send a list of
 * characters associated with the account to the client - in
 * this way, it lets the client present a nice list so that
 * the player can choose one.
 * Note it is important that ns->account_name is set before
 * calling this.
 * Note 2: Some of the operations here are not especially
 * efficient - O(n^2) or repeating same loop instead of
 * trying to combine them.  This is not a worry as
 * MAX_CHARACTERS_PER_ACCOUNT is a fairly low value
 * (less than 20), so even inefficient operations don't take
 * much time.  If that value as a lot larger, then some
 * rethink may be needed.  For now, having clearer code
 * is better than trying to save a few microseconds of
 * execution time.
 *
 * @param ns
 * socket structure to send data for.
 */
void send_account_players(socket_struct *ns)
{
    SockList sl;
    Account_Char *acn;
    int i, num_chars, need_send[MAX_CHARACTERS_PER_ACCOUNT];
    char **chars;

    ns->account_chars = account_char_load(ns->account_name);

    /*
     * The acocunt logic here is a little tricky - account_char_load()
     * is best source as it has a lot more data.  However, if a user
     * has just added an player to an account, that will not be filled
     * in until the player has actually logged in with that character -
     * to fill the data in at time of adding the character to the account
     * requires a fair amount of work to check_login(), since the load
     * of the player file and other initialization is fairly closely
     * intertwined.  So until that is done, we still at least have
     * account names we can get and send.
     * note: chars[] has the last entry NULL terminated - thus,
     * if there are 2 valid accounts, chars[0], chars[1] will be
     * set, and chars[2] will be NULL.  chars[3...MAX] will have
     * undefined values.
     */
    chars = account_get_players_for_account(ns->account_name);

    SockList_Init(&sl);
    SockList_AddString(&sl, "accountplayers ");
    num_chars=0;

    /* First, set up an array so we know which character we may
     * need to send specially.  Only non NULL values would
     * ever need to get sent.
     */
    for (i=0; i<MAX_CHARACTERS_PER_ACCOUNT; i++) {
        if (chars[i])
            need_send[i] = 1;
        else break;
    }
    /* This counts up the number of characters.
     * But also, we look and see if the character exists
     * in chars[i] - if so, we set need_send[i] to 0.
     */
    for (acn = ns->account_chars; acn; acn = acn->next) {
        num_chars++;
        for (i=0; i<MAX_CHARACTERS_PER_ACCOUNT; i++) {
            /* If this is NULL, we know there will not be
             * any more entries - so break out.
             */
            if (!chars[i]) break;

            if (!strcmp(chars[i], acn->name)) {
                need_send[i] = 0;
                break;
            }
        }
    }

    /* total up number with limited information */
    for (i=0; i< MAX_CHARACTERS_PER_ACCOUNT; i++) {
        if (!chars[i]) break;

        if (need_send[i])
            num_chars++;
    }

    SockList_AddChar(&sl, num_chars);

    /* Now add real character data */
    for (acn = ns->account_chars; acn; acn = acn->next) {
        /* Ignore a dead character. They don't need to show up. */
        if (acn->isDead)
            continue;
        uint16 faceno;

        add_char_field(&sl, ACL_NAME, acn->name);
        add_char_field(&sl, ACL_CLASS, acn->character_class);
        add_char_field(&sl, ACL_RACE, acn->race);
        add_char_field(&sl, ACL_FACE, acn->face);
        if (acn->face[0] != 0 ) {
            faceno = find_face(acn->face, 0);

            if (faceno != 0) {
                if (!(ns->faces_sent[faceno]&NS_FACESENT_FACE)) {
                    esrv_send_face(ns, faceno, 0);
                }
            }
        } else
            faceno=0;

        add_char_field(&sl, ACL_PARTY, acn->party);
        add_char_field(&sl, ACL_MAP, acn->map);
        SockList_AddChar(&sl, 3);
        SockList_AddChar(&sl, ACL_LEVEL);
        SockList_AddShort(&sl, acn->level);
        if (faceno) {
            SockList_AddChar(&sl, 3);
            SockList_AddChar(&sl, ACL_FACE_NUM);
            SockList_AddShort(&sl, faceno);
        }

        SockList_AddChar(&sl, 0);
    }
    /* Now for any characters where we just have the name */
    for (i=0; i< MAX_CHARACTERS_PER_ACCOUNT; i++) {
        if (!chars[i]) break;

        if (need_send[i]) {
            add_char_field(&sl, ACL_NAME, chars[i]);
            SockList_AddChar(&sl, 0);
        }
    }

    Send_With_Handling(ns, &sl);
    SockList_Term(&sl);
}

/**
 * This is a basic routine which extracts the name/password
 * from the buffer.  Several of the account login routines
 * provide a length prefixed string for name, and another for
 * password.
 *
 * @param buf
 * character data to process.
 * @param len
 * length of this buffer.  This will be updated to be the next
 * byte to process in the case that there is additional data
 * in this packet that the caller may need to process.
 * @param name
 * preallocated (MAX_BUF) buffer to return the name (really, first string) in
 * @param password
 * preallocated (MAX_BUF) buffer to return the password (second string) in
 * @return
 * 0 - success
 * 1 - name is too long
 * 2 - password is too long
 */
static int decode_name_password(const char *buf, int *len, char *name, char *password)
{
    int nlen, plen;

    if (*len < 2) {
        return 1;
    }

    nlen = (unsigned char)buf[0];
    if (nlen >= MAX_BUF || nlen > *len-2) {
        return 1;
    }
    memcpy(name, buf+1, nlen);
    name[nlen] = 0;

    plen = (unsigned char)buf[nlen+1];
    if (plen >= MAX_BUF || plen > *len-2-nlen) {
        return 2;
    }
    memcpy(password, buf+2+nlen, plen);
    password[plen] = 0;

    *len = nlen+plen+2;

    return 0;
}
/**
 * Handles the account login
 *
 * @param buf
 * remaining socket data - from this we need to extract name & password
 * @param len
 * length of this buffer
 * @param ns
 * pointer to socket structure.
 */
void account_login_cmd(char *buf, int len, socket_struct *ns) {
    char name[MAX_BUF], password[MAX_BUF];
    int status;
    SockList sl;

    SockList_Init(&sl);

    status = decode_name_password(buf, &len, name, password);

    if (status == 1) {
        SockList_AddString(&sl, "failure accountlogin Name is too long");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }
    if (status == 2) {
        SockList_AddString(&sl, "failure accountlogin Password is too long");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }

    if (!account_exists(name)) {
        SockList_AddString(&sl, "failure accountlogin No such account name exists on this server");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }

    if (account_check_name_password(name, password)) {
        player *pl;
        socket_struct *tns;

        /* Checking against init_sockets must be done before
         * we set ns->account - otherwise we will match
         * that.  What we are doing here is limiting the account
         * to only one login.
         */
        tns = account_get_logged_in_init_socket(name);
        /* Other code will clean this up.  We could try to
         * set the same state this other socket is in, but we
         * really don't know what that state is, and then
         * we would have to try to communicate that to the client
         * so it can activate the right dialogs.  Simpler to
         * just go to a known state.
         */
        if (tns && tns != ns)
            tns->status = Ns_Dead;

        /* Same note as above applies - it can be simpler in
         * this case - we could check against the ST_PLAYING
         * value, but right now we don't have a method to
         * tell the client to go directly from login to playing.
         */
        pl = account_get_logged_in_player(name);
        if (pl)
            pl->socket.status = Ns_Dead;


        if (ns->account_name) free(ns->account_name);
        /* We want to store away official name so we do not
         * have any case sensitivity issues on the files.
         * because we have already checked password, we
         * know that account_exists should never return NULL in
         * this case.
         */
        ns->account_name = strdup_local(account_exists(name));

        send_account_players(ns);

    } else {
        SockList_AddString(&sl, "failure accountlogin Incorrect password for account");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
    }
}

/**
 * Checks if account creation is blocked for this connection.
 *
 * @param ns
 * pointer to socket structure.
 * @retval 0 Account creation is not blocked.
 * @retval 1 Account creation is blocked for this connection.
 */
static int account_block_create(const socket_struct *ns) {
    /* Check if account creation is blocked. */
    if(!settings.account_block_create) {
        /* Account creation is allowed for everyone. */
        return 0;
    }

    /* Has the trusted host been defined? */
    if(settings.account_trusted_host == NULL) {
      /* No, allocate it and set it to localhost now. */
      settings.account_trusted_host = strdup_local("127.0.0.1");
    }

    /* Return false if the client connected from the trusted host. */
    if(strcmp(ns->host, settings.account_trusted_host) == 0){
      return 0;
    }

    /*
     * If we are here, then we are blocking account create and we do
     * not trust this client's IP address.
     */
    return 1;
}

/**
 * Handles the account creation  This function shares a fair amount of
 * the same logic as account_login_cmd() above.
 *
 * @param buf
 * remaining socket data - from this we need to extract name & password
 * @param len
 * length of this buffer
 * @param ns
 * pointer to socket structure.
 */
void account_new_cmd(char *buf, int len, socket_struct *ns) {
    char name[MAX_BUF], password[MAX_BUF];
    int status;
    SockList sl;

    SockList_Init(&sl);

    status = decode_name_password(buf, &len, name, password);

    if (account_block_create(ns)) {
        LOG(llevInfo, "Account create blocked from %s\n", ns->host);
        SockList_AddString(&sl, "failure accountnew Account creation is disabled");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }

    if (status == 1) {
        SockList_AddString(&sl, "failure accountnew Name is too long");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }
    if (status == 2) {
        SockList_AddString(&sl, "failure accountnew Password is too long");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }
    /*The minimum length isn't exactly required, but in the current implementation,
     * client will send the same password for character for which there is a
     * 2 character minimum size. Thus an account with a one character password
     * won't be able to create a character. */
    if (strlen(password)<2) {
        SockList_AddString(&sl, "failure accountnew Password is too short");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }

    if (account_exists(name)) {
        SockList_AddString(&sl, "failure accountnew That account already exists on this server");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }

    status = account_check_string(name);
    if (status == 1) {
        SockList_AddString(&sl,
                       "failure accountnew That account name contains invalid characters.");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }

    if (status == 2) {
        SockList_AddString(&sl,
                           "failure accountnew That account name is too long");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }

    status = account_check_string(password);
    if (status == 1) {
        SockList_AddString(&sl,
                       "failure accountnew That password contains invalid characters.");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }

    if (status == 2) {
        SockList_AddString(&sl,
                           "failure accountnew That password is too long");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }

    /* If we got here, we passed all checks - so now add it */
    if (ns->account_name) free(ns->account_name);
    ns->account_name = strdup_local(name);
    account_add_account(name, password);
    /* save account information */
    accounts_save();
    send_account_players(ns);
}

/**
 * Handle accountaddplayer from server (add a character to this
 * account).
 * We check to see if character exists, if password is correct,
 * if character is associated with other account.
 *
 * @param buf
 * socket data to process
 * @param len
 * length of socket data.
 * @param ns
 * socket of incoming request.
 */
void account_add_player_cmd(char *buf, int len, socket_struct *ns) {
    char name[MAX_BUF], password[MAX_BUF];
    int status, force, nlen;
    SockList sl;
    const char *cp;

    SockList_Init(&sl);

    if (ns->account_name == NULL) {
        SockList_AddString(&sl, "failure accountaddplayer Not logged in");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }

    force = buf[0];
    nlen = len - 1;
    status = decode_name_password(buf+1, &nlen, name, password);
    if (status == 1) {
        SockList_AddString(&sl, "failure accountaddplayer Name is too long");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }
    if (status == 2) {
        SockList_AddString(&sl, "failure accountaddplayer Password is too long");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }

    status = verify_player(name, password);
    if (status) {
        /* From a security standpoint, telling random folks if it
         * it as wrong password makes it easier to hack.  However,
         * it is fairly easy to determine what characters exist on a server
         * (either by trying to create a new one and see if the name is in
         * in use, or just looking at the high score file), so this
         * really does not make things much less secure
         */
        if (status == 1)
            SockList_AddString(&sl, "failure accountaddplayer 0 The character does not exist.");
        else
            SockList_AddString(&sl, "failure accountaddplayer 0 That password is incorrect.");

        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }
    /* Check to see if this character is associated with an account.
     */
    cp = account_get_account_for_char(name);
    if (cp) {
        if (!strcmp(cp, ns->account_name)) {
            SockList_AddString(&sl, "failure accountaddplayer 0 That character is already connected to this account.");
            Send_With_Handling(ns, &sl);
            SockList_Term(&sl);
            return;
        } else {
            if (!force) {
                SockList_AddString(&sl, "failure accountaddplayer 1 That character is already connected to a different account.");
                Send_With_Handling(ns, &sl);
                SockList_Term(&sl);
                return;
            } else if (account_is_logged_in(cp)) {
                /* We could be clever and try to handle this case, but it is
                 * trickier.  If the character is logged in, it has to
                 * be logged out.  And the socket holds some data which
                 * needs to be cleaned up.  Since it should be fairly
                 * uncommon that users need to do this, just disallowing
                 * it makes things a lot simpler.
                 */
                SockList_AddString(&sl, "failure accountaddplayer 0 That character is already connected to a different account which is currently logged in.");
                Send_With_Handling(ns, &sl);
                SockList_Term(&sl);
                return;
            }
        }
    }
    /* If we have gotten this far, the name/password provided is OK,
     * and the character is not associated with a different account (or
     * force is true).  Now try to add the character to this account.
     */
    status = account_add_player_to_account(ns->account_name, name);

    /* This should never happen, but check for it just in case -
     * if we were able to log in, the account should exist.  but
     * if this fails, need to give the user some clue.
     */
    if (status==1) {
        SockList_AddString(&sl, "failure accountaddplayer 0 Could not find your account.");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    } else if (status == 2) {
        SockList_AddString(&sl, "failure accountaddplayer 0 You have reached the maximum number of characters allowed per account.");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }

    /* If cp is set, then this character used to belong to a different
     * account.  Remove it now.
     */
    if (cp) {
        Account_Char *chars;

        account_remove_player_from_account(cp, name);
        chars = account_char_load(cp);
        chars=account_char_remove(chars, name);
        account_char_save(cp, chars);
        account_char_free(chars);
    }

    send_account_players(ns);

    /* store data so nothing is lost in case of crash */
    account_char_save(ns->account_name, ns->account_chars);
}

/**
 * We have received an accountplay command.
 * try to log in and play the character.
 */
void account_play_cmd(char *buf, int len, socket_struct *ns)
{
    char **chars;
    int i;
    SockList sl;
    player *pl;

    SockList_Init(&sl);

    if (!buf[0]) {
        SockList_AddString(&sl, "failure accountplay Malformed character name");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }

    if (ns->account_name == NULL) {
        SockList_AddString(&sl, "failure accountplay Not logged in");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }

    chars = account_get_players_for_account(ns->account_name);

    for (i=0; i<MAX_CHARACTERS_PER_ACCOUNT; i++) {
        if (!chars[i] || !strcmp(chars[i], buf)) break;
    }
    /* Make sure a client is not trying to spoof us here */
    if (i == MAX_CHARACTERS_PER_ACCOUNT || !chars[i]) {
        SockList_AddPrintf(&sl,
                           "failure accountplay Character %s is not associated with account %s",
                           buf, ns->account_name);
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }

    /* from a protocol standpoint, accountplay can be used
     * before there is a player structure (first login) or after
     * (character has logged in and is changing characters).
     * Checkthe sockets for that second case - if so,
     * we don't need to make a new player object, etc.
     */
    for (pl=first_player; pl; pl=pl->next) {
        if (&pl->socket == ns) {
	  /* The player still in the socket must be saved first. */
	  save_player(pl->ob, 0);
	  break;
	} 
    }

    /* Some of this logic is from add_player()
     * we just don't use add_player() as it does some other work
     * we don't really want to do.
     */
    if (!pl) {
        pl = get_player(NULL);
        memcpy(&pl->socket, ns, sizeof(socket_struct));
        ns->faces_sent = NULL;
        SockList_ResetRead(&pl->socket.inbuf);
    } else {
        pl->state = ST_PLAYING;
    }

    pl->ob->name = add_string(buf);
    check_login(pl->ob, 0);

    SockList_AddString(&sl, "addme_success");
    Send_With_Handling(ns, &sl);
    SockList_Term(&sl);

    ns->status = Ns_Avail;
}

#define MAX_CHOICES 100
/**
 * We have received a createplayer command.
 * @param buf received command.
 * @param len length of buf.
 * @param ns socket to communicate with the client.
 */
void create_player_cmd(char *buf, int len, socket_struct *ns)
{
    char name[MAX_BUF], password[MAX_BUF], *choices[MAX_CHOICES];
    int status, nlen, choice_num=0, i;
    SockList sl;
    player *pl;
    archetype *map=NULL, *race_a=NULL, *class_a=NULL;
    living  new_stats;

    SockList_Init(&sl);

    nlen = len;
    status = decode_name_password(buf, &nlen, name, password);
    if (status == 1) {
        SockList_AddString(&sl, "failure createplayer Name is too long");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }

    /* 2 characters minimum for password */
    if (strlen(password)<2) {
        SockList_AddString(&sl, "failure createplayer Password is too short");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }

    /** too long, buffer overflow */
    if (status == 2) {
        SockList_AddString(&sl, "failure createplayer Password is too long");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }

    /* This is a fairly ugly solution - we're truncating the password.
     * however, the password information for characters is really
     * a legacy issue - when every character is associated with
     * an account, legacy login (character name/password) will get
     * removed, at which point the only use for password might be
     * to move characters from one account to another, but not sure
     * if that is something we want to allow.
     */
    if (strlen(password)>17)
        password[16] = 0;

    /* We just can't call check_name(), since that uses draw_info() to
     * report status.  We are also more permissive on names, so we use
     * account_check_string() - if that is safe for account, also safe
     * for player names.
     */
    if (account_check_string(name)) {
        SockList_AddString(&sl, "failure createplayer The name contains illegal characters");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }

    /* 1 means no such player, 0 is correct name/password (which in this
     * case, is an error), and 2 is incorrect password. Only way we
     * go onward is if there is no such player.
     */
    if (verify_player(name, password) != 1) {
        SockList_AddString(&sl, "failure createplayer That name is already in use");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }

    /* from a protocol standpoint, accountplay can be used
     * before there is a player structure (first login) or after
     * (character has logged in and is changing characters).
     * Check the sockets for that second case - if so,
     * we don't need to make a new player object, etc.
     */
    for (pl=first_player; pl; pl=pl->next)
      if (&pl->socket == ns) {
        if (pl->ob->name && !strcmp(pl->ob->name, name)) {
          /* For some reason not only the socket is the same but also 
           * the player is already playing. If this happens at this
           * point let's assume the character never was able to apply
           * a bet of reality to make a correct first-time save.
           * So, for safety remove it and start over. 
           */
          if (!QUERY_FLAG(pl->ob, FLAG_REMOVED))
            object_remove(pl->ob);
        }
        break;
      }

    /* In this mode, we have additional data
     * Note that because there are a lot of failure cases in here
     * (where we end up not creating the new player), the code
     * to create the new player is done within this routine
     * after all checks pass.  Note that all of the checks
     * done are done without using the player structure,
     * as pl may be null right now.
     */
    if (ns->login_method >= 2) {
        int i, j, stat_total=0;
        char *key, *value, *race=NULL, *class=NULL;

        /* By setting this to zero, then we can easily
         * check to see if all stats have been set.
         */
        memset(&new_stats, 0, sizeof(living));

        while (nlen < len) {
            i = buf[nlen];  /* Length of this line */
            /* Sanity check from client - don't want to loop
             * forever if there is a 0 length, and don't
             * want to read beyond size of packet.
             * Likewise, client should have sent
             * the string to us already null terminated,
             * but we will just make sure.
             */
            if ((i == 0) || (nlen + i > len)) break;
            buf[nlen + i] = 0;

            /* What we have are a series of lines -
             * 'key value' format.  Find that space,
             * and null it out so we can do strcasecmp.
             * If no space, abort processing
             */
            key = buf + nlen + 1;
            value = strchr(key, ' ');
            if (!value) break;
            *value = 0;
            value++;

            if (!strcasecmp(key,"race"))    race = value;
            else if (!strcasecmp(key,"class"))   class = value;
            else if (!strcasecmp(key,"starting_map")) {
                map = try_find_archetype(value);
                if (!map || map->clone.type != MAP || map->clone.subtype !=MAP_TYPE_CHOICE) {
                    SockList_AddString(&sl,
                               "failure createplayer Invalid or map");
                    Send_With_Handling(ns, &sl);
                    SockList_Term(&sl);
                    return;
                }
            }
            else if (!strcasecmp(key,"choice")) {
                /* In general, MAX_CHOICES should be large enough
                 * to always handle the choices from the client - of
                 * course, the client could be broken and send us many
                 * more choices than we should have, so handle that.
                 */
                if (choice_num == MAX_CHOICES) {
                    LOG(llevError,
                        "Number of choices receive exceed max value: %d>%d\n",
                        choice_num, MAX_CHOICES);
                } else {
                    choices[choice_num] = value;
                    choice_num++;
                }
            }
            else {
                /* Do stat processing here */
                for (j=0; j < NUM_STATS; j++) {
                    if (!strcasecmp(key,short_stat_name[j])) {
                        int val = atoi(value);

                        set_attr_value(&new_stats, j, val);
                        break;
                    }
                }
                if (j >= NUM_STATS) {
                    /* Bad clients could do this - we should at least report
                     * it, and useful when trying to add new parameters.
                     */
                    LOG(llevError, "Got unknown key/value from client: %s %s\n", key, value);
                }
            }
            nlen += i + 1;
        }
        /* Do some sanity checking now.  But checking the stat
         * values here, we will catch any 0 values since we do
         * a memset above.  A properly behaving client should
         * never do any of these things, but we do not presume
         * clients will behave properly.
         */
        for (j=0; j<NUM_STATS; j++) {
            int val = get_attr_value(&new_stats, j);

            stat_total += val;
            if (val > settings.starting_stat_max ||
                val < settings.starting_stat_min) {
                SockList_AddPrintf(&sl,
                    "failure createplayer Stat value is out of range - %d must be between %d and %d",
                     val, settings.starting_stat_min, settings.starting_stat_max);
                Send_With_Handling(ns, &sl);
                SockList_Term(&sl);
                return;
            }
        }
        if (stat_total > settings.starting_stat_points) {
            SockList_AddPrintf(&sl,
                "failure createplayer Total allocated statistics is higher than allowed (%d>%d)",
                stat_total, settings.starting_stat_points);
            Send_With_Handling(ns, &sl);
            SockList_Term(&sl);
            return;
        }

        if (race)
            race_a = try_find_archetype(race);

        if (class)
            class_a = try_find_archetype(class);

        /* This should never happen with a properly behaving client, so the error message
         * doesn't have to be that great.
         */
        if (!race_a || race_a->clone.type != PLAYER || !class_a || class_a->clone.type != CLASS) {
            SockList_AddString(&sl,
                               "failure createplayer Invalid or unknown race or class");
            Send_With_Handling(ns, &sl);
            SockList_Term(&sl);
            return;
        }

        /* At current time, only way this can fail is if the adjusted
         * stat is less than 1.
         */
        if (check_race_and_class(&new_stats, race_a, class_a)) {
            SockList_AddString(&sl,
                               "failure createplayer Unable to apply race or class - statistic is out of bounds");
            Send_With_Handling(ns, &sl);
            SockList_Term(&sl);
            return;
        }

        if (!pl)
            pl = add_player(ns, ADD_PLAYER_NEW | ADD_PLAYER_NO_MAP | ADD_PLAYER_NO_STATS_ROLL);

        apply_race_and_class(pl->ob, race_a, class_a, &new_stats);

    }  else {
        /* In thise case, old login method */
        if (!pl)
            pl = add_player(ns, ADD_PLAYER_NEW);
/* already done by add_player
        roll_again(pl->ob);
        pl->state = ST_ROLL_STAT;
        set_first_map(pl->ob);*/
    }

    /* add_player does a lot of the work, but there are a few
     * things we need to update, like starting name and
     * password.
     * This is done before processing in login_method>2.
     * The character creation process it does when
     * applying the race/class will use this
     * name information.
     */
    FREE_AND_COPY(pl->ob->name, name);
    FREE_AND_COPY(pl->ob->name_pl, name);
    pl->name_changed = 1;
    strcpy(pl->password, crypt_string(password, NULL));

    SockList_AddString(&sl, "addme_success");
    Send_With_Handling(ns, &sl);
    SockList_Term(&sl);

    if (ns->login_method >= 2) {
        /* The client could have provided us a map - if so, map will be set
         * and we don't want to overwrite it
         */
        if (!map)
            map = get_archetype_by_type_subtype(MAP, MAP_TYPE_DEFAULT);

        if (!map) {
            /* This should never happen - its not something that can
             * be easily worked around without other weird issues,
             * like having 2 classes.
             */
            LOG(llevError, "Can not find object of type MAP subtype MAP_TYPE_DEFAULT.\n");
            LOG(llevError, "Are the archetype files up to date?  Can not continue.\n");
            abort();
        }

        enter_exit(pl->ob, &map->clone);

        if (pl->ob->map == NULL) {
            LOG(llevError, "Couldn't put player %s on start map %s!", pl->ob->name, map->name);
            abort();
        }

        /* copy information to bed of reality information, in case the player dies */
        snprintf(pl->savebed_map, sizeof(pl->savebed_map), "%s", pl->ob->map->path);
        pl->bed_x = pl->ob->x;
        pl->bed_y = pl->ob->y;

        player_set_state(pl, ST_PLAYING);
    }

    /* We insert any objects after we have put the player on the map -
     * this makes things safer, as certain objects may expect a normal
     * environment.  Note that choice_num will only be set in the
     * loginmethod > 2, which also checks (and errors out) if the
     * race/class is not set, which is why explicit checking for
     * those is not need.
     */
    for (i=0; i < choice_num; i++) {
        char *choiceval, *cp;
        const char *value;
        archetype *arch;
        object *op;

        choiceval = strchr(choices[i], ' ');
        if (!choiceval) {
            LOG(llevError, "Choice does not specify value: %s\n", choices[i]);
            continue;
        }
        *choiceval=0;
        choiceval++;
        value = object_get_value(&race_a->clone, choices[i]);
        if (!value)
            value = object_get_value(&class_a->clone, choices[i]);

        if (!value) {
            LOG(llevError, "Choice not found in archetype: %s\n", choices[i]);
            continue;
        }
        cp = strstr(value, choiceval);
        if (!cp) {
            LOG(llevError, "Choice value not found in archetype: %s %s\n",
                choices[i], choiceval);
            continue;
        }

        /* Check to make sure that the matched string is an entire word,
         * and not a substring (eg, valid choice being great_sword but
         * we just get sword) - the space after the match should either be a
         * space or null, and space before match should also be a space
         * or the start of the string.
         */
        if ((cp[strlen(choiceval)] != ' ') && (cp[strlen(choiceval)] != 0) &&
            (cp != value) && (*(cp-1) != ' ')) {

            LOG(llevError, "Choice value matches substring but not entire word: %s substring %s\n",
                choiceval, value);
            continue;
        }
        arch = try_find_archetype(choiceval);
        if (!arch) {
            LOG(llevError, "Choice value can not find archetype %s\n", choiceval);
            continue;
        }
        op = arch_to_object(arch);
        op = object_insert_in_ob(op, pl->ob);
        if (QUERY_FLAG(op, FLAG_AUTO_APPLY))
            ob_apply(op, pl->ob, 0);
    }

    ns->status = Ns_Avail;
}

/**
 * Handles the account password change.
 *
 * @param buf
 * remaining socket data - from this we need to extract old & new password
 * @param len
 * length of this buffer
 * @param ns
 * pointer to socket structure.
 */
void account_password(char *buf, int len, socket_struct *ns) {
    char old[MAX_BUF], change[MAX_BUF];
    int status;
    SockList sl;

    SockList_Init(&sl);

    if (ns->account_name == NULL) {
        SockList_AddString(&sl, "failure accountpw Not logged in");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }

    status = decode_name_password(buf, &len, old, change);
    if (status == 1) {
        SockList_AddString(&sl, "failure accountpw Old password is too long");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }
    if (status == 2) {
        SockList_AddString(&sl, "failure accountpw New password is too long");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }
    /*The minimum length isn't exactly required, but in the current implementation,
     * client will send the same password for character for which there is a
     * 2 character minimum size. Thus an account with a one character password
     * won't be able to create a character. */
    if (strlen(change)<2) {
        SockList_AddString(&sl, "failure accountpw New password is too short");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }

    status = account_check_string(change);
    if (status == 1) {
        SockList_AddString(&sl,
                       "failure accountpw That password contains invalid characters.");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }

    if (status == 2) {
        SockList_AddString(&sl,
                           "failure accountpw That password is too long");
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }

    status = account_change_password(ns->account_name, old, change);
    if (status != 0) {
        const char *error;
        if (status == 1)
            error = "failure accountpw Invalid characters";
        else if (status == 2)
            error = "failure accountpw Invalid account";
        else
            error = "failure accountpw Invalid password for account";
        SockList_AddString(&sl, error);
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }

    /* If we got here, we passed all checks, and password was changed */
    send_account_players(ns);
}
