/*****************************************************************************/
/* Crossfire Plugin Interface Common Parts                                   */
/* Version: 2.0beta8 (also known as "Alexander")                             */
/* Contact: yann.chachkoff@myrealbox.com                                     */
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
#ifndef PLUGIN_COMMON_H
#define PLUGIN_COMMON_H

#ifdef WIN32
#define CF_PLUGIN __declspec(dllexport)
#else
#ifdef HAVE_VISIBILITY
#define CF_PLUGIN __attribute__((visibility("default")))
#else
#define CF_PLUGIN
#endif
#endif

#include <plugin.h>

#define PLUGIN_ERROR_INT        0x0FFFFF

extern int cf_init_plugin(f_plug_api getHooks);
extern void cf_system_register_global_event(int event, const char *name, f_plug_event hook);
extern void cf_system_unregister_global_event(int event, const char *name);

/* General functions */
extern sstring      cf_add_string(const char *str);
extern void         cf_free_string(sstring str);
extern sstring      cf_find_string(const char *str);
extern char        *cf_strdup_local(const char *str);
extern char        *cf_get_maps_directory(const char *name, char *buf, int size);
extern int          cf_find_animation(const char *txt);
extern int          cf_find_face(const char *name, int error);
extern void         cf_log(LogLevel logLevel, const char *format, ...);
extern void         cf_log_plain(LogLevel logLevel, const char *message);
extern void         cf_get_time(timeofday_t *tod);
extern int          cf_timer_create(object *ob, long delay, int mode);
extern int          cf_timer_destroy(int id);
extern const char  *cf_get_directory(int id);
extern const char  *cf_re_cmp(const char *str, const char *regexp);
extern const char  *cf_get_season_name(int index);
extern const char  *cf_get_month_name(int index);
extern const char  *cf_get_weekday_name(int index);
extern const char  *cf_get_periodofday_name(int index);
extern void         cf_cost_string_from_value(uint64 cost, int largest_coin, char *buffer, int length);

/* Objects */
extern void         cf_object_set_int_property(object *op, int propcode, int value);
extern int          cf_object_get_int_property(object *op, int propcode);
extern long         cf_object_get_long_property(object *op, long propcode);
extern void         cf_object_set_movetype_property(object *op, int propcode, MoveType value);
extern MoveType     cf_object_get_movetype_property(object *op, int propcode);
extern object      *cf_object_get_object_property(object *op, int propcode);
extern void         cf_object_set_object_property(object *op, int propcode, object *value);
extern float        cf_object_get_float_property(object *op, int propcode);
extern void         cf_object_set_float_property(object *op, int propcode, float value);
extern mapstruct   *cf_object_get_map_property(object *op, int propcode);
extern archetype   *cf_object_get_archetype_property(object *op, int propcode);
extern partylist   *cf_object_get_partylist_property(object *op, int propcode);
extern sint64       cf_object_get_int64_property(object *op, int propcode);
extern void         cf_object_set_int64_property(object *op, int propcode, sint64 value);
extern double       cf_object_get_double_property(object *op, int propcode);
extern sstring      cf_object_get_sstring_property(object *op, int propcode);
extern char        *cf_object_get_string_property(object *op, int propcode, char *buf, int size);
extern void         cf_fix_object(object *op);
extern char        *cf_query_name(object *ob, char *name, int size);
extern sstring      cf_query_name_pl(object *ob);
extern char        *cf_query_base_name(object *ob, int plural, char *name, int size);
extern const char  *cf_object_get_msg(object *);
extern void         cf_object_set_weight(object *ob, int weight);
extern int          cf_object_get_weight(object *ob);
extern void         cf_object_set_weight_limit(object *ob, int weight);
extern int          cf_object_get_weight_limit(object *ob);
extern int          cf_object_set_nrof(object *, int nrof);
extern int          cf_object_get_nrof(object *);
extern int          cf_object_get_flag(object *ob, int flag);
extern void         cf_object_set_flag(object *ob, int flag, int value);
extern object      *cf_object_insert_in_ob(object *op, object *where);
extern void         cf_object_set_string_property(object *op, int propcode, const char *value);
extern void         cf_spring_trap(object *trap, object *victim);
extern int          cf_object_check_trigger(object *op, object *cause);
extern int          cf_object_query_money(const object *op);
extern int          cf_object_query_cost(const object *tmp, object *who, int flag);
extern void         cf_object_query_cost_string(const object *tmp, object *who, int flag, char *buffer, int length);
extern int          cf_object_cast_spell(object *op, object *caster, int dir, object *spell_ob, char *stringarg);
extern void         cf_object_learn_spell(object *op, object *spell, int special_prayer);
extern void         cf_object_forget_spell(object *op, object *sp);
extern object      *cf_object_check_for_spell(object *op, const char *name);
extern int          cf_object_cast_ability(object *caster, object *ctoo, int dir, object *sp, char *flags);
extern int          cf_object_pay_amount(object *pl, uint64 to_pay);
extern int          cf_object_pay_item(object *op, object *pl);
extern void         cf_object_set_long_property(object *op, int propcode, long value);
extern int          cf_object_transfer(object *op, int x, int y, int randomly, object *originator);
extern int          cf_object_move_to(object *op, int x, int y);
extern int          cf_object_out_of_map(object *op, int x, int y);
extern void         cf_object_drop(object *op, object *author);
extern void         cf_object_say(object *op, char *msg);
extern object      *cf_object_insert_object(object *op, object *container);
extern object      *cf_object_present_archname_inside(object *op, char *whatstr);
extern int          cf_object_apply(object *op, object *author, int flags);
extern void         cf_object_remove(object *op);
extern void         cf_object_free_drop_inventory(object *ob);
extern object      *cf_create_object(void);
extern object      *cf_create_object_by_name(const char *name);
extern object      *cf_object_change_map(object *op, mapstruct *m, object *originator, int flag, int x, int y);
extern int          cf_object_teleport(object *ob, mapstruct *map, int x, int y);
extern void         cf_object_update(object *op, int flags);
extern void         cf_object_pickup(object *op, object *what);
extern const char  *cf_object_get_key(object *op, const char *keyname);
extern int          cf_object_set_key(object *op, const char *keyname, const char *value, int add_key);
extern sint16       cf_object_get_resistance(object *op, int rtype);
extern void         cf_object_set_resistance(object *op, int rtype, sint16 value);
extern int          cf_object_move(object *op, int dir, object*originator);
extern void         cf_object_apply_below(object *pl);
extern object      *cf_object_clone(object *op, int clonetype);
extern void         cf_object_change_exp(object *op, sint64 exp, const char *skill_name, int flag);
extern int          cf_object_change_abil(object *op, object *tmp);
extern int          cf_object_user_event(object *op, object *activator, object *third, const char *message, int fix);
extern int          cf_object_remove_depletion(object *op, int level);
extern object      *cf_object_find_by_arch_name(const object *who, const char *name);
extern object      *cf_object_find_by_name(const object *who, const char *name);
extern object      *cf_object_split(object *orig_ob, uint32 nr, char *err, size_t size);
extern object      *cf_object_clone(object *op, int clonetype);
extern int          cf_object_set_face(object *op, const char *face);
extern int          cf_object_set_animation(object *op, const char *animation);
extern object      *cf_identify(object *op);
/* Maps */
/*extern void        *cf_map_get_property(mapstruct *map, int propcode);*/
extern sstring cf_map_get_sstring_property(mapstruct *map, int propcode);
extern mapstruct *cf_map_get_map_property(mapstruct *map, int propcode);
extern region *cf_map_get_region_property(mapstruct *map, int propcode);
extern int cf_map_get_int_property(mapstruct *map, int property);

extern void         cf_map_set_int_property(mapstruct *map, int propcode, int value);
extern void         cf_map_set_string_property(mapstruct *map, int propcode, const char *value);
extern mapstruct   *cf_map_get_map(const char *name, int flags);
extern mapstruct   *cf_get_empty_map(int sizex, int sizey);
extern mapstruct   *cf_map_get_first(void);
extern mapstruct   *cf_map_has_been_loaded(const char *name);
extern void         cf_map_message(mapstruct *m, const char *msg, int color);
extern object      *cf_map_get_object_at(mapstruct *m, int x, int y);
extern object      *cf_map_insert_object(mapstruct *where, object *op, int x, int y);
extern object      *cf_map_insert_object_around(mapstruct *where, object *op, int x, int y);
extern object      *cf_map_find_by_archetype_name(const char *str, mapstruct *map, int nx, int ny);
extern int          cf_map_get_flags(mapstruct *oldmap, mapstruct **newmap, sint16 x, sint16 y, sint16 *nx, sint16 *ny);
extern object      *cf_map_insert_object_there(object *op, mapstruct *m, object *originator, int flag);
extern int          cf_map_get_difficulty(mapstruct *map);
extern int          cf_map_get_reset_time(mapstruct *map);
extern int          cf_map_get_reset_timeout(mapstruct *map);
extern int          cf_map_get_players(mapstruct *map);
extern int          cf_map_get_darkness(mapstruct *map);
extern int          cf_map_get_light(mapstruct *map);
extern int          cf_map_get_width(mapstruct *map);
extern int          cf_map_get_height(mapstruct *map);
extern int          cf_map_get_enter_x(mapstruct *map);
extern int          cf_map_get_enter_y(mapstruct *map);
extern int          cf_map_change_light(mapstruct *m, int change);
extern void         cf_map_trigger_connected(objectlink *ol, object *cause, int state);

/* Random maps */
extern int          cf_random_map_set_variable(RMParms *rp, const char *buf);
extern mapstruct   *cf_random_map_generate(const char *OutFileName, RMParms *RP, char **use_layout);

/* Players */
extern char        *cf_player_get_title(object *op, char *title, int size);
extern void         cf_player_set_title(object *op, const char *title);
extern sstring      cf_player_get_ip(object *op);
extern object      *cf_player_get_marked_item(object *op);
extern void         cf_player_set_marked_item(object *op, object *ob);
extern player      *cf_player_find(const char *plname);
extern void         cf_player_message(object *op, char *txt, int flags);
extern int          cf_player_move(player *pl, int dir);
extern partylist   *cf_player_get_party(object *op);
extern void         cf_player_set_party(object *op, partylist *party);
extern int          cf_player_can_pay(object *op);
extern int          cf_player_knowledge_has(object *op, const char *knowledge);
extern void         cf_player_knowledge_give(object *op, const char *knowledge);
extern int          cf_player_arrest(object *who);

/* Archetypes */
extern archetype   *cf_archetype_get_first(void);
extern sstring      cf_archetype_get_name(archetype *arch);
extern archetype   *cf_archetype_get_next(archetype *arch);
extern archetype   *cf_archetype_get_more(archetype *arch);
extern archetype   *cf_archetype_get_head(archetype *arch);
extern object      *cf_archetype_get_clone(archetype *arch);

/* Parties */
extern partylist   *cf_party_get_first(void);
extern const char  *cf_party_get_name(partylist *party);
extern partylist   *cf_party_get_next(partylist *party);
extern const char  *cf_party_get_password(partylist *party);
extern player      *cf_party_get_first_player(partylist *party);
extern player      *cf_party_get_next_player(partylist *party, player *op);

/* Regions */
extern region      *cf_region_get_first(void);
extern const char  *cf_region_get_name(region *reg);
extern region      *cf_region_get_next(region *reg);
extern region      *cf_region_get_parent(region *reg);
extern const char  *cf_region_get_longname(region *reg);
extern const char  *cf_region_get_message(region *reg);
extern int         cf_region_get_jail_x(region *reg);
extern int         cf_region_get_jail_y(region *reg);
extern const char  *cf_region_get_jail_path(region *reg);

/* Friendly list */
extern object      *cf_friendlylist_get_first(void);
extern object      *cf_friendlylist_get_next(object *ob);

/* Quest-related functions */
extern int          cf_quest_get_player_state(object *pl, sstring quest_code);
extern void         cf_quest_start(object *pl, sstring quest_code, int state);
extern void         cf_quest_set_player_state(object *pl, sstring quest_code, int state);
extern int          cf_quest_was_completed(object *pl, sstring quest_code);


#ifdef WIN32

struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};

int gettimeofday(struct timeval *time_Info, struct timezone *timezone_Info);

#endif

#endif /* PLUGIN_COMMON_H */
