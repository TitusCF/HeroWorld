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
 * @file
 * Daemon mode for the server.
 *
 * Shamelessly swiped from xdm source code.
 * Appropriate copyrights kept, and hereby follow
 * ERic mehlhaff, 3/11/92
 *
 * xdm - display manager daemon
 *
 * $XConsortium: daemon.c,v 1.5 89/01/20 10:43:49 jim Exp $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

#include <global.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#endif
#include <sys/ioctl.h>
#ifdef hpux
#include <sys/ptyio.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <sys/file.h>

/**
 * Starts the server as a daemon.
 */
void become_daemon(void) {
    register int i;
    int forkresult;

    /* Print a large banner to make different runs visible in the log. */
    LOG(llevInfo, "========================\n");
    LOG(llevInfo, "Begin New Server Session\n");
    LOG(llevInfo, "========================\n");

    /*
     * fork so that the process goes into the background automatically. Also
     * has a nice side effect of having the child process get inherited by
     * init (pid 1).
     */

    if ((forkresult = fork())) {   /* if parent */
        if (forkresult < 0) {
            perror("Fork error!");
        }
        exit(0);    /* then no more work to do */
    }

    /*
     * Close standard file descriptors and get rid of controlling tty
     */

    close(0);
    close(1);
    close(2);

    /*
     * Set up the standard file descriptors.
     */
    (void)open("/dev/null", O_RDONLY);  /* root inode already in core */
    (void)dup2(0, 1);
    (void)dup2(0, 2);

    if ((i = open("/dev/tty", O_RDWR)) >= 0) {  /* did open succeed? */
#if (defined(SYSV) || defined(hpux)) && defined(TIOCTTY)
        int zero = 0;
        (void)ioctl(i, TIOCTTY, &zero);
#else

#  ifdef HAVE_TERMIOS_H
#    include <termios.h>
#  elif HAVE_SYS_TERMIOS_H
#    include <sys/termios.h>
#  else
#    ifdef HAVE_SYS_TTYCOM_H
#      include <sys/ttycom.h>
#    endif
#  endif
        (void)ioctl(i, TIOCNOTTY, (char *)0);     /* detach, BSD style */
#endif
        (void)close(i);
    }

#ifdef HAVE_SETSID
    setsid();
#else
    /* Are these really different?  */
#  if defined(SYSV) || defined(SVR4)
    setpgrp(0, 0);
#  else /* Non SYSV machines */
    setpgrp(0, getpid());
#  endif
#endif
}
