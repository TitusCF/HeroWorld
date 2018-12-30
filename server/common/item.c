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
 * Those functions deal with objects in general, including description, body location, and such.
 *
 * @todo
 * put const char *instead of char *when possible.
 */

#include <stdio.h>
#include <assert.h>
#include <global.h>
#include <living.h>
#include <spells.h>

/**
 * The ordering of this is actually doesn't make a difference
 * However, for ease of use, new entries should go at the end
 * so those people that debug the code that get used to something
 * being in the location 4 don't get confused.
 *
 * The ordering in save_name, use_name, nonuse_name.
 * save_name is the name used to load/save it from files.  It should
 * match that of the doc/Developers/objects.  The only
 * real limitation is that it shouldn't have spaces or other characters
 * that may mess up the match code.  It must also start with body_
 * use_name is how we describe the location if we can use it.
 * nonuse_name is how we describe it if we can't use it.  I think
 * the values below will make it pretty clear how those work out
 * They are basically there to make life a little easier - if a character
 * examines an item and it says it goes on 'your arm', its pretty clear
 * they can use it.  See the last sample (commented out) for a dragon
 * Note that using the term 'human' may not be very accurate, humanoid
 * may be better.
 * Basically, for the use/nonuse, the code does something like:
 * "This item goes %s\n", with the use/nonuse values filling in the %s
 */
body_locations_struct body_locations[NUM_BODY_LOCATIONS] = {
    { "body_range",    "in your range slot",     "in a human's range slot" },
    { "body_arm",      "on your arm",            "on a human's arm" },
    { "body_torso",    "on your body",           "on a human's torso" },
    { "body_head",     "on your head",           "on a human's head" },
    { "body_neck",     "around your neck",       "around a humans neck" },
    { "body_skill",    "in your skill slot",     "in a human's skill slot" },
    { "body_finger",   "on your finger",         "on a human's finger" },
    { "body_shoulder", "around your shoulders",  "around a human's shoulders" },
    { "body_foot",     "on your feet",           "on a human's feet" },
    { "body_hand",     "on your hands",          "on a human's hands" },
    { "body_wrist",    "around your wrists",     "around a human's wrist" },
    { "body_waist",    "around your waist",      "around a human's waist" },
    { "body_leg",      "around your legs",       "around a human's legs" },
    /*{"body_dragon_torso", "your body", "a dragon's body"} */
};

/** Tens */
static const char *const numbers_10[] = {
    "zero", "ten", "twenty", "thirty", "fourty", "fifty", "sixty", "seventy",
    "eighty", "ninety"
};

/** Levels as a full name and not a number. */
static const char *const levelnumbers[] = {
    "zeroth", "first", "second", "third", "fourth", "fifth", "sixth", "seventh",
    "eighth", "ninth", "tenth", "eleventh", "twelfth", "thirteenth",
    "fourteenth", "fifteenth", "sixteenth", "seventeenth", "eighteenth",
    "nineteenth", "twentieth"
};

/** Tens for levels */
static const char *const levelnumbers_10[] = {
    "zeroth", "tenth", "twentieth", "thirtieth", "fortieth", "fiftieth", "sixtieth",
    "seventieth", "eightieth", "ninetieth"
};

/**
 * The following is a large table of item types, the fields are:
 * item number, item name, item name (plural), and two numbers that are the skills
 * used to identify them. Anytime a new item type is added or removed, this list
 * should be altered to reflect that. The defines for the numerical values are in
 * define.h
 */
static const typedata item_types[] = {
    { 0, "none", "none", 0, 0 },
    { PLAYER, "player", "players", 0, 0 },
    { ROD, "rod", "rods", SK_THAUMATURGY, 0 },
    { TREASURE, "treasure", "treasure", 0, 0 },
    { POTION, "potion", "potions", SK_ALCHEMY, 0 },
    { FOOD, "food", "food", SK_WOODSMAN, 0 },
    { POISON, "poison", "poisons", SK_ALCHEMY, 0 },
    { BOOK, "book", "books", SK_LITERACY, 0 },
    { CLOCK, "clock", "clocks", 0, 0 },
    { ARROW, "arrow", "arrows", SK_BOWYER, 0 },
    { BOW, "bow", "bows", SK_BOWYER, 0 },
    { WEAPON, "weapon", "weapons", SK_SMITHERY, 0 },
    { ARMOUR, "armour", "armour", SK_SMITHERY, 0 },
    { PEDESTAL, "pedestal", "pedestals", 0, 0 },
    { ALTAR, "altar", "altars", 0, 0 },
    { LOCKED_DOOR, "locked door", "locked doors", 0, 0 },
    { SPECIAL_KEY, "special key", "special keys", 0, 0 },
    { MAP, "map", "maps", 0, 0 },
    { DOOR, "door", "doors", 0, 0 },
    { KEY, "key", "keys", 0, 0 },
    { TIMED_GATE, "timed_gate", "timed_gates", 0, 0 },
    { TRIGGER, "trigger", "triggers", 0, 0 },
    { GRIMREAPER, "grimreaper", "grimreapers", 0, 0 },
    { MAGIC_EAR, "magic ear", "magic ears", 0, 0 },
    { TRIGGER_BUTTON, "trigger button", "trigger buttons", 0, 0 },
    { TRIGGER_ALTAR, "trigger altar", "trigger altars", 0, 0 },
    { TRIGGER_PEDESTAL, "trigger pedestal", "trigger pedestals", 0, 0 },
    { SHIELD, "shield", "shields", SK_SMITHERY, 0 },
    { HELMET, "helmet", "helmets", SK_SMITHERY, 0 },
    { MONEY, "money", "money", 0, 0 },
    { CLASS, "class", "classes", 0, 0 },
    { AMULET, "amulet", "amulets", SK_JEWELER, 0 },
    { PLAYERMOVER, "player mover", "player movers", 0, 0 },
    { TELEPORTER, "teleporter", "teleporters", 0, 0 },
    { CREATOR, "creator", "creators", 0, 0 },
    { SKILL, "skill", "skills", 0, 0 },
    { EARTHWALL, "earthwall", "earthwalls", 0, 0 },
    { GOLEM, "golem", "golems", 0, 0 },
    { THROWN_OBJ, "projectile", "projectiles", 0, 0 },
    { BLINDNESS, "blindness", "blindness", 0, 0 },
    { GOD, "god", "gods", 0, 0 },
    { DETECTOR, "detector", "detectors", 0, 0 },
    { TRIGGER_MARKER, "trigger marker", "trigger markers", 0, 0 },
    { DEAD_OBJECT, "dead object", "dead objects", 0, 0 },
    { DRINK, "drink", "drinks", SK_WOODSMAN, SK_ALCHEMY },
    { MARKER, "marker", "markers", 0, 0 },
    { HOLY_ALTAR, "holy altar", "holy altars", 0, 0 },
    { PLAYER_CHANGER, "player changer", "player changers", 0, 0 },
    { BATTLEGROUND, "battleground", "battlegrounds", 0, 0 },
    { PEACEMAKER, "peacemaker", "peacemakers", 0, 0 },
    { GEM, "gem", "gems", SK_JEWELER, 0 },
    { FIREWALL, "firewall", "firewalls", 0, 0 },
    { CHECK_INV, "inventory checker", "inventory checkers", 0, 0 },
    { MOOD_FLOOR, "mood floor", "mood floors", 0, 0 },
    { EXIT, "exit", "exits", 0, 0 },
    { ENCOUNTER, "encounter", "encounters", 0, 0 },
    { SHOP_FLOOR, "shop floor", "shop floors", 0, 0 },
    { SHOP_MAT, "shop mat", "shop mats", 0, 0 },
    { RING, "ring", "rings", SK_JEWELER, 0 },
    { FLOOR, "floor", "floors", 0, 0 },
    { FLESH, "flesh", "flesh", SK_WOODSMAN, 0 },
    { INORGANIC, "inorganic", "inorganics", SK_ALCHEMY, 0 },
    { SKILL_TOOL, "skill tool", "skill tools", 0, 0 },
    { LIGHTER, "lighter", "lighters", 0, 0 },
    { WALL, "wall", "walls", 0, 0 },
    { MISC_OBJECT, "bric-a-brac", "bric-a-brac", 0, 0 },
    { MONSTER, "monster", "monsters", 0, 0 },
    { LAMP, "lamp", "lamps", 0, 0 },
    { DUPLICATOR, "duplicator", "duplicators", 0, 0 },
    { SPELLBOOK, "spellbook", "spellbooks", SK_LITERACY, 0 },
    { CLOAK, "cloak", "cloaks", SK_SMITHERY, 0 },
    { SPINNER, "spinner", "spinners", 0, 0 },
    { GATE, "gate", "gates", 0, 0 },
    { BUTTON, "button", "buttons", 0, 0 },
    { CF_HANDLE, "cf handle", "cf handles", 0, 0 },
    { HOLE, "hole", "holes", 0, 0 },
    { TRAPDOOR, "trapdoor", "trapdoors", 0, 0 },
    { SIGN, "sign", "signs", 0, 0 },
    { BOOTS, "boots", "boots", SK_SMITHERY, 0 },
    { GLOVES, "gloves", "gloves", SK_SMITHERY, 0 },
    { SPELL, "spell", "spells", 0, 0 },
    { SPELL_EFFECT, "spell effect", "spell effects", 0, 0 },
    { CONVERTER, "converter", "converters", 0, 0 },
    { BRACERS, "bracers", "bracers", SK_SMITHERY, 0 },
    { POISONING, "poisoning", "poisonings", 0, 0 },
    { SAVEBED, "savebed", "savebeds", 0, 0 },
    { WAND, "wand", "wands", SK_THAUMATURGY, 0 },
    { SCROLL, "scroll", "scrolls", SK_LITERACY, 0 },
    { DIRECTOR, "director", "directors", 0, 0 },
    { GIRDLE, "girdle", "girdles", SK_SMITHERY, 0 },
    { FORCE, "force", "forces", 0, 0 },
    { POTION_RESIST_EFFECT, "potion effect", "potion effects", 0, 0 },
    { CLOSE_CON, "closed container", "closed container", 0, 0 },
    { CONTAINER, "container", "containers", SK_ALCHEMY, 0 },
    { ARMOUR_IMPROVER, "armour improver", "armour improvers", SK_LITERACY, 0 },
    { WEAPON_IMPROVER, "weapon improver", "weapon improvers", SK_LITERACY, 0 },
    { SKILLSCROLL, "skillscroll", "skillscrolls", SK_LITERACY, 0 },
    { DEEP_SWAMP, "deep swamp", "deep swamps", 0, 0 },
    { IDENTIFY_ALTAR, "identify altar", "identify altars", 0, 0 },
    { SHOP_INVENTORY, "inventory list", "inventory lists", 0, 0 },
    { RUNE, "rune", "runes", 0, 0 },
    { TRAP, "trap", "traps", 0, 0 },
    { POWER_CRYSTAL, "power_crystal", "power_crystals", 0, 0 },
    { CORPSE, "corpse", "corpses", 0, 0 },
    { DISEASE, "disease", "diseases", 0, 0 },
    { SYMPTOM, "symptom", "symptoms", 0, 0 },
    { BUILDER, "item builder", "item builders", 0, 0 },
    { MATERIAL, "building material", "building materials", 0, 0 },
};

/** Number of items in ::item_types array. */
static const int item_types_size = sizeof(item_types)/sizeof(*item_types);

/** This curve may be too steep.  But the point is that there should
 * be tough choices - there is no real point to this if everyone can
 * wear whatever they want with no worries.  Perhaps having the steep
 * curve is good (maybe even steeper), but allowing players to
 * have 2 * level instead.  Ideally, top level characters should only be
 * able to use 2-3 of the most powerful items.
 * note that this table is only really used for program generated items -
 * custom objects can use whatever they want.
 */
static const int enc_to_item_power[] = {
    0, 0, 1, 2, 3, 4,    /* 5 */
    5, 7, 9, 11, 13,    /* 10 */
    15, 18, 21, 24, 27, /* 15 */
    30, 35, 40, 45, 50  /* 20 */
};

int get_power_from_ench(int ench) {
    if (ench < 0)
        ench = 0;
    if (ench > (int)(sizeof(enc_to_item_power)/sizeof(*enc_to_item_power)-1))
        ench = sizeof(enc_to_item_power)/sizeof(*enc_to_item_power)-1;
    return enc_to_item_power[ench];
}

/**
 * This takes an object 'op' and figures out what its item_power
 * rating should be.  This should only really be used by the treasure
 * generation code, and when loading legacy objects.  It returns
 * the item_power it calculates.
 *
 * @param op
 * object of which to compute the item_power
 * @return op's item power.
 */
int calc_item_power(const object *op) {
    int i, tmp, enc;

    enc = 0;
    for (i = 0; i < NUM_STATS; i++)
        enc += get_attr_value(&op->stats, i);

    /* This protection logic is pretty flawed.  20% fire resistance
     * is much more valuable than 20% confusion, or 20% slow, or
     * several others.  Start at 1 - ignore physical - all that normal
     * armour shouldn't be counted against
     */
    tmp = 0;
    for (i = 1; i < NROFATTACKS; i++)
        tmp += op->resist[i];

    /* Add/substract 10 so that the rounding works out right */
    if (tmp > 0)
        enc += (tmp+10)/20;
    else if (tmp < 0)
        enc += (tmp-10)/20;

    enc += op->magic;

    /* For each attacktype a weapon has, one more encantment.  Start at 1 -
     * physical doesn't count against total.
     */
    if (op->type == WEAPON) {
        for (i = 1; i < NROFATTACKS; i++)
            if (op->attacktype&(1<<i))
                enc++;
        if (op->slaying)
            enc += 2;     /* What it slays is probably more relevent */
    }
    /* Items the player can equip */
    if ((op->type == WEAPON)
    || (op->type == ARMOUR)
    || (op->type == HELMET)
    || (op->type == SHIELD)
    || (op->type == RING)
    || (op->type == BOOTS)
    || (op->type == GLOVES)
    || (op->type == AMULET)
    || (op->type == GIRDLE)
    || (op->type == BRACERS)
    || (op->type == CLOAK)) {
        enc += op->stats.food;     /* sustenance */
        enc += op->stats.hp;       /* hp regen */
        enc += op->stats.sp;       /* mana regen */
        enc += op->stats.grace;    /* grace regen */
        enc += op->stats.exp;      /* speed bonus */
    }
    enc += op->stats.luck;

    /* Do spell paths now */
    for (i = 1; i < NRSPELLPATHS; i++) {
        if (op->path_attuned&(1<<i))
            enc++;
        else if (op->path_denied&(1<<i))
            enc -= 2;
        else if (op->path_repelled&(1<<i))
            enc--;
    }

    if (QUERY_FLAG(op, FLAG_LIFESAVE))
        enc += 5;
    if (QUERY_FLAG(op, FLAG_REFL_SPELL))
        enc += 3;
    if (QUERY_FLAG(op, FLAG_REFL_MISSILE))
        enc += 2;
    if (QUERY_FLAG(op, FLAG_STEALTH))
        enc += 1;
    if (QUERY_FLAG(op, FLAG_XRAYS))
        enc += 2;
    if (QUERY_FLAG(op, FLAG_SEE_IN_DARK))
        enc += 1;
    if (QUERY_FLAG(op, FLAG_MAKE_INVIS))
        enc += 1;

    return get_power_from_ench(enc);
}

/**
 * @param itemtype
 * item type for which to return typedata.
 * @return
 * typedata that has a number equal to itemtype, if there
 * isn't one, returns NULL */
const typedata *get_typedata(int itemtype) {
    int i;

    for (i = 0; i < item_types_size; i++)
        if (item_types[i].number == itemtype)
            return &item_types[i];
    return NULL;
}

/**
 * @param name
 * item name for which to return typedata. Singular form is preferred.
 * @return
 * typedata that has a name equal to itemtype, if there
 * isn't one, return the plural name that matches, if there still isn't
 * one return NULL
 *
 * @note
 * will emit an Info if found by plural form.
 */
const typedata *get_typedata_by_name(const char *name) {
    int i;

    for (i = 0; i < item_types_size; i++)
        if (!strcmp(item_types[i].name, name))
            return &item_types[i];
    for (i = 0; i < item_types_size; i++)
        if (!strcmp(item_types[i].name_pl, name)) {
            LOG(llevInfo, "get_typedata_by_name: I have been sent the plural %s, the singular form %s is preffered\n", name, item_types[i].name);
            return &item_types[i];
        }
    return NULL;
}

/**
 * Generates the visible naming for resistances.
 *
 * @param op
 * object we want information about.
 * @param newline
 * If TRUE, we don't put parens around the description
 * but do put a newline at the end. Useful when dumping to files
 * @param buf
 * buffer that will receive the description. Can be NULL.
 * @return buf, a new StringBuffer the caller should free if buf was NULL.
 */
StringBuffer *describe_resistance(const object *op, int newline, StringBuffer *buf) {
    int tmpvar;

    if (buf == NULL)
        buf = stringbuffer_new();

    for (tmpvar = 0; tmpvar < NROFATTACKS; tmpvar++) {
        if (op->resist[tmpvar] && (op->type != FLESH || atnr_is_dragon_enabled(tmpvar) == 1)) {
            if (!newline)
                stringbuffer_append_printf(buf, "(%s %+d)", resist_plus[tmpvar], op->resist[tmpvar]);
            else
                stringbuffer_append_printf(buf, "%s %d\n", resist_plus[tmpvar], op->resist[tmpvar]);
        }
    }

    return buf;
}

/**
 * Formats the item's weight.
 * @param op
 * object we want the weight of.
 * @param[out] buf
 * buffer to write to.
 * @param size
 * buffer size.
 */
void query_weight(const object *op, char *buf, size_t size) {
    sint32 i = (op->nrof ? op->nrof : 1)*op->weight+op->carrying;

    if (op->weight < 0)
        snprintf(buf, size, "      ");
    else if (i%1000)
        snprintf(buf, size, "%6.1f", i/1000.0);
    else
        snprintf(buf, size, "%4d  ", i/1000);
}

/**
 * Formats a level.
 * @param i
 * level to format.
 * @param[out] buf
 * buffer which will contain the level.
 * @param size
 * size of the buffer.
 */
void get_levelnumber(int i, char *buf, size_t size) {
    if (i > 99 || i < 0) {
        snprintf(buf, size, "%d.", i);
        return;
    }
    if (i < 21) {
        snprintf(buf, size, "%s", levelnumbers[i]);
        return;
    }
    if (!(i%10)) {
        snprintf(buf, size, "%s", levelnumbers_10[i/10]);
        return;
    }
    snprintf(buf, size, "%s%s", numbers_10[i/10], levelnumbers[i%10]);
    return;
}

/**
 * Describes a ring or amulet, or a skill.
 * @param op
 * item to describe, must be RING, AMULET or SKILL.
 * @param buf
 * buffer that will contain the description. If NULL a new one is created.
 * @return buf, or a new StringBuffer the caller should free if buf was NULL.
 * @todo why does this also describe a SKILL?
 */
static StringBuffer *ring_desc(const object *op, StringBuffer *buf) {
    int attr, val;
    size_t len;

    assert(op != NULL);
    assert(op->type == RING || op->type == AMULET || op->type == SKILL);

    if (buf == NULL)
        buf = stringbuffer_new();
    len = stringbuffer_length(buf);

    if (!QUERY_FLAG(op, FLAG_IDENTIFIED))
        return buf;

    for (attr = 0; attr < NUM_STATS; attr++) {
        if ((val = get_attr_value(&(op->stats), attr)) != 0) {
            stringbuffer_append_printf(buf, "(%s%+d)", short_stat_name[attr], val);
        }
    }
    if (op->stats.exp)
        stringbuffer_append_printf(buf, "(speed %+"FMT64")", op->stats.exp);
    if (op->stats.wc)
        stringbuffer_append_printf(buf, "(wc%+d)", op->stats.wc);
    if (op->stats.dam)
        stringbuffer_append_printf(buf, "(dam%+d)", op->stats.dam);
    if (op->stats.ac)
        stringbuffer_append_printf(buf, "(ac%+d)", op->stats.ac);

    describe_resistance(op, 0, buf);

    if (op->stats.food != 0)
        stringbuffer_append_printf(buf, "(sustenance%+d)", op->stats.food);
    if (op->stats.grace)
        stringbuffer_append_printf(buf, "(grace%+d)", op->stats.grace);
    if (op->stats.sp && op->type != SKILL)
        stringbuffer_append_printf(buf, "(magic%+d)", op->stats.sp);
    if (op->stats.hp)
        stringbuffer_append_printf(buf, "(regeneration%+d)", op->stats.hp);
    if (op->stats.luck)
        stringbuffer_append_printf(buf, "(luck%+d)", op->stats.luck);
    if (QUERY_FLAG(op, FLAG_LIFESAVE))
        stringbuffer_append_printf(buf, "(lifesaving)");
    if (QUERY_FLAG(op, FLAG_REFL_SPELL))
        stringbuffer_append_printf(buf, "(reflect spells)");
    if (QUERY_FLAG(op, FLAG_REFL_MISSILE))
        stringbuffer_append_printf(buf, "(reflect missiles)");
    if (QUERY_FLAG(op, FLAG_STEALTH))
        stringbuffer_append_printf(buf, "(stealth)");
    if (op->glow_radius)
        stringbuffer_append_string(buf, "(glowing)");

    describe_spellpath_attenuation("Attuned", op->path_attuned, buf);
    describe_spellpath_attenuation("Repelled", op->path_repelled, buf);
    describe_spellpath_attenuation("Denied", op->path_denied, buf);

    /* item_power is done by the caller */
 /*   if (op->item_power)
        snprintf(buf+strlen(buf), size-strlen(buf), "(item_power %+d)", op->item_power);*/
    if (stringbuffer_length(buf) == len && op->type != SKILL)
        stringbuffer_append_string(buf, "of adornment");

    return buf;
}

/**
 * query_short_name(object) is similar to query_name(), but doesn't
 * contain any information about object status (worn/cursed/etc.)
 *
 * @param op
 * object to describe.
 * @param buf
 * buffer which will contain the name. Must not be NULL.
 * @param size
 * buffer length.
 */
void query_short_name(const object *op, char *buf, size_t size) {
    size_t len = 0;

    if (op->name == NULL) {
        snprintf(buf, size, "(null)");
        return;
    }
    if (!op->nrof && !op->weight && !op->title && !is_magical(op)) {
        snprintf(buf, size, "%s", op->name); /* To speed things up (or make things slower?) */
        return;
    }
    buf[0] = '\0';

    if (op->nrof <= 1)
        safe_strcat(buf, op->name, &len, size);
    else
        safe_strcat(buf, op->name_pl, &len, size);

    if (op->title && QUERY_FLAG(op, FLAG_IDENTIFIED)) {
        safe_strcat(buf, " ", &len, size);
        safe_strcat(buf, op->title, &len, size);
    }

    switch (op->type) {
    case SPELLBOOK:
    case SCROLL:
    case WAND:
    case ROD:
        if (QUERY_FLAG(op, FLAG_IDENTIFIED) || QUERY_FLAG(op, FLAG_BEEN_APPLIED)) {
            if (!op->title) {
                safe_strcat(buf, " of ", &len, size);
                if (op->inv)
                    safe_strcat(buf, op->inv->name, &len, size);
                else
                    LOG(llevError, "Spellbook %s lacks inventory\n", op->name);
            }
            if (op->type != SPELLBOOK) {
                snprintf(buf+len, size-len, " (lvl %d)", op->level);
                len += strlen(buf+len);
            }
        }
        break;

    case SKILL:
    case AMULET:
    case RING:
        if (!op->title) {
            /* If ring has a title, full description isn't so useful */
            char* desc;

            desc = stringbuffer_finish(ring_desc(op, NULL));
            if (desc[0]) {
                safe_strcat(buf, " ", &len, size);
                safe_strcat(buf, desc, &len, size);
            }
            free(desc);
        }
        break;

    default:
        if (op->magic
        && ((QUERY_FLAG(op, FLAG_BEEN_APPLIED) && need_identify(op)) || QUERY_FLAG(op, FLAG_IDENTIFIED))) {
            snprintf(buf+len, size-len, " %+d", op->magic);
            len += strlen(buf+len);
        }
    }
}

/**
 * Describes an item.
 *
 * @param op
 * item to describe. Must not be NULL.
 * @param buf
 * buffer that will contain the description.
 * @param size
 * size of buffer.
 */
void query_name(const object *op, char *buf, size_t size) {
    size_t len = 0;

    buf[0] = '\0';

    query_short_name(op, buf+len, size-len);
    len += strlen(buf+len);

    if (QUERY_FLAG(op, FLAG_INV_LOCKED))
        safe_strcat(buf, " *", &len, size);
    if (op->type == CONTAINER
    && ((op->env && op->env->container == op) || (!op->env && QUERY_FLAG(op, FLAG_APPLIED))))
        safe_strcat(buf, " (open)", &len, size);

    if (QUERY_FLAG(op, FLAG_KNOWN_CURSED)) {
        if (QUERY_FLAG(op, FLAG_DAMNED))
            safe_strcat(buf, " (damned)", &len, size);
        else if (QUERY_FLAG(op, FLAG_CURSED))
            safe_strcat(buf, " (cursed)", &len, size);
    }
    if (QUERY_FLAG(op, FLAG_BLESSED) && QUERY_FLAG(op, FLAG_KNOWN_BLESSED))
        safe_strcat(buf, " (blessed)", &len, size);

    /* Basically, if the object is known magical (detect magic spell on it),
     * and it isn't identified,  print out the fact that
     * it is magical.  Assume that the detect magical spell will only set
     * KNOWN_MAGICAL if the item actually is magical.
     *
     * Changed in V 0.91.4 - still print that the object is magical even
     * if it has been applied.  Equipping an item does not tell full
     * abilities, especially for artifact items.
     */
    if (QUERY_FLAG(op, FLAG_KNOWN_MAGICAL) && !QUERY_FLAG(op, FLAG_IDENTIFIED))
        safe_strcat(buf, " (magic)", &len, size);

    if (QUERY_FLAG(op, FLAG_APPLIED)) {
        switch (op->type) {
        case BOW:
        case WAND:
        case ROD:
            safe_strcat(buf, " (readied)", &len, size);
            break;

        case WEAPON:
            safe_strcat(buf, " (wielded)", &len, size);
            break;

        case ARMOUR:
        case HELMET:
        case SHIELD:
        case RING:
        case BOOTS:
        case GLOVES:
        case AMULET:
        case GIRDLE:
        case BRACERS:
        case CLOAK:
            safe_strcat(buf, " (worn)", &len, size);
            break;

        case CONTAINER:
            safe_strcat(buf, " (active)", &len, size);
            break;

        case SKILL:
        default:
            safe_strcat(buf, " (applied)", &len, size);
        }
    }
    if (QUERY_FLAG(op, FLAG_UNPAID))
        safe_strcat(buf, " (unpaid)", &len, size);
}

/**
 * Query a short name for the item.
 *
 * This is a lot like query_name(), but we
 * don't include the item count or item status.  Used for inventory sorting
 * and sending to client.
 * If plural is set, we generate the plural name of this.
 *
 * @param op
 * item we want the name of.
 * @param plural
 * whether to get the singular or plural name
 * @param buf
 * buffer that will contain the object's name. Must not be NULL.
 * @param size
 * buffer's length
 */
void query_base_name(const object *op, int plural, char *buf, size_t size) {
    size_t len;

    if ((!plural && !op->name)
    || (plural && !op->name_pl)) {
        strncpy(buf, "(null)", size);
        return;
    }

    if (!op->nrof && !op->weight && !op->title && !is_magical(op)) {
        strncpy(buf, op->name, size); /* To speed things up (or make things slower?) */
        return;
    }

    buf[0] = '\0';

    snprintf(buf, size, "%s", plural ? op->name_pl : op->name);
    len = strlen(buf);

    if (op->title && QUERY_FLAG(op, FLAG_IDENTIFIED)) {
        safe_strcat(buf, " ", &len, size);
        safe_strcat(buf, op->title, &len, size);
    }

    switch (op->type) {
    case SPELLBOOK:
    case SCROLL:
    case WAND:
    case ROD:
        if (QUERY_FLAG(op, FLAG_IDENTIFIED) || QUERY_FLAG(op, FLAG_BEEN_APPLIED)) {
            if (!op->title) {
                safe_strcat(buf, " of ", &len, size);
                if (op->inv)
                    safe_strcat(buf, op->inv->name, &len, size);
                else
                    LOG(llevError, "Spellbook %s lacks inventory\n", op->name);
            }
            if (op->type != SPELLBOOK) {
                snprintf(buf+len, size-len, " (lvl %d)", op->level);
                len += strlen(buf+len);
            }
        }
        break;

    case SKILL:
    case AMULET:
    case RING:
        if (!op->title) {
            /* If ring has a title, full description isn't so useful */
            char* s;

            s = stringbuffer_finish(ring_desc(op, NULL));
            if (s[0]) {
                safe_strcat(buf, " ", &len, size);
                safe_strcat(buf, s, &len, size);
            }
            free(s);
        }
        break;

    default:
        if (op->magic
        && ((QUERY_FLAG(op, FLAG_BEEN_APPLIED) && need_identify(op)) || QUERY_FLAG(op, FLAG_IDENTIFIED))) {
            snprintf(buf+strlen(buf), size-strlen(buf), " %+d", op->magic);
        }
    }
}

/**
 * Describes a monster.
 *
 * @param op
 * monster to describe. Must not be NULL, and must have FLAG_MONSTER or be a PLAYER.
 * @param buf
 * buffer that will contain the description. Can be NULL.
 * @return buf, or a new StringBuffer the caller should clear if buf was NULL.
 *
 * @todo
 * Rename to describe_living (or equivalent) since called for player too.
 * Fix weird sustenance logic.
 */
StringBuffer *describe_monster(const object *op, StringBuffer *buf) {
    int i;

    assert(op != NULL);
    assert(QUERY_FLAG(op, FLAG_MONSTER) || op->type == PLAYER);

    if (buf == NULL)
        buf = stringbuffer_new();

    /* Note that the resolution this provides for players really isn't
     * very good.  Any player with a speed greater than .67 will
     * fall into the 'lightning fast movement' category.
     */
    if (FABS(op->speed) > MIN_ACTIVE_SPEED) {
        switch ((int)((FABS(op->speed))*15)) {
        case 0:
            stringbuffer_append_string(buf, "(very slow movement)");
            break;

        case 1:
            stringbuffer_append_string(buf, "(slow movement)");
            break;

        case 2:
            stringbuffer_append_string(buf, "(normal movement)");
            break;

        case 3:
        case 4:
            stringbuffer_append_string(buf, "(fast movement)");
            break;

        case 5:
        case 6:
            stringbuffer_append_string(buf, "(very fast movement)");
            break;

        case 7:
        case 8:
        case 9:
        case 10:
            stringbuffer_append_string(buf, "(extremely fast movement)");
            break;

        default:
            stringbuffer_append_string(buf, "(lightning fast movement)");
            break;
        }
    }
    if (QUERY_FLAG(op, FLAG_UNDEAD))
        stringbuffer_append_string(buf, "(undead)");
    if (QUERY_FLAG(op, FLAG_SEE_INVISIBLE))
        stringbuffer_append_string(buf, "(see invisible)");
    if (QUERY_FLAG(op, FLAG_USE_WEAPON))
        stringbuffer_append_string(buf, "(wield weapon)");
    if (QUERY_FLAG(op, FLAG_USE_BOW))
        stringbuffer_append_string(buf, "(archer)");
    if (QUERY_FLAG(op, FLAG_USE_ARMOUR))
        stringbuffer_append_string(buf, "(wear armour)");
    if (QUERY_FLAG(op, FLAG_USE_RING))
        stringbuffer_append_string(buf, "(wear ring)");
    if (QUERY_FLAG(op, FLAG_USE_SCROLL))
        stringbuffer_append_string(buf, "(read scroll)");
    if (QUERY_FLAG(op, FLAG_USE_RANGE))
        stringbuffer_append_string(buf, "(fires wand/rod/horn)");
    if (QUERY_FLAG(op, FLAG_CAN_USE_SKILL))
        stringbuffer_append_string(buf, "(skill user)");
    if (QUERY_FLAG(op, FLAG_CAST_SPELL))
        stringbuffer_append_string(buf, "(spellcaster)");
    if (QUERY_FLAG(op, FLAG_FRIENDLY))
        stringbuffer_append_string(buf, "(friendly)");
    if (QUERY_FLAG(op, FLAG_UNAGGRESSIVE))
        stringbuffer_append_string(buf, "(unaggressive)");
    if (QUERY_FLAG(op, FLAG_HITBACK))
        stringbuffer_append_string(buf, "(hitback)");
    if (QUERY_FLAG(op, FLAG_STEALTH))
        stringbuffer_append_string(buf, "(stealthy)");
    if (op->randomitems != NULL) {
        treasure *t;
        int first = 1;

        for (t = op->randomitems->items; t != NULL; t = t->next)
            if (t->item && (t->item->clone.type == SPELL)) {
                if (first) {
                    first = 0;
                    stringbuffer_append_string(buf, "(Spell abilities:)");
                }
                stringbuffer_append_printf(buf, "(%s)",  t->item->clone.name);
            }
    }
    if (op->type == PLAYER) {
        if (op->contr->digestion) {
            if (op->contr->digestion != 0)
                stringbuffer_append_printf(buf, "(sustenance%+d)", op->contr->digestion);
        }
        if (op->contr->gen_grace) {
            stringbuffer_append_printf(buf, "(grace%+d)", op->contr->gen_grace);
        }
        if (op->contr->gen_sp) {
            stringbuffer_append_printf(buf, "(magic%+d)", op->contr->gen_sp);
        }
        if (op->contr->gen_hp) {
            stringbuffer_append_printf(buf, "(regeneration%+d)", op->contr->gen_hp);
        }
        if (op->stats.luck) {
            stringbuffer_append_printf(buf, "(luck%+d)", op->stats.luck);
        }
    }

    /* describe attacktypes */
    if (is_dragon_pl(op)) {
        /* for dragon players display the attacktypes from clawing skill
         * Break apart the for loop - move the comparison checking down -
         * this makes it more readable.
         */
        object *tmp;

        tmp = object_find_by_type_and_name(op, SKILL, "clawing");
        if (tmp && tmp->attacktype != 0) {
            describe_attacktype("Claws", tmp->attacktype, buf);
        } else {
            describe_attacktype("Attacks", op->attacktype, buf);
        }
    } else {
        describe_attacktype("Attacks", op->attacktype, buf);
    }
    describe_spellpath_attenuation("Attuned", op->path_attuned &~ op->path_denied, buf);
    describe_spellpath_attenuation("Repelled", op->path_repelled &~ op->path_denied, buf );
    describe_spellpath_attenuation("Denied", op->path_denied, buf);
    for (i = 0; i < NROFATTACKS; i++) {
        if (op->resist[i]) {
            stringbuffer_append_printf(buf, "(%s %+d)", resist_plus[i], op->resist[i]);
        }
    }

    return buf;
}

/**
 * Describes an item, in all its details.
 *
 * \li If it is a monster, lots of information about its abilities
 * will be returned.
 * \li If it is an item, lots of information about which abilities
 * will be gained about its user will be returned.
 * \li If it is a player, it writes out the current abilities
 * of the player, which is usually gained by the items applied.
 *
 * It would be really handy to actually pass another object
 * pointer on who is examining this object.  Then, you could reveal
 * certain information depending on what the examiner knows, eg,
 * wouldn't need to use the SEE_INVISIBLE flag to know it is
 * a dragon player examining food.  Could have things like
 * a dwarven axe, in which the full abilities are only known to
 * dwarves, etc.
 *
 * Add 'owner' who is the person examining this object.
 * owner can be null if no one is being associated with this
 * item (eg, debug dump or the like)
 *
 * @param op
 * object to describe. Must not be NULL.
 * @param owner
 * player examining the object.
 * @param buf
 * buffer that will contain the description. Can be NULL.
 * @return buf, or new StringBuffer the caller must free if buf was NULL.
 *
 * @note
 * This function is really much more complicated than it should
 * be, because different objects have different meanings
 * for the same field (eg, wands use 'food' for charges).  This
 * means these special cases need to be worked out.
 *
 * @todo
 * Check whether owner is really needed.
 */
StringBuffer *describe_item(const object *op, const object *owner, StringBuffer *buf) {
    int identified, i;

    if (buf == NULL)
        buf = stringbuffer_new();

    if (QUERY_FLAG(op, FLAG_MONSTER) || op->type == PLAYER) {
        return describe_monster(op, buf);
    }

    /* figure this out once, instead of making multiple calls to need_identify.
     * also makes the code easier to read.
     */
    if (!need_identify(op) || QUERY_FLAG(op, FLAG_IDENTIFIED))
        identified = 1;
    else {
        stringbuffer_append_string(buf, "(unidentified)");
        identified = 0;
    }
    switch (op->type) {
    case BOW:
    case ARROW:
    case WAND:
    case ROD:
    case WEAPON:
    case ARMOUR:
    case HELMET:
    case SHIELD:
    case BOOTS:
    case GLOVES:
    case GIRDLE:
    case BRACERS:
    case CLOAK:
    case SKILL_TOOL:
        break;  /* We have more information to do below this switch */

    case LAMP:
        break; /* just so we get the "glowing" part. */

    case POWER_CRYSTAL:
        /* Avoid division by zero... */
        if (op->stats.maxsp == 0) {
            stringbuffer_append_printf(buf, "(capacity %d).", op->stats.maxsp);
        } else {
            if (op->stats.maxsp > 1000) { /*higher capacity crystals*/
                i = (op->stats.maxsp%1000)/100;
                if (i)
                    stringbuffer_append_printf(buf, "(capacity %d.%dk). It is ", op->stats.maxsp/1000, i);
                else
                    stringbuffer_append_printf(buf, "(capacity %dk). It is ", op->stats.maxsp/1000);
            } else
                stringbuffer_append_printf(buf, "(capacity %d). It is ", op->stats.maxsp);
            i = (op->stats.sp*10)/op->stats.maxsp;
            if (op->stats.sp == 0)
                stringbuffer_append_string(buf, "empty.");
            else if (i == 0)
                stringbuffer_append_string(buf, "almost empty.");
            else if (i < 3)
                stringbuffer_append_string(buf, "partially filled.");
            else if (i < 6)
                stringbuffer_append_string(buf, "half full.");
            else if (i < 9)
                stringbuffer_append_string(buf, "well charged.");
            else if (op->stats.sp == op->stats.maxsp)
                stringbuffer_append_string(buf, "fully charged.");
            else
                stringbuffer_append_string(buf, "almost full.");
        }
        break;

    case FOOD:
    case FLESH:
    case DRINK:
        if (identified || QUERY_FLAG(op, FLAG_BEEN_APPLIED)) {
            stringbuffer_append_printf(buf, "(food+%d)", op->stats.food);

            if (op->type == FLESH && op->last_eat > 0 && atnr_is_dragon_enabled(op->last_eat)) {
                stringbuffer_append_printf(buf, "(%s metabolism)", change_resist_msg[op->last_eat]);
            }

            if (!QUERY_FLAG(op, FLAG_CURSED)) {
                if (op->stats.hp)
                    stringbuffer_append_string(buf, "(heals)");
                if (op->stats.sp)
                    stringbuffer_append_string(buf, "(spellpoint regen)");
            } else {
                if (op->stats.hp)
                    stringbuffer_append_string(buf, "(damages)");
                if (op->stats.sp)
                    stringbuffer_append_string(buf, "(spellpoint depletion)");
            }
        }
        break;

    case SKILL:
    case RING:
    case AMULET:
        if (op->item_power) {
            stringbuffer_append_printf(buf, "(item_power %+d)", op->item_power);
        }
        if (op->title) {
            ring_desc(op, buf);
        }
        return buf;

    default:
        return buf;
    }

    /* Down here, we more further describe equipment type items.
     * only describe them if they have been identified or the like.
     */
    if (identified || QUERY_FLAG(op, FLAG_BEEN_APPLIED)) {
        int attr, val;

        for (attr = 0; attr < NUM_STATS; attr++) {
            if ((val = get_attr_value(&(op->stats), attr)) != 0) {
                stringbuffer_append_printf(buf, "(%s%+d)", short_stat_name[attr], val);
            }
        }
        if (op->glow_radius)
            stringbuffer_append_string(buf, "(glowing)");

        switch (op->type) {
        case FLESH:
            break;

        default:
            if (op->stats.exp) {
                stringbuffer_append_printf(buf, "(speed %+"FMT64")", op->stats.exp);
            }
            break;
        }
        switch (op->type) {
        case BOW:
        case ARROW:
        case GIRDLE:
        case HELMET:
        case SHIELD:
        case BOOTS:
        case GLOVES:
        case WEAPON:
        case SKILL:
        case RING:
        case AMULET:
        case ARMOUR:
        case BRACERS:
        case FORCE:
        case CLOAK:
            if (op->stats.wc) {
                stringbuffer_append_printf(buf, "(wc%+d)", op->stats.wc);
            }
            if (op->stats.dam) {
                stringbuffer_append_printf(buf, "(dam%+d)", op->stats.dam);
            }
            if (op->stats.ac) {
                stringbuffer_append_printf(buf, "(ac%+d)", op->stats.ac);
            }
            if ((op->type == WEAPON || op->type == BOW) && op->level > 0) {
                stringbuffer_append_printf(buf, "(improved %d/%d)", op->last_eat, op->level);
            }
            break;

        default:
            break;
        }
        if (QUERY_FLAG(op, FLAG_XRAYS))
            stringbuffer_append_string(buf, "(xray-vision)");
        if (QUERY_FLAG(op, FLAG_SEE_IN_DARK))
            stringbuffer_append_string(buf, "(infravision)");

        /* levitate was what is was before, so we'll keep it */
        if (op->move_type&MOVE_FLY_LOW)
            stringbuffer_append_string(buf, "(levitate)");

        if (op->move_type&MOVE_FLY_HIGH)
            stringbuffer_append_string(buf, "(fly)");

        if (op->move_type&MOVE_SWIM)
            stringbuffer_append_string(buf, "(swim)");

        /* walking is presumed as 'normal', so doesn't need mentioning */

        if (op->item_power) {
            stringbuffer_append_printf(buf, "(item_power %+d)", op->item_power);
        }
    } /* End if identified or applied */

    /* This blocks only deals with fully identified object.
     * it is intentional that this is not an 'else' from a above -
     * in this way, information is added.
      */
    if (identified) {
        int more_info = 0;

        switch (op->type) {
        case ROD:  /* These use stats.sp for spell selection and stats.food */
        case BOW:  /* and stats.hp for spell-point regeneration... */
        case ARROW:
        case WAND:
        case FOOD:
        case FLESH:
        case DRINK:
            more_info = 0;
            break;

            /* Armor type objects */
        case ARMOUR:
        case HELMET:
        case SHIELD:
        case BOOTS:
        case GLOVES:
        case GIRDLE:
        case BRACERS:
        case CLOAK:
            if (ARMOUR_SPEED(op)) {
                stringbuffer_append_printf(buf, "(Max speed %1.2f)", ARMOUR_SPEED(op)/10.0);
            }
            if (ARMOUR_SPELLS(op)) {
                stringbuffer_append_printf(buf, "(Spell regen penalty %d)", ARMOUR_SPELLS(op));
            }
            more_info = 1;
            break;

        case WEAPON:
            /* Calculate it the same way fix_object does so the results
             * make sense.
             */
            i = (WEAPON_SPEED(op)*2-op->magic)/2;
            if (i < 0)
                i = 0;

            stringbuffer_append_printf(buf, "(weapon speed %d)", i);
            more_info = 1;
            break;
        }
        if (more_info) {
            if (op->stats.food) {
                if (op->stats.food != 0)
                    stringbuffer_append_printf(buf, "(sustenance%+d)", op->stats.food);
            }
            if (op->stats.grace) {
                stringbuffer_append_printf(buf, "(grace%+d)", op->stats.grace);
            }
            if (op->stats.sp) {
                stringbuffer_append_printf(buf, "(magic%+d)", op->stats.sp);
            }
            if (op->stats.hp) {
                stringbuffer_append_printf(buf, "(regeneration%+d)", op->stats.hp);
            }
        }

        if (op->stats.luck) {
            stringbuffer_append_printf(buf, "(luck%+d)", op->stats.luck);
        }
        if (QUERY_FLAG(op, FLAG_LIFESAVE))
            stringbuffer_append_string(buf, "(lifesaving)");
        if (QUERY_FLAG(op, FLAG_REFL_SPELL))
            stringbuffer_append_string(buf, "(reflect spells)");
        if (QUERY_FLAG(op, FLAG_REFL_MISSILE))
            stringbuffer_append_string(buf, "(reflect missiles)");
        if (QUERY_FLAG(op, FLAG_STEALTH))
            stringbuffer_append_string(buf, "(stealth)");
        if (op->slaying != NULL && op->type != FOOD) {
            stringbuffer_append_printf(buf, "(slay %s)", op->slaying);
        }
        describe_attacktype("Attacks", op->attacktype, buf);
        /* resistance on flesh is only visible for dragons.  If
         * non flesh, everyone can see its resistances
         */
        if (op->type != FLESH || (owner && is_dragon_pl(owner))) {
            describe_resistance(op, 0, buf);
        }
        describe_spellpath_attenuation("Attuned", op->path_attuned &~ op->path_denied, buf);
        describe_spellpath_attenuation("Repelled", op->path_repelled &~ op->path_denied, buf);
        describe_spellpath_attenuation("Denied", op->path_denied, buf);
    }

    return buf;
}

/**
 * Checks whether object is magical.
 *
 *  A magical item is one that
 * increases/decreases any abilities, provides a resistance,
 * has a generic magical bonus, or is an artifact.
 *
 * @param op
 * item to check.
 * @return
 * true if the item is magical.
 */
int is_magical(const object *op) {
    int i;

    /* living creatures are considered non magical */
    if (QUERY_FLAG(op, FLAG_ALIVE))
        return 0;

    /* This is a test for it being an artifact, as artifacts have titles */
    if (op->title != NULL)
        return 1;

    /* Handle rings and amulets specially.  If they change any of these
     * values, it means they are magical.
     */
    if ((op->type == AMULET || op->type == RING)
    && (op->stats.ac || op->stats.food || op->stats.exp || op->stats.dam || op->stats.wc || op->stats.sp || op->stats.hp || op->stats.luck))
        return 1;

    /* Check for stealty, speed, flying, or just plain magic in the boots */
    /* Presume any boots that have a move_type are special. */
    if (op->type == BOOTS
    && ((QUERY_FLAG(op, FLAG_STEALTH) || op->move_type || op->stats.exp)))
        return 1;

    /* Take care of amulet/shield that reflects spells/missiles */
    if ((op->type == AMULET || op->type == SHIELD)
    && (QUERY_FLAG(op, FLAG_REFL_SPELL) || QUERY_FLAG(op, FLAG_REFL_MISSILE)))
        return 1;

    /* Take care of helmet of xrays */
    if (op->type == HELMET
    && QUERY_FLAG(op, FLAG_XRAYS))
        return 1;

    /* Potions & rods are always magical.  Wands/staves are also magical,
     * assuming they still have any charges left.
     */
    if (op->type == POTION || op->type == ROD || (op->type == WAND && op->stats.food))
        return 1;

    /* if something gives a protection, either positive or negative, its magical */
    /* This is really a pretty bad hack - as of now, ATNR_PHYSICAL is 0,
     * so this always works out fine.
     */
    for (i = ATNR_PHYSICAL+1; i < NROFATTACKS; i++)
        if (op->resist[i])
            return 1;

    /* Physical protection is expected on some item types, so they should
     * not be considered magical.
     */
    if (op->resist[ATNR_PHYSICAL]
    && op->type != HELMET
    && op->type != SHIELD
    && op->type != BOOTS
    && op->type != GLOVES
    && op->type != ARMOUR)
        return 1;

    /* power crystal, spellbooks, and scrolls are always magical.  */
    if (op->magic
    || op->type == POWER_CRYSTAL
    || op->type == SPELLBOOK
    || op->type == SCROLL
    || op->type == GIRDLE)
        return 1;

    /* Check to see if it increases/decreases any stats */
    for (i = 0; i < NUM_STATS; i++)
        if (get_attr_value(&(op->stats), i) != 0)
            return 1;

    /* If it doesn't fall into any of the above categories, must
     * be non magical.
     */
    return 0;
}

/**
 * This function really should not exist - by default, any item not identified
 * should need it.
 *
 * @param op
 * item to check.
 * @return
 * true if the item should be identified.
 * @todo
 * either remove this function, or fix comment above :)
 */
int need_identify(const object *op) {
    switch (op->type) {
    case RING:
    case WAND:
    case ROD:
    case SCROLL:
    case SKILL:
    case SKILLSCROLL:
    case SPELLBOOK:
    case FOOD:
    case POTION:
    case BOW:
    case ARROW:
    case WEAPON:
    case ARMOUR:
    case SHIELD:
    case HELMET:
    case AMULET:
    case BOOTS:
    case GLOVES:
    case BRACERS:
    case GIRDLE:
    case CONTAINER:
    case DRINK:
    case FLESH:
    case INORGANIC:
    case CLOSE_CON:
    case CLOAK:
    case GEM:
    case POWER_CRYSTAL:
    case POISON:
    case BOOK:
    case SKILL_TOOL:
    case ARMOUR_IMPROVER:
    case WEAPON_IMPROVER:
        return 1;
    }
    return 0;
}

/**
 * Ensure op has all its "identified" properties set.
 * @param op object to process.
 */
void object_give_identified_properties(object *op) {
    sstring key;

    key = object_get_value(op, "identified_face");
    if (key != NULL) {
        op->face = &new_faces[find_face(key, op->face->number)];
        /* if the face is defined, clean the animation, because else
         * the face can be lost ; if an animation is defined, it'll be
         * processed later on */
        CLEAR_FLAG(op, FLAG_CLIENT_ANIM_RANDOM);
        CLEAR_FLAG(op, FLAG_CLIENT_ANIM_SYNC);
        op->anim_speed = 0;
        op->animation_id = 0;
        object_set_value(op, "identified_face", NULL, 0);
    }

    if (object_get_value(op, "identified_anim_random") != NULL) {
        SET_FLAG(op, FLAG_CLIENT_ANIM_RANDOM);
        object_set_value(op, "identified_anim_random", NULL, 0);
    }

    key = object_get_value(op, "identified_anim_speed");
    if (key != NULL) {
        op->anim_speed = atoi(key);
        op->last_anim = 1;
        object_set_value(op, "identified_anim_speed", NULL, 0);
    }

    key = object_get_value(op, "identified_animation");
    if (key != NULL) {
        op->animation_id = atoi(key);
        if (!QUERY_FLAG(op, FLAG_IS_TURNABLE))
            SET_FLAG(op, FLAG_ANIMATE);
        animate_object(op, op->facing);
        object_set_value(op, "identified_animation", NULL, 0);
    }

    key = object_get_value(op, "identified_name");
    if (key != NULL) {
        FREE_AND_COPY(op->name, key);
        object_set_value(op, "identified_name", NULL, 0);
    }
    key = object_get_value(op, "identified_name_pl");
    if (key != NULL) {
        FREE_AND_COPY(op->name_pl, key);
        object_set_value(op, "identified_name_pl", NULL, 0);
    }
}

/**
 * Identifies an item.
 * Supposed to fix face-values as well here, but later.
 * Note - this may merge op with other object, so
 * this function returns either the merged object
 * or the original if no merge happened.
 *
 * @param op
 * item to identify. Can be already identified without ill effects.
 * @retval object
 * The identify object - this may vary from op if the object was
 * merged.
 */
object *identify(object *op) {
    object *pl, *op1;

    SET_FLAG(op, FLAG_IDENTIFIED);
    CLEAR_FLAG(op, FLAG_KNOWN_MAGICAL);
    CLEAR_FLAG(op, FLAG_NO_SKILL_IDENT);

    object_give_identified_properties(op);

    /*
     * We want autojoining of equal objects:
     */
    if (QUERY_FLAG(op, FLAG_CURSED) || QUERY_FLAG(op, FLAG_DAMNED))
        SET_FLAG(op, FLAG_KNOWN_CURSED);

    if (QUERY_FLAG(op, FLAG_BLESSED))
        SET_FLAG(op, FLAG_KNOWN_BLESSED);

    if (op->type == POTION) {
        if (op->inv && op->randomitems) {
            if (op->title)
                free_string(op->title);
            op->title = add_refcount(op->inv->name);
        } else if (op->arch) {
            free_string(op->name);
            op->name = add_refcount(op->arch->clone.name);
            free_string(op->name_pl);
            op->name_pl = add_refcount(op->arch->clone.name_pl);
        }
    }

    if (op->map) {
        /* If the object is on a map, make sure we update its face.
         * Also send name and such information to a player standing on it.
         */
        object *player = map_find_by_type(op->map, op->x, op->y, PLAYER);

        object_update(op, UP_OBJ_FACE);
        op1 = object_merge(op, GET_MAP_TOP(op->map, op->x, op->y));
        if (op1) op = op1;

        if (player)
            esrv_update_item(UPD_FACE | UPD_NAME | UPD_FLAGS, player, op);

    } else {
        pl = object_get_player_container(op->env);
        op1 = object_merge(op, op->env->inv);
        if (op1) op = op1;

        if (pl)
            /* A lot of the values can change from an update - might as well send
             * it all.
             */
            esrv_update_item(UPD_ALL, pl, op);
    }
    return op;
}
