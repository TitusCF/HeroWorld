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
 * Low-level socket-related functions.
 *
 * \date 2003-12-02
 *
 * Contains some base functions that both the client and server
 * can use.  As such, depending what we are being compiled for will
 * determine what we can include.  the client is designed have
 * CFCLIENT defined as part of its compile flags.
 */

#include <assert.h>
#include <stdarg.h>
#include <global.h>
#include <shared/newclient.h>
#include <sproto.h>
#include <errno.h>

/***********************************************************************
 *
 * SockList functions/utilities
 *
 **********************************************************************/

/**
 * Initializes the SockList instance. Must be called before other socklist
 * functions are called.
 * @param sl the SockList instance to initialize
 */
void SockList_Init(SockList *sl) {
    SockList_Reset(sl);
}

/**
 * Frees all resources allocated by a SockList instance. Must be called when
 * the instance is not needed anymore. Afterwards no socklist functions except
 * SockList_Init may be called.
 * @param sl the SockList instance to free
 */
void SockList_Term(SockList *sl) {
}

/**
 * Resets the length of the stored data for writing. Does not free or
 * re-allocate resources.
 * @param sl the SockList instance to reset
 */
void SockList_Reset(SockList *sl) {
    sl->len = 2;
}

/**
 * Resets the length of the stored data for reading. Does not free or
 * re-allocate resources.
 * @param sl the SockList instance to reset
 */
void SockList_ResetRead(SockList *sl) {
    sl->len = 0;
}

/**
 * Checks that at least a given number of bytes is available in a SockList
 * instance. Returns normal if the space is available. Otherwise calls
 * fatal(OUT_OF_MEMORY);
 *
 * @param sl the SockList instance to check
 * @param size the number of bytes to ensure
 */
static void SockList_Ensure(const SockList *sl, size_t size) {
    if (sl->len+size > sizeof(sl->buf)) {
        fatal(OUT_OF_MEMORY);
    }
}

/**
 * Adds an 8 bit value.
 * @param sl the SockList instance to add to
 * @param data the value to add
 */
void SockList_AddChar(SockList *sl, char data) {
    SockList_Ensure(sl, 1);
    sl->buf[sl->len++] = data;
}

/**
 * Adds a 16 bit value.
 * @param sl the SockList instance to add to
 * @param data the value to add
 */
void SockList_AddShort(SockList *sl, uint16 data) {
    SockList_Ensure(sl, 2);
    sl->buf[sl->len++] = (data>>8)&0xff;
    sl->buf[sl->len++] = data&0xff;
}

/**
 * Adds a 32 bit value.
 * @param sl the SockList instance to add to
 * @param data the value to add
 */
void SockList_AddInt(SockList *sl, uint32 data) {
    SockList_Ensure(sl, 4);
    sl->buf[sl->len++] = (data>>24)&0xff;
    sl->buf[sl->len++] = (data>>16)&0xff;
    sl->buf[sl->len++] = (data>>8)&0xff;
    sl->buf[sl->len++] = data&0xff;
}

/**
 * Adds a 64 bit value.
 * @param sl the SockList instance to add to
 * @param data the value to add
 */
void SockList_AddInt64(SockList *sl, uint64 data) {
    SockList_Ensure(sl, 8);
    sl->buf[sl->len++] = (char)((data>>56)&0xff);
    sl->buf[sl->len++] = (char)((data>>48)&0xff);
    sl->buf[sl->len++] = (char)((data>>40)&0xff);
    sl->buf[sl->len++] = (char)((data>>32)&0xff);
    sl->buf[sl->len++] = (char)((data>>24)&0xff);
    sl->buf[sl->len++] = (char)((data>>16)&0xff);
    sl->buf[sl->len++] = (char)((data>>8)&0xff);
    sl->buf[sl->len++] = (char)(data&0xff);
}

/**
 * Adds a string without length.
 * @param sl the SockList instance to add to
 * @param data the value to add
 */
void SockList_AddString(SockList *sl, const char *data) {
    SockList_AddData(sl, data, strlen(data));
}

/**
 * Adds a data block.
 * @param sl the SockList instance to add to
 * @param data the value to add
 * @param len the length in byte
 */
void SockList_AddData(SockList *sl, const void *data, size_t len) {
    SockList_Ensure(sl, len);
    memcpy(sl->buf+sl->len, data, len);
    sl->len += len;
}

/**
 * Adds a data block prepended with an 8 bit length field.
 * @param sl the SockList instance to add to
 * @param data the value to add
 * @param len the length in byte; must not exceed 255
 */
void SockList_AddLen8Data(SockList *sl, const void *data, size_t len) {
    assert(len <= 255);
    SockList_AddChar(sl, len);
    SockList_AddData(sl, data, len);
}

/**
 * Adds a data block prepended with an 16 bit length field.
 * @param sl the SockList instance to add to
 * @param data the value to add
 * @param len the length in byte; must not exceed 65535
 */
void SockList_AddLen16Data(SockList *sl, const void *data, size_t len) {
    assert(len <= 65535);
    SockList_AddShort(sl, len);
    SockList_AddData(sl, data, len);
}

/**
 * Adds a printf like formatted string.
 * @param sl the SockList instance to add to
 * @param format the format specifier
 */
void SockList_AddPrintf(SockList *sl, const char *format, ...) {
    size_t size;
    int n;
    va_list arg;

    size = sizeof(sl->buf)-sl->len;

    va_start(arg, format);
    n = vsnprintf((char *)sl->buf+sl->len, size, format, arg);
    va_end(arg);

    if (n <= -1 || (size_t)n >= size) {
        fatal(OUT_OF_MEMORY);
    }
    sl->len += (size_t)n;
}

/**
 * Deallocates string buffer instance and appends its contents. The passed
 * StringBuffer must not be accessed afterwards.
 * @param sl the SockList instance to add to
 * @param sb the StringBuffer to deallocate
 */
void SockList_AddStringBuffer(SockList *sl, StringBuffer *sb) {
    char *p;

    p = stringbuffer_finish(sb);
    SockList_AddString(sl, p);
    free(p);
}

/**
 * Adds a NUL byte without changing the length.
 * @param sl the SockList instance to add to
 */
void SockList_NullTerminate(SockList *sl) {
    SockList_Ensure(sl, 1);
    sl->buf[sl->len] = '\0';
}

/**
 * Returns the available bytes in a SockList instance.
 * @return the available bytes
 */
size_t SockList_Avail(const SockList *sl) {
    return sizeof(sl->buf)-sl->len;
}

/**
 * Basically does the reverse of SockList_AddInt, but on
 * strings instead.  Same for the GetShort, but for 16 bits.
 */
int GetInt_String(const unsigned char *data) {
    return ((data[0]<<24)+(data[1]<<16)+(data[2]<<8)+data[3]);
}

short GetShort_String(const unsigned char *data) {
    return ((data[0]<<8)+data[1]);
}

/******************************************************************************
 *
 * Start of read routines.
 *
 ******************************************************************************/

/**
 * This reads from fd and puts the data in sl.  We return true if we think
 * we have a full packet, 0 if we have a partial packet, or -1 if an error
 * occurred.  The only processing we do is remove the initial size value.  len
 * (As passed) is the size of the buffer allocated in the socklist.  We make
 * the assumption the buffer is at least 2 bytes long.
 */
int SockList_ReadPacket(int fd, SockList *sl, int len) {
    int stat, toread;
    char err[MAX_BUF];

    /* We already have a partial packet */
    if (sl->len < 2) {
#ifdef WIN32 /* ***WIN32 SockList_ReadPacket: change read() to recv() */

        stat = recv(fd, sl->buf+sl->len, 2-sl->len, 0);

#else
        do {
            stat = read(fd, sl->buf+sl->len, 2-sl->len);
        } while ((stat == -1) && (errno == EINTR));
#endif
        if (stat < 0) {
            /* In non blocking mode, EAGAIN is set when there is no
             * data available.
             */
#ifdef WIN32 /* ***WIN32 SockList_ReadPacket: error handling for win32 */
            if ((stat == -1) && WSAGetLastError() != WSAEWOULDBLOCK) {
                if (WSAGetLastError() == WSAECONNRESET)
                    LOG(llevDebug, "Connection closed by client\n");
                else {
                    LOG(llevDebug, "ReadPacket got error %d, returning -1\n", WSAGetLastError());
                }
                return -1; /* kick this user! */
            }
#else
            if (errno == ECONNRESET) {
                LOG(llevDebug, "ReadPacket got error %s, returning -1\n", strerror_local(errno, err, sizeof(err)));
                return -1;
            }
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                LOG(llevDebug, "ReadPacket got error %s, returning 0\n", strerror_local(errno, err, sizeof(err)));
            }
#endif
            return 0; /*Error */
        }
        if (stat == 0)
            return -1;
        sl->len += stat;
#ifdef CS_LOGSTATS
        cst_tot.ibytes += stat;
        cst_lst.ibytes += stat;
#endif
        if (stat < 2)
            return 0; /* Still don't have a full packet */
    }
    /* Figure out how much more data we need to read.  Add 2 from the
     * end of this - size header information is not included.
     */
    toread = 2+(sl->buf[0]<<8)+sl->buf[1]-sl->len;
    if ((toread+(int)sl->len) >= len) {
        LOG(llevError, "SockList_ReadPacket: Want to read more bytes than will fit in buffer (%lu>=%lu).\n", (unsigned long)toread+sl->len, (unsigned long)len);
        /* Quick hack in case for 'oldsocketmode' input.  If we are
         * closing the socket anyways, then reading this extra 100 bytes
         * shouldn't hurt.
         */
#ifdef WIN32 /* ***win32 SockList_ReadPacket: change read() to recv() */
        recv(fd, sl->buf+2, 100, 0);
#else
        read(fd, sl->buf+2, 100);
#endif /* end win32 */

        /* return error so the socket is closed */
        return -1;
    }
    do {
#ifdef WIN32 /* ***win32 SockList_ReadPacket: change read() to recv() */
        stat = recv(fd, sl->buf+sl->len, toread, 0);
#else
        do {
            stat = read(fd, sl->buf+sl->len, toread);
        } while ((stat < 0) && (errno == EINTR));
#endif
        if (stat < 0) {
#ifdef WIN32 /* ***win32 SockList_ReadPacket: change error handling for win32 */
            if ((stat == -1) && WSAGetLastError() != WSAEWOULDBLOCK) {
                if (WSAGetLastError() == WSAECONNRESET)
                    LOG(llevDebug, "Connection closed by client\n");
                else {
                    LOG(llevDebug, "ReadPacket got error %d, returning -1\n", WSAGetLastError());
                }
                return -1; /* kick this user! */
            }
#else
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                LOG(llevDebug, "ReadPacket got error %s, returning 0\n", strerror_local(errno, err, sizeof(err)));
            }
#endif
            return 0; /*Error */
        }
        if (stat == 0)
            return -1;
        sl->len += stat;
#ifdef CS_LOGSTATS
        cst_tot.ibytes += stat;
        cst_lst.ibytes += stat;
#endif
        toread -= stat;
        if (toread == 0)
            return 1;
        if (toread < 0) {
            LOG(llevError, "SockList_ReadPacket: Read more bytes than desired.\n");
            return 1;
        }
    } while (toread > 0);
    return 0;
}

/*******************************************************************************
 *
 * Start of write related routines.
 *
 ******************************************************************************/

/**
 * Adds data to a socket buffer for whatever reason.
 *
 * ns is the socket we are adding the data to, buf is the start of the
 * data, and len is the number of bytes to add.
 */
static void add_to_buffer(socket_struct *ns, const unsigned char *buf, int len) {
    int avail, end;

    if ((len+ns->outputbuffer.len) > SOCKETBUFSIZE) {
        LOG(llevDebug, "Socket on fd %d has overrun internal buffer - marking as dead\n", ns->fd);
        ns->status = Ns_Dead;
        return;
    }

    /* data + end is where we start putting the new data.  The last byte
     * currently in use is actually data + end -1
     */

    end = ns->outputbuffer.start+ns->outputbuffer.len;
    /* The buffer is already in a wrapped state, so adjust end */
    if (end >= SOCKETBUFSIZE)
        end -= SOCKETBUFSIZE;
    avail = SOCKETBUFSIZE-end;

    /* We can all fit it behind the current data without wrapping */
    if (avail >= len) {
        memcpy(ns->outputbuffer.data+end, buf, len);
    } else {
        memcpy(ns->outputbuffer.data+end, buf, avail);
        memcpy(ns->outputbuffer.data, buf+avail, len-avail);
    }
    ns->outputbuffer.len += len;
}

/**
 * Writes data to socket.
 *
 * When the socket is clear to write, and we have backlogged data, this
 * is called to write it out.
 */
void write_socket_buffer(socket_struct *ns) {
    int amt, max;

    if (ns->outputbuffer.len == 0) {
        LOG(llevDebug, "write_socket_buffer called when there is no data, fd=%d\n", ns->fd);
        return;
    }

    do {
        max = SOCKETBUFSIZE-ns->outputbuffer.start;
        if (ns->outputbuffer.len < max)
            max = ns->outputbuffer.len;

#ifdef WIN32 /* ***win32 write_socket_buffer: change write() to send() */
        amt = send(ns->fd, ns->outputbuffer.data+ns->outputbuffer.start, max, 0);
#else
        do {
            amt = write(ns->fd, ns->outputbuffer.data+ns->outputbuffer.start, max);
        } while ((amt < 0) && (errno == EINTR));
#endif

        if (amt < 0) { /* We got an error */
#ifdef WIN32 /* ***win32 write_socket_buffer: change error handling */
            if (amt == -1 && WSAGetLastError() != WSAEWOULDBLOCK) {
                LOG(llevError, "New socket write failed (wsb) (%d).\n", WSAGetLastError());
#else
            if (errno != EWOULDBLOCK) {
                char err[MAX_BUF];

                LOG(llevError, "New socket write failed (wsb) (%d: %s).\n", errno, strerror_local(errno, err, sizeof(err)));
#endif
                ns->status = Ns_Dead;
                return;
            } else { /* EWOULDBLOCK */
                /* can't write it, so store it away. */
                ns->can_write = 0;
                return;
            }
        }
        ns->outputbuffer.start += amt;
        /* wrap back to start of buffer */
        if (ns->outputbuffer.start == SOCKETBUFSIZE)
            ns->outputbuffer.start = 0;
        ns->outputbuffer.len -= amt;
#ifdef CS_LOGSTATS
        cst_tot.obytes += amt;
        cst_lst.obytes += amt;
#endif
    } while (ns->outputbuffer.len > 0);
}

/**
 * This writes data to the socket. - It is very low level -
 * all we try and do is write out the data to the socket
 * provided (ns).  buf is the data to write, len is the number
 * of bytes to write.  IT doesn't return anything - rather, it
 * updates the ns structure if we get an  error.
 */
static void Write_To_Socket(socket_struct *ns, const unsigned char *buf, int len) {
    int amt = 0;
    const unsigned char *pos = buf;
    char err[MAX_BUF];

    if (ns->status == Ns_Dead || !buf) {
        LOG(llevDebug, "Write_To_Socket called with dead socket\n");
        return;
    }

#ifndef __GNU__ /* This caused problems on Hurd */
    if (!ns->can_write) {
        add_to_buffer(ns, buf, len);
        return;
    }
#endif
    /* If we manage to write more than we wanted, take it as a bonus */
    while (len > 0) {
#ifdef WIN32 /* ***win32 Write_To_Socket: change write() to send() */
        amt = send(ns->fd, pos, len, 0);
#else
        do {
            amt = write(ns->fd, pos, len);
        } while ((amt < 0) && (errno == EINTR));
#endif

        if (amt < 0) { /* We got an error */
#ifdef WIN32 /* ***win32 Write_To_Socket: change error handling */
            if (amt == -1 && WSAGetLastError() != WSAEWOULDBLOCK) {
                LOG(llevError, "New socket write failed WTS (%d).\n", WSAGetLastError());
#else
            if (errno != EWOULDBLOCK) {
                LOG(llevError, "New socket write failed WTS (%d: %s).\n", errno, strerror_local(errno, err, sizeof(err)));
#endif
                ns->status = Ns_Dead;
                return;
            } else { /* EWOULDBLOCK */
                /* can't write it, so store it away. */
                add_to_buffer(ns, pos, len);
                ns->can_write = 0;
                return;
            }
        /* amt gets set to 0 above in blocking code, so we do this as
         * an else if to make sure we don't reprocess it.
         */
        } else if (amt == 0) {
            LOG(llevError, "Write_To_Socket: No data written out.\n");
        }
        len -= amt;
        pos += amt;
#ifdef CS_LOGSTATS
        cst_tot.obytes += amt;
        cst_lst.obytes += amt;
#endif
    }
}

/**
 * Calls Write_To_Socket to send data to the client.
 *
 * The only difference in this function is that we take a SockList
 *, and we prepend the length information.
 */
void Send_With_Handling(socket_struct *ns, SockList *sl) {
    if (ns->status == Ns_Dead || sl == NULL)
        return;

    sl->buf[0] = ((sl->len-2)>>8)&0xFF;
    sl->buf[1] = (sl->len-2)&0xFF;
    Write_To_Socket(ns, sl->buf, sl->len);
}

/******************************************************************************
 *
 * statistics logging functions.
 *
 ******************************************************************************/

#ifdef CS_LOGSTATS
/* cst_tot is for the life of the server, cst_last is for the last series of
 * stats
 */
CS_Stats cst_tot, cst_lst;

/**
 * Writes out the gathered stats.  We clear cst_lst.
 */
void write_cs_stats(void) {
    time_t now = time(NULL);

    /* If no connections recently, don't bother to log anything */
    if (cst_lst.ibytes == 0 && cst_lst.obytes == 0)
        return;

    /* CSSTAT is put in so scripts can easily find the line */
    LOG(llevInfo, "CSSTAT: %.16s tot %d %d %d %ld inc %d %d %d %ld\n",
        ctime(&now), cst_tot.ibytes, cst_tot.obytes, cst_tot.max_conn,
        (long)(now-cst_tot.time_start), cst_lst.ibytes, cst_lst.obytes,
        cst_lst.max_conn, (long)(now-cst_lst.time_start));
    cst_lst.ibytes = 0;
    cst_lst.obytes = 0;
    cst_lst.max_conn = socket_info.allocated_sockets;
    cst_lst.time_start = now;
}
#endif

