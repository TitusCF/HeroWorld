/**
 * @file
 * Core defines: object types, flags, etc.
 *
 * This file is really too large.  With all the .h files
 * around, this file should be better split between them - things
 * that deal with objects should be in objects.h, things dealing
 * with players in player.h, etc.  As it is, everything just seems
 * to be dumped in here.
 *
 * This file is best viewed with a window width of about 100 character.
 */

#ifndef DEFINE_H
#define DEFINE_H

/*
 * Crossfire requires ANSI-C, but some compilers "forget" to define it.
 * Thus the prototypes made by cextract don't get included correctly.
 */
#if !defined(__STDC__)
/* Removed # from start of following line.  makedepend was picking it up.
 * The following should still hopefully result in an error.
 */
error - Your ANSI C compiler should be defining __STDC__;
#endif

#ifndef WIN32 /* ---win32 exclude unix configuration part */
#include <autoconf.h>
#endif

/** Decstations have trouble with fabs()... */
#define FABS(x) ((x) < 0 ? -(x) : (x))

#ifdef __NetBSD__
#include <sys/param.h>
#endif
#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif

/** NAME_MAX used by random maps may not be defined on pure ansi systems */
#ifndef NAME_MAX
#define NAME_MAX 255
#endif

#define MIN_STAT                1       /**< The minimum legal value of any stat */

#define MAX_BUF                 256     /**< Used for all kinds of things */
#define VERY_BIG_BUF            1024
#define HUGE_BUF                4096    /**< Used for messages - some can be quite long */

#define MAX_ANIMATIONS          256

#define MAX_NAME 48
#define BIG_NAME 32

/**
 * Fatal variables; used as arguments to fatal().
 */
/*@{*/
#define OUT_OF_MEMORY           0
#define MAP_ERROR               1
#define ARCHTABLE_TOO_SMALL     2
#define ARCHETYPE_ISSUE         3
#define SEE_LAST_ERROR          4

/*@}*/

/**
 * @defgroup OBJECT_TYPE Object types.
 *
 * Only add new values to this list if somewhere in the program code, it is
 * actually needed.  Just because you add a new monster does not mean it has to
 * have a type defined here.  That only needs to happen if in some .c file, it
 * needs to do certain special actions based on the monster type, that can not
 * be handled by any of the numerous flags.  Also, if you add new entries, try
 * and fill up the holes in this list.  Additionally, when you add a new entry,
 * include it in the table in common/item.c
 *
 * type 0 is undefined and indicates non-valid type information.
 */
/*@{*/
#define PLAYER                      1
#define TRANSPORT                   2     /**< see doc/Developers/objects */
#define ROD                         3
#define TREASURE                    4
#define POTION                      5
#define FOOD                        6
#define POISON                      7
#define BOOK                        8
#define CLOCK                       9
#define DRAGON_FOCUS                10    /**< Used during character creation */
/*#define FBALL                     11 */
/*#define LIGHTNING                 12 */
#define ARROW                       13
#define BOW                         14
#define WEAPON                      15
#define ARMOUR                      16
#define PEDESTAL                    17
#define ALTAR                       18
/*#define CONFUSION                 19 */
#define LOCKED_DOOR                 20
#define SPECIAL_KEY                 21
#define MAP                         22
#define DOOR                        23
#define KEY                         24
/*#define MMISSILE                  25 */
#define TIMED_GATE                  26
#define TRIGGER                     27
#define GRIMREAPER                  28
#define MAGIC_EAR                   29
#define TRIGGER_BUTTON              30
#define TRIGGER_ALTAR               31
#define TRIGGER_PEDESTAL            32
#define SHIELD                      33
#define HELMET                      34
/*#define HORN                      35    has been merged into ROD */
#define MONEY                       36
#define CLASS                       37    /**< Object for applying character class
                                           * modifications to someone */
/*#define GRAVESTONE                38 */
#define AMULET                      39
#define PLAYERMOVER                 40
#define TELEPORTER                  41
#define CREATOR                     42
#define SKILL                       43    /**< Also see ::SKILL_TOOL (74) below */
#define EARTHWALL                   45
#define GOLEM                       46
/*#define BOMB                      47 */
#define THROWN_OBJ                  48
#define BLINDNESS                   49
#define GOD                         50

#define DETECTOR                    51    /**< peterm:  detector is an object
                                           * which notices the presense of
                                           * another object and is triggered
                                           * like buttons. */
#define TRIGGER_MARKER              52    /**< inserts an invisible, weightless
                                           * force into a player with a
                                           * specified string WHEN TRIGGERED. */
#define DEAD_OBJECT                 53
#define DRINK                       54
#define MARKER                      55    /**< inserts an invisible, weightless
                                           * force into a player with a
                                           * specified string. */
#define HOLY_ALTAR                  56
#define PLAYER_CHANGER              57
#define BATTLEGROUND                58    /**< battleground, by Andreas Vogl */

#define PEACEMAKER                  59    /**< Object owned by a player which can
                                           * convert a monster into a peaceful
                                           * being incapable of attack.  */
#define GEM                         60
/*#define FIRECHEST                 61 */ /* FIRECHEST folded into FIREWALL */
#define FIREWALL                    62
/*#define ANVIL                     63 */
#define CHECK_INV                   64    /**< b.t. thomas@nomad.astro.psu.edu */
#define MOOD_FLOOR                  65    /**< b.t. thomas@nomad.astro.psu.edu
                                           * values of last_sp set how to
                                           * change:
                                           * 0 = furious, all monsters become
                                           *     aggressive
                                           * 1 = angry, all but friendly become
                                           *     aggressive
                                           * 2 = calm, all aggressive monsters
                                           *     calm down
                                           * 3 = sleep, all monsters fall asleep
                                           * 4 = charm, monsters become pets */
#define EXIT                        66
#define ENCOUNTER                   67
#define SHOP_FLOOR                  68
#define SHOP_MAT                    69
#define RING                        70
#define FLOOR                       71    /**< Floor tile -> native layer 0 */
#define FLESH                       72    /**< animal 'body parts' -b.t. */
#define INORGANIC                   73    /**< metals, minerals, dragon scales */
#define SKILL_TOOL                  74    /**< Allows the use of a skill */
#define LIGHTER                     75

/* The trap_part, wall, light_source, misc_object, monster, and spawn_generator
 * types are not used in any archetypes, and should perhaps be removed.
 */
/*#define TRAP_PART                 76 */ /* Needed by set traps skill -b.t. */

#define WALL                        77    /**< Wall. Put it always in layer 1 if
                                           * not set is_floor */
/*#define LIGHT_SOURCE              78 */ /* torches, lamps, etc. */
#define MISC_OBJECT                 79    /**< misc. objects are for objects
                                           * without a function in the engine.
                                           * Like statues, clocks, chairs...
                                           *  If perhaps we create a function
                                           * where we can sit on chairs, we
                                           * create a new type and remove all
                                           * chairs from here. */
#define MONSTER                     80    /**< A real, living creature */
/*#define SPAWN_GENERATOR           81 */ /* Spawn point or monster generator */
#define LAMP                        82    /**< Lamp */
#define DUPLICATOR                  83    /**< Duplicator/multiplier object */
/*#define TOOL                      84 */ /* Tool for building objects */
#define SPELLBOOK                   85
/*#define BUILDFAC                  86 */ /* Facilities for building objects */
#define CLOAK                       87
/*#define CONE                      88 */
/*#define AURA                      89 */ /* Aura spell object */

#define SPINNER                     90
#define GATE                        91
#define BUTTON                      92
#define CF_HANDLE                   93
#define HOLE                        94    /* When open, objects fall through */
#define TRAPDOOR                    95
/*#define WORD_OF_RECALL            96 */
/*#define PARAIMAGE                 97 */
#define SIGN                        98
#define BOOTS                       99
#define GLOVES                      100
#define SPELL                       101
#define SPELL_EFFECT                102
#define CONVERTER                   103
#define BRACERS                     104
#define POISONING                   105
#define SAVEBED                     106
/*#define POISONCLOUD               107*/
/*#define FIREHOLES                 108*/
#define WAND                        109
/*#define ABILITY                   110*/
#define SCROLL                      111
#define DIRECTOR                    112
#define GIRDLE                      113
#define FORCE                       114
#define POTION_RESIST_EFFECT        115   /**< A force, holding the effect of a
                                           * resistance potion */
#define EVENT_CONNECTOR             116   /**< Lauwenmark: an invisible object
                                           * holding a plugin event hook */
#define CLOSE_CON                   121   /**< Eneq((at)csd.uu.se): Id for
                                           * close_container archetype. */
#define CONTAINER                   122
#define ARMOUR_IMPROVER             123
#define WEAPON_IMPROVER             124

/* unused: 125 - 129
 * type 125 was MONEY_CHANGER
 */
#define SKILLSCROLL                 130   /**< can add a skill to player's
                                           * inventory -bt.*/
#define DEEP_SWAMP                  138
#define IDENTIFY_ALTAR              139
/*#define CANCELLATION              141*/ /* not used with new spell code */
#define SHOP_INVENTORY              150   /**< Mark Wedel (mark@pyramid.com) Shop
                                           * inventories */
/*#define BALL_LIGHTNING            151*/ /* peterm:  ball lightning and color
                                           * spray */
/*#define SWARM_SPELL               153*/
#define RUNE                        154
#define TRAP                        155

#define POWER_CRYSTAL               156
#define CORPSE                      157

#define DISEASE                     158
#define SYMPTOM                     159

#define BUILDER                     160   /**< Generic item builder, see subtypes
                                           * below */
#define MATERIAL                    161   /**< Material for building */

#define OBJECT_TYPE_MAX             162   /**< Update if you add new types */
/* END TYPE DEFINE */
/*@}*/

/**
 * @defgroup TYPE_BUILDER Subtypes for ::BUILDER objects.
 */
/*@{*/
#define ST_BD_BUILD      1 /**< Builds an item */
#define ST_BD_REMOVE     2 /**< Removes an item */
/*@}*/

/**
 * @defgroup TYPE_MATERIAL Subtypes for ::MATERIAL objects.
 */
/*@{*/
#define ST_MAT_FLOOR    1 /**< Floor. */
#define ST_MAT_WALL     2 /**< Wall. */
#define ST_MAT_ITEM     3 /**< Most other items, including doors & such. */
#define ST_MAT_WINDOW   4 /**< Window. */
/*@}*/

/**
 * @defgroup TYPE_WEAPON Weapon types
 */
/*@{*/
#define WEAP_HIT        0  /**< the basic */
#define WEAP_SLASH      1  /**< slash */
#define WEAP_PIERCE     2  /**< arrows, stiletto */
#define WEAP_CLEAVE     3  /**< axe */
#define WEAP_SLICE      4  /**< katana */
#define WEAP_STAB       5  /**< knife, dagger */
#define WEAP_WHIP       6  /**< whips n chains */
#define WEAP_CRUSH      7  /**< big hammers, flails */
#define WEAP_BLUD       8  /**< bludgeoning, club, stick */
/*@}*/

/** Link an object type with skill needed to identify, and general name. */
typedef struct typedata {
    int number;         /**< Type. */
    const char *name;   /**< Object name. */
    const char *name_pl;/**< Plural name. */
    int identifyskill;  /**< Skill used to identify this object class. */
    int identifyskill2; /**< Second skill used to identify this object class. */
} typedata;


/**
 * @defgroup PU_xxx Pickup modes
 * Definitions for detailed pickup descriptions.
 *   The objective is to define intelligent groups of items that the
 *   user can pick up or leave as he likes.
 */
/*@{*/
/* high bit as flag for new pickup options */
#define PU_NOTHING              0x00000000

#define PU_DEBUG                0x10000000
#define PU_INHIBIT              0x20000000
#define PU_STOP                 0x40000000
#define PU_NEWMODE              0x80000000

#define PU_RATIO                0x0000000F

#define PU_FOOD                 0x00000010
#define PU_DRINK                0x00000020
#define PU_VALUABLES            0x00000040
#define PU_BOW                  0x00000080

#define PU_ARROW                0x00000100
#define PU_HELMET               0x00000200
#define PU_SHIELD               0x00000400
#define PU_ARMOUR               0x00000800

#define PU_BOOTS                0x00001000
#define PU_GLOVES               0x00002000
#define PU_CLOAK                0x00004000
#define PU_KEY                  0x00008000

#define PU_MISSILEWEAPON        0x00010000
#define PU_MELEEWEAPON          0x00020000
#define PU_MAGICAL              0x00040000
#define PU_POTION               0x00080000

#define PU_SPELLBOOK            0x00100000
#define PU_SKILLSCROLL          0x00200000
#define PU_READABLES            0x00400000
#define PU_MAGIC_DEVICE         0x00800000

#define PU_NOT_CURSED           0x01000000
#define PU_JEWELS               0x02000000
#define PU_FLESH                0x04000000
#define PU_CONTAINER            0x08000000
/*@}*/

/* Instead of using arbitrary constants for indexing the
 * freearr, add these values.  <= SIZEOFFREE1 will get you
 * within 1 space.  <= SIZEOFFREE2 wll get you withing
 * 2 spaces, and the entire array (< SIZEOFFREE) is
 * three spaces
 */
#define SIZEOFFREE1 8
#define SIZEOFFREE2 24
#define SIZEOFFREE 49

#define NROF_SOUNDS (23+NROFREALSPELLS) /* Number of sounds */

/**
 * @defgroup IS_xxx Convenience macros to determine what kind of things we are dealing with.
 */
/*@{*/
#define IS_WEAPON(op) \
        (op->type == ARROW || op->type == BOW || op->type == WEAPON)

#define IS_ARMOR(op) \
        (op->type == ARMOUR || op->type == HELMET || \
         op->type == BOOTS || op->type == GLOVES)

#define IS_SHIELD(op) \
        (op->type == SHIELD)

#define IS_LIVE(op) \
        ((op->type == PLAYER || QUERY_FLAG(op, FLAG_MONSTER) || \
        (QUERY_FLAG(op, FLAG_ALIVE) && !QUERY_FLAG(op, FLAG_GENERATOR) && \
        !op->type == DOOR)) && (!QUERY_FLAG(op, FLAG_IS_A_TEMPLATE)))

#define IS_ARROW(op) \
        (op->type == ARROW || \
        (op->type == SPELL_EFFECT && (op->subtype == SP_BULLET || op->subtype == SP_MAGIC_MISSILE)))
/*@}*/

/** This return TRUE if object has still randomitems which could be expanded. */
#define HAS_RANDOM_ITEMS(op) (op->randomitems && (!QUERY_FLAG(op, FLAG_IS_A_TEMPLATE)))

/**
 * @defgroup FLAG_xxx Object flags
 *
 * Those flags are object-related flags, stored in the obj::flags fields.
 *
 * Flag structure now changed.
 * Each flag is now a bit offset, starting at zero.  The macros
 * will update/read the appropriate flag element in the object
 * structure.
 *
 * Hopefully, since these offsets are integer constants set at run time,
 * the compiler will reduce the macros something as simple as the
 * old system was.
 *
 * Flags now have FLAG as the prefix.  This to be clearer, and also
 * to make sure F_ names are not still being used anyplace.
 *
 * The macros below assume that the flag size for each element is 32
 * bits.  IF it is smaller, bad things will happen.  See structs.h
 * for more info.
 *
 * All functions should use the macros below.  In process of converting
 * to the new system, I find several files that did not use the previous
 * macros.
 *
 * If any FLAG's are or changed, make sure the flag_names structure in
 * common/loader.l is updated.
 *
 * - flags[0] is 0 to 31
 * - flags[1] is 32 to 63
 * - flags[2] is 64 to 95
 * - flags[3] is 96 to 127
 *
 * Values can go up to 127 before the size of the flags array in the
 * object structure needs to be enlarged.
 * So there are 18 available flags slots
 */
/*@{*/
#define SET_FLAG(xyz, p) \
    ((xyz)->flags[p/32] |= (1U<<(p%32)))
#define CLEAR_FLAG(xyz, p) \
    ((xyz)->flags[p/32] &= ~(1U<<(p%32)))
#define QUERY_FLAG(xyz, p) \
    ((xyz)->flags[p/32]&(1U<<(p%32)))
#define COMPARE_FLAGS(p, q) \
    (                                                \
        ((p)->flags[0] == (q)->flags[0]) &&          \
        ((p)->flags[1] == (q)->flags[1]) &&          \
        ((p)->flags[2] == (q)->flags[2]) &&          \
        ((p)->flags[3] == (q)->flags[3])             \
    )

/* the flags themselves. */

#define FLAG_ALIVE              0  /**< Object can fight (or be fought) */
#define FLAG_WIZ                1  /**< Object has special privilegies */
#define FLAG_REMOVED            2  /**< Object is not in any map or invenory */
#define FLAG_FREED              3  /**< Object is in the list of free objects */
#define FLAG_WAS_WIZ            4  /**< Player was once a wiz */
#define FLAG_APPLIED            5  /**< Object is ready for use by living */
#define FLAG_UNPAID             6  /**< Object hasn't been paid for yet */
#define FLAG_USE_SHIELD         7  /**< Can this creature use a shield? */

#define FLAG_NO_PICK            8  /**< Object can't be picked up */
#define FLAG_CLIENT_ANIM_SYNC   9  /**< Let client animate this, synchronized */
#define FLAG_CLIENT_ANIM_RANDOM 10 /**< Client animate this, randomized */
#define FLAG_ANIMATE            11 /**< The object looks at archetype for faces */
#define FLAG_DIALOG_PARSED      12 /**< Was the object::msg field parsed? Temporary flag not saved */
#define FLAG_NO_SAVE            13 /**< If set (through plugins), the object is not saved on maps. */
#define FLAG_MONSTER            14 /**< Will attack players */
#define FLAG_FRIENDLY           15 /**< Will help players */

#define FLAG_GENERATOR          16 /**< Will generate type ob->stats.food */
#define FLAG_IS_THROWN          17 /**< Object is designed to be thrown. */
#define FLAG_AUTO_APPLY         18 /**< Will be applied when created */
#define FLAG_TREASURE           19 /**< Will generate treasure when applied */
#define FLAG_PLAYER_SOLD        20 /**< Object was sold to a shop by a player. */
#define FLAG_SEE_INVISIBLE      21 /**< Will see invisible player */
#define FLAG_CAN_ROLL           22 /**< Object can be rolled */
#define FLAG_OVERLAY_FLOOR      23 /**< Object is an overlay floor */
#define FLAG_IS_TURNABLE        24 /**< Object can change face with direction */
#define FLAG_PROBE              25 /**< Object displays HP information to player. */
/*#define FLAG_FLY_ON           26*//* As WALK_ON, but only with FLAG_FLYING */
/*#define FLAG_FLY_OFF          27*//* As WALK_OFF, but only with FLAG_FLYING */
#define FLAG_IS_USED_UP         28 /**< When (--food<0) the object will exit */
#define FLAG_IDENTIFIED         29 /**< Player knows full info about item */
#define FLAG_REFLECTING         30 /**< Object reflects from walls (lightning) */
#define FLAG_CHANGING           31 /**< Changes to other_arch when anim is done*/

/* Start of values in flags[1] */
#define FLAG_SPLITTING          32 /**< Object splits into stats.food other objs */
#define FLAG_HITBACK            33 /**< Object will hit back when hit */
#define FLAG_STARTEQUIP         34 /**< Object was given to player at start */
#define FLAG_BLOCKSVIEW         35 /**< Object blocks view */
#define FLAG_UNDEAD             36 /**< Monster is undead */
#define FLAG_SCARED             37 /**< Monster is scared (mb player in future)*/
#define FLAG_UNAGGRESSIVE       38 /**< Monster doesn't attack players */
#define FLAG_REFL_MISSILE       39 /**< Arrows will reflect from object */

#define FLAG_REFL_SPELL         40 /**< Spells (some) will reflect from object */
#define FLAG_NO_MAGIC           41 /**< Spells (some) can't pass this object */
#define FLAG_NO_FIX_PLAYER      42 /**< fix_object() won't be called */
#define FLAG_IS_LIGHTABLE       43 /**< object can be lit */
#define FLAG_TEAR_DOWN          44 /**< at->faces[hp*animations/maxhp] at hit */
#define FLAG_RUN_AWAY           45 /**< Object runs away from nearest player \
                                      but can still attack at a distance */
/*#define FLAG_PASS_THRU        46*/ /* Objects with can_pass_thru can pass \
                                      thru this object as if it wasn't there */
/*#define FLAG_CAN_PASS_THRU    47*/ /* Can pass thru... */

/*#define FLAG_PICK_UP          48*/ /* Can pick up */
#define FLAG_UNIQUE             49 /**< Item is really unique (UNIQUE_ITEMS) */
#define FLAG_NO_DROP            50 /**< Object can't be dropped */
#define FLAG_WIZCAST            51 /**< The wizard can cast spells in no-magic area */
#define FLAG_CAST_SPELL         52 /**< (Monster) can learn and cast spells */
#define FLAG_USE_SCROLL         53 /**< (Monster) can read scroll */
#define FLAG_USE_RANGE          54 /**< (Monster) can apply and use range items */
#define FLAG_USE_BOW            55 /**< (Monster) can apply and fire bows */

#define FLAG_USE_ARMOUR         56 /**< (Monster) can wear armour/shield/helmet */
#define FLAG_USE_WEAPON         57 /**< (Monster) can wield weapons */
#define FLAG_USE_RING           58 /**< (Monster) can use rings, boots, gauntlets, etc */
#define FLAG_READY_RANGE        59 /**< (Monster) has a range attack readied... 8) */
#define FLAG_READY_BOW          60 /**< not implemented yet */
#define FLAG_XRAYS              61 /**< X-ray vision */
#define FLAG_NO_APPLY           62 /**< Avoids step_on/fly_on to this object */
#define FLAG_IS_FLOOR           63 /**< Can't see what's underneath this object */

/* Start of values in flags[2] */
#define FLAG_LIFESAVE           64 /**< Saves a players' life once, then destr. */
#define FLAG_NO_STRENGTH        65 /**< Strength-bonus not added to wc/dam */
#define FLAG_SLEEP              66 /**< NPC is sleeping */
#define FLAG_STAND_STILL        67 /**< NPC will not (ever) move */
#define FLAG_RANDOM_MOVE        68 /**< NPC will move randomly */
#define FLAG_ONLY_ATTACK        69 /**< NPC will evaporate if there is no enemy */
#define FLAG_CONFUSED           70 /**< Will also be unable to cast spells */
#define FLAG_STEALTH            71 /**< Will wake monsters with less range */

#define FLAG_WIZPASS            72 /**< The wizard can go through walls */
#define FLAG_IS_LINKED          73 /**< The object is linked with other objects */
#define FLAG_CURSED             74 /**< The object is cursed */
#define FLAG_DAMNED             75 /**< The object is _very_ cursed */
#define FLAG_SEE_ANYWHERE       76 /**< The object will be visible behind walls */
#define FLAG_KNOWN_MAGICAL      77 /**< The object is known to be magical */
#define FLAG_KNOWN_CURSED       78 /**< The object is known to be cursed */
#define FLAG_CAN_USE_SKILL      79 /**< The monster can use skills */

#define FLAG_BEEN_APPLIED       80 /**< The object has been applied */
#define FLAG_READY_SCROLL       81 /**< monster has scroll in inv and can use it */
/*#define FLAG_USE_ROD          82 unused (Monster) can apply and use rods */
/*#define FLAG_READY_HORN       83 unused (Monster) has a horn readied */
/*#define FLAG_USE_HORN         84 unused (Monster) can apply and use horns */
#define FLAG_MAKE_INVIS         85 /**< (Item) gives invisibility when applied */
#define FLAG_INV_LOCKED         86 /**< Item will not be dropped from inventory */
#define FLAG_IS_WOODED          87 /**< Item is wooded terrain */

#define FLAG_IS_HILLY           88 /**< Item is hilly/mountain terrain */
#define FLAG_READY_SKILL        89 /**< (Monster or Player) has a skill readied */
#define FLAG_READY_WEAPON       90 /**< (Monster or Player) has a weapon readied */
#define FLAG_NO_SKILL_IDENT     91 /**< If set, item cannot be identified w/ a skill */
#define FLAG_BLIND              92 /**< If set, object cannot see (visually) */
#define FLAG_SEE_IN_DARK        93 /**< if set ob not effected by darkness */
#define FLAG_IS_CAULDRON        94 /**< container can make alchemical stuff */
/*#define FLAG_DUST             95 *//* item is a 'powder', effects throwing */

/* Start of values in flags[3] */
#define FLAG_NO_STEAL           96 /**< Item can't be stolen */
#define FLAG_ONE_HIT            97 /**< Monster can only hit once before going
                                    * away (replaces ghosthit)
                                    */
#define FLAG_CLIENT_SENT        98 /**< THIS IS A DEBUG FLAG ONLY.  We use it to
                                    * detect cases were the server is trying
                                    * to send an upditem when we have not
                                    * actually sent the item.
                                    */

#define FLAG_BERSERK            99  /**< monster will attack closest living
                                     * object */
#define FLAG_NEUTRAL            100 /**< monster is from type neutral */
#define FLAG_NO_ATTACK          101 /**< monster don't attack */
#define FLAG_NO_DAMAGE          102 /**< monster can't be damaged */
#define FLAG_OBJ_ORIGINAL       103 /**< NEVER SET THIS.  Item was loaded by
                                     * load_original_map() */
/*#define FLAG_OBJ_SAVE_ON_OVL    104 *//* this object should be saved on
                                     * the overlay, and is not subject to
                                     * decay. */
#define FLAG_ACTIVATE_ON_PUSH    105 /**< connected object is activated when 'pushed' */
#define FLAG_ACTIVATE_ON_RELEASE 106 /**< connected object is activated when 'released' */
#define FLAG_IS_WATER            107
#define FLAG_CONTENT_ON_GEN      108
#define FLAG_IS_A_TEMPLATE       109 /**< Object has no ingame life until instantiated*/
#define FLAG_IS_BUILDABLE        110 /**< Can build on item */
#define FLAG_AFK                 111 /**< Player is AFK */
#define FLAG_BLESSED             112 /**< Item has a blessing, opposite of cursed/damned. */
#define FLAG_KNOWN_BLESSED       113 /**< Item is known to be blessed. */
#define NUM_FLAGS                113 /**< Should always be equal to the last
                                      * defined flag.  If you change this,
                                      * make sure you update the flag_links
                                      * in common/loader.l
                                      */

/*@}*/

/**
 * @defgroup MOVE_xxx Movement types and related macros.
 *
 * Those flags are all possible movement types.
 *
 * If you add new movement types, you may need to update
 * describe_item() so properly describe those types.
 * change_abil() probably should be updated also.
 */
/*@{*/
#define MOVE_WALK       0x1     /**< Object walks. */
#define MOVE_FLY_LOW    0x2     /**< Low flying object. */
#define MOVE_FLY_HIGH   0x4     /**< High flying object. */
#define MOVE_FLYING     0x6     /**< Combo of fly_low and fly_high. */
#define MOVE_SWIM       0x8     /**< Swimming object. */
#define MOVE_BOAT       0x10    /**< Boats/sailing. */
#define MOVE_ALL        0x1f    /**< Mask of all movement types. */

/**
 * The normal assumption is that objects are walking/flying.
 * So often we don't want to block movement, but still don't want
 * to allow all types (swimming is rather specialized) - I also
 * expect as more movement types show up, this is likely to get
 * updated.  Basically, this is the default for spaces that allow
 * movement - anything but swimming right now.  If you really
 * want nothing at all, then can always set move_block to 0
 */
#define MOVE_BLOCK_DEFAULT MOVE_SWIM

/**
 * Typdef here to define type large enough to hold bitmask of
 * all movement types.  Make one declaration so easy to update.
 * uint8 is defined yet, so just use what that would define it
 * at anyways.
 */
typedef unsigned char MoveType;

/**
 * Basic macro to see if ob2 blocks ob1 from moving onto this space.
 * Basically, ob2 has to block all of ob1 movement types.
 */
#define OB_MOVE_BLOCK(ob1, ob2) \
    ((ob1->move_type&ob2->move_block) == ob1->move_type)

/**
 * Basic macro to see if if ob1 can not move onto a space based
 * on the 'type' move_block parameter
 * Add check - if type is 0, don't stop anything from moving
 * onto it.
 */
#define OB_TYPE_MOVE_BLOCK(ob1, type) \
    ((type != 0) && (ob1->move_type&type) == ob1->move_type)
/*@}*/

#define SET_GENERATE_TYPE(xyz, va) (xyz)->stats.sp = (va)
#define GENERATE_TYPE(xyz)      ((xyz)->stats.sp)
#define GENERATE_SPEED(xyz)     ((xyz)->stats.maxsp) /* if(!RANDOM()%<speed>) */

#define EXIT_PATH(xyz)          (xyz)->slaying
#define EXIT_LEVEL(xyz)         (xyz)->stats.food
#define EXIT_X(xyz)             (xyz)->stats.hp
#define EXIT_Y(xyz)             (xyz)->stats.sp
#define EXIT_ALT_X(xyz)         (xyz)->stats.maxhp
#define EXIT_ALT_Y(xyz)         (xyz)->stats.maxsp

/**
 * Max radii for 'light' object, really large values allow objects that can
 * slow down the game
 */
#define MAX_LIGHT_RADII         4
/**
 * Maximum map darkness, there is no practical reason to exceed this
 **/
#define MAX_DARKNESS            5

/**
 * Maximum item power an item can have. If changed, check object::item_power for overflow issues.
 */
#define MAX_WEAPON_ITEM_POWER   100

/**
 * @defgroup BS_xxx Buy/sell flags.
 * Those flags are mostly used for query_cost() and similar functions.
 */
/*@{*/
#define BS_BUY          0   /**< Item is being bought by player. */
#define BS_SELL         1   /**< Item is being sold by player. */
#define BS_TRUE         2   /**< True value of item, unadjusted. */
#define BS_NO_BARGAIN   4   /**< Combine with ::BS_BUY or ::BS_SELL to disable bargaining calc. */
#define BS_IDENTIFIED   8   /**< Flag to calculate value of identified item. */
#define BS_NOT_CURSED   16  /**< Flag to calculate value of uncursed item. */
#define BS_APPROX       32  /**< Flag to give a guess of item value. */
#define BS_SHOP         64  /**< Consider the effect that the shop that the player is in has. */
/*@}*/

#define DIRX(xyz)       freearr_x[(xyz)->direction]
#define DIRY(xyz)       freearr_y[(xyz)->direction]

#define ARMOUR_SPEED(xyz)       (xyz)->last_sp
#define ARMOUR_SPELLS(xyz)      (xyz)->gen_sp_armour
#define WEAPON_SPEED(xyz)       (xyz)->last_sp

/**
 * @defgroup MONSTER_MOVEMENT Monster movements
 * @author kholland@sunlab.cit.cornell.edu
 *
 * if your monsters start acting wierd, mail me.
 */
/*@{*/
/*
 * The following definitions are for the attack_movement variable in monsters.
 * If the attack_variable movement is left out of the monster archetype, or is
 * set to zero, the standard mode of movement from previous versions of
 * Crossfire will be used. The upper four bits of movement data are not in
 * effect when the monster has an enemy. These should only be used for non
 * agressive monsters. To program a monsters movement add the attack movement
 * numbers to the movement numbers example a monster that moves in a circle
 * until attacked and then attacks from a distance:
 *                                                      CIRCLE1 = 32
 *                                              +       DISTATT = 1
 *                                      -------------------
 *                      attack_movement = 33
 */
#define DISTATT  1 /**< Move toward a player if far, but maintain some space,
                    * attack from a distance - good for missile users only.   */
#define RUNATT   2 /**< Run but attack if player catches up to object.        */
#define HITRUN   3 /**< Run to then hit player then run away cyclicly.        */
#define WAITATT  4 /**< Wait for player to approach then hit, move if hit.    */
#define RUSH     5 /**< Rush toward player blindly, similiar to dumb monster. */
#define ALLRUN   6 /**< Always run, never attack good for sim. of weak player.*/
#define DISTHIT  7 /**< Attack from a distance if hit as recommended by Frank.*/
#define WAIT2    8 /**< Monster does not try to move towards player if far.
                    * Maintains comfortable distance.                         */
#define PETMOVE 16 /**< If the upper four bits of attack_movement
                    * are set to this number, the monster follows a player
                    * until the owner calls it back or off
                    * player followed denoted by ob->owner
                    * the monster will try to attack whatever the player is
                    * attacking, and will continue to do so until the
                    * calls off the monster - a key command will be
                    * inserted to do so.                                      */
#define CIRCLE1 32 /**< If the upper four bits of move_type / attack_movement
                    * are set to this number, the monster will move in a
                    * circle until it is attacked, or the enemy field is
                    * set, this is good for non-aggressive monsters and NPC.  */
#define CIRCLE2 48 /**< Same as ::CIRCLE1 but a larger circle is used.        */
#define PACEH   64 /**< The monster will pace back and forth until attacked.
                    * This is HORIZONTAL movement.                            */
#define PACEH2  80 /**< The monster will pace as above but the length of the
                    * pace area is longer and the monster stops before
                    * changing directions.
                    * This is HORIZONTAL movement.                            */
#define RANDO   96 /**< The monster will go in a random direction until
                    * it is stopped by an obstacle, then it chooses another
                    * direction.                                              */
#define RANDO2 112 /**< Constantly move in a different random direction.      */
#define PACEV  128 /**< The monster will pace back and forth until attacked.
                    * This is VERTICAL movement.                              */
#define PACEV2 144 /**< The monster will pace as above but the length of the
                    * pace area is longer and the monster stops before
                    * changing directions.
                    * This is VERTICAL movement.                              */
#define LO4     15 /**< bitmasks for upper and lower 4 bits from 8 bit fields */
#define HI4    240
/*@}*/

/**
 * @defgroup ST_xxx Player state.
 * Use of the state-variable in player objects.
 *
 * See the @ref page_connection "login process" page.
 */
/*@{*/
#define ST_PLAYING                  0   /**< Usual state. */
#define ST_PLAY_AGAIN               1   /**< Player left through a bed of reality, and can login again. */
#define ST_ROLL_STAT                2   /**< New character, rolling stats. */
#define ST_CHANGE_CLASS             3   /**< New character, choosing class. */
#define ST_CONFIRM_QUIT             4   /**< Player used the 'quit' command, make sure that's ok. */
#define ST_GET_NAME                 6   /**< Player just connected. */
#define ST_GET_PASSWORD             7   /**< Name entered, now for password. */
#define ST_CONFIRM_PASSWORD         8   /**< New character, confirm password. */
#define ST_GET_PARTY_PASSWORD       10  /**< Player tried to join a password-protected party. */
#define ST_CHANGE_PASSWORD_OLD      11  /**< Player is entering old password to change password */
#define ST_CHANGE_PASSWORD_NEW      12  /**< Player is entering new password */
#define ST_CHANGE_PASSWORD_CONFIRM  13  /**< Player is confirming new password */
/*@}*/

#define BLANK_FACE_NAME "blank.111"
#define EMPTY_FACE_NAME "empty.111"
#define SMOOTH_FACE_NAME "default_smoothed.111"

/*
 * Defines for the luck/random functions to make things more readable
 */

#define PREFER_HIGH     1
#define PREFER_LOW      0

/**
 * Simple function we use below to keep adding to the same string
 * but also make sure we don't overwrite that string.
 *
 * @param dest
 * string to append to.
 * @param orig
 * string to append.
 * @param[out] curlen
 * current length of dest. Will be updated by this function.
 * @param maxlen
 * maximum length of dest buffer.
 */
static inline void safe_strcat(char *dest, const char *orig, size_t *curlen, size_t maxlen) {
    if (*curlen == (maxlen-1))
        return;
    strncpy(dest+*curlen, orig, maxlen-*curlen-1);
    dest[maxlen-1] = 0;
    *curlen += strlen(orig);
    if (*curlen > (maxlen-1))
        *curlen = maxlen-1;
}

/**
 * @defgroup AP_xxx Flags for apply_special().
 *
 * Those flags correspond to the aflags parameter of the apply_special() function.
 */
/*@{*/
/* Basic flags, always use one of these */
#define AP_NULL         0   /**< Nothing specific. */
#define AP_APPLY        1   /**< Item is to be applied. */
#define AP_UNAPPLY      2   /**< Item is to be remvoed. */

#define AP_BASIC_FLAGS  15

/* Optional flags, for bitwise or with a basic flag */
#define AP_NO_MERGE     16  /**< Don't try to merge object after (un)applying it. */
#define AP_IGNORE_CURSE 32  /**< Apply/unapply regardless of cursed/damned status. */
#define AP_PRINT        64  /**< Print what to do, don't actually do it
                             * Note this is supported in all the functions */
#define AP_NOPRINT      128 /**< Don't print messages - caller will do that
                             * may be some that still print */
/*@}*/

/**
 * @defgroup AC_PLAYER_STAT for apply_changes_to_player()
 *
 * These flags determine what apply_changes_to_player() does for
 * stat adjustment, if anything.
 */
#define AC_PLAYER_STAT_LIMIT        1   /** Limit stats to racial maximum */
#define AC_PLAYER_STAT_NO_CHANGE    2   /** Do not make any stat adjustments */


/**
 * @defgroup CAN_APPLY_xxx Values returned by apply_can_apply_object().
 *
 * Bitmask values for 'apply_can_apply_object()' return values.
 * the CAN_APPLY_ prefix is to just note what function the
 * are returned from.
 *
 * - CAN_APPLY_NEVER: who will never be able to use this - requires a body
 *      location who doesn't have.
 * - CAN_APPLY_RESTRICTION: There is some restriction from using this item -
 *      this basically means one of the FLAGS are set saying you can't
 *      use this.
 * - CAN_APPLY_NOT_MASK - this can be used to check the return value to see
 *      if this object can do anything to use this object.  If the value
 *      returned from apply_can_apply_object() anded with the mask is non zero,
 *      then it is out of the control of this creature to use the item.
 *      otherwise it means that by unequipping stuff, they could apply the object
 * - CAN_APPLY_UNAPPLY: Player needs to unapply something before applying
 *      this.
 * - CAN_APPLY_UNAPPLY_MULT: There are multiple items that need to be
 *      unapplied before this can be applied.  Think of switching to
 *      a bow but you have a sword & shield - both the sword and
 *      shield need to be uneqipped before you can do the bow.
 * - CAN_APPLY_UNAPPLY_CHOICE: There is a choice of items to unapply before
 *      this one can be applied.  Think of rings - human is wearing two
 *      rings and tries to apply one - there are two possible rings he
 *      could remove.
 */
/*@{*/
#define CAN_APPLY_NEVER             0x1
#define CAN_APPLY_RESTRICTION       0x2
#define CAN_APPLY_NOT_MASK          0xf
#define CAN_APPLY_UNAPPLY           0x10
#define CAN_APPLY_UNAPPLY_MULT      0x20
#define CAN_APPLY_UNAPPLY_CHOICE    0x40
/*@}*/

/** Cut off point of when an object is put on the active list or not */
#define MIN_ACTIVE_SPEED        0.00001

/*
 * random() is much better than rand().  If you have random(), use it instead.
 * You shouldn't need to change any of this
 *
 * 0.93.3: It looks like linux has random (previously, it was set below
 * to use rand).  Perhaps old version of linux lack rand?  IF you run into
 * problems, add || defined(__linux__) the #if immediately below.
 *
 * 0.94.2 - you probably shouldn't need to change any of the rand stuff
 * here.
 */

#ifdef HAVE_SRANDOM
#define RANDOM() random()
#define SRANDOM(xyz) srandom(xyz)
#else
#  ifdef HAVE_SRAND48
#  define RANDOM() lrand48()
#  define SRANDOM(xyz) srand48(xyz)
#  else
#    ifdef HAVE_SRAND
#      define RANDOM() rand()
#      define SRANDOM(xyz) srand(xyz)
#    else
#      error "Could not find a usable random routine"
#    endif
#  endif
#endif

/**
 * Returns the weight of the given object. Note: it does not take the number of
 * items (nrof) into account.
 */
#define WEIGHT(op) (op->nrof ? op->weight : op->weight+op->carrying)

/**
 * Constructs a loop iterating over the inventory of an object. Example:
 * <pre>
 * FOR_INV_PREPARE(op, inv)
 *     LOG(llevInfo, "%s\n", inv->name);
 * FOR_INV_FINISH();
 * </pre>
 * or
 * <pre>
 * FOR_INV_PREPARE(op, inv) {
 *     LOG(llevInfo, "%s\n", inv->name);
 * } FOR_INV_FINISH();
 * </pre>
 * @param op_ the object to iterate over
 * @param it_ a variable name that holds the inventory objects; a new variable
 * will be declared; modifications do not affect the loop
 */
#define FOR_INV_PREPARE(op_, it_)                               \
    do {                                                        \
        object *it_ = (op_)->inv;                               \
        FOR_OB_AND_BELOW_PREPARE(it_);
/**
 * Finishes #FOR_INV_PREPARE().
 */
#define FOR_INV_FINISH()                                        \
        FOR_OB_AND_BELOW_FINISH();                              \
    } while(0)

/**
 * Constructs a loop iterating over all objects above an object.
 * @param op_ the object to iterate over
 * @param it_ a variable name that holds the inventory objects; a new variable
 * will be declared; modifications do not affect the loop
 */
#define FOR_ABOVE_PREPARE(op_, it_)                             \
    do {                                                        \
        object *it_ = (op_)->above;                             \
        FOR_OB_AND_ABOVE_PREPARE(it_);
/**
 * Finishes #FOR_ABOVE_PREPARE().
 */
#define FOR_ABOVE_FINISH()                                      \
        FOR_OB_AND_ABOVE_FINISH();                              \
    } while(0)

/**
 * Constructs a loop iterating over all objects below an object.
 * @param op_ the object to iterate over
 * @param it_ a variable name that holds the inventory objects; a new variable
 * will be declared; modifications do not affect the loop
 */
#define FOR_BELOW_PREPARE(op_, it_)                             \
    do {                                                        \
        object *it_ = (op_)->below;                             \
        FOR_OB_AND_BELOW_PREPARE(it_);
/**
 * Finishes #FOR_BELOW_PREPARE().
 */
#define FOR_BELOW_FINISH()                                      \
        FOR_OB_AND_BELOW_FINISH();                              \
    } while(0)

/**
 * Constructs a loop iterating over all objects of a map tile.
 * @param map_ the map to iterate over
 * @param mx_ the map's x-coordinate to iterate over
 * @param my_ the map's y-coordinate to iterate over
 * @param it_ a variable name that holds the inventory objects; a new variable
 * will be declared; modifications do not affect the loop
 */
#define FOR_MAP_PREPARE(map_, mx_, my_, it_)                    \
    do {                                                        \
        object *it_ = GET_MAP_OB((map_), (mx_), (my_));         \
        FOR_OB_AND_ABOVE_PREPARE(it_);
/**
 * Finishes #FOR_MAP_PREPARE().
 */
#define FOR_MAP_FINISH()                                        \
        FOR_OB_AND_ABOVE_FINISH();                              \
    } while(0)

/**
 * Constructs a loop iterating over an object and all objects above it in the
 * same pile.
 * @param op_ the object to start with
 */
#define FOR_OB_AND_ABOVE_PREPARE(op_) FOR_OB_PREPARE(op_, above, __LINE__)
/**
 * Finishes #FOR_OB_AND_ABOVE_PREPARE().
 */
#define FOR_OB_AND_ABOVE_FINISH() FOR_OB_FINISH()

/**
 * Constructs a loop iterating over an object and all objects below it in the
 * same pile.
 * @param op_ the object to start with
 */
#define FOR_OB_AND_BELOW_PREPARE(op_) FOR_OB_PREPARE(op_, below, __LINE__)
/**
 * Finishes #FOR_OB_AND_BELOW_PREPARE().
 */
#define FOR_OB_AND_BELOW_FINISH() FOR_OB_FINISH()

/**
 * Constructs a loop iterating over a set of objects. This function should not
 * be used directly in client code.
 * @param op_ the object start start with
 * @param field_ the field that holds the "next" pointer within op_
 * @param suffix_ a suffix for constructing unique variable names
 */
#define FOR_OB_PREPARE(op_, field_, suffix_) FOR_OB_PREPARE2(op_, field_, suffix_)
/**
 * Constructs a loop iterating over a set of objects. This function should not
 * be used at all. Use FOR_OB_PREPARE() instead.
 * @param op_ the object start start with
 * @param field_ the field that holds the "next" pointer within op_
 * @param suffix_ a suffix for constructing unique variable names
 */
#define FOR_OB_PREPARE2(op_, field_, suffix_)                     \
    do {                                                        \
        object *next##suffix_ = (op_);                        \
        tag_t next_tag##suffix_ = next##suffix_ == NULL ? 0 : next##suffix_->count;\
        while (((op_) = next##suffix_) != NULL) {              \
            if (object_was_destroyed(next##suffix_, next_tag##suffix_)) {\
                break;                                          \
            }                                                   \
            next##suffix_ = next##suffix_->field_;                               \
            next_tag##suffix_ = next##suffix_ == NULL ? 0 : next##suffix_->count;
/**
 * Finishes #FOR_OB_PREPARE().
 */
#define FOR_OB_FINISH()                                         \
        }                                                       \
    } while(0)

#endif /* DEFINE_H */
