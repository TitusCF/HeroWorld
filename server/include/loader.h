/**
 * @file
 * Defines for loader.l / loader.c.
 */

#ifndef LOADER_H
#define LOADER_H

#define LL_IGNORED  -1
#define LL_EOF      0
#define LL_MORE     1
#define LL_NORMAL   2

/* see loader.l for more details on this */
#define LO_REPEAT   0
#define LO_LINEMODE 1
#define LO_NEWFILE  2
#define LO_NOREAD   3

#endif /* LOADER_H */
