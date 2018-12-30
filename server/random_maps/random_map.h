/**
 * @file random_maps/random_map.h
 * Random map related variables.
 */

#ifndef _RANDOM_MAP_H
#define _RANDOM_MAP_H

#define RM_SIZE         512

/**
 * Random map parameters.
 */
typedef struct RMParms {
    /**
     * Name of the wall style file, in /styles/wallstyles, can be an empty
     * string in which case a random one is chosen, or "none".
     */
    char wallstyle[RM_SIZE];
    /**
     * Will contain the actual wall archetype, can be an empty
     * string in which case a random one is chosen, or "none".
     */
    char wall_name[RM_SIZE];
    /**
     * Name of the floor style file, in /styles/floors, can be an empty
     * string in which case a random one is chosen, or "none".
     */
    char floorstyle[RM_SIZE];
    /**
     * Name of the monster style directory, in /styles/monsters, can be an empty
     * string in which case a random one is chosen.
     */
    char monsterstyle[RM_SIZE];
    /**
     * Name of the treasures style file, in /styles/treasurestyles, can be an empty
     * string in which case a random one is chosen, or "none".
     */
    char treasurestyle[RM_SIZE];
    /** Contains the layout type to generate, see layoutgen() for valid types. */
    char layoutstyle[RM_SIZE];
    /** Name of the doors style file, in /styles/doorstyles, see put_doors(). */
    char doorstyle[RM_SIZE];
    /**
     * Name of the decor style file, in /styles/decorstyles, can be an empty
     * string in which case a random one is chosen, or "none".
     */
    char decorstyle[RM_SIZE];
    /** Path to the map this random map is generated from, to make an exit back. */
    char origin_map[RM_SIZE];
    /** If not empty, the path of the final map this whole maze leads to. */
    char final_map[RM_SIZE];
    /** If not empty, the archetype name of the exit leading to the final map. */
    char final_exit_archetype[RM_SIZE];
    /** Name of the exit style files, in /styles/exitstyles/{up,down}, can be
     * an empty string in which case a random one is chosen, or "none".
     */
    char exitstyle[RM_SIZE];
    /** @todo remove, never non zero */
    char this_map[RM_SIZE];
    /**
     * If this is "no", then no exit will be made to the final map from the
     * previous random map.
     */
    char exit_on_final_map[RM_SIZE];
    /** If not empty, will be used in the name of the random keys. */
    char dungeon_name[RM_SIZE];

    int Xsize;
    int Ysize;
    int expand2x;
    int layoutoptions1;
    int layoutoptions2;
    int symmetry;
    int difficulty;
    /** If non zero, this means the difficulty was not zero initially. */
    int difficulty_given;
    float difficulty_increase;
    int dungeon_level;
    int dungeon_depth;
    int decoroptions;
    int orientation;
    int origin_y;
    int origin_x;
    int random_seed;
    int map_layout_style;
    /** Total hit points of the monsters in the map, used for treasure generation. */
    long unsigned int total_map_hp;
    int treasureoptions;
    int symmetry_used;
    struct regiondef *region;
    /**
     * If non zero, then the map will have multiple floors, else only one
     * floor will be used.
     */
    int multiple_floors;
} RMParms;

int load_parameters(FILE *fp, int bufstate, RMParms *RP);

/**
 * @defgroup RM_LAYOUT Random map layout
 */
/*@{*/
#define ONION_LAYOUT 1
#define MAZE_LAYOUT 2
#define SPIRAL_LAYOUT 3
#define ROGUELIKE_LAYOUT 4
#define SNAKE_LAYOUT 5
#define SQUARE_SPIRAL_LAYOUT 6
#define NROFLAYOUTS 6
/*@}*/

/**
 * @defgroup OPT_xxx Random map layout options.
 *
 * Move these defines out of room_gen_onion.c to here, as
 * other files (like square_spiral) also uses them.
 */
/*@{*/
#define OPT_RANDOM     0     /**< Random option. */
#define OPT_CENTERED   1     /**< Centered. */
#define OPT_LINEAR     2     /**< Linear doors (default is nonlinear). */
#define OPT_BOTTOM_C   4     /**< Bottom-centered. */
#define OPT_BOTTOM_R   8     /**< Bottom-right centered. */
#define OPT_IRR_SPACE  16    /**< Irregularly/randomly spaced layers (default: regular). */
#define OPT_WALL_OFF   32    /**< No outer wall. */
#define OPT_WALLS_ONLY 64    /**< Only walls. */
#define OPT_NO_DOORS   256   /**< Place walls insead of doors.  Produces broken map. */
/*@}*/

/**
 * @defgroup SYM_xxx Random map symetry
 * Symmetry definitions -- used in this file AND in @ref treasure.c,
 * the numerical values matter so don't change them.
 */
/*@{*/
#define RANDOM_SYM  0   /**< Random symmetry. */
#define NO_SYM      1   /**< No symmetry. */
#define X_SYM       2   /**< Vertical symmetry. */
#define Y_SYM       3   /**< Horizontal symmetry. */
#define XY_SYM      4   /**< Reflection. */
/*@}*/

/** Minimal size a random should have to actually be generated. */
#define MIN_RANDOM_MAP_SIZE 10

/**
 * Macro to get a strongly centered random distribution,
 * from 0 to x, centered at x/2 */
#define BC_RANDOM(x) ((int) ((RANDOM()%(x)+RANDOM()%(x)+RANDOM()%(x))/3.))

int set_random_map_variable(RMParms *rp, const char *buf);

#endif
