/*****************************************************************************/
/* hashtable.c                                                               */
/* Author: Alex Schultz, 2006                                                */
/* Based upon shstr.c, origionally written by Kjetil T. Homme, Oslo 1992.    */
/*****************************************************************************/
/* This is a pointer association hash table library for plugins to use with  */
/* a simple interface. This file is named as it is for other hash table      */
/* types to be added if people wish to.                                      */
/*****************************************************************************/
/* That code is placed under the GNU General Public Licence (GPL)            */
/* (C)2001-2005 by Chachkoff Yann (Feel free to deliver your complaints)     */
/*****************************************************************************/
/*  CrossFire, A Multiplayer game for X-windows                              */
/*                                                                           */
/*  Copyright (C) 2000 Mark Wedel                                            */
/*  Copyright (C) 1992 Frank Tore Johansen                                   */
/*                                                                           */
/*  This program is free software; you can redistribute it and/or modify     */
/*  it under the terms of the GNU General Public License as published by     */
/*  the Free Software Foundation; either version 2 of the License, or        */
/*  (at your option) any later version.                                      */
/*                                                                           */
/*  This program is distributed in the hope that it will be useful,          */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of           */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            */
/*  GNU General Public License for more details.                             */
/*                                                                           */
/*  You should have received a copy of the GNU General Public License        */
/*  along with this program; if not, write to the Free Software              */
/*  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                */
/*                                                                           */
/*****************************************************************************/

#include <string.h>
#include <stdlib.h>

#ifdef WIN32
#include <global.h>
typedef UINT_PTR uintptr_t;
#include <malloc.h>
#else
#include <stdint.h>
#include <autoconf.h>
#endif
#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif

#include <hashtable.h>

/**
 * Initialises the hash table for a pointer association table.
 *
 * @param hash_table
 * Pointer to the hash table to initialise.
 */
void init_ptr_assoc_table(ptr_assoc **hash_table) {
    (void)memset((void *)hash_table, 0, PTR_ASSOC_TABLESIZE*sizeof(ptr_assoc *));
}

/**
 * Hashing-function used by the pointer association library. Currently
 * just takes the pointer modulus the table size (which should be a prime
 * number).
 *
 * @param ptr
 * The pointer to hash.
 *
 * @return
 * The returned hash value.
 */
static int hashptr(void *ptr) {
    return (int)((uintptr_t)ptr%PTR_ASSOC_TABLESIZE);
}

/**
 * Allocates and initialises a new ptr_assoc structure.
 *
 * @param key
 * The key to lookup by in the association.
 * @param value
 * The value to store with the key.
 *
 * @return
 * The new ptr_assoc structure.
 */
static ptr_assoc *new_ptr_assoc(void *key, void *value) {
    ptr_assoc *assoc;

    assoc = (ptr_assoc *)malloc(sizeof(ptr_assoc));
    assoc->previous = NULL;
    assoc->array = NULL;
    assoc->next = NULL;
    assoc->key = key;
    assoc->value = value;
    return assoc;
}

/**
 * Adds a value to a hash table which one can lookup with key.
 *
 * @param hash_table
 * Pointer to the hash table to add to.
 * @param key
 * The key to lookup by in the association.
 * @param value
 * The value to store with the key.
 */
void add_ptr_assoc(ptr_assoc **hash_table, void *key, void *value) {
    ptr_assoc *assoc;
    int ind;

    ind = hashptr(key);
    assoc = hash_table[ind];

    /* Is there an entry for that hash? */
    if (assoc) {
        /* Simple case first: See if the first pointer matches. */
        if (key != assoc->key) {
            /* Apparantly, a association with the same hash value has this
             * slot. We must see in the list if this perticular key has
             * been registered before.
             */
            while (assoc->next) {
                assoc = assoc->next;
                if (key != assoc->key) {
                    /* This wasn't the right key... */
                    continue;
                }
                /* We found an entry for this key. Make sure the value
                 * is set as we want it.
                 */
                assoc->value = value;
                return;
            }
            /* There are no occurences of this key in the hash table. */
            {
                ptr_assoc *new_assoc;

                new_assoc = new_ptr_assoc(key, value);
                assoc->next = new_assoc;
                new_assoc->previous = assoc;
                return;
            }
        }
        return;
    } else {
        /* The string isn't registered, and the slot is empty. */
        hash_table[ind] = new_ptr_assoc(key, value);

        hash_table[ind]->array = &(hash_table[ind]);

        return;
    }
}

/**
 * Find the ptr_assoc with a given key.
 *
 * @param hash_table
 * Pointer to the hash table to search.
 * @param key
 * The key to lookup by in the association.
 *
 * @return
 * The ptr_assoc that is found, or null if none is found.
 */
static ptr_assoc *find_ptr_assoc(ptr_assoc **hash_table, void *key) {
    ptr_assoc *assoc;
    int ind;

    ind = hashptr(key);
    assoc = hash_table[ind];

    /* Is there an entry for that hash? */
    if (assoc) {
        /* Simple case first: Is the first key the right one? */
        if (assoc->key == key) {
            return assoc;
        } else {
            /* Recurse through the linked list, if there's one. */
            while (assoc->next) {
                assoc = assoc->next;
                if (assoc->key == key) {
                    return assoc;
                }
            }
            /* No match. Fall through. */
        }
    }
    return NULL;
}

/**
 * Find the value associated with a given key.
 *
 * @param hash_table
 * Pointer to the hash table to search.
 * @param key
 * The key to lookup by in the association.
 *
 * @return
 * The value associated with the key.
 */
void *find_assoc_value(ptr_assoc **hash_table, void *key) {
    ptr_assoc *assoc;

    assoc = find_ptr_assoc(hash_table, key);
    if (!assoc)
        return NULL;
    return assoc->value;
}

/**
 * Remove the association with a given key.
 *
 * @param hash_table
 * Pointer to the hash table to search.
 * @param key
 * The key to lookup by in the association.
 */
void free_ptr_assoc(ptr_assoc **hash_table, void *key) {
    ptr_assoc *assoc;

    assoc = find_ptr_assoc(hash_table, key);
    if (!assoc)
        return;

    if (assoc->array) {
        /* We must put a new value into the hash_table[].
         */
        if (assoc->next) {
            *(assoc->array) = assoc->next;
            assoc->next->previous = NULL;
            assoc->next->array = assoc->array;
        } else {
            *(assoc->array) = NULL;
        }
         free(assoc);
    } else {
        /* Relink and free this struct.
         */
        if (assoc->next)
            assoc->next->previous = assoc->previous;
        assoc->previous->next = assoc->next;
        free(assoc);
    }
}
