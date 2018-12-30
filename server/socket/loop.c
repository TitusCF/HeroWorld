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
 * Main client/server loops.
 *
 * \date 2003-12-02
 *
 * Mainly deals with initialization and higher level socket
 * maintenance (checking for lost connections and if data has arrived.)
 * The reading of data is handled in lowlevel.c
 */

#include <global.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#include <sockproto.h>
#endif

#ifndef WIN32 /* ---win32 exclude unix headers */
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#endif /* end win32 */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include <image.h>
#include <newserver.h>

/*****************************************************************************
 * Start of command dispatch area.
 * The commands here are protocol commands.
 ****************************************************************************/

/* Either keep this near the start or end of the file so it is
 * at least reasonablye easy to find.
 * There are really 2 commands - those which are sent/received
 * before player joins, and those happen after the player has joined.
 * As such, we have function types that might be called, so
 * we end up having 2 tables.
 */

/** Prototype for functions the client sends without player interaction. */
typedef void (*func_uint8_int_ns)(char *, int, socket_struct *);

/** Definition of a function the client sends without player interaction. */
struct client_cmd_mapping {
    const char *cmdname;        /**< Command name. */
    const func_uint8_int_ns cmdproc;  /**< Function to call. */
};

/** Prototype for functions used to handle player actions. */
typedef void (*func_uint8_int_pl)(char *, int, player *);
/** Definition of a function called in reaction to player's action. */
struct player_cmd_mapping {
    const char *cmdname;             /**< Command name. */
    const func_uint8_int_pl cmdproc; /**< Function to call. */
    const uint8 flag;                /**< If set, the player must be in the ST_PLAYING state for this command to be available. */
};

/**
 * Dispatch tables for the server.
 *
 * CmdMapping is the dispatch table for the server, used in handle_client,
 * which gets called when the client has input.  All commands called here
 * use the same parameter form (char *data, int len, int clientnum.
 * We do implicit casts, because the data that is being passed is
 * unsigned (pretty much needs to be for binary data), however, most
 * of these treat it only as strings, so it makes things easier
 * to cast it here instead of a bunch of times in the function itself.
 * flag is 1 if the player must be in the playing state to issue the
 * command, 0 if they can issue it at any time.
 */

/** Commands sent by the client reacting to player's actions. */
static const struct player_cmd_mapping player_commands[] = {
    { "examine",    examine_cmd,                       1 },
    { "apply",      apply_cmd,                         1 },
    { "move",       move_cmd,                          1 },
    { "reply",      reply_cmd,                         0 },
    { "ncom",       (func_uint8_int_pl)new_player_cmd, 1 },
    { "lookat",     look_at_cmd,                       1 },
    { "lock",       (func_uint8_int_pl)lock_item_cmd,  1 },
    { "mark",       (func_uint8_int_pl)mark_item_cmd,  1 },
    { "mapredraw",  map_redraw_cmd,                    0 },  /* Added: phil */
    { "inscribe",   inscribe_scroll_cmd,               0 },
    { NULL,         NULL,                              0 }   /* terminator */
};

/** Commands sent directly by client, when connecting or when needed. */
static const struct client_cmd_mapping client_commands[] = {
    { "addme",               add_me_cmd },
    { "askface",             send_face_cmd },             /* Added: phil */
    { "requestinfo",         request_info_cmd },
    { "setup",               set_up_cmd },
    { "version",             version_cmd },
    { "asksmooth",           ask_smooth_cmd },            /*Added: tchize (smoothing technologies)*/
    { "accountlogin",       account_login_cmd },
    { "accountnew",         account_new_cmd },
    { "accountaddplayer",   account_add_player_cmd },
    { "accountplay",        account_play_cmd },
    { "accountpw",          account_password },
    { "createplayer",       create_player_cmd },
    { NULL,                  NULL }                       /* terminator (I, II & III)*/
};

/**
 * request_info_cmd is sort of a meta command. There is some specific
 * request of information, but we call other functions to provide
 * that information.
 * @param buf buffer containing the information requested.
 * @param len length of buf, ignored.
 * @param ns socket to write data to.
 */
void request_info_cmd(char *buf, int len, socket_struct *ns) {
    char *params = NULL, *cp;
    /* No match */
    SockList sl;

    /* Set up replyinfo before we modify any of the buffers - this is used
     * if we don't find a match.
     */
    SockList_Init(&sl);
    SockList_AddString(&sl, "replyinfo ");
    SockList_AddString(&sl, buf);

    /* find the first space, make it null, and update the
     * params pointer.
     */
    for (cp = buf; *cp != '\0'; cp++)
        if (*cp == ' ') {
            *cp = '\0';
            params = cp+1;
            break;
        }
    if (!strcmp(buf, "image_info"))
        send_image_info(ns);
    else if (!strcmp(buf, "image_sums"))
        send_image_sums(ns, params);
    else if (!strcmp(buf, "skill_info"))
        send_skill_info(ns, params);
    else if (!strcmp(buf, "spell_paths"))
        send_spell_paths(ns, params);
    else if (!strcmp(buf, "exp_table"))
        send_exp_table(ns, params);
    else if (!strcmp(buf, "race_list"))
        send_race_list(ns, params);
    else if (!strcmp(buf, "race_info"))
        send_race_info(ns, params);
    else if (!strcmp(buf, "class_list"))
        send_class_list(ns, params);
    else if (!strcmp(buf, "class_info"))
        send_class_info(ns, params);
    else if (!strcmp(buf, "rules"))
        send_file(ns, "rules");
    else if (!strcmp(buf, "motd"))
        send_file(ns, "motd");
    else if (!strcmp(buf, "news"))
        send_file(ns, "news");
    else if (!strcmp(buf,"newcharinfo"))
        send_new_char_info(ns);
    else if (!strcmp(buf,"startingmap"))
        send_map_info(ns);
    else if (!strcmp(buf, "knowledge_info"))
        knowledge_send_info(ns);
    else
        Send_With_Handling(ns, &sl);
    SockList_Term(&sl);
}

/**
 * Handle client commands.
 *
 * We only get here once there is input, and only do basic connection checking.
 *
 * @param ns
 * socket sending the command. Will be set to Ns_Dead if read error.
 * @param pl
 * player associated to the socket. If NULL, only commands in client_cmd_mapping will be checked.
 */
void handle_client(socket_struct *ns, player *pl) {
    int len, i, command_count;
    unsigned char *data;

    /* Loop through this - maybe we have several complete packets here. */
    /* Command_count is used to limit the number of requests from
     * clients that have not logged in - we do not want an unauthenticated
     * connection to spew us with hundreds of requests.  As such,
     * this counter is only increased in the case of socket level commands.
     * Note that this also has the effect of throttling down face and other
     * socket commands from the client.  As such, if we have a player attached,
     * we will process more of these, as getting a fair number when entering
     * a map may not be uncommon.
     */
    command_count=0;
    while ((pl && command_count < 25) || command_count < 5) {
        /* If it is a player, and they don't have any speed left, we
         * return, and will read in the data when they do have time.
         */
        if (pl && pl->state == ST_PLAYING && pl->ob != NULL && pl->ob->speed_left < 0) {
            return;
        }

        i = SockList_ReadPacket(ns->fd, &ns->inbuf, sizeof(ns->inbuf.buf)-1);
        if (i < 0) {
#ifdef ESRV_DEBUG
            LOG(llevDebug, "handle_client: Read error on connection player %s\n", (pl ? pl->ob->name : "None"));
#endif
            /* Caller will take care of cleaning this up */
            ns->status = Ns_Dead;
            return;
        }
        /* Still dont have a full packet */
        if (i == 0)
            return;

        SockList_NullTerminate(&ns->inbuf); /* Terminate buffer - useful for string data */

        /* First, break out beginning word.  There are at least
         * a few commands that do not have any paremeters.  If
         * we get such a command, don't worry about trying
         * to break it up.
         */
        data = (unsigned char *)strchr((char *)ns->inbuf.buf+2, ' ');
        if (data) {
            *data = '\0';
            data++;
            len = ns->inbuf.len-(data-ns->inbuf.buf);
        } else
            len = 0;

        for (i = 0; client_commands[i].cmdname != NULL; i++) {
            if (strcmp((char *)ns->inbuf.buf+2, client_commands[i].cmdname) == 0) {
                client_commands[i].cmdproc((char *)data, len, ns);
                SockList_ResetRead(&ns->inbuf);
                command_count++;
                /* Evil case, and not a nice workaround, but well...
                 * If we receive eg an accountplay command, the socket is copied
                 * to the player structure, and its faces_sent is set to NULL.
                 * This leads to issues when processing the next commands in the queue,
                 * especially if related to faces...
                 * So get out of here in this case, which we detect because faces_sent is NULL.
                 */
                if (ns->faces_sent == NULL) {
                    command_count = 6;
                }
                break;
            }
        }
        if (client_commands[i].cmdname != NULL)
            /* handle another command, up to 25 */
            continue;
        /* Player must be in the playing state or the flag on the
         * the command must be zero for the user to use the command -
         * otherwise, a player cam save, be in the play_again state, and
         * the map they were on getsswapped out, yet things that try to look
         * at the map causes a crash.  If the command is valid, but
         * one they can't use, we still swallow it up.
         */
        if (pl)
            for (i = 0; player_commands[i].cmdname != NULL; i++) {
                if (strcmp((char *)ns->inbuf.buf+2, player_commands[i].cmdname) == 0) {
                    if (pl->state == ST_PLAYING || player_commands[i].flag == 0)
                        player_commands[i].cmdproc((char *)data, len, pl);
                    SockList_ResetRead(&ns->inbuf);
                    return;
                }
            }
        /* If we get here, we didn't find a valid command.  Logging
         * this might be questionable, because a broken client/malicious
         * user could certainly send a whole bunch of invalid commands.
         * limit number of these we read per tick also.
         */
        LOG(llevDebug, "Bad command from client (%s)\n", ns->inbuf.buf+2);
        SockList_ResetRead(&ns->inbuf);
        command_count++;
    }
}

/*****************************************************************************
 *
 * Low level socket looping - select calls and watchdog udp packet
 * sending.
 *
 ******************************************************************************/

#ifdef WATCHDOG
/**
 * Tell watchdog that we are still alive
 *
 * I put the function here since we should hopefully already be getting
 * all the needed include files for socket support
 */
void watchdog(void) {
    static int fd = -1;
    static struct sockaddr_in insock;

    if (fd == -1) {
        struct protoent *protoent;

        if ((protoent = getprotobyname("udp")) == NULL
        || (fd = socket(PF_INET, SOCK_DGRAM, protoent->p_proto)) == -1) {
            return;
        }
        insock.sin_family = AF_INET;
        insock.sin_port = htons((unsigned short)13325);
        insock.sin_addr.s_addr = inet_addr("127.0.0.1");
    }
    sendto(fd, (void *)&fd, 1, 0, (struct sockaddr *)&insock, sizeof(insock));
}
#endif

extern unsigned long todtick;

/**
 * Waits for new connection when there is no one connected.
 */
static void block_until_new_connection(void) {
    struct timeval Timeout;
    fd_set readfs;
    int cycles;
    int i;

    LOG(llevInfo, "Waiting for connections...\n");

    cycles = 1;
    do {
        /* Every minutes is a bit often for updates - especially if nothing is going
         * on.  This slows it down to every 6 minutes.
         */
        cycles++;
        if (cycles%2 == 0)
            tick_the_clock();

        FD_ZERO(&readfs);
        for (i = 0; i < socket_info.allocated_sockets && init_sockets[i].listen; i++)
            if (init_sockets[i].status == Ns_Add)
                FD_SET((uint32)init_sockets[i].fd, &readfs);

        /* If fastclock is set, we need to seriously slow down the updates
         * to the metaserver as well as watchdog.  Do same for flush_old_maps() -
         * that is time sensitive, so there is no good reason to call it 2000 times
         * a second.
         */
        if (settings.fastclock > 0) {
#ifdef WATCHDOG
            if (cycles%120000 == 0) {
                watchdog();
                flush_old_maps();
            }
#endif
            if (cycles == 720000) {
                metaserver_update();
                cycles = 1;
            }
            Timeout.tv_sec = 0;
            Timeout.tv_usec = 50;
        } else {
            Timeout.tv_sec = 60;
            Timeout.tv_usec = 0;
            if (cycles == 7) {
                metaserver_update();
                cycles = 1;
            }
            flush_old_maps();
        }
    } while (select(socket_info.max_filedescriptor, &readfs, NULL, NULL, &Timeout) == 0);

    reset_sleep(); /* Or the game would go too fast */
}

/**
 * Checks if file descriptor is valid.
 *
 * @param fd
 * file descriptor to check.
 * @return
 * 1 if fd is valid, 0 else.
 */
static int is_fd_valid(int fd) {
#ifndef WIN32
    return fcntl(fd, F_GETFL) != -1 || errno != EBADF;
#else
    return 1;
#endif
}

/**
 * Handle a new connection from a client.
 * @param listen_fd file descriptor the request came from.
 */
static void new_connection(int listen_fd) {
    int newsocknum = -1, j;
#ifdef HAVE_GETNAMEINFO
    struct sockaddr_storage addr;
#else
    struct sockaddr_in addr;
#endif
    socklen_t addrlen = sizeof(addr);
    char err[MAX_BUF];

#ifdef ESRV_DEBUG
    LOG(llevDebug, "do_server: New Connection\n");
#endif

    for (j = 0; j < socket_info.allocated_sockets; j++)
        if (init_sockets[j].status == Ns_Avail) {
            newsocknum = j;
            break;
        }

    if (newsocknum == -1) {
        /* If this is the case, all sockets currently in used */
        init_sockets = realloc(init_sockets, sizeof(socket_struct)*(socket_info.allocated_sockets+1));
        if (!init_sockets)
            fatal(OUT_OF_MEMORY);
        newsocknum = socket_info.allocated_sockets;
        socket_info.allocated_sockets++;
        init_sockets[newsocknum].listen = NULL;
        init_sockets[newsocknum].faces_sent_len = nrofpixmaps;
        init_sockets[newsocknum].faces_sent = calloc(1, nrofpixmaps*sizeof(*init_sockets[newsocknum].faces_sent));
        if (!init_sockets[newsocknum].faces_sent)
            fatal(OUT_OF_MEMORY);
        init_sockets[newsocknum].status = Ns_Avail;
    }

    if (newsocknum < 0) {
        LOG(llevError, "FATAL: didn't allocate a newsocket?? alloc = %d, newsocknum = %d", socket_info.allocated_sockets, newsocknum);
        fatal(SEE_LAST_ERROR);
    }

    init_sockets[newsocknum].fd = accept(listen_fd, (struct sockaddr *)&addr, &addrlen);
    if (init_sockets[newsocknum].fd == -1) {
        LOG(llevError, "accept failed: %s\n", strerror_local(errno, err, sizeof(err)));
    } else {
        char buf[MAX_BUF];
#ifndef HAVE_GETNAMEINFO
        long ip;
#endif
        socket_struct *ns;

        ns = &init_sockets[newsocknum];

#ifdef HAVE_GETNAMEINFO
        getnameinfo((struct sockaddr *) &addr, addrlen, buf, sizeof(buf), NULL, 0, NI_NUMERICHOST);
#else
        ip = ntohl(addr.sin_addr.s_addr);
        snprintf(buf, sizeof(buf), "%ld.%ld.%ld.%ld", (ip>>24)&255, (ip>>16)&255, (ip>>8)&255, ip&255);
#endif

        if (checkbanned(NULL, buf)) {
            LOG(llevInfo, "Banned host tried to connect: [%s]\n", buf);
            close(init_sockets[newsocknum].fd);
            init_sockets[newsocknum].fd = -1;
        } else {
            init_connection(ns, buf);
        }
    }
}

/**
 * This checks the sockets for input and exceptions, does the right thing.
 *
 * A bit of this code is grabbed out of socket.c
 * There are 2 lists we need to look through - init_sockets is a list
 *
 */
void do_server(void) {
    int i, pollret, active = 0;
    fd_set tmp_read, tmp_exceptions, tmp_write;
    player *pl, *next;
    char err[MAX_BUF];

#ifdef CS_LOGSTATS
    if ((time(NULL)-cst_lst.time_start) >= CS_LOGTIME)
        write_cs_stats();
#endif

    FD_ZERO(&tmp_read);
    FD_ZERO(&tmp_write);
    FD_ZERO(&tmp_exceptions);

    for (i = 0; i < socket_info.allocated_sockets; i++) {
        if (init_sockets[i].status == Ns_Add && !is_fd_valid(init_sockets[i].fd)) {
            LOG(llevError, "do_server: invalid waiting fd %d\n", i);
            init_sockets[i].status = Ns_Dead;
        }
        if (init_sockets[i].status == Ns_Dead) {
            if (init_sockets[i].listen) {
                /* try to reopen the listening socket */
                init_listening_socket(&init_sockets[i]);
            } else {
                free_newsocket(&init_sockets[i]);
                init_sockets[i].status = Ns_Avail;
            }
        } else if (init_sockets[i].status != Ns_Avail) {
            FD_SET((uint32)init_sockets[i].fd, &tmp_read);
            FD_SET((uint32)init_sockets[i].fd, &tmp_write);
            FD_SET((uint32)init_sockets[i].fd, &tmp_exceptions);
            active++;
        }
    }

    /* Go through the players.  Let the loop set the next pl value,
     * since we may remove some
     */
    for (pl = first_player; pl != NULL; ) {
        if (pl->socket.status != Ns_Dead && !is_fd_valid(pl->socket.fd)) {
            LOG(llevError, "do_server: invalid file descriptor for player %s [%s]: %d\n", (pl->ob && pl->ob->name) ? pl->ob->name : "(unnamed player?)", (pl->socket.host) ? pl->socket.host : "(unknown ip?)", pl->socket.fd);
            pl->socket.status = Ns_Dead;
        }

        if (pl->socket.status == Ns_Dead) {
            player *npl = pl->next;

            save_player(pl->ob, 0);
            leave(pl, 1);
            final_free_player(pl);
            pl = npl;
        } else {
            FD_SET((uint32)pl->socket.fd, &tmp_read);
            FD_SET((uint32)pl->socket.fd, &tmp_write);
            FD_SET((uint32)pl->socket.fd, &tmp_exceptions);
            pl = pl->next;
        }
    }

    if (active == 1 && first_player == NULL)
        block_until_new_connection();

    /* Reset timeout each time, since some OS's will change the values on
     * the return from select.
     */
    socket_info.timeout.tv_sec = 0;
    socket_info.timeout.tv_usec = 0;

    pollret = select(socket_info.max_filedescriptor, &tmp_read, &tmp_write, &tmp_exceptions, &socket_info.timeout);

    if (pollret == -1) {
        LOG(llevError, "select failed: %s\n", strerror_local(errno, err, sizeof(err)));
        return;
    }

    /* We need to do some of the processing below regardless */
    /*    if (!pollret) return;*/

    /* Check for any exceptions/input on the sockets */
    if (pollret)
        for (i = 0; i < socket_info.allocated_sockets; i++) {
            /* listen sockets can stay in status Ns_Dead */
            if (init_sockets[i].status != Ns_Add)
                continue;
            if (FD_ISSET(init_sockets[i].fd, &tmp_exceptions)) {
                free_newsocket(&init_sockets[i]);
                init_sockets[i].status = Ns_Avail;
                continue;
            }
            if (FD_ISSET(init_sockets[i].fd, &tmp_read)) {
                if (init_sockets[i].listen)
                    new_connection(init_sockets[i].fd);
                else
                    handle_client(&init_sockets[i], NULL);
            }
            if (FD_ISSET(init_sockets[i].fd, &tmp_write)) {
                init_sockets[i].can_write = 1;
            }
        }

    /* This does roughly the same thing, but for the players now */
    for (pl = first_player; pl != NULL; pl = next) {
        next = pl->next;
        if (pl->socket.status == Ns_Dead)
            continue;

        if (FD_ISSET(pl->socket.fd, &tmp_write)) {
            if (!pl->socket.can_write)  {
                pl->socket.can_write = 1;
                write_socket_buffer(&pl->socket);
            }
            /* if we get an error on the write_socket buffer, no reason to
             * continue on this socket.
             */
            if (pl->socket.status == Ns_Dead)
                continue;
        } else
            pl->socket.can_write = 0;

        if (FD_ISSET(pl->socket.fd, &tmp_exceptions)) {
            save_player(pl->ob, 0);
            leave(pl, 1);
            final_free_player(pl);
        } else {
            handle_client(&pl->socket, pl);

            /* There seems to be rare cases where next points to a removed/freed player.
             * My belief is that this player does something (shout, move, whatever)
             * that causes data to be sent to the next player on the list, but
             * that player is defunct, so the socket codes removes that player.
             * End result is that next now points at the removed player, and
             * that has garbage data so we crash.  So update the next pointer
             * while pl is still valid.  MSW 2007-04-21
             */
            next = pl->next;


            /* If the player has left the game, then the socket status
             * will be set to this be the leave function.  We don't
             * need to call leave again, as it has already been called
             * once.
             */
            if (pl->socket.status == Ns_Dead) {
                save_player(pl->ob, 0);
                leave(pl, 1);
                final_free_player(pl);
            } else {
                /* Update the players stats once per tick.  More efficient than
                 * sending them whenever they change, and probably just as useful
                 */
                esrv_update_stats(pl);
                if (pl->last_weight != -1 && pl->last_weight != WEIGHT(pl->ob)) {
                    esrv_update_item(UPD_WEIGHT, pl->ob, pl->ob);
                    if (pl->last_weight != WEIGHT(pl->ob))
                        LOG(llevError, "esrv_update_item(UPD_WEIGHT) did not set player weight: is %lu, should be %lu\n", (unsigned long)pl->last_weight, (unsigned long)WEIGHT(pl->ob));
                }
                /* draw_client_map does sanity checking that map is
                 * valid, so don't do it here.
                 */
                draw_client_map(pl->ob);
                if (pl->socket.update_look)
                    esrv_draw_look(pl->ob);
                if (pl->socket.update_inventory) {
                    if (pl->ob->container != NULL)
                        esrv_send_inventory(pl->ob, pl->ob->container);
                    pl->socket.update_inventory = 0;
                }
                if (pl->socket.tick)
                    send_tick(pl);
            }
        }
    }
}
