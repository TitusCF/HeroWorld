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
 * @file porting.c
 * This file contains various functions that are not really unique for
 * crossfire, but rather provides what should be standard functions
 * for systems that do not have them.  In this way, most of the
 * nasty system dependent stuff is contained here, with the program
 * calling these functions.
 */

#include <string.h>
#ifdef WIN32 /* ---win32 exclude/include headers */
#include "process.h"
#include <fcntl.h>
#define pid_t int  /* we include it non global, because there is a redefinition in python.h */
#else
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <sys/param.h>
#include <stdio.h>

/* Need to pull in the HAVE_... values somehow */
/* win32 reminder: always put this in a ifndef win32 block */
#include <autoconf.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdarg.h>
/* Has to be after above includes so we don't redefine some values */
#include "global.h"

/** Used to generate temporary unique name. */
static unsigned int curtmp = 0;

/*****************************************************************************
 * File related functions
 ****************************************************************************/

/**
 * A replacement for the tempnam() function since it's not defined
 * at some unix variants. Do not use this function for new code, use
 * tempnam_secure() instead.
 *
 * @param dir
 * directory where to create the file. Can be NULL, in which case NULL is returned.
 * @param pfx
 * prefix to create unique name. Can be NULL.
 * @return
 * path to temporary file, or NULL if failure. Must be freed by caller.
 */
char *tempnam_local(const char *dir, const char *pfx) {
    char *name;
    pid_t pid = getpid();

/* HURD does not have a hard limit, but we do */
#ifndef MAXPATHLEN
#define MAXPATHLEN 4096
#endif

    if (!pfx)
        pfx = "cftmp.";

    /* This is a pretty simple method - put the pid as a hex digit and
     * just keep incrementing the last digit.  Check to see if the file
     * already exists - if so, we'll just keep looking - eventually we should
     * find one that is free.
     */
    if (dir != NULL) {
        if (!(name = (char *)malloc(MAXPATHLEN)))
            return(NULL);
        do {
#ifdef HAVE_SNPRINTF
            (void)snprintf(name, MAXPATHLEN, "%s/%s%x.%u", dir, pfx, (unsigned int)pid, curtmp);
#else
            (void)sprintf(name, "%s/%s%x%u", dir, pfx, (unsigned int)pid, curtmp);
#endif
            curtmp++;
        } while (access(name, F_OK) != -1);
        return(name);
    }
    return(NULL);
}

/**
 * A replacement for the tempnam_local() function since that one is not very
 * secure. This one will open the file in an atomic way on platforms where it is
 * possible.
 *
 * @param dir
 * Directory where to create the file. Can be NULL, in which case NULL is returned.
 * @param pfx
 * Prefix to create unique name. Can be NULL.
 * @param filename
 * This should be a pointer to a char*, the function will overwrite the char*
 * with the name of the resulting file. Must be freed by caller. Value is
 * unchanged if the function returns NULL.
 * @return
 * A pointer to a FILE opened exclusively, or NULL if failure.
 * It is up to the caller to properly close it.
 * @note
 * The file will be opened read-write.
 *
 * @todo
 * Maybe adding some ifdef for non-UNIX? I don't have any such system around
 * to test with.
 */
FILE *tempnam_secure(const char *dir, const char *pfx, char **filename) {
    char *tempname = NULL;
    int fd;
    int i;
    FILE *file = NULL;
#define MAXTMPRETRY 10

    /* Limit number of retries to MAXRETRY */
    for (i = 0; i < MAXTMPRETRY; i++) {
        tempname = tempnam_local(dir, pfx);
        /* tempnam_local only fails for really bad stuff, so lets bail out right
         * away then.
         */
        if (!tempname)
            return NULL;

        fd = open(tempname, O_CREAT|O_EXCL|O_RDWR, S_IRUSR|S_IWUSR);
        if (fd != -1)
            break;
        if (errno == EEXIST)
            LOG(llevError, "Created file detected in tempnam_secure. Someone hoping for a race condition?\n");
        free(tempname);
    }
    /* Check that we successfully got an fd. */
    if (fd == -1)
        return NULL;

    file = fdopen(fd, "w+");
    if (!file) {
        LOG(llevError, "fdopen() failed in tempnam_secure()!\n");
        free(tempname);
        return NULL;
    }

    *filename = tempname;
    return file;
}

/**
 * This function removes everything in the directory, and the directory itself.
 *
 * Errors are LOG() to error level.
 *
 * @param path
 * directory to remove.
 *
 * @note
 * will fail if any file has a name starting by .
 */
void remove_directory(const char *path) {
    DIR *dirp;
    char buf[MAX_BUF];
    struct stat statbuf;
    int status;

    if ((dirp = opendir(path)) != NULL) {
        struct dirent *de;

        for (de = readdir(dirp); de; de = readdir(dirp)) {
            /* Don't remove '.' or '..'  In  theory we should do a better
             * check for .., but the directories we are removing are fairly
             * limited and should not have dot files in them.
             */
            if (de->d_name[0] == '.')
                continue;

            /* Linux actually has a type field in the dirent structure,
             * but that is not portable - stat should be portable
             */
            status = stat(de->d_name, &statbuf);
            if ((status != -1) && (S_ISDIR(statbuf.st_mode))) {
                snprintf(buf, sizeof(buf), "%s/%s", path, de->d_name);
                remove_directory(buf);
                continue;
            }
            snprintf(buf, sizeof(buf), "%s/%s", path, de->d_name);
            if (unlink(buf)) {
                LOG(llevError, "Unable to remove %s\n", path);
            }
        }
        closedir(dirp);
    }
    if (rmdir(path)) {
        LOG(llevError, "Unable to remove directory %s\n", path);
    }
}

#if defined(sgi)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define popen fixed_popen

/**
 * Executes a command in the background through a call to /bin/sh.
 *
 * @param command
 * command which will be launched.
 * @param type
 * whether we want to read or write to that command. Must be "r" or "w".
 * @return
 * pointer to stream to command, NULL on failure.
 * @note
 * for SGI only.
 *
 * @todo
 * is this actually used?
 */
FILE *popen_local(const char *command, const char *type) {
    int fd[2];
    int pd;
    FILE *ret;
    if (!strcmp(type, "r")) {
        pd = STDOUT_FILENO;
    } else if (!strcmp(type, "w")) {
        pd = STDIN_FILENO;
    } else {
        return NULL;
    }
    if (pipe(fd) != -1) {
        switch (fork()) {
        case -1:
            close(fd[0]);
            close(fd[1]);
            break;

        case 0:
            close(fd[0]);
            if ((fd[1] == pd) || (dup2(fd[1], pd) == pd)) {
                if (fd[1] != pd) {
                    close(fd[1]);
                }
                execl("/bin/sh", "sh", "-c", command, NULL);
                close(pd);
            }
            exit(1);
            break;

        default:
            close(fd[1]);
            if (ret = fdopen(fd[0], type)) {
                return ret;
            }
            close(fd[0]);
            break;
        }
    }
    return NULL;
}

#endif /* defined(sgi) */

/*****************************************************************************
 * String related function
 ****************************************************************************/

/**
 * A replacement of strdup(), since it's not defined at some
 * unix variants.
 *
 * @param str
 * string to duplicate.
 * @return
 * copy, needs to be freed by caller. NULL on memory allocation error.
 */
char *strdup_local(const char *str) {
    char *c = (char *)malloc(strlen(str)+1);
    if (c != NULL)
        strcpy(c, str);
    return c;
}

/** Converts x to number */
#define DIGIT(x) (isdigit(x) ? (x)-'0' : \
islower(x) ? (x)+10-'a' : (x)+10-'A')
#define MBASE ('z'-'a'+1+10)

#if !defined(HAVE_STRTOL)
/**
 * Converts a string to long.
 *
 * A replacement of strtol() since it's not defined at
 * many unix systems.
 *
 * @param str
 * string to convert.
 * @param ptr
 * will point to first invalid character in str.
 * @param base
 * base to consider to convert to long.
 *
 * @todo
 * check weird -+ handling (missing break?)
 */
long strtol(register char *str, char **ptr, register int base) {
    register long val;
    register int c;
    int xx, neg = 0;

    if (ptr != (char **)0)
        *ptr = str;         /* in case no number is formed */
    if (base < 0 || base > MBASE)
        return (0);         /* base is invalid */
    if (!isalnum(c = *str)) {
        while (isspace(c))
            c = *++str;
        switch (c) {
        case '-':
            neg++;
        case '+':
            c = *++str;
        }
    }
    if (base == 0) {
        if (c != '0')
            base = 10;
        else {
            if (str[1] == 'x' || str[1] == 'X')
                base = 16;
            else
                base = 8;
        }
    }
    /*
     * For any base > 10, the digits incrementally following
     * 9 are assumed to be "abc...z" or "ABC...Z"
     */
    if (!isalnum(c) || (xx = DIGIT(c)) >= base)
        return 0;           /* no number formed */
    if (base == 16 && c == '0' && isxdigit(str[2]) && (str[1] == 'x' || str[1] == 'X'))
        c = *(str += 2);    /* skip over leading "0x" or "0X" */
    for (val = -DIGIT(c); isalnum(c = *++str) && (xx = DIGIT(c)) < base; )
        /* accumulate neg avoids surprises near
        MAXLONG */
        val = base*val-xx;
    if (ptr != (char **)0)
        *ptr = str;
    return (neg ? val : -val);
}
#endif

/**
 * Case-insensitive comparaison of strings.
 *
 * This seems to be lacking on some system.
 *
 * @param s1
 * @param s2
 * strings to compare.
 * @param n
 * maximum number of chars to compare.
 * @return
 * @li -1 if s1 is less than s2
 * @li 0 if s1 equals s2
 * @li 1 if s1 is greater than s2
 */
#if !defined(HAVE_STRNCASECMP)
int strncasecmp(const char *s1, const char *s2, int n) {
    register int c1, c2;

    while (*s1 && *s2 && n) {
        c1 = tolower(*s1);
        c2 = tolower(*s2);
        if (c1 != c2)
            return (c1-c2);
        s1++;
        s2++;
        n--;
    }
    if (!n)
        return (0);
    return (int)(*s1-*s2);
}
#endif

#if !defined(HAVE_STRCASECMP)
/**
 * Case-insensitive comparaison of strings.
 *
 * This seems to be lacking on some system.
 *
 * @param s1
 * @param s2
 * strings to compare.
 * @return
 * @li -1 if s1 is less than s2
 * @li 0 if s1 equals s2
 * @li 1 if s1 is greater than s2
 */
int strcasecmp(const char *s1, const char *s2) {
    register int c1, c2;

    while (*s1 && *s2) {
        c1 = tolower(*s1);
        c2 = tolower(*s2);
        if (c1 != c2)
            return (c1-c2);
        s1++;
        s2++;
    }
    if (*s1 == '\0' && *s2 == '\0')
        return 0;
    return (int)(*s1-*s2);
}
#endif

/**
 * Finds a substring in a string, in a case-insensitive manner.
 *
 * @param s
 * string we're searching into.
 * @param find
 * string we're searching for.
 * @return
 * pointer to first occurrence of find in s, NULL if not found.
 */
const char *strcasestr_local(const char *s, const char *find) {
    char c, sc;
    size_t len;

    if ((c = *find++) != 0) {
        c = tolower(c);
        len = strlen(find);
        do {
            do {
                if ((sc = *s++) == 0)
                    return NULL;
            } while (tolower(sc) != c);
        } while (strncasecmp(s, find, len) != 0);
        s--;
    }
    return s;
}

#if !defined(HAVE_SNPRINTF)
/**
 * Formats to a string, in a size-safe way.
 *
 * @param dest
 * where to write.
 * @param max
 * max length of dest.
 * @param format
 * format specifier, and arguments.
 * @return
 * number of chars written to dest.
 *
 * @warning
 * this function will abort() if there is an overflow.
 *
 * @todo
 * try to do something better than abort()?
 */
int snprintf(char *dest, int max, const char *format, ...) {
    va_list var;
    int ret;

    va_start(var, format);
    ret = vsprintf(dest, format, var);
    va_end(var);
    if (ret > max)
        abort();

    return ret;
}
#endif

/**
 * This takes an err number and returns a string with a description of
 * the error.
 *
 * @param errnum
 * error we want the description of.
 * @param buf
 * buffer to contain the description.
 * @param size
 * buf's length.
 * @return
 * buf.
 */
char *strerror_local(int errnum, char *buf, size_t size) {
#if defined(HAVE_STRERROR_R)
    /* Then what flavour of strerror_r... */
# if defined(STRERROR_R_CHAR_P)
    char *bbuf;

    buf[0] = 0;
    bbuf = (char *)strerror_r(errnum, buf, size);
    if ((buf[0] == 0) && (bbuf != NULL))
        snprintf(buf, size, "%s", bbuf);
# else
    if (strerror_r(errnum, buf, size) != 0) {
        /* EINVAL and ERANGE are possible errors from this strerror_r */
        if (errno == ERANGE) {
            snprintf(buf, size, "Too small buffer.");
        } else if (errno == EINVAL) {
            snprintf(buf, size, "Error number invalid.");
        }
    }
# endif /* STRERROR_R_CHAR_P */

#else /* HAVE_STRERROR_R */

# if defined(HAVE_STRERROR)
    snprintf(buf, size, "%s", strerror(errnum));
# else
#  error If this is C89 the compiler should have strerror!;
# endif
#endif /* HAVE_STRERROR_R */
    return buf;
}

/**
 * Computes the square root.
 * Based on (n+1)^2 = n^2 + 2n + 1
 * given that   1^2 = 1, then
 *              2^2 = 1 + (2 + 1) = 1 + 3 = 4
 *              3^2 = 4 + (4 + 1) = 4 + 5 = 1 + 3 + 5 = 9
 *              4^2 = 9 + (6 + 1) = 9 + 7 = 1 + 3 + 5 + 7 = 16
 *              ...
 * In other words, a square number can be express as the sum of the
 * series n^2 = 1 + 3 + ... + (2n-1)
 *
 * @param n
 * number of which to compute the root.
 * @return
 * square root.
 */
int isqrt(int n) {
    int result, sum, prev;

    result = 0;
    prev = sum = 1;
    while (sum <= n) {
        prev += 2;
        sum += prev;
        ++result;
    }
    return result;
}

/**
 * Checks if any directories in the given path doesn't exist, and creates if necessary.
 *
 * @param filename
 * file path we'll want to access. Can be NULL.
 *
 * @note
 * will LOG() to debug and error.
 */
void make_path_to_file(const char *filename) {
    char buf[MAX_BUF], *cp = buf;
    struct stat statbuf;

    if (!filename || !*filename)
        return;

    strcpy(buf, filename);
    LOG(llevDebug, "make_path_tofile %s...\n", filename);
    while ((cp = strchr(cp+1, (int)'/'))) {
        *cp = '\0';
        if (stat(buf, &statbuf) || !S_ISDIR(statbuf.st_mode)) {
            LOG(llevDebug, "Was not dir: %s\n", buf);
            if (mkdir(buf, SAVE_DIR_MODE)) {
                char err[MAX_BUF];

                LOG(llevError, "Cannot mkdir %s: %s\n", buf, strerror_local(errno, err, sizeof(err)));
                return;
            }
        }
        *cp = '/';
    }
}
