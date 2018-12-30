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

/** Registered method handlers. */
ob_methods type_methods[OBJECT_TYPE_MAX];

/**
 * Initializes a ob_methods struct. Make sure this always matches ob_methods.h
 * @param methods ob_method structure to initialize
 * @param fallback Default fallback for the ob_method
 */
void init_ob_method_struct(ob_methods *methods, ob_methods *fallback) {
    methods->fallback = fallback;
    methods->apply = NULL;
    methods->process = NULL;
    methods->describe = NULL;
    methods->move_on = NULL;
    methods->trigger = NULL;
}

/**
 * Initializes the object system.
 *
 * @param base_type
 * base type to use as a base for all types.
 * @todo when migration is complete, the parameter should go, and this function should be called from
 * init_library() instead of init_ob_methods() in server/ob_methods.c.
 */
void init_ob_types(ob_methods *base_type) {
    int tmp;

    for (tmp = 0; tmp < OBJECT_TYPE_MAX; tmp++)
        init_ob_method_struct(&type_methods[tmp], base_type);
}

/* Functions for registering methods for types */
/**
 * Registers the apply method for the given type.
 * @param ob_type The type of object to register this method to
 * @param method The method to link
 */
void register_apply(int ob_type, apply_func method) {
    type_methods[ob_type].apply = method;
}

/**
 * Registers the process method for the given type.
 * @param ob_type The type of object to register this method to
 * @param method The method to link
 */
void register_process(int ob_type, process_func method) {
    type_methods[ob_type].process = method;
}

/**
 * Registers the describe method for the given type.
 * @param ob_type The type of object to register this method to
 * @param method The method to link
 */
void register_describe(int ob_type, describe_func method) {
    type_methods[ob_type].describe = method;
}

/**
 * Registers the move_on method for the given type.
 * @param ob_type The type of object to register this method to
 * @param method The method to link
 */
void register_move_on(int ob_type, move_on_func method) {
    type_methods[ob_type].move_on = method;
}

/**
 * Registers the trigger method for the given type.
 * @param ob_type The type of object to register this method to
 * @param method The method to link
 */
void register_trigger(int ob_type, trigger_func method) {
    type_methods[ob_type].trigger = method;
}
