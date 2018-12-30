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
 * @file anim.c
 * This file contains animation-related code.
 **/

#include <global.h>
#include <stdio.h>
#include <assert.h>

/**
 * Clears all animation-related memory.
 **/
void free_all_anim(void) {
    int i;

    for (i = 0; i <= num_animations; i++) {
        free_string(animations[i].name);
        free(animations[i].faces);
    }
    free(animations);
}

/**
 * Loads the lib/animations file.
 * Can be called multiple times without ill effects.
 **/
void init_anim(void) {
    char buf[MAX_BUF];
    FILE *fp;
    int num_frames = 0, i;
    const New_Face *faces[MAX_ANIMATIONS];

    animations_allocated = 9;
    num_animations = 0;
    /* Make a default.  New animations start at one, so if something
     * thinks it is animated but hasn't set the animation_id properly,
     * it will have a default value that should be pretty obvious.
     */
    animations = malloc(10*sizeof(Animations));
    /* set the name so we don't try to dereferance null.
     * Put # at start so it will be first in alphabetical
     * order.
     */
    animations[0].name = add_string("###none");
    animations[0].num_animations = 1;
    animations[0].faces = malloc(sizeof(New_Face*));
    animations[0].faces[0] = &new_faces[find_face("bug.111", 0)];
    animations[0].facings = 0;

    snprintf(buf, sizeof(buf), "%s/animations", settings.datadir);
    LOG(llevDebug, "Reading animations from %s...\n", buf);
    if ((fp = fopen(buf, "r")) == NULL) {
        LOG(llevError, "Cannot open animations file %s: %s\n", buf, strerror_local(errno, buf, sizeof(buf)));
        exit(-1);
    }
    while (fgets(buf, MAX_BUF-1, fp) != NULL) {
        if (*buf == '#')
            continue;
        if (strlen(buf) == 0)
            break;
        /* Kill the newline */
        buf[strlen(buf)-1] = '\0';
        if (!strncmp(buf, "anim ", 5)) {
            if (num_frames) {
                LOG(llevError, "Didn't get a mina before %s\n", buf);
                num_frames = 0;
            }
            num_animations++;
            if (num_animations == animations_allocated) {
                animations = realloc(animations, sizeof(Animations)*(animations_allocated+10));
                animations_allocated += 10;
            }
            animations[num_animations].name = add_string(buf+5);
            animations[num_animations].num = num_animations; /* for bsearch */
            animations[num_animations].facings = 1;
        } else if (!strncmp(buf, "mina", 4)) {
            assert(num_frames > 0);
            animations[num_animations].faces = malloc(sizeof(New_Face*)*num_frames);
            for (i = 0; i < num_frames; i++)
                animations[num_animations].faces[i] = faces[i];
            animations[num_animations].num_animations = num_frames;
            if (num_frames <= 1) {
                LOG(llevDebug, "Animation %s has only one or none face!\n", animations[num_animations].name);
            }
            if (num_frames%animations[num_animations].facings) {
                LOG(llevDebug, "Animation %s frame numbers (%d) is not a multiple of facings (%d)\n",
                    animations[num_animations].name, num_frames, animations[num_animations].facings);
            }
            num_frames = 0;
        } else if (!strncmp(buf, "facings", 7)) {
            if (!(animations[num_animations].facings = atoi(buf+7))) {
                LOG(llevDebug, "Animation %s has 0 facings, line=%s\n",
                    animations[num_animations].name, buf);
                animations[num_animations].facings = 1;
            }
        } else {
            unsigned face = find_face(buf, (unsigned)-1);
            if (face == (unsigned)-1) {
                LOG(llevError, "Could not find face %s for animation %s\n",
                    buf, animations[num_animations].name);
                face = find_face("bug.111", 0);
            }
            faces[num_frames++] = &new_faces[face];
        }
    }
    fclose(fp);
    LOG(llevDebug, "done. got (%d)\n", num_animations);
}

/**
 * Utility function to compare 2 animations based on their name.
 * Used for sorting/searching.
 */
static int anim_compare(const Animations *a, const Animations *b) {
    return strcmp(a->name, b->name);
}

/**
 * Finds the animation id that matches name. Will LOG() an error if not found.
 * @param name
 * the animation's name.
 * @return
 * animation number, or 0 if no match found (animation 0 is initialized as the 'bug' face).
 * @see try_find_animation
 */
int find_animation(const char *name) {
    int face = try_find_animation(name);
    if (!face)
        LOG(llevError, "Unable to find animation %s\n", name);
    return face;
}

/**
 * Tries to find the animation id that matches name, don't LOG() an error if not found.
 * @param name
 * the animation's name.
 * @return
 * animation number, or 0 if no match found (animation 0 is initialized as the 'bug' face).
 * @see find_animation
 */
int try_find_animation(const char *name) {
    Animations search, *match;

    search.name = name;

    match = (Animations *)bsearch(&search, animations, (num_animations+1), sizeof(Animations), (int (*)(const void *, const void *))anim_compare);


    if (match)
        return match->num;
    return 0;
}

/**
 * Updates the face-variable of an object.
 * If the object is the head of a multi-object, all objects are animated.
 * The object's state is not changed, but merely updated if needed (out of bounds of
 * new animation, reached end of animation, ...).
 * This function correctly handles FLAG_IS_TURNABLE too.
 *
 * @param op is the object to animate.
 * @param dir is the direction the object is facing.  This is generally same as
 *    op->direction, but in some cases, op->facing is used instead - the
 *    caller has a better idea which one it really wants to be using,
 *    so let it pass along the right one. This parameter is ignored if
 *    the object has a head or is turnable.
 */
void animate_object(object *op, int dir) {
    int max_state;  /* Max animation state object should be drawn in */
    int base_state; /* starting index # to draw from */
    int oldface = op->face->number;

    if (!op->animation_id || !NUM_ANIMATIONS(op)) {
        StringBuffer *sb;
        char *diff;

        LOG(llevError, "Object lacks animation.\n");
        sb = stringbuffer_new();
        object_dump(op, sb);
        diff = stringbuffer_finish(sb);
        LOG(llevError, "%s", diff);
        free(diff);
        return;
    }

    if (op->head) {
        dir = op->head->direction;

        if (NUM_ANIMATIONS(op) == NUM_ANIMATIONS(op->head))
            op->state = op->head->state;
    } else if (QUERY_FLAG(op, FLAG_IS_TURNABLE)) {
        dir = op->direction;
    }

    /* If object is turning, then max animation state is half through the
     * animations.  Otherwise, we can use all the animations.
     */
    max_state = NUM_ANIMATIONS(op)/NUM_FACINGS(op);
    base_state = 0;
    /* at least in the older aniamtions that used is_turning, the first half
     * of the animations were left facing, the second half right facing.
     * Note in old the is_turning, it was set so that the animation for a monster
     * was always towards the enemy - now it is whatever direction the monster
     * is facing.
     */
    if (NUM_FACINGS(op) == 2) {
        if (dir < 5)
            base_state = 0;
        else
            base_state = NUM_ANIMATIONS(op)/2;
    } else if (NUM_FACINGS(op) == 4) {
        if (dir == 0)
            base_state = 0;
        else
            base_state = ((dir-1)/2)*(NUM_ANIMATIONS(op)/4);
    } else if (NUM_FACINGS(op) == 8) {
        if (dir == 0)
            base_state = 0;
        else
            base_state = (dir-1)*(NUM_ANIMATIONS(op)/8);
    } else if (QUERY_FLAG(op, FLAG_IS_TURNABLE)) {
        base_state = (NUM_ANIMATIONS(op) / 9) * (dir);
        max_state = NUM_ANIMATIONS(op) / 9;
    }

    /* If beyond drawable states, reset */
    if (op->state >= max_state) {
        op->state = 0;
        if (op->temp_animation_id) {
            op->temp_animation_id = 0;
            animate_object(op, dir);
            return;
        }
    }
    SET_ANIMATION(op, op->state+base_state);

    if (op->face == blank_face)
        op->invisible = 1;

    /* This block covers monsters (eg, pixies) which are supposed to
     * cycle from visible to invisible and back to being visible.
     * as such, disable it for players, as then players would become
     * visible.
     */
    else if (op->type != PLAYER && QUERY_FLAG((&op->arch->clone), FLAG_ALIVE)) {
        if (op->face->number == 0) {
            op->invisible = 1;
            CLEAR_FLAG(op, FLAG_ALIVE);
        } else {
            op->invisible = 0;
            SET_FLAG(op, FLAG_ALIVE);
        }
    }

    if (op->more)
        animate_object(op->more, dir);

    /* object_update() will also recursively update all the pieces.
     * as such, we call it last, and only call it for the head
     * piece, and not for the other tail pieces.
     */
    if (!op->head && (oldface != op->face->number))
        object_update(op, UP_OBJ_FACE);
}

/**
 * Applies a compound animation to an object.
 *
 * @param who
 * object to apply the animation to. Must not be NULL.
 * @param suffix
 * animation suffix to apply. Must not be NULL.
 */
void apply_anim_suffix(object *who, sstring suffix) {
    int anim;
    object *head, *orig;
    char buf[MAX_BUF];

    assert(who);
    assert(suffix);

    if (who->temp_animation_id != 0)
        /* don't overlap animation, let the current one finish. */
        return;

    head = HEAD(who);
    orig = head;
    snprintf(buf, MAX_BUF, "%s_%s", animations[head->animation_id].name, suffix);
    anim = try_find_animation(buf);
    if (anim) {
        for (; head != NULL; head = head->more) {
            head->temp_animation_id = anim;
            head->temp_anim_speed = animations[anim].num_animations/animations[anim].facings;
            head->last_anim = 0;
            head->state = 0;
        }
        animate_object(orig, orig->facing);
    }
}

/**
 * Dump all animations to stderr, for debugging purposes.
 */
void dump_animations(void) {
    int a;
    fprintf(stderr, "id    name                                               faces\n");
    for (a = 0; a < num_animations; a++) {
        fprintf(stderr, "%5d %50s %5d\n", animations[a].num, animations[a].name, animations[a].num_animations);
    }
}
