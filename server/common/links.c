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
 * @file links.c
 * Utility functions for links between objects.
 */

#include <global.h>

/**
 * Allocates a new objectlink structure, initialises it, and returns
 * a pointer to it.
 *
 * @note
 * will call fatal() if memory allocation failure, thus never return NULL.
 *
 * @return
 * new link object, cleared.
 */
objectlink *get_objectlink(void) {
    objectlink *ol = (objectlink *)CALLOC(1, sizeof(objectlink));
    if (!ol)
        fatal(OUT_OF_MEMORY);
    ol->ob = NULL;
    ol->next = NULL;
    ol->id = 0;
    return ol;
}

/**
 * Allocates a new oblinkpt structure, initialises it, and returns
 * a pointer to it.
 *
 * @note
 * will call fatal() if memory allocation failure, thus never return NULL.
 *
 * @return
 * new link pointer.
 */
oblinkpt *get_objectlinkpt(void) {
    oblinkpt *obp = (oblinkpt *)malloc(sizeof(oblinkpt));

    if (!obp)
        fatal(OUT_OF_MEMORY);
    obp->link = NULL;
    obp->next = NULL;
    obp->value = 0;
    return obp;
}

/**
 * Recursively frees all objectlinks.
 *
 * @param ol
 * object link to free.
 */

void free_objectlink(objectlink *ol) {
    if (ol->next)
        free_objectlink(ol->next);
    free(ol);
}

/**
 * Recursively frees all linked list of objectlink pointers
 *
 * @param obp
 * pointer to free.
 */
void free_objectlinkpt(oblinkpt *obp) {
    if (obp->next)
        free_objectlinkpt(obp->next);
    if (obp->link)
        free_objectlink(obp->link);
    free(obp);
}
