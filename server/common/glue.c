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

#include <global.h>
#include <sproto.h>

/**
 * @file glue.c
 *
 * All this glue is currently needed to connect the game with the
 * server.  I'll try to make the library more able to "stand on it's
 * own legs" later; not done in 5 minutes to separate two parts of
 * the code which were connected, well, can you say "spagetti"?
 *
 * Glue has been cleaned, so this file is almost empty now :)
 * Ryo 2005-07-15
 */

/**
 * Error messages to display.
 */
static const char *const fatalmsgs[] = {
    "Failed to allocate memory",
    "Failed repeatedly to load maps",
    "Hashtable for archetypes is too small",
    "Fatal issue in archetype file",
    "See last error",
};

/**
 * fatal() is meant to be called whenever a fatal signal is intercepted.
 * It will call the emergency_save and the clean_tmp_files functions.
 *
 * @note
 * this function never returns, as it calls exit().
 */
void fatal(int err) {
    fprintf(logfile, "Fatal: %s\n", (unsigned)err < sizeof(fatalmsgs)/sizeof(*fatalmsgs) ? fatalmsgs[(unsigned)err] : "invalid error code");
    emergency_save(0);
    clean_tmp_files();
    fprintf(logfile, "Exiting...\n");
    exit(err);
}
