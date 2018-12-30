/**
 * @file
 * Defines and variables used by the artifact generation routines.
 */

#ifndef TREASURE_H
#define TREASURE_H

/** Chance an item becomes an artifact if not magic is 1 in this value. */
#define CHANCE_FOR_ARTIFACT     20

/** Maximum magic for difficulty/magic mapping. */
#define MAXMAGIC 4

/** Maximum difficulty for difficulty/magic mapping. */
#define DIFFLEVELS 201

/** Highest level of rods/staves/scrolls to generate. */
#define MAX_SPELLITEM_LEVEL 110

/**
 * Flags to generate_treasures():
 *
 * @todo
 * document some flags.
 *
 * @anchor GT_xxx
 */

enum {
    GT_ENVIRONMENT = 0x0001,    /**< ? */
    GT_INVISIBLE = 0x0002,      /**< Unused? */
    GT_STARTEQUIP = 0x0004,     /**< Generated items have the ::FLAG_STARTEQUIP. */
    GT_ONLY_GOOD = 0x0008,      /**< Don't generate bad/cursed items. Used for new player's equipment. */
    GT_UPDATE_INV = 0x0010,     /**< When object has been generated, send its information to player. */
    GT_MINIMAL = 0x0020         /**< Do minimal adjustments, don't make artifacts, and so on. */
};

/**
 * when a treasure got cloned from archlist, we want perhaps change some default
 * values. All values in this structure will override the default arch.
 * TODO: It is a bad way to implement this with a special structure.
 * Because the real arch list is a at runtime not changed, we can grap for example
 * here a clone of the arch, store it in the treasure list and then run the original
 * arch parser over this clone, using the treasure list as script until an END comes.
 * This will allow ANY changes which is possible and we use ony one parser.
 *
 * @todo
 * is this still used somewhere in the maps/code??
 */
typedef struct _change_arch {
    const char *name;              /**< is != NULL, copy this over the original arch name */
    const char *title;             /**< is != NULL, copy this over the original arch name */
    const char *slaying;           /**< is != NULL, copy this over the original arch name */
} _change_arch;

/**
 * treasure is one element in a linked list, which together consist of a
 * complete treasure-list.  Any arch can point to a treasure-list
 * to get generated standard treasure when an archetype of that type
 * is generated (from a generator)
*/
typedef struct treasurestruct {
    struct archt *item;                 /**< Which item this link can be */
    const char *name;                   /**< If non null, name of list to use instead */
    struct treasurestruct *next;        /**< Next treasure-item in a linked list */
    struct treasurestruct *next_yes;    /**< If this item was generated, use this link instead of ->next */
    struct treasurestruct *next_no;     /**< If this item was not generated, then continue here */
    struct _change_arch change_arch;    /**< Override default arch values if set in treasure list */
    uint8 chance;                       /**< Percent chance for this item */
    uint8 magic;                        /**< Max magic bonus to item
                                         * If the entry is a list transition,
                                         * 'magic' contains the difficulty
                                         * required to go to the new list
                                         */
    uint16 nrof;                        /**< random 1 to nrof items are generated */
} treasure;

/**
 * treasureliststruct represents one logical group of items to be generated together.
 */
typedef struct treasureliststruct {
    const char *name;                   /**< Usually monster-name/combination */
    sint16 total_chance;                /**< If non-zero, only 1 item on this
                                         * list should be generated.  The
                                         * total_chance contains the sum of
                                         * the chance for this list.
                                         */
    struct treasureliststruct *next;    /**< Next treasure-item in linked list */
    struct treasurestruct *items;       /**< Items in this list, linked */
} treasurelist;

#endif /* TREASURE_H */
