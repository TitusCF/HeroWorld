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
 * Those functions deal with the object/type system.
 */

#include <global.h>
#include <ob_types.h>
#include <ob_methods.h>

#ifndef __CEXTRACT__
#include <sproto.h>
#endif

/**
 * Calls the intialization functions for all individual types.
 * @todo this should probably be moved to a file in the types/ directory, to separate types and server.
 */
void register_all_ob_types(void) {
    /* init_type_foobar() here, where foobar is for a type. In other words,
     * from here, call functions that register object methods for types.
     */
    init_type_altar();
    init_type_armour_improver();
    init_type_arrow();
    init_type_blindness();
    init_type_book();
    init_type_button();
    init_type_cf_handle();
    init_type_check_inv();
    init_type_clock();
    init_type_container();
    init_type_converter();
    init_type_creator();
    init_type_deep_swamp();
    init_type_detector();
    init_type_director();
    init_type_duplicator();
    init_type_exit();
    init_type_food();
    init_type_gate();
    init_type_hole();
    init_type_identify_altar();
    init_type_lamp();
    init_type_lighter();
    init_type_marker();
    init_type_mood_floor();
    init_type_peacemaker();
    init_type_pedestal();
    init_type_player_changer();
    init_type_player_mover();
    init_type_poison();
    init_type_poisoning();
    init_type_potion();
    init_type_power_crystal();
    init_type_rune();
    init_type_savebed();
    init_type_scroll();
    init_type_shop_inventory();
    init_type_shop_mat();
    init_type_sign();
    init_type_skillscroll();
    init_type_spell_effect();
    init_type_spellbook();
    init_type_spinner();
    init_type_teleporter();
    init_type_thrown_object();
    init_type_transport();
    init_type_trap();
    init_type_trapdoor();
    init_type_treasure();
    init_type_trigger();
    init_type_trigger_altar();
    init_type_trigger_button();
    init_type_trigger_pedestal();
    init_type_weapon_improver();
    init_type_dragon_focus();
}
