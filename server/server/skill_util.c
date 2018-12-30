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
 * Various skill-related functions.
 *
 * Created July 95 to separate skill utilities from actual skills -b.t.
 *
 * Reconfigured skills code to allow linking of skills to experience
 * categories. This is done solely through the init_new_exp_system() fctn.
 * June/July 1995 -b.t. thomas@astro.psu.edu
 *
 *
 * July 1995 - Initial associated skills coding. Experience gains
 * come solely from the use of skills. Overcoming an opponent (in combat,
 * finding/disarming a trap, stealing from somebeing, etc) gains
 * experience. Calc_skill_exp() handles the gained experience using
 * modifications in the skills[] table. - b.t.
 */

/* define the following for skills utility debugging */
/* #define SKILL_UTIL_DEBUG */

#define WANT_UNARMED_SKILLS

#include <global.h> 
#include <object.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#endif
#include <living.h> /* for defs of STR,CON,DEX,etc. -b.t.*/
#include <spells.h>
#include <skills.h>

static void attack_hth(object *pl, int dir, const char *string, object *skill);
static void attack_melee_weapon(object *op, int dir, const char *string, object *skill);

/**
 * Will contain a number-name mapping for skills, initialized by init_skills().
 */
const char *skill_names[NUM_SKILLS];
/**
 * Will contain the face numbers for the skills, initialized by init_skill().
 */
int skill_faces[NUM_SKILLS];

/**
 * This just sets up the ::skill_names table above. The index into the array is set up by the
 * subtypes.
 */
void init_skills(void)
{
    int i;
    archetype *at;

    for (i = 0; i < NUM_SKILLS; i++)
    {
        skill_names[i] = NULL;
        skill_faces[i] = -1;
    }

    for (at = first_archetype; at != NULL; at = at->next)
    {
        if (at->clone.type == SKILL)
        {
            if (at->clone.subtype >= sizeof(skill_names) / sizeof(*skill_names))
            {
                LOG(llevError, "init_skills: invalid skill subtype %d for skill %s\n", at->clone.subtype, at->clone.skill);
            }
            else if (skill_names[at->clone.subtype] != NULL)
            {
                LOG(llevError, "init_skills: multiple skill using same subtype %d, %s, %s\n",
                    at->clone.subtype, skill_names[at->clone.subtype], at->clone.skill);
            }
            else
            {
                skill_names[at->clone.subtype] = add_refcount(at->clone.skill);
                if (at->clone.face != NULL)
                    skill_faces[at->clone.subtype] = at->clone.face->number;
            }
        }
    }

    /* This isn't really an error if there is no skill subtype set, but
     * checking for this may catch some user errors.
     * On the other hand, it'll crash later on, which is not nice. Thus giving a dummy name.
     */
    for (i = 1; i < NUM_SKILLS; i++)
    {
        if (!skill_names[i])
        {
            LOG(llevError, "init_skills: skill subtype %d doesn't have a name?\n", i);
            skill_names[i] = add_string("dummy skill");
        }
    }
}

/**
 * This function goes through the player inventory and sets
 * up the last_skills[] array in the player object.
 * The last_skills[] is used to more quickly lookup skills -
 * mostly used for sending exp.
 *
 * @param op
 * player to link skills for. Must be a player.
 */
void link_player_skills(object *op)
{
    FOR_INV_PREPARE(op, tmp)
    {
        if (tmp->type == SKILL)
        {
            /* This is really a warning, hence no else below */
            if (op->contr->last_skill_ob[tmp->subtype] && op->contr->last_skill_ob[tmp->subtype] != tmp)
            {
                LOG(llevError, "Multiple skills with the same subtype? %s, %s\n", op->contr->last_skill_ob[tmp->subtype]->skill, tmp->skill);
            }
            if (tmp->subtype >= NUM_SKILLS)
            {
                LOG(llevError, "Invalid subtype number %d (range 0-%d)\n", tmp->subtype, NUM_SKILLS);
            }
            else
            {
                op->contr->last_skill_ob[tmp->subtype] = tmp;
                op->contr->last_skill_exp[tmp->subtype] = -1;
            }
        }
    }
    FOR_INV_FINISH();
}

/**
 * This returns specified skill if it can be used, potentially using tool to help.
 *
 * Skill and tool can't be NULL at the same time.
 *
 * Factored from find_skill_by_name() and find_skill_by_number().
 *
 * The skill tool will be applied if not NULL, so the player can benefit from its bonuses.
 *
 * @param who
 * player trying to ready a skill.
 * @param skill
 * skill to ready. Can be NULL.
 * @param skill_tool
 * skill tool. Can be NULL.
 * @return
 * skill object if it can be used, NULL else.
 * @note
 * clawing skill is special, as claws are declared as SKILLTOOL but shouldn't be applied.
 * @todo
 * rewrite some.
 */
static object *adjust_skill_tool(object *who, object *skill, object *skill_tool)
{
    if (!skill && !skill_tool)
        return NULL;

    /* If this is a skill that can be used without a tool and no tool found, return it */
    if (skill && QUERY_FLAG(skill, FLAG_CAN_USE_SKILL) && (!skill_tool || QUERY_FLAG(skill_tool, FLAG_APPLIED) || strcmp(skill->skill, "clawing") == 0))
        return skill;

    /* Player has a tool to use the skill.  If not applied, apply it -
     * if not successful, return skill if can be used.  If they do have the skill tool
     * but not the skill itself, give it to them.
     */
    if (skill_tool)
    {
        if (!QUERY_FLAG(skill_tool, FLAG_APPLIED))
        {
            if (apply_special(who, skill_tool, 0))
            {
                if (skill && QUERY_FLAG(skill, FLAG_CAN_USE_SKILL))
                    return skill;
                else
                    return NULL;
            }
        }
        if (!skill)
        {
            skill = give_skill_by_name(who, skill_tool->skill);
            link_player_skills(who);
        }
        return skill;
    }
    if (skill && QUERY_FLAG(skill, FLAG_CAN_USE_SKILL))
        return skill;
    else
        return NULL;
}

/**
 * This returns the skill pointer of the given name (the
 * one that accumulates exp, has the level, etc).
 *
 * It is presumed that the player will be needing to actually
 * use the skill, so a skill tool will be equipped if one if found
 * to benefit from its bonuses.
 *
 * This code is basically the same as find_skill_by_number() below,
 * but instead of a skill number, we search by the name.
 *
 * Because a multiple skill names are allowed, we have to use arrays
 * to store the intermediate results - we can't have skill_tool
 * point to praying with skill pointing to fire magic.
 *
 * @param who
 * Player to get skill.
 * @param name
 * skill to find. Needs not to be a shared string.  name can be a comma
 * separated list of skill names - in that case, we will find the skill
 * of the highest level.
 * @return
 * pointer to skill object, or NULL if player doesn't have it.
 *
 * @todo
 * Maybe better selection of skill when choice of multiple skills is in
 * use (highest level may not be the best answer?)
 *
 */
object *find_skill_by_name(object *who, const char *name)
{
    object *skill = NULL, *skills[NUM_SKILLS], *skill_tools[NUM_SKILLS];
    const char *skill_names[NUM_SKILLS];
    char *ourname = NULL;
    int num_names, highest_level_skill = 0, i;

    if (!name)
        return NULL;

    /* Simple case - no commas in past in name, so don't need to tokenize */
    if (!strchr(name, ','))
    {
        skill_names[0] = name;
        skill_tools[0] = NULL;
        skills[0] = NULL;
        num_names = 1;
    }
    else
    {
        /* strtok_r is destructive, so we need our own copy */
        char *lasts;
        ourname = strdup(name);

        if ((skill_names[0] = strtok_r(ourname, ",", &lasts)) == NULL)
        {
            /* This should really never happen */
            LOG(llevError, "find_skill_by_name: strtok_r returned null, but strchr did not?\n");
            return NULL;
        }
        else
        {
            skill_tools[0] = NULL;
            skills[0] = NULL;
            /* we already have the first name from the strtok_r above */
            num_names = 1;
            while ((skill_names[num_names] = strtok_r(NULL, ",", &lasts)) != NULL)
            {
                /* Clean out any leading spacing.  typical string would be
                 * skill1, skill2, skill3, ...
                 */
                while (isspace(*skill_names[num_names]))
                    skill_names[num_names]++;
                skills[num_names] = NULL;
                skill_tools[num_names] = NULL;
                num_names++;
            }
        }
        /* While we don't use ourname below this point, the skill_names[] points into
         * it, so we can't free it yet.
         */
    }

    FOR_INV_PREPARE(who, tmp)
    {
        /* We make sure the length of the string in the object is greater
        * in length than the passed string. Eg, if we have a skill called
        * 'hi', we don't want to match if the user passed 'high'
        */
        if (tmp->type == SKILL || tmp->type == SKILL_TOOL)
        {
            for (i = 0; i < num_names; i++)
            {
                if (!strncasecmp(skill_names[i], tmp->skill, strlen(skill_names[i])) &&
                    strlen(tmp->skill) >= strlen(skill_names[i]))
                {
                    if (tmp->type == SKILL)
                    {
                        skills[i] = tmp;
                        if (!skill || tmp->level > skill->level)
                        {
                            skill = tmp;
                            highest_level_skill = i;
                        }
                    }
                    else
                    {
                        /* Skill tools don't have levels, so we basically find the
                         * 'best' skill tool for this skill.
                         */
                        if (QUERY_FLAG(tmp, FLAG_APPLIED) || !skill_tools[i] ||
                            !QUERY_FLAG(skill_tools[i], FLAG_APPLIED))
                        {
                            skill_tools[i] = tmp;
                        }
                    }
                    /* Got a matching name - no reason to look through rest of names */
                    break;
                }
            }
        }
    }
    FOR_INV_FINISH();
    free(ourname);
    return adjust_skill_tool(who, skills[highest_level_skill], skill_tools[highest_level_skill]);
}

/**
 * This returns the skill pointer of the given name (the
 * one that accumulates exp, has the level, etc).
 *
 * It is presumed that the player will be needing to actually
 * use the skill, so a skill tool will be equipped if one if found
 * to benefit from its bonuses.
 *
 * This code is basically the same as find_skill_by_name() above,
 * but instead of a skill name, we search by matching number.
 *
 * @param who
 * player applying a skill.
 * @param skillno
 * skill subtype.
 * @return
 * skill object if player can use it, NULL else.
 */
object *find_skill_by_number(object *who, int skillno)
{
    object *skill = NULL, *skill_tool = NULL;

    if (skillno < 1 || skillno >= NUM_SKILLS)
        return NULL;

    FOR_INV_PREPARE(who, tmp)
    {
        if (tmp->type == SKILL && tmp->subtype == skillno)
            skill = tmp;

        /* Try to find appropriate skilltool.  If the player has one already
         * applied, we try to keep using that one.
         */
        else if (tmp->type == SKILL_TOOL && tmp->subtype == skillno)
        {
            if (QUERY_FLAG(tmp, FLAG_APPLIED))
                skill_tool = tmp;
            else if (!skill_tool || !QUERY_FLAG(skill_tool, FLAG_APPLIED))
                skill_tool = tmp;
        }
    }
    FOR_INV_FINISH();

    return adjust_skill_tool(who, skill, skill_tool);
}

/**
 * This changes the object's skill to new_skill.
 * Note that this function doesn't always need to get used -
 * you can now add skill exp to the player without the chosen_skill being
 * set.  This function is of most interest to players to update
 * the various range information.
 *
 * @param who
 * living to change skill for.
 * @param new_skill
 * skill to use. If NULL, this just unapplies the current skill.
 * @param flag
 * has the current meaning:
 * - 0x1: If set, don't update the range pointer.  This is useful when we
 *   need to ready a new skill, but don't want to clobber range.
 * @retval 0
 * change failure.
 * @retval 1
 * success
 */
int change_skill(object *who, object *new_skill, int flag)
{
    rangetype old_range;

    if (who->type != PLAYER)
        return 0;

    old_range = who->contr->shoottype;

    /* The readied skill can be a skill tool, so check on actual skill instead of object. */
    if (who->chosen_skill && who->chosen_skill->skill == new_skill->skill)
    {
        /* optimization for changing skill to current skill */
        if (!(flag & 0x1))
            who->contr->shoottype = range_skill;
        return 1;
    }

    if (!new_skill || who->chosen_skill)
        if (who->chosen_skill)
            apply_special(who, who->chosen_skill, AP_UNAPPLY | (flag & AP_NOPRINT));

    /* Only goal in this case was to unapply a skill */
    if (!new_skill)
        return 0;

    if (apply_special(who, new_skill, AP_APPLY | (flag & AP_NOPRINT)))
    {
        return 0;
    }
    if (flag & 0x1)
        who->contr->shoottype = old_range;

    return 1;
}

/**
 * This function just clears the chosen_skill and range_skill values
 * in the player.
 *
 * @param who
 * living to clear.
 */
void clear_skill(object *who)
{
    who->chosen_skill = NULL;
    CLEAR_FLAG(who, FLAG_READY_SKILL);
    if (who->type == PLAYER)
    {
        who->contr->ranges[range_skill] = NULL;
        if (who->contr->shoottype == range_skill)
            who->contr->shoottype = range_none;
    }
}

/**
 * Main skills use function-similar in scope to cast_spell().
 * We handle all requests for skill use outside of some combat here.
 * We require a separate routine outside of fire() so as to allow monsters
 * to utilize skills.
 *
 * This is changed (2002-11-30) from the old method that returned
 * exp - no caller needed that info, but it also prevented the callers
 * from know if a skill was actually used, as many skills don't
 * give any exp for their direct use (eg, throwing).
 *
 * Gives experience if appropriate.
 *
 * @param op The object actually using the skill
 * @param part actual part using the skill, used by throwing for monsters
 * @param skill The skill used by op
 * @param dir The direction in which the skill is used
 * @param string A parameter string, necessary to use some skills
 * @retval 0
 * skill failure.
 * @retval 1
 * use of the skill was successful.
 */
int do_skill(object *op, object *part, object *skill, int dir, const char *string)
{
    int success = 0, exp = 0;

    if (!skill)
        return 0;

    /* The code below presumes that the skill points to the object that
     * holds the exp, level, etc of the skill.  So if this is a player
     * go and try to find the actual real skill pointer, and if the
     * the player doesn't have a bucket for that, create one.
     */
    if (skill->type != SKILL && op->type == PLAYER)
    {
        object *tmp;

        tmp = object_find_by_type_and_skill(op, SKILL, skill->skill);
        if (!tmp)
            tmp = give_skill_by_name(op, skill->skill);
        skill = tmp;
    }

    if (skill->anim_suffix)
        apply_anim_suffix(op, skill->anim_suffix);

    switch (skill->subtype)
    {
    case SK_LEVITATION:
        /* Not 100% sure if this will work with new movement code -
         * the levitation skill has move_type for flying, so when
         * equipped, that should transfer to player, when not,
         * shouldn't.
         */
        if (QUERY_FLAG(skill, FLAG_APPLIED))
        {
            CLEAR_FLAG(skill, FLAG_APPLIED);
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_SUCCESS,
                          "You come to earth.");
        }
        else
        {
            SET_FLAG(skill, FLAG_APPLIED);
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_SUCCESS,
                          "You rise into the air!.");
        }
        fix_object(op);
        success = 1;
        break;

    case SK_STEALING:
        exp = success = steal(op, dir, skill);
        break;

    case SK_LOCKPICKING:
        exp = success = pick_lock(op, dir, skill);
        break;

    case SK_HIDING:
        exp = success = hide(op, skill);
        break;

    case SK_JUMPING:
        success = jump(op, dir, skill);
        break;

    case SK_INSCRIPTION:
        exp = success = write_on_item(op, string, skill);
        break;

    case SK_MEDITATION:
        meditate(op, skill);
        success = 1;
        break;
        /* note that the following 'attack' skills gain exp through hit_player() */

    case SK_KARATE:
        attack_hth(op, dir, "karate-chopped", skill);
        break;

    case SK_PUNCHING:
        attack_hth(op, dir, "punched", skill);
        break;

    case SK_FLAME_TOUCH:
        attack_hth(op, dir, "flamed", skill);
        break;

    case SK_CLAWING:
        attack_hth(op, dir, "clawed", skill);
        break;

    case SK_WRAITH_FEED:
        attack_hth(op, dir, "fed upon", skill);
        break;

    case SK_ONE_HANDED_WEAPON:
    case SK_TWO_HANDED_WEAPON:
        (void)attack_melee_weapon(op, dir, NULL, skill);
        break;

    case SK_FIND_TRAPS:
        exp = success = find_traps(op, skill);
        break;

    case SK_SINGING:
        exp = success = singing(op, dir, skill);
        break;

    case SK_ORATORY:
        exp = success = use_oratory(op, dir, skill);
        break;

    case SK_SMITHERY:
        if (use_smithery(op) == 1)
            exp = success = skill_ident(op, skill);
        break;
    case SK_BOWYER:
        if (use_bowery(op) == 1)
            exp = success = skill_ident(op, skill);
        break;
    case SK_JEWELER:
        if (use_jeweler(op) == 1)
            exp = success = skill_ident(op, skill);
        break;
    case SK_WOODSMAN:
    case SK_THAUMATURGY:
    case SK_LITERACY:
    case SK_ALCHEMY:
        if (use_alchemy(op) == 1)
            exp = success = skill_ident(op, skill);
        break;

    case SK_DET_MAGIC:
    case SK_DET_CURSE:
        exp = success = skill_ident(op, skill);
        break;

    case SK_DISARM_TRAPS:
        exp = success = remove_trap(op, skill);
        break;

    case SK_THROWING:
        success = skill_throw(op, part, dir, skill);
        break;

    case SK_SET_TRAP:
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                      "This skill is not currently implemented.");
        break;

    case SK_USE_MAGIC_ITEM:
    case SK_MISSILE_WEAPON:
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                      "There is no special attack for this skill.");
        break;

    case SK_PRAYING:
        success = pray(op, skill);
        break;

    case SK_BARGAINING:
        success = describe_shop(op);
        break;

    case SK_SORCERY:
    case SK_EVOCATION:
    case SK_PYROMANCY:
    case SK_SUMMONING:
    case SK_CLIMBING:
    case SK_EARTH_MAGIC:
    case SK_AIR_MAGIC:
    case SK_FIRE_MAGIC:
    case SK_WATER_MAGIC:
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                      "This skill is already in effect.");
        break;

    case SK_HARVESTING:
        do_harvest(op, dir, skill);
        success = 0;
        break;

    default:
    {
        char name[MAX_BUF];

        query_name(op, name, MAX_BUF);
        LOG(llevDebug, "%s attempted to use unknown skill: %d\n", name, op->chosen_skill->stats.sp);
        break;
    }
    }

    /* For players we now update the speed_left from using the skill.
     * Monsters have no skill use time because of the random nature in
     * which use_monster_skill is called already simulates this.
     * If certain skills should take more/less time, that should be
     * in the code for the skill itself.
     */

    if (op->type == PLAYER)
        op->speed_left -= 1.0;

    /* this is a good place to add experience for successful use of skills.
     * Note that add_exp() will figure out player/monster experience
     * gain problems.
     */

    if (success && exp)
        change_exp(op, exp, skill->skill, SK_SUBTRACT_SKILL_EXP);

    return success;
}

/**
 * Calculates amount of experience can be gained for
 * successful use of a skill.
 *
 * Here we take the view that a player must 'overcome an opponent'
 * in order to gain experience. Examples include foes killed combat,
 * finding/disarming a trap, stealing from somebeing, etc.
 *
 * The gained experience is based primarily on the difference in levels,
 * exp point value of vanquished foe, the relevant stats of the skill being
 * used and modifications in the skills[] table.
 *
 * For now, monsters and players will be treated differently. Below I give
 * the algorithm for *PLAYER *experience gain. Monster exp gain is simpler.
 * Monsters just get 10% of the exp of the opponent.
 *
 * Players get a ratio, eg, opponent lvl / player level.  This is then
 * multiplied by various things.  If simple exp is true, then
 * this multiplier, include the level difference, is always 1.
 * This revised method prevents some cases where there are big gaps
 * in the amount you get just because you are now equal level vs lower
 * level
 *
 * @param who
 * player/creature that used the skill.
 * @param op
 * object that was 'defeated'.
 * @param skill
 * used skill.  If none, it should just point back to who or be NULL.
 * @return
 * experience for the skill use.
 */
sint64 calc_skill_exp(const object *who, const object *op, const object *skill)
{
    sint64 op_exp;
    int op_lvl;
    float base, value, lvl_mult;

    if (!skill)
        skill = who;

    /* Oct 95 - where we have an object, I expanded our treatment
     * to 3 cases:
     * non-living magic obj, runes and everything else.
     *
     * If an object is not alive and magical we set the base exp higher to
     * help out exp awards for skill_ident skills. Also, if
     * an item is type RUNE, we give out exp based on stats.Cha
     * and level (this was the old system) -b.t.
     */

    if (!op)
    { /* no item/creature */
        op_lvl = MAX(who->map->difficulty, 1);
        op_exp = 0;
    }
    else if (op->type == RUNE || op->type == TRAP)
    { /* all traps. If stats.Cha > 1 we use that
     * for the amount of experience */
        op_exp = op->stats.Cha > 1 ? op->stats.Cha : op->stats.exp;
        op_lvl = op->level;
    }
    else
    { /* all other items/living creatures */
        op_exp = op->stats.exp;
        op_lvl = op->level;
        if (!QUERY_FLAG(op, FLAG_ALIVE))
        { /* for ident/make items */
            op_lvl += 5 * abs(op->magic);
        }
    }

    if (op_lvl < 1)
        op_lvl = 1;

    if (who->type != PLAYER)
    {                                        /* for monsters only */
        return ((sint64)(op_exp * 0.1) + 1); /* we add one to insure positive value is returned */
    }

    /* for players */
    base = op_exp;
    /* if skill really is a skill, then we can look at the skill archetype for
     * base reward value (exp) and level multiplier factor.
     */
    if (skill->type == SKILL)
    {
        base += skill->arch->clone.stats.exp;
        if (settings.simple_exp)
        {
            if (skill->arch->clone.level)
                lvl_mult = (float)skill->arch->clone.level / 100.0;
            else
                lvl_mult = 1.0; /* no adjustment */
        }
        else
        {
            if (skill->level)
                lvl_mult = ((float)skill->arch->clone.level * (float)op_lvl) / ((float)skill->level * 100.0);
            else
                lvl_mult = 1.0;
        }
    }
    else
    {
        /* Don't divide by zero here! */
        lvl_mult = (float)op_lvl / (float)(skill->level ? skill->level : 1);
    }

    /* assemble the exp total, and return value */

    value = base * lvl_mult;
    if (value < 1)
        value = 1; /* Always give at least 1 exp point */

#ifdef SKILL_UTIL_DEBUG
    LOG(llevDebug, "calc_skill_exp(): who: %s(lvl:%d)  op:%s(lvl:%d)\n", who->name, skill->level, op->name, op_lvl);
#endif
    return ((sint64)value);
}

/**
 * Player is trying to learn a skill. Success is based on Int.
 *
 * This inserts the requested skill in the player's
 * inventory. The skill field of the scroll should have the
 * exact name of the requested skill.
 *
 * This one actually teaches the player the skill as something
 * they can equip.
 *
 * @retval 0
 * player already knows the skill.
 * @retval 1
 * the player learns the skill.
 * @retval 2
 * some failure.
 */
int learn_skill(object *pl, object *scroll)
{
    object *tmp;

    if (!scroll->skill)
    {
        LOG(llevError, "skill scroll %s does not have skill pointer set.\n", scroll->name);
        return 2;
    }

    /* can't use find_skill_by_name because we want skills the player knows
     * but can't use natively.
     */

    tmp = NULL;
    FOR_INV_PREPARE(pl, inv)
    if (inv->type == SKILL && !strncasecmp(scroll->skill, inv->skill, strlen(scroll->skill)))
    {
        tmp = inv;
        break;
    }
    FOR_INV_FINISH();

    /* player already knows it */
    if (tmp && QUERY_FLAG(tmp, FLAG_CAN_USE_SKILL))
        return 0;

    /* now a random change to learn, based on player Int.
     * give bonus based on level - otherwise stupid characters
     * might never be able to learn anything.
     */
    if (random_roll(0, 99, pl, PREFER_LOW) > (get_learn_spell(pl->stats.Int) + (pl->level / 5)))
        return 2; /* failure :< */

    if (!tmp)
        tmp = give_skill_by_name(pl, scroll->skill);

    if (!tmp)
    {
        LOG(llevError, "skill scroll %s does not have valid skill name (%s).\n", scroll->name, scroll->skill);
        return 2;
    }

    SET_FLAG(tmp, FLAG_CAN_USE_SKILL);
    link_player_skills(pl);
    return 1;
}

/**
 * Gives a percentage clipped to 0% -> 100% of a/b.
 *
 * @param a
 * current value
 * @param b
 * max value
 * @return
 * value between 0 and 100.
 * @todo Probably belongs in some global utils-type file?
 */
static int clipped_percent(sint64 a, sint64 b)
{
    int rv;

    if (b <= 0)
        return 0;

    rv = (int)((100.0f * ((float)a) / ((float)b)) + 0.5f);

    if (rv < 0)
        return 0;
    else if (rv > 100)
        return 100;

    return rv;
}

/**
 * Displays a player's skill list, and some other non skill related info (god,
 * max weapon improvements, item power).
 *
 * This shows the amount of exp they have in the skills.
 *
 * Note this function is a bit more complicated because we
 * we want ot sort the skills before printing them.  If we
 * just dumped this as we found it, this would be a bit
 * simpler.
 *
 * @param op
 * player wanting to examine skills.
 * @param search
 * optional string to restrict skills to show.
 */
void show_skills(object *op, const char *search)
{
    const char *cp;
    int i, num_skills_found = 0;
    static const char *const periods = "........................................";
    /* Need to have a pointer and use strdup for qsort to work properly */
    char skills[NUM_SKILLS][MAX_BUF];

    FOR_INV_PREPARE(op, tmp)
    {
        if (tmp->type == SKILL)
        {
            char buf[MAX_BUF];

            if (search && strstr(tmp->name, search) == NULL)
                continue;
            /* Basically want to fill this out to 40 spaces with periods */
            snprintf(buf, sizeof(buf), "%s%s", tmp->name, periods);
            buf[40] = 0;

            if (settings.permanent_exp_ratio)
            {
                snprintf(skills[num_skills_found++], MAX_BUF, "%slvl:%3d (xp:%" FMT64 "/%" FMT64 "/%d%%)",
                         buf, tmp->level,
                         tmp->stats.exp,
                         level_exp(tmp->level + 1, op->expmul),
                         clipped_percent(tmp->perm_exp, tmp->stats.exp));
            }
            else
            {
                snprintf(skills[num_skills_found++], MAX_BUF, "%slvl:%3d (xp:%" FMT64 "/%" FMT64 ")",
                         buf, tmp->level,
                         tmp->stats.exp,
                         level_exp(tmp->level + 1, op->expmul));
            }
            /* I don't know why some characters get a bunch of skills, but
             * it sometimes happens (maybe a leftover from buggier earlier code
             * and those character are still about).  In any case, lets handle
             * it so it doesn't crash the server - otherwise, one character may
             * crash the server numerous times.
             */
            if (num_skills_found >= NUM_SKILLS)
            {
                draw_ext_info(NDI_RED, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                              "Your character has too many skills. "
                              "Something isn't right - contact the server admin");
                break;
            }
        }
    }
    FOR_INV_FINISH();

    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_LIST,
                  "Player skills:");

    if (num_skills_found > 1)
        qsort(skills, num_skills_found, MAX_BUF, (int (*)(const void *, const void *))strcmp);

    for (i = 0; i < num_skills_found; i++)
    {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_LIST,
                      skills[i]);
    }

    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_LIST,
                         "You can handle %d weapon improvements.",
                         op->level / 5 + 5);

    cp = determine_god(op);
    if (strcmp(cp, "none") == 0)
        cp = NULL;
    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_LIST,
                         "You worship %s.",
                         cp ? cp : "no god at current time");

    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_LIST,
                         "Your equipped item power is %d out of %d\n",
                         op->contr->item_power, op->level);
}

/**
 * Similar to invoke command, it executes the skill in the
 * direction that the user is facing.
 *
 * This is tricky because skills can have spaces.  We basically roll
 * our own find_skill_by_name so we can try to do better string matching.
 *
 * @param op
 * player trying to use a skill.
 * @param string
 * parameter for the skill to use.
 * @retval 0
 * unable to change to the requested skill, or unable to use the skill properly.
 * @retval 1
 * skill correctly used.
 */
int use_skill(object *op, const char *string)
{
    object *skop;
    size_t len;

    if (!string)
        return 0;

    skop = NULL;
    FOR_INV_PREPARE(op, tmp)
    {
        if (tmp->type == SKILL && QUERY_FLAG(tmp, FLAG_CAN_USE_SKILL) && !strncasecmp(string, tmp->skill, MIN(strlen(string), strlen(tmp->skill))))
        {
            skop = tmp;
            break;
        }
        else if (tmp->type == SKILL_TOOL && !strncasecmp(string, tmp->skill, MIN(strlen(string), strlen(tmp->skill))))
        {
            skop = tmp;
            break;
        }
    }
    FOR_INV_FINISH();
    if (!skop)
    {
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_MISSING,
                             "Unable to find skill %s",
                             string);
        return 0;
    }

    len = strlen(skop->skill);

    /* All this logic goes and skips over the skill name to find any
     * options given to the skill.  Its pretty simple - if there
     * are extra parameters (as determined by string length), we
     * want to skip over any leading spaces.
     */
    if (len >= strlen(string))
    {
        string = NULL;
    }
    else
    {
        string += len;
        while (*string == 0x20)
            string++;
        if (strlen(string) == 0)
            string = NULL;
    }

#ifdef SKILL_UTIL_DEBUG
    LOG(llevDebug, "use_skill() got skill: %s\n", sknum > -1 ? skills[sknum].name : "none");
#endif

    /* Change to the new skill, then execute it. */
    if (do_skill(op, op, skop, op->facing, string))
        return 1;

    return 0;
}

/**
 * Finds the best unarmed skill the player has, and returns
 * it.  Best can vary a little - we consider clawing to always
 * be the best for dragons.
 *
 * This could be more intelligent, eg, look at the skill level
 * of the skill and go from there (eg, a level 40 puncher is
 * is probably better than level 1 karate).  OTOH, if you
 * don't bother to set up your skill properly, that is the players
 * problem (although, it might be nice to have a preferred skill
 * field the player can set.
 *
 * Unlike the old code, we don't give out any skills - it is
 * possible you just don't have any ability to get into unarmed
 * combat.  If everyone race/class should have one, this should
 * be handled in the starting treasurelists, not in the code.
 *
 * @param op
 * player to get skill for.
 * @return
 * attack skill, NULL if no suitable found.
 */
static object *find_best_player_hth_skill(object *op)
{
    object *best_skill = NULL;
    int last_skill;

    if (op->contr->unarmed_skill)
    {
        /* command_unarmed_skill() already does these checks, and right
         * now I do not think there is any way to lose unarmed skills.
         * But maybe in the future there will be (polymorph?) so handle
         * it appropriately.  MSW 2009-07-03
         *
         * Note that the error messages should only print out once when
         * the initial failure to switch skills happens, so the player
         * should not get spammed with tons of messages unless they have
         * no valid unarmed skill
         */

        best_skill = find_skill_by_name(op, op->contr->unarmed_skill);

        if (!best_skill)
        {
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_MISSING,
                                 "Unable to find skill %s - using default unarmed skill",
                                 op->contr->unarmed_skill);
        }
        else
        {
            size_t i;

            for (i = 0; i < sizeof(unarmed_skills); i++)
                if (best_skill->subtype == unarmed_skills[i])
                    break;
            if (i < sizeof(unarmed_skills))
                return (best_skill);
        }
        /* If for some reason the unarmed_skill is not valid, we fall
         * through the processing below.
         */
    }

    /* Dragons are a special case - gros 25th July 2006 */
    /* Perhaps this special case should be removed and unarmed_skill
     * set to clawing for dragon characters?  MSW 2009-07-03
     */
    if (is_dragon_pl(op))
    {
        object *tmp;

        tmp = find_skill_by_number(op, SK_CLAWING);
        if (tmp) /* I suppose it should always be true - but maybe there's
                  * draconic toothache ? :) */
            return tmp;
    }

    last_skill = sizeof(unarmed_skills);
    FOR_INV_PREPARE(op, tmp)
    {
        if (tmp->type == SKILL)
        {
            int i;

            /* The order in the array is preferred order.  So basically,
             * we just cut down the number to search - eg, if we find a skill
             * early on in flame touch, then we only need to look into the unarmed_array
             * to the entry before flame touch - don't care about the entries afterward,
             * because they are inferior skills.
             * if we end up finding the best skill (i==0) might as well return
             * right away - can't get any better than that.
             */
            for (i = 0; i < last_skill; i++)
            {
                if (tmp->subtype == unarmed_skills[i] && QUERY_FLAG(tmp, FLAG_CAN_USE_SKILL))
                {
                    best_skill = tmp;
                    last_skill = i;
                    if (i == 0)
                        return best_skill;
                }
            }
        }
    }
    FOR_INV_FINISH();
    return best_skill;
}

/**
 * We have got an appropriate opponent from either
 * move_player_attack() or skill_attack(). In this part we get on with
 * attacking, take care of messages from the attack and changes in invisible.
 * @param tmp
 * targeted monster.
 * @param op
 * what is attacking.
 * @param string
 * describes the damage ("claw", "punch", ...).
 * @param skill
 * skill used to damage.
 */
static void do_skill_attack(object *tmp, object *op, const char *string, object *skill)
{
    int success;

    /* For Players only: if there is no ready weapon, and no "attack" skill
     * is readied either then try to find a skill for the player to use.
     * it is presumed that if skill is set, it is a valid attack skill (eg,
     * the caller should have set it appropriately).  We still want to pass
     * through that code if skill is set to change to the skill.
     */
    if (op->type == PLAYER)
    {
        if (!QUERY_FLAG(op, FLAG_READY_WEAPON))
        {
            size_t i;

            if (!skill)
            {
                /* See if the players chosen skill is a combat skill, and use
                 * it if appropriate.
                 */
                if (op->chosen_skill)
                {
                    /* the list is 0-terminated, and talismans, which can be in chosen_skill,
                     * have a subtype of 0, therefore don't check the 0 */
                    for (i = 0; unarmed_skills[i] != 0; i++)
                        if (op->chosen_skill->subtype == unarmed_skills[i])
                        {
                            skill = op->chosen_skill;
                            break;
                        }
                }
                /* If we didn't find a skill above, look harder for a good skill */
                if (!skill)
                {
                    skill = find_best_player_hth_skill(op);

                    if (!skill)
                    {
                        draw_ext_info(NDI_BLACK, 0, op,
                                      MSG_TYPE_SKILL, MSG_TYPE_SKILL_MISSING,
                                      "You have no unarmed combat skills!");
                        return;
                    }
                }
            }
            if (skill != op->chosen_skill)
            {
                /* now try to ready the new skill */
                if (!change_skill(op, skill, 1))
                { /* oh oh, trouble! */
                    draw_ext_info_format(NDI_UNIQUE, 0, tmp,
                                         MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                                         "Couldn't change to skill %s",
                                         skill->name);
                    return;
                }
            }
        }
        else
        {
            /* Seen some crashes below where current_weapon is not set,
             * even though the flag says it is.  So if current weapon isn't set,
             * do some work in trying to find the object to use.
             */
            if (!op->current_weapon)
            {
                object *tmp;

                LOG(llevError, "Player %s does not have current weapon set but flag_ready_weapon is set\n", op->name);
                tmp = object_find_by_type_applied(op, WEAPON);
                if (!tmp)
                {
                    LOG(llevError, "Could not find applied weapon on %s\n", op->name);
                    op->current_weapon = NULL;
                    return;
                }
                else
                {
                    char weapon[MAX_BUF];

                    query_name(tmp, weapon, MAX_BUF);
                    op->current_weapon = tmp;
                }
            }

            /* Has ready weapon - make sure chosen_skill is set up properly */
            if (!op->chosen_skill || op->current_weapon->skill != op->chosen_skill->skill)
            {
                change_skill(op, find_skill_by_name(op, op->current_weapon->skill), 1);
            }
        }
    }

    /* lose invisibility/hiding status for running attacks */

    if (op->type == PLAYER && op->contr->tmp_invis)
    {
        op->contr->tmp_invis = 0;
        op->invisible = 0;
        op->hide = 0;
        object_update(op, UP_OBJ_FACE);
    }

    success = attack_ob(tmp, op);

    /* print appropriate  messages to the player */

    if (success && string != NULL && tmp && !QUERY_FLAG(tmp, FLAG_FREED))
    {
        char op_name[MAX_BUF];

        if (op->type == PLAYER)
        {
            query_name(tmp, op_name, MAX_BUF);
            draw_ext_info_format(NDI_UNIQUE, 0, op,
                                 MSG_TYPE_ATTACK, MSG_TYPE_ATTACK_DID_HIT,
                                 "You %s %s!",
                                 string, op_name);
        }
        else if (tmp->type == PLAYER)
        {
            query_name(op, op_name, MAX_BUF);
            draw_ext_info_format(NDI_UNIQUE, 0, tmp,
                                 MSG_TYPE_VICTIM, MSG_TYPE_VICTIM_WAS_HIT,
                                 "%s %s you!",
                                 op_name, string);
        }
    }
    else if (tmp && !QUERY_FLAG(tmp, FLAG_FREED))
    {
        char op_name[MAX_BUF];

        query_name(tmp, op_name, MAX_BUF);
        draw_ext_info_format(NDI_UNIQUE, 0, op,
                             MSG_TYPE_ATTACK, MSG_TYPE_ATTACK_MISS,
                             "You miss %s!",
                             op_name);
    }
}

/**
 * Core routine for use when we attack using a skills system.
 * In essence, this code handles
 * all skill-based attacks, ie hth, missile and melee weapons should be
 * treated here. If an opponent is already supplied by move_player(),
 * we move right onto do_skill_attack(), otherwise we find if an
 * appropriate opponent exists.
 *
 * This is called by move_player() and attack_hth()
 *
 * Initial implementation by -bt thomas@astro.psu.edu
 *
 * @param tmp
 * victim. Can be NULL.
 * @param pl
 * who is attacking.
 * @param dir
 * direction to attack.
 * @param string
 * describes the damage ("claw", "punch", ...).
 * @param skill
 * attack skill.
 */
void skill_attack(object *tmp, object *pl, int dir, const char *string, object *skill)
{
    sint16 tx, ty;
    mapstruct *m;
    int mflags;

    if (!dir)
        dir = pl->facing;
    tx = freearr_x[dir];
    ty = freearr_y[dir];

    /* If we don't yet have an opponent, find if one exists, and attack.
     * Legal opponents are the same as outlined in move_player_attack()
     */

    if (tmp == NULL)
    {
        m = pl->map;
        tx = pl->x + freearr_x[dir];
        ty = pl->y + freearr_y[dir];

        mflags = get_map_flags(m, &m, tx, ty, &tx, &ty);
        if (mflags & P_OUT_OF_MAP)
            return;

        /* space must be blocked for there to be anything interesting to do */
        if (!(mflags & P_IS_ALIVE) && !OB_TYPE_MOVE_BLOCK(pl, GET_MAP_MOVE_BLOCK(m, tx, ty)))
        {
            return;
        }

        FOR_MAP_PREPARE(m, tx, ty, tmp2)
        if ((QUERY_FLAG(tmp2, FLAG_ALIVE) && tmp2->stats.hp >= 0) || QUERY_FLAG(tmp2, FLAG_CAN_ROLL) || tmp2->type == LOCKED_DOOR)
        {
            /* Don't attack party members */
            if ((pl->type == PLAYER && tmp2->type == PLAYER) && (pl->contr->party != NULL && pl->contr->party == tmp2->contr->party))
                return;
            tmp = tmp2;
            break;
        }
        FOR_MAP_FINISH();
    }
    if (!tmp)
    {
        if (pl->type == PLAYER)
            draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                          "There is nothing to attack!");
        return;
    }

    do_skill_attack(tmp, pl, string, skill);
}

/**
 * This handles all hand-to-hand attacks.
 *
 * Will unapply equipped weapons if needed.
 *
 * July 5, 1995 - I broke up attack_hth() into 2 parts. In the first
 * (attack_hth) we check for weapon use, etc in the second (the new
 * function skill_attack() we actually attack.
 *
 * @param pl
 * object attacking.
 * @param dir
 * attack direction.
 * @param string
 * describes the attack ("claw", "punch", ...).
 * @param skill
 * attack skill used.
 */
static void attack_hth(object *pl, int dir, const char *string, object *skill)
{
    object *weapon;

    if (QUERY_FLAG(pl, FLAG_READY_WEAPON))
    {
        weapon = object_find_by_type_applied(pl, WEAPON);
        if (weapon != NULL)
        {
            if (apply_special(pl, weapon, AP_UNAPPLY | AP_NOPRINT))
            {
                char weaponname[MAX_BUF];

                query_name(weapon, weaponname, MAX_BUF);
                draw_ext_info_format(NDI_UNIQUE, 0, pl,
                                     MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                                     "You are unable to unwield %s in order to attack with %s.",
                                     weaponname, skill->name);
                return;
            }
            else
            {
                draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                              "You unwield your weapon in order to attack.");
            }
        }
    }
    skill_attack(NULL, pl, dir, string, skill);
}

/**
 * This handles melee weapon attacks -b.t.
 *
 * For now we are just checking to see if we have a ready weapon here.
 * But there is a real neato possible feature of this scheme which
 * bears mentioning:
 * Since we are only calling this from do_skill() in the future
 * we may make this routine handle 'special' melee weapons attacks
 * (like disarming maneuver with sai) based on player SK_level and
 * weapon type.
 *
 * @param op
 * living thing attacking.
 * @param dir
 * attack direction.
 * @param string
 * describes the attack ("claw", "punch", ...).
 * @param skill
 * attack skill used.
 */
static void attack_melee_weapon(object *op, int dir, const char *string, object *skill)
{
    if (!QUERY_FLAG(op, FLAG_READY_WEAPON))
    {
        if (op->type == PLAYER)
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_ERROR,
                          "You have no ready weapon to attack with!");
        return;
    }
    skill_attack(NULL, op, dir, string, skill);
}
