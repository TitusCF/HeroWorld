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
#include <ob_methods.h>
#include <ob_types.h>

#ifndef __CEXTRACT__
#include <sproto.h>
#endif

static ob_methods base_type;

static ob_methods legacy_type;

/**
 * Initializes the ob_method system. This means initializing legacy_type,
 * base_type, and also calling init_ob_types() from ob_types.c
 */
void init_ob_methods(void) {
    /* Init legacy_type. Note, this is just used as a transitionary fallback
     * until refactoring of type-specific code is complete, and when it is this
     * ob_methods struct should be removed.
     */
    init_ob_method_struct(&legacy_type, NULL);
    legacy_type.apply = legacy_ob_apply;
    legacy_type.process = legacy_ob_process;
    legacy_type.describe = legacy_ob_describe;
    legacy_type.move_on = NULL;

    /* Init base_type, inheriting from legacy_type. The base_type is susposed to
     * be a base class of object that all other object types inherit methods
     * they don't handle individually. Things such as generic drop/pickup code
     * should go here, in addition some other things such as "I don't know how
     * to apply that." messages should be handled from here.
     */
    init_ob_method_struct(&base_type, &legacy_type);
    /* base_type.foobar = common_ob_foobar; */

    /* Init object types methods, inheriting from base_type. */
    init_ob_types(&base_type);
    register_all_ob_types();
}
