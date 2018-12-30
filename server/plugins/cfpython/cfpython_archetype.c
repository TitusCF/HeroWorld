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

static PyObject *Crossfire_Archetype_GetName(Crossfire_Archetype *whoptr, void *closure) {
    return Py_BuildValue("s", cf_archetype_get_name(whoptr->arch));
}

static PyObject *Crossfire_Archetype_GetNext(Crossfire_Archetype *who, void *closure) {
    return Crossfire_Archetype_wrap(cf_archetype_get_next(who->arch));
}

static PyObject *Crossfire_Archetype_GetMore(Crossfire_Archetype *who, void *closure) {
    return Crossfire_Archetype_wrap(cf_archetype_get_more(who->arch));
}

static PyObject *Crossfire_Archetype_GetHead(Crossfire_Archetype *who, void *closure) {
    return Crossfire_Archetype_wrap(cf_archetype_get_head(who->arch));
}

static PyObject *Crossfire_Archetype_GetClone(Crossfire_Archetype *who, void *closure) {
    return Crossfire_Object_wrap(cf_archetype_get_clone(who->arch));
}

static PyObject *Crossfire_Archetype_GetNewObject(Crossfire_Archetype *who, PyObject *args) {
    return Crossfire_Object_wrap(cf_create_object_by_name(cf_archetype_get_name(who->arch)));
}

PyObject *Crossfire_Archetype_wrap(archetype *what) {
    Crossfire_Archetype *wrapper;

    /* return None if no object was to be wrapped */
    if (what == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    wrapper = PyObject_NEW(Crossfire_Archetype, &Crossfire_ArchetypeType);
    if (wrapper != NULL)
        wrapper->arch = what;
    return (PyObject *)wrapper;
}

static int Crossfire_Archetype_InternalCompare(Crossfire_Archetype *left, Crossfire_Archetype *right) {
    return (left->arch < right->arch ? -1 : (left->arch == right->arch ? 0 : 1));
}

static PyObject *Crossfire_Archetype_RichCompare(Crossfire_Archetype *left, Crossfire_Archetype *right, int op) {
    int result;
    if (!left
        || !right
        || !PyObject_TypeCheck((PyObject*)left, &Crossfire_ArchetypeType)
        || !PyObject_TypeCheck((PyObject*)right, &Crossfire_ArchetypeType)) {
        Py_INCREF(Py_NotImplemented);
        return Py_NotImplemented;
    }
    result = Crossfire_Archetype_InternalCompare(left, right);
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

static PyGetSetDef Archetype_getseters[] = {
    { "Name",       (getter)Crossfire_Archetype_GetName,     NULL, NULL, NULL },
    { "Next",       (getter)Crossfire_Archetype_GetNext,     NULL, NULL, NULL },
    { "More",       (getter)Crossfire_Archetype_GetMore,     NULL, NULL, NULL },
    { "Head",       (getter)Crossfire_Archetype_GetHead,     NULL, NULL, NULL },
    { "Clone",      (getter)Crossfire_Archetype_GetClone,    NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL }
};

static PyMethodDef ArchetypeMethods[] = {
    { "NewObject", (PyCFunction)Crossfire_Archetype_GetNewObject, METH_NOARGS, NULL },
    { NULL, NULL, 0, NULL }
};

/* Our actual Python ArchetypeType */
PyTypeObject Crossfire_ArchetypeType = {
#ifdef IS_PY3K
    /* See http://bugs.python.org/issue4385 */
    PyVarObject_HEAD_INIT(NULL, 0)
#else
    PyObject_HEAD_INIT(NULL)
    0,                         /* ob_size*/
#endif
    "Crossfire.Archetype",     /* tp_name*/
    sizeof(Crossfire_Archetype), /* tp_basicsize*/
    0,                         /* tp_itemsize*/
    NULL,                      /* tp_dealloc*/
    NULL,                      /* tp_print*/
    NULL,                      /* tp_getattr*/
    NULL,                      /* tp_setattr*/
#ifdef IS_PY3K
    NULL,                      /* tp_reserved */
#else
    (cmpfunc)Crossfire_Archetype_InternalCompare, /* tp_compare*/
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
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags*/
    "Crossfire archetypes",    /* tp_doc */
    NULL,                      /* tp_traverse */
    NULL,                      /* tp_clear */
    (richcmpfunc)Crossfire_Archetype_RichCompare, /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    NULL,                      /* tp_iter */
    NULL,                      /* tp_iternext */
    ArchetypeMethods,          /* tp_methods */
    NULL,                      /* tp_members */
    Archetype_getseters,       /* tp_getset */
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
