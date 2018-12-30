/**
 * @file
 * Face-related structures.
 */

#ifndef FACE_H
#define FACE_H

/**
 * New face structure - this enforces the notion that data is face by
 * face only - you can not change the color of an item - you need to instead
 * create a new face with that color.
 */
typedef struct new_face_struct {
    uint16 number;           /**< This is the image id.  It should be the
                              * same value as its position in the array */
    uint8 visibility;
    uint8 magicmap;          /**< Color to show this in magic map */
    uint16 smoothface;       /**< Smoothed face for this, (uint16)-1 for none. */
    const char *name;        /**< Face name, as used by archetypes and such. */
} New_Face;

/**
 * This represents one animation. It points to different faces.
 */
typedef struct animations_struct {
    const char *name;       /**< Name of the animation sequence. */
    uint8 num_animations;   /**< How many different faces to animate, size of the faces array. */
    uint8 facings;          /**< How many facings (1,2,4,8). */
    uint16 num;             /**< Where we are in the array. */
    const New_Face **faces; /**< The actual faces for the animation. */
} Animations;

#endif /* FACE_H */
