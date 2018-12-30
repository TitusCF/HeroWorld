/*****************************************************************************/
/* Crossfire Plugin Interface Common Parts                                   */
/* Version: 2.0beta8 (also known as "Alexander")                             */
/* Contact: yann.chachkoff@myrealbox.com                                     */
/*****************************************************************************/
/* The CPICP are the various functions of the server that can be used by all */
/* plugins. It is recommended that plugins do not call the server-provided   */
/* functions directly, but instead use the wrappers provided here by the     */
/* CPICP interface. By doing so, various validity checks can be performed    */
/* by CPICP, protecting the server from potential nasty effects of passing   */
/* it wrong parameter values.                                                */
/*****************************************************************************/
/* That code is placed under the GNU General Public Licence (GPL)            */
/* (C)2001-2005 by Chachkoff Yann (Feel free to deliver your complaints)     */
/*****************************************************************************/
/*  CrossFire, A Multiplayer game for X-windows                              */
/*                                                                           */
/*  Copyright (C) 2000 Mark Wedel                                            */
/*  Copyright (C) 1992 Frank Tore Johansen                                   */
/*                                                                           */
/*  This program is free software; you can redistribute it and/or modify     */
/*  it under the terms of the GNU General Public License as published by     */
/*  the Free Software Foundation; either version 2 of the License, or        */
/*  (at your option) any later version.                                      */
/*                                                                           */
/*  This program is distributed in the hope that it will be useful,          */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of           */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            */
/*  GNU General Public License for more details.                             */
/*                                                                           */
/*  You should have received a copy of the GNU General Public License        */
/*  along with this program; if not, write to the Free Software              */
/*  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                */
/*                                                                           */
/*****************************************************************************/

#include <plugin_common.h>
#include <assert.h>

static f_plug_api cfapiSystem_add_string = NULL;
static f_plug_api cfapiSystem_find_string = NULL;
static f_plug_api cfapiSystem_register_global_event = NULL;
static f_plug_api cfapiSystem_remove_string = NULL;
static f_plug_api cfapiSystem_unregister_global_event = NULL;
static f_plug_api cfapiSystem_strdup_local = NULL;
static f_plug_api cfapiSystem_find_animation = NULL;
static f_plug_api cfapiSystem_find_face = NULL;
static f_plug_api cfapiSystem_log = NULL;
static f_plug_api cfapiSystem_get_time = NULL;
static f_plug_api cfapiSystem_timer_create = NULL;
static f_plug_api cfapiSystem_timer_destroy = NULL;
static f_plug_api cfapiSystem_directory = NULL;
static f_plug_api cfapiSystem_re_cmp = NULL;

static f_plug_api cfapiObject_get_property = NULL;
static f_plug_api cfapiObject_set_property = NULL;
static f_plug_api cfapiObject_apply = NULL;
static f_plug_api cfapiObject_identify = NULL;
static f_plug_api cfapiObject_describe = NULL;
static f_plug_api cfapiObject_drain = NULL;
static f_plug_api cfapiObject_fix = NULL;
static f_plug_api cfapiObject_give_skill = NULL;
static f_plug_api cfapiObject_transmute = NULL;
static f_plug_api cfapiObject_remove = NULL;
static f_plug_api cfapiObject_delete = NULL;
static f_plug_api cfapiObject_clone = NULL;
static f_plug_api cfapiObject_create = NULL;
static f_plug_api cfapiObject_insert = NULL;
static f_plug_api cfapiObject_split = NULL;
static f_plug_api cfapiObject_merge = NULL;
static f_plug_api cfapiObject_distance = NULL;
static f_plug_api cfapiObject_update = NULL;
static f_plug_api cfapiObject_clear = NULL;
static f_plug_api cfapiObject_reset = NULL;
static f_plug_api cfapiObject_activate_rune = NULL;
static f_plug_api cfapiObject_check_trigger = NULL;
static f_plug_api cfapiObject_query_money = NULL;
static f_plug_api cfapiObject_query_cost = NULL;
static f_plug_api cfapiObject_query_cost_string = NULL;
static f_plug_api cfapiObject_cast = NULL;
static f_plug_api cfapiObject_learn_spell = NULL;
static f_plug_api cfapiObject_forget_spell = NULL;
static f_plug_api cfapiObject_check_spell = NULL;
static f_plug_api cfapiObject_pay_amount = NULL;
static f_plug_api cfapiObject_pay_item = NULL;
static f_plug_api cfapiObject_transfer = NULL;
static f_plug_api cfapiObject_find_archetype_inside = NULL;
static f_plug_api cfapiObject_find_by_arch_name = NULL;
static f_plug_api cfapiObject_find_by_name = NULL;
static f_plug_api cfapiObject_out_of_map = NULL;
static f_plug_api cfapiObject_drop = NULL;
static f_plug_api cfapiObject_change_abil = NULL;
static f_plug_api cfapiObject_say = NULL;
static f_plug_api cfapiMap_get_property = NULL;
static f_plug_api cfapiMap_set_property = NULL;
static f_plug_api cfapiMap_get_map = NULL;
static f_plug_api cfapiMap_message = NULL;
static f_plug_api cfapiMap_get_object_at = NULL;
static f_plug_api cfapiMap_find_by_archetype_name = NULL;
static f_plug_api cfapiMap_create_path = NULL;
static f_plug_api cfapiMap_has_been_loaded = NULL;
static f_plug_api cfapiMap_change_light = NULL;
static f_plug_api cfapiMap_trigger_connected = NULL;
static f_plug_api cfapiPlayer_find = NULL;
static f_plug_api cfapiPlayer_message = NULL;
static f_plug_api cfapiObject_teleport = NULL;
static f_plug_api cfapiObject_pickup = NULL;
static f_plug_api cfapiObject_get_key = NULL;
static f_plug_api cfapiObject_set_key = NULL;
static f_plug_api cfapiObject_move = NULL;
static f_plug_api cfapiObject_apply_below = NULL;
static f_plug_api cfapiArchetype_get_property = NULL;
static f_plug_api cfapiParty_get_property = NULL;
static f_plug_api cfapiRegion_get_property = NULL;
static f_plug_api cfapiPlayer_can_pay = NULL;
static f_plug_api cfapiFriendlylist_get_next = NULL;
static f_plug_api cfapiSet_random_map_variable = NULL;
static f_plug_api cfapiGenerate_random_map = NULL;
static f_plug_api cfapiObject_change_exp = NULL;
static f_plug_api cfapiSystem_get_month_name = NULL;
static f_plug_api cfapiSystem_get_season_name = NULL;
static f_plug_api cfapiSystem_get_weekday_name = NULL;
static f_plug_api cfapiSystem_get_periodofday_name = NULL;
static f_plug_api cfapiObject_user_event = NULL;
static f_plug_api cfapiCost_string_from_value = NULL;
static f_plug_api cfapiPlayer_quest = NULL;
static f_plug_api cfapiObject_remove_depletion = NULL;
static f_plug_api cfapiPlayer_knowledge = NULL;

#define GET_HOOK(x, y, z) { \
    getHooks(&z, 1, y, &x); \
    if (z != CFAPI_FUNC) { \
        printf("unable to find hook %s!\n", y); \
        return 0;                               \
    } \
}

int cf_init_plugin(f_plug_api getHooks) {
    int z;

    GET_HOOK(cfapiSystem_strdup_local, "cfapi_system_strdup_local", z);
    GET_HOOK(cfapiSystem_add_string, "cfapi_system_add_string", z);
    GET_HOOK(cfapiSystem_register_global_event, "cfapi_system_register_global_event", z);
    GET_HOOK(cfapiSystem_remove_string, "cfapi_system_remove_string", z);
    GET_HOOK(cfapiSystem_directory, "cfapi_system_directory", z);
    GET_HOOK(cfapiSystem_unregister_global_event, "cfapi_system_unregister_global_event", z);
    GET_HOOK(cfapiSystem_find_animation, "cfapi_system_find_animation", z);
    GET_HOOK(cfapiSystem_find_face, "cfapi_system_find_face", z);
    GET_HOOK(cfapiSystem_re_cmp, "cfapi_system_re_cmp", z);
    GET_HOOK(cfapiObject_get_property, "cfapi_object_get_property", z);
    GET_HOOK(cfapiObject_set_property, "cfapi_object_set_property", z);
    GET_HOOK(cfapiObject_apply, "cfapi_object_apply", z);
    GET_HOOK(cfapiObject_identify, "cfapi_object_identify", z);
    GET_HOOK(cfapiObject_describe, "cfapi_object_describe", z);
    GET_HOOK(cfapiObject_drain, "cfapi_object_drain", z);
    GET_HOOK(cfapiObject_fix, "cfapi_object_fix", z);
    GET_HOOK(cfapiObject_give_skill, "cfapi_object_give_skill", z);
    GET_HOOK(cfapiObject_transmute, "cfapi_object_transmute", z);
    GET_HOOK(cfapiObject_remove, "cfapi_object_remove", z);
    GET_HOOK(cfapiObject_delete, "cfapi_object_delete", z);
    GET_HOOK(cfapiObject_clone, "cfapi_object_clone", z);
    GET_HOOK(cfapiObject_create, "cfapi_object_create", z);
    GET_HOOK(cfapiObject_insert, "cfapi_object_insert", z);
    GET_HOOK(cfapiObject_split, "cfapi_object_split", z);
    GET_HOOK(cfapiObject_merge, "cfapi_object_merge", z);
    GET_HOOK(cfapiObject_distance, "cfapi_object_distance", z);
    GET_HOOK(cfapiObject_update, "cfapi_object_update", z);
    GET_HOOK(cfapiObject_clear, "cfapi_object_clear", z);
    GET_HOOK(cfapiObject_reset, "cfapi_object_reset", z);
    GET_HOOK(cfapiObject_activate_rune, "cfapi_object_spring_trap", z);
    GET_HOOK(cfapiObject_check_trigger, "cfapi_object_check_trigger", z);
    GET_HOOK(cfapiObject_query_money, "cfapi_object_query_money", z);
    GET_HOOK(cfapiObject_query_cost, "cfapi_object_query_cost", z);
    GET_HOOK(cfapiObject_query_cost_string, "cfapi_object_query_cost_string", z);
    GET_HOOK(cfapiObject_cast, "cfapi_object_cast", z);
    GET_HOOK(cfapiObject_learn_spell, "cfapi_object_learn_spell", z);
    GET_HOOK(cfapiObject_forget_spell, "cfapi_object_forget_spell", z);
    GET_HOOK(cfapiObject_check_spell, "cfapi_object_check_spell", z);
    GET_HOOK(cfapiObject_pay_amount, "cfapi_object_pay_amount", z);
    GET_HOOK(cfapiObject_pay_item, "cfapi_object_pay_item", z);
    GET_HOOK(cfapiObject_transfer, "cfapi_object_transfer", z);
    GET_HOOK(cfapiObject_find_archetype_inside, "cfapi_object_find_archetype_inside", z);
    GET_HOOK(cfapiObject_remove, "cfapi_object_remove", z);
    GET_HOOK(cfapiObject_delete, "cfapi_object_delete", z);
    GET_HOOK(cfapiObject_out_of_map, "cfapi_map_out_of_map", z);
    GET_HOOK(cfapiObject_drop, "cfapi_object_drop", z);
    GET_HOOK(cfapiObject_change_abil, "cfapi_object_change_abil", z);
    GET_HOOK(cfapiObject_say, "cfapi_object_say", z);
    GET_HOOK(cfapiMap_create_path, "cfapi_map_create_path", z);
    GET_HOOK(cfapiMap_get_property, "cfapi_map_get_property", z);
    GET_HOOK(cfapiMap_set_property, "cfapi_map_set_property", z);
    GET_HOOK(cfapiMap_get_map, "cfapi_map_get_map", z);
    GET_HOOK(cfapiMap_message, "cfapi_map_message", z);
    GET_HOOK(cfapiMap_get_object_at, "cfapi_map_get_object_at", z);
    GET_HOOK(cfapiMap_find_by_archetype_name, "cfapi_map_find_by_archetype_name", z);
    GET_HOOK(cfapiMap_change_light, "cfapi_map_change_light", z);
    GET_HOOK(cfapiMap_has_been_loaded, "cfapi_map_has_been_loaded", z);
    GET_HOOK(cfapiMap_trigger_connected, "cfapi_map_trigger_connected", z);
    GET_HOOK(cfapiPlayer_find, "cfapi_player_find", z);
    GET_HOOK(cfapiPlayer_message, "cfapi_player_message", z);
    GET_HOOK(cfapiObject_teleport, "cfapi_object_teleport", z);
    GET_HOOK(cfapiObject_pickup, "cfapi_object_pickup", z);
    GET_HOOK(cfapiObject_get_key, "cfapi_object_get_key", z);
    GET_HOOK(cfapiObject_set_key, "cfapi_object_set_key", z);
    GET_HOOK(cfapiObject_move, "cfapi_object_move", z);
    GET_HOOK(cfapiObject_apply_below, "cfapi_object_apply_below", z);
    GET_HOOK(cfapiArchetype_get_property, "cfapi_archetype_get_property", z);
    GET_HOOK(cfapiParty_get_property, "cfapi_party_get_property", z);
    GET_HOOK(cfapiRegion_get_property, "cfapi_region_get_property", z);
    GET_HOOK(cfapiPlayer_can_pay, "cfapi_player_can_pay", z);
    GET_HOOK(cfapiSystem_log, "cfapi_log", z);
    GET_HOOK(cfapiSystem_get_time, "cfapi_system_get_time", z);
    GET_HOOK(cfapiSystem_timer_create, "cfapi_system_timer_create", z);
    GET_HOOK(cfapiSystem_timer_destroy, "cfapi_system_timer_destroy", z);
    GET_HOOK(cfapiFriendlylist_get_next, "cfapi_friendlylist_get_next", z);
    GET_HOOK(cfapiSet_random_map_variable, "cfapi_set_random_map_variable", z);
    GET_HOOK(cfapiGenerate_random_map, "cfapi_generate_random_map", z);
    GET_HOOK(cfapiObject_change_exp, "cfapi_object_change_exp", z);
    GET_HOOK(cfapiSystem_get_season_name, "cfapi_system_get_season_name", z);
    GET_HOOK(cfapiSystem_get_month_name, "cfapi_system_get_month_name", z);
    GET_HOOK(cfapiSystem_get_weekday_name, "cfapi_system_get_weekday_name", z);
    GET_HOOK(cfapiSystem_get_periodofday_name, "cfapi_system_get_periodofday_name", z);
    GET_HOOK(cfapiObject_user_event, "cfapi_object_user_event", z);
    GET_HOOK(cfapiSystem_find_string, "cfapi_system_find_string", z);
    GET_HOOK(cfapiCost_string_from_value, "cfapi_cost_string_from_value", z);
    GET_HOOK(cfapiPlayer_quest, "cfapi_player_quest", z);
    GET_HOOK(cfapiObject_remove_depletion, "cfapi_object_remove_depletion", z);
    GET_HOOK(cfapiObject_find_by_arch_name, "cfapi_object_find_by_arch_name", z);
    GET_HOOK(cfapiObject_find_by_name, "cfapi_object_find_by_name", z);
    GET_HOOK(cfapiPlayer_knowledge, "cfapi_player_knowledge", z);
    return 1;
}

/* Should get replaced by tons of more explicit wrappers */
/*void *cf_map_get_property(mapstruct *map, int propcode) {
    int type;
    return cfapiMap_get_property(&type, propcode, map);
}*/

int cf_map_get_int_property(mapstruct *map, int property) {
    int type, value;

    cfapiMap_get_property(&type, map, property, &value);
    assert(type == CFAPI_INT);
    return value;
}

int cf_object_user_event(object *op, object *activator, object *third, const char *message, int fix) {
    int type, value;

    cfapiObject_user_event(&type, op, activator, third, message, fix, &value);
    assert(type == CFAPI_INT);
    return value;
}

sstring cf_map_get_sstring_property(mapstruct *map, int propcode) {
    int type;
    sstring value;

    cfapiMap_get_property(&type, map, propcode, &value);
    assert(type == CFAPI_SSTRING);
    return value;
}

mapstruct *cf_map_get_map_property(mapstruct *map, int propcode) {
    int type;
    mapstruct *value;

    cfapiMap_get_property(&type, map, propcode, &value);
    assert(type == CFAPI_PMAP);
    return value;
}

region *cf_map_get_region_property(mapstruct *map, int propcode) {
    int type;
    region *value;

    cfapiMap_get_property(&type, map, propcode, &value);
    assert(type == CFAPI_PREGION);
    return value;
}

/* Should get replaced by tons of more explicit wrappers */
void cf_map_set_int_property(mapstruct *map, int propcode, int value) {
    int type;

    cfapiMap_set_property(&type, map, propcode, value);
    assert(type == CFAPI_INT);
}

void cf_map_set_string_property(mapstruct *map, int propcode, const char *value) {
    int type;

    cfapiMap_set_property(&type, map, propcode, value);
    assert(type == CFAPI_STRING);
}

/* Should get replaced by tons of more explicit wrappers */
sint16 cf_object_get_resistance(object *op, int rtype) {
    int type;
    sint16 resist;

    cfapiObject_get_property(&type, op, CFAPI_OBJECT_PROP_RESIST, rtype, &resist);
    assert(type == CFAPI_INT16);
    return resist;
}
void cf_object_set_resistance(object *op, int rtype, sint16 value) {
    int type;

    cfapiObject_set_property(&type, op, CFAPI_OBJECT_PROP_RESIST, rtype, value);
    assert(type == CFAPI_INT16);
}

/* Should get replaced by tons of more explicit wrappers */
void cf_object_set_int_property(object *op, int propcode, int value) {
    int type;

    cfapiObject_set_property(&type, op, propcode, value);
    assert(type == CFAPI_INT);
}
int cf_object_get_int_property(object *op, int propcode) {
    int type, value;

    cfapiObject_get_property(&type, op, propcode, &value);
    assert(type == CFAPI_INT);
    return value;
}
long cf_object_get_long_property(object *op, long propcode) {
    int type;
    long value;

    cfapiObject_get_property(&type, op, propcode, &value);
    assert(type == CFAPI_LONG);
    return value;
}
void cf_object_set_movetype_property(object *op, int propcode, MoveType value) {
    int type;

    /* note: MoveType can't be used through va_arg, so use MoveType * */
    cfapiObject_set_property(&type, op, propcode, &value);
    assert(type == CFAPI_MOVETYPE);
}
MoveType cf_object_get_movetype_property(object *op, int propcode) {
    int type;
    MoveType value;

    cfapiObject_get_property(&type, op, propcode, &value);
    assert(type == CFAPI_MOVETYPE);
    return value;
}
object *cf_object_get_object_property(object *op, int propcode) {
    int type;
    object *value;

    cfapiObject_get_property(&type, op, propcode, &value);
    assert(type == CFAPI_POBJECT);
    return value;
}
mapstruct *cf_object_get_map_property(object *op, int propcode) {
    int type;
    mapstruct *value;

    cfapiObject_get_property(&type, op, propcode, &value);
    assert(type == CFAPI_PMAP);
    return value;
}
sint64 cf_object_get_int64_property(object *op, int propcode) {
    int type;
    sint64 value;

    cfapiObject_get_property(&type, op, propcode, &value);
    assert(type == CFAPI_SINT64);
    return value;
}
/* Should get replaced by tons of more explicit wrappers */
void cf_object_set_long_property(object *op, int propcode, long value) {
    int type;

    cfapiObject_set_property(&type, op, propcode, value);
    assert(type == CFAPI_LONG);
}
void cf_object_set_float_property(object *op, int propcode, float value) {
    int type;

    cfapiObject_set_property(&type, op, propcode, value);
    assert(type == CFAPI_FLOAT);
}
void cf_object_set_int64_property(object *op, int propcode, sint64 value) {
    int type;

    cfapiObject_set_property(&type, op, propcode, value);
    assert(type == CFAPI_SINT64);
}
float cf_object_get_float_property(object *op, int propcode) {
    int type;
    float value;

    cfapiObject_get_property(&type, op, propcode, &value);
    assert(type == CFAPI_FLOAT);
    return value;
}
archetype *cf_object_get_archetype_property(object *op, int propcode) {
    int type;
    archetype *value;

    cfapiObject_get_property(&type, op, propcode, &value);
    assert(type == CFAPI_PARCH);
    return value;
}
partylist *cf_object_get_partylist_property(object *op, int propcode) {
    int type;
    partylist *value;

    cfapiObject_get_property(&type, op, propcode, &value);
    assert(type == CFAPI_PPARTY);
    return value;
}
double cf_object_get_double_property(object *op, int propcode) {
    int type;
    double value;

    cfapiObject_get_property(&type, op, propcode, &value);
    assert(type == CFAPI_DOUBLE);
    return value;
}
sstring cf_object_get_sstring_property(object *op, int propcode) {
    int type;
    sstring value;

    cfapiObject_get_property(&type, op, propcode, &value);
    assert(type == CFAPI_SSTRING);
    return value;
}
char *cf_object_get_string_property(object *op, int propcode, char *buf, int size) {
    int type;

    cfapiObject_get_property(&type, op, propcode, buf, size);
    assert(type == CFAPI_STRING);
    return buf;
}
/* Should get replaced by tons of more explicit wrappers */
void cf_object_set_string_property(object *op, int propcode, const char *value) {
    int type;

    /* use cf_object_set_face() for changing the face! */
    assert(propcode != CFAPI_OBJECT_PROP_FACE);
    /* use cf_object_set_animation() to change the animation */
    assert(propcode != CFAPI_OBJECT_PROP_ANIMATION);

    cfapiObject_set_property(&type, op, propcode, value);
    assert(type == CFAPI_STRING);
}

/**
 * Set the object's face.
 * @param op who to change the face for.
 * @param face face to set.
 * @return 0 if invalid face, non zero is set.
 */
int cf_object_set_face(object *op, const char *face) {
    int type, ret;
    cfapiObject_set_property(&type, op, CFAPI_OBJECT_PROP_FACE, face, &ret);
    assert(type == CFAPI_INT);
    return ret;
}
/**
 * Set the object's animation.
 * @param op who to change the animation for.
 * @param animation animation's name to set.
 * @return 0 if invalid animation, non zero is set.
 */
int cf_object_set_animation(object *op, const char *animation) {
    int type, ret;
    cfapiObject_set_property(&type, op, CFAPI_OBJECT_PROP_ANIMATION, animation, &ret);
    assert(type == CFAPI_INT);
    return ret;
}

void cf_object_set_object_property(object *op, int propcode, object *value) {
    int type;

    cfapiObject_set_property(&type, op, propcode, value);
    assert(type == CFAPI_POBJECT);
}

/**
 * Wrapper for change_exp().
 * @copydoc change_exp().
 */
void cf_object_change_exp(object *op, sint64 exp, const char *skill_name, int flag) {
    int type;

    cfapiObject_change_exp(&type, op, exp, skill_name && strlen(skill_name) > 0 ? skill_name : NULL, flag);
    assert(type == CFAPI_NONE);
}
int cf_player_move(player *pl, int dir) {
    int type, ret;

    cfapiObject_move(&type, 1, pl, dir, &ret);
    assert(type == CFAPI_INT);
    return ret;
}
int cf_object_move(object *op, int dir, object*originator) {
    int type, ret;

    cfapiObject_move(&type, 0, op, dir, originator, &ret);
    assert(type == CFAPI_INT);
    return ret;
}
/**
 * Wrapper for apply_manual().
 * @copydoc apply_manual()
 */
int cf_object_apply(object *op, object *tmp, int aflag) {
    int type, ret;

    cfapiObject_apply(&type, op, tmp, aflag, &ret);
    return ret;
}

/**
 * Wrapper for apply_by_living_below().
 * @copydoc apply_by_living_below()
 */
void cf_object_apply_below(object *pl) {
    int type;

    cfapiObject_apply_below(&type, pl);
}
/**
 * Wrapper for object_remove().
 * @copydoc object_remove()
 */
void cf_object_remove(object *op) {
    int type;

    cfapiObject_remove(&type, op);
}
/**
 * Wrapper for object_free_drop_inventory().
 * @copydoc object_free_drop_inventory()
 */
void cf_object_free_drop_inventory(object *ob) {
    int type;

    cfapiObject_delete(&type, ob);
}
/**
 * Kinda wrapper for arch_present_in_ob().
 */
object *cf_object_present_archname_inside(object *op, char *whatstr) {
    int type;
    object *value;

    cfapiObject_find_archetype_inside(&type, op, whatstr, &value);
    assert(type == CFAPI_POBJECT);
    return value;
}

/**
 * Wrapper for object_find_by_arch_name().
 * @copydoc object_find_by_arch_name()
 */
object *cf_object_find_by_arch_name(const object *who, const char *name) {
    int type;
    object *result;

    cfapiObject_find_by_arch_name(&type, who, name, &result);
    assert(type == CFAPI_POBJECT);
    return result;
}

/**
 * Wrapper for object_find_by_name().
 * @copydoc object_find_by_name()
 */
object *cf_object_find_by_name(const object *who, const char *name) {
    int type;
    object *result;

    cfapiObject_find_by_name(&type, who, name, &result);
    assert(type == CFAPI_POBJECT);
    return result;
}

/**
 * Wrapper for transfer_ob().
 * @copydoc transfer_ob()
 */
int cf_object_transfer(object *op, int x, int y, int randomly, object *originator) {
    int type, value;

    cfapiObject_transfer(&type, op, 0, x, y, randomly, originator, &value);
    assert(type == CFAPI_INT);
    return value;
}

/**
 * Wrapper for move_to().
 * @copydoc move_to()
 */
int cf_object_move_to(object *op, int x, int y) {
    int type, value;

    cfapiObject_transfer(&type, op, 2, x, y, &value);
    assert(type == CFAPI_INT);
    return value;
}

/**
 * Wrapper for object_insert_in_map_at().
 * @copydoc object_insert_in_map_at().
 */
object *cf_object_change_map(object *op, mapstruct *m, object *originator, int flag, int x, int y) {
    int type;
    object *value;

    cfapiObject_transfer(&type, op, 1, m, originator, flag, x, y, &value);
    assert(type == CFAPI_POBJECT);
    return value;
}

/**
 * Wrapper for GET_MAP_OB().
 * @copydoc GET_MAP_OB()
 */
object *cf_map_get_object_at(mapstruct *m, int x, int y) {
    int type;
    object *value;

    cfapiMap_get_object_at(&type, m, x, y, &value);
    assert(type == CFAPI_POBJECT);
    return value;
}
/**
 * Partial wrapper for ext_info_map().
 * @todo add missing parameters.
 */
void cf_map_message(mapstruct *m, const char *msg, int color) {
    int type;

    cfapiMap_message(&type, m, msg, color);
}

/**
 * Clone an object.
 * @param op
 * what to clone.
 * @param clonetype
 * - 0 means to clone through object_create_clone().
 * - 1 means to clone through object_copy().
 * @return
 * clone.
 */
object *cf_object_clone(object *op, int clonetype) {
    int type;
    object *value;

    cfapiObject_clone(&type, op, clonetype, &value);
    assert(type == CFAPI_POBJECT);
    return value;
}


/**
 * Wrapper for object_split().
 * @copydoc object_split().
 */
object *cf_object_split(object *orig_ob, uint32 nr, char *err, size_t size) {
    int type;
    object *value;

    cfapiObject_split(&type, orig_ob, nr, err, size, &value);

    if (value == NULL)
    {
        assert(type == CFAPI_NONE);
    }
    else
    {
        assert(type == CFAPI_POBJECT);
    }

    return value;
}


/**
 * Wrapper for pay_for_item().
 * @copydoc pay_for_item().
 */
int cf_object_pay_item(object *op, object *pl) {
    int type, value;

    cfapiObject_pay_item(&type, op, pl, &value);
    assert(type == CFAPI_INT);
    return value;
}

/**
 * Wrapper for pay_for_amount().
 * @copydoc pay_for_amount().
 */
int cf_object_pay_amount(object *pl, uint64 to_pay) {
    int type, value;

    cfapiObject_pay_amount(&type, pl, to_pay, &value);
    assert(type == CFAPI_INT);
    return value;
}

/**
 * Wrapper for cast_spell().
 * @copydoc cast_spell().
 */
int cf_object_cast_spell(object *op, object *caster, int dir, object *spell_ob, char *stringarg) {
    int type, value;

    cfapiObject_cast(&type, op, caster, dir, spell_ob, stringarg, &value);
    assert(type == CFAPI_INT);
    return value;
}
/**
 * Should there be a difference nowadays between that and cast_spell ?
 * @todo
 * either totally remove or replace by cf_object_cast_spell().
 */
int cf_object_cast_ability(object *caster, object *ctoo, int dir, object *sp, char *flags) {
    int type, value;

    cfapiObject_cast(&type, caster, ctoo, dir, sp, flags, &value);
    assert(type == CFAPI_INT);
    return value;
}

/**
 * Wrapper for do_learn_spell().
 * @copydoc do_learn_spell().
 */
void cf_object_learn_spell(object *op, object *spell, int special_prayer) {
    int type;

    cfapiObject_learn_spell(&type, op, spell, special_prayer);
    assert(type == CFAPI_NONE);
}

/**
 * Wrapper for do_forget_spell(), except takes an object, not a string.
 * @todo
 * make coherent with do_forget_spell() (string instead of ob).
 */
void cf_object_forget_spell(object *op, object *sp) {
    int type;

    cfapiObject_forget_spell(&type, op, sp);
}

/**
 * Wrapper for check_spell_known().
 * @copydoc check_spell_known()
 */
object *cf_object_check_for_spell(object *op, const char *name) {
    int type;
    object *value;

    cfapiObject_check_spell(&type, op, name, &value);
    assert(type == CFAPI_POBJECT);
    return value;
}

void cf_player_message(object *op, char *txt, int flags) {
    int type;

    cfapiPlayer_message(&type, flags, 0, op, txt);
    assert(type == CFAPI_NONE);
}

/**
 * Wrapper for remove_depletion().
 * @copydoc remove_depletion()
 */
int cf_object_remove_depletion(object *op, int level) {
    int type, result;

    cfapiObject_remove_depletion(&type, op, level, &result);
    assert(type == CFAPI_INT);
    return result;
}

/**
 * Wrapper for identify().
 * @copydoc identify()
 * @param op
 * @return
 */
object *cf_identify(object *op) {
    int type;
    object *result;

    cfapiObject_identify(&type, op, &result);
    assert(type == CFAPI_POBJECT);
    return result;
}

/**
 * Wrapper for find_player_partial_name().
 * @copydoc find_player_partial_name().
 */
player *cf_player_find(const char *plname) {
    int type;
    player *value;

    cfapiPlayer_find(&type, plname, &value);
    assert(type == CFAPI_PPLAYER);
    return value;
}

char *cf_player_get_title(object *op, char *title, int size) {
    int type;

    cfapiObject_get_property(&type, op, CFAPI_PLAYER_PROP_TITLE, title, size);
    assert(type == CFAPI_STRING);
    return title;
}

void cf_player_set_title(object *op, const char *title) {
    int type;

    cfapiObject_set_property(&type, op, CFAPI_PLAYER_PROP_TITLE, title);
}

sstring cf_player_get_ip(object *op) {
    int type;
    sstring value;

    cfapiObject_get_property(&type, op, CFAPI_PLAYER_PROP_IP, &value);
    assert(type == CFAPI_SSTRING);
    return value;
}

object *cf_player_get_marked_item(object *op) {
    int type;
    object *value;

    cfapiObject_get_property(&type, op, CFAPI_PLAYER_PROP_MARKED_ITEM, &value);
    assert(type == CFAPI_POBJECT);
    return value;
}

void cf_player_set_marked_item(object *op, object *ob) {
    int type;

    cfapiObject_set_property(&type, op, CFAPI_PLAYER_PROP_MARKED_ITEM, ob);
}

partylist *cf_player_get_party(object *op) {
    return cf_object_get_partylist_property(op, CFAPI_PLAYER_PROP_PARTY);
}

void cf_player_set_party(object *op, partylist *party) {
    int type;

    cfapiObject_set_property(&type, op, CFAPI_PLAYER_PROP_PARTY, party);
}

/**
 * Wrapper for can_pay().
 * @copydoc can_pay().
 */
int cf_player_can_pay(object *pl) {
    int type, value;

    cfapiPlayer_can_pay(&type, pl, &value);
    assert(type == CFAPI_INT);
    return value;
}

/**
 * Wrapper for knowledge_player_has().
 * @param op who to check knowledge for.
 * @param knowledge what to check for.
 * @return 0 if op is not a player or knowledge is not known, 1 else.
 */
int cf_player_knowledge_has(object *op, const char *knowledge) {
    int type, value;

    cfapiPlayer_knowledge(&type, 1, op, knowledge, &value);
    assert(type == CFAPI_INT);
    return value;
}

/**
 * Wrapper for knowledge_give();
 * @param op who to give knowledge to.
 * @param knowledge knowledge to give, internal value.
 */
void cf_player_knowledge_give(object *op, const char *knowledge) {
    int type;

    cfapiPlayer_knowledge(&type, 2, op, knowledge);
    assert(type == CFAPI_NONE);
}

/**
 * Wrapper for player_arrest().
 * @copydoc player_arrest()
 */
int cf_player_arrest(object *who) {
    int type, value;
    cfapiObject_move(&type, 2, who, &value);
    assert(type == CFAPI_INT);
    return value;
}

/**
 * Wrapper for ready_map_name().
 * @copydoc ready_map_name()
 */
mapstruct *cf_map_get_map(const char *name, int flags) {
    int type;
    mapstruct *ret;

    cfapiMap_get_map(&type, 1, name, flags, &ret);
    assert(type == CFAPI_PMAP);
    return ret;
}

/**
 * Wrapper for get_empty_map().
 * @copydoc get_empty_map().
 */
mapstruct *cf_get_empty_map(int sizex, int sizey) {
    int type;
    mapstruct *ret;

    cfapiMap_get_map(&type, 0, sizex, sizey, &ret);
    assert(type == CFAPI_PMAP);
    return ret;
}

/**
 * Wrapper for has_been_loaded().
 * @copydoc has_been_loaded()
 */
mapstruct *cf_map_has_been_loaded(const char *name) {
    int type;
    mapstruct *ret;

    cfapiMap_has_been_loaded(&type, name, &ret);
    assert(type == CFAPI_PMAP);
    return ret;
}

/**
 * Gives access to ::first_map.
 * @return
 * ::first_map.
 */
mapstruct *cf_map_get_first(void) {
    return cf_map_get_map_property(NULL, CFAPI_MAP_PROP_NEXT);
}

/**
 * Wrapper for query_money().
 * @copydoc query_money().
 */
int cf_object_query_money(const object *op) {
    int type, value;

    cfapiObject_query_money(&type, op, &value);
    assert(type == CFAPI_INT);
    return value;
}

/**
 * Wrapper for query_cost().
 * @copydoc query_cost().
 */
int cf_object_query_cost(const object *tmp, object *who, int flag) {
    int type, value;

    cfapiObject_query_cost(&type, tmp, who, flag, &value);
    assert(type == CFAPI_INT);
    return value;
}

/**
 * Wrapper for query_cost_string modifier to take a char* and length instead of StringBuffer.
 */
void cf_object_query_cost_string(const object *tmp, object *who, int flag, char *buffer, int length) {
    int type;

    cfapiObject_query_cost_string(&type, tmp, who, flag, buffer, length);
    assert(type == CFAPI_NONE);
}

/**
 * Wrapper for cost_string_from_value modified to take a char* and length instead of a StringBuffer.
 */
void cf_cost_string_from_value(uint64 cost, int largest_coin, char *buffer, int length) {
    int type;

    cfapiCost_string_from_value(&type, cost, largest_coin, buffer, length);
    assert(type == CFAPI_NONE);
}

/**
 * Wrapper for spring_trap().
 * @copydoc spring_trap().
 */
void cf_spring_trap(object *trap, object *victim) {
    int type;

    if (trap)
        cfapiObject_activate_rune(&type, trap, victim);
}

/**
 * Wrapper for check_trigger().
 * @copydoc check_trigger().
 */
int cf_object_check_trigger(object *op, object *cause) {
    int type, value;

    cfapiObject_check_trigger(&type, op, cause, &value);
    assert(type == CFAPI_INT);
    return value;
}

/**
 * Wrapper for trigger_connected().
 * @copydoc trigger_connected().
 */
void cf_map_trigger_connected(objectlink *ol, object *cause, int state) {
    int type;

    cfapiMap_trigger_connected(&type, ol, cause, state);
    assert(type == CFAPI_NONE);
}

int cf_object_out_of_map(object *op, int x, int y) {
    int type, value;

    cfapiObject_out_of_map(&type, op->map, x, y, &value);
    assert(type == CFAPI_INT);
    return value;
}

void cf_object_drop(object *op, object *author) {
    int type;

    cfapiObject_drop(&type, op, author);
}

void cf_object_say(object *op, char *msg) {
    int type, value;

    cfapiObject_say(&type, op, msg, &value);
    assert(type == CFAPI_INT);
}

object *cf_object_insert_object(object *op, object *container) {
    int type;
    object *value;

    cfapiObject_insert(&type, op, 3, container, &value);
    assert(type == CFAPI_POBJECT);
    return value;
}

/**
 * Wrapper for create_pathname().
 * @copydoc create_pathname()
 */
char *cf_get_maps_directory(const char *name, char *buf, int size) {
    int type;

    cfapiMap_create_path(&type, 0, name, buf, size);
    assert(type == CFAPI_STRING);
    return buf;
}

/**
 * Wrapper for object_new().
 * @copydoc object_new().
 */
object *cf_create_object(void) {
    int type;
    object *value;

    cfapiObject_create(&type, 0, &value);
    assert(type == CFAPI_POBJECT);
    return value;
}

/**
 * Wrapper for create_archetype() and create_archetype_by_object_name().
 */
object *cf_create_object_by_name(const char *name) {
    int type;
    object *value;

    cfapiObject_create(&type, 1, name, &value);
    assert(type == CFAPI_POBJECT);
    return value;
}

void cf_system_register_global_event(int event, const char *name, f_plug_event hook) {
    int type;

    cfapiSystem_register_global_event(&type, event, name, hook);
    assert(type == CFAPI_NONE);
}

void cf_system_unregister_global_event(int event, const char *name) {
    int type;

    cfapiSystem_unregister_global_event(&type, event, name);
    assert(type == CFAPI_NONE);
}

/**
 * Gets a directory Crossfire uses.
 * @param id
 * what directory to return:
 * -# @copydoc Settings::mapdir
 * -# @copydoc Settings::uniquedir
 * -# @copydoc Settings::tmpdir
 * -# @copydoc Settings::confdir
 * -# @copydoc Settings::localdir
 * -# @copydoc Settings::playerdir
 * -# @copydoc Settings::datadir
 * @return
 * directory. Must not be altered. NULL if invalid value.
 */
const char *cf_get_directory(int id) {
    int type;
    const char *ret;

    cfapiSystem_directory(&type, id, &ret);
    assert(type == CFAPI_STRING);
    return ret;
}

/**
 * Wrapper for re_cmp().
 * @copydoc re_cmp()
 */
const char *cf_re_cmp(const char *str, const char *regexp) {
    int type;
    const char *result;

    cfapiSystem_re_cmp(&type, str, regexp, &result);
    assert(type == CFAPI_STRING);
    return result;
}

/**
 * Wrapper for fix_object().
 * @copydoc fix_object()
 */
void cf_fix_object(object *op) {
    int type;

    if (op)
        cfapiObject_fix(&type, op);
}

/**
 * Wrapper for add_string().
 * @copydoc add_string()
 */
sstring cf_add_string(const char *str) {
    int type;
    sstring ret;

    if (!str)
        return NULL;
    cfapiSystem_add_string(&type, str, &ret);
    assert(type == CFAPI_SSTRING);
    return ret;
}

/**
 * Wrapper for free_string().
 * @copydoc free_string()
 */
void cf_free_string(sstring str) {
    int type;

    if (str)
        cfapiSystem_remove_string(&type, str);
}

sstring cf_find_string(const char *str) {
    int type;
    sstring ret;

    if (!str)
        return NULL;

    cfapiSystem_find_string(&type, str, &ret);
    assert(type == CFAPI_SSTRING);
    return ret;
}

char *cf_query_name(object *ob, char *name, int size) {
    int type;

    cfapiObject_get_property(&type, ob, CFAPI_OBJECT_PROP_NAME, name, size);
    assert(type == CFAPI_STRING);
    return name;
}

sstring cf_query_name_pl(object *ob) {
    int type;
    sstring value;

    cfapiObject_get_property(&type, ob, CFAPI_OBJECT_PROP_NAME_PLURAL, &value);
    assert(type == CFAPI_SSTRING);
    return value;
}

char *cf_query_base_name(object *ob, int plural, char *name, int size) {
    int type;

    cfapiObject_get_property(&type, ob, CFAPI_OBJECT_PROP_BASE_NAME, name, size);
    assert(type == CFAPI_STRING);
    return name;
}

sstring cf_object_get_msg(object *ob) {
    int type;
    sstring value;

    cfapiObject_get_property(&type, ob, CFAPI_OBJECT_PROP_MESSAGE, &value);
    assert(type == CFAPI_SSTRING);
    return value;
}

void cf_object_set_weight(object *ob, int weight) {
    int type;

    cfapiObject_set_property(&type, ob, CFAPI_OBJECT_PROP_WEIGHT, weight);
    assert(type == CFAPI_INT);
}

void cf_object_set_weight_limit(object *ob, int weight_limit) {
    int type;

    cfapiObject_set_property(&type, ob, CFAPI_OBJECT_PROP_WEIGHT_LIMIT, weight_limit);
    assert(type == CFAPI_INT);
}

int cf_object_get_weight(object *ob) {
    int type, weight;

    cfapiObject_get_property(&type, ob, CFAPI_OBJECT_PROP_WEIGHT, &weight);
    assert(type == CFAPI_INT);
    return weight;
}

int cf_object_get_weight_limit(object *ob) {
    int type, limit;

    cfapiObject_get_property(&type, ob, CFAPI_OBJECT_PROP_WEIGHT_LIMIT, &limit);
    assert(type == CFAPI_INT);
    return limit;
}

/**
 * @return -1=nrof is invalid, 0=nrof is ok#
 */
int cf_object_set_nrof(object *ob, int nrof) {
    int type;

    if (nrof < 0)
        return -1;

    cfapiObject_set_property(&type, ob, CFAPI_OBJECT_PROP_NROF, nrof);
    return 0;
}

int cf_object_get_nrof(object *ob) {
    int type, nrof;

    cfapiObject_get_property(&type, ob, CFAPI_OBJECT_PROP_NROF, &nrof);
    return nrof;
}

int cf_object_get_flag(object *ob, int flag) {
    int type;
    int rv;

    cfapiObject_get_property(&type, ob, CFAPI_OBJECT_PROP_FLAGS, flag, &rv);
    if (rv != 0)
        return 1;
    else
        return 0;
}

void cf_object_set_flag(object *ob, int flag, int value) {
    int type;

    cfapiObject_set_property(&type, ob, CFAPI_OBJECT_PROP_FLAGS, flag, value ? 1 : 0);
}

/**
 * Wrapper for object_insert_in_ob().
 * @copydoc object_insert_in_ob().
 */
object *cf_object_insert_in_ob(object *op, object *where) {
    int type;
    object *value;

    if (!cf_object_get_flag(op, FLAG_REMOVED)) {
        cfapiObject_remove(&type, op);
    }

    cfapiObject_insert(&type, op, 3, where, &value);
    assert(type == CFAPI_POBJECT);
    return value;
}

/**
 * Wrapper for object_insert_in_map().
 * @copydoc object_insert_in_map().
 */
object *cf_map_insert_object_there(object *op, mapstruct *m, object *originator, int flag) {
    int type;
    object *value;

    cfapiObject_insert(&type, op, 1, m, originator, flag, &value);
    assert(type == CFAPI_POBJECT);
    return value;
}

/**
 * Wrapper for object_insert_in_map_at().
 * @todo
 * merge/replace with cf_object_change_map
 */
object *cf_map_insert_object(mapstruct *where, object *op, int x, int y) {
    int type;
    object *value;

    cfapiObject_insert(&type, op, 0, where, NULL, 0 , x, y, &value);
    assert(type == CFAPI_POBJECT);
    return value;
}

/**
 * Will insert op in the map where around the spot x, y.
 * Combination of object_find_free_spot and object_insert_in_map_at.
 * @param where
 * map to insert into.
 * @param op
 * what to insert.
 * @param x
 * @param y
 * where to insert op.
 * @return
 * NULL if op couldn't be inserted around the spot, else op or an object it merged with.
 */
object *cf_map_insert_object_around(mapstruct *where, object *op, int x, int y) {
    int type;
    object *value;

    cfapiObject_insert(&type, op, 2, where, NULL, 0 , x, y, &value);
    assert(type == CFAPI_POBJECT);
    return value;
}

int cf_object_teleport(object *op, mapstruct *map, int x, int y) {
    int type, value;

    cfapiObject_teleport(&type, op, map, x, y, &value);
    assert(type == CFAPI_INT);
    return value;
}

/**
 * Kinda wrapper for map_find_by_archetype_name().
 */
object *cf_map_find_by_archetype_name(const char *str, mapstruct *map, int nx, int ny) {
    int type;
    object *value;

    cfapiMap_find_by_archetype_name(&type, str, map, nx, ny, &value);
    assert(type == CFAPI_POBJECT);
    return value;
}

int cf_map_get_difficulty(mapstruct *map) {
    return cf_map_get_int_property(map, CFAPI_MAP_PROP_DIFFICULTY);
}

int cf_map_get_reset_time(mapstruct *map) {
    return cf_map_get_int_property(map, CFAPI_MAP_PROP_RESET_TIME);
}

int cf_map_get_reset_timeout(mapstruct *map) {
    return cf_map_get_int_property(map, CFAPI_MAP_PROP_RESET_TIMEOUT);
}

int cf_map_get_players(mapstruct *map) {
    return cf_map_get_int_property(map, CFAPI_MAP_PROP_PLAYERS);
}

int cf_map_get_darkness(mapstruct *map) {
    return cf_map_get_int_property(map, CFAPI_MAP_PROP_DARKNESS);
}

int cf_map_get_width(mapstruct *map) {
    return cf_map_get_int_property(map, CFAPI_MAP_PROP_WIDTH);
}

int cf_map_get_height(mapstruct *map) {
    return cf_map_get_int_property(map, CFAPI_MAP_PROP_HEIGHT);
}

int cf_map_get_enter_x(mapstruct *map) {
    return cf_map_get_int_property(map, CFAPI_MAP_PROP_ENTER_X);
}

int cf_map_get_enter_y(mapstruct *map) {
    return cf_map_get_int_property(map, CFAPI_MAP_PROP_ENTER_Y);
}

/**
 * Wrapper for change_map_light().
 * @copydoc change_map_light().
 */
int cf_map_change_light(mapstruct *m, int change) {
    int type, value;

    cfapiMap_change_light(&type, m, change, &value);
    assert(type == CFAPI_INT);
    return value;
}

void cf_object_update(object *op, int flags) {
    int type;

    cfapiObject_update(&type, op, flags);
}

void cf_object_pickup(object *op, object *what) {
    int type;

    cfapiObject_pickup(&type, op, what);
    assert(type == CFAPI_NONE);
}

/**
 * Wrapper for strdup_local().
 *
 * @copydoc strdup_local().
 */
char *cf_strdup_local(const char *str) {
    int type;
    char *dup;

    if (str == NULL)
        return NULL;
    cfapiSystem_strdup_local(&type, str, &dup);
    assert(type == CFAPI_STRING);
    return dup;
}

/**
 * Wrapper for get_map_flags().
 * @copydoc get_map_flags()
 */
int cf_map_get_flags(mapstruct *oldmap, mapstruct **newmap, sint16 x, sint16 y, sint16 *nx, sint16 *ny) {
    int type, value;

    cfapiMap_get_property(&type, oldmap, CFAPI_MAP_PROP_FLAGS, newmap, x, y, nx, ny, &value);
    assert(type == CFAPI_INT);
    return value;
}

/**
 * Wrapper for set_random_map_variable().
 * @copydoc set_random_map_variable()
 */
int cf_random_map_set_variable(RMParms *rp, const char *buf) {
    int type, ret;

    cfapiSet_random_map_variable(&type, rp, buf, &ret);
    assert(type == CFAPI_INT);
    return ret;
}

/**
 * Wrapper for generate_random_map().
 * @copydoc generate_random_map()
 */
mapstruct *cf_random_map_generate(const char *OutFileName, RMParms *RP, char **use_layout) {
    int type;
    mapstruct *map;

    cfapiGenerate_random_map(&type, OutFileName, RP, use_layout, &map);
    assert(type == CFAPI_PMAP);
    return map;
}

/**
 * Wrapper for find_animation().
 * @copydoc find_animation().
 */
int cf_find_animation(const char *name) {
    int type, anim;

    cfapiSystem_find_animation(&type, name, &anim);
    assert(type == CFAPI_INT);
    return anim;
}

/**
 * Wrapper for find_face().
 * @copydoc find_face().
 */
int cf_find_face(const char *name, int error) {
    int type, anim;

    cfapiSystem_find_face(&type, name, error, &anim);
    assert(type == CFAPI_INT);
    return anim;
}

/**
 * Wrapper for LOG().
 * @copydoc LOG().
 */
void cf_log(LogLevel logLevel, const char *format, ...) {
    int type;
    /* Copied from common/logger.c */
    char buf[20480];  /* This needs to be really really big - larger than any other buffer, since that buffer may
        need to be put in this one. */
    va_list ap;

    va_start(ap, format);
    buf[0] = '\0';
    vsprintf(buf, format, ap);
    va_end(ap);

    cfapiSystem_log(&type, logLevel, buf);
    assert(type == CFAPI_NONE);
}

/**
 * Wrapper for LOG() that
 * uses directly a buffer, without format
 */
void cf_log_plain(LogLevel logLevel, const char *message) {
    int type;

    cfapiSystem_log(&type, logLevel, message);
    assert(type == CFAPI_NONE);
}

void cf_get_time(timeofday_t *tod) {
    int type;

    cfapiSystem_get_time(&type, tod);
    assert(type == CFAPI_NONE);
}

const char *cf_get_season_name(int index) {
    int type;
    char *result;

    cfapiSystem_get_season_name(&type, index, &result);
    assert(type == CFAPI_STRING);
    return result;
}

const char *cf_get_month_name(int index) {
    int type;
    char *result;

    cfapiSystem_get_month_name(&type, index, &result);
    assert(type == CFAPI_STRING);
    return result;
}

const char *cf_get_weekday_name(int index) {
    int type;
    char *result;

    cfapiSystem_get_weekday_name(&type, index, &result);
    assert(type == CFAPI_STRING);
    return result;
}

const char *cf_get_periodofday_name(int index) {
    int type;
    char *result;

    cfapiSystem_get_periodofday_name(&type, index, &result);
    assert(type == CFAPI_STRING);
    return result;
}

/**
 * Creates a timer, equivalent of calling cftimer_create().
 *
 * @param ob
 * ::object that will get called. Should handle ::EVENT_TIMER.
 * @param delay
 * delay, seconds or ticks.
 * @param mode
 * timer mode, ::TIMER_MODE_SECONDS or ::TIMER_MODE_CYCLES
 * @return
 * timer identifier, or one of ::TIMER_ERR_ID, ::TIMER_ERR_OBJ or ::TIMER_ERR_MODE
 */
int cf_timer_create(object *ob, long delay, int mode) {
    int type, timer;

    cfapiSystem_timer_create(&type, ob, delay, mode, &timer);
    assert(type == CFAPI_INT);
    return timer;
}

/**
 * Destroys specified timer, equivalent of calling cftimer_destroy().
 *
 * @param id
 * timer to destroy
 * @return
 * ::TIMER_ERR_ID if invalid id, ::TIMER_ERR_NONE else.
 */
int cf_timer_destroy(int id) {
    int type, code;

    cfapiSystem_timer_destroy(&type, id, &code);
    assert(type == CFAPI_INT);
    return code;
}

/**
 * Gets value for specified key, equivalent of object_get_value().
 * @param op
 * ::object for which we search a key.
 * @param keyname
 * key to look for. Not required to be a shared string.
 * @return
 * value (shared string), or NULL if not found.
 */
const char *cf_object_get_key(object *op, const char *keyname) {
    int type;
    const char *value;

    cfapiObject_get_key(&type, op, keyname, &value);
    assert(type == CFAPI_SSTRING);
    return value;
}

/**
 * Sets a value for specified key, equivalent to object_set_value().
 * @param op
 * ::object which will contain the key/value
 * @param keyname
 * key
 * @param value
 * value
 * @param add_key
 * if 0, key is only updated if it exists, else it's updated or added.
 * @return
 * TRUE or FALSE.
 */
int cf_object_set_key(object *op, const char *keyname, const char *value, int add_key) {
    int type, ret;

    cfapiObject_set_key(&type, op, keyname, value, add_key, &ret);
    assert(type == CFAPI_INT);
    return ret;
}

/**
 * Wrapper for change_abil().
 * @copydoc change_abil().
 */
int cf_object_change_abil(object *op, object *tmp) {
    int type, ret;

    cfapiObject_change_abil(&type, op, tmp, &ret);
    assert(type == CFAPI_INT);
    return ret;
}

/* Archetype-related functions */

/**
 * Get first archetype.
 * @return
 * first archetype in the archetype list.
 */
archetype *cf_archetype_get_first(void) {
    int type;
    archetype *value;

    cfapiArchetype_get_property(&type, NULL, CFAPI_ARCH_PROP_NEXT, &value);
    assert(type == CFAPI_PARCH);
    return value;
}

/**
 * Get archetype's name.
 * @param arch
 * archetype, mustn't be NULL.
 * @return
 * archetype's name.
 */
sstring cf_archetype_get_name(archetype *arch) {
    int type;
    sstring name;

    cfapiArchetype_get_property(&type, arch, CFAPI_ARCH_PROP_NAME, &name);
    assert(type == CFAPI_SSTRING);
    return name;
}

/**
 * Get next archetype in linked list.
 * @param arch
 * archetype for which we want the next. Can be NULL, in which case it is equivalent
 * to calling cf_archetype_get_first().
 * @return
 * next archetype.
 */
archetype *cf_archetype_get_next(archetype *arch) {
    int type;
    archetype *value;

    cfapiArchetype_get_property(&type, arch, CFAPI_ARCH_PROP_NEXT, &value);
    assert(type == CFAPI_PARCH);
    return value;
}

/**
 * Get next part of archetype.
 * @param arch
 * archetype, mustn't be NULL.
 * @return
 * archetype's more field.
 */
archetype *cf_archetype_get_more(archetype *arch) {
    int type;
    archetype *value;

    cfapiArchetype_get_property(&type, arch, CFAPI_ARCH_PROP_MORE, &value);
    assert(type == CFAPI_PARCH);
    return value;
}

/**
 * Get head of archetype.
 * @param arch
 * archetype, mustn't be NULL.
 * @return
 * archetype's head field.
 */
archetype *cf_archetype_get_head(archetype *arch) {
    int type;
    archetype *value;

    cfapiArchetype_get_property(&type, arch, CFAPI_ARCH_PROP_HEAD, &value);
    assert(type == CFAPI_PARCH);
    return value;
}

/**
 * Get clone of archetype.
 * @param arch
 * archetype, mustn't be NULL.
 * @return
 * archetype's clone. Will never be NULL.
 */
object *cf_archetype_get_clone(archetype *arch) {
    int type;
    object *value;

    cfapiArchetype_get_property(&type, arch, CFAPI_ARCH_PROP_CLONE, &value);
    assert(type == CFAPI_POBJECT);
    return value;
}

/* Party-related functions */

/**
 * Get first party.
 * @return
 * first party in partylist.
 */
partylist *cf_party_get_first(void) {
    int type;
    partylist *value;

    cfapiParty_get_property(&type, NULL, CFAPI_PARTY_PROP_NEXT, &value);
    assert(type == CFAPI_PPARTY);
    return value;
}

/**
 * @param party
 * party, mustn't be NULL.
 * @return
 * party's name.
 */
const char *cf_party_get_name(partylist *party) {
    int type;
    sstring value;

    cfapiParty_get_property(&type, party, CFAPI_PARTY_PROP_NAME, &value);
    assert(type == CFAPI_SSTRING);
    return value;
}

/**
 * Get next party in party list.
 * @param party
 * party, can be NULL in which case behaves like cf_party_get_first().
 * @return
 * party's next field.
 */
partylist *cf_party_get_next(partylist *party) {
    int type;
    partylist *value;

    cfapiParty_get_property(&type, party, CFAPI_PARTY_PROP_NEXT, &value);
    assert(type == CFAPI_PPARTY);
    return value;
}

/**
 * Get party's password.
 * @param party
 * party, mustn't be NULL.
 * @return
 * party's password field.
 */
const char *cf_party_get_password(partylist *party) {
    int type;
    sstring value;

    cfapiParty_get_property(&type, party, CFAPI_PARTY_PROP_PASSWORD, &value);
    assert(type == CFAPI_SSTRING);
    return value;
}

/**
 * Get first player in party.
 * @param party
 * party, mustn't be NULL.
 * @return
 * party's first player.
 */
player *cf_party_get_first_player(partylist *party) {
    int type;
    player *value;

    cfapiParty_get_property(&type, party, CFAPI_PARTY_PROP_PLAYER, NULL, &value);
    assert(type == CFAPI_PPLAYER);
    return value;
}

/**
 * Get next player in party.
 * @param party
 * party, mustn't be NULL.
 * @param op
 * player we want the next of. Can be NULL, in this case behaves like cf_party_get_first_player().
 * @return
 * party's name.
 */
player *cf_party_get_next_player(partylist *party, player *op) {
    int type;
    player *value;

    cfapiParty_get_property(&type, party, CFAPI_PARTY_PROP_PLAYER, op, &value);
    assert(type == CFAPI_PPLAYER);
    return value;
}

/**
 * Get first region in region list.
 * @return
 * first region.
 */
region *cf_region_get_first(void) {
    int type;
    region *value;

    cfapiRegion_get_property(&type, NULL, CFAPI_REGION_PROP_NEXT, &value);
    assert(type == CFAPI_PREGION);
    return value;
}

/**
 * Get name of region.
 * @param reg
 * region. Mustn't be NULL.
 * @return
 * region's name.
 */
const char *cf_region_get_name(region *reg) {
    int type;
    sstring value;

    cfapiRegion_get_property(&type, reg, CFAPI_REGION_PROP_NAME, &value);
    assert(type == CFAPI_SSTRING);
    return value;
}

/**
 * Get next region in region list.
 * @param reg
 * region. Can be NULL in which case equivalent of cf_region_get_first().
 * @return
 * next region.
 */
region *cf_region_get_next(region *reg) {
    int type;
    region *value;

    cfapiRegion_get_property(&type, reg, CFAPI_REGION_PROP_NEXT, &value);
    assert(type == CFAPI_PREGION);
    return value;
}

/**
 * Get parent of region.
 * @param reg
 * region. Mustn't be NULL.
 * @return
 * region's parent.
 */
region *cf_region_get_parent(region *reg) {
    int type;
    region *value;

    cfapiRegion_get_property(&type, reg, CFAPI_REGION_PROP_PARENT, &value);
    assert(type == CFAPI_PREGION);
    return value;
}

/**
 * Get longname of region.
 * @param reg
 * region. Mustn't be NULL.
 * @return
 * region's longname.
 */
const char *cf_region_get_longname(region *reg) {
    int type;
    sstring value;

    cfapiRegion_get_property(&type, reg, CFAPI_REGION_PROP_LONGNAME, &value);
    assert(type == CFAPI_SSTRING);
    return value;
}

/**
 * Get message of region.
 * @param reg
 * region. Mustn't be NULL.
 * @return
 * region's message.
 */
const char *cf_region_get_message(region *reg) {
    int type;
    sstring value;

    cfapiRegion_get_property(&type, reg, CFAPI_REGION_PROP_MESSAGE, &value);
    assert(type == CFAPI_SSTRING);
    return value;
}

/**
 * Get region's jail x coordinate.
 * @param reg
 * region. Mustn't be NULL.
 * @return
 * region's x coordinate for jail.
 */
int cf_region_get_jail_x(region *reg) {
    int type, value;

    cfapiRegion_get_property(&type, reg, CFAPI_REGION_PROP_JAIL_X, &value);
    assert(type == CFAPI_INT);
    return value;
}

/**
 * Get region's jail y coordinate.
 * @param reg
 * region. Mustn't be NULL.
 * @return
 * region's y coordinate for jail.
 */
int cf_region_get_jail_y(region *reg) {
    int type, value;

    cfapiRegion_get_property(&type, reg, CFAPI_REGION_PROP_JAIL_Y, &value);
    assert(type == CFAPI_INT);
    return value;
}

/**
 * Get jail path of region.
 * @param reg
 * region. Mustn't be NULL.
 * @return
 * region's jail path, can be NULL.
 */
const char *cf_region_get_jail_path(region *reg) {
    int type;
    sstring value;

    cfapiRegion_get_property(&type, reg, CFAPI_REGION_PROP_JAIL_PATH, &value);
    assert(type == CFAPI_SSTRING);
    return value;
}

/* Friendlylist functions. */

/**
 * Get first object on friendly list.
 * @return
 * first object on friendly list.
 */
object *cf_friendlylist_get_first(void) {
    int type;
    object *value;

    cfapiFriendlylist_get_next(&type, NULL, &value);
    assert(type == CFAPI_POBJECT);
    return value;
}

/**
 * Get next object on friendly list.
 * @param ob
 * object we want the next of. If NULL then equivalent of cf_friendlylist_get_first().
 * @return
 * next object.
 */
object *cf_friendlylist_get_next(object *ob) {
    int type;
    object *value;

    cfapiFriendlylist_get_next(&type, ob, &value);
    assert(type == CFAPI_POBJECT);
    return value;
}

/* Quest-related functions */

/**
 * Wrapper for quest_get_player_state().
 * @copydoc quest_get_player_state()
 */
int cf_quest_get_player_state(object *pl, sstring quest_code) {
    int type, ret;

    cfapiPlayer_quest(&type, CFAPI_PLAYER_QUEST_GET_STATE, pl, quest_code, &ret);
    assert(type == CFAPI_INT);

    return ret;
}

/**
 * Wrapper for quest_start().
 * @copydoc quest_start()
 */
void cf_quest_start(object *pl, sstring quest_code, int state) {
    int type;

    cfapiPlayer_quest(&type, CFAPI_PLAYER_QUEST_START, pl, quest_code, state);
    assert(type == CFAPI_NONE);
}

/**
 * Wrapper for quest_set_player_state();
 * @copydoc quest_set_player_state()
 */
void cf_quest_set_player_state(object *pl, sstring quest_code, int state) {
    int type;

    cfapiPlayer_quest(&type, CFAPI_PLAYER_QUEST_SET_STATE, pl, quest_code, state);
    assert(type == CFAPI_NONE);
}

/**
 * Wrapper for quest_was_completed().
 * @copydoc quest_was_completed()
 */
int cf_quest_was_completed(object *pl, sstring quest_code) {
    int type, ret;

    cfapiPlayer_quest(&type, CFAPI_PLAYER_QUEST_WAS_COMPLETED, pl, quest_code, &ret);
    assert(type == CFAPI_INT);

    return ret;
}

#ifdef WIN32
int gettimeofday(struct timeval *time_Info, struct timezone *timezone_Info) {
    /* Get the time, if they want it */
    if (time_Info != NULL) {
        time_Info->tv_sec = time(NULL);
        time_Info->tv_usec = timeGetTime()*1000;
    }
    /* Get the timezone, if they want it */
    if (timezone_Info != NULL) {
        _tzset();
        timezone_Info->tz_minuteswest = _timezone;
        timezone_Info->tz_dsttime = _daylight;
    }
    /* And return */
    return 0;
}
#endif
