/**
 * @file
 * Path name definition.
 * @todo
 * only used in the documentation, the variable is also defined in init.c. So fix documentation and trash.
 */

#ifndef SPELLIST_H
#define SPELLIST_H

#include "spells.h"

const char *spellpathnames[NRSPELLPATHS] = {
    "Protection",
    "Fire",
    "Frost",
    "Electricity",
    "Missiles",
    "Self",
    "Summoning",
    "Abjuration",
    "Restoration",
    "Detonation",
    "Mind",
    "Creation",
    "Teleportation",
    "Information",
    "Transmutation",
    "Transferrence",
    "Turning",
    "Wounding",
    "Death",
    "Light"
};

#endif /* SPELLIST_H */
