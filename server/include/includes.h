/**
 * @file
 * Standard includes.
 */

#ifndef INCLUDES_H
#define INCLUDES_H

#if defined(osf1) && !defined(__osf__)
#  define __osf__
#endif

#if defined(sgi) && !defined(__sgi__)
#  define __sgi__
#endif

#ifdef sun
#  ifndef __sun__
#    define __sun__
#  endif
#endif

#if defined(ultrix) && !defined(__ultrix__)
#  define __ultrix__
#endif

/* Include this first, because it lets us know what we are missing */
#ifdef WIN32 /* ---win32 exclude this, config comes from VC ide */
#include "win32.h"
#else
#include <autoconf.h>
/* socklen_t is defined in this file on some systems, and that type is
 * used in newserver.h, which is used in all other files
 */
#ifdef __WIN32__ /* ---win32 exclude this, config comes from VC ide */
#   include <winsock2.h>
#else
#   include <sys/socket.h>
#endif
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <setjmp.h>
#include <stdlib.h>

#ifdef __NetBSD__
#include <math.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#if defined(HAVE_TIME_H) && defined(TIME_WITH_SYS_TIME)
#include <time.h>
#endif

/* stddef is for offsetof */
#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif

#include <sys/types.h>

#include <sys/stat.h>

#include "config.h"
#include "define.h"
#include "logger.h"
#include "shared/newclient.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#if defined(vax) || defined(ibm032)
size_t strftime(char *, size_t, const char *, const struct tm *);
time_t mktime(struct tm *);
#endif

#endif /* INCLUDES_H */
