/*****************************************************************************/
/* Crossfire plugin support - (C) 2001 by Yann Chachkoff.                    */
/* This code is placed under the GPL.                                        */
/*****************************************************************************/

/**
 * @file
 * Definitions for the plugin system.
 * @todo link to plugin stuff when the documentation exists in doxygen form :) remove commented things line 329+.
 */

/*****************************************************************************/
/* Headers needed.                                                           */
/*****************************************************************************/

#ifndef PLUGIN_H
#define PLUGIN_H

/*****************************************************************************/
/* This one does not exist under Win32.                                      */
/*****************************************************************************/
#ifndef WIN32
#include <dlfcn.h>
#endif

#undef MODULEAPI
#ifdef WIN32
# ifdef PYTHON_PLUGIN_EXPORTS
#  define MODULEAPI __declspec(dllexport)
# else
#  define MODULEAPI __declspec(dllimport)
# endif
#else
#ifdef HAVE_VISIBILITY
# define MODULEAPI __attribute__((visibility("default")))
#else
# define MODULEAPI
#endif
#endif

#include <global.h>
#include <object.h>
#include <logger.h>

#ifdef HAVE_TIME_H
#include <time.h>
#endif
#include <../random_maps/random_map.h>
#include <../random_maps/rproto.h>

/*******************************************************************************/
/* This one does not exist under Win32.                                        */
/*******************************************************************************/
#ifndef WIN32
#include <dirent.h>
#endif

/*******************************************************************************/
/* Event ID codes. I sorted them to present local events first, but it is      */
/* just a 'cosmetic' thing.                                                    */
/*******************************************************************************/
/*******************************************************************************/
/* Local events. Those are always linked to a specific object.                 */
/*******************************************************************************/
#define EVENT_NONE      0  /**< No event. This exists only to reserve the "0". */
#define EVENT_APPLY     1  /**< Object applied-unapplied.                      */
#define EVENT_ATTACKED  2  /**< Object attacked, with weapon or spell.         */
#define EVENT_ATTACKS   33 /**< Weapon or arrow hitting something.             */
#define EVENT_DEATH     3  /**< Player or monster dead.                        */
#define EVENT_DROP      4  /**< Object dropped on the floor.                   */
#define EVENT_PICKUP    5  /**< Object picked up.                              */
#define EVENT_SAY       6  /**< Someone speaks.                                */
#define EVENT_STOP      7  /**< Thrown object stopped.                         */
#define EVENT_TIME      8  /**< Triggered each time the object can react/move. */
#define EVENT_THROW     9  /**< Object is thrown.                              */
#define EVENT_TRIGGER   10 /**< Button pushed, lever pulled, etc.              */
#define EVENT_CLOSE     11 /**< Container closed.                              */
#define EVENT_TIMER     12 /**< Timer connected triggered it.                  */
#define EVENT_DESTROY   13 /**< Object destroyed (includes map reset/swapout)  */
#define EVENT_USER      31 /**< User-defined event.                            */
#define EVENT_SELLING   32 /**< Object is being sold by another one.           */
/*******************************************************************************/
/* Global events. Those are never linked to a specific object.                 */
/*******************************************************************************/
#define EVENT_BORN      14 /**< A new character has been created.              */
#define EVENT_CLOCK     15 /**< Global time event.                             */
#define EVENT_CRASH     16 /**< Triggered when the server crashes. Not recursive */
#define EVENT_PLAYER_DEATH  17 /**< Global Death event                         */
#define EVENT_GKILL     18 /**< Triggered when anything got killed by anyone.  */
#define EVENT_LOGIN     19 /**< Player login.                                  */
#define EVENT_LOGOUT    20 /**< Player logout.                                 */
#define EVENT_MAPENTER  21 /**< A player entered a map.                        */
#define EVENT_MAPLEAVE  22 /**< A player left a map.                           */
#define EVENT_MAPRESET  23 /**< A map is resetting.                            */
#define EVENT_REMOVE    24 /**< A Player character has been removed.           */
#define EVENT_SHOUT     25 /**< A player 'shout' something.                    */
#define EVENT_TELL      26 /**< A player 'tell' something.                     */
#define EVENT_MUZZLE    27 /**< A player was Muzzled (no_shout set).           */
#define EVENT_KICK      28 /**< A player was Kicked by a DM                    */
#define EVENT_MAPUNLOAD 29 /**< A map is freed (includes swapping out)         */
#define EVENT_MAPLOAD   30 /**< A map is loaded                                */
#define NR_EVENTS 34

#include <stdarg.h>

#define CFAPI_NONE    0
#define CFAPI_INT     1
#define CFAPI_LONG    2
#define CFAPI_CHAR    3
#define CFAPI_STRING  4 /* String with a length that must be given too. */
#define CFAPI_POBJECT 5
#define CFAPI_PMAP    6
#define CFAPI_FLOAT   7
#define CFAPI_DOUBLE  8
#define CFAPI_PARCH   9
#define CFAPI_FUNC    10
#define CFAPI_PPLAYER 11
#define CFAPI_PPARTY  12
#define CFAPI_PREGION 13
#define CFAPI_INT16   14
#define CFAPI_TIME    15
#define CFAPI_SINT64  16
#define CFAPI_SSTRING 17 /* Shared string that shouldn't be changed, or const char* */
#define CFAPI_MOVETYPE 18 /* MoveType */

/** General API function. */
typedef void (*f_plug_api)(int *type, ...);
/** Function called after the plugin was initialized. */
typedef int (*f_plug_postinit)(void);
/** First function called in a plugin. */
typedef int (*f_plug_init)(const char *iversion, f_plug_api gethooksptr);
/** Function to call to handle global or object-related events. */
typedef int (*f_plug_event)(int *type, ...);
/** Get various plugin properties. */
typedef void *(*f_plug_property)(int *type, ...);

#ifndef WIN32
#define LIBPTRTYPE void *
#else
/** Library handle. */
#define LIBPTRTYPE HMODULE
#endif

/** One loaded plugin. */
typedef struct _crossfire_plugin {
    f_plug_event    eventfunc;          /**< Event Handler function            */
    f_plug_property propfunc;           /**< Plugin getProperty function       */
    f_plug_postinit closefunc;          /**< Plugin Termination function       */
    LIBPTRTYPE      libptr;             /**< Pointer to the plugin library     */
    char            id[MAX_BUF];        /**< Plugin identification string      */
    char            fullname[MAX_BUF];  /**< Plugin full name                  */
    f_plug_event    gevent[NR_EVENTS];  /**< Global events registered          */
    struct _crossfire_plugin *next;     /**< Next plugin in list               */
    struct _crossfire_plugin *prev;     /**< Previous plugin in list           */
} crossfire_plugin;

#ifdef WIN32

#define plugins_dlopen(fname) LoadLibrary(fname)
#define plugins_dlclose(lib) FreeLibrary(lib)
#define plugins_dlsym(lib, name) GetProcAddress(lib, name)

#else /*WIN32 */

#define plugins_dlopen(fname) dlopen(fname, RTLD_NOW|RTLD_GLOBAL)   /**< Load a shared library. */
#define plugins_dlclose(lib) dlclose(lib)                           /**< Unload a shared library. */
#define plugins_dlsym(lib, name) dlsym(lib, name)                   /**< Get a function from a shared library. */
#define plugins_dlerror() dlerror()                                 /**< Library error. */
#endif /* WIN32 */

/* OBJECT-RELATED HOOKS */

#define CFAPI_OBJECT_PROP_OB_ABOVE          1
#define CFAPI_OBJECT_PROP_OB_BELOW          2
#define CFAPI_OBJECT_PROP_NEXT_ACTIVE_OB    3
#define CFAPI_OBJECT_PROP_PREV_ACTIVE_OB    4
#define CFAPI_OBJECT_PROP_INVENTORY         5
#define CFAPI_OBJECT_PROP_ENVIRONMENT       6
#define CFAPI_OBJECT_PROP_HEAD              7
#define CFAPI_OBJECT_PROP_CONTAINER         8
#define CFAPI_OBJECT_PROP_MAP               9
#define CFAPI_OBJECT_PROP_COUNT             10
#define CFAPI_OBJECT_PROP_NAME              12
#define CFAPI_OBJECT_PROP_NAME_PLURAL       13
#define CFAPI_OBJECT_PROP_TITLE             14
#define CFAPI_OBJECT_PROP_RACE              15
#define CFAPI_OBJECT_PROP_SLAYING           16
#define CFAPI_OBJECT_PROP_SKILL             17
#define CFAPI_OBJECT_PROP_MESSAGE           18
#define CFAPI_OBJECT_PROP_LORE              19
#define CFAPI_OBJECT_PROP_X                 20
#define CFAPI_OBJECT_PROP_Y                 21
#define CFAPI_OBJECT_PROP_SPEED             22
#define CFAPI_OBJECT_PROP_SPEED_LEFT        23
#define CFAPI_OBJECT_PROP_NROF              24
#define CFAPI_OBJECT_PROP_DIRECTION         25
#define CFAPI_OBJECT_PROP_FACING            26
#define CFAPI_OBJECT_PROP_TYPE              27
#define CFAPI_OBJECT_PROP_SUBTYPE           28
#define CFAPI_OBJECT_PROP_CLIENT_TYPE       29
#define CFAPI_OBJECT_PROP_RESIST            30
#define CFAPI_OBJECT_PROP_ATTACK_TYPE       31
#define CFAPI_OBJECT_PROP_PATH_ATTUNED      32
#define CFAPI_OBJECT_PROP_PATH_REPELLED     33
#define CFAPI_OBJECT_PROP_PATH_DENIED       34
#define CFAPI_OBJECT_PROP_MATERIAL          35
#define CFAPI_OBJECT_PROP_MATERIAL_NAME     36
#define CFAPI_OBJECT_PROP_MAGIC             37
#define CFAPI_OBJECT_PROP_VALUE             38
#define CFAPI_OBJECT_PROP_LEVEL             39
#define CFAPI_OBJECT_PROP_LAST_HEAL         40
#define CFAPI_OBJECT_PROP_LAST_SP           41
#define CFAPI_OBJECT_PROP_LAST_GRACE        42
#define CFAPI_OBJECT_PROP_LAST_EAT          43
#define CFAPI_OBJECT_PROP_INVISIBLE_TIME    44
#define CFAPI_OBJECT_PROP_PICK_UP           45
#define CFAPI_OBJECT_PROP_ITEM_POWER        46
#define CFAPI_OBJECT_PROP_GEN_SP_ARMOUR     47
#define CFAPI_OBJECT_PROP_WEIGHT            48
#define CFAPI_OBJECT_PROP_WEIGHT_LIMIT      49
#define CFAPI_OBJECT_PROP_CARRYING          50
#define CFAPI_OBJECT_PROP_GLOW_RADIUS       51
#define CFAPI_OBJECT_PROP_PERM_EXP          52
#define CFAPI_OBJECT_PROP_CURRENT_WEAPON    53
#define CFAPI_OBJECT_PROP_ENEMY             54
#define CFAPI_OBJECT_PROP_ATTACKED_BY       55
#define CFAPI_OBJECT_PROP_RUN_AWAY          56
#define CFAPI_OBJECT_PROP_CHOSEN_SKILL      57
#define CFAPI_OBJECT_PROP_HIDDEN            58
#define CFAPI_OBJECT_PROP_MOVE_STATUS       59
#define CFAPI_OBJECT_PROP_ATTACK_MOVEMENT   60
#define CFAPI_OBJECT_PROP_SPELL_ITEM        61
#define CFAPI_OBJECT_PROP_EXP_MULTIPLIER    62
#define CFAPI_OBJECT_PROP_ARCHETYPE         63
#define CFAPI_OBJECT_PROP_OTHER_ARCH        64
#define CFAPI_OBJECT_PROP_CUSTOM_NAME       65
#define CFAPI_OBJECT_PROP_ANIM_SPEED        66
#define CFAPI_OBJECT_PROP_FRIENDLY          67
#define CFAPI_OBJECT_PROP_SHORT_NAME        68
#define CFAPI_OBJECT_PROP_BASE_NAME         69
#define CFAPI_OBJECT_PROP_MAGICAL           70
#define CFAPI_OBJECT_PROP_LUCK              71
#define CFAPI_OBJECT_PROP_EXP               72
#define CFAPI_OBJECT_PROP_OWNER             73
#define CFAPI_OBJECT_PROP_PRESENT           74
#define CFAPI_OBJECT_PROP_CHEATER           75
#define CFAPI_OBJECT_PROP_MERGEABLE         76
#define CFAPI_OBJECT_PROP_PICKABLE          77
#define CFAPI_OBJECT_PROP_FLAGS             78
#define CFAPI_OBJECT_PROP_STR               79
#define CFAPI_OBJECT_PROP_DEX               80
#define CFAPI_OBJECT_PROP_CON               81
#define CFAPI_OBJECT_PROP_WIS               82
#define CFAPI_OBJECT_PROP_INT               83
#define CFAPI_OBJECT_PROP_POW               84
#define CFAPI_OBJECT_PROP_CHA               85
#define CFAPI_OBJECT_PROP_WC                86
#define CFAPI_OBJECT_PROP_AC                87
#define CFAPI_OBJECT_PROP_HP                88
#define CFAPI_OBJECT_PROP_SP                89
#define CFAPI_OBJECT_PROP_GP                90
#define CFAPI_OBJECT_PROP_FP                91
#define CFAPI_OBJECT_PROP_MAXHP             92
#define CFAPI_OBJECT_PROP_MAXSP             93
#define CFAPI_OBJECT_PROP_MAXGP             94
#define CFAPI_OBJECT_PROP_DAM               95
#define CFAPI_OBJECT_PROP_GOD               96
#define CFAPI_OBJECT_PROP_ARCH_NAME         97
#define CFAPI_OBJECT_PROP_INVISIBLE         98
#define CFAPI_OBJECT_PROP_FACE              99
#define CFAPI_OBJECT_PROP_ANIMATION         100
/*#define CFAPI_OBJECT_PROP_NO_SAVE           101*/
#define CFAPI_OBJECT_PROP_MOVE_TYPE         102
#define CFAPI_OBJECT_PROP_MOVE_BLOCK        103
#define CFAPI_OBJECT_PROP_MOVE_ALLOW        104
#define CFAPI_OBJECT_PROP_MOVE_ON           105
#define CFAPI_OBJECT_PROP_MOVE_OFF          106
#define CFAPI_OBJECT_PROP_MOVE_SLOW         107
#define CFAPI_OBJECT_PROP_MOVE_SLOW_PENALTY 108
#define CFAPI_OBJECT_PROP_DURATION          109
#define CFAPI_OBJECT_PROP_RAW_NAME          110

#define CFAPI_PLAYER_PROP_IP                150
#define CFAPI_PLAYER_PROP_MARKED_ITEM       151
#define CFAPI_PLAYER_PROP_PARTY             152
#define CFAPI_PLAYER_PROP_BED_MAP           153
#define CFAPI_PLAYER_PROP_BED_X             154
#define CFAPI_PLAYER_PROP_BED_Y             155
#define CFAPI_PLAYER_PROP_NEXT              156
#define CFAPI_PLAYER_PROP_TITLE             157

#define CFAPI_PLAYER_QUEST_START            0
#define CFAPI_PLAYER_QUEST_GET_STATE        1
#define CFAPI_PLAYER_QUEST_SET_STATE        2
#define CFAPI_PLAYER_QUEST_WAS_COMPLETED    3

#define CFAPI_MAP_PROP_FLAGS                0
#define CFAPI_MAP_PROP_DIFFICULTY           1
#define CFAPI_MAP_PROP_PATH                 2
#define CFAPI_MAP_PROP_TMPNAME              3
#define CFAPI_MAP_PROP_NAME                 4
#define CFAPI_MAP_PROP_RESET_TIME           5
#define CFAPI_MAP_PROP_RESET_TIMEOUT        6
#define CFAPI_MAP_PROP_PLAYERS              7
#define CFAPI_MAP_PROP_LIGHT                8
#define CFAPI_MAP_PROP_DARKNESS             9
#define CFAPI_MAP_PROP_WIDTH                10
#define CFAPI_MAP_PROP_HEIGHT               11
#define CFAPI_MAP_PROP_ENTER_X              12
#define CFAPI_MAP_PROP_ENTER_Y              13
#define CFAPI_MAP_PROP_MESSAGE              22
#define CFAPI_MAP_PROP_NEXT                 23
#define CFAPI_MAP_PROP_REGION               24
#define CFAPI_MAP_PROP_UNIQUE               25

#define CFAPI_ARCH_PROP_NAME                0
#define CFAPI_ARCH_PROP_NEXT                1
#define CFAPI_ARCH_PROP_HEAD                2
#define CFAPI_ARCH_PROP_MORE                3
#define CFAPI_ARCH_PROP_CLONE               4

#define CFAPI_PARTY_PROP_NAME               0
#define CFAPI_PARTY_PROP_NEXT               1
#define CFAPI_PARTY_PROP_PASSWORD           2
#define CFAPI_PARTY_PROP_PLAYER             3

#define CFAPI_REGION_PROP_NAME              0
#define CFAPI_REGION_PROP_NEXT              1
#define CFAPI_REGION_PROP_PARENT            2
#define CFAPI_REGION_PROP_LONGNAME          3
#define CFAPI_REGION_PROP_MESSAGE           4
#define CFAPI_REGION_PROP_JAIL_X            5
#define CFAPI_REGION_PROP_JAIL_Y            6
#define CFAPI_REGION_PROP_JAIL_PATH         7

/*****************************************************************************/
/* Exportable functions. Any plugin should define all those.                 */
/* initPlugin        is called when the plugin initialization process starts.*/
/* endPlugin         is called before the plugin gets unloaded from memory.  */
/* getPluginProperty is currently unused.                                    */
/* registerHook      is used to transmit hook pointers from server to plugin.*/
/* triggerEvent      is called whenever an event occurs.                     */
/*****************************************************************************/
/*extern MODULEAPI CFParm *initPlugin(CFParm *PParm);
extern MODULEAPI CFParm *endPlugin(CFParm *PParm);
extern MODULEAPI CFParm *getPluginProperty(CFParm *PParm);
extern MODULEAPI CFParm *registerHook(CFParm *PParm);
extern MODULEAPI CFParm *triggerEvent(CFParm *PParm);
*/

/** One function the server exposes to plugins. */
typedef struct _hook_entry {
    f_plug_api func;        /**< Function itself. */
    int fid;                /**< Function identifier. */
    const char fname[256];  /**< Function name. */
} hook_entry;

#endif /* PLUGIN_H */
