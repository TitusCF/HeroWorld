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

/* First let's include the header file needed                                */

#include <assert.h>
#include <cfanim.h>
#include <stdarg.h>
#include <svnversion.h>

CF_PLUGIN char SvnRevPlugin[] = SVN_REV;

static CFanimation *first_animation = NULL;  /**< Animations we're currently processing. */

static int get_boolean(const char *strg, int *bl);

/**
 * Returns the direction from its name.
 * @param name direction's name
 * @return direction or -1 if unknown.
 */
static int get_dir_from_name(const char *name) {
    if (!strcmp(name, "north"))
        return 1;
    if (!strcmp(name, "north_east"))
        return 2;
    if (!strcmp(name, "east"))
        return 3;
    if (!strcmp(name, "south_east"))
        return 4;
    if (!strcmp(name, "south"))
        return 5;
    if (!strcmp(name, "south_west"))
        return 6;
    if (!strcmp(name, "west"))
        return 7;
    if (!strcmp(name, "north_west"))
        return 8;
    return -1;
}

static long int initmovement(const char *name, char *parameters, struct CFmovement_struct *move_entity) {
    int dir;

    dir = get_dir_from_name(name);
    move_entity->parameters = NULL;
    return dir;
}

static anim_move_result runmovement(struct CFanimation_struct *animation, long int id, void *parameters) {
    object *op = animation->victim;
    int dir = id;

    cf_log(llevDebug, "CFAnim: Moving in direction %ld\n", id);
    if (op->type == PLAYER)
        cf_player_move(op->contr, dir);
    else
        cf_object_move(op, dir, op);
    return mr_finished;
}

static long int initfire(const char *name, char *parameters, struct CFmovement_struct *move_entity) {
    int dir;

    dir = get_dir_from_name(&(name[5]));
    move_entity->parameters = NULL;
    return dir;
}

/** @todo fix */
static anim_move_result runfire(struct CFanimation_struct *animation, long int id, void *parameters) {
    if (animation->verbose)
        cf_log(llevDebug, "CFAnim: Firing in direction %ld\n", id);
    return mr_finished;
}

static long int initturn(const char *name, char *parameters, struct CFmovement_struct *move_entity) {
    int dir;

    dir = get_dir_from_name(&(name[5]));
    move_entity->parameters = NULL;
    return dir;
}

static anim_move_result runturn(struct CFanimation_struct *animation, long int id, void *parameters) {
    object *op = animation->victim;
    int dir = id;
    /*int face;*/

    if (animation->verbose)
        cf_log(llevDebug, "CFAnim: Turning in direction %ld\n", id);
    op->facing = dir;
    /** @todo fix suspicious or missing call */
/*    cf_object_set_int_property(op, CFAPI_OBJECT_PROP_ANIMATION, face);*/
    return mr_finished;
}

static long int initcamera(const char *name, char *parameters, struct CFmovement_struct *move_entity) {
    int dir;

    dir = get_dir_from_name(&(name[7]));
    move_entity->parameters = NULL;
    return dir;
}

/** @todo fix */
static anim_move_result runcamera(struct CFanimation_struct *animation, long int id, void *parameters) {
    if (animation->verbose)
        cf_log(llevDebug, "CFAnim: Moving the camera in direction %ld\n", id);
    return mr_finished;
    /*if (animation->victim->type == PLAYER)
        hook_scroll_map(animation->victim, id);
    else
        printf("CFAnim: Not a player\n");
    return 1;*/
}

static long int initvisible(const char *name, char *parameters, struct CFmovement_struct *move_entity) {
    int result;

    if (get_boolean(parameters, &result))
        return result;
    cf_log(llevError, "CFAnim: Error in animation - possible values for 'invisible' are 'yes' and 'no'\n");
    return -1;
}

static anim_move_result runvisible(struct CFanimation_struct *animation, long int id, void *parameters) {
    if (id == -1)
        return mr_finished;
    animation->invisible = id;
    return mr_finished;
}

static long int initwizard(const char *name, char *parameters, struct CFmovement_struct *move_entity) {
    int result;

    if (get_boolean(parameters, &result))
        return result;
    cf_log(llevError, "CFAnim: Error in animation - possible values for 'wizard' are 'yes' and 'no'\n");
    return -1;
}

static anim_move_result runwizard(struct CFanimation_struct *animation, long int id, void *parameters) {
    if (id == -1)
        return 1;
    animation->wizard = id;
    return mr_finished;
}

static long int initsay(const char *name, char *parameters, struct CFmovement_struct *move_entity) {
    if (parameters)
        move_entity->parameters = cf_strdup_local(parameters);
    else
        move_entity->parameters = NULL;
    if (move_entity->parent->verbose)
        cf_log(llevDebug, "CFAnim: init say: parameters: %s\n", parameters ? parameters : "null");
    return 1;
}

static anim_move_result runsay(struct CFanimation_struct *animation, long int id, void *parameters) {
    if (parameters) {
        cf_object_say(animation->victim, parameters);
        free(parameters);
    } else
        cf_log(llevError, "CFAnim: Error in animation: nothing to say with say function\n");
    return mr_finished;
}

static long int initapply(const char *name, char *parameters, struct CFmovement_struct *move_entity) {
    return 1;
}

static anim_move_result runapply(struct CFanimation_struct *animation, long int id, void *parameters) {
    object *current_container;

    if (animation->victim->type != PLAYER)
        return mr_finished;
    current_container = animation->victim->container;
    animation->victim->container = NULL;
    cf_object_apply_below(animation->victim);
    animation->victim->container = current_container;
    return mr_finished;
}

static long int initapplyobject(const char *name, char *parameters, struct CFmovement_struct *move_entity) {
    move_entity->parameters = parameters ? (void*)cf_add_string(parameters) : NULL;
    return 1;
}

static anim_move_result runapplyobject(struct CFanimation_struct *animation, long int id, void *parameters) {
    object *current;
    int aflag;

    if (!parameters)
        return mr_finished;
    current = animation->victim->below;
    FOR_OB_AND_BELOW_PREPARE(current)
        if (current->name == parameters)
            break;
    FOR_OB_AND_BELOW_FINISH();
    if (!current)
        current = cf_object_find_by_name(animation->victim, parameters);
    if (!current) {
        cf_free_string(parameters);
        return mr_finished;
    }
    aflag = AP_APPLY;
    cf_object_apply(animation->victim, current, aflag);
    cf_free_string(parameters);
    return mr_finished;
}

static long int initdropobject(const char *name, char *parameters, struct CFmovement_struct *move_entity) {
    move_entity->parameters = parameters ? cf_add_string(parameters) : NULL;
    return 1;
}

static anim_move_result rundropobject(struct CFanimation_struct *animation, long int id, void *parameters) {
    object *what;
    if (!parameters)
        return mr_finished;
    what = cf_object_find_by_name(animation->victim, parameters);
    if (what != NULL)
        cf_object_drop(what, animation->victim);
    cf_free_string(parameters);
    return mr_finished;
}

static long int initpickup(const char *name, char *parameters, struct CFmovement_struct *move_entity) {
    return 1;
}

static anim_move_result runpickup(struct CFanimation_struct *animation, long int id, void *parameters) {
    object *current;

    current = animation->victim->below;
    if (!current)
        return mr_finished;
    cf_object_pickup(animation->victim, current);
    return mr_finished;
}

static long int initpickupobject(const char *name, char *parameters, struct CFmovement_struct *move_entity) {
    move_entity->parameters = parameters ? (void*)cf_add_string(parameters) : NULL;
    return 1;
}

static anim_move_result runpickupobject(struct CFanimation_struct *animation, long int id, void *parameters) {
    if (!parameters)
        return mr_finished;
    FOR_BELOW_PREPARE(animation->victim, current)
        if (current->name == parameters) {
            cf_object_pickup(animation->victim, current);
            break;
        }
    FOR_BELOW_FINISH();
    cf_free_string(parameters);
    return mr_finished;
}

static long int initghosted(const char *name, char *parameters, struct CFmovement_struct *move_entity) {
    int result;

    if (get_boolean(parameters, &result))
        return result;
    cf_log(llevError, "CFAnim: Error in animation: possible values for 'ghosted' are 'yes' and 'no'\n");
    return -1;
}

static anim_move_result runghosted(struct CFanimation_struct *animation, long int id, void *parameters) {
    object *corpse;

    if ((id && animation->ghosted)
    || (!id && !animation->ghosted))
        runghosted(animation, !id, parameters);
    if (id) { /*Create a ghost/corpse pair*/
        corpse = cf_object_clone(animation->victim, 1);
        corpse->x = animation->victim->x;
        corpse->y = animation->victim->y;
        corpse->type = 0;
        CLEAR_FLAG(corpse, FLAG_WIZ);
        corpse->contr = NULL;
        cf_map_insert_object_there(corpse, animation->victim->map, NULL, 0);
        animation->wizard = 1;
        animation->invisible = 1;
        animation->corpse = corpse;
    } else { /*Remove a corpse, make current player visible*/
        animation->wizard = 0;
        animation->invisible = 0;
        cf_object_remove(animation->corpse);
        cf_object_free_drop_inventory(animation->corpse);
        animation->corpse = NULL;
        animation->victim->invisible = 0;
        cf_player_move(animation->victim->contr, 0);
    }
    animation->ghosted = id;
    return mr_finished;
}

typedef struct {
    char *mapname;
    int mapx;
    int mapy;
} teleport_params;

static long int initteleport(const char *name, char *parameters, struct CFmovement_struct *move_entity) {
    char *mapname;
    int mapx;
    int mapy;
    teleport_params *teleport;

    move_entity->parameters = NULL;
    cf_log(llevDebug, ".(%s)\n", parameters);
    if (!parameters) {
        cf_log(llevError, "CFAnim: Error - no parameters for teleport\n");
        return 0;
    }
    mapname = strstr(parameters, " ");
    cf_log(llevDebug, ".(%s)\n", parameters);
    if (!mapname)
        return 0;
    *mapname = '\0';
    mapx = atoi(parameters);
    mapname++;
    parameters = mapname;
    if (!parameters) {
        cf_log(llevError, "CFAnim: Error - not enough parameters for teleport\n");
        return 0;
    }
    cf_log(llevDebug, ".(%s)\n", parameters);
    mapname = strstr(parameters, " ");
    cf_log(llevDebug, ".\n");
    if (!mapname)
        return 0;
    *mapname = '\0';
    mapy = atoi(parameters);
    mapname++;
    if (mapname[0] == '\0')
        return 0;
    teleport = (teleport_params *)malloc(sizeof(teleport_params));
    teleport->mapname = cf_strdup_local(mapname);
    teleport->mapx = mapx;
    teleport->mapy = mapy;
    move_entity->parameters = teleport;
    return 1;
}

static anim_move_result runteleport(struct CFanimation_struct *animation, long int id, void *parameters) {
    teleport_params *teleport = (teleport_params *)parameters;

    if (!parameters)
        return mr_finished;
    cf_object_teleport(animation->victim, cf_map_get_map(teleport->mapname, 0), teleport->mapx, teleport->mapy);
    free(parameters);
    return mr_finished;
}

static long int initnotice(const char *name, char *parameters, struct CFmovement_struct *move_entity) {
    move_entity->parameters = parameters ? cf_strdup_local(parameters) : NULL;
    return 1;
}

static  anim_move_result runnotice(struct CFanimation_struct *animation, long int id, void *parameters) {
    int val;

    val = NDI_NAVY|NDI_UNIQUE;

    cf_player_message(animation->victim, parameters, val);
    return mr_finished;
}

static long int initstop(const char *name, char *parameters, struct CFmovement_struct *move_entity) {
    return 1;
}

/** @todo fix */
static anim_move_result runstop(struct CFanimation_struct *animation, long int id, void *parameters) {
    if (animation->verbose)
        cf_log(llevDebug, "CFAnim: stop encountered\n");
    return mr_finished;
}

/** Destination for moveto command. */
typedef struct {
    int x, y;   /**< Coordinates. */
} param_moveto;

static long int initmoveto(const char *name, char *parameters, struct CFmovement_struct *move_entity) {
    param_moveto *moveto;
    int x, y;

    if (sscanf(parameters, "%d %d", &x, &y) != 2)
        return 0;

    moveto = (param_moveto *)calloc(1, sizeof(param_moveto));
    moveto->x = x;
    moveto->y = y;
    move_entity->parameters = moveto;

    return 1;
}

static anim_move_result runmoveto(struct CFanimation_struct *animation, long int id, void *parameters) {
    int move;
    param_moveto *dest = (param_moveto *)parameters;

    if (!dest)
        return mr_finished;

    move = cf_object_move_to(animation->victim, dest->x, dest->y);

    if (animation->victim->x == dest->x && animation->victim->y == dest->y) {
        free(parameters);
        return mr_finished;
    }

    if (move == 1)
        return mr_again;

    return mr_finished;
}

static long int initmessage(const char *name, char *parameters, struct CFmovement_struct *move_entity) {
    if (parameters)
        move_entity->parameters = strdup(parameters);
    else
        move_entity->parameters = NULL;
    return 1;
}

static anim_move_result runmessage(struct CFanimation_struct *animation, long int id, void *parameters) {
    if (parameters && animation->victim->map) {
        cf_map_message(animation->victim->map, (const char *)parameters, NDI_UNIQUE|NDI_GREEN);
        free(parameters);
    }

    return mr_finished;
}

static long int inittrigger(const char *name, char *parameters, struct CFmovement_struct *move_entity) {
    long int connection;

    move_entity->parameters = NULL;
    if (sscanf(parameters, "%ld", &connection) != 1 || connection <= 0) {
        cf_log(llevError, "CFAnim: invalid connection %s\n", parameters);
        return 0;
    }
    return connection;
}

static anim_move_result runtrigger(struct CFanimation_struct *animation, long int id, void *parameters) {
    oblinkpt *olp;
    mapstruct *map;
    objectlink *ol = NULL;

    if (animation->victim == NULL || animation->victim->map == NULL) {
        cf_log(llevError, "CFAnim: trigger for victim not on map or NULL.\n");
        return mr_finished;
    }

    map = animation->victim->map;

    /* locate objectlink for this connected value */
    if (!map->buttons) {
        cf_log(llevError, "Map %s called for trigger on connected %d but there ain't any button list for that map!\n", cf_map_get_sstring_property(map, CFAPI_MAP_PROP_PATH), id);
        return mr_finished;
    }
    for (olp = map->buttons; olp; olp = olp->next) {
        if (olp->value == id) {
            ol = olp->link;
            break;
        }
    }
    if (ol == NULL) {
        cf_log(llevError, "Map %s called for trigger on connected %d but there ain't any button list for that map!\n", cf_map_get_sstring_property(map, CFAPI_MAP_PROP_PATH), id);
        return mr_finished;
    }
    /* run the object link */
    cf_map_trigger_connected(ol, NULL, 1);

    return mr_finished;
}

/** Available animation commands. */
CFanimationHook animationbox[] = {
    { "north", initmovement, runmovement },
    { "north_east", initmovement, runmovement },
    { "east", initmovement, runmovement },
    { "south_east", initmovement, runmovement },
    { "south", initmovement, runmovement },
    { "south_west", initmovement, runmovement },
    { "west", initmovement, runmovement },
    { "north_west", initmovement, runmovement },
    { "fire_north", initfire, runfire },
    { "fire_north_east", initfire, runfire },
    { "fire_east", initfire, runfire },
    { "fire_south_east", initfire, runfire },
    { "fire_south", initfire, runfire },
    { "fire_south_west", initfire, runfire },
    { "fire_west", initfire, runfire },
    { "fire_north_west", initfire, runfire },
    { "turn_north", initturn, runturn },
    { "turn_north_east", initturn, runturn },
    { "turn_east", initturn, runturn },
    { "turn_south_east", initturn, runturn },
    { "turn_south", initturn, runturn },
    { "turn_south_west", initturn, runturn },
    { "turn_west", initturn, runturn },
    { "turn_north_west", initturn, runturn },
    { "camera_north", initcamera, runcamera },
    { "camera_north_east", initcamera, runcamera },
    { "camera_east", initcamera, runcamera },
    { "camera_south_east", initcamera, runcamera },
    { "camera_south", initcamera, runcamera },
    { "camera_south_west", initcamera, runcamera },
    { "camera_west", initcamera, runcamera },
    { "camera_north_west", initcamera, runcamera },
    { "invisible", initvisible, runvisible },
    { "wizard", initwizard, runwizard },
    { "say", initsay, runsay },
    { "apply", initapply, runapply },
    { "apply_object", initapplyobject, runapplyobject },
    { "drop_object", initdropobject, rundropobject },
    { "pickup", initpickup, runpickup },
    { "pickup_object", initpickupobject, runpickupobject },
    { "ghosted", initghosted, runghosted },
    { "teleport", initteleport, runteleport },
    { "notice", initnotice, runnotice },
    { "stop", initstop, runstop },
    { "moveto", initmoveto, runmoveto },
    { "message", initmessage, runmessage },
    { "trigger", inittrigger, runtrigger }
};

int animationcount = sizeof(animationbox)/sizeof(CFanimationHook);

static int ordered_commands = 0;

static int compareAnims(const void *a, const void *b) {
    return strcmp(((const CFanimationHook *)a)->name, ((const CFanimationHook *)b)->name);
}

static void prepare_commands(void) {
    qsort(animationbox, animationcount, sizeof(CFanimationHook), compareAnims);
    ordered_commands = 1;
}

static CFanimationHook *get_command(char *command) {
    CFanimationHook dummy;

    dummy.name = command;
    if (!ordered_commands)
        prepare_commands();
    return (CFanimationHook *)bsearch(&dummy, animationbox, animationcount, sizeof(CFanimationHook), compareAnims);
}

/**
 * Parse an animation block in the animation file.
 * @param buffer buffer to read data info, will have been modified when function exits.
 * @param buffer_size size of buffer.
 * @param fichier file to read from.
 * @param parent animation we're reading the block for.
 * @return one animation frame.
 */
static CFmovement *parse_animation_block(char *buffer, size_t buffer_size, FILE *fichier, CFanimation *parent) {
    CFmovement *first = NULL;
    CFmovement *current = NULL;
    CFmovement *next;
    char *time;
    char *name;
    char *parameters;
    int tick;
    CFanimationHook *animationhook;

    if (parent->verbose)
        cf_log(llevDebug, "CFAnim: In parse block for %s\n", buffer);
    while (fgets(buffer, buffer_size, fichier)) {
        if (buffer[0] == '[')
            break;
        if (buffer[0] == '#')
            continue;
        buffer[strlen(buffer)-strlen("\n")] = '\0';
        while (buffer[strlen(buffer)-1] == ' ')
            buffer[strlen(buffer)-1] = '\0';
        if (strlen(buffer) <= 0)
            continue;
        time = buffer;

        name = strstr(buffer, " ");
        if (!name)
            continue;
        *name = '\0';
        name++;
        while (*name == ' ')
            name++;

        tick = atoi(time);
        if (tick < 0)
            continue;

        parameters = strstr(name, " ");
        if (parameters) { /*Parameters may be nul*/
            *parameters = '\0';
            parameters++;
            while (*parameters == ' ')
                parameters++;
            if (*parameters == '\0')
                parameters = NULL;
        }
        animationhook = get_command(name);
        if (!animationhook)
            cf_log(llevError, "CFAnim: %s - Unknown animation command\n", name);
        else if (parent->verbose) {
            cf_log(llevDebug, "CFAnim: Parsed %s -> %p\n", name, animationhook);
        }
        if (!animationhook) {
            if (parent->errors_allowed)
                continue;
            else
                break;
        }
        next = (CFmovement *)malloc(sizeof(CFmovement));
        if (!next)
            continue;
        next->parent = parent;
        next->tick = tick;
        next->next = NULL;
        if (animationhook->funcinit)
            next->id = animationhook->funcinit(name, parameters, next);
        next->func = animationhook->funcrun;
        if (current)
            current->next = next;
        else
            first = next;
        current = next;
    }
    return first;
}

/**
 * This function take buffer with a value like "blabla= things" and extracts some things.
 *
 * @param buffer where equality is written
 * @param[out] variable will be positionned to where in buffer the
 *  variable name starts. leading spaces will be converted to \0
 * @param[out] value same as above but for the value part
 * @note variable and value become pointers to internals of
 *  buffer. If buffer chages, they will change too and/or become invalid!
 */
static int equality_split(char *buffer, char **variable, char **value) {
    if (!strcmp(&buffer[strlen(buffer)-strlen("\n")], "\n"))
        buffer[strlen(buffer)-strlen("\n")] = '\0';
    *value = strstr(buffer, "=");
    if (!*value)
        return 0;
    **value = '\0';
    *variable = buffer;
    (*value)++;
    while ((strlen(*variable) > 0) && ((*variable)[strlen(*variable)-1] == ' '))
        (*variable)[strlen(*variable)-1] = '\0';
    while ((strlen(*value) > 0) && ((*value)[strlen(*value)-1] == ' '))
        (*value)[strlen(*value)-1] = '\0';
    while (**value == ' ')
        (*value)++;
    if ((**variable == '\0') || (**value == '\0'))
        return 0;
    return 1;
}

/**
 * This function gets a string containing
 * [Y/y](es)/[N/n](o), 1/0
 * and set bl according to what's read
 * if return value is true, strg was set successfully
 * else, an error occured and bl was not touched
 *
 * @param strg string to process.
 * @param bl value strg meant.
 * @return 1 if strg was processed, 0 else.
 */
static int get_boolean(const char *strg, int *bl) {
    if (!strncmp(strg, "y", 1))
        *bl = 1;
    else if (!strncmp(strg, "n", 1))
        *bl = 0;
    else if (!strncmp(strg, "Y", 1))
        *bl = 1;
    else if (!strncmp(strg, "N", 1))
        *bl = 0;
    else if (!strncmp(strg, "1", 1))
        *bl = 1;
    else if (!strncmp(strg, "0", 1))
        *bl = 0;
    else
        return 0;
    return 1;
}

/**
 * Is specified object currently being animated?
 * @param ob object to search for.
 * @return 1 if ob is part of animation, 0 else.
 */
static int is_animated_object(const object *ob) {
    CFanimation *current;

    for (current = first_animation; current; current = current->nextanimation)
        if (current->victim == ob) {
            return 1;
        }
    return 0;
}

/**
 * Create a new animation.
 * @return new animation pointer inserted in the list of animations.
 */
static CFanimation *create_animation(void) {
    CFanimation *new;
    CFanimation *current;

    new = (CFanimation *)malloc(sizeof(CFanimation));
    if (!new)
        return NULL;
    new->name = NULL;
    new->victim = NULL;
    new->event = NULL;
    new->nextmovement = NULL;
    new->nextanimation = NULL;
    new->delete_end = 0;
    for (current = first_animation; (current && current->nextanimation); current = current->nextanimation)
        ;
    if (!current)
        first_animation = new;
    else
        current->nextanimation = new;
    return new;
}

static object *find_by_name(object *origin, const char *name) {
    int x, y, w, h;
    mapstruct *map;
    const char *sname;

    sname = cf_find_string(name);
    if (!sname)
        return NULL;

    while (origin && !origin->map)
        origin = origin->env;

    if (!origin || !origin->map)
        return NULL;

    map = origin->map;

    w = cf_map_get_width(map);
    h = cf_map_get_height(map);

    for (x = 0; x < w; x++) {
        for (y = 0; y < h; y++) {
            FOR_MAP_PREPARE(map, x, y, ob) {
                if (/*cf_object_get_sstring_property(ob, CFAPI_OBJECT_PROP_NAME)*/ob->name == sname)
                    return ob;
            } FOR_MAP_FINISH();
        }
    }

    return NULL;
}

/**
 * Create a new animation object according to file, option and activator (who)
 *
 * @param who - object that raised the event leading to the plugin.
 * @param activator - object that caused who to get an event.
 * @param event - actual event object linking who and this plugin. Can be removed.
 * @param file - filename to read from, should be accessible from the current path.
 * @param message - if non empty, will be the name of the used animation instead of the one specified in the file.
 * @return 1 - if the animation was created, 0 else.
 * @todo fix memory leaks in case of errors.
 */
static int start_animation(object *who, object *activator, object *event, const char *file, const char *message) {
    FILE    *fichier;
    char    *name = NULL;
    int     victimtype = 0;
    object  *victim = NULL;
    int     unique = 0;
    int     always_delete = 0;
    int     delete_end = 0;
    int     parallel = 0;
    int     paralyzed = 1;
    int     invisible = 0;
    int     wizard = 0;
    enum    time_enum timetype;
    int     errors_allowed = 0;
    int     verbose = 0;
    const char *animationitem = NULL;
    char    buffer[HUGE_BUF];
    char    *variable;
    char    *value;
    int     errors_found = 0;
    CFanimation *current_anim;

    fichier = fopen(file, "r");
    if (fichier == NULL) {
        cf_log(llevDebug, "CFAnim: Unable to open %s\n", file);
        return 0;
    }
    while (fgets(buffer, HUGE_BUF, fichier)) {
        if (buffer[0] == '[')
            break;
        if (buffer[0] == '#')
            continue;
        if (!strcmp(buffer, "\n"))
            continue;
        errors_found = 1;
        cf_log(llevError, "CFAnim: '%s' has an invalid syntax.\n", buffer);
    }
    if (feof(fichier)) {
        fclose(fichier);
        return 0;
    }
    if (strncmp(buffer, "[Config]", 8)) {
        cf_log(llevError, "CFAnim: Fatal error in %s: [Config] must be the first group defined.\n", file);
        fclose(fichier);
        return 0;
    }
    while (fgets(buffer, HUGE_BUF, fichier)) {
        if (buffer[0] == '[')
            break;
        if (buffer[0] == '#')
            continue;
        if (!strcmp(buffer, "\n"))
            continue;
        if (!equality_split(buffer, &variable, &value))
            errors_found = 1;
        else {
            if (!strcmp(variable, "name")) {
                if (*value == '"')
                    value++;
                if (value[strlen(value)-1] == '"')
                    value[strlen(value)-1] = '\0';
                name = cf_strdup_local(value);
            } else if (!strcmp(variable, "victimtype")) {
                if (!strcmp(value, "player"))
                    victimtype = 0;
                else if (!strcmp(value, "object"))
                    victimtype = 1;
                else if (!strcmp(value, "any"))
                    victimtype = 2;
                else if (!strcmp(value, "byname"))
                    victimtype = 3;
                else
                    errors_found = 1;
            } else if (!strcmp(variable, "victim")) {
                cf_log(llevDebug, "CFAnim: Setting victim to %s\n", value);
                if (!strcmp(value, "who"))
                    victim = who;
                else if (!strcmp(value, "activator"))
                    victim = activator;
                else if (!strcmp(value, "who_owner"))
                    if (!who) {
                        errors_found = 1;
                        cf_log(llevError, "CFAnim: Warning: object \"who\" doesn't exist and you're victimized it's owner\n");
                    } else
                        victim = who->env;
                else if (!strcmp(value, "activator_owner"))
                    if (!activator) {
                        errors_found = 1;
                        cf_log(llevError, "CFAnim: Warning: object \"activator\" doesn't exist and you're victimized it's owner\n");
                    } else
                        victim = activator->env;
                else if (victimtype == 3) {
                    victim = find_by_name(who, value);
                } else
                    errors_found = 1;
            } else if (!strcmp(variable, "unique")) {
                if (!get_boolean(value, &unique))
                    errors_found = 1;
            } else if (!strcmp(variable, "always_delete")) {
                if (!get_boolean(value, &always_delete))
                    errors_found = 1;
            } else if (!strcmp(variable, "delete_event_end")) {
                if (!get_boolean(value, &delete_end))
                    errors_found = 1;
            } else if (!strcmp(variable, "parallel")) {
                if (!get_boolean(value, &parallel))
                    errors_found = 1;
            } else if (!strcmp(variable, "paralyzed")) {
                if (!get_boolean(value, &paralyzed))
                    errors_found = 1;
            } else if (!strcmp(variable, "invisible")) {
                if (!get_boolean(value, &invisible))
                    errors_found = 1;
            } else if (!strcmp(variable, "wizard")) {
                if (!get_boolean(value, &wizard))
                    errors_found = 1;
            } else if (!strcmp(variable, "errors_allowed")) {
                if (!get_boolean(value, &errors_allowed))
                    errors_found = 1;
            } else if (!strcmp(variable, "verbose")) {
                if (!get_boolean(value, &verbose))
                    errors_found = 1;
            } else if (!strcmp(variable, "time_representation")) {
                if (!strcmp(value, "second"))
                    timetype = time_second;
                else if (!strcmp(value, "tick"))
                    timetype = time_tick;
                else
                    errors_found = 1;
            } else if (!strcmp(variable, "animation")) {
                animationitem = cf_add_string(value);
            } else
                errors_found = 1;
        }
    }

    if (message && message[0] != '\0') {
        cf_free_string(animationitem);
        animationitem = cf_add_string(message);
    }

    if (buffer[0] == '\0') {
        if (animationitem)
            cf_free_string(animationitem);
        cf_log(llevError, "CFAnim: Errors occurred during the parsing of %s\n", file);
        fclose(fichier);
        return 0;
    }
    if (!animationitem) {
        cf_log(llevError, "CFAnim: no animation specified when using %s\n", file);
        fclose(fichier);
        return 0;
    }
    if (!victim) {
        if (animationitem)
            cf_free_string(animationitem);
        cf_log(llevError,  "CFAnim: Fatal error - victim is NULL");
        fclose(fichier);
        return 0;
    }
    if (!(current_anim = create_animation())) {
        if (animationitem)
            cf_free_string(animationitem);
        cf_log(llevError, "CFAnim: Fatal error - Not enough memory.\n");
        fclose(fichier);
        return 0;
    }
    if (always_delete) {
        /*if (verbose) printf("CFAnim: Freeing event nr. %d for %s.\n", current_event, who->name);*/
        cf_object_remove(event);
        event = NULL;
    }
    if (((victim->type == PLAYER) && (victimtype == 1))
    || ((victim->type != PLAYER) && (victimtype == 0))
    || (errors_found && !errors_allowed)) {
        if (verbose)
            cf_log(llevError, "CFAnim: No correct victim found or errors found, aborting.\n");
        if (animationitem)
            cf_free_string(animationitem);
        free(current_anim);
        fclose(fichier);
        return 0;
    }
    if (unique && !always_delete) {
        /*if (verbose) printf("CFAnim: Freeing event nr. %d for %s.\n", current_event, who->name);*/
        cf_object_remove(event);
        event = NULL;
    }
    current_anim->name = name;
    current_anim->victim = victim;
    current_anim->event = event;
    current_anim->paralyze = paralyzed;
    current_anim->invisible = invisible;
    current_anim->wizard = wizard;
    current_anim->unique = unique;
    current_anim->delete_end = delete_end;
    current_anim->ghosted = 0;
    current_anim->corpse = NULL;
    current_anim->time_representation = timetype;
    current_anim->verbose = verbose;
    current_anim->tick_left = 0;
    current_anim->errors_allowed = errors_allowed;

    while (buffer[0] == '[') {
        while (strncmp(&buffer[1], animationitem, strlen(animationitem))) {
            while ((value = fgets(buffer, HUGE_BUF, fichier)) != NULL)
                if (buffer[0] == '[')
                    break;
            if (value == NULL) {
                cf_log(llevError, "CFAnim: no matching animation %s in file.\n", animationitem);
                cf_free_string(animationitem);
                fclose(fichier);
                return 0;
            }
        }
        current_anim->nextmovement = parse_animation_block(buffer, HUGE_BUF, fichier, current_anim);
        if (current_anim->nextmovement)
            break;
    }
    fclose(fichier);
    return 1;
}

/**
 * Checks if an animation can execute one or more moves, and if so does them.
 * @param animation animation to check
 * @param milliseconds time elapsed since the last time this function was called.
 */
static void animate_one(CFanimation *animation, long int milliseconds) {
    CFmovement *current;
    int mult = 1;
    anim_move_result result;

    if (animation->time_representation == time_second) {
        animation->tick_left += milliseconds;
        mult = 1000;
    } else
        animation->tick_left++;

    if (animation->verbose)
        cf_log(llevDebug, "CFAnim: Ticking %s for %s. Tickleft is %ld\n", animation->name, animation->victim->name, animation->tick_left);
    if (animation->invisible)
        animation->victim->invisible = 10;
    if (animation->wizard && animation->victim->type == PLAYER) {
        /* setting FLAG_WIZ *on non player leads to issues, as many functions expect contr to not be NULL in this case. */
        if (animation->verbose)
            cf_log(llevDebug, "CFAnim: Setting wizard flags\n");
        cf_object_set_flag(animation->victim, FLAG_WIZPASS, 1);
        cf_object_set_flag(animation->victim, FLAG_WIZCAST, 1);
        cf_object_set_flag(animation->victim, FLAG_WIZ, 1);
        if (animation->verbose)
            cf_log(llevDebug, "CFAnim: Setting wizard flags done\n");

    }
    if (animation->paralyze)
        animation->victim->speed_left = -99999;

    cf_object_update(animation->victim, UP_OBJ_CHANGE);

    if (animation->nextmovement)
        while (animation->tick_left > animation->nextmovement->tick*mult) {
            animation->tick_left -= animation->nextmovement->tick*mult;
            result = animation->nextmovement->func(animation, animation->nextmovement->id, animation->nextmovement->parameters);
            if (result == mr_again)
                continue;

            current = animation->nextmovement;
            animation->nextmovement = animation->nextmovement->next;
            free(current);
            if (!animation->nextmovement)
                break;
        }
    cf_object_set_flag(animation->victim, FLAG_WIZPASS, 0);
    cf_object_set_flag(animation->victim, FLAG_WIZCAST, 0);
    cf_object_set_flag(animation->victim, FLAG_WIZ, 0);
}

/**
 * Animates all currently running animations.
 */
static void animate(void) {
    CFanimation *current;
    CFanimation *next;
    CFanimation *previous_anim = NULL;
    struct timeval now;
    static struct timeval yesterday;
    static int already_passed = 0;
    long int delta_milli;

    (void)GETTIMEOFDAY(&now);
    if (!already_passed) {
        already_passed = 1;
        memcpy(&yesterday, &now, sizeof(struct timeval));
        return;
    }
    delta_milli = (now.tv_sec-yesterday.tv_sec)*1000+(now.tv_usec/1000-yesterday.tv_usec/1000);
    /*printf("Working for %ld milli seconds\n", delta_milli);*/
    memcpy(&yesterday, &now, sizeof(struct timeval));
    for (current = first_animation; current; current = current->nextanimation)
        animate_one(current, delta_milli);
    current = first_animation;
    while (current) {
        if (!current->nextmovement) {
            if (current->paralyze)
                current->victim->speed_left = current->victim->speed;
            next = current->nextanimation;
            if (first_animation == current)
                first_animation = next;
            else {
                previous_anim->nextanimation = next;
            }
            if (current->delete_end && current->event != NULL)
                cf_object_remove(current->event);
            free(current->name);
            free(current);
            current = next;
        } else {
            previous_anim = current;
            current = current->nextanimation;
        }
    }
}

/**
 * Plugin initialisation function.
 * @param iversion server version.
 * @param gethooksptr function to get the hooks.
 * @return 0
 */
CF_PLUGIN int initPlugin(const char *iversion, f_plug_api gethooksptr) {
    cf_init_plugin(gethooksptr);
    cf_log(llevDebug, "CFAnim 2.0a init\n");

    /* Place your initialization code here */
    return 0;
}

CF_PLUGIN void *getPluginProperty(int *type, ...) {
    va_list args;
    const char *propname;
    char *buf;
    int size;

    va_start(args, type);
    propname = va_arg(args, const char *);

    if (!strcmp(propname, "Identification")) {
        buf = va_arg(args, char *);
        size = va_arg(args, int);
        va_end(args);
        snprintf(buf, size, PLUGIN_NAME);
        return NULL;
    } else if (!strcmp(propname, "FullName")) {
        buf = va_arg(args, char *);
        size = va_arg(args, int);
        va_end(args);
        snprintf(buf, size, PLUGIN_VERSION);
        return NULL;
    }
    va_end(args);
    return NULL;
}

CF_PLUGIN anim_move_result cfanim_runPluginCommand(object *op, char *params) {
    return -1;
}

CF_PLUGIN int postInitPlugin(void) {
    cf_log(llevDebug, "CFAnim 2.0a post init\n");
    /* Pick the global events you want to monitor from this plugin */
    cf_system_register_global_event(EVENT_CLOCK, PLUGIN_NAME, cfanim_globalEventListener);
    return 0;
}

CF_PLUGIN int cfanim_globalEventListener(int *type, ...) {
    //cf_log(llevDebug, "Entering cfanim_globalEventListener.");
    va_list args;
    int rv = 0;
    int event_code;

    va_start(args, type);
    event_code = va_arg(args, int);
    assert(event_code == EVENT_CLOCK);

    animate();

    va_end(args);

    return rv;
}

CF_PLUGIN int eventListener(int *type, ...) {
    //cf_log(llevDebug, "Entering eventListener.");
    int rv = 0;
    va_list args;
    char *buf, message[MAX_BUF], script[MAX_BUF];
    object *who, *activator/*, *third*/, *event;
    int query;

    va_start(args, type);

    who = va_arg(args, object *);
    activator = va_arg(args, object *);
    /*third =*/ va_arg(args, object *);
    buf = va_arg(args, char *);

    if (buf != NULL)
        strcpy(message, buf);
    else
        message[0] = '\0';

    query = va_arg(args, int); /* 'fix', ignored */
    event = va_arg(args, object *);

    if (query == 1 && strcmp(message, "query_object_is_animated") == 0) {
        rv = is_animated_object(who);
        return rv;
    }

    /** @todo build from current map's path, probably */
    cf_get_maps_directory(event->slaying, script, sizeof(script));
    va_end(args);

    /* Put your plugin action(s) here */
    if (activator != NULL) {
        cf_log(llevDebug, "CFAnim: %s called animator script %s\n", activator->name, script);
    } else if (who != NULL) {
        activator = who;
        cf_log(llevDebug, "CFAnim: %s called animator script %s\n", who->name, script);
    }

    rv = start_animation(who, activator, event, script, message);

    return rv;
}

CF_PLUGIN int   closePlugin(void) {
    cf_log(llevDebug, "CFAnim 2.0a closing\n");
    return 0;
}
