#ifndef WIN32_H
#define WIN32_H

/**
 * @file
 * Structures and types used to implement opendir/readdir/closedir
 * on Windows 95/NT and set the loe level defines.
 *
 * Also some Windows-specific includes and tweaks.
 */

#if !defined(AFX_STDAFX_H__31666CA1_2474_11D5_AE6C_F07569C10000__INCLUDED_)
#define AFX_STDAFX_H__31666CA1_2474_11D5_AE6C_F07569C10000__INCLUDED_

/* Define the version here.  In Unixland, it's defined on the command line now. */
#define VERSION "2.1.1 - titus fixes"

#pragma warning(disable: 4761) /* integral size mismatch in argument; conversion supplied */

#if _MSC_VER > 1000
#pragma once
#endif /* _MSC_VER > 1000 */

#endif /* !defined(AFX_STDAFX_H__31666CA1_2474_11D5_AE6C_F07569C10000__INCLUDED_) */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <winsock2.h>
#include <time.h>
#include <direct.h>
#include <math.h>

#include <sys/stat.h>   /* somewhat odd, but you don't get stat here with __STDC__ */

#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <process.h>

#define __STDC__ 1      /* something odd, CF want this, but don'T include it */
                        /* before the standard includes */

#ifndef HAVE_SNPRINTF
#define HAVE_SNPRINTF 1
#endif
#define snprintf _snprintf

/* include all needed autoconfig.h defines */
#define CS_LOGSTATS
#define HAVE_SRAND
#ifndef HAVE_FCNTL_H
    #define HAVE_FCNTL_H
#endif
#ifndef HAVE_STDDEF_H
    #define HAVE_STDDEF_H
#endif
#define GETTIMEOFDAY_TWO_ARGS
#define MAXPATHLEN 256
#define HAVE_STRTOL
#define HAVE_STRERROR

/* Many defines to redirect unix functions or fake standard unix values */
#define inline __inline
#define unlink(__a) _unlink(__a)
#define mkdir(__a, __b) mkdir(__a)
#define getpid() _getpid()
#define popen(__a, __b) _popen(__a, __b)
#define pclose(__a) _pclose(__a)
#define vsnprintf _vsnprintf
#define strtok_r(x, y, z) strtok(x, y)

#define R_OK 6          /* for __access() */
#define F_OK 6

#define PREFIXDIR ""

#define S_ISDIR(x) (((x)&S_IFMT) == S_IFDIR)
#define S_ISREG(x) (((x)&S_IFMT) == S_IFREG)

#ifndef S_ISGID
#define S_ISGID 0002000
#endif
#ifndef S_IWOTH
#define S_IWOTH 0000200
#endif
#ifndef S_IWGRP
#define S_IWGRP 0000020
#endif
#ifndef S_IWUSR
#define S_IWUSR 0000002
#endif
#ifndef S_IROTH
#define S_IROTH 0000400
#endif
#ifndef S_IRGRP
#define S_IRGRP 0000040
#endif
#ifndef S_IRUSR
#define S_IRUSR 0000004
#endif

#define WIFEXITED(x) 1
#define WEXITSTATUS(x) x

/* Location of read-only machine independent data */
#define DATADIR "share"
#define LIBDIR "share"
#define CONFDIR "share"

/* Location of changeable single system data (temp maps, hiscore, etc) */
#define LOCALDIR "var"

#define COMPRESS "/usr/bin/compress"
#define UNCOMPRESS "/usr/bin/uncompress"
#define GZIP "/bin/gzip"
#define GUNZIP "/bin/gunzip"
#define BZIP "/usr/bin/bzip2"
#define BUNZIP "/usr/bin/bunzip2"

/* Suffix for libraries */
#define PLUGIN_SUFFIX ".dll"

/* struct dirent - same as Unix */

typedef struct dirent {
    long d_ino;                         /* inode (always 1 in WIN32) */
    off_t d_off;                        /* offset to this dirent */
    unsigned short d_reclen;            /* length of d_name */
    char d_name[_MAX_FNAME+1];          /* filename (null terminated) */
}dirent;

#define NAMLEN(dirent) strlen((dirent)->d_name)

/* typedef DIR - not the same as Unix */
typedef struct {
    long handle;                        /* _findfirst/_findnext handle */
    short offset;                       /* offset into directory */
    short finished;                     /* 1 if there are not more files */
    struct _finddata_t fileinfo;        /* from _findfirst/_findnext */
    char *dir;                          /* the dir we are reading */
    struct dirent dent;                 /* the dirent to return */
} DIR;

#ifndef socklen_t
#define socklen_t int /* Doesn't exist, just a plain int */
#endif

/* Function prototypes */
extern int gettimeofday(struct timeval *time_Info, struct timezone *timezone_Info);
extern DIR *opendir(const char *);
extern struct dirent *readdir(DIR *);
extern int closedir(DIR *);
extern void rewinddir(DIR *);
extern int strncasecmp(const char *s1, const char *s2, int n);
extern int strcasecmp(const char *s1, const char *s2);
extern void service_register();
extern void service_unregister();
extern void service_handle();

#define HAVE_CURL_CURL_H

/* For Win32 service */
extern int bRunning;

/* Win32's Sleep takes milliseconds, not seconds. */
#define sleep(x) Sleep(x*1000)

#endif /* WIN32_H */
