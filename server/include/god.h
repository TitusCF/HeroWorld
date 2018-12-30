/**
 * @file
 * God-related structure.
 */

#ifndef GOD_H
#define GOD_H

/**
 * Used to link together the gods.
 */
typedef struct glnk {
    const char *name;     /**< Name of this god. */
    struct archt *arch;   /**< Pointer to the archetype of this god. */
    int id;               /**< Id of the god. */
    struct glnk *next;    /**< Next god. */
} godlink;


/**
 * @defgroup GOD_xxx God description flags, used with describe_god().
 */
/*@{*/
#define GOD_ENEMY       1       /**< Write down god's enemy. */
#define GOD_HOLYWORD    2       /**< Write holy word information. */
#define GOD_RESISTANCES 4       /**< Write resistances. */
#define GOD_SACRED      8       /**< Write sacred creatures. */
#define GOD_BLESSED     16      /**< Write various information (resistances?). */
#define GOD_IMMUNITIES  32      /**< Write immunities. */
#define GOD_PATHS       64      /**< Path information. */
#define GOD_ALL         0xFF    /**< Give all god information. */
/*@}*/

#endif /* GOD_H */
