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
 * All player communication commands, except the 'gsay' one.
 */

#include <global.h>
#include <loader.h>
#include <sproto.h>

/**
 * 'say' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_say(object *op, const char *params) {
    if (*params == '\0')
        return;
    monster_communicate(op, params);
}

/**
 * 'me' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_me(object *op, const char *params) {
    char buf[MAX_BUF];

    if (*params == '\0')
        return;
    snprintf(buf, sizeof(buf), "%s %s", op->name, params);
    ext_info_map(NDI_UNIQUE|NDI_BLUE, op->map, MSG_TYPE_COMMUNICATION, MSG_TYPE_COMMUNICATION_ME,
                 buf);
}

/**
 * 'cointoss' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_cointoss(object *op, const char *params) {
    char buf[MAX_BUF];
    const char *result;

    result = rndm(1, 2) == 1 ? "Heads" : "Tails";

    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMUNICATION, MSG_TYPE_COMMUNICATION_RANDOM,
        "You flip a coin.... %s!",
        result);

    snprintf(buf, sizeof(buf), "%s flips a coin.... %s!", op->name, result);
    ext_info_map_except(NDI_WHITE, op->map, op, MSG_TYPE_COMMUNICATION, MSG_TYPE_COMMUNICATION_RANDOM,
        buf);
}

/** Results for the "orcknucle" game. */
static const char *const orcknuckle[7] = {
    "none",
    "beholder",
    "ghost",
    "knight",
    "princess",
    "dragon",
    "orc"
};

#define DICE    4 /**< How many dice to roll for orcknuckle. */

/**
 * Plays the "orcknucke" game.
 *
 * If there is an "dice" archetype in server arches, this command will
 * require the player to have at least 4 dice to play. There is a 5%
 * chance to lose one dice at each play. Dice can be made through alchemy
 * (finding the recipe is left as an exercice to the player).
 * Note that the check is on the name 'dice', so you can have multiple
 * archetypes for that name, they'll be all taken into account.
 *
 * @param op
 * player who plays.
 * @param params
 * string sent by the player. Ignored.
 */
void command_orcknuckle(object *op, const char *params) {
    char buf[MAX_BUF];
    char buf2[MAX_BUF];
    object *dice[DICE];
    int i, j, k, l, dice_count, number_dice;
    const char *name;

    /* We only use dice if the archetype is present ingame. */
    name = find_string("dice");
    if (name) {
        for (dice_count = 0; dice_count < DICE; dice_count++)
            dice[dice_count] = NULL;
        dice_count = 0;
        number_dice = 0;

        FOR_INV_PREPARE(op, ob) {
            if (dice_count >= DICE || number_dice >= DICE)
                break;
            if (ob->name == name) {
                number_dice += ob->nrof;
                dice[dice_count++] = ob;
            }
        } FOR_INV_FINISH();

        if (number_dice < DICE) {
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMUNICATION, MSG_TYPE_COMMUNICATION_RANDOM,
                                 "You need at least %d dice to play orcknuckle!", DICE);
            return;
        }
    } else {
        dice_count = 0;
    }

    i = rndm(1, 5);
    j = rndm(1, 5);
    k = rndm(1, 5);
    l = rndm(1, 6);

    snprintf(buf2, sizeof(buf2), "%s rolls %s, %s, %s, %s!", op->name, orcknuckle[i], orcknuckle[j], orcknuckle[k], orcknuckle[l]);
    snprintf(buf, sizeof(buf), "You roll %s, %s, %s, %s!", orcknuckle[i], orcknuckle[j], orcknuckle[k], orcknuckle[l]);

    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMUNICATION, MSG_TYPE_COMMUNICATION_RANDOM,
                  buf);
    ext_info_map_except(NDI_UNIQUE, op->map, op, MSG_TYPE_COMMUNICATION, MSG_TYPE_COMMUNICATION_RANDOM,
                        buf2);

    if (name) {
        /* Randomly lose dice */
        if (die_roll(1, 100, op, 1) < 5) {
            /* Lose one randomly. */
            object_decrease_nrof_by_one(dice[rndm(1, dice_count)-1]);
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMUNICATION, MSG_TYPE_COMMUNICATION_RANDOM,
                          "Oops, you lost a die!");
        }
    }
#undef DICE
}

/**
 * Utility function for chat or shout.
 *
 * @param op
 * player.
 * @param params
 * message.
 * @param pri
 * message priority.
 * @param color
 * message color.
 * @param subtype
 * message subtype.
 * @param desc
 * 'chat' or 'shouts', will be appened after the player's name and before a :.
 */
static void command_tell_all(object *op, const char *params, int pri, int color, int subtype, const char *desc) {
    if (op->contr->no_shout == 1) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "You are no longer allowed to shout or chat.");
        return;
    }

    if (*params == '\0') {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Shout/Chat what?");
        return;
    }

    draw_ext_info_format(NDI_UNIQUE|NDI_ALL|color, pri, NULL, MSG_TYPE_COMMUNICATION, subtype,
                         "%s %s: %s",
                         op->name, desc, params);

    /* Lauwenmark : Here we handle the SHOUT global event */
    execute_global_event(EVENT_SHOUT, op, params, pri);
}

/**
 * 'shout' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_shout(object *op, const char *params) {
    command_tell_all(op, params, 1, NDI_RED, MSG_TYPE_COMMUNICATION_SHOUT, "shouts");
}

/**
 * 'chat' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_chat(object *op, const char *params) {
    command_tell_all(op, params, 9, NDI_BLUE, MSG_TYPE_COMMUNICATION_CHAT, "chats");
}

/**
 * Actual function sending a private message.
 *
 * @param op
 * player trying to tell something to someone.
 * @param params
 * who to tell, and message
 * @param adjust_listen
 * if non-zero, recipient can't ignore the message through 'listen' levels.
 */
static void do_tell(object *op, const char *params, int adjust_listen) {
    char buf[MAX_BUF], name[MAX_BUF];
    char *msg = NULL;
    player *pl;
    uint8 original_listen;

    snprintf(name, sizeof(name), "%s", params);

    msg = strchr(name, ' ');
    if (msg) {
        (*msg) = '\0';
        msg++;
        if ((*msg) == '\0')
            msg = NULL;
    }

    if (strlen(name) == 0) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Tell whom what?");
        return;
    }

    if (msg == NULL) {
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                             "Tell %s what?",
                             name);
        return;
    }

    snprintf(buf, sizeof(buf), "%s tells you: %s", op->name, msg);

    pl = find_player_partial_name(name);
    if (pl) {
        if (adjust_listen) {
            original_listen = pl->listening;
            pl->listening = 10;
        } else {
            original_listen = 0;
        }

        execute_global_event(EVENT_TELL, op, msg, pl->ob);

        draw_ext_info(NDI_UNIQUE|NDI_ORANGE, 0, pl->ob, MSG_TYPE_COMMUNICATION, MSG_TYPE_COMMUNICATION_TELL,
                      buf);

        if (adjust_listen)
            pl->listening = original_listen;

        /* Update last_tell value [mids 01/14/2002] */
        snprintf(pl->last_tell, sizeof(pl->last_tell), "%s", op->name);

        /* Hidden DMs get the message, but player should think DM isn't online. */
        if (!pl->hidden || QUERY_FLAG(op, FLAG_WIZ)) {
            draw_ext_info_format(NDI_UNIQUE|NDI_ORANGE, 0, op, MSG_TYPE_COMMUNICATION, MSG_TYPE_COMMUNICATION_TELL,
                                 "You tell %s: %s",
                                 pl->ob->name, msg);

            return;
        }
    }

    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                  "No such player or ambiguous name.");
}

/**
 * Private communication.
 *
 * @param op
 * player trying to tell something to someone.
 * @param params
 * who to tell, and message.
 */
void command_tell(object *op, const char *params) {
    do_tell(op, params, 0);
}

/**
 * Private communication, by a DM (can't be ignored by player).
 *
 * @param op
 * player trying to tell something to someone.
 * @param params
 * who to tell, and message.
 */
void command_dmtell(object *op, const char *params) {
    do_tell(op, params, 1);
}

/**
 * Reply to last person who told you something [mids 01/14/2002]
 *
 * Must have been told something by someone first.
 *
 * @param op
 * who is telling.
 * @param params
 * message to say.
 * @return
 * 1.
 */
void command_reply(object *op, const char *params) {
    player *pl;

    if (*params == '\0') {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Reply what?");
        return;
    }

    if (op->contr->last_tell[0] == '\0') {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "You can't reply to nobody.");
        return;
    }

    /* Find player object of player to reply to and check if player still exists */
    pl = find_player(op->contr->last_tell);
    if (pl == NULL) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "You can't reply, this player left.");
        return;
    }

    /* Update last_tell value */
    strcpy(pl->last_tell, op->name);

    draw_ext_info_format(NDI_UNIQUE|NDI_ORANGE, 0, pl->ob, MSG_TYPE_COMMUNICATION, MSG_TYPE_COMMUNICATION_TELL,
                         "%s tells you: %s",
                         op->name, params);

    if (pl->hidden && !QUERY_FLAG(op, FLAG_WIZ)) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "You can't reply, this player left.");
        return;
    }

    draw_ext_info_format(NDI_UNIQUE|NDI_ORANGE, 0, op, MSG_TYPE_COMMUNICATION, MSG_TYPE_COMMUNICATION_TELL,
                         "You tell to %s: %s",
                         pl->ob->name, params);
}

/**
 * @defgroup EMOTE_xxx Emotes players can use to communicate
 */
/*@{*/
#define EMOTE_FIRST     0
#define EMOTE_NOD       1
#define EMOTE_DANCE     2
#define EMOTE_KISS      3
#define EMOTE_BOUNCE    4
#define EMOTE_SMILE     5
#define EMOTE_CACKLE    6
#define EMOTE_LAUGH     7
#define EMOTE_GIGGLE    8
#define EMOTE_SHAKE     9
#define EMOTE_PUKE      10
#define EMOTE_GROWL     11
#define EMOTE_SCREAM    12
#define EMOTE_SIGH      13
#define EMOTE_SULK      14
#define EMOTE_HUG       15
#define EMOTE_CRY       16
#define EMOTE_POKE      17
#define EMOTE_ACCUSE    18
#define EMOTE_GRIN      19
#define EMOTE_BOW       20
#define EMOTE_CLAP      21
#define EMOTE_BLUSH     22
#define EMOTE_BURP      23
#define EMOTE_CHUCKLE   24
#define EMOTE_COUGH     25
#define EMOTE_FLIP      26
#define EMOTE_FROWN     27
#define EMOTE_GASP      28
#define EMOTE_GLARE     29
#define EMOTE_GROAN     30
#define EMOTE_HICCUP    31
#define EMOTE_LICK      32
#define EMOTE_POUT      33
#define EMOTE_SHIVER    34
#define EMOTE_SHRUG     35
#define EMOTE_SLAP      36
#define EMOTE_SMIRK     37
#define EMOTE_SNAP      38
#define EMOTE_SNEEZE    39
#define EMOTE_SNICKER   40
#define EMOTE_SNIFF     41
#define EMOTE_SNORE     42
#define EMOTE_SPIT      43
#define EMOTE_STRUT     44
#define EMOTE_THANK     45
#define EMOTE_TWIDDLE   46
#define EMOTE_WAVE      47
#define EMOTE_WHISTLE   48
#define EMOTE_WINK      49
#define EMOTE_YAWN      50
#define EMOTE_BEG       51
#define EMOTE_BLEED     52
#define EMOTE_CRINGE    53
#define EMOTE_THINK     54
#define EMOTE_LAST      55
/*@}*/

/**
 * Emote texts when the player does not specify who to apply the emote.
 * Two strings, first is sent to the player herself, second to other players (must contain %s).
 * The @ref EMOTE_xxx "emotes" will be seeked at the index of their value minus 1.
 * If an entry is NULL, then a default text will be used.
 */
static const char* single_emotes[EMOTE_LAST - 1][2] = {
    { "You nod solemnly.", "%s nods solemnly." },
    { "You dance with glee.", "%s expresses himself through interpretive dance." },
    { "All the lonely people...", "%s makes a weird facial contortion" },
    { "BOIINNNNNNGG!", "%s bounces around." },
    { "You smile happily.", "%s smiles happily." },
    { "You cackle gleefully.", "%s throws back his head and cackles with insane glee!" },
    { "You fall down laughing.", "%s falls down laughing." },
    { "You giggle.", "%s giggles." },
    { "You shake your head.", "%s shakes his head." },
    { "Bleaaaaaghhhhhhh!", "%s pukes." },
    { "Grrrrrrrrr....", "%s growls." },
    { "ARRRRRRRRRRGH!!!!!", "%s screams at the top of his lungs!" },
    { "You sigh.", "%s sighs loudly." },
    { "You sulk.", "%s sulks in the corner." },
    { NULL, NULL },
    { "Waaaaaaahhh...", "%s bursts into tears." },
    { NULL, NULL },
    { NULL, NULL },
    { "You grin evilly.", "%s grins evilly." },
    { "You bow deeply.", "%s bows deeply." },
    { "Clap, clap, clap.", "%s gives a round of applause." },
    { "Your cheeks are burning.", "%s blushes." },
    { "You burp loudly.", "%s burps loudly." },
    { "You chuckle politely", "%s chuckles politely." },
    { "Yuck, try to cover your mouth next time!", "%s coughs loudly." },
    { "You flip head over heels.", "%s flips head over heels." },
    { "What's bothering you?", "%s frowns." },
    { "You gasp in astonishment.", "%s gasps in astonishment." },
    { "You glare at nothing in particular.", "%s glares around him." },
    { "You groan loudly.", "%s groans loudly." },
    { "*HIC*", "%s hiccups." },
    { "You lick your mouth and smile.", "%s licks his mouth and smiles." },
    { "Aww, don't take it so hard.", "%s pouts." },
    { "Brrrrrrrrr.", "%s shivers uncomfortably." },
    { "You shrug.", "%s shrugs helplessly." },
    { NULL, NULL },
    { "You smirk.", "%s smirks." },
    { "PRONTO! You snap your fingers.", "%s snaps his fingers." },
    { "Gesundheit!", "%s sneezes." },
    { "You snicker softly.", "%s snickers softly." },
    { "You sniff sadly. *SNIFF*", "%s sniffs sadly." },
    { "Zzzzzzzzzzzzzzz.", "%s snores loudly." },
    { "You spit over your left shoulder.", "%s spits over his left shoulder." },
    { "Strut your stuff.", "%s struts proudly." },
    { NULL, NULL },
    { "%s patiently twiddles his thumbs.", "You patiently twiddle your thumbs." },
    { "You wave.", "%s waves happily." },
    { "You whistle appreciatively.", "%s whistles appreciatively." },
    { "Have you got something in your eye?", "%s winks suggestively." },
    { "You open up your yap and let out a big breeze of stale air.", "%s yawns sleepily." },
    { NULL, NULL },
    { "You bleed all over your nice new armour.", "%s is bleeding all over the carpet - got a spare tourniquet?" },
    { "You cringe in terror.", "%s cringes in terror!" },
    { "Anything in particular that you'd care to think about?", "%s closes his eyes and thinks really hard." },
};
/**
 * Emote texts when the player applies the emote to herself.
 * Two strings, first is sent to the player, second to other players (must contain %s).
 * The @ref EMOTE_xxx "emotes" will be seeked at the index of their value minus 1.
 * If an entry is NULL, then a default text will be used.
 */
static const char* self_emotes[EMOTE_LAST - 1][2] = {
    { "My god! is that LEGAL?", "You look away from %s." },
    { "You skip and dance around by yourself.", "%s embraces himself and begins to dance!"},
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { "Laugh at yourself all you want, the others won't understand." "%s is laughing at something." },
    { NULL, NULL },
    { "You are shaken by yourself.", "%s shakes and quivers like a bowlful of jelly." },
    { "You puke on yourself.", "%s pukes on his clothes." },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { "You hug yourself.", "%s hugs himself." },
    { "You cry to yourself.", "%s sobs quietly to himself." },
    { "You poke yourself in the ribs, feeling very silly.", "%s pokes himself in the ribs, looking very sheepish." },
    { "You accuse yourself.", "%s seems to have a bad conscience." },
    { NULL, NULL },
    { "You kiss your toes.", "%s folds up like a jackknife and kisses his own toes." },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { "You frown at yourself.", "%s frowns at himself." },
    { NULL, NULL },
    { "You glare icily at your feet, they are suddenly very cold.", "%s glares at his feet, what is bothering him?" },
    { NULL, NULL },
    { NULL, NULL },
    { "You lick yourself.", "%s licks himself - YUCK." },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { "You slap yourself, silly you.", "%s slaps himself, really strange..." },
    { NULL, NULL },
    { NULL, NULL },
    { "You sneeze on yourself, what a mess!", "%s sneezes, and covers himself in a slimy substance." },
    { NULL, NULL },
    { "You sniff yourself.", "%s sniffs himself." },
    { NULL, NULL },
    { "You drool all over yourself.", "%s drools all over himself." },
    { NULL, NULL },
    { "You thank yourself since nobody else wants to!", "%s thanks himself since you won't." },
    { NULL, NULL },
    { "Are you going on adventures as well??", "%s waves goodbye to himself."},
    { "You whistle while you work.", "%s whistles to himself in boredom." },
    { "You wink at yourself?? What are you up to?", "%s winks at himself - something strange is going on..." },
    { NULL, NULL },
    { NULL, NULL },
    { "Very impressive! You wipe your blood all over yourself.", "%s performs some satanic ritual while wiping his blood on himself." },
    { NULL, NULL },
    { NULL, NULL },
};
/**
 * Emote texts when the player applies the emote to someone else.
 * Three strings, first is sent to the player, second to the recipient, third to others.
 * The first two must contain %s (player's name), second must contain 2 %s (player
 * and victim's name).
 * The @ref EMOTE_xxx "emotes" will be seeked at the index of their value minus 1.
 * If an entry is NULL, then a default text will be used.
 */
static const char* other_emotes[EMOTE_LAST - 1][3] = {
    { "You nod solemnly to %s.", "%s nods solemnly to you.", "%s nods solemnly to %s." },
    { "You grab %s and begin doing the Cha-Cha!", "%s grabs you, and begins dancing!", "Yipe! %s and %s are doing the Macarena!" },
    { "You kiss %s.", "%s kisses you.", "%s kisses %s." },
    { "You bounce around the room with %s.", "%s bounces around the room with you.", "%s bounces around the room with %s." },
    { "You smile at %s.", "%s smiles at you.", "%s beams a smile at %s." },
    { NULL, NULL, NULL },
    { "You take one look at %s and fall down laughing.", "%s looks at you and falls down on the ground laughing.", "%s looks at %s and falls down on the ground laughing." },
    { NULL, NULL, NULL },
    { "You shake %s's hand.", "%s shakes your hand.", "%s shakes %s's hand." },
    { "You puke on %s.", "%s pukes on your clothes!", "%s pukes on %s." },
    { NULL, NULL, NULL },
    { NULL, NULL, NULL },
    { NULL, NULL, NULL },
    { NULL, NULL, NULL },
    { "You hug %s.", "%s hugs you.", "%s hugs %s." },
    { "You cry on %s's shoulder.", "%s cries on your shoulder.", "%s cries on %s's shoulder." },
    { "You poke %s in the ribs.", "%s pokes you in the ribs.", "%s pokes %s in the ribs." },
    { "You look accusingly at %s.", "%s looks accusingly at you.", "%s looks accusingly at %s." },
    { "You grin at %s.", "%s grins evilly at you.", "%s grins evilly at %s." },
    { "You bow before %s.", "%s bows before you.", "%s bows before %s." },
    { NULL, NULL, NULL },
    { NULL, NULL, NULL },
    { NULL, NULL, NULL },
    { NULL, NULL, NULL },
    { NULL, NULL, NULL },
    { NULL, NULL, NULL },
    { "You frown darkly at %s.", "%s frowns darkly at you.", "%s frowns darkly at %s." },
    { NULL, NULL, NULL },
    { "You glare icily at %s.", "%s glares icily at you, you feel cold to your bones.", "%s glares at %s." },
    { NULL, NULL, NULL },
    { NULL, NULL, NULL },
    { "You lick %s.", "%s licks you.", "%s licks %s." },
    { NULL, NULL, NULL },
    { NULL, NULL, NULL },
    { "You shrug at %s.", "%s shrugs at you.", "%s shrugs at %s." },
    { "You slap %s.", "You are slapped by %s.", "%s slaps %s." },
    { NULL, NULL, NULL },
    { NULL, NULL, NULL },
    { "You sneeze at %s and a film of snot shoots onto him.", "%s sneezes on you, you feel the snot cover you. EEEEEEW.", "%s sneezes on %s and a film of snot covers him." },
    { NULL, NULL, NULL },
    { "You sniff %s.", "%s sniffs you.", "%s sniffs %s" },
    { NULL, NULL, NULL },
    { "You spit on %s.", "%s spits in your face!", "%s spits in %s's face." },
    { NULL, NULL, NULL },
    { "You thank %s heartily.", "%s thanks you heartily.", "%s thanks %s heartily." },
    { NULL, NULL, NULL },
    { "You wave goodbye to %s.", "%s waves goodbye to you. Have a good journey.", "%s waves goodbye to %s." },
    { "You whistle at %s.", "%s whistles at you.", "%s whistles at %s." },
    { "You wink suggestively at %s.", "%s winks suggestively at you.", "%s winks at %s." },
    { NULL, NULL, NULL },
    { "You beg %s for mercy.", "%s begs you for mercy! Show no quarter!", "%s begs %s for mercy!" },
    { "You slash your wrist and bleed all over %s", "%s slashes his wrist and bleeds all over you.", "%s slashes his wrist and bleeds all over %s." },
    { "You cringe away from %s.", "%s cringes away from you.", "%s cringes away from %s in mortal terror." },
    { NULL, NULL, NULL },
};

/**
 * This function covers basic emotions a player can have.  An emotion can be
 * one of three things currently.  Directed at oneself, directed at someone,
 * or directed at nobody.  The first set is nobody, the second at someone, and
 * the third is directed at oneself.  Every emotion does not have to be
 * filled out in every category.  The default case will take care of the ones
 * that are not.  Helper functions will call basic_emote with the proper
 * arguments, translating them into commands.  Adding a new emotion can be
 * done by editing command.c and command.h.
 * [garbled 09-25-2001]
 *
 * @param op
 * player.
 * @param params
 * message.
 * @param emotion
 * emotion code, one of @ref EMOTE_xxx "EMOTE_xxx".
 * @todo simplify function (indexed array, for instance).
 */
static void basic_emote(object *op, const char *params, int emotion) {
    char buf[MAX_BUF], buf2[MAX_BUF], buf3[MAX_BUF];
    player *pl;

    if (*params == '\0') {
        if (emotion > EMOTE_FIRST && emotion < EMOTE_LAST && single_emotes[emotion - 1][0] != NULL) {
            snprintf(buf, sizeof(buf), single_emotes[emotion - 1][0]);
            snprintf(buf2, sizeof(buf2), single_emotes[emotion - 1][1], op->name);
        } else {
            snprintf(buf, sizeof(buf), "You are a nut.");
            snprintf(buf2, sizeof(buf2), "%s dances with glee.", op->name);
      }

      ext_info_map_except(NDI_WHITE, op->map, op, MSG_TYPE_COMMUNICATION, MSG_TYPE_COMMUNICATION_EMOTE,
                          buf2);
      draw_ext_info(NDI_UNIQUE|NDI_WHITE, 0, op, MSG_TYPE_COMMUNICATION, MSG_TYPE_COMMUNICATION_EMOTE,
                    buf);
        return;
    }

    pl = find_player_options(params, FIND_PLAYER_NO_HIDDEN_DM | FIND_PLAYER_PARTIAL_NAME, op->map);

    if (pl == NULL) {
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                             "%s is not around.",
                             params);
        return;
    }

    if (pl->ob == op) {
        /* Player doing self-emote. */
        if (emotion > EMOTE_FIRST && emotion < EMOTE_LAST && self_emotes[emotion - 1][0] != NULL) {
            snprintf(buf, sizeof(buf), "%s", self_emotes[emotion - 1][0]);
            snprintf(buf2, sizeof(buf2), self_emotes[emotion - 1][1], op->name);
        } else {
            snprintf(buf, sizeof(buf), "My god! is that LEGAL?");
            snprintf(buf2, sizeof(buf2), "You look away from %s.", op->name);
        }

        draw_ext_info(NDI_UNIQUE|NDI_WHITE, 0, op, MSG_TYPE_COMMUNICATION, MSG_TYPE_COMMUNICATION_EMOTE,
                    buf);
        ext_info_map_except(NDI_WHITE, op->map, op, MSG_TYPE_COMMUNICATION, MSG_TYPE_COMMUNICATION_EMOTE,
                          buf2);
      return;
    }

    if (emotion > EMOTE_FIRST && emotion < EMOTE_LAST && other_emotes[emotion - 1][0] != NULL) {
        snprintf(buf, sizeof(buf), other_emotes[emotion - 1][0], pl->ob->name);
        snprintf(buf2, sizeof(buf2), other_emotes[emotion - 1][1], op->name);
        snprintf(buf3, sizeof(buf3), other_emotes[emotion - 1][2], op->name, pl->ob->name);
    } else {
        snprintf(buf, sizeof(buf), "You are still nuts.");
        snprintf(buf2, sizeof(buf2), "You get the distinct feeling that %s is nuts.", op->name);
        snprintf(buf3, sizeof(buf3), "%s is eyeing %s quizzically.", pl->ob->name, op->name);
    }

    draw_ext_info(NDI_UNIQUE|NDI_WHITE, 0, op, MSG_TYPE_COMMUNICATION, MSG_TYPE_COMMUNICATION_EMOTE,
                  buf);
    draw_ext_info(NDI_UNIQUE|NDI_WHITE, 0, pl->ob, MSG_TYPE_COMMUNICATION, MSG_TYPE_COMMUNICATION_EMOTE,
                  buf2);
    ext_info_map_except2(NDI_WHITE, op->map, op, pl->ob, MSG_TYPE_COMMUNICATION, MSG_TYPE_COMMUNICATION_EMOTE,
                         buf3);
}

/*
 * everything from here on out are just wrapper calls to basic_emote
 */

/**
 * 'nod' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_nod(object *op, const char *params) {
    basic_emote(op, params, EMOTE_NOD);
}

/**
 * 'dance' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_dance(object *op, const char *params) {
    basic_emote(op, params, EMOTE_DANCE);
}

/**
 * 'kiss' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_kiss(object *op, const char *params) {
    basic_emote(op, params, EMOTE_KISS);
}

/**
 * 'bounce' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_bounce(object *op, const char *params) {
    basic_emote(op, params, EMOTE_BOUNCE);
}

/**
 * 'smile' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_smile(object *op, const char *params) {
    basic_emote(op, params, EMOTE_SMILE);
}

/**
 * 'cackle' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_cackle(object *op, const char *params) {
    basic_emote(op, params, EMOTE_CACKLE);
}

/**
 * 'laugh' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_laugh(object *op, const char *params) {
    basic_emote(op, params, EMOTE_LAUGH);
}

/**
 * 'giggle' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_giggle(object *op, const char *params) {
    basic_emote(op, params, EMOTE_GIGGLE);
}

/**
 * 'shake' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_shake(object *op, const char *params) {
    basic_emote(op, params, EMOTE_SHAKE);
}

/**
 * 'puke' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_puke(object *op, const char *params) {
    basic_emote(op, params, EMOTE_PUKE);
}

/**
 * 'growl' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_growl(object *op, const char *params) {
    basic_emote(op, params, EMOTE_GROWL);
}

/**
 * 'scream' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_scream(object *op, const char *params) {
    basic_emote(op, params, EMOTE_SCREAM);
}

/**
 * 'sigh' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_sigh(object *op, const char *params) {
    basic_emote(op, params, EMOTE_SIGH);
}

/**
 * 'sulk' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_sulk(object *op, const char *params) {
    basic_emote(op, params, EMOTE_SULK);
}

/**
 * 'hug' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_hug(object *op, const char *params) {
    basic_emote(op, params, EMOTE_HUG);
}

/**
 * 'cry' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_cry(object *op, const char *params) {
    basic_emote(op, params, EMOTE_CRY);
}

/**
 * 'poke' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_poke(object *op, const char *params) {
    basic_emote(op, params, EMOTE_POKE);
}

/**
 * 'accuse' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_accuse(object *op, const char *params) {
    basic_emote(op, params, EMOTE_ACCUSE);
}

/**
 * 'grin' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_grin(object *op, const char *params) {
    basic_emote(op, params, EMOTE_GRIN);
}

/**
 * 'bow' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_bow(object *op, const char *params) {
    basic_emote(op, params, EMOTE_BOW);
}

/**
 * 'clap' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_clap(object *op, const char *params) {
    basic_emote(op, params, EMOTE_CLAP);
}

/**
 * 'blush' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_blush(object *op, const char *params) {
    basic_emote(op, params, EMOTE_BLUSH);
}

/**
 * 'burp' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_burp(object *op, const char *params) {
    basic_emote(op, params, EMOTE_BURP);
}

/**
 * 'chuckle' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_chuckle(object *op, const char *params) {
    basic_emote(op, params, EMOTE_CHUCKLE);
}

/**
 * 'cough' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_cough(object *op, const char *params) {
    basic_emote(op, params, EMOTE_COUGH);
}

/**
 * 'flip' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_flip(object *op, const char *params) {
    basic_emote(op, params, EMOTE_FLIP);
}

/**
 * 'frown' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_frown(object *op, const char *params) {
    basic_emote(op, params, EMOTE_FROWN);
}

/**
 * 'gasp' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_gasp(object *op, const char *params) {
    basic_emote(op, params, EMOTE_GASP);
}

/**
 * 'glare' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_glare(object *op, const char *params) {
    basic_emote(op, params, EMOTE_GLARE);
}

/**
 * 'groan' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_groan(object *op, const char *params) {
    basic_emote(op, params, EMOTE_GROAN);
}

/**
 * 'hiccup' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_hiccup(object *op, const char *params) {
    basic_emote(op, params, EMOTE_HICCUP);
}

/**
 * 'lick' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_lick(object *op, const char *params) {
    basic_emote(op, params, EMOTE_LICK);
}

/**
 * 'pout' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_pout(object *op, const char *params) {
    basic_emote(op, params, EMOTE_POUT);
}

/**
 * 'shiver' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_shiver(object *op, const char *params) {
    basic_emote(op, params, EMOTE_SHIVER);
}

/**
 * 'shrug' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_shrug(object *op, const char *params) {
    basic_emote(op, params, EMOTE_SHRUG);
}

/**
 * 'slap' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_slap(object *op, const char *params) {
    basic_emote(op, params, EMOTE_SLAP);
}

/**
 * 'smirk' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_smirk(object *op, const char *params) {
    basic_emote(op, params, EMOTE_SMIRK);
}

/**
 * 'snap' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_snap(object *op, const char *params) {
    basic_emote(op, params, EMOTE_SNAP);
}

/**
 * 'sneeze' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_sneeze(object *op, const char *params) {
    basic_emote(op, params, EMOTE_SNEEZE);
}

/**
 * 'snicker' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_snicker(object *op, const char *params) {
    basic_emote(op, params, EMOTE_SNICKER);
}

/**
 * 'sniff' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_sniff(object *op, const char *params) {
    basic_emote(op, params, EMOTE_SNIFF);
}

/**
 * 'snore' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_snore(object *op, const char *params) {
    basic_emote(op, params, EMOTE_SNORE);
}

/**
 * 'spit' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_spit(object *op, const char *params) {
    basic_emote(op, params, EMOTE_SPIT);
}

/**
 * 'strut' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_strut(object *op, const char *params) {
    basic_emote(op, params, EMOTE_STRUT);
}

/**
 * 'thank' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_thank(object *op, const char *params) {
    basic_emote(op, params, EMOTE_THANK);
}

/**
 * 'twiddle' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_twiddle(object *op, const char *params) {
    basic_emote(op, params, EMOTE_TWIDDLE);
}

/**
 * 'wave' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_wave(object *op, const char *params) {
    basic_emote(op, params, EMOTE_WAVE);
}

/**
 * 'whistle' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_whistle(object *op, const char *params) {
    basic_emote(op, params, EMOTE_WHISTLE);
}

/**
 * 'wink' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_wink(object *op, const char *params) {
    basic_emote(op, params, EMOTE_WINK);
}

/**
 * 'yawn' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_yawn(object *op, const char *params) {
    basic_emote(op, params, EMOTE_YAWN);
}

/**
 * 'beg' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_beg(object *op, const char *params) {
    basic_emote(op, params, EMOTE_BEG);
}

/**
 * 'bleed' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_bleed(object *op, const char *params) {
    basic_emote(op, params, EMOTE_BLEED);
}

/**
 * 'cringe' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_cringe(object *op, const char *params) {
    basic_emote(op, params, EMOTE_CRINGE);
}

/**
 * 'think' command.
 * @param op
 * player.
 * @param params
 * message.
 */
void command_think(object *op, const char *params) {
    basic_emote(op, params, EMOTE_THINK);
}
