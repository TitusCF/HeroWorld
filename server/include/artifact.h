/**
 * @file
 * Artifact-related structures.
 *
 * See the @ref page_artifact "page on artifacts" for more information.
 */

#ifndef ARTIFACT_H
#define ARTIFACT_H

/**
 * This is one artifact, ie one special item.
 */
typedef struct artifactstruct {
    object *item;                   /**< Special values of the artifact. Note that this object is malloc() ed. */
    uint16 chance;                  /**< Chance of the artifact to happen. */
    uint8 difficulty;               /**< Minimum map difficulty for the artifact to happen. */
    struct artifactstruct *next;    /**< Next artifact in the list. */
    linked_char *allowed;           /**< List of archetypes the artifact can affect. */
    int allowed_size;               /**< Length of allowed, for faster computation. */
} artifact;

/**
 * This represents all archetypes for one particular object type.
 */
typedef struct artifactliststruct {
    uint8 type;                         /**< Object type that this list represents. */
    uint16 total_chance;                /**< Sum of chance for are artifacts on this list. */
    struct artifactliststruct *next;    /**< Next list of artifacts. */
    struct artifactstruct *items;       /**< Artifacts for this type. Will never be NULL. */
} artifactlist;

#endif /* ARTIFACT_H */
