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

#include <cfpython.h>

static PyObject *Crossfire_Region_GetName(Crossfire_Region *regionptr, void *closure) {
    return Py_BuildValue("s", cf_region_get_name(regionptr->reg));
}

static PyObject *Crossfire_Region_GetLongname(Crossfire_Region *regionptr, void *closure) {
    return Py_BuildValue("s", cf_region_get_longname(regionptr->reg));
}

static PyObject *Crossfire_Region_GetMessage(Crossfire_Region *regionptr, void *closure) {
    return Py_BuildValue("s", cf_region_get_message(regionptr->reg));
}

static PyObject *Crossfire_Region_GetNext(Crossfire_Region *party, void *closure) {
    return Crossfire_Region_wrap(cf_region_get_next(party->reg));
}

static PyObject *Crossfire_Region_GetParent(Crossfire_Region *party, PyObject *args) {
    return Crossfire_Region_wrap(cf_region_get_parent(party->reg));
}

static PyObject *Crossfire_Region_GetJailX(Crossfire_Region *regionptr, void *closure) {
    return Py_BuildValue("i", cf_region_get_jail_x(regionptr->reg));
}

static PyObject *Crossfire_Region_GetJailY(Crossfire_Region *regionptr, void *closure) {
    return Py_BuildValue("i", cf_region_get_jail_y(regionptr->reg));
}

static PyObject *Crossfire_Region_GetJailPath(Crossfire_Region *regionptr, void *closure) {
    return Py_BuildValue("s", cf_region_get_jail_path(regionptr->reg));
}

PyObject *Crossfire_Region_wrap(region *what) {
    Crossfire_Region *wrapper;

    /* return None if no object was to be wrapped */
    if (what == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    wrapper = PyObject_NEW(Crossfire_Region, &Crossfire_RegionType);
    if (wrapper != NULL)
        wrapper->reg = what;
    return (PyObject *)wrapper;
}

static int Crossfire_Region_InternalCompare(Crossfire_Region *left, Crossfire_Region *right) {
    return (left->reg < right->reg ? -1 : (left->reg == right->reg ? 0 : 1));
}

static PyObject *Crossfire_Region_RichCompare(Crossfire_Region *left, Crossfire_Region *right, int op) {
    int result;
    if (!left
        || !right
        || !PyObject_TypeCheck((PyObject*)left, &Crossfire_RegionType)
        || !PyObject_TypeCheck((PyObject*)right, &Crossfire_RegionType)) {
        Py_INCREF(Py_NotImplemented);
        return Py_NotImplemented;
    }
    result = Crossfire_Region_InternalCompare(left, right);
    /* Based on how Python 3.0 (GPL compatible) implements it for internal types: */
    switch (op) {
        case Py_EQ:
            result = (result == 0);
            break;
        case Py_NE:
            result = (result != 0);
            break;
        case Py_LE:
            result = (result <= 0);
            break;
        case Py_GE:
            result = (result >= 0);
            break;
        case Py_LT:
            result = (result == -1);
            break;
        case Py_GT:
            result = (result == 1);
            break;
    }
    return PyBool_FromLong(result);
}

static PyGetSetDef Region_getseters[] = {
    { "Name",       (getter)Crossfire_Region_GetName,     NULL, NULL, NULL },
    { "Longname",   (getter)Crossfire_Region_GetLongname, NULL, NULL, NULL },
    { "Message",    (getter)Crossfire_Region_GetMessage,  NULL, NULL, NULL },
    { "Next",       (getter)Crossfire_Region_GetNext,     NULL, NULL, NULL },
    { "JailX",      (getter)Crossfire_Region_GetJailX,    NULL, NULL, NULL },
    { "JailY",      (getter)Crossfire_Region_GetJailY,    NULL, NULL, NULL },
    { "JailPath",   (getter)Crossfire_Region_GetJailPath, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL }
};

static PyMethodDef RegionMethods[] = {
    { "GetParent", (PyCFunction)Crossfire_Region_GetParent, METH_NOARGS, NULL },
    { NULL, NULL, 0, NULL }
};

/* Our actual Python ArchetypeType */
PyTypeObject Crossfire_RegionType = {
#ifdef IS_PY3K
    /* See http://bugs.python.org/issue4385 */
    PyVarObject_HEAD_INIT(NULL, 0)
#else
    PyObject_HEAD_INIT(NULL)
    0,                         /* ob_size*/
#endif
    "Crossfire.Region",        /* tp_name*/
    sizeof(Crossfire_Region),  /* tp_basicsize*/
    0,                         /* tp_itemsize*/
    NULL,                      /* tp_dealloc*/
    NULL,                      /* tp_print*/
    NULL,                      /* tp_getattr*/
    NULL,                      /* tp_setattr*/
#ifdef IS_PY3K
    NULL,                      /* tp_reserved */
#else
    (cmpfunc)Crossfire_Region_InternalCompare, /* tp_compare*/
#endif
    NULL,                      /* tp_repr*/
    NULL,                      /* tp_as_number*/
    NULL,                      /* tp_as_sequence*/
    NULL,                      /* tp_as_mapping*/
    PyObject_HashNotImplemented, /* tp_hash */
    NULL,                      /* tp_call*/
    NULL,                      /* tp_str*/
    PyObject_GenericGetAttr,   /* tp_getattro*/
    PyObject_GenericSetAttr,   /* tp_setattro*/
    NULL,                      /* tp_as_buffer*/
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,        /* tp_flags*/
    "Crossfire regions",       /* tp_doc */
    NULL,                      /* tp_traverse */
    NULL,                      /* tp_clear */
    (richcmpfunc)Crossfire_Region_RichCompare, /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    NULL,                      /* tp_iter */
    NULL,                      /* tp_iternext */
    RegionMethods,             /* tp_methods */
    NULL,                      /* tp_members */
    Region_getseters,          /* tp_getset */
    NULL,                      /* tp_base */
    NULL,                      /* tp_dict */
    NULL,                      /* tp_descr_get */
    NULL,                      /* tp_descr_set */
    0,                         /* tp_dictoffset */
    NULL,                      /* tp_init */
    NULL,                      /* tp_alloc */
    NULL,                      /* tp_new */
    NULL,                      /* tp_free */
    NULL,                      /* tp_is_gc */
    NULL,                      /* tp_bases */
    NULL,                      /* tp_mro */
    NULL,                      /* tp_cache */
    NULL,                      /* tp_subclasses */
    NULL,                      /* tp_weaklist */
    NULL,                      /* tp_del */
};
