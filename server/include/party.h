/**
 * @file
 * Party-specific structures.
 */

#ifndef PARTY_H
#define PARTY_H

/** One party. First item is ::firstparty. */
typedef struct party_struct {
    char *partyleader;          /**< Who is the leader. */
    char passwd[9];             /**< Party password. */
    struct party_struct *next;  /**< Next party in list. */
    char *partyname;            /**< Party name. */

#ifdef PARTY_KILL_LOG
    struct party_kill {
        char killer[MAX_NAME+1], dead[MAX_NAME+1];
        sint64 exp;
    } party_kills[PARTY_KILL_LOG];
    sint64 total_exp;
    uint32  kills;
#endif
} partylist;

#endif
