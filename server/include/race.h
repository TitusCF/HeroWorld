#ifndef RACE_H
#define RACE_H

/**
 * @file
 * Race-related structure.
 */

/**
 * Contains information about a race.
 * Used to link the race lists together, first item is ::first_race. */
typedef struct ralnk {
    const char *name;     /**< Name of this race entry. */
    int nrof;             /**< Number of things belonging to this race. */
    struct oblnk *member; /**< Linked object list of things belonging to this race. */
    struct ralnk *next;   /**< Next race. */
} racelink;

#endif /* RACE_H */
