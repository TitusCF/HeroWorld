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
@file
Quest-related low-level mechanisms.

You should only need to call the public functions, all that are not static.

Data is loaded on a need-only basis - when a player quest state is queried or modified, data is read.
Also, free_quest() can be called to release memory without preventing quest operations after.

Write is done for each player whenever the state changes, to ensure data integrity.

@todo
- add protocol commands to send notifications instead of using draw_ext_info()

*/

#include <global.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#endif

/** Quest status that indicates a quest was completed and may be restarted. */
#define QC_CAN_RESTART -1

/** Information about a quest for a player. */
typedef struct quest_state {
    sstring code;               /**< Quest internal code. */
    int state;                  /**< State for the player. */
    int was_completed;          /**< Whether the quest was completed once or not, indepandently of the state. */
    int is_complete;            /**< Whether the quest is complete in the current playthrough */
    int sent_to_client;         /**< Whether this state was sent to the client or not. */
    struct quest_state *next;   /**< Next quest on the list. */
} quest_state;

/** Information about a player. */
typedef struct quest_player {
    sstring player_name;        /**< Player's name. */
    struct quest_state *quests; /**< Quests done or in progress. */
    struct quest_player *next;  /**< Next player on the list. */
} quest_player;

/** Player quest state. */
static quest_player *player_states = NULL;

/** One condition to automatically move to a quest step. */
typedef struct quest_condition {
    sstring quest_code;          /**< The quest that triggers the condition */
    int minstep;                  /**< The earliest step in the quest that triggers the condition,
                                    -1 means finished, 0 means not started */
    int maxstep;                  /**< The latest step that triggers the condition, to match,
                                        the stages must be between minstep and maxstep */
    struct quest_condition *next; /**< The next condition to check */
} quest_condition;

/** One step of a quest. */
typedef struct quest_step_definition {
    int step;                           /**< Step identifier. */
    sstring step_description;           /**< Step description to show player. */
    int is_completion_step:1;           /**< Whether this step completes the quest (1) or not (0) */
    struct quest_step_definition *next; /**< Next step. */
    quest_condition *conditions;        /**< The conditions that must be satisfied to trigger the step */
} quest_step_definition;

/** Definition of an in-game quest. */
typedef struct quest_definition {
    sstring quest_code;             /**< Quest internal code. */
    sstring quest_title;            /**< Quest title for player. */
    sstring quest_description;      /**< Quest longer description. */
    int quest_restart;              /**< If non zero, can be restarted. */
    int face;                       /**< Face associated with this quest. */
    uint32 client_code;             /**< The code used to communicate with the client, merely a unique index. */
    quest_step_definition *steps;   /**< Quest steps. */
    struct quest_definition *parent;/**< Parent for this quest, NULL if it is a 'top-level' quest */
    struct quest_definition *next;  /**< Next quest in the definition list. */
} quest_definition;

static int quests_loaded = 0;           /**< Did we already read the 'default.quests' file? */
static quest_definition *quests = NULL; /**< All known quests. */

/**
 * Allocate a quest_step_definition, will call fatal() if out of memory.
 * @return new structure.
 */
static quest_step_definition *quest_create_step(void) {
    quest_step_definition *step = calloc(1, sizeof(quest_step_definition));
    if (!step)
        fatal(OUT_OF_MEMORY);
    return step;
}

/**
 * Allocate a quest_condition, will call fatal() if out of memory.
 * @return new structure.
 */
static quest_condition *quest_create_condition(void) {
    quest_condition *cond = calloc(1, sizeof(quest_condition));
    if (!cond)
        fatal(OUT_OF_MEMORY);
    return cond;
}


/**
 * Allocate a quest_definition, will call fatal() if out of memory.
 * @return new structure.
 */
static quest_definition *quest_create_definition(void) {
    quest_definition *quest = calloc(1, sizeof(quest_definition));
    if (!quest)
        fatal(OUT_OF_MEMORY);
    return quest;
}

/**
 * Find a quest from its code. This is called by quest_get and also
 * used in the quest loading code
 * @param code quest to search.
 * @return quest, or NULL if no such quest.
 */
static quest_definition *quest_get_by_code(sstring code) {
    quest_definition *quest;

    quest = quests;
    while (quest) {
        if (quest->quest_code == code)
            return quest;

        quest = quest->next;
    }
    return NULL;
}

/**
 * @defgroup QUESTFILE_xxx Quest file parsing state.
 *
 * This is the parsing state when loading a file through load_quests_from_file().
 */
/*@{*/
#define QUESTFILE_NEXTQUEST 0   /**< Waiting for next quest definition. */
#define QUESTFILE_QUEST 1       /**< In a quest definition. */
#define QUESTFILE_QUESTDESC 2   /**< In a quest description. */
#define QUESTFILE_STEP 3        /**< In a quest step. */
#define QUESTFILE_STEPDESC 4    /**< In a quest step description. */
#define QUESTFILE_STEPCOND 5    /**< In a quest step conditions. */
/*@}*/

/**
 * Loads all of the quests which are found in the given file, any global states
 * for quest loading are passed down into this function, but not back up again.
 * @param filename filename to load quests from.
 * @return number of quests loaded from file, negative value if there was an error.
 */
static int load_quests_from_file(const char *filename) {
    int i, in = QUESTFILE_NEXTQUEST, condition_parsed;
    int minstep, maxstep;
    char namedquest[MAX_BUF];
    quest_definition *quest = NULL;
    quest_condition *cond = NULL;
    char includefile[MAX_BUF];
    sstring questname;
    quest_step_definition *step = NULL;
    char final[MAX_BUF], read[MAX_BUF];
    FILE *file;
    StringBuffer *buf;

    int loaded_quests =0, found =0;
    snprintf(final, sizeof(final), "%s/%s/%s", settings.datadir, settings.mapdir, filename);
    file = fopen(final, "r");
    if (!file) {
        LOG(llevError, "Can't open %s for reading quests", filename);
        return -1;
    }

    while (fgets(read, sizeof(read), file) != NULL) {
        if (in == QUESTFILE_STEPCOND) {
            if (strcmp(read, "end_setwhen\n") == 0) {
                in = QUESTFILE_STEP;
                continue;
            }
            /* we are reading in a list of conditions for the 'setwhen' block for a quest step
             * There will be one entry per line, containing the quest, and the steps that it applies to.
             * This may be expressed as one of the following
             * questcode 20 (the quest questcode must be at step 20)
             * questcode <=20 (the quest questcode must not be beyond step 20)
             * questcode 10-20 (the quest questcode must be between steps 10 and 20)
             * questcode finished (the quest questcode must have been completed)
             */

            minstep = 0;
            maxstep = 0;
            condition_parsed = 0;
            namedquest[0]='\0';
            if (sscanf(read, "%s %d-%d\n", namedquest, &minstep, &maxstep)!=3) {
                if (sscanf(read, "%s <=%d\n", namedquest, &maxstep)== 2) {
                    minstep=0;
                    condition_parsed =1;
                } else if (sscanf(read, "%s %d\n", namedquest, &minstep)==2) {
                    maxstep = minstep;
                    condition_parsed =1;
                } else if (strstr(read, "finished")) {
                    if (sscanf(read, "%s finished\n", namedquest)==1) {
                        minstep = maxstep = -1;
                        condition_parsed =1;
                    }
                }
            } else
                condition_parsed =1;
            if (!condition_parsed) {
                LOG(llevError, "Invalid line '%s' in setwhen block for quest %s", read, quest->quest_code);
                continue;
            }

            cond = quest_create_condition();
            cond->minstep = minstep;
            cond->maxstep = maxstep;
            cond->quest_code = add_string(namedquest);
            cond->next = step->conditions;
            step->conditions = cond;
            LOG(llevDebug, "condition added for step %d of quest %s, looking for quest %s between steps %d and %d\n",
                    step->step, quest->quest_code, cond->quest_code, cond->minstep, cond->maxstep);
            continue;
        }
        if (in == QUESTFILE_STEPDESC) {
            if (strcmp(read, "end_description\n") == 0) {
                char *message;

                in = QUESTFILE_STEP;

                message = stringbuffer_finish(buf);
                buf = NULL;

                step->step_description = add_string(message);
                free(message);

                continue;
            }

            stringbuffer_append_string(buf, read);
            continue;
        }

        if (in == QUESTFILE_STEP) {
            if (strcmp(read, "end_step\n") == 0) {
                step = NULL;
                in = QUESTFILE_QUEST;
                continue;
            }
            if (strcmp(read, "finishes_quest\n") == 0) {
                step->is_completion_step=1;
                continue;
            }
            if (strcmp(read, "description\n") == 0) {
                buf = stringbuffer_new();
                in = QUESTFILE_STEPDESC;
                continue;
            }
            if (strcmp(read, "setwhen\n") == 0) {
                in = QUESTFILE_STEPCOND;
                continue;
            }
            LOG(llevError, "quests: invalid line %s in definition of quest %s in file %s!\n",
                    read, quest->quest_code, filename);
            continue;
        }

        if (in == QUESTFILE_QUESTDESC) {
            if (strcmp(read, "end_description\n") == 0) {
                char *message;

                in = QUESTFILE_QUEST;

                message = stringbuffer_finish(buf);
                buf = NULL;

                quest->quest_description = add_string(message);
                free(message);

                continue;
            }
            stringbuffer_append_string(buf, read);
            continue;
        }

        if (in == QUESTFILE_QUEST) {
            if (strcmp(read, "end_quest\n") == 0) {
                quest = NULL;
                in = QUESTFILE_NEXTQUEST;
                continue;
            }

            if (strcmp(read, "description\n") == 0) {
                in = QUESTFILE_QUESTDESC;
                buf = stringbuffer_new();
                continue;
            }

            if (strncmp(read, "title ", 6) == 0) {
                read[strlen(read) - 1] = '\0';
                quest->quest_title = add_string(read + 6);
                continue;
            }

            if (sscanf(read, "step %d\n", &i)) {
                step = quest_create_step();
                step->step = i;
                step->next = quest->steps;
                quest->steps = step;
                in = QUESTFILE_STEP;
                continue;
            }

            if (sscanf(read, "restart %d\n", &i)) {
                quest->quest_restart = i;
                continue;
            }
            if (strncmp(read, "parent ", 7) == 0) {
                read[strlen(read) - 1] = '\0';
                questname = add_string(read + 7);
                if (!quest_get_by_code(questname)) {
                    LOG(llevError, "Quest %s lists %s, as a parent, but this hasn't been defined\n", quest->quest_code, questname);
                } else {
                    quest->parent = quest_get_by_code(questname);
                }
                free_string(questname);
                continue;
            }

            if (strncmp(read, "face ", 5) == 0) {
                int face;
                read[strlen(read) - 1] = '\0';
                face = find_face(read + 5, 0);
                if (face == 0) {
                    LOG(llevError, "Quest %s has invalid face %s.\n", quest->quest_code, read + 5);
                } else {
                    quest->face = face;
                }
                continue;
            }
        }

        if (read[0] == '#')
            continue;

        if (strncmp(read, "quest ", 6) == 0) {
            quest = quest_create_definition();
            read[strlen(read) - 1] = '\0';
            quest->quest_code = add_string(read + 6);
            if (quest_get_by_code(quest->quest_code)) {
                LOG(llevError, "Quest %s is listed in file %s, but this quest has already been defined\n", quest->quest_code, filename);
            }
            /* Set a default face, which will be overwritten if a face is defined. */
            quest->face = find_face("quest_generic.111", 0);
            quest->next = quests;
            if (quests != NULL)
                quest->client_code = quests->client_code + 1;
            else
                quest->client_code = 1;
            quests = quest;
            in = QUESTFILE_QUEST;
            loaded_quests++;
            continue;
        }
        if (sscanf(read, "include %s\n", includefile)) {
            char inc_path[HUGE_BUF];
            path_combine_and_normalize(filename, includefile, inc_path, sizeof(inc_path));
            found = load_quests_from_file(inc_path);
            if (found >=0) {
                LOG(llevDebug, "loaded %d quests from file %s\n", found, inc_path);
                loaded_quests += found;
            } else {
                LOG(llevError, "Failed to load quests from file %s\n", inc_path);
            }
            continue;
        }

        if (strcmp(read, "\n") == 0)
            continue;

        LOG(llevError, "quest: invalid file format for %s, I don't know what to do with the line %s\n", final, read);
    }

    fclose(file);

    if (in != 0)
        LOG(llevError, "quest: quest definition file %s read in, ends with state %d\n", final, in);

    return loaded_quests;
}

/** Load all quest definitions. Can be called multiple times, will be ignored. */
static void quest_load_definitions(void) {
    int found = 0;
    if (quests_loaded)
        return;
    quests_loaded = 1;
    found = load_quests_from_file("world.quests");
    if (found >= 0) {
        LOG(llevInfo, "%d quests found.\n", found);
    } else {
        LOG(llevError, "Quest Loading Failed");
    }
}

/**
 * Get a step for the specified quest.
 * @param quest quest to consider.
 * @param step step to find.
 * @return step, or NULL if no such step in which case a llevError is emitted.
 */
static quest_step_definition *quest_get_step(quest_definition *quest, int step) {
    quest_step_definition *qsd = quest->steps;

    while (qsd) {
        if (qsd->step == step)
            return qsd;

        qsd = qsd->next;
    }

    LOG(llevError, "quest %s has no required step %d\n", quest->quest_code, step);
    return NULL;
}

/**
 * Find a quest from its code.
 * @param code quest to search.
 * @return quest, , or NULL if no such quest in which case a llevError is emitted.
 */
static quest_definition *quest_get(sstring code) {
    quest_definition *quest;

    quest_load_definitions();

    quest = quest_get_by_code(code);
    if (!quest) {
        LOG(llevError, "quest %s required but not found!\n", code);
        return NULL;
    }
    return quest;
}

/**
 * Return a new quest_state*, calling fatal() if memory shortage.
 * @return new value, never NULL.
 */
static quest_state *get_new_quest_state(void) {
    quest_state *qs = calloc(1, sizeof(quest_state));
    if (qs == NULL)
        fatal(OUT_OF_MEMORY);
    return qs;
}

/**
 * Read quest-data information for a player.
 * @param pq player to read data for.
 */
static void quest_read_player_data(quest_player *pq) {
    FILE *file;
    char final[MAX_BUF], read[MAX_BUF], data[MAX_BUF];
    StringBuffer *buf = NULL;
    quest_state *qs = NULL, *prev = NULL;
    int warned = 0, state;
    quest_definition *quest = NULL;

    /* needed, so we can check ending steps. */
    quest_load_definitions();

    snprintf(final, sizeof(final), "%s/%s/%s/%s.quest", settings.localdir, settings.playerdir, pq->player_name, pq->player_name);

    file = fopen(final, "r");
    if (!file) {
        /* no quest yet, no big deal */
        return;
    }

    while (fgets(read, sizeof(read), file) != NULL) {
        if (sscanf(read, "quest %s\n", data)) {
            qs = get_new_quest_state();
            qs->code = add_string(data);
            quest = quest_get_by_code(qs->code);
            continue;
        }

        if (!qs) {
            if (!warned)
                LOG(llevError, "quest: invalid file format for %s\n", final);
            warned = 1;
            continue;
        }

        if (sscanf(read, "state %d\n", &state)) {
            qs->state = state;
            if (quest != NULL && state != -1) {
                quest_step_definition *step = quest_get_step(quest, state);
                if (step == NULL) {
                    LOG(llevError, "invalid quest step %d for %s in %s", state, quest->quest_code, final);
                }
                else if (step->is_completion_step)
                    qs->is_complete = 1;
            }
            continue;
        }
        if (strcmp(read, "end_quest\n") == 0) {
            if (quest == NULL) {
                LOG(llevDebug, "Unknown quest %s in quest file %s", qs->code, final);
                free(qs);
            } else {
                if (prev == NULL) {
                    pq->quests = qs;
                } else {
                    prev->next = qs;
                }
                prev = qs;
            }
            qs = NULL;
            continue;
        }
        if (sscanf(read, "completed %d\n", &state)) {
            qs->was_completed = state ? 1 : 0;
            continue;
        }

        LOG(llevError, "quest: invalid line in %s: %s\n", final, read);
    }

    if (qs)
        LOG(llevError, "quest: missing end_quest in %s\n", final);
    if (buf)
        free(stringbuffer_finish(buf));

    fclose(file);
}

/**
 * Write quest-data information for a player.
 * @param pq player to write data for.
 */
static void quest_write_player_data(const quest_player *pq) {
    FILE *file;
    char write[MAX_BUF], final[MAX_BUF];
    const quest_state *state;

    snprintf(final, sizeof(final), "%s/%s/%s/%s.quest", settings.localdir, settings.playerdir, pq->player_name, pq->player_name);
    snprintf(write, sizeof(write), "%s.new", final);

    file = fopen(write, "w+");
    if (!file) {
        LOG(llevError, "quest: couldn't open player quest file %s!", write);
        draw_ext_info(NDI_UNIQUE | NDI_ALL_DMS, 0, NULL, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_LOADSAVE, "File write error on server!");
        return;
    }

    state = pq->quests;

    while (state) {
        fprintf(file, "quest %s\n", state->code);
        fprintf(file, "state %d\n", state->state);
        fprintf(file, "completed %d\n", state->was_completed);
        fprintf(file, "end_quest\n");
        state = state->next;
    }

    fclose(file);
    /** @todo rename/backup, stuff like that */
    unlink(final);
    rename(write, final);
}

/**
 * Get the state of a quest for a player, not creating if not existing yet.
 * @param pq player to get state for.
 * @param name quest to get state of.
 * @return NULL if quest isn't started yet for this player, else quest's state information.
 */
static quest_state *get_state(quest_player *pq, sstring name) {
    quest_state *qs = pq->quests;

    while (qs) {
        if (qs->code == name)
            return qs;
        qs = qs->next;
    }

    return NULL;
}

/**
 * Get the state of a quest for a player, creating it if not existing yet.
 * @param pq player to get state for.
 * @param name quest to get state of.
 * @return quest's state information, newly created if it wasn't done yet.
 */
static quest_state *get_or_create_state(quest_player *pq, sstring name) {
    quest_state *qs = get_state(pq, name);

    if (!qs) {
        qs = calloc(1, sizeof(quest_state));
        if (!qs)
            fatal(OUT_OF_MEMORY);
        qs->code = add_refcount(name);
        if (pq->quests != NULL) {
            quest_state *last;
            for (last = pq->quests ; last->next != NULL; last = last->next)
                ;
            last->next = qs;
        } else {
            pq->quests = qs;
        }
    }

    return qs;
}

/**
 * Get quest status for a player, not creating it if it doesn't exist.
 * @param pl player to get information of.
 * @return quest status, NULL if not loaded/found yet.
 */
static quest_player *get_quest(player *pl) {
    quest_player *pq = player_states;

    while (pq) {
        if (pq->player_name == pl->ob->name)
            return pq;
        pq = pq->next;
    }

    return NULL;
}

/**
 * Get quest status for a player, creating it if it doesn't exist yet.
 * Calls fatal() if memory allocation error.
 * @param pl player to get information of.
 * @return quest status, never NULL.
 */
static quest_player *get_or_create_quest(player *pl) {
    quest_player *pq = get_quest(pl);

    if (!pq) {
        pq = calloc(1, sizeof(quest_player));
        if (!pq)
            fatal(OUT_OF_MEMORY);
        pq->player_name = add_refcount(pl->ob->name);
        pq->next = player_states;
        player_states = pq;
        quest_read_player_data(pq);
    }

    return pq;
}

/* quest_set_state can call itself through the function update_quests, so it needs to be declared here */
static void quest_set_state(player *pl, sstring quest_code, int state, int started);

/**
 * Checks whether the conditions for a given step are met.
 * @param condition the linked list of conditions to check.
 * @param pl the player to evaluate conditions for.
 * @return 1 if the conditions match, 0 if they don't.
 */
static int evaluate_quest_conditions(const quest_condition *condition, player *pl) {
    const quest_condition *cond;
    int current_step;

    if (!condition)
        return 0;
    cond = condition;
    while (cond) {
        current_step = quest_get_player_state(pl, cond->quest_code);
        if (cond->minstep < 0 && cond->maxstep < 0) {
            /* we are checking for the quest to have been completed. */
            if (!quest_was_completed(pl, cond->quest_code))
                return 0;
        } else {
            if (current_step < cond->minstep || current_step > cond->maxstep)
                return 0;
        }
        cond = cond->next;
    }
    return 1;
}

/**
 * Look through all of the quests for the given player, and see if any need to be updated.
 * @param pl
 */
static void update_quests(player *pl) {
    const quest_definition *quest;
    const quest_step_definition *step;

    /* we are going to check the conditions for every step, and then find the highest
     * numbered step for which all conditions match, this will then be updated if that
     * is a later stage than the player is at currently.
     */
    int new_step, current_step;
    quest = quests;
    while (quest) {
        new_step=0;
        step = quest->steps;
        while (step) {
            if (step->conditions)
                if (evaluate_quest_conditions(step->conditions, pl)) {
                    new_step=new_step<step->step?step->step:new_step;
                }
            step = step->next;
        }
        if (new_step > 0) {
            current_step = quest_get_player_state(pl, quest->quest_code);
            if (new_step > current_step) {
                quest_set_state(pl, quest->quest_code, new_step, 0);
            }
        }
        quest = quest->next;
    }
}

/**
 * Set the state of a quest for a player.
 * @param pl player to set the state for.
 * @param quest_code quest internal code.
 * @param state new state for the quest, must be greater than 0 else forced to 100 and a warning is emitted.
 * @param started if 1, quest must have been started first or a warning is emitted, else it doesn't matter.
 */
static void quest_set_state(player *pl, sstring quest_code, int state, int started) {
    quest_player *pq = get_or_create_quest(pl);
    quest_state *qs = get_or_create_state(pq, quest_code);
    quest_definition *quest = quest_get(quest_code);
    quest_step_definition *step;

    if (!quest) {
        LOG(llevError, "quest: asking for set_state of unknown quest %s!\n", quest_code);
        return;
    }

    if (state <= 0) {
        LOG(llevDebug, "quest_set_player_state: warning: called with invalid state %d for quest %s, player %s", state, pl->ob->name, quest_code);
        state = 100;
    }

    step = quest_get_step(quest, state);
    if (!step) {
        LOG(llevError, "quest_set_player_state: couldn't find state definition %d for quest %s, player %s", state, quest_code, pl->ob->name);
        return;
    }

    if (started && qs->state == 0) {
        LOG(llevDebug, "quest_set_player_state: warning: called for player %s not having started quest %s\n", pl->ob->name, quest_code);
    }

    qs->state = state;
    if (step->is_completion_step) {
        /* don't send an update note if the quest was already completed, this is just to show the outcome afterwards. */
        if (!qs->is_complete)
            draw_ext_info_format(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_INFO, "Quest %s completed.", quest->quest_title);
        qs->was_completed = 1;
        if (quest->quest_restart)
            qs->state = QC_CAN_RESTART;
        else
            qs->is_complete =1;

    } else {
        draw_ext_info_format(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_INFO, "New objective for the quest '%s':", quest->quest_title);
        draw_ext_info(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_INFO, step->step_description);
    }

    if (pl->socket.notifications > 0) {
        SockList sl;
        SockList_Init(&sl);

        if (qs->sent_to_client) {
            SockList_AddString(&sl, "updquest ");
        } else {
            SockList_AddString(&sl, "addquest ");
        }

        SockList_AddInt(&sl, quest->client_code);
        if (qs->sent_to_client == 0) {
            SockList_AddLen16Data(&sl, quest->quest_title, strlen(quest->quest_title));
            if (quest->face && !(pl->socket.faces_sent[quest->face]&NS_FACESENT_FACE))
                esrv_send_face(&pl->socket, quest->face, 0);
            SockList_AddInt(&sl, quest->face);
            SockList_AddChar(&sl, quest->quest_restart ? 1 : 0);
            SockList_AddInt(&sl, quest->parent ? quest->parent->client_code : 0);
        }

        SockList_AddChar(&sl, (step == NULL || step->is_completion_step) ? 1 : 0);
        if (step != NULL)
            SockList_AddLen16Data(&sl, step->step_description, strlen(step->step_description));
        else
            SockList_AddShort(&sl, 0);

        Send_With_Handling(&pl->socket, &sl);
        SockList_Term(&sl);

        qs->sent_to_client = 1;
    }

    if (pl->has_directory)
        quest_write_player_data(pq);
    update_quests(pl);
    LOG(llevDebug, "quest_set_player_state %s %s %d\n", pl->ob->name, quest_code, state);

}

/**
 * Utility function to display a quest list. Will show a header before the list if not empty.
 * @param pl player to display list of quests.
 * @param pq quests to display.
 * @param showall if 0, only shows quests in progress and a summary of completed quests, else shows all quests.
 */
static void quest_display(player *pl, quest_player *pq, int showall) {
    quest_state *state;
    quest_definition *quest;
    const char *restart;
    int completed_count = 0, restart_count = 0, total_count = 0, current_count = 0;

    state = pq->quests;
    while (state) {
        quest = quest_get(state->code);
        if (quest->parent == NULL) {
            total_count++;
            /* count up the number of completed quests first */
            if (state->state == QC_CAN_RESTART) {
                restart_count++;
                completed_count++;
            } else if(state->is_complete) {
                completed_count++;
            }
        }
        state = state->next;
    }
    if (completed_count > 0) {
        if (!showall) {
            if (restart_count > 0)
                draw_ext_info_format(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_INFO,
                        "You have completed %d quests, of which %d may be restarted", completed_count, restart_count);
            else
                draw_ext_info_format(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_INFO,
                        "You have completed %d quests", completed_count);
            current_count = completed_count;
        } else {
            draw_ext_info_format(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_INFO,
                    "You have completed the following quests:");
            state = pq->quests;
            while (state) {
                quest = quest_get(state->code);
                if (quest->parent == NULL) {
                    if (state->state == QC_CAN_RESTART || state->is_complete) {

                        restart = state->state == QC_CAN_RESTART?" (can be replayed)":"";
                        draw_ext_info_format(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_INFO,
                            "(%3d) %s%s", ++current_count, quest->quest_title, restart);
                    }
                }
                state = state->next;
           }
        }
    }
    if (total_count > completed_count) {
        draw_ext_info(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_INFO,
                "You have started the following quests:");
        state = pq->quests;
        while (state) {
            quest = quest_get(state->code);
            if (quest->parent == NULL) {
                if (state->state != QC_CAN_RESTART && state->is_complete==0) {
                    quest = quest_get(state->code);
                    draw_ext_info_format(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_INFO,
                        "(%3d) %s", ++current_count, quest->quest_title);
                }
            }
            state = state->next;
        }
    }
}

/**
 * Display current and completed player quests.
 * @param pl player to display for.
 * @param showall - whether to show all of the quests in full, just summary information for the completed ones
 */
static void quest_list(player *pl, int showall) {
    quest_player *pq;

    /* ensure we load data if not loaded yet */
    pq = get_or_create_quest(pl);
    if (!pq->quests) {
        draw_ext_info(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_INFO, "You didn't start any quest.");
        return;
    }

    quest_display(pl, pq, showall);
}

/**
 * Quest command help.
 * @param pl player to display help for.
 */
static void quest_help(player *pl) {
    draw_ext_info(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_INFO, "Quest commands:");
    draw_ext_info(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_INFO, " - list: displays quests you are currently attempting add 'all' to show completed quests also");
    draw_ext_info(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_INFO, " - info: displays information about the specified (by number) quest");
}

/**
 * returns the quest state which corresponds to a certain number for the given player.
 * @param pl player asking for details.
 * @param number quest number.
 * @return quest state corresponding to the number provided, NULL if there is no such quest state.
 */

static quest_state *get_quest_by_number(player *pl, int number) {
    quest_state *state;
    quest_player *pq = get_or_create_quest(pl);
    int questnum = 0;

    if (number <= 0 || !pq) {
        return NULL;
    }
    /* count through completed quests first */
    state = pq->quests;
    while (state) {
            /* count up the number of completed quests first */
            if (!(quest_get(state->code)->parent) && (state->state == QC_CAN_RESTART || state->is_complete))
                if (++questnum == number) return state;
            state = state->next;
        }
    /* then active ones */
    state = pq->quests;
    while (state) {
        /* count up the number of completed quests first */
        if (!(quest_get(state->code)->parent) && state->state != QC_CAN_RESTART && state->is_complete ==0)
            if (++questnum == number) return state;
        state = state->next;
    }
    /* Ok, we didn't find our quest, return NULL*/
    return NULL;
}

/**
 * Give details about a quest.
 * @param pl player to give quest details to.
 * @param qs quest_state to give details about
 * @param level The level of recursion for the quest info that's being provided
 */
static void quest_info(player *pl, quest_state *qs, int level) {
    quest_definition *quest, *child;
    quest_state *state;
    quest_player *pq = get_or_create_quest(pl);
    quest_step_definition *step;
    const char *prefix;

    if (!qs) {
        draw_ext_info(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_INFO, "Invalid quest number");
        return;
    }
    quest = quest_get(qs->code);
    if (!quest) {
        /* already warned by quest_get */
        draw_ext_info(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_INFO, "Quest: (internal error)");
        return;
    }

    draw_ext_info_format(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_INFO, "Quest: %s", quest->quest_title);
    draw_ext_info_format(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_INFO, "Description: %s", quest->quest_description);

    step = quest_get_step(quest, qs->state);
    if (qs->state == QC_CAN_RESTART || qs->is_complete) {
        const char *restart = "";
        if (quest->quest_restart)
            restart = " (can be replayed)";
        draw_ext_info_format(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_INFO, "This quest has been completed%s.", restart);
    }
    prefix = "";
    if (qs->state != QC_CAN_RESTART) {
        /* ie, if we are in progress or completed for a non-restartable quest */
        if (!step) {
            /* already warned by quest_get_step */
            draw_ext_info_format(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_INFO, " \nOutcome: (invalid quest)");
            return;
        }
        if (level > 0) {
            prefix = " * ";
        } else if (qs->is_complete)
            prefix = "Outcome";
        else
            prefix = "Current Status";
        draw_ext_info_format(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_INFO, " \n%s: %s", prefix, step->step_description);
    }

    /* ok, now check all of the player's other quests for any children, and print those in order */
    state = pq->quests;
    while (state) {
        child = quest_get(state->code);
        if (child->parent == quest)
            quest_info(pl, state, level+1);
        state = state->next;
    }
    return;
}

/**
 * Free quests structures.
 * @param pq what to free.
 */
static void free_state(quest_player *pq) {
    quest_state *qs = pq->quests, *next;

    while (qs) {
        next = qs->next;
        free_string(qs->code);
        free(qs);
        qs = next;
    }
    pq->quests = NULL;
}


/* public functions */

/**
 * Get the quest state for a player.
 * @param pl player.
 * @param quest_code internal quest code.
 * @return QC_COMPLETED if finished and quest can't be replayed, 0 if not started or finished and can be replayed, else quest-specific value.
 */
int quest_get_player_state(player *pl, sstring quest_code) {
    quest_player *q = get_or_create_quest(pl);
    quest_state *s = get_state(q, quest_code);
    quest_definition *quest = quest_get(quest_code);

    if (!s)
        return 0;

    if (s->state == QC_CAN_RESTART && quest && quest->quest_restart)
        return 0;

    return s->state;
}

/**
 * Start a quest for a player. Will notify the player.
 * @param pl player.
 * @param quest_code internal quest code.
 * @param state initial quest state, must be greater than 0 else forced to 100 and warning emitted.
 */
void quest_start(player *pl, sstring quest_code, int state) {
    quest_player *pq;
    quest_state *q;
    quest_definition *quest;

    quest = quest_get(quest_code);
    if (!quest) {
        LOG(llevError, "quest_start: requested unknown quest %s\n", quest_code);
        return;
    }
    pq = get_or_create_quest(pl);
    q = get_or_create_state(pq, quest_code);

    if (state <= 0) {
        state = 100;
        LOG(llevDebug, "quest_start: negative state %d for %s quest %s\n", state, pl->ob->name, quest_code);
    }

    /* if completed already, assume the player can redo it */
    if (q->state > 0) {
        LOG(llevDebug, "quest_start: warning: player %s has already started quest %s\n", pl->ob->name, quest_code);
    }

    draw_ext_info_format(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_INFO, "New quest started: %s", quest->quest_title);

    quest_set_state(pl, quest_code, state, 0);

    /* saving state will be done in quest_set_state(). */
}

/**
 * Set the state of a quest for a player.
 * @param pl player to set the state for.
 * @param quest_code quest internal code.
 * @param state new state for the quest, must be greater than 0 else forced to 100 and a warning is emitted.
 */
void quest_set_player_state(player *pl, sstring quest_code, int state) {
    quest_set_state(pl, quest_code, state, 1);
}

/**
 * Check if a quest was completed once for a player, without taking account the current state.
 * @param pl who to check for.
 * @param quest_code quest internal code.
 * @return 1 if the quest was already completed at least once, 0 else.
 */
int quest_was_completed(player *pl, sstring quest_code) {
    quest_player *qp = get_or_create_quest(pl);
    quest_state *state = get_state(qp, quest_code);

    return (state && state->was_completed);
}

/**
 * Command handler for 'quest'.
 * @param op player asking for information, warning emitted if not a player.
 * @param params extra parameters for command.
 */
void command_quest(object *op, const char *params) {
    if (!op->contr) {
        LOG(llevError, "command_quest called for a non player!\n");
        return;
    }

    if (!params || *params == '\0') {
        quest_help(op->contr);
        return;
    }
    if (strcmp(params, "list all") == 0) {
        quest_list(op->contr, 1);
        return;
    }

    if (strcmp(params, "list") == 0) {
        quest_list(op->contr, 0);
        return;
    }

    if (strncmp(params, "info ", 5) == 0) {
        int number = atoi(params+5);
        quest_info(op->contr, get_quest_by_number(op->contr, number), 0);
        return;
    }

    quest_help(op->contr);
}

/**
 * Dump all defined quests on the logfile. Will call itself recursively.
 * @param parent only quests with a parent of this value will be displayed.
 * Use NULL to display top-level quests.
 * @param level number of '-' to display before the quest's name.
 */
static void output_quests(quest_definition *parent, int level) {
    quest_definition *quest;
    quest_step_definition *step;
    char prefix[MAX_BUF];
    int questcount = 0, stepcount, i;

    /* we only need to set the prefix once,
     * all quests that are printed in this call will be at the same level */
    prefix[0]='\0';
    for (i=0; i<level; i++) {
        strncat(prefix, "-", MAX_BUF - 1);
    }
    prefix[MAX_BUF - 1] = '\0';

    quest = quests;
    while (quest) {
        if (quest->parent == parent) {
            questcount++;
            stepcount=0;
            step = quest->steps;
            while (step) {
                stepcount++;
                step= step->next;
            }
            fprintf(logfile, "%s%s - %s - %d steps (%srestartable)\n", prefix, quest->quest_code, quest->quest_title, stepcount, quest->quest_restart?"":"not ");
            output_quests(quest, level+1);
        }
        quest = quest->next;
    }
}

/**
 * Dump all of the quests, then calls exit() - useful in terms of debugging to make sure that
 * quests are set up and recognised correctly.
 */
void dump_quests(void) {
    quest_load_definitions();
    output_quests(NULL, 0);
    exit(0);
}

/**
 * Free all quest status structures. It is all right to call quest functions again after that.
 */
void free_quest(void) {
    quest_player *pq = player_states, *next;

    while (pq) {
        next = pq->next;
        free_state(pq);
        free_string(pq->player_name);
        free(pq);
        pq = next;
    }
    player_states = NULL;
}

/**
 * Free all quest definitions and steps.
 * Can be called multiple times.
 * Used by DMs through the 'purge_quests' command.
 */
void free_quest_definitions(void) {
    quest_definition *quest = quests, *next_quest;
    quest_step_definition *step, *next_step;
    quest_condition *condition, *next_condition;

    while (quest != NULL) {
        next_quest = quest->next;
        free_string(quest->quest_code);
        if (quest->quest_description != NULL)
            free_string(quest->quest_description);
        if (quest->quest_title != NULL)
            free_string(quest->quest_title);
        step = quest->steps;
        while (step != NULL) {
            next_step = step->next;
            free_string(step->step_description);
            condition = step->conditions;
            while (condition != NULL) {
                next_condition = condition->next;
                free_string(condition->quest_code);
                free(condition);
                condition = next_condition;
            }
            free(step);
            step = next_step;
        }
        free(quest);
        quest = next_quest;
    }

    quests = NULL;
    quests_loaded = 0;
}

/**
 * Send the current quest states for the specified player, if the client
 * supports those notifications.
 * @param pl who to send quests for.
 */
void quest_send_initial_states(player *pl) {
    quest_player *states = NULL;
    quest_state *state = NULL;
    SockList sl;
    size_t size;
    quest_definition *quest;
    quest_step_definition *step;

    if (pl->socket.notifications < 1)
        return;

    /* ensure quest definitions are loaded */
    quest_load_definitions();

    states = get_or_create_quest(pl);

    SockList_Init(&sl);
    SockList_AddString(&sl, "addquest ");
    for (state = states->quests; state != NULL; state = state->next) {

        quest = quest_get_by_code(state->code);
        if (state->state == -1)
            step = NULL;
        else
            step = quest_get_step(quest, state->state);

        size = 2 + (2 + strlen(quest->quest_title)) + 4 + 1 + (2 + (step != NULL ? strlen(step->step_description) : 0));

        if (SockList_Avail(&sl) < size) {
            Send_With_Handling(&pl->socket, &sl);
            SockList_Reset(&sl);
            SockList_AddString(&sl, "addquest ");
        }

        SockList_AddInt(&sl, quest->client_code);
        SockList_AddLen16Data(&sl, quest->quest_title, strlen(quest->quest_title));
        if (quest->face && !(pl->socket.faces_sent[quest->face]&NS_FACESENT_FACE))
            esrv_send_face(&pl->socket, quest->face, 0);
        SockList_AddInt(&sl, quest->face);
        SockList_AddChar(&sl, quest->quest_restart ? 1 : 0);
        SockList_AddInt(&sl, quest->parent ? quest->parent->client_code : 0);
        SockList_AddChar(&sl, (step == NULL || step->is_completion_step) ? 1 : 0);
        if (step != NULL)
            SockList_AddLen16Data(&sl, step->step_description, strlen(step->step_description));
        else
            SockList_AddShort(&sl, 0);

        state->sent_to_client = 1;
    }

    Send_With_Handling(&pl->socket, &sl);
    SockList_Term(&sl);
}

/**
 * Ensure the quest state is correctly saved for a player.
 * This function should only be called once, when the player's save directory
 * is created. All other quest functions save the state automatically, but save
 * can only happen when the player directory exists.
 * @param pl who to save quests for.
 */
void quest_first_player_save(player *pl) {
    quest_player *qp = get_quest(pl);
    if (qp != NULL && qp->quests != NULL) {
        quest_write_player_data(qp);
    }
}
