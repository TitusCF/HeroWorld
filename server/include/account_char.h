/**
 * @file
 * Characters associated with an account.n
 */


#ifndef ACCOUNT_CHAR_H
#define ACCOUNT_CHAR_H

/**
 * The maximum characters per account is really driven by the size of
 * the buffer we use to read in the data.  Take 150 characters off for
 * the account name, password, overhead, and other wiggle room and
 * find a maximum.  From my quick calculations, this amounts to
 * 18 characters/account.  I think that is sufficient - moving to a
 * HUGE_BUF would allow 82.
 * The code could be more clever and look at the length of each
 * character name and total it up, but having the same limit for everyone
 * is better IMO.
 */
#define MAX_CHARACTERS_PER_ACCOUNT (VERY_BIG_BUF - 150) / (MAX_NAME+1)


/**
 * One character account.
 */
typedef struct account_char_struct {
    const char *name;         /**< Name of this character/player */
    const char *character_class;        /**< Class of this character */
    const char *race;         /**< Race of this character */
    uint8 level;              /**< Level of this character */
    const char *face;         /**< Face of this character */
    const char *party;        /**< Character this party belonged to */
    const char *map;          /**< Last map this character was on */
    uint8 isDead;             /**< Should stay at zero if alive, anything else if dead (hopefully 1, but doesn't have to be) */
    struct account_char_struct  *next;
} Account_Char;

#endif /* OBJECT_H */
