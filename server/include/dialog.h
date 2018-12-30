#ifndef DIALOG_H
#define DIALOG_H

#include "global.h"

/**
 * Various kind of messages a player or NPC can say.
 */
typedef enum {
    rt_say = 0,         /**< Basic sentence. */
    rt_reply = 1,       /**< Reply to something. */
    rt_question = 2     /**< Asking a question. */
} reply_type;

/**
 * One reply a NPC can expect.
 */
typedef struct struct_dialog_reply {
    char *reply;                        /**< Reply expected from the player. */
    char *message;                      /**< What the player will actually say for this reply. */
    reply_type type;                    /**< Type of message. */
    struct struct_dialog_reply *next;   /**< Next reply, NULL for last. */
} struct_dialog_reply;

/**
 * One message a NPC can react to.
 */
typedef struct struct_dialog_message {
    char *match;                                /**< What the player should say, can be a regexp. */
    char *message;                              /**< What the NPC will say. */
    struct struct_dialog_reply *replies;        /**< Replies this message has. */
    struct struct_dialog_message *next;         /**< Next message, NULL if last. */
} struct_dialog_message;

/**
 * Message information for a NPC.
 */
typedef struct struct_dialog_information {
    struct struct_dialog_reply *all_replies;    /**< All replies, to quickly search things. */
    struct struct_dialog_message *all_messages; /**< Messages the NPC can use. */
} struct_dialog_information;

/** How many NPC replies maximum to tell the player. */
#define MAX_REPLIES 10
/** How many NPCs maximum will reply to the player. */
#define MAX_NPC     5
/**
 * Structure used to build up dialog information when a player says something.
 * @sa monster_communicate().
 */
typedef struct talk_info {
    struct obj *who;                    /**< Player saying something. */
    const char *text;                   /**< What the player actually said. */
    sstring message;                    /**< If not NULL, what the player will be displayed as said. */
    int message_type;                   /**< A reply_type value for message. */
    int replies_count;                  /**< How many items in replies_words and replies. */
    sstring replies_words[MAX_REPLIES]; /**< Available reply words. */
    sstring replies[MAX_REPLIES];       /**< Description for replies_words. */
    int npc_msg_count;                  /**< How many NPCs reacted to the text being said. */
    sstring npc_msgs[MAX_NPC];          /**< What the NPCs will say. */
} talk_info;


#endif /* DIALOG_H */
