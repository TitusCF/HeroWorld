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
 * @file
 * Contains the definition for all in-game commands a player can issue.
 */

#include <global.h>
#include <commands.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#endif
#include <ctype.h>

/* Added times to all the commands.  However, this was quickly done,
 * and probably needs more refinements.  All socket and DM commands
 * take 0 time.
 */

/**
 * Normal game commands.
 */
command_array_struct Commands[] = {
    { "afk", command_afk,                0.0 },
    { "apply", command_apply,            1.0 },   /* should be variable */
    { "applymode", command_applymode,    1.0 },   /* should be variable */
    { "body", command_body,              0.0 },
    { "bowmode", command_bowmode,        0.0 },
    { "brace", command_brace,            0.0 },
    { "cast", command_cast,              0.2 },   /* Is this right? */
    { "disarm", command_disarm,          1.0 },
    { "dm", command_dm,                  0.0 },
    { "dmhide", command_dmhide,          0.0 }, /* Like dm, but don't tell a dm arrived, hide player */
    { "drop", command_drop,              1.0 },
    { "dropall", command_dropall,        1.0 },
    { "empty", command_empty,            1.0 },
    { "examine", command_examine,        0.5 },
    { "fix_me", command_fix_me,          0.0 },
    { "get", command_take,               1.0 },
    { "help", command_help,              0.0 },
    { "hiscore", command_hiscore,        0.0 },
    { "inventory", command_inventory,    0.0 },
    { "invoke", command_invoke,          1.0 },
    { "killpets", command_kill_pets,     0.0 },
    { "language", command_language,      0.0 },
    { "listen", command_listen,          0.0 },
    { "lock", command_lock_item,         0.0 },
    { "maps", command_maps,              0.0 },
    { "mapinfo", command_mapinfo,        0.0 },
    { "mark", command_mark,              0.0 },
    { "motd", command_motd,              0.0 },
    { "news", command_news,              0.0 },
    { "party", command_party,            0.0 },
    { "party_rejoin", command_party_rejoin, 0.0 },
    { "passwd", command_passwd,          0.0 },
    { "peaceful", command_peaceful,      0.0 },
    { "petmode", command_petmode,        0.0 },
    { "pickup", command_pickup,          1.0 },
    { "prepare", command_prepare,        1.0 },
    { "printlos", command_printlos,      0.0 },
    { "quit", command_quit,              0.0 },
    { "ready_skill", command_rskill,     1.0 },
    { "rename", command_rename_item,     0.0 },
    { "resistances", command_resistances, 0.0 },
    { "rotateshoottype", command_rotateshoottype, 0.0 },
    { "rules", command_rules,            0.0 },
    { "save", command_save,              0.0 },
    { "skills", command_skills,          0.0 },   /* shows player list of skills */
    { "use_skill", command_uskill,       1.0 },
    { "search", command_search,          1.0 },
    { "search-items", command_search_items, 0.0 },
    { "showpets", command_showpets,      1.0 },
    { "sound", command_sound,            0.0 },
    { "statistics", command_statistics,  0.0 },
    { "take", command_take,              1.0 },
    { "throw", command_throw,            1.0 },
    { "time", command_time,              0.0 },
    { "title", command_title,            0.0 },
    { "use", command_use,                1.0 },
    { "usekeys", command_usekeys,        0.0 },
    { "whereabouts", command_whereabouts, 0.0 },
    { "whereami", command_whereami,      0.0 },
    { "unarmed_skill", command_unarmed_skill, 0.0 },
#ifdef DEBUG_MALLOC_LEVEL
    { "verify", command_malloc_verify,   0.0 },
#endif
    { "version", command_version,        0.0 },
    { "wimpy", command_wimpy,            0.0 },
    { "who", command_who,                0.0 },
    /*
     * directional commands
     */
    { "stay", command_stay,              1.0 }, /* 1.0 because it is used when using a
                                               *  skill on yourself */
    { "north", command_north,            1.0 },
    { "east", command_east,              1.0 },
    { "south", command_south,            1.0 },
    { "west", command_west,              1.0 },
    { "northeast", command_northeast,    1.0 },
    { "southeast", command_southeast,    1.0 },
    { "southwest", command_southwest,    1.0 },
    { "northwest", command_northwest,    1.0 },
    { "run", command_run,                1.0 },
    { "run_stop", command_run_stop,      0.0 },
    { "fire", command_fire,              1.0 },
    { "fire_stop", command_fire_stop,    0.0 },
    { "quest", command_quest,            0.0 },
    { "knowledge", command_knowledge,    0.0 }
};

/** Length of ::Commands array. */
const int CommandsSize = sizeof(Commands)/sizeof(command_array_struct);

/** Chat/shout related commands. */
command_array_struct CommunicationCommands [] = {
    { "tell", command_tell,              0.1 },
    { "reply", command_reply,            0.0 },
    { "say", command_say,                0.1 },
    { "gsay", command_gsay,              1.0 },
    { "shout", command_shout,            0.1 },
    { "chat", command_chat,              0.1 },
    { "me", command_me,                  0.1 },
    { "cointoss", command_cointoss,      0.0 },
    { "orcknuckle", command_orcknuckle,  0.0 },
    /*
     * begin emotions
     */
    { "nod", command_nod,                0.0 },
    { "dance", command_dance,            0.0 },
    { "kiss", command_kiss,              0.0 },
    { "bounce", command_bounce,          0.0 },
    { "smile", command_smile,            0.0 },
    { "cackle", command_cackle,          0.0 },
    { "laugh", command_laugh,            0.0 },
    { "giggle", command_giggle,          0.0 },
    { "shake", command_shake,            0.0 },
    { "puke", command_puke,              0.0 },
    { "growl", command_growl,            0.0 },
    { "scream", command_scream,          0.0 },
    { "sigh", command_sigh,              0.0 },
    { "sulk", command_sulk,              0.0 },
    { "hug", command_hug,                0.0 },
    { "cry", command_cry,                0.0 },
    { "poke", command_poke,              0.0 },
    { "accuse", command_accuse,          0.0 },
    { "grin", command_grin,              0.0 },
    { "bow", command_bow,                0.0 },
    { "clap", command_clap,              0.0 },
    { "blush", command_blush,            0.0 },
    { "burp", command_burp,              0.0 },
    { "chuckle", command_chuckle,        0.0 },
    { "cough", command_cough,            0.0 },
    { "flip", command_flip,              0.0 },
    { "frown", command_frown,            0.0 },
    { "gasp", command_gasp,              0.0 },
    { "glare", command_glare,            0.0 },
    { "groan", command_groan,            0.0 },
    { "hiccup", command_hiccup,          0.0 },
    { "lick", command_lick,              0.0 },
    { "pout", command_pout,              0.0 },
    { "shiver", command_shiver,          0.0 },
    { "shrug", command_shrug,            0.0 },
    { "slap", command_slap,              0.0 },
    { "smirk", command_smirk,            0.0 },
    { "snap", command_snap,              0.0 },
    { "sneeze", command_sneeze,          0.0 },
    { "snicker", command_snicker,        0.0 },
    { "sniff", command_sniff,            0.0 },
    { "snore", command_snore,            0.0 },
    { "spit", command_spit,              0.0 },
    { "strut", command_strut,            0.0 },
    { "thank", command_thank,            0.0 },
    { "twiddle", command_twiddle,        0.0 },
    { "wave", command_wave,              0.0 },
    { "whistle", command_whistle,        0.0 },
    { "wink", command_wink,              0.0 },
    { "yawn", command_yawn,              0.0 },
    { "beg", command_beg,                0.0 },
    { "bleed", command_bleed,            0.0 },
    { "cringe", command_cringe,          0.0 },
    { "think", command_think,            0.0 }
};

/** Length of the ::CommunicationCommands array. */
const int CommunicationCommandSize = sizeof(CommunicationCommands)/sizeof(command_array_struct);

/** Wizard commands. */
command_array_struct WizCommands [] = {
    { "abil", command_abil,                      0.0 },
    { "addexp", command_addexp,                  0.0 },
    { "archs", command_archs,                    0.0 },
    { "arrest", command_arrest,                  0.0 },
    { "banish", command_banish,                  0.0 },
    { "create", command_create,                  0.0 },
    { "debug", command_debug,                    0.0 },
    { "diff", command_diff,                      0.0 },
    { "dmtell", command_dmtell,                  0.0 },
    { "dump", command_dump,                      0.0 },
    { "dumpbelow", command_dumpbelow,            0.0 },
    { "dumpfriendlyobjects", command_dumpfriendlyobjects, 0.0 },
    { "dumpallarchetypes", command_dumpallarchetypes, 0.0 },
    { "dumpallmaps", command_dumpallmaps,        0.0 },
    { "dumpallobjects", command_dumpallobjects,  0.0 },
    { "dumpmap", command_dumpmap,                0.0 },
    { "follow", command_follow,                  0.0 },
    { "forget_spell", command_forget_spell,      0.0 },
    { "free", command_free,                      0.0 },
    { "freeze", command_freeze,                  0.0 },
    { "goto", command_goto,                      0.0 },
    { "hide", command_hide,                      0.0 },
    { "insert_into", command_insert_into,        0.0 },
    { "invisible", command_invisible,            0.0 },
    { "kick", command_kick,                      0.0 },
    { "learn_special_prayer", command_learn_special_prayer, 0.0 },
    { "learn_spell", command_learn_spell,        0.0 },
    { "malloc", command_malloc,                  0.0 },
    { "nodm", command_nowiz,                     0.0 },
    { "nowiz", command_nowiz,                    0.0 },
    { "patch", command_patch,                    0.0 },
    { "players", command_players,                0.0 },
    { "plugin", command_loadplugin,              0.0 },
    { "pluglist", command_listplugins,           0.0 },
    { "plugout", command_unloadplugin,           0.0 },
    { "purge_quest_state", command_purge_quest,  0.0 },
    { "purge_quests", command_purge_quest_definitions,  0.0 },
    { "remove", command_remove,                  0.0 },
    { "reset", command_reset,                    0.0 },
    { "set_god", command_setgod,                 0.0 },
    { "settings", command_settings,              0.0 },
    { "server_speed", command_speed,             0.0 },
    { "shutdown", command_shutdown,              0.0 },
    { "ssdumptable", command_ssdumptable,        0.0 },
    { "stack_clear", command_stack_clear,        0.0 },
    { "stack_list", command_stack_list,          0.0 },
    { "stack_pop", command_stack_pop,            0.0 },
    { "stack_push", command_stack_push,          0.0 },
    { "stats", command_stats,                    0.0 },
    { "strings", command_strings,                0.0 },
    { "style_info", command_style_map_info,      0.0 },        /* Costly command, so make it wiz only */
    { "summon", command_summon,                  0.0 },
    { "teleport", command_teleport,              0.0 },
    { "toggle_shout", command_toggle_shout,      0.0 },
    { "wizpass", command_wizpass,                0.0 },
    { "wizcast", command_wizcast,                0.0 },
    { "overlay_save", command_overlay_save,      0.0 },
    { "overlay_reset", command_overlay_reset,    0.0 },
/*    { "possess", command_possess, 0.0 }, */
    { "mon_aggr", command_mon_aggr,              0.0 },
    { "loadtest", command_loadtest,              0.0 },
};
/** Length of ::WizCommands array. */
const int WizCommandsSize = sizeof(WizCommands)/sizeof(command_array_struct);

/**
 * Comparison function for 2 command_array_struct.
 * @param a
 * @param b
 * commands to compare.
 * @retval -1
 * a is less than b.
 * @retval 0
 * a equals b.
 * @retval 1
 * a is greater than b.
 */
static int compare_A(const void *a, const void *b) {
    return strcmp(((const command_array_struct *)a)->name, ((const command_array_struct *)b)->name);
}

/**
 * Sorts the command arrays for easy search through bsearch().
 */
void init_commands(void) {
    qsort(Commands, CommandsSize, sizeof(command_array_struct), compare_A);
    qsort(CommunicationCommands, CommunicationCommandSize, sizeof(command_array_struct), compare_A);
    qsort(WizCommands, WizCommandsSize, sizeof(command_array_struct), compare_A);
}
