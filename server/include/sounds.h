/**
 * @file
 * Sound-related defines.
 */

#ifndef SOUNDS_H
#define SOUNDS_H

/**
 * @defgroup Soundtypes Sound types
 */
/*@{*/
#define SOUND_TYPE_LIVING       1
#define SOUND_TYPE_SPELL        2
#define SOUND_TYPE_ITEM         3
#define SOUND_TYPE_GROUND       4
#define SOUND_TYPE_HIT          5
#define SOUND_TYPE_HIT_BY       6
/*@}*/

/**
 * Those flags are for the 'socket.sound' field.
 */
#define SND_EFFECTS     1       /**< Client wands regular sounds. */
#define SND_MUSIC       2       /**< Client wants background music info. */
#define SND_MUTE        64      /**< Don't sent anything for now. */

#define MAX_SOUNDS_TICK     3 /**< Maximum number of sounds a player can receive for each tick. */

#endif /* SOUNDS_H */
