
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
 * Structures and functions used for the @ref page_dialog "dialog system".
*/

#include <string.h>
#include "global.h"
#include "define.h"
#include "object.h"
#include "dialog.h"

/**
 * Frees obj::dialog_information.
 * @param op what to clean for.
 */
void free_dialog_information(object *op) {
    struct_dialog_message *current, *next;
    struct_dialog_reply *currep, *nextrep;

    if (!QUERY_FLAG(op, FLAG_DIALOG_PARSED))
        return;

    CLEAR_FLAG(op, FLAG_DIALOG_PARSED);
    if (!op->dialog_information)
        return;

    current = op->dialog_information->all_messages;
    while (current) {
        next = current->next;
        free(current->match);
        free(current->message);
        currep = current->replies;
        while (currep) {
            nextrep = currep->next;
            free(currep->reply);
            free(currep->message);
            currep = nextrep;
        }
        free(current);
        current = next;
    }

    currep = op->dialog_information->all_replies;
    while (currep) {
        nextrep = currep->next;
        free(currep->reply);
        free(currep->message);
        free(currep);
        currep = nextrep;
    }

    free(op->dialog_information);
    op->dialog_information = NULL;
}

/**
 * Does the text match the expression?
 * @param exp expression to try to match.
 * @param text what to test.
 * @return 1 if match, 0 else.
 * @todo better * handling (incorrect now, will match even if trailing chars)
 */
static int matches(const char *exp, const char *text) {
    char *pipe, *save, *msg;
    int match = 0;

    if (exp[0] == '*')
        return 1;

    msg = strdup(exp);

    pipe = strtok_r(msg, "|", &save);
    while (pipe) {
        if (re_cmp(text, pipe)) {
            match = 1;
            break;
        }
        pipe = strtok_r(NULL, "|", &save);
    }

    free(msg);
    return match;
}

/**
 * Parse the dialog information for op, and fills in obj::dialog_information.
 * Can be called safely multiple times (will just ignore the other calls).
 *
 * @param op object to parse the obj::msg field.
 */
static void parse_dialog_information(object *op) {
    struct_dialog_message *message = NULL, *last = NULL;
    struct_dialog_reply *reply = NULL;
    char *current, *save, *msg, *cp;
    int len;
    /* Used for constructing message with */
    char *tmp = NULL;
    size_t tmplen = 0;

    if (QUERY_FLAG(op, FLAG_DIALOG_PARSED))
        return;
    SET_FLAG(op, FLAG_DIALOG_PARSED);

    op->dialog_information = (struct_dialog_information *)calloc(1, sizeof(struct_dialog_information));
    if (op->dialog_information == NULL)
        fatal(OUT_OF_MEMORY);

    if (!op->msg)
        return;

    msg = strdup(op->msg);
    current = strtok_r(msg, "\n", &save);

    while (current) {
        if (strncmp(current, "@match ", 7) == 0) {
            if (message) {
                message->message = tmp;
                tmp = NULL;
                tmplen = 0;
            }

            message = (struct_dialog_message *)calloc(1, sizeof(struct_dialog_message));
            if (last)
                last->next = message;
            else
                op->dialog_information->all_messages = message;
            last = message;

            message->match = strdup(current+7);
        } else if ((strncmp(current, "@reply ", 7) == 0 && (len = 7)) || (strncmp(current, "@question ", 10) == 0 && (len = 10))) {
            if (message) {
                reply = (struct_dialog_reply *)calloc(1, sizeof(struct_dialog_reply));
                reply->type = (len == 7 ? rt_reply : rt_question);
                cp = strchr(current+len, ' ');
                if (cp) {
                    *cp = '\0';
                    reply->reply = strdup(current+len);
                    reply->message = strdup(cp+1);
                } else {
                    reply->reply = strdup(current+len);
                    reply->message = strdup(reply->reply);
                    LOG(llevDebug, "Warning: @reply/@question without message for %s!\n", op->name);
                }
                reply->next = message->replies;
                message->replies = reply;

                reply = (struct_dialog_reply *)calloc(1, sizeof(struct_dialog_reply));
                reply->reply = strdup(message->replies->reply);
                reply->message = strdup(message->replies->message);
                reply->type = message->replies->type;
                reply->next = op->dialog_information->all_replies;
                op->dialog_information->all_replies = reply;
            } else
                LOG(llevDebug, "Warning: @reply not in @match block for %s!\n", op->name);
        } else if (message) {
            /* Needed to set initial \0 */
            int wasnull = FALSE;
            tmplen += strlen(current)+2;
            if (!tmp)
                wasnull = TRUE;
            tmp = realloc(tmp, tmplen*sizeof(char));
            if (!tmp)
                fatal(OUT_OF_MEMORY);
            if (wasnull)
                tmp[0] = 0;
            strncat(tmp, current, tmplen-strlen(tmp)-1);
            strncat(tmp, "\n", tmplen-strlen(tmp)-1);
        }
        current = strtok_r(NULL, "\n", &save);
    }

    if (message) {
        if (!tmp)
            message->message = strdup("");
        else
            message->message = tmp;
        tmp = NULL;
        tmplen = 0;
    }

    free(msg);
}

/**
 * Tries to find a message matching the said text.
 * @param op who is being talked to.
 * @param text what is being said.
 * @param[out] message what op should say. Won't be NULL if return is 1.
 * @param[out] reply text the one talking should say based on the text. Can be NULL.
 * @return 0 if no match, 1 if a message did match the text.
 * @todo smarter match, try to find exact before joker (*) one.
 */
int get_dialog_message(object *op, const char *text, struct_dialog_message **message, struct_dialog_reply **reply) {
    if (!QUERY_FLAG(op, FLAG_DIALOG_PARSED))
        parse_dialog_information(op);

    for (*message = op->dialog_information->all_messages; *message; *message = (*message)->next) {
        if (matches((*message)->match, text)) {
            break;
        }
    }
    if (!*message)
        return 0;

    for (*reply = op->dialog_information->all_replies; *reply; *reply = (*reply)->next) {
        if (strcmp((*reply)->reply, text) == 0)
            break;
    }

    return 1;
}
