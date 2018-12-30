/**
 * @file
 * Material structures and defines.
 */

/* #defines are needed by living.h, so they must be loaded early */
#ifndef MATERIAL_H
#define MATERIAL_H

/**
 * @defgroup M_xxx Material types.
 */
/*@{*/
#define M_PAPER                 1     /**< Paper. */
#define M_IRON                  2     /**< Iron. */
#define M_GLASS                 4     /**< Glass. */
#define M_LEATHER               8     /**< Leather. */
#define M_WOOD                  16    /**< Wood. */
#define M_ORGANIC               32    /**< General organic. */
#define M_STONE                 64    /**< Stone. */
#define M_CLOTH                 128   /**< Cloth. */
#define M_ADAMANT               256   /**< Adamant. */
#define M_LIQUID                512   /**< Liquid. */
#define M_SOFT_METAL            1024  /**< Soft metal. */
#define M_BONE                  2048  /**< Bone. */
#define M_ICE                   4096  /**< Ice. */
#define M_SPECIAL               8192  /**< When displaying names, don't show the
                                           materialname. */
/*@}*/

/** One material type. */
typedef struct _materialtype {
    const char  *name;            /**< Name of the material. */
    const char  *description;     /**< Description, unused. */
    int         material;         /**< What basic type(s) it is linked to. */
    sint8       save[NROFATTACKS];/**< Save chances for the attacks. */
    sint8       mod[NROFATTACKS]; /**< Modification to resistances. */
    struct _materialtype *next;   /**< Next item on the list. */
} materialtype_t;

/** Material types. */
EXTERN materialtype_t *materialt;

#endif  /* MATERIAL_H */
