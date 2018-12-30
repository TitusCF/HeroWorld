/*****************************************************************************/
/* Crossfire Animator v2.0a                                                  */
/* Contacts: yann.chachkoff@myrealbox.com, tchize@myrealbox.com              */
/*****************************************************************************/
/* That code is placed under the GNU General Public Licence (GPL)            */
/*                                                                           */
/* (C) 2001 David Delbecq for the original code version.                     */
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
#ifndef PLUGIN_ANIM_H
#define PLUGIN_ANIM_H

#define PLUGIN_NAME    "Animator"
#define PLUGIN_VERSION "CFAnim Plugin 2.0"

#ifndef __CEXTRACT__
#include <plugin.h>
#include <plugin_common.h>
#endif

#include <plugin_common.h>
/** Time units the animation can use. @todo add owner's speed unit */
enum time_enum {
    time_second,    /**< One second. */
    time_tick       /**< One server tick. */
};

/** Result of one animation move. */
typedef enum anim_move_result {
    mr_finished,        /**< Move completed. */
    mr_again            /**< Move should continue next time. */
} anim_move_result;

struct CFanimation_struct;
struct CFmovement_struct;

typedef anim_move_result (*CFAnimRunFunc)(struct CFanimation_struct *animation, long int id, void *parameters);

typedef long int (*CFAnimInitFunc)(const char *name, char *parameters, struct CFmovement_struct *);

/** One move in an animation. */
typedef struct CFmovement_struct {
    struct CFanimation_struct *parent;  /**< Animation this move is linked to. */
    CFAnimRunFunc func;                 /**< Function to run for this move. */
    void *parameters;                   /**< Parameters to the function. */
    long int id;                        /**< Identifier, used for various things. */
    int tick;                           /**< Move duration, units depending on parent's time_representation. */
    struct CFmovement_struct *next;     /**< Next move in the animation. */
} CFmovement;

/** One full animation. */
typedef struct CFanimation_struct {
    char *name;
    object *victim;
    object *event;
    int paralyze;
    int invisible;
    int wizard;
    int unique;
    int verbose;
    int ghosted;
    int errors_allowed;
    int delete_end;
    object *corpse;
    long int tick_left;
    enum time_enum time_representation;
    struct CFmovement_struct *nextmovement;
    struct CFanimation_struct *nextanimation;
} CFanimation;

/** Available animation move. */
typedef struct {
    const char *name;           /**< Name as it appears in the animation file. */
    CFAnimInitFunc funcinit;    /**< Function to process the parameters of the move. */
    CFAnimRunFunc funcrun;      /**< Function to run the move. */
} CFanimationHook;

extern CFanimationHook animationbox[];

extern int animationcount;

#ifndef __CEXTRACT__
#include <cfanim_proto.h>
#endif

#endif /* PLUGIN_ANIM_H */
