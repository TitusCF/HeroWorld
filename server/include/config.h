#ifndef CONFIG_H
#define CONFIG_H

/**
 * @file
 * This file contains various \#defines that select various options.
 * Some may not be desirable, and some just may not work.
 *
 * There are some options that are not selectable in this file which
 * may not always be undesirable.  An example would be certain
 * treasures that you may not want to have available.  To remove the
 * activation code would make these items worthless - instead remove
 * these from the treasure file.  Some things to look for are:
 *
 * prepare_weapon, improve_*: Allow characters to enchant their own
 *      weapons
 * ench_armour: Allow characters to enchant their armor.
 *
 * In theory, most of the values here should just be defaults, and
 * everything here should just be selectable by different run time
 * flags  However, for some things, that would just be too messy.
 */

/* There are 4 main sections to this file-
 * Section 1 is feature selection (enabling/disabling certain features)
 *
 * Section 2 is compiler/machine dependant section (stuff that just
 *     makes the program compile and run properly, but don't change the
 *     behavior)
 *
 * Section 3 is location of certain files and other defaults.  Things in
 *     this section generally do not need to be changed, and generally do
 *     not alter the play as perceived by players.  However, you may
 *     have your own values you want to set here.
 *
 * Section 4 deals with save file related options.
 */

/*******************************************************************
 * SECTION 1 - FEATURES
 *
 * You don't have to change anything here to get a working program, but
 * you may want to on personal preferance.  Items are arranged
 * alphabetically.
 *
 * Short list of features, and what to search for:
 * CS_LOGSTATS - log various new client/server data.
 * DEBUG - more verbose message logging?
 * MAP_CLIENT_X, MAP_CLIENT_Y - determines max size client map will receive
 * MAX_TIME - how long an internal tick is in microseconds
 * MANY_CORES - generate core dumps on gross errors instead of continuing?
 * PARTY_KILL_LOG - stores party kill information
 * WATCHDOG - allows use of an external watchdog program
 *
 ***********************************************************************/

/* These are basically experimental values right now - perhaps should
 * be turned into settings values at load time, but want to make
 * sure we go down this road before doing that.
 *
 * MIN_PLAYER_SPEED: If a player is fully loaded with junk, this is
 * how slow the character will move.
 *
 * MAX_PLAYER_SPEED: If the player is unloaded, this is how fast the
 * player will move.  Note that armor will still affect this (a
 * character otherwise unloaded but wearing plate will be slower)
 *
 * FREE_PLAYER_LOAD_PERCENT: This value is how much of the players
 * load is 'free' before it starts slowing them down - just because
 * I pick up a book doesn't make me slower.
 *
 * Example:  Player has a max carrying capacity of 1000 kg.  If he is
 * carrying 1000 KG, he moves at MIN_PLAYER_SPEED (0.25 by default).
 * The free load is 0.50 (50%), which means he can carry 500 kg before
 * he starts to slow down.  So if his load is anywhere between 0 and 500 kg
 * and character is otherwise not wearing armor or other things that slow
 * him down, he moves at 0.75
 * Once he goes above 500 kg, his speed is now a linear difference of
 * MIN and MAX.  With the nice round numbers chosen, it amounts to
 * each 1 kg of look slows character down 0.001 (500 * 0.001 = 0.5,
 * 0.75 - 0.5 = 0.25)
 *
 * The values can be adjusted in various ways - a free load percent
 * of 1.0 means how much you carry does not affect movement speed.
 * One could also adjust these values for slower of faster movement.
 *
 * 0.75 was chosen as default for fast movement as this gives room
 * for magic (and other effects) to speed up the player, but speed
 * still remain below 1.
 *
 * 0.25 was chosen as low end as that is still fairly slow (about
 * 2 spaces/second), but not so slow to be really painful -
 * typically it is low level characters that have this issue
 * more, so I think this will improve playing quality.
 */
#define MIN_PLAYER_SPEED    0.25
#define MAX_PLAYER_SPEED    0.75
#define FREE_PLAYER_LOAD_PERCENT    0.50

/*
 * BASE_WEAPON_SPEED is sort of like the speed - what the characters
 * weapon_speed is before we start factoring in bonuses & penalties.
 * Want fast melee?  Increase this.  Slow melee?  decrease this.
 * Note that weapon_speed decreases this, so generally you want
 * to start higher, as there isn't a lot of things that
 * will increase this
 */
#define BASE_WEAPON_SPEED   1.0

/* Use balanced stat loss code?
 * This code is a little more merciful with repeated stat loss at lower
 * levels. Basically, the more stats you have lost, the less likely that
 * you will lose more. Additionally, lower level characters are shown
 * a lot more mercy (there are caps on how much of a stat you can lose too).
 * On the nasty side, if you are higher level, you can lose mutiple stats
 * _at_once_ and are shown less mercy when you die. But when you're higher
 * level, it is much easier to buy back your stats with potions.
 * Turn this on if you want death-based stat loss to be more merciful
 * at low levels and more cruel at high levels.
 * Only works when stats are depleted rather than lost. This option has
 * no effect if you are using genuine stat loss.
 *
 * The BALSL_.. values control this behaviour.
 * BALSL_NUMBER_LOSSES_RATIO determines the number of stats to lose.
 * the character level is divided by that value, and that is how many
 * stats are lost.
 *
 * BALSL_MAX_LOSS_RATIO puts the upper limit on depletion of a stat -
 * basically, level/max_loss_ratio is the most a stat can be depleted.
 *
 * BALSL_LOSS_CHANCE_RATIO controls how likely it is a stat is depleted.
 * The chance not to lose a stat is
 * depleteness^2 / (depletedness^2+ level/ratio).
 * ie, if the stats current depleted value is 2 and the character is level
 * 15, the chance not to lose the stat is 4/(4+3) or 4/7.  The higher the
 * level, the more likely it is a stat can get really depleted, but
 * this gets more offset as the stat gets more depleted.
 *
 */
/* GD */

#define BALSL_LOSS_CHANCE_RATIO    4
#define BALSL_NUMBER_LOSSES_RATIO  6
#define BALSL_MAX_LOSS_RATIO       2

/* Don't edit these values.  They are configured in lib/settings.  These are
   Simply the defaults. */

#define BALANCED_STAT_LOSS FALSE
#define PERMANENT_EXPERIENCE_RATIO 50
#define DEATH_PENALTY_RATIO 20
#define DEATH_PENALTY_LEVEL 3
#define SET_TITLE TRUE
#define SIMPLE_EXP TRUE
#define SPELLPOINT_LEVEL_DEPEND TRUE
#define SPELL_ENCUMBRANCE TRUE
#define SPELL_FAILURE_EFFECTS FALSE
#define REAL_WIZ TRUE
#define RECYCLE_TMP_MAPS FALSE
#define RESURRECTION FALSE
#define SEARCH_ITEMS TRUE
#define NOT_PERMADETH TRUE
#define STAT_LOSS_ON_DEATH FALSE
#define PK_LUCK_PENALTY 1
#define CASTING_TIME FALSE
#define SET_FRIENDLY_FIRE 5
#define ARMOR_MAX_ENCHANT 5
#define ARMOR_WEIGHT_REDUCTION 10
#define ARMOR_WEIGHT_LINEAR TRUE
#define ARMOR_SPEED_IMPROVEMENT 10
#define ARMOR_SPEED_LINEAR TRUE

/* you can edit the ones below */

/**
 * CS_LOGSTATS will cause the server to log various usage stats
 * (number of connections, amount of data sent, amount of data received,
 * and so on.)  This can be very useful if you are trying to measure
 * server/bandwidth usage.  It will periodially dump out information
 * which contains usage stats for the last X amount of time.
 * CS_LOGTIME is how often it will print out stats.
 */
#ifndef WIN32    /* ***win32 we set the following stuff in the IDE */
#define CS_LOGSTATS
#endif
#ifdef CS_LOGSTATS
#define CS_LOGTIME  600
#endif

/**
 * DEBUG generates copious amounts of output.  I tend to change the CC options
 * in the crosssite.def file if I want this.  By default, you probably
 * dont want this defined.
 */
#ifndef WIN32                   /* ***win32 we set the following stuff in the IDE */
#ifndef DEBUG
#define DEBUG
#endif
#endif

/**
 * This option creates more core files.  In some areas, there are certain
 * checks done to try and make the program more stable (ie, check
 * parameter for null, return if it is).  These checks are being done
 * for things that should not happen (ie, being supplied a null parameter).
 * What MANY_CORES does, is if one of these checks is true, it will
 * dump core at that time, allowing for fairly easy tracking down of the
 * problem.  Better to fix problems than create thousands of checks.
 */
#define MANY_CORES

/**
 * This determines the maximum map size the client can request (and
 * thus what the server will send to the client.
 *
 * Client can still request a smaller map size (for bandwidth reasons
 * or display size of whatever else).
 *
 * The larger this number, the more cpu time and memory the server will
 * need to spend to figure this out in addition to bandwidth needs.
 * The server cpu time should be pretty trivial.
 *
 * There may be reasons to keep it smaller for the 'classic' crossfire
 * experience which was 11x11.  Big maps will likely make the same at
 * least somewhat easier, but client will need to worry about lag
 * more.
 *
 * I put support in for non square map updates in the define, but
 * there very well might be things that break horribly if this is
 * used.  I figure it is easier to fix that if needed than go back
 * at the future and have to redo a lot of stuff to support rectangular
 * maps at that point.
 *
 * MSW 2001-05-28
 */
#define MAP_CLIENT_X    25
#define MAP_CLIENT_Y    25

/**
 * If you feel the game is too fast or too slow, change MAX_TIME.
 * You can experiment with the 'speed \<new_max_time\>' command first.
 * The length of a tick is MAX_TIME microseconds.  During a tick,
 * players, monsters, or items with speed 1 can do one thing.
 */
#define MAX_TIME        120000

/**
 * Polymorph as it currently stands is unbalancing, so by default
 * we have it disabled.  It can be enabled and it works, but
 * it can be abused in various ways.
 */
#define NO_POLYMORPH

/**
 * This determine how many entries are stored in the kill log.  You
 * can see this information with the 'party kills' command.  More entries
 * mean slower performance and more memory.  IF this is not defined, then
 * this feature is disabled.
 */
/*
#define PARTY_KILL_LOG 20
*/

/**
 * The PERM_EXP values adjust the behaviour of permenent experience. - if
 * the setting permanent_experience_percentage is zero, these values have
 * no meaning. The value in the settings file is the percentage of the
 * experience that is permenent, the rest could be lost on death. When dying,
 * the greatest amount of non-permenent exp it is possible to lose at one time
 * is PERM_EXP_MAX_LOSS_RATIO  - this is calculated as
 * total exp - perm exp * loss ratio. The gain ratio is how much of experienced
 * experience goes to the permanent value. This does not detract from total
 * exp gain (ie, if you gained 100 exp, 100 would go to the skill total and
 * 10 to the permanent value).
 *
 * A few thoughts on these default value (by MSW)
 * gain ratio is pretty much meaningless until exp has been lost, as until
 * that poin, the value in the settings file will be used.
 * It is also impossible for the exp to actually be reduced to the permanent
 * exp ratio - since the loss ratio is .5, it will just get closer and
 * closer.  However, after about half a dozen hits, pretty much all the
 * exp that can be lost has been lost, and after that, only minor loss
 * will occur.
 */
/* GD */

#define PERM_EXP_GAIN_RATIO           0.50f
#define PERM_EXP_MAX_LOSS_RATIO       0.50f

/**
 * WATCHDOG lets sends datagrams to port 13325 on localhost
 * in (more-or-less) regular intervals, so an external watchdog
 * program can kill the server if it hangs (for whatever reason).
 * It shouldn't hurt anyone if this is defined but you don't
 * have an watchdog program.
 */
#ifndef WIN32   /* ***win32 disable watchdog as win32 default */
#define WATCHDOG
#endif


/***********************************************************************
 * SECTION 2 - Machine/Compiler specific stuff.
 *
 * Short list of items:
 * COMPRESS_SUFFIX - selection of compression programs
 * O_NDELAY - If you don't have O_NDELAY, uncomment it.
 *
 ***********************************************************************/

/**
 * If you compress your files to save space, set the COMPRESS_SUFFIX below
 * to the compression suffix you want (.Z, .gz, .bz2).  The autoconf
 * should already find the program to use.  If you set the suffix to
 * something that autoconf did not find, you are likely to have serious
 * problems, so make sure you have the appropriate compression tool installed
 * before you set this.  You can look at the autoconf.h file to see
 * what compression tools it found (search for COMPRESS).
 * Note that this is used when saving files.  Crossfire will search all
 * methods when loading a file to see if it finds a match
 */

#ifndef COMPRESS_SUFFIX
/* #define COMPRESS_SUFFIX ".Z" */
#endif

/**
 * If you get a complaint about O_NDELAY not being known/undefined, try
 * uncommenting this.
 * This may cause problems - O_NONBLOCK will return -1 on blocking writes
 * and set error to EAGAIN.  O_NDELAY returns 0.  This is only if no bytes
 * can be written - otherwise, the number of bytes written will be returned
 * for both modes.
 */

/*
#define O_NDELAY O_NONBLOCK
*/


/***********************************************************************
 * Section 3
 *
 * General file and other defaults that don't need to be changed, and
 * do not change gameplay as percieved by players much.  Some options
 * may affect memory consumption however.
 *
 * Values:
 *
 * BANFILE - ban certain users/hosts.
 * CSPORT - port to use for new client/server
 * DMFILE - file with dm/wizard access lists
 * LOGFILE - where to log if using -daemon option
 * MAP_ - various map timeout and swapping parameters
 * MAX_OBJECTS - how many objects to keep in memory.
 * MAX_OBJECTS_LWM - only swap maps out if below that value
 * MOTD - message of the day - printed each time someone joins the game
 * PERM_FILE - limit play times
 * SHUTDOWN - used when shutting down the server
 * SOCKETBUFSIZE - size of buffer used internally by the server for storing
 *    backlogged messages.
 * TMPDIR - directory to use for temp files
 * UNIQUE_DIR - directory to put unique item files into
 * USE_CALLOC for some memory requests
 ***********************************************************************
 */

/**
 * BANFILE - file used to ban certain sites from playing.  See the example
 * ban_file for examples.
 */
#ifndef BANFILE
#define BANFILE         "ban_file"
#endif

/**
 * CSPORT is the port used for the new client/server code.  Change
 * if desired.  Only of relevance if ERIC_SERVER is set above
 */
#define CSPORT 13327 /* old port + 1 */

/**
 * File containing valid names that can be dm, one on each line.  See
 * example dm_file for syntax help.
 */
#ifndef DMFILE
#define DMFILE "dm_file"
#endif

/**
 * LOGFILE specifies which file to log to when playing with the
 * -daemon option.
 */
#ifndef LOGFILE
#ifdef WIN32 /* change define path */
#define LOGFILE "var\\crossfire.log"
#else
#define LOGFILE "/var/log/crossfire/logfile"
#endif
#endif

/**
 * MAP_MAXTIMEOUT tells the maximum of ticks until a map is swapped out
 * after a player has left it.  If it is set to 0, maps are
 * swapped out the instant the last player leaves it.
 * If you are low on memory, you should set this to 0.
 * Note that depending on the map timeout variable, the number of
 * objects can get quite high.  This is because depending on the maps,
 * a player could be having the objects of several maps in memory
 * (the map he is in right now, and the ones he left recently.)
 * Each map has it's own TIMEOUT value and value field and it is
 * defaulted to 300
 *
 * Having a nonzero value can be useful: If a player leaves a map (and thus
 * is on a new map), and realizes they want to go back pretty quickly, the
 * old map is still in memory, so don't need to go disk and get it.
 *
 * MAP_MINTIMEOUT is used as a minimum timeout value - if the map is set
 * to swap out in less than that many ticks, we use the MINTIMEOUT value
 * below.  If MINTIMEOUT > MAXTIMEOUT, MAXTIMEOUT will be used for all
 * maps.
 */

/** How many ticks till maps are swapped out. */
#define MAP_MAXTIMEOUT  1000
/** At least that many ticks before swapout. */
#define MAP_MINTIMEOUT  500

/**
 * MAP_MAXRESET is the maximum time a map can have before being reset.  It
 * will override the time value set in the map, if that time is longer than
 * MAP_MAXRESET.  This value is in seconds.  If you are low on space on the
 * TMPDIR device, set this value to somethign small.  The default
 * value in the map object is MAP_DEFAULTRESET (given in seconds.)
 * I personally like 1 hour myself, for solo play.  It is long enough that
 * maps won't be resetting as a solve a quest, but short enough that some
 * maps (like shops and inns) will be reset during the time I play.
 * Comment out MAP_MAXRESET time if you always want to use the value
 * in the map archetype.
 */

/** Maximum time to reset. */
#define MAP_MAXRESET    7200
/** Default time to reset. */
#define MAP_DEFAULTRESET       7200

/**
 * MAX_OBJECTS is no hard limit.  If this limit is exceeded, Crossfire
 * will look for maps which are already scheldued for swapping, and
 * promptly swap them out before new maps are being loaded.
 * If playing only by yourself, this number can probably be as low as
 * 3000.  If in server mode, probably figure about 1000-2000 objects per
 * active player (if they typically play on different maps), for some guess
 * on how many to define.  If it is too low, maps just get swapped out
 * immediately, causing a performance hit.  If it is too high, the program
 * consumes more memory.  If you have gobs of free memory, a high number
 * might not be a bad idea.  Each object is around 350 bytes right now.
 * 25000 is about 8.5 MB
 */
#define MAX_OBJECTS     100000

/**
 * Max objects low water mark (lwm).  If defined, the map swapping strategy
 * is a bit different:
 * 1) We only start swapping maps if the number of objects in use is
 *    greater than MAX_OBJECTS above.
 * 2) We keep swapping maps until there are no more maps to swap or the number
 *    of used objects drop below this low water mark value.
 *
 * If this is not defined, maps are swapped out on the timeout value above,
 * or if the number of objects used is greater than MAX_OBJECTS above.
 *
 * Note:  While this will prevent the pauses noticed when saving maps, there
 * can instead be cpu performance penalties - any objects in memory get
 * processed.  So if there are 4000 objects in memory, and 1000 of them
 * are living objects, the system will process all 1000 objects each tick.
 * With swapping enable, maybe 600 of the objects would have gotten swapped
 * out.  This is less likely a problem with a smaller number of MAX_OBJECTS
 * than if it is very large.
 * Also, the pauses you do get can be worse, as if you enter a map with
 * a lot of new objects and go above MAX_OBJECTS, it may have to swap out
 * many maps to get below the low water mark.
 */
/*#define MAX_OBJECTS_LWM       MAX_OBJECTS/2*/

/**
 * Turning on MEMORY_DEBUG slows down execution, but makes it easier
 * to find memory corruption and leaks.  Currently, the main thing
 * that happens with this activated is that one malloc is done for
 * each object - thus whatever debugging mechanism the malloc library
 * (or other debugging tool provides, like purify), it can track this
 * individual malloc.  Default behaviour when turned off is that
 * enough memory is malloced for a large group of objects so malloc does
 * not need to be called as often.
 * This should only be turned on if some form of memory debugging tool
 * is being used - otherwise, turning this on will cause some performance
 * hit with no useful advantage.
 *
 * Define to 2 for stricter checks (known to currently break).
 * Define to 3 for even stricter checks (known to currently break even more).
 */
/*#define MEMORY_DEBUG 1*/

/**
 * If you want to have a Message Of The Day file, define MOTD to be
 * the file with the message.  If the file doesn't exist or if it
 * is empty, no message will be displayed.
 * (It resides in the CONFDIR directory)
 */
#define MOTD "motd"

/**
 * You can restrict playing in certain times by creating a PERMIT_FILE
 * in CONFDIR. See the sample for usage notes.
 */
#define PERM_FILE "forbid"

/**
 * If you want to take the game down while installing new versions, or
 * for other reasons, put a message into the SHUTDOWN_FILE file.
 * Remember to delete it when you open the game again.
 * (It resides in the CONFDIR directory)
 */
#ifndef SHUTDOWN_FILE
#define SHUTDOWN_FILE "shutdown"
#endif

/**
 * SOCKETBUFSIZE is the size of the buffer used internally by the server for
 * storing backlogged messages for the client.  This is not operating system
 * buffers or the like.  This amount is used per connection (client).
 * This buffer is in addition to OS buffers, so it may not need to be very
 * large.  When the OS buffer and this buffer is exhausted, the server
 * will drop the client connection for falling too far behind.  So if
 * you have very slow client connections, a larger value may be
 * warranted.
 */
#define SOCKETBUFSIZE 256*1024

/**
 * Your tmp-directory should be large enough to hold the uncompressed
 * map-files for all who are playing.
 * It ought to be locally mounted, since the function used to generate
 * unique temporary filenames isn't guaranteed to work over NFS or AFS
 * On the other hand, if you know that only one crossfire server will be
 * running using this temporary directory, it is likely to be safe to use
 * something that is NFS mounted (but performance may suffer as NFS is
 * slower than local disk)
 */
/*#define TMPDIR "/home/hugin/a/crossfire/crossfire/tmp"*/
#ifdef WIN32 /* change define path tmp */
#define TMPDIR "tmp"
#else
#define TMPDIR "/tmp"
#endif

/**
 * Directory to use for unique items. This is placed into the 'lib'
 * directory.  Changing this will cause any old unique items file
 * not to be used.
 */
#define UNIQUE_DIR "unique-items"

/**
 * If undefined, malloc is always used.
 * It looks like this can be oboleted.  However, it can be useful to
 * track down some bugs, as it will make sure that the entire data structure
 * is set to 0, at the expense of speed.
 * Rupert Goldie has run Purify against the code, and if this is disabled,
 * apparantly there are a lot of uninitialized memory reads - I haven't
 * seen any problem (maybe the memory reads are copies, and the destination
 * doesn't actually use the garbage values either?), but the impact on speed
 * of using this probably isn't great, and should make things more stable.
 * Msw 8-9-97
 */
#define USE_CALLOC

/**
 * These define the players starting map and location on that map, and where
 * emergency saves are defined.  This should be left as is unless you make
 * major changes to the map.
 */
#ifdef WIN32 /* change define path city */
#  define EMERGENCY_MAPPATH "\\world\\world_105_115"
#else
#  define EMERGENCY_MAPPATH "/world/world_105_115"
#endif
#  define EMERGENCY_X 5
#  define EMERGENCY_Y 37

/**
 * These defines tells where, relative to LIBDIR, the maps, the map-index,
 * archetypes highscore and treaures files and directories can be found.
 */
#define MAPDIR          "maps"
#define TEMPLATE_DIR    "template-maps"
#define ARCHETYPES      "archetypes"
#define REGIONS         "regions.reg"
#define HIGHSCORE       "highscore"
#define TREASURES       "treasures"
#define BANISHFILE      "banish_file"

#define MAX_ERRORS      25      /**< Bail out if more are received during tick. */
#define STARTMAX        500     /**< How big array of objects to start with. */
#define OBJ_EXPAND      100     /**< How big steps to use when expanding array. */

#define HIGHSCORE_LENGTH 1000   /**< How many entries there are room for. */

#define ARCHTABLE 8192          /**< Used when hashing archetypes. */
#define MAXSTRING 20

#define COMMAND_HASH_SIZE 107   /**< If you change this, delete all characters :) */

/***********************************************************************
 * Section 4 - save player options.
 *
 * There are a lot of things that deal with the save files, and what
 * gets saved with them, so I put them in there own section.
 *
 ***********************************************************************/

/**
 * If you want the players to be able to save their characters between
 * games, define SAVE_PLAYER and set PLAYERDIR to the directories
 * where the player-files will be put.
 * Remember to create the directory (make install will do that though).
 *
 * If you intend to run a central server, and not allow the players to
 * start their own Crossfire, you won't need to define this.
 *
 */
#ifndef PLAYERDIR
#define PLAYERDIR "players"
#endif

/**
 * If you have defined SAVE_PLAYER, you might want to change this, too.
 * This is the access rights for the players savefiles.
 * Given that crossfire runs in a client/server model, there should
 * be no issue setting these to be quite restrictive (600 and 700).
 * Before client/server, multiple people might run the executable,
 * thus requiring that the server be setuid/setgid, and more generous
 * permisisons needed.
 * SAVE_MODE is permissions for the files, SAVE_DIR_MODE is permission
 * for nay directories created.
 */
#define SAVE_MODE       0660
#define SAVE_DIR_MODE   0770

/* NOTE ON SAVE_INTERVAL and AUTOSAVE:  Only one of these two really
 * needs to be selected.  You can set both, and things will work fine,
 * however, it just means that a lot more saving will be done, which
 * can slow things down some.
 */

/**
 * How often (in seconds) the player is saved if he drops things.  If it is
 * set to 0, the player will be saved for every item he drops.  Otherwise,
 * if the player drops and item, and the last time he was saved
 * due to item drop is longer
 * the SAVE_INTERVAL seconds, he is then saved.  Depending on your playing
 * environment, you may want to set this to a higher value, so that
 * you are not spending too much time saving the characters.
 * This option should now work (Crossfire 0.90.5)
 */
/*#define SAVE_INTERVAL 300*/

/**
 * AUTOSAVE saves the player every AUTOSAVE ticks.  A value of
 * 5000 with MAX_TIME set at 120,000 means that the player will be
 * saved every 10 minutes.  Some effort should probably be made to
 * spread out these saves, but that might be more effort than it is
 * worth (Depending on the spacing, if enough players log on, the spacing
 * may not be large enough to save all of them.)  As it is now, it will
 * just set the base tick of when they log on, which should keep the
 * saves pretty well spread out (in a fairly random fashion.)
 */
#define AUTOSAVE 5000

/**
 * Often, emergency save fails because the memory corruption that caused
 * the crash has trashed the characters too. Define NO_EMERGENCY_SAVE
 * to disable emergency saves.  This actually does
 * prevent emergency saves now (Version 0.90.5).
 */
#define NO_EMERGENCY_SAVE

/**
 * By selecting the following, whenever a player does a backup save (with
 * the 'save' command), the player will be saved at home (EMERGENCY_MAP_*
 * information that is specified later).  If this is not set, the player
 * will be saved at his present location.
 */
#define BACKUP_SAVE_AT_HOME

/**
 * RESET_LOCATION_TIME is the number of seconds that must elapse before
 * we fill return the player to his savebed location.  If this is zero,
 * this feature is disabled (player will resume where ever he was
 * when he last logged off).  If this is set to less than two hours,
 * it will prevent players from camping out in treasure rooms.
 * Do not comment this out - it must be set to something - if you
 * comment this out, the program will not compile.
 *
 * This will work to BACKUP_SAVE_AT_HOME at home above, but where the player
 * where appear under what conditions is a little complicated depending
 * on how the player exited the game.  But if the elapsed time is greater than
 * the value below, player will always get returned to savebed location
 * location.
 *
 * Set to one hour as default
 */
#define RESET_LOCATION_TIME     3600

#endif /* CONFIG_H */
