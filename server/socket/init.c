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
 * Socket general functions
 *
 * \date 2003-12-02
 *
 * Mainly deals with initialization and higher level socket
 * maintenance (checking for lost connections and if data has arrived.)
 * The reading of data is handled in ericserver.c
 */

#include <assert.h>
#include <global.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#endif
#ifndef WIN32 /* ---win32 exclude include files */
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#endif /* win32 */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#include <newserver.h>
#include <image.h>

/** Socket information. */
Socket_Info socket_info;

/**
 * Established connections for clients not yet playing.  See the page on
 * @ref page_connection "the login process" for a description of its use.
 * Socket at index 0 is the socket listening for connections, and must not
 * be freed.
 * If this socket becomes invalid, then the server will try to reopen it.
 */
socket_struct *init_sockets;

/**
 * Initializes a connection. Really, it just sets up the data structure,
 * socket setup is handled elsewhere.  We do send a version to the
 * client.
 */
void init_connection(socket_struct *ns, const char *from_ip) {
    SockList sl;
    int bufsize = 65535; /*Supposed absolute upper limit */
    int oldbufsize;
    socklen_t buflen = sizeof(int);

#ifdef WIN32 /* ***WIN32 SOCKET: init win32 non blocking socket */
    int temp = 1;

    if (ioctlsocket(ns->fd, FIONBIO , &temp) == -1)
        LOG(llevError, "init_connection:  Error on ioctlsocket.\n");
#else
    if (fcntl(ns->fd, F_SETFL, O_NONBLOCK) == -1) {
        LOG(llevError, "init_connection:  Error on fcntl.\n");
    }
#endif /* end win32 */

    if (getsockopt(ns->fd, SOL_SOCKET, SO_SNDBUF, (char *)&oldbufsize, &buflen) == -1)
        oldbufsize = 0;
    if (oldbufsize < bufsize) {
#ifdef ESRV_DEBUG
        LOG(llevDebug, "Default buffer size was %d bytes, will reset it to %d\n", oldbufsize, bufsize);
#endif
        if (setsockopt(ns->fd, SOL_SOCKET, SO_SNDBUF, (char *)&bufsize, sizeof(bufsize))) {
            LOG(llevError, "init_connection: setsockopt unable to set output buf size to %d\n", bufsize);
        }
    }
    buflen = sizeof(oldbufsize);
    getsockopt(ns->fd, SOL_SOCKET, SO_SNDBUF, (char *)&oldbufsize, &buflen);
#ifdef ESRV_DEBUG
    LOG(llevDebug, "Socket buffer size now %d bytes\n", oldbufsize);
#endif

    ns->faceset = 0;
    ns->facecache = 0;
    ns->sound = 0;
    ns->sounds_this_tick = 0;
    ns->monitor_spells = 0;
    ns->darkness = 1;
    ns->status = Ns_Add;
    ns->mapx = 11;
    ns->mapy = 11;
    ns->look_position = 0;
    ns->container_position = 0;
    ns->update_look = 0;
    ns->update_inventory = 0;
    ns->tick = 0;
    ns->is_bot = 0;
    ns->num_look_objects = DEFAULT_NUM_LOOK_OBJECTS;
    ns->want_pickup = 0;
    ns->extended_stats = 0;
    ns->account_name = NULL;
    ns->account_chars = NULL;
    ns->login_method = 0;
    ns->notifications = 0;

    /* we should really do some checking here - if total clients overflows
     * we need to do something more intelligent, because client id's will start
     * duplicating (not likely in normal cases, but malicous attacks that
     * just open and close connections could get this total up.
     */
    SockList_Init(&ns->inbuf);
    SockList_ResetRead(&ns->inbuf);
    /* Basic initialization. Needed because we do a check in
     * handle_client for oldsocketmode without checking the
     * length of data.
     */
    memset(ns->inbuf.buf, 0, sizeof(ns->inbuf.buf));
    memset(&ns->lastmap, 0, sizeof(struct Map));
    if (!ns->faces_sent)
        ns->faces_sent = calloc(sizeof(*ns->faces_sent), nrofpixmaps);
    ns->faces_sent_len = nrofpixmaps;

    memset(&ns->anims_sent, 0, sizeof(ns->anims_sent));
    memset(&ns->stats, 0, sizeof(struct statsinfo));
    ns->map_scroll_x = 0;
    ns->map_scroll_y = 0;
    /* Do this so we don't send a face command for the client for
     * this face.  Face 0 is sent to the client to say clear
     * face information.
     */
    ns->faces_sent[0] = NS_FACESENT_FACE;

    ns->outputbuffer.start = 0;
    ns->outputbuffer.len = 0;
    ns->can_write = 1;
    ns->password_fails = 0;

    ns->host = strdup_local(from_ip);
    SockList_Init(&sl);
    SockList_AddPrintf(&sl, "version %d %d %s\n", VERSION_CS, VERSION_SC, VERSION_INFO);
    Send_With_Handling(ns, &sl);
    SockList_Term(&sl);
#ifdef CS_LOGSTATS
    if (socket_info.allocated_sockets > cst_tot.max_conn)
        cst_tot.max_conn = socket_info.allocated_sockets;
    if (socket_info.allocated_sockets > cst_lst.max_conn)
        cst_lst.max_conn = socket_info.allocated_sockets;
#endif
}

/**
 * This opens *ns for listening to connections.
 * The structure must be allocated already and
 * ns->listen must be initialized.
 * No other variable is changed.
 */
void init_listening_socket(socket_struct *ns) {
    struct linger linger_opt;
    char buf1[MAX_BUF], buf2[MAX_BUF];
    char err[MAX_BUF];

    if (ns == NULL || ns->listen == NULL) {
        LOG(llevError, "init_listening_socket: missing listen info in socket_struct?!\n");
        fatal(SEE_LAST_ERROR);
    }

    ns->status = Ns_Dead;

    ns->fd = socket(ns->listen->family, ns->listen->socktype, ns->listen->protocol);
    if (ns->fd == -1) {
        LOG(llevError, "Cannot create socket: %s\n", strerror_local(errno, err, sizeof(err)));
        return;
    }

    linger_opt.l_onoff = 0;
    linger_opt.l_linger = 0;
    if (setsockopt(ns->fd, SOL_SOCKET, SO_LINGER, (char *)&linger_opt, sizeof(struct linger))) {
        LOG(llevError, "Cannot setsockopt(SO_LINGER): %s\n", strerror_local(errno, err, sizeof(err)));
    }
/* Would be nice to have an autoconf check for this.  It appears that
 * these functions are both using the same calling syntax, just one
 * of them needs extra valus passed.
 */
#if defined(__osf__) || defined(hpux) || defined(sgi) || defined(NeXT) || \
    defined(__sun__) || defined(__linux__) || defined(SVR4) ||          \
    defined(__FreeBSD__) || defined(__OpenBSD__) ||                     \
    defined(WIN32) /* ---win32 add this here */  ||                     \
    defined(__GNU__) /* HURD */
    {
#ifdef WIN32
        char tmp = 1;
#else
        int tmp = 1;
#endif

        if (setsockopt(ns->fd, SOL_SOCKET, SO_REUSEADDR, &tmp, sizeof(tmp))) {
            LOG(llevError, "Cannot setsockopt(SO_REUSEADDR): %s\n", strerror_local(errno, err, sizeof(err)));
        }
#ifdef HAVE_GETADDRINFO
        if ((ns->listen->family == AF_INET6) && setsockopt(ns->fd, IPPROTO_IPV6, IPV6_V6ONLY, &tmp, sizeof(tmp))) {
            LOG(llevError, "Cannot setsockopt(IPV6_V6ONLY): %s\n", strerror_local(errno, err, sizeof(err)));
        }
#endif
    }
#else
    if (setsockopt(ns->fd, SOL_SOCKET, SO_REUSEADDR, (char *)NULL, 0)) {
        LOG(llevError, "Cannot setsockopt(SO_REUSEADDR): %s\n", strerror_local(errno, err, sizeof(err)));
    }
#endif

    if (bind(ns->fd, ns->listen->addr, ns->listen->addrlen) == (-1)) {
#ifdef HAVE_GETNAMEINFO
        getnameinfo(ns->listen->addr, ns->listen->addrlen, buf1, sizeof(buf1), buf2, sizeof(buf2), NI_NUMERICHOST|NI_NUMERICSERV);
#else
        short port;
        long ip;

        ip = ntohl(((struct sockaddr_in *)ns->listen->addr)->sin_addr.s_addr);
        port = ntohs(((struct sockaddr_in *)ns->listen->addr)->sin_port);
        snprintf(buf1, sizeof(buf1), "%ld.%ld.%ld.%ld", (ip>>24)&255, (ip>>16)&255, (ip>>8)&255, ip&255);
        snprintf(buf2, sizeof(buf2), "%d", port&65535);
#endif
        LOG(llevError, "Cannot bind socket to [%s]:%s: %s\n", buf1, buf2, strerror_local(errno, err, sizeof(err)));
#ifdef WIN32 /* ***win32: close() -> closesocket() */
        shutdown(ns->fd, SD_BOTH);
        closesocket(ns->fd);
#else
        close(ns->fd);
#endif /* win32 */
        ns->fd = -1;
        return;
    }
    if (listen(ns->fd, 5) == (-1))  {
        LOG(llevError, "Cannot listen on socket: %s\n", strerror_local(errno, err, sizeof(err)));
#ifdef WIN32 /* ***win32: close() -> closesocket() */
        shutdown(ns->fd, SD_BOTH);
        closesocket(ns->fd);
#else
        close(ns->fd);
#endif /* win32 */
        ns->fd = -1;
        return;
    }
    ns->status = Ns_Add;
}

/** This sets up the socket and reads all the image information into memory.
 * @todo fix socket_info.max_filedescriptor hack. */
void init_server(void) {
    int i, e, listen_socket_count;
#ifdef HAVE_GETADDRINFO
    struct addrinfo *ai, *ai_p;
    struct addrinfo hints;
    char buf[MAX_BUF];
#else
    struct sockaddr_in *insock;
    struct protoent *protox;
#endif

#ifdef WIN32 /* ***win32  -  we init a windows socket */
    WSADATA w;

    socket_info.max_filedescriptor = 1; /* used in select, ignored in winsockets */
    WSAStartup(0x0101, &w);     /* this setup all socket stuff */
    /* ill include no error tests here, winsocket 1.1 should always work */
    /* except some old win95 versions without tcp/ip stack */
#else /* non windows */

#ifdef HAVE_SYSCONF
    socket_info.max_filedescriptor = sysconf(_SC_OPEN_MAX);
#else
#  ifdef HAVE_GETDTABLESIZE
    socket_info.max_filedescriptor = getdtablesize();
#  else
    "Unable to find usable function to get max filedescriptors";
#  endif
#endif
#endif /* win32 */

    /*
     * There is a bug on FreeBSD 9 in that the value we get here is over 1024,
     * which is the limit FD_ZERO and friends will use.
     * So later on file descriptors are not correctly cleared, and select() fails.
     * Therefore here's a hack to prevent that, but the real solution would be
     * to figure why this happens and how to fix it.
     * Nicolas W, 2012 feb 07.
     */
    if (socket_info.max_filedescriptor > 1024) {
        LOG(llevDebug, "warning, socket_info.max_filedescriptor is %d, setting to 1024.\n", socket_info.max_filedescriptor);
        socket_info.max_filedescriptor = 1024;
    }
    /* end of hack */

    socket_info.timeout.tv_sec = 0;
    socket_info.timeout.tv_usec = 0;

#ifdef CS_LOGSTATS
    memset(&cst_tot, 0, sizeof(CS_Stats));
    memset(&cst_lst, 0, sizeof(CS_Stats));
    cst_tot.time_start = time(NULL);
    cst_lst.time_start = time(NULL);
#endif

#ifdef HAVE_GETADDRINFO
    memset(&hints, '\0', sizeof(hints));
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    hints.ai_socktype = SOCK_STREAM;
    snprintf(buf, sizeof(buf), "%d", settings.csport);
    e = getaddrinfo(NULL, buf, &hints, &ai);
    if (e != 0) {
        LOG(llevError, "init_server: getaddrinfo: %s\n", gai_strerror(e));
        fatal(SEE_LAST_ERROR);
    }

    listen_socket_count = 0;
    for (ai_p = ai; ai_p != NULL; ai_p = ai_p->ai_next) {
        listen_socket_count++;
    }

    assert(listen_socket_count > 0);
#else
    listen_socket_count = 1;
#endif

    LOG(llevDebug, "Initialize new client/server data\n");
    init_sockets = malloc(sizeof(socket_struct) * listen_socket_count);
    socket_info.allocated_sockets = listen_socket_count;
    for (i = 0; i < listen_socket_count; i++) {
        init_sockets[i].listen = calloc(sizeof(struct listen_info), 1);
        init_sockets[i].faces_sent = NULL; /* unused */
        init_sockets[i].account_name = NULL; /* Must be set to avoid undef behaviour elsewhere. */
    }

#ifdef HAVE_GETADDRINFO
    for (i = 0, ai_p = ai; i < listen_socket_count && ai_p != NULL; i++, ai_p = ai_p->ai_next) {
        init_sockets[i].listen->family   = ai_p->ai_family;
        init_sockets[i].listen->socktype = ai_p->ai_socktype;
        init_sockets[i].listen->protocol = ai_p->ai_protocol;
        init_sockets[i].listen->addrlen  = ai_p->ai_addrlen;
        init_sockets[i].listen->addr     = malloc(ai_p->ai_addrlen);
        memcpy(init_sockets[i].listen->addr, ai_p->ai_addr, ai_p->ai_addrlen);
    }
    freeaddrinfo(ai);
#else
    protox = getprotobyname("tcp");
    if (protox == NULL) {
        LOG(llevError, "init_server: Error getting protox\n");
        fatal(SEE_LAST_ERROR);
    }
    init_sockets[0].listen->family   = PF_INET;
    init_sockets[0].listen->socktype = SOCK_STREAM;
    init_sockets[0].listen->protocol = protox->p_proto;
    insock = calloc(sizeof(struct sockaddr_in), 1);
    insock->sin_family = AF_INET;
    insock->sin_port = htons(settings.csport);
    insock->sin_addr.s_addr = htonl(INADDR_ANY);
    init_sockets[0].listen->addr = (struct sockaddr *) insock;
    init_sockets[0].listen->addrlen = sizeof(struct sockaddr_in);
#endif

    e = 1;
    for (i = 0; i < listen_socket_count; i++) {
        init_listening_socket(&init_sockets[i]);
        if (init_sockets[i].fd != -1)
            e = 0;
    }
    if (e != 0) {
        LOG(llevError, "init_server: can't open any listening socket\n");
        fatal(SEE_LAST_ERROR);
    }

    read_client_images();
}

/*******************************************************************************
 *
 * Start of functions dealing with freeing of the data.
 *
 ******************************************************************************/

/** Free's all the memory that ericserver allocates. */
void free_all_newserver(void) {
    int i;
    LOG(llevDebug, "Freeing all new client/server information.\n");
    free_socket_images();
    for (i = 0; i < socket_info.allocated_sockets && init_sockets[i].listen; i++) {
        free(init_sockets[i].listen->addr);
        free(init_sockets[i].listen);
    }
    free(init_sockets);
}

/**
 * Frees a socket.
 * Basically, all we need to do here is free all data structures that
 * might be associated with the socket.  It is up to the caller to
 * update the list
 */

void free_newsocket(socket_struct *ns) {
#ifdef WIN32 /* ***win32: closesocket in windows style */
    shutdown(ns->fd, SD_BOTH);
    if (closesocket(ns->fd)) {
#else
    if (close(ns->fd)) {
#endif /* win32 */

#ifdef ESRV_DEBUG
        LOG(llevDebug, "Error closing socket %d\n", ns->fd);
#endif
    }
    ns->fd = -1;
    if (ns->stats.range)
        FREE_AND_CLEAR(ns->stats.range);
    if (ns->stats.title)
        FREE_AND_CLEAR(ns->stats.title);
    if (ns->host)
        FREE_AND_CLEAR(ns->host);
    if (ns->account_name) {
        account_char_save(ns->account_name, ns->account_chars);
        FREE_AND_CLEAR(ns->account_name);
    }
    if (ns->account_chars) {
        account_char_free(ns->account_chars);
        ns->account_chars = NULL;
    }
    SockList_Term(&ns->inbuf);
}

/** Sends the 'goodbye' command to the player, and closes connection. */
void final_free_player(player *pl) {
    SockList sl;

    SockList_Init(&sl);
    SockList_AddString(&sl, "goodbye");
    Send_With_Handling(&pl->socket, &sl);
    SockList_Term(&sl);
    free_newsocket(&pl->socket);
    free_player(pl);
}
