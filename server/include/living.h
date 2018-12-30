/**
 * @file
 * Structure containing object statistics.
 */

#ifndef LIVING_H
#define LIVING_H

/** Object statistics. */
enum {
    STR = 0,       /**< Strength. */
    DEX = 1,       /**< Dexterity. */
    CON = 2,       /**< Constitution. */
    WIS = 3,       /**< Wisdom. */
    CHA = 4,       /**< Charisma. */
    INT = 5,       /**< Intelligence. */
    POW = 6,       /**< Power. */
    NUM_STATS = 7  /**< Number of statistics. */
};

/** Maximum level a player can reach. */
#define MAXLEVEL 115

extern const char *const attacks[NROFATTACKS];

extern const char *const restore_msg[NUM_STATS];
extern const char *const statname[NUM_STATS];
extern const char *const short_stat_name[NUM_STATS];
extern const char *const lose_msg[NUM_STATS];

/**
 * Various statistics of objects.
 */
typedef struct liv {
    sint8         Str, Dex, Con, Wis, Cha, Int, Pow;
    sint8         wc;         /**< Weapon Class, how skilled, the lower the better. */
    sint8         ac;         /**< Armour Class, how hard to hit, the lower the better. */
    sint8         luck;       /**< Affects thaco and ac from time to time */
    sint16        hp;         /**< Hit Points. */
    sint16        maxhp;      /**< Max hit points. */
    sint16        sp;         /**< Spell points.  Used to cast mage spells. */
    sint16        maxsp;      /**< Max spell points. */
    sint16        grace;      /**< Grace.  Used to invoke clerical prayers. */
    sint16        maxgrace;   /**< Maximum grace.  Used to invoke clerical prayers. */
    sint16        dam;        /**< How much damage this object does when hitting */
    sint64        exp;        /**< Experience.  Killers gain 1/10. */
    sint32        food;       /**< How much food in stomach.  0 = starved. */
} living;

int get_cha_bonus(int stat);
int get_dex_bonus(int stat);
int get_thaco_bonus(int stat);
uint32 get_weight_limit(int stat);
int get_learn_spell(int stat);
int get_cleric_chance(int stat);
int get_turn_bonus(int stat);
int get_dam_bonus(int stat);
float get_speed_bonus(int stat);
int get_fear_bonus(int stat);

#endif /* LIVING_H */
