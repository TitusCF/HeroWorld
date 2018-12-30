/**
 * @file
 * This small program will extract information from Gridarta's types.xml file to generate documentation about types and fields.
 * Files are placed in developer's documentation subdirs by default.
 *
 * To build: <pre>gcc -g -pg -O0 -Wall -pedantic gridarta-types-convert.c -I../include -o gridarta-types-convert</pre>
 * To run: <pre>./gridarta-types-convert ../../gridarta/crossfire/resource/conf/types.xml</pre>
 * (adjust the path according to your setup)
 *
 * Note that someone wishing to tweak this program should know the format of Gridarta's types.xml.
 *
 * This program isn't terribly robust, there could be more conditions, and it just doesn't free memory.
 * May be fixed someday, but since it's a "run and exit", not high priority.
 *
 * This program can be modified and altered at will, as long as it's for the Crossfire project. Else don't touch it :)
 *
 * Note that "attribute" is used for "field in a object/living structure".
 *
 * @author
 * Nicolas Weeger / Ryo_ on IRC
 * @date 2008-01-06
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "define.h"

const char *destination_dir = "../doc/Developers"; /**< Root destination dir. */
const char *field_dir = "fields"; /**< Where the files about the fields will be stored. */
const char *type_dir = "types"; /**< Where the files about types will be stored. */

/** One attribute in a type. */
typedef struct {
    char *field;
    char *name;
    char *description;
} type_attribute;

/** One object type. */
typedef struct {
    int number;
    char *name;
    char *description;
    char *use;
    type_attribute **attributes;
    int attribute_count;
    char **required;
    int require_count;
} type_definition;

/** Defined types. */
type_definition **types = NULL;

int type_count = 0;

/** Definitions all types have by default. */
type_definition *default_type = NULL;

/** Dummy object type that non defined objects use. */
type_definition *fallback_type = NULL;

/** One list of fields to ignore. */
typedef struct {
    char *name;
    int count;
    char **fields;
} ignore_list;

ignore_list **lists = NULL;
int list_count = 0;

/** One type for an attribute. */
typedef struct {
    char **type;
    int *number;
    int count;
    char *description;
} attribute_type;

/** One attribute. */
typedef struct {
    char *field;
    attribute_type **types;
    int type_count;
} attribute_definition;

attribute_definition **attributes = NULL;
int attribute_count = 0;

/** One flag. */
typedef struct {
    const char *field;
    const char *code_name;
} flag_definition;

/** Flag mapping. */
static const flag_definition flags[] = {
    { "alive", "FLAG_ALIVE" },
    { "wiz", "FLAG_WIZ" },
    { "was_wiz", "FLAG_WAS_WIZ" },
    { "applied", "FLAG_APPLIED" },
    { "unpaid", "FLAG_UNPAID" },
    { "can_use_shield", "FLAG_USE_SHIELD" },
    { "no_pick", "FLAG_NO_PICK" },
    { "client_anim_sync", "FLAG_CLIENT_ANIM_SYNC" },
    { "client_anim_random", "FLAG_CLIENT_ANIM_RANDOM" },
    { "is_animated", "FLAG_ANIMATE" },
    { "monster", "FLAG_MONSTER" },
    { "friendly", "FLAG_FRIENDLY" },
    { "generator", "FLAG_GENERATOR" },
    { "is_thrown", "FLAG_IS_THROWN" },
    { "auto_apply", "FLAG_AUTO_APPLY" },
    { "treasure", "FLAG_TREASURE" },
    { "player sold", "FLAG_PLAYER_SOLD" },
    { "see_invisible", "FLAG_SEE_INVISIBLE" },
    { "can_roll", "FLAG_CAN_ROLL" },
    { "overlay_floor", "FLAG_OVERLAY_FLOOR" },
    { "is_turnable", "FLAG_IS_TURNABLE" },
    { "is_used_up", "FLAG_IS_USED_UP" },
    { "identified", "FLAG_IDENTIFIED" },
    { "reflecting", "FLAG_REFLECTING" },
    { "changing", "FLAG_CHANGING" },
    { "splitting", "FLAG_SPLITTING" },
    { "hitback", "FLAG_HITBACK" },
    { "startequip", "FLAG_STARTEQUIP" },
    { "blocksview", "FLAG_BLOCKSVIEW" },
    { "undead", "FLAG_UNDEAD" },
    { "scared", "FLAG_SCARED" },
    { "unaggressive", "FLAG_UNAGGRESSIVE" },
    { "reflect_missile", "FLAG_REFL_MISSILE" },
    { "reflect_spell", "FLAG_REFL_SPELL" },
    { "no_magic", "FLAG_NO_MAGIC" },
    { "no_fix_player", "FLAG_NO_FIX_PLAYER" },
    { "is_lightable", "FLAG_IS_LIGHTABLE" },
    { "tear_down", "FLAG_TEAR_DOWN" },
    { "run_away", "FLAG_RUN_AWAY" },
    { "unique", "FLAG_UNIQUE" },
    { "no_drop", "FLAG_NO_DROP" },
    { "can_cast_spell", "FLAG_CAST_SPELL" },
    { "can_use_scroll", "FLAG_USE_SCROLL" },
    { "can_use_range", "FLAG_USE_RANGE" },
    { "can_use_bow", "FLAG_USE_BOW" },
    { "can_use_armour", "FLAG_USE_ARMOUR" },
    { "can_use_weapon", "FLAG_USE_WEAPON" },
    { "can_use_ring", "FLAG_USE_RING" },
    { "has_ready_range", "FLAG_READY_RANGE" },
    { "has_ready_bow", "FLAG_READY_BOW" },
    { "xrays", "FLAG_XRAYS" },
    { "is_floor", "FLAG_IS_FLOOR" },
    { "lifesave", "FLAG_LIFESAVE" },
    { "no_strength", "FLAG_NO_STRENGTH" },
    { "sleep", "FLAG_SLEEP" },
    { "stand_still", "FLAG_STAND_STILL" },
    { "random_movement", "FLAG_RANDOM_MOVE" },
    { "only_attack", "FLAG_ONLY_ATTACK" },
    { "confused", "FLAG_CONFUSED" },
    { "stealth", "FLAG_STEALTH" },
    { "cursed", "FLAG_CURSED" },
    { "damned", "FLAG_DAMNED" },
    { "see_anywhere", "FLAG_SEE_ANYWHERE" },
    { "known_magical", "FLAG_KNOWN_MAGICAL" },
    { "known_cursed", "FLAG_KNOWN_CURSED" },
    { "can_use_skill", "FLAG_CAN_USE_SKILL" },
    { "been_applied", "FLAG_BEEN_APPLIED" },
    { "has_ready_scroll", "FLAG_READY_SCROLL" },
    { "make_invisible", "FLAG_MAKE_INVIS" },
    { "inv_locked", "FLAG_INV_LOCKED" },
    { "is_wooded", "FLAG_IS_WOODED" },
    { "is_hilly", "FLAG_IS_HILLY" },
    { "has_ready_skill", "FLAG_READY_SKILL" },
    { "has_ready_weapon", "FLAG_READY_WEAPON" },
    { "no_skill_ident", "FLAG_NO_SKILL_IDENT" },
    { "is_blind", "FLAG_BLIND" },
    { "can_see_in_dark", "FLAG_SEE_IN_DARK" },
    { "is_cauldron", "FLAG_IS_CAULDRON" },
    { "no_steal", "FLAG_NO_STEAL" },
    { "one_hit", "FLAG_ONE_HIT" },
    { "berserk", "FLAG_BERSERK" },
    { "neutral", "FLAG_NEUTRAL" },
    { "no_attack", "FLAG_NO_ATTACK" },
    { "no_damage", "FLAG_NO_DAMAGE" },
    { "activate_on_push", "FLAG_ACTIVATE_ON_PUSH" },
    { "activate_on_release", "FLAG_ACTIVATE_ON_RELEASE" },
    { "is_water", "FLAG_IS_WATER" },
    { "use_content_on_gen", "FLAG_CONTENT_ON_GEN" },
    { "is_buildable", "FLAG_IS_BUILDABLE" },
    { "blessed", "FLAG_BLESSED" },
    { "known_blessed", "FLAG_KNOWN_BLESSED" },
    { NULL, NULL }
};

/** Return flag if exists, NULL else. */
const flag_definition *find_flag(const char *name) {
    int flag;

    for (flag = 0; flags[flag].field; flag++)
        if (!strcmp(flags[flag].field, name))
            return &flags[flag];
    return NULL;
}

typedef struct {
    const char *code_name;
    int value;
} type_name;

static type_name type_names[] = {
    { "PLAYER", PLAYER },
    { "TRANSPORT", TRANSPORT },
    { "ROD", ROD },
    { "TREASURE", TREASURE },
    { "POTION", POTION },
    { "FOOD", FOOD },
    { "POISON", POISON },
    { "BOOK", BOOK },
    { "CLOCK", CLOCK },
    { "ARROW", ARROW },
    { "BOW", BOW },
    { "WEAPON", WEAPON },
    { "ARMOUR", ARMOUR },
    { "PEDESTAL", PEDESTAL },
    { "ALTAR", ALTAR },
    { "LOCKED_DOOR", LOCKED_DOOR },
    { "SPECIAL_KEY", SPECIAL_KEY },
    { "MAP", MAP },
    { "DOOR", DOOR },
    { "KEY", KEY },
    { "TIMED_GATE", TIMED_GATE },
    { "TRIGGER", TRIGGER },
    { "GRIMREAPER", GRIMREAPER },
    { "MAGIC_EAR", MAGIC_EAR },
    { "TRIGGER_BUTTON", TRIGGER_BUTTON },
    { "TRIGGER_ALTAR", TRIGGER_ALTAR },
    { "TRIGGER_PEDESTAL", TRIGGER_PEDESTAL },
    { "SHIELD", SHIELD },
    { "HELMET", HELMET },
    { "MONEY", MONEY },
    { "CLASS", CLASS },
    { "AMULET", AMULET },
    { "PLAYERMOVER", PLAYERMOVER },
    { "TELEPORTER", TELEPORTER },
    { "CREATOR", CREATOR },
    { "SKILL", SKILL },
    { "EARTHWALL", EARTHWALL },
    { "GOLEM", GOLEM },
    { "THROWN_OBJ", THROWN_OBJ },
    { "BLINDNESS", BLINDNESS },
    { "GOD", GOD },
    { "DETECTOR", DETECTOR },
    { "TRIGGER_MARKER", TRIGGER_MARKER },
    { "DEAD_OBJECT", DEAD_OBJECT },
    { "DRINK", DRINK },
    { "MARKER", MARKER },
    { "HOLY_ALTAR", HOLY_ALTAR },
    { "PLAYER_CHANGER", PLAYER_CHANGER },
    { "BATTLEGROUND", BATTLEGROUND },
    { "PEACEMAKER", PEACEMAKER },
    { "GEM", GEM },
    { "FIREWALL", FIREWALL },
    { "CHECK_INV", CHECK_INV },
    { "MOOD_FLOOR", MOOD_FLOOR },
    { "EXIT", EXIT },
    { "ENCOUNTER", ENCOUNTER },
    { "SHOP_FLOOR", SHOP_FLOOR },
    { "SHOP_MAT", SHOP_MAT },
    { "RING", RING },
    { "FLOOR", FLOOR },
    { "FLESH", FLESH },
    { "INORGANIC", INORGANIC },
    { "SKILL_TOOL", SKILL_TOOL },
    { "LIGHTER", LIGHTER },
    { "WALL", WALL },
    { "MISC_OBJECT", MISC_OBJECT },
    { "MONSTER", MONSTER },
    { "LAMP", LAMP },
    { "DUPLICATOR", DUPLICATOR },
    { "SPELLBOOK", SPELLBOOK },
    { "CLOAK", CLOAK },
    { "SPINNER", SPINNER },
    { "GATE", GATE },
    { "BUTTON", BUTTON },
    { "CF_HANDLE", CF_HANDLE },
    { "HOLE", HOLE },
    { "TRAPDOOR", TRAPDOOR },
    { "SIGN", SIGN },
    { "BOOTS", BOOTS },
    { "GLOVES", GLOVES },
    { "SPELL", SPELL },
    { "SPELL_EFFECT", SPELL_EFFECT },
    { "CONVERTER", CONVERTER },
    { "BRACERS", BRACERS },
    { "POISONING", POISONING },
    { "SAVEBED", SAVEBED },
    { "WAND", WAND },
    { "SCROLL", SCROLL },
    { "DIRECTOR", DIRECTOR },
    { "GIRDLE", GIRDLE },
    { "FORCE", FORCE },
    { "POTION_RESIST_EFFECT", POTION_RESIST_EFFECT },
    { "EVENT_CONNECTOR", EVENT_CONNECTOR },
    { "CLOSE_CON", CLOSE_CON },
    { "CONTAINER", CONTAINER },
    { "ARMOUR_IMPROVER", ARMOUR_IMPROVER },
    { "WEAPON_IMPROVER", WEAPON_IMPROVER },
    { "SKILLSCROLL", SKILLSCROLL },
    { "DEEP_SWAMP", DEEP_SWAMP },
    { "IDENTIFY_ALTAR", IDENTIFY_ALTAR },
    { "SHOP_INVENTORY", SHOP_INVENTORY },
    { "RUNE", RUNE },
    { "TRAP", TRAP },
    { "POWER_CRYSTAL", POWER_CRYSTAL },
    { "CORPSE", CORPSE },
    { "DISEASE", DISEASE },
    { "SYMPTOM", SYMPTOM },
    { "BUILDER", BUILDER },
    { "MATERIAL", MATERIAL },
    { NULL, 0 }
};

type_attribute *duplicate_attribute(type_attribute *attr) {
    type_attribute *ret = calloc(1, sizeof(type_attribute));
    ret->field = strdup(attr->field);
    ret->name = strdup(attr->name);
    ret->description = strdup(attr->description);
    return ret;
}

void free_attribute(type_attribute *attr) {
    free(attr->field);
    free(attr->name);
    free(attr->description);
    free(attr);
}

/**
 * Gets the attribute for the specified type. If it doesn't exist, create it.
 * If the attribute is already defined, return the existing one, after cleaning its fields if clean is set.
 */
type_attribute *get_attribute_for_type(type_definition *type, const char *attribute, int clean) {
    type_attribute *ret;
    int test;

    for (test = 0; test < type->attribute_count; test++) {
        if (!strcmp(type->attributes[test]->field, attribute)) {
            ret = type->attributes[test];
            if (clean) {
                free(ret->name);
                ret->name = NULL;
                free(ret->description);
                ret->description = NULL;
            }
            return ret;
        }
    }
    ret = calloc(1, sizeof(type_attribute));
    ret->field = strdup(attribute);

    type->attribute_count++;
    type->attributes = realloc(type->attributes, type->attribute_count*sizeof(type_attribute *));
    type->attributes[type->attribute_count-1] = ret;

    return ret;
}

void copy_attributes(const type_definition *source, type_definition *type) {
    int attr;
    type_attribute *add;

    assert(source);
    if (source->attribute_count == 0)
        return;

    for (attr = 0; attr < source->attribute_count; attr++) {
        add = get_attribute_for_type(type, source->attributes[attr]->field, 1);
        add->name = strdup(source->attributes[attr]->name);
        if (source->attributes[attr]->description)
            add->description = strdup(source->attributes[attr]->description);
    }
}

void copy_default_attributes(type_definition *type) {
    if (!default_type)
        return;
    copy_attributes(default_type, type);
}

/**
 * Returns a new type_definition having the default attributes.
 */
type_definition *get_type_definition(void) {
    type_definition *ret = calloc(1, sizeof(type_definition));

    ret->attribute_count = 0;
    ret->attributes = NULL;
    assert(ret->description == NULL);

    if (default_type)
        copy_default_attributes(ret);

    return ret;
}

/**
 * Used for type import.
 */
type_definition *find_type_definition(const char *name) {
    int type;

    for (type = 0; type < type_count; type++) {
        if (!strcmp(types[type]->name, name))
            return types[type];
    }
    printf("type not found: %s\n", name);
    return NULL;
}

/** To sort attributes. */
int sort_type_attribute(const void *a, const void *b) {
    const type_attribute **la = (const type_attribute **)a;
    const type_attribute **lb = (const type_attribute **)b;

    return strcmp((*la)->name, (*lb)->name);
}

ignore_list *find_ignore_list(const char *name) {
    int list;

    for (list = 0; list < list_count; list++) {
        if (strcmp(lists[list]->name, name) == 0)
            return lists[list];
    }
    return NULL;
}

/**
 * @todo remove spaces at line start/end.
 */
char *read_line(char *buffer, int size, FILE *file) {
    return fgets(buffer, 200, file);
}

/** Remove an attribute from the type. */
void ignore_attribute(type_definition *type, const char *attribute) {
    int find;

    for (find = 0; find < type->attribute_count; find++) {
        if (!strcmp(attribute, type->attributes[find]->field)) {
            /*printf("rem %s from %s\n", list->fields[attr], type->name);*/
            free_attribute(type->attributes[find]);
            if (find < type->attribute_count-1)
                type->attributes[find] = type->attributes[type->attribute_count-1];
            type->attribute_count--;
            return;
        }
    }
}

/** Remove all attributes in the specified list from the type. */
void ignore_attributes(type_definition *type, ignore_list *list) {
    int attr;

    if (!list) {
        printf("empty ignore list?\n");
        return;
    }

    for (attr = 0; attr < list->count; attr++) {
        ignore_attribute(type, list->fields[attr]);
     }
}

/** Add a required parameter to the specified type. buf is the line read from the file, non processed. */
void add_required_parameter(type_definition *type, const char *buf) {
    char *sn, *en, *sv, *ev;
    char value[200], name[200], temp[200];
    const flag_definition *flag;

    if (type == fallback_type)
        /* the "Misc" type has dummy requirements, don't take that into account. */
        return;

    sn = strstr(buf, "arch");
    if (!sn)
        return;
    sn = strchr(sn, '"');
    en = strchr(sn+1, '"');
    sv = strstr(buf, "value");
    sv = strchr(sv, '"');
    ev = strchr(sv+1, '"');

    name[en-sn-1] = '\0';
    strncpy(name, sn+1, en-sn-1);
    value[ev-sv-1] = '\0';
    strncpy(value, sv+1, ev-sv-1);

    type->require_count++;
    type->required = realloc(type->required, type->require_count*sizeof(char *));

    flag = find_flag(name);
    if (flag)
        snprintf(temp, 200, "@ref %s %s", flag->code_name, strcmp(value, "0") ? "set" : "unset");
    else
        snprintf(temp, 200, "@ref object::%s = %s", name, value);
    type->required[type->require_count-1] = strdup(temp);
}

/** Read all lines related to a type, stop when "block_end" is found on a line. */
void read_type(type_definition *type, FILE *file, const char *block_end) {
    char buf[200], tmp[200];
    char *find, *end;
    type_attribute *attr;

    while (read_line(buf, 200, file)) {
        if (strstr(buf, block_end) != NULL) {
            if (type->attribute_count)
                qsort(type->attributes, type->attribute_count, sizeof(type_attribute *), sort_type_attribute);
            return;
        }
        if (strstr(buf, "<description>") != NULL) {
            while (read_line(buf, 200, file)) {
                if (strstr(buf, "</description>") != NULL)
                    break;

                if (type->description) {
                    type->description = realloc(type->description, strlen(type->description)+strlen(buf)+1);
                    strcat(type->description, buf);
                }
                else
                    type->description = strdup(buf);
            }
            find = strstr(type->description, "]]>");
            if (find)
                type->description[find-type->description] = '\0';
            while (type->description[strlen(type->description)-1] == '\n')
                type->description[strlen(type->description)-1] = '\0';
            /*printf(" => desc = %s\n", type->description);*/
        }

        if (strstr(buf, "<ignore_list") != NULL) {
            find = strstr(buf, "name=");
            if (!find)
                return;
            find = strchr(find+1, '"');
            if (!find)
                return;
            end = strchr(find+1, '"');
            if (!end)
                return;
            tmp[end-find-1] = '\0';
            strncpy(tmp, find+1, end-find-1);
            ignore_attributes(type, find_ignore_list(tmp));
        }

        if (strstr(buf, "<ignore>") != NULL) {
            while (read_line(buf, 200, file)) {
                if (strstr(buf, "</ignore>") != NULL)
                    break;
                find = strstr(buf, "arch=");
                if (!find)
                    continue;
                find = strchr(find+1, '"');
                if (!find)
                    continue;
                end = strchr(find+1, '"');
                if (!end)
                    continue;
                tmp[end-find-1] = '\0';
                strncpy(tmp, find+1, end-find-1);
                ignore_attribute(type, tmp);
            }
        }

        if (strstr(buf, "<required>") != NULL) {
            while (read_line(buf, 200, file)) {
                if (strstr(buf, "</required>") != NULL)
                    break;
                add_required_parameter(type, buf);
            }
        }

        if (strstr(buf, "<import_type") != NULL) {
            type_definition *import;

            find = strstr(buf, "name=");
            if (!find)
                return;
            find = strchr(find+1, '"');
            if (!find)
                return;
            end = strchr(find+1, '"');
            if (!end)
                return;
            tmp[end-find-1] = '\0';
            strncpy(tmp, find+1, end-find-1);
            import = find_type_definition(tmp);
            if (import) {
                /*printf("%s import %s\n", type->name, tmp);*/
                copy_attributes(import, type);
            }
            else
                printf("%s: import %s not found\n", type->name, tmp);
        }

        if (strstr(buf, "<attribute") != NULL) {
            find = strstr(buf, "arch");
            if (!find)
                continue;
            find = strchr(find, '"');
            end = strchr(find+1, '"');
            if (end == find+1)
                /* empty arch, meaning inventory or such, ignore. */
                continue;

            tmp[end-find-1] = '\0';
            strncpy(tmp, find+1, end-find-1);
            /*printf(" => attr %s\n", tmp);*/

            find = strstr(buf, "editor");
            if (find == NULL)
                /* fixed or other, ignore */
                continue;
            attr = get_attribute_for_type(type, tmp, 1);
            find = strchr(find, '"');
            end = strchr(find+1, '"');
            tmp[end-find-1] = '\0';
            strncpy(tmp, find+1, end-find-1);
            attr->name = strdup(tmp);

            /* Description can be empty, with end tag on the same line. */
            if (strstr(buf, "</attribute>") == NULL && strstr(buf, "/>") == NULL) {
                while (read_line(buf, 200, file)) {
                    if (strstr(buf, "<![CDATA[<html>") != NULL)
                        /* some data is in HTML, that's ok */
                        continue;
                    if (strstr(buf, "]]>") != NULL)
                        /* end of cdata html */
                        continue;
                    if (strstr(buf, "</attribute>") != NULL)
                        break;
                    if (attr->description) {
                        attr->description = realloc(attr->description, strlen(attr->description)+strlen(buf)+1);
                        strcat(attr->description, buf);
                    }
                    else
                        attr->description = strdup(buf);
                }
            }
            if (attr->description)
                while (attr->description[strlen(attr->description)-1] == '\n')
                    attr->description[strlen(attr->description)-1] = '\0';

        }
    }
}

void dump_type(type_definition *type) {
    int attr;

    printf("type: %s [%d]\n", type->name, type->number);
    printf(" attributes:\n");
    for (attr = 0; attr < type->attribute_count; attr++) {
        printf(" %30s: %s\n", type->attributes[attr]->field, type->attributes[attr]->name);
        printf("     %s\n", type->attributes[attr]->description);
    }
}

void dump_types(void) {
    int t;
    type_definition *type;

    for (t = 0; t < type_count; t++) {
        type = types[t];
        dump_type(type);
     }
}

/** Get an attribute, create it if it doesn't exist yet. */
attribute_definition *get_attribute(const char *name) {
    int attr;
    attribute_definition *ret;

    for (attr = 0; attr < attribute_count; attr++) {
        if (!strcmp(attributes[attr]->field, name))
            return attributes[attr];
    }

    ret = calloc(1, sizeof(attribute_definition));
    attribute_count++;
    attributes = realloc(attributes, attribute_count*sizeof(attribute_definition *));
    attributes[attribute_count-1] = ret;

    ret->field = strdup(name);

    return ret;
}

/** Gets a type description for specified attribute, create it if doesn't exist. */
attribute_type *get_description_for_attribute(attribute_definition *attribute, const char *description) {
    int desc;
    attribute_type *add;

    for (desc = 0; desc < attribute->type_count; desc++) {
        if (!description && !attribute->types[desc]->description)
            return attribute->types[desc];
        if (description && attribute->types[desc]->description && !strcmp(description, attribute->types[desc]->description))
            return attribute->types[desc];
    }

    add = calloc(1, sizeof(attribute_type));
    attribute->type_count++;
    attribute->types = realloc(attribute->types, attribute->type_count*sizeof(attribute_type));
    attribute->types[attribute->type_count-1] = add;

    if (description)
        add->description = strdup(description);

    return add;
}

void add_type_to_attribute(attribute_definition *attribute, type_definition *type, int attr) {
    attribute_type *att;

    att = get_description_for_attribute(attribute, type->attributes[attr]->description);
    att->count++;
    att->type = realloc(att->type, att->count*sizeof(const char *));
    att->number = realloc(att->number, att->count*sizeof(int));
    att->type[att->count-1] = strdup(type->name);
    att->number[att->count-1] = type->number;
}

/** Read the contents of a <code>\<ignore_list\></code> tag. */
void read_ignore_list(const char *name, FILE *file) {
    char buf[200], tmp[200];
    char *start, *end;
    ignore_list *list;

    /*printf("il %s:", name);*/
    list = calloc(1, sizeof(ignore_list));
    list_count++;
    lists = realloc(lists, list_count*sizeof(ignore_list *));
    lists[list_count-1] = list;
    list->name = strdup(name);

    while (read_line(buf, 200, file)) {
        if (strstr(buf, "</ignore_list>") != NULL) {
            /*printf("\n");*/
            return;
        }
        start = strstr(buf, "arch=");
        if (!start)
            continue;
        start = strchr(start+1, '"');
        if (!start)
            continue;
        end = strchr(start+1, '"');
        if (!end)
            continue;

        tmp[end-start-1] = '\0';
        strncpy(tmp, start+1, end-start-1);
        /*printf(" %s", tmp);*/

        list->count++;
        list->fields = realloc(list->fields, list->count*sizeof(char *));
        list->fields[list->count-1] = strdup(tmp);
    }
}

void dump_ignore_lists(void) {
    int list, field;

    printf("ignore lists:\n");
    for (list = 0; list < list_count; list++) {
        printf(" %s:", lists[list]->name);
        for (field = 0; field < lists[list]->count; field++)
            printf(" %s", lists[list]->fields[field]);
        printf("\n");
    }
}

/** Fields part of the living structure. */
static const char *in_living[] = {
    "Str",
    "Dex",
    "Con",
    "Wis",
    "Cha",
    "Int",
    "Pow",
    "wc",
    "ac",
    "hp",
    "maxhp",
    "sp",
    "maxsp",
    "grace",
    "maxgrace",
    "exp",
    "food",
    "dam",
    "luck",
    NULL
};

/** Custom attributes we know about, to point to the right page. */
static const char *custom_attributes[] = {
    /* transports */
    "weight_speed_ratio",
    "base_speed",
    "passenger_limit",
    "face_full",
    "anim_full",
    /* misc */
    "accept_alive",
    "death_animation",
    "face_opened",
    "generator_code",
    "generator_limit",
    "generator_max_map",
    "generator_radius",
    "no_mood_change",
    "on_use_yield",
    "race_restriction",
    "wc_increase_rate",
    "price_adjustment",
    "price_adjustment_buy",
    "price_adjustment_sell",
    "casting_requirements",
    "identified_name",
    "identified_name_pl",
    "identified_face",
    "identified_animation",
    "identified_anim_speed",
    "identified_anim_random",
    "immunity_chance",
    "elevation",
    NULL
};

int is_custom_attribute(const char *attribute) {
    int val;

    for (val = 0; custom_attributes[val] != NULL; val++) {
        if (!strcmp(custom_attributes[val], attribute)) {
            return 1;
        }
    }
    return 0;
}

/** Write the part to the right of a \@ref for the specified attribute. */
void write_attribute_reference(const char *attribute, FILE *file) {
    const flag_definition *flag = find_flag(attribute);
    int val;

    if (flag) {
        fprintf(file, "%s", flag->code_name);
        return;
    }
    for (val = 0; in_living[val] != NULL; val++) {
        if (!strcmp(in_living[val], attribute)) {
            fprintf(file, "liv::%s", attribute);
            return;
        }
    }
    if (is_custom_attribute(attribute)) {
        fprintf(file, "page_custom_attribute_%s \"%s\"", attribute, attribute);
        return;
        }
    if (strstr(attribute, "resist_")) {
        fprintf(file, "obj::resist");
        return;
    }
    if (!strcmp(attribute, "connected")) {
        fprintf(file, "page_connected \"connection value\"");
        return;
    }
    fprintf(file, "obj::%s", attribute);
}

/** Write a type definition file. */
void write_type_file(type_definition *type) {
    FILE *file;
    char buf[200];
    int attr, req;

    snprintf(buf, 200, "%s/%s/type_%d.dox", destination_dir, type_dir, type->number);
    file = fopen(buf, "w+");

    fprintf(file, "/**\n");

    /* auto-generate documentation for the type, so no need to change define.h */
    if (type->number > 0) {
        for (req = 0; type_names[req].code_name != NULL; req++) {
            if (type_names[req].value == type->number) {
                fprintf(file, "@var %s\nSee @ref page_type_%d\n*/\n\n/**\n", type_names[req].code_name, type->number);
                break;
            }
        }
    }

    fprintf(file, "@page page_type_%d %s\n\n", type->number, type->name);
    fprintf(file, "\n@section Description\n");
    fprintf(file, "%s\n\n", type->description);
    if (type != fallback_type) {
        fprintf(file, "\n\nType defined by:\n");
        if (type->number && type->number < OBJECT_TYPE_MAX)
            fprintf(file, "- @ref object::type = %d\n", type->number);
        for (req = 0; req < type->require_count; req++)
            fprintf(file, "- %s\n", type->required[req]);
    }

    fprintf(file, "\n\n@section Attributes\n\n");
    fprintf(file, "<table>\n\t<tr>\n\t\t<th>Attribute</th>\n\t\t<th>Field</th>\n\t\t<th>Description</th>\n\t</tr>\n");
    for (attr = 0; attr < type->attribute_count; attr++) {
        fprintf(file, "\t<tr>\n\t\t<td>%s</td>\n\t\t<td>@ref ", type->attributes[attr]->name);
        write_attribute_reference(type->attributes[attr]->field, file);
        fprintf(file, "</td>\n\t\t<td>%s\n\t\t</td>\n\t</tr>\n", type->attributes[attr]->description ? type->attributes[attr]->description : "(no description)");
    }

    fprintf(file, "</table>\n*/\n");

    fclose(file);
}

/** Write index of all types. */
void write_type_index(void) {
    FILE *index;
    int type;
    char buf[200];

    snprintf(buf, 200, "%s/%s/types.dox", destination_dir, type_dir);
    index = fopen(buf, "w+");
    fprintf(index, "/**\n@page type_index Type index\n");

    fprintf(index, "Types not listed here have the attributes defined in @ref page_type_0 \"this page\".\n\n");

    for (type = 0; type < type_count; type++) {
        fprintf(index, "- @ref page_type_%d \"%s\"\n", types[type]->number, types[type]->name);
    }

    fprintf(index, "*/\n");

    fclose(index);
}

/** Write the index of all attributes. */
void write_attribute_index(void) {
    FILE *index;
    int attribute;
    char buf[200];

    snprintf(buf, 200, "%s/%s/fields.dox", destination_dir, field_dir);
    index = fopen(buf, "w+");
    fprintf(index, "/**\n@page field_index Field index\n");

    fprintf(index, "This page lists all attributes.\n\n");

    for (attribute = 0; attribute < attribute_count; attribute++) {
        fprintf(index, "- @ref ");
        write_attribute_reference(attributes[attribute]->field, index);
        fprintf(index, "\n");
    }

    fprintf(index, "*/\n");

    fclose(index);
}

/** Write the description of a field. */
void write_attribute_file(attribute_definition *attribute) {
    FILE *file;
    char buf[200];
    int type, desc;
    const char *end;

    snprintf(buf, 200, "%s/%s/field_%s.dox", destination_dir, field_dir, attribute->field);
    file = fopen(buf, "w+");

    if (is_custom_attribute(attribute->field)) {
        fprintf(file, "/**\n@page page_custom_attribute_%s %s", attribute->field, attribute->field);
        fprintf(file, "\nThis is a @ref page_custom_attributes \"custom attribute\".\n");
    } else {
        if (strcmp(attribute->field, "connected") == 0)
            fprintf(file, "/**\n@page ");
        else
            fprintf(file, "/**\n@fn ");
        write_attribute_reference(attribute->field, file);
    }

    /* resistances are special, they'll be merged in the obj::resist paragraph, so specify the name. */
    if (strstr(attribute->field, "resist_"))
        fprintf(file, "\n@section %s %s resistance\n", attribute->field, attribute->field+7);
    else
        fprintf(file, "\n@section Use\n");

    fprintf(file, "<table>\n\t<tr>\n\t\t<th>Type(s)</th>\n\t\t<th>Description</th>\n\t</tr>");

    for (desc = 0; desc < attribute->type_count; desc++) {
        assert(attribute->types[desc]->count > 0);

        fprintf(file, "\t<tr>\n\t\t<td>\n");

        for (type = 0; type < attribute->types[desc]->count; type++) {
            if (type < attribute->types[desc]->count-1)
                end = ", ";
            else
                end = "\n";
            fprintf(file, "@ref page_type_%d%s", attribute->types[desc]->number[type], end);
        }
        fprintf(file, "\t\t</td><td>%s</td>\n\t</tr>\n", attribute->types[desc]->description ? attribute->types[desc]->description : "(no description)");
    }

    fprintf(file, "\n*/\n");

    fclose(file);
}

int main(int argc, char **argv) {
    FILE *xml;
    int number, attr, dummy;
    char buf[200], tmp[200];
    char *start, *end;
    type_definition *type;

    if (argc < 2) {
        printf("Syntax: %s /path/to/Gridarta/types.xml\n", argv[0]);
        return 1;
    }

    /* dummy type number for special types. */
    dummy = OBJECT_TYPE_MAX+50;

    xml = fopen(argv[1], "r");

    if (!xml) {
        printf("Could not find file %s\n", argv[1]);
        return 1;
    }

    while (read_line(buf, 200, xml) != NULL) {
        if (buf[0] == '#')
            continue;
        if (strstr(buf, "<default_type>")) {
            default_type = get_type_definition();
            default_type->name = strdup("(default type)");
            read_type(default_type, xml, "</default_type>");
            continue;
        }

        if (strstr(buf, "<ignore_list") != NULL) {
            start = strstr(buf, "name=");
            start = strchr(start+1, '"');
            end = strchr(start+1, '"');
            tmp[end-start-1] = '\0';
            strncpy(tmp, start+1, end-start-1);
            read_ignore_list(tmp, xml);
            continue;
        }

        start = strstr(buf, "<type number");
        if (start) {
            start = strchr(start, '"');
            /*if (!start)
                break;*/
            end = strchr(start+1, '"');
            /*if (!end)
                break;*/
            tmp[end-start-1] = '\0';
            strncpy(tmp, start+1, end-start-1);
            /*printf("type %s ", tmp);*/

            number = atoi(tmp);
            start = strstr(end, "name=");
            start = strchr(start, '"');
            end = strchr(start+1, '"');
            tmp[end-start-1] = '\0';
            strncpy(tmp, start+1, end-start-1);

            if (!strcmp(tmp, "Misc")) {
                fallback_type = get_type_definition();
                type = fallback_type;
            }
            else {
                if (number == 0)
                    number = dummy++;
                type = get_type_definition();
                type_count++;
                types = realloc(types, type_count*sizeof(type_definition *));
                types[type_count-1] = type;
            }

#if 0
            if (!number || number >= OBJECT_TYPE_MAX || types[number] != NULL) {
                /*printf("=> skip\n");*/
                while (read_line(buf, 200, xml) != NULL && strstr(buf, "</type>") == NULL)
                    /*printf(" => skip %s\n", buf)*/;
                /*printf(" => end of skip\n");*/
                continue;
            }
#endif

            type->number = number;

            /*printf("nom %s\n", tmp);*/
            type->name = strdup(tmp);

            read_type(type, xml, "</type>");
        }
    }

    free(fallback_type->description);
    fallback_type->description = strdup("This type regroups all types who don't have a specific definition.");

    for (number = 0; number < type_count; number++) {
        for (attr = 0; attr < types[number]->attribute_count; attr++)
            add_type_to_attribute(get_attribute(types[number]->attributes[attr]->field), types[number], attr);
    }

/*    dump_types();*/
/*    dump_type(default_type);*/
/*    dump_ignore_lists();*/

    write_type_index();
    for (number = 0; number < type_count; number++)
        write_type_file(types[number]);
    write_type_file(fallback_type);

    write_attribute_index();
    for (attr = 0; attr < attribute_count; attr++)
        write_attribute_file(attributes[attr]);

    fclose(xml);
    free(types);
    return 0;
}
