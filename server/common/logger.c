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
 * @file logger.c
 * This handles logging, to file or strerr/stdout.
 */

#include <stdarg.h>
#include <global.h>
#include <sproto.h>

int reopen_logfile = 0; /* May be set in SIGHUP handler */

/**
 * Human-readable name of log levels.
 */
static const char *const loglevel_names[] = {
    "[EE] ",
    "[II] ",
    "[DD] ",
    "[MM] ",
};

/**
 * Logs a message to stderr, or to file.
 * Or discards the message if it is of no importance, and none have
 * asked to hear messages of that logLevel.
 *
 * See include/logger.h for possible logLevels.  Messages with llevInfo
 * and llevError are always printed, regardless of debug mode.
 *
 * @param logLevel
 * level of the message
 * @param format
 * message to log. Works like printf() and such
 */
void LOG(LogLevel logLevel, const char *format, ...) {
    char buf[20480];  /* This needs to be really really big - larger
                       * than any other buffer, since that buffer may
                       * need to be put in this one.
                       */

    char time_buf[2048];

    va_list ap;
    va_start(ap, format);

    buf[0] = '\0';
    if (logLevel <= settings.debug) {
        time_buf[0] = '\0';
        if (settings.log_timestamp == TRUE) {
            struct tm *time_tmp;
            time_t now = time((time_t *)NULL);

            time_tmp = localtime(&now);
            if (time_tmp != NULL) {
                if (strftime(time_buf, sizeof(time_buf), settings.log_timestamp_format, time_tmp) == 0) {
                    time_buf[0] = '\0';
                }
            }
        }

        vsnprintf(buf, sizeof(buf), format, ap);
#ifdef WIN32 /* ---win32 change log handling for win32 */
        if (time_buf[0] != 0) {
            fputs(time_buf, logfile);
            fputs(" ", logfile);
        }
        fputs(loglevel_names[logLevel], logfile);    /* wrote to file or stdout */
        fputs(buf, logfile);    /* wrote to file or stdout */
#ifdef DEBUG    /* if we have a debug version, we want see ALL output */
        fflush(logfile);    /* so flush this! */
#endif
        if (logfile != stderr) {  /* if was it a logfile wrote it to screen too */
            if (time_buf[0] != 0) {
                fputs(time_buf, stderr);
                fputs(" ", stderr);
            }
            fputs(loglevel_names[logLevel], stderr);
            fputs(buf, stderr);
        }
#else /* not WIN32 */
        if (reopen_logfile) {
            reopen_logfile = 0;
            if (fclose(logfile) != 0) {
                /* stderr has been closed if -detach was used, but it's better
                 * to try to report about this anyway. */
                perror("tried to close log file after SIGHUP in logger.c:LOG()");
            }
            if ((logfile = fopen(settings.logfilename, "a")) == NULL) {
                /* There's likely to be something very wrong with the OS anyway
                 * if reopening fails. */
                perror("tried to open log file after SIGHUP in logger.c:LOG()");
                emergency_save(0);
                clean_tmp_files();
                exit(1);
            }
            setvbuf(logfile, NULL, _IOLBF, 0);
            LOG(llevInfo, "logfile reopened\n");
        }

        if (time_buf[0] != 0) {
            fputs(time_buf, logfile);
            fputs(" ", logfile);
        }
        fputs(loglevel_names[logLevel], logfile);
        fputs(buf, logfile);
#endif
    }
    if (!exiting
    && !trying_emergency_save
    && logLevel == llevError
    && ++nroferrors > MAX_ERRORS) {
        exiting = 1;
        if (!trying_emergency_save)
            emergency_save(0);
    }
    va_end(ap);
}
