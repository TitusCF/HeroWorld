/*****************************************************************************/
/* CFPython - A Python module for Crossfire RPG.                             */
/* Version: 2.0beta8 (also known as "Alexander")                             */
/* Contact: yann.chachkoff@myrealbox.com                                     */
/*****************************************************************************/
/* That code is placed under the GNU General Public Licence (GPL)            */
/* (C)2001-2005 by Chachkoff Yann (Feel free to deliver your complaints)     */
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
#ifndef CFPYTHON_MAP_H
#define CFPYTHON_MAP_H

typedef struct {
    PyObject_HEAD
    mapstruct *map;
    int valid;
} Crossfire_Map;

extern PyTypeObject Crossfire_MapType;

#define MAPEXISTCHECK(map) { \
    if (!(map) || ((map)->valid == 0)) { \
        PyErr_SetString(PyExc_ReferenceError, "Crossfire map no longer exists"); \
        return NULL; \
    } \
}

#define MAPEXISTCHECK_INT(map) { \
    if (!(map) || ((map)->valid == 0)) { \
        PyErr_SetString(PyExc_ReferenceError, "Crossfire map no longer exists"); \
        return -1; \
    } \
}

extern PyObject *Crossfire_Map_wrap(mapstruct *what);

#endif /* CFPYTHON_MAP_H */
