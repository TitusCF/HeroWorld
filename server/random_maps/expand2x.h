/**
 * @file random_maps/expand2x.h
 * Expands a layout by 2x in each dimension.
 */

#ifndef _EXPAND2X_H
#define _EXPAND2X_H

/* Expands a layout by 2x in each dimension.
 * Resulting layout is actually (2*xsize-1)x(2*ysize-1). (Because of the cheesy
 * algorithm, but hey, it works).
 *
 * Don't forget to free the old layout after this is called (it does not
 * presume to do so itself).
 */
char **expand2x(char **layout, int xsize, int ysize);

#endif
