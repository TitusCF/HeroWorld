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
#include <hashtable.h>

/* Table for keeping track of which PyObject goes with with Crossfire object */
static ptr_assoc_table map_assoc_table;

/* Helper functions for dealing with object_assoc_table */
void init_map_assoc_table(void) {
    init_ptr_assoc_table(map_assoc_table);
}

static void add_map_assoc(mapstruct *key, Crossfire_Map *value) {
    add_ptr_assoc(map_assoc_table, key, value);
}

static PyObject *find_assoc_pymap(mapstruct *key) {
    return (PyObject *)find_assoc_value(map_assoc_table, key);
}

static void free_map_assoc(mapstruct *key) {
    free_ptr_assoc(map_assoc_table, key);
}


/** This makes sure the map is in memory and not swapped out. */
static void ensure_map_in_memory(Crossfire_Map *map) {
    assert(map->map != NULL);
    if (map->map->in_memory != MAP_IN_MEMORY) {
        char* mapname = map->map->path;
        int is_unique = cf_map_get_int_property(map->map, CFAPI_MAP_PROP_UNIQUE);
        /* If the map is unique the path name will be freed. We need to handle that. */
        if (is_unique) {
            char* tmp = strdup(mapname);
            if (!tmp) {
                /* FIXME: We should fatal() here, but that doesn't exist in plugins. */
                cf_log(llevError, "Out of memory in ensure_map_in_memory()!\n");
                abort();
            }
            mapname = tmp;
        }
        cf_log(llevDebug, "MAP %s AIN'T READY ! Loading it...\n", mapname);
        /* Map pointer may change for player unique maps. */
        /* Also, is the MAP_PLAYER_UNIQUE logic correct? */
        map->map = cf_map_get_map(mapname, is_unique ? MAP_PLAYER_UNIQUE : 0);
        if (is_unique)
            free(mapname);
    }
}

static PyObject *Map_GetDifficulty(Crossfire_Map *whoptr, void *closure) {
    MAPEXISTCHECK(whoptr);
    return Py_BuildValue("i", cf_map_get_difficulty(whoptr->map));
}

static PyObject *Map_GetPath(Crossfire_Map *whoptr, void *closure) {
    MAPEXISTCHECK(whoptr);
    return Py_BuildValue("s", cf_map_get_sstring_property(whoptr->map, CFAPI_MAP_PROP_PATH));
}

static PyObject *Map_GetTempName(Crossfire_Map *whoptr, void *closure) {
    MAPEXISTCHECK(whoptr);
    return Py_BuildValue("s", cf_map_get_sstring_property(whoptr->map, CFAPI_MAP_PROP_TMPNAME));
}

static PyObject *Map_GetName(Crossfire_Map *whoptr, void *closure) {
    MAPEXISTCHECK(whoptr);
    return Py_BuildValue("s", cf_map_get_sstring_property(whoptr->map, CFAPI_MAP_PROP_NAME));
}

static PyObject *Map_GetResetTime(Crossfire_Map *whoptr, void *closure) {
    MAPEXISTCHECK(whoptr);
    return Py_BuildValue("i", cf_map_get_reset_time(whoptr->map));
}

static PyObject *Map_GetResetTimeout(Crossfire_Map *whoptr, void *closure) {
    MAPEXISTCHECK(whoptr);
    return Py_BuildValue("i", cf_map_get_reset_timeout(whoptr->map));
}

static PyObject *Map_GetPlayers(Crossfire_Map *whoptr, void *closure) {
    MAPEXISTCHECK(whoptr);
    return Py_BuildValue("i", cf_map_get_players(whoptr->map));
}

static PyObject *Map_GetDarkness(Crossfire_Map *whoptr, void *closure) {
    MAPEXISTCHECK(whoptr);
    return Py_BuildValue("i", cf_map_get_darkness(whoptr->map));
}

static PyObject *Map_GetWidth(Crossfire_Map *whoptr, void *closure) {
    MAPEXISTCHECK(whoptr);
    return Py_BuildValue("i", cf_map_get_width(whoptr->map));
}

static PyObject *Map_GetHeight(Crossfire_Map *whoptr, void *closure) {
    MAPEXISTCHECK(whoptr);
    return Py_BuildValue("i", cf_map_get_height(whoptr->map));
}

static PyObject *Map_GetEnterX(Crossfire_Map *whoptr, void *closure) {
    MAPEXISTCHECK(whoptr);
    return Py_BuildValue("i", cf_map_get_int_property(whoptr->map, CFAPI_MAP_PROP_ENTER_X));
}

static PyObject *Map_GetEnterY(Crossfire_Map *whoptr, void *closure) {
    MAPEXISTCHECK(whoptr);
    return Py_BuildValue("i", cf_map_get_enter_x(whoptr->map));
}

static PyObject *Map_GetMessage(Crossfire_Map *whoptr, void *closure) {
    MAPEXISTCHECK(whoptr);
    return Py_BuildValue("s", cf_map_get_sstring_property(whoptr->map, CFAPI_MAP_PROP_MESSAGE));
}

static PyObject *Map_GetRegion(Crossfire_Map *whoptr, void *closure) {
    MAPEXISTCHECK(whoptr);
    return Crossfire_Region_wrap(cf_map_get_region_property(whoptr->map, CFAPI_MAP_PROP_REGION));
}

static int Map_SetPath(Crossfire_Map *whoptr, PyObject *value, void *closure) {
    const char *val;

    MAPEXISTCHECK_INT(whoptr);
    if (!PyArg_Parse(value, "s", &val))
        return -1;

    cf_map_set_string_property(whoptr->map, CFAPI_MAP_PROP_PATH, val);
    return 0;

}

static PyObject *Map_GetUnique(Crossfire_Map *whoptr, void *closure) {
    MAPEXISTCHECK(whoptr);
    return Py_BuildValue("i", cf_map_get_int_property(whoptr->map, CFAPI_MAP_PROP_UNIQUE));
}

static PyObject *Map_Message(Crossfire_Map *map, PyObject *args) {
    int color = NDI_BLUE|NDI_UNIQUE;
    char *message;

    if (!PyArg_ParseTuple(args, "s|i", &message, &color))
        return NULL;

    MAPEXISTCHECK(map);

    cf_map_message(map->map, message, color);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *Map_GetFirstObjectAt(Crossfire_Map *map, PyObject *args) {
    int x, y;
    object *val;

    if (!PyArg_ParseTuple(args, "ii", &x, &y))
        return NULL;

    MAPEXISTCHECK(map);

    /* make sure the map is swapped in */
    ensure_map_in_memory(map);

    val = cf_map_get_object_at(map->map, x, y);
    return Crossfire_Object_wrap(val);
}

static PyObject *Map_CreateObject(Crossfire_Map *map, PyObject *args) {
    char *txt;
    int x, y;
    object *op;

    if (!PyArg_ParseTuple(args, "sii", &txt, &x, &y))
        return NULL;

    MAPEXISTCHECK(map);

    /* make sure the map is swapped in */
    ensure_map_in_memory(map);

    op = cf_create_object_by_name(txt);

    if (op)
        op = cf_map_insert_object(map->map, op, x, y);
    return Crossfire_Object_wrap(op);
}

static PyObject *Map_Check(Crossfire_Map *map, PyObject *args) {
    char *what;
    int x, y;
    object *foundob;
    sint16 nx, ny;
    int mflags;

    if (!PyArg_ParseTuple(args, "s(ii)", &what, &x, &y))
        return NULL;

    MAPEXISTCHECK(map);

    /* make sure the map is swapped in */
    ensure_map_in_memory(map);

    mflags = cf_map_get_flags(map->map, &(map->map), (sint16)x, (sint16)y, &nx, &ny);
    if (mflags&P_OUT_OF_MAP) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    foundob = cf_map_find_by_archetype_name(what, map->map, nx, ny);
    return Crossfire_Object_wrap(foundob);
}

static PyObject *Map_Next(Crossfire_Map *map, PyObject *args) {
    MAPEXISTCHECK(map);
    return Crossfire_Map_wrap(cf_map_get_map_property(map->map, CFAPI_MAP_PROP_NEXT));
}

static PyObject *Map_Insert(Crossfire_Map *map, PyObject *args) {
    int x, y;
    Crossfire_Object *what;

    if (!PyArg_ParseTuple(args, "O!ii", &Crossfire_ObjectType, &what, &x, &y))
        return NULL;

    MAPEXISTCHECK(map);

    /* make sure the map is swapped in */
    ensure_map_in_memory(map);

    return Crossfire_Object_wrap(cf_map_insert_object(map->map, what->obj, x, y));
}

static PyObject *Map_InsertAround(Crossfire_Map *map, PyObject *args) {
    int x, y;
    Crossfire_Object *what;

    if (!PyArg_ParseTuple(args, "O!ii", &Crossfire_ObjectType, &what, &x, &y))
        return NULL;

    MAPEXISTCHECK(map);

    /* make sure the map is swapped in */
    ensure_map_in_memory(map);

    return Crossfire_Object_wrap(cf_map_insert_object_around(map->map, what->obj, x, y));
}

static PyObject *Map_ChangeLight(Crossfire_Map *map, PyObject *args) {
    int change;

    if (!PyArg_ParseTuple(args, "i", &change))
        return NULL;

    MAPEXISTCHECK(map);

    return Py_BuildValue("i", cf_map_change_light(map->map, change));
}
/**
 * Python backend method for Map.TriggerConnected(int connected, CfObject cause, int state)
 *
 * Expected arguments:
 * - connected will be used to locate Objectlink with given id on map
 * - state: 0=trigger the "release", other is trigger the "push", default is push
 * - cause, eventual CfObject causing this trigger
 *
 * @param map
 * map we're on.
 * @param args
 * arguments as explained above.
 * @return
 * NULL if error, Py_None else.
 */
static PyObject *Map_TriggerConnected(Crossfire_Map *map, PyObject *args) {
    objectlink *ol = NULL;
    int connected;
    int state;
    Crossfire_Object *cause = NULL;
    oblinkpt *olp;

    if (!PyArg_ParseTuple(args, "ii|O!", &connected, &state, &Crossfire_ObjectType, &cause))
        return NULL;

    MAPEXISTCHECK(map);

    /* make sure the map is swapped in */
    ensure_map_in_memory(map);

    /* locate objectlink for this connected value */
    if (!map->map->buttons) {
        cf_log(llevError, "Map %s called for trigger on connected %d but there ain't any button list for that map!\n", cf_map_get_sstring_property(map->map, CFAPI_MAP_PROP_PATH), connected);
        PyErr_SetString(PyExc_ReferenceError, "No objects connected to that ID on this map.");
        return NULL;
    }
    for (olp = map->map->buttons; olp; olp = olp->next) {
        if (olp->value == connected) {
            ol = olp->link;
            break;
        }
    }
    if (ol == NULL) {
        cf_log(llevInfo, "Map %s called for trigger on connected %d but there ain't any button list for that map!\n", cf_map_get_sstring_property(map->map, CFAPI_MAP_PROP_PATH), connected);
        /* FIXME: I'm not sure about this message... */
        PyErr_SetString(PyExc_ReferenceError, "No objects with that connection ID on this map.");
        return NULL;
    }
    /* run the object link */
    cf_map_trigger_connected(ol, cause ? cause->obj : NULL, state);

    Py_INCREF(Py_None);
    return Py_None;
}

static int Map_InternalCompare(Crossfire_Map *left, Crossfire_Map *right) {
    MAPEXISTCHECK_INT(left);
    MAPEXISTCHECK_INT(right);
    return left->map < right->map ? -1 : (left->map == right->map ? 0 : 1);
}

static PyObject *Crossfire_Map_RichCompare(Crossfire_Map *left, Crossfire_Map *right, int op) {
    int result;
    if (!left
        || !right
        || !PyObject_TypeCheck((PyObject*)left, &Crossfire_MapType)
        || !PyObject_TypeCheck((PyObject*)right, &Crossfire_MapType)) {
        Py_INCREF(Py_NotImplemented);
        return Py_NotImplemented;
    }
    result = Map_InternalCompare(left, right);
    /* Handle removed maps. */
    if (result == -1 && PyErr_Occurred())
        return NULL;
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

/* Legacy code: convert to long so that non-object functions work correctly */
static PyObject *Crossfire_Map_Long(PyObject *obj) {
    MAPEXISTCHECK((Crossfire_Map *)obj);
    return Py_BuildValue("l", ((Crossfire_Map *)obj)->map);
}

#ifndef IS_PY3K
static PyObject *Crossfire_Map_Int(PyObject *obj) {
    MAPEXISTCHECK((Crossfire_Map *)obj);
    return Py_BuildValue("i", ((Crossfire_Map *)obj)->map);
}
#endif

/**
 * Python initialized.
 **/
static PyObject *Crossfire_Map_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    Crossfire_Map *self;

    self = (Crossfire_Map *)type->tp_alloc(type, 0);
    if (self)
        self->map = NULL;

    return (PyObject *)self;
}

static void Crossfire_Map_dealloc(PyObject *obj) {
    Crossfire_Map *self;

    self = (Crossfire_Map *)obj;
    if (self) {
        if (self->map && self->valid) {
            free_map_assoc(self->map);
        }
        Py_TYPE(self)->tp_free(obj);
    }
}

void Handle_Map_Unload_Hook(Crossfire_Map *map) {
    map->valid = 0;
    free_map_assoc(map->map);
}

PyObject *Crossfire_Map_wrap(mapstruct *what) {
    Crossfire_Map *wrapper;

    /* return None if no object was to be wrapped */
    if (what == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    wrapper = (Crossfire_Map *)find_assoc_pymap(what);
    if (!wrapper) {
        wrapper = PyObject_NEW(Crossfire_Map, &Crossfire_MapType);
        if (wrapper != NULL) {
            wrapper->map = what;
            wrapper->valid = 1;
            add_map_assoc(what, wrapper);
        }
    } else {
        Py_INCREF(wrapper);
    }

    return (PyObject *)wrapper;
}

/* Python binding */
static PyGetSetDef Map_getseters[] = {
    { "Difficulty",      (getter)Map_GetDifficulty,  NULL, NULL, NULL },
    { "Path",            (getter)Map_GetPath,        (setter)Map_SetPath, NULL, NULL },
    { "TempName",        (getter)Map_GetTempName,    NULL, NULL, NULL },
    { "Name",            (getter)Map_GetName,        NULL, NULL, NULL },
    { "ResetTime",       (getter)Map_GetResetTime,   NULL, NULL, NULL },
    { "ResetTimeout",    (getter)Map_GetResetTimeout, NULL, NULL, NULL },
    { "Players",         (getter)Map_GetPlayers,     NULL, NULL, NULL },
    { "Light",           (getter)Map_GetDarkness,    NULL, NULL, NULL },
    { "Darkness",        (getter)Map_GetDarkness,    NULL, NULL, NULL },
    { "Width",           (getter)Map_GetWidth,       NULL, NULL, NULL },
    { "Height",          (getter)Map_GetHeight,      NULL, NULL, NULL },
    { "EnterX",          (getter)Map_GetEnterX,      NULL, NULL, NULL },
    { "EnterY",          (getter)Map_GetEnterY,      NULL, NULL, NULL },
    { "Message",         (getter)Map_GetMessage,     NULL, NULL, NULL },
    { "Region",          (getter)Map_GetRegion,      NULL, NULL, NULL },
    { "Unique",          (getter)Map_GetUnique,      NULL, NULL, NULL },
    {  NULL, NULL, NULL, NULL, NULL }
};

static PyMethodDef MapMethods[] = {
    { "Print",            (PyCFunction)Map_Message,          METH_VARARGS, NULL },
    { "ObjectAt",         (PyCFunction)Map_GetFirstObjectAt, METH_VARARGS, NULL },
    { "CreateObject",     (PyCFunction)Map_CreateObject,     METH_VARARGS, NULL },
    { "Check",            (PyCFunction)Map_Check,            METH_VARARGS, NULL },
    { "Next",             (PyCFunction)Map_Next,             METH_NOARGS,  NULL },
    { "Insert",           (PyCFunction)Map_Insert,           METH_VARARGS, NULL },
    { "InsertAround",     (PyCFunction)Map_InsertAround,     METH_VARARGS, NULL },
    { "ChangeLight",      (PyCFunction)Map_ChangeLight,      METH_VARARGS, NULL },
    { "TriggerConnected", (PyCFunction)Map_TriggerConnected, METH_VARARGS, NULL },
    { NULL, NULL, 0, NULL }
};

static PyNumberMethods MapConvert = {
    NULL,            /* binaryfunc nb_add; */        /* __add__ */
    NULL,            /* binaryfunc nb_subtract; */   /* __sub__ */
    NULL,            /* binaryfunc nb_multiply; */   /* __mul__ */
#ifndef IS_PY3K
    NULL,            /* binaryfunc nb_divide; */     /* __div__ */
#endif
    NULL,            /* binaryfunc nb_remainder; */  /* __mod__ */
    NULL,            /* binaryfunc nb_divmod; */     /* __divmod__ */
    NULL,            /* ternaryfunc nb_power; */     /* __pow__ */
    NULL,            /* unaryfunc nb_negative; */    /* __neg__ */
    NULL,            /* unaryfunc nb_positive; */    /* __pos__ */
    NULL,            /* unaryfunc nb_absolute; */    /* __abs__ */
#ifdef IS_PY3K
    NULL,            /* inquiry nb_bool; */          /* __bool__ */
#else
    NULL,            /* inquiry nb_nonzero; */       /* __nonzero__ */
#endif
    NULL,            /* unaryfunc nb_invert; */      /* __invert__ */
    NULL,            /* binaryfunc nb_lshift; */     /* __lshift__ */
    NULL,            /* binaryfunc nb_rshift; */     /* __rshift__ */
    NULL,            /* binaryfunc nb_and; */        /* __and__ */
    NULL,            /* binaryfunc nb_xor; */        /* __xor__ */
    NULL,            /* binaryfunc nb_or; */         /* __or__ */
#ifndef IS_PY3K
    NULL,            /* coercion nb_coerce; */       /* __coerce__ */
#endif
#ifdef IS_PY3K
    /* This is not a typo. For Py3k it should be Crossfire_Map_Long
     * and NOT Crossfire_Map_Int.
     */
    Crossfire_Map_Long, /* unaryfunc nb_int; */      /* __int__ */
    NULL,               /* void *nb_reserved; */
#else
    Crossfire_Map_Int,  /* unaryfunc nb_int; */      /* __int__ */
    Crossfire_Map_Long, /* unaryfunc nb_long; */     /* __long__ */
#endif
    NULL,            /* unaryfunc nb_float; */       /* __float__ */
#ifndef IS_PY3K
    NULL,            /* unaryfunc nb_oct; */         /* __oct__ */
    NULL,            /* unaryfunc nb_hex; */         /* __hex__ */
#endif
    NULL,            /* binaryfunc nb_inplace_add; */
    NULL,            /* binaryfunc nb_inplace_subtract; */
    NULL,            /* binaryfunc nb_inplace_multiply; */
#ifndef IS_PY3K
    NULL,            /* binaryfunc nb_inplace_divide; */
#endif
    NULL,            /* binaryfunc nb_inplace_remainder; */
    NULL,            /* ternaryfunc nb_inplace_power; */
    NULL,            /* binaryfunc nb_inplace_lshift; */
    NULL,            /* binaryfunc nb_inplace_rshift; */
    NULL,            /* binaryfunc nb_inplace_and; */
    NULL,            /* binaryfunc nb_inplace_xor; */
    NULL,            /* binaryfunc nb_inplace_or; */

    NULL,            /* binaryfunc nb_floor_divide; */
    NULL,            /* binaryfunc nb_true_divide; */
    NULL,            /* binaryfunc nb_inplace_floor_divide; */
    NULL,            /* binaryfunc nb_inplace_true_divide; */
#if defined(IS_PY25) || defined(IS_PY3K)
    NULL             /* unaryfunc nb_index; */
#endif
};

/* Our actual Python MapType */
PyTypeObject Crossfire_MapType = {
#ifdef IS_PY3K
    /* See http://bugs.python.org/issue4385 */
    PyVarObject_HEAD_INIT(NULL, 0)
#else
    PyObject_HEAD_INIT(NULL)
    0,                         /* ob_size*/
#endif
    "Crossfire.Map",           /* tp_name*/
    sizeof(Crossfire_Map),     /* tp_basicsize*/
    0,                         /* tp_itemsize*/
    Crossfire_Map_dealloc,     /* tp_dealloc*/
    NULL,                      /* tp_print*/
    NULL,                      /* tp_getattr*/
    NULL,                      /* tp_setattr*/
#ifdef IS_PY3K
    NULL,                      /* tp_reserved */
#else
    (cmpfunc)Map_InternalCompare, /* tp_compare*/
#endif
    NULL,                      /* tp_repr*/
    &MapConvert,               /* tp_as_number*/
    NULL,                      /* tp_as_sequence*/
    NULL,                      /* tp_as_mapping*/
    PyObject_HashNotImplemented, /* tp_hash */
    NULL,                      /* tp_call*/
    NULL,                      /* tp_str*/
    PyObject_GenericGetAttr,   /* tp_getattro*/
    PyObject_GenericSetAttr,   /* tp_setattro*/
    NULL,                      /* tp_as_buffer*/
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags*/
    "Crossfire maps",          /* tp_doc */
    NULL,                      /* tp_traverse */
    NULL,                      /* tp_clear */
    (richcmpfunc)Crossfire_Map_RichCompare, /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    NULL,                      /* tp_iter */
    NULL,                      /* tp_iternext */
    MapMethods,                /* tp_methods */
    NULL,                      /* tp_members */
    Map_getseters,             /* tp_getset */
    NULL,                      /* tp_base */
    NULL,                      /* tp_dict */
    NULL,                      /* tp_descr_get */
    NULL,                      /* tp_descr_set */
    0,                         /* tp_dictoffset */
    NULL,                      /* tp_init */
    NULL,                      /* tp_alloc */
    Crossfire_Map_new,         /* tp_new */
    NULL,                      /* tp_free */
    NULL,                      /* tp_is_gc */
    NULL,                      /* tp_bases */
    NULL,                      /* tp_mro */
    NULL,                      /* tp_cache */
    NULL,                      /* tp_subclasses */
    NULL,                      /* tp_weaklist */
    NULL,                      /* tp_del */
};
