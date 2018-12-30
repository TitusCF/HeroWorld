/*
 * Copyright (C) 2006-2007 Dan Pascu.
 * Author: Dan Pascu <dan@ag-projects.com>
 *
 * Fast JSON encoder/decoder implementation for Python
 * This file is under GPL licence
 */

#include <cfpython.h>
#include <Python.h>
#include <stddef.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

#if defined(IS_PY26)
#    include <bytesobject.h>
#elif !defined(IS_PY3K)
/* Python versions 2.5 or older doesn't have bytesobject.h */
#    define PyBytesObject PyStringObject

#    define PyBytes_Check PyString_Check
#    define PyBytes_AS_STRING PyString_AS_STRING
#    define PyBytes_GET_SIZE PyString_GET_SIZE

#    define PyBytes_FromStringAndSize PyString_FromStringAndSize
#    define PyBytes_FromString PyString_FromString
#    define PyBytes_Size PyString_Size
#    define PyBytes_AsString PyString_AsString
#    define PyBytes_Concat PyString_Concat
#    define PyBytes_ConcatAndDel PyString_ConcatAndDel
#    define _PyBytes_Resize _PyString_Resize
#    define PyBytes_DecodeEscape PyString_DecodeEscape
#    define _PyBytes_Join _PyString_Join
#    define PyBytes_AsStringAndSize PyString_AsStringAndSize
#endif

typedef struct JSONData {
    char *str; /* the actual json string */
    char *end; /* pointer to the string end */
    char *ptr; /* pointer to the current parsing position */
    int  all_unicode; /* make all output strings unicode if true */
} JSONData;

static PyObject *encode_object(PyObject *object);
static PyObject *encode_string(PyObject *object);
static PyObject *encode_unicode(PyObject *object);
static PyObject *encode_tuple(PyObject *object);
static PyObject *encode_list(PyObject *object);
static PyObject *encode_dict(PyObject *object);

static PyObject *decode_json(JSONData *jsondata);
static PyObject *decode_null(JSONData *jsondata);
static PyObject *decode_bool(JSONData *jsondata);
static PyObject *decode_string(JSONData *jsondata);
static PyObject *decode_inf(JSONData *jsondata);
static PyObject *decode_nan(JSONData *jsondata);
static PyObject *decode_number(JSONData *jsondata);
static PyObject *decode_array(JSONData *jsondata);
static PyObject *decode_object(JSONData *jsondata);

static PyObject *JSON_Error;
static PyObject *JSON_EncodeError;
static PyObject *JSON_DecodeError;

#if PY_VERSION_HEX < 0x02050000
typedef int Py_ssize_t;

#define PY_SSIZE_T_MAX INT_MAX
#define PY_SSIZE_T_MIN INT_MIN

#define SSIZE_T_F "%d"
#else
#define SSIZE_T_F "%zd"
#endif

#define True  1
#define False 0

#ifndef INFINITY
#define INFINITY HUGE_VAL
#endif

#ifndef NAN
#define NAN (HUGE_VAL-HUGE_VAL)
#endif

#ifndef Py_IS_NAN
#define Py_IS_NAN(X) ((X) != (X))
#endif

#define skipSpaces(d) while (*((d)->ptr) && isspace(*((d)->ptr))) (d)->ptr++

/* ------------------------------ Utility ----------------------------- */

#ifdef IS_PY3K
/** Same as PyObject_Str but return a UTF-8 encoded Bytes object instead. */
static PyObject* cjson_PyObject_Str(PyObject* obj) {
    PyObject *tmp_str, *result;
    tmp_str = PyObject_Str(obj);
    if (!tmp_str)
        return NULL;
    result = PyUnicode_AsUTF8String(tmp_str);
    Py_DECREF(tmp_str);
    return result;
}
#else
#  define cjson_PyObject_Str PyObject_Str
#endif

/* ------------------------------ Decoding ----------------------------- */

static void getRowAndCol(char *begin, char *current, int *row, int *col) {
    *col = 1;
    *row = 1;
    while (current > begin) {
        if (*current == '\n')
            (*row)++;
        if (*row < 2)
            (*col)++;
        current--;
    }
}
static PyObject *decode_null(JSONData *jsondata) {
    ptrdiff_t left;

    left = jsondata->end-jsondata->ptr;

    if (left >= 4 && strncmp(jsondata->ptr, "null", 4) == 0) {
        jsondata->ptr += 4;
        Py_INCREF(Py_None);
        return Py_None;
    } else {
        int row, col;

        getRowAndCol(jsondata->str, jsondata->ptr, &row, &col);
        PyErr_Format(JSON_DecodeError, "cannot parse JSON description: %.20s" "(row "SSIZE_T_F", col "SSIZE_T_F")", jsondata->ptr, (Py_ssize_t)row, (Py_ssize_t)col);
        return NULL;
    }
}

static PyObject *decode_bool(JSONData *jsondata) {
    ptrdiff_t left;

    left = jsondata->end-jsondata->ptr;

    if (left >= 4 && strncmp(jsondata->ptr, "true", 4) == 0) {
        jsondata->ptr += 4;
        Py_INCREF(Py_True);
        return Py_True;
    } else if (left >= 5 && strncmp(jsondata->ptr, "false", 5) == 0) {
        jsondata->ptr += 5;
        Py_INCREF(Py_False);
        return Py_False;
    } else {
        int row, col;

        getRowAndCol(jsondata->str, jsondata->ptr, &row, &col);
        PyErr_Format(JSON_DecodeError, "cannot parse JSON description: %.20s" "(row "SSIZE_T_F", col "SSIZE_T_F")", jsondata->ptr, (Py_ssize_t)row, (Py_ssize_t)col);
        return NULL;
    }
}

static PyObject *decode_string(JSONData *jsondata) {
    PyObject *object;
    int c, escaping, has_unicode, string_escape;
    Py_ssize_t len;
    char *ptr;

    /* look for the closing quote */
    escaping = has_unicode = string_escape = False;
    ptr = jsondata->ptr+1;
    while (True) {
        c = *ptr;
        if (c == 0) {
            int row, col;

            getRowAndCol(jsondata->str, jsondata->ptr, &row, &col);
            PyErr_Format(JSON_DecodeError, "unterminated string starting at position "SSIZE_T_F"(row "SSIZE_T_F", col "SSIZE_T_F")",
                         (Py_ssize_t)(jsondata->ptr-jsondata->str), (Py_ssize_t)row, (Py_ssize_t)col);
            return NULL;
        }
        if (!escaping) {
            if (c == '\\') {
                escaping = True;
            } else if (c == '"') {
                break;
            } else if (!isascii(c)) {
                has_unicode = True;
            }
        } else {
            switch (c) {
            case 'u':
                has_unicode = True;
                break;
            case '"':
            case 'r':
            case 'n':
            case 't':
            case 'b':
            case 'f':
            case '\\':
                string_escape = True;
                break;
            }
            escaping = False;
        }
        ptr++;
    }

    len = ptr-jsondata->ptr-1;

    if (has_unicode || jsondata->all_unicode)
        object = PyUnicode_DecodeUnicodeEscape(jsondata->ptr+1, len, NULL);
    else if (string_escape)
        object = PyBytes_DecodeEscape(jsondata->ptr+1, len, NULL, 0, NULL);
    else
        object = PyBytes_FromStringAndSize(jsondata->ptr+1, len);

    if (object == NULL) {
        PyObject *type, *value, *tb, *reason;

        PyErr_Fetch(&type, &value, &tb);
        if (type == NULL) {
            int row, col;

            getRowAndCol(jsondata->str, jsondata->ptr, &row, &col);
            PyErr_Format(JSON_DecodeError, "invalid string starting at position "SSIZE_T_F"(row "SSIZE_T_F", col "SSIZE_T_F")",
                         (Py_ssize_t)(jsondata->ptr-jsondata->str), (Py_ssize_t)row, (Py_ssize_t)col);
        } else {
            if (PyErr_GivenExceptionMatches(type, PyExc_UnicodeDecodeError)) {
                int row, col;

                reason = PyObject_GetAttrString(value, "reason");
#ifdef IS_PY3K
                /* If we are using Python3 we need to convert to bytes here. */
                if (reason && PyUnicode_Check(reason)) {
                    PyObject * bytes_reason = PyUnicode_AsUTF8String(reason);
                    Py_DECREF(reason);
                    reason = bytes_reason;
                }
#endif
                getRowAndCol(jsondata->str, jsondata->ptr, &row, &col);
                PyErr_Format(JSON_DecodeError, "cannot decode string starting" " at position "SSIZE_T_F"(row "SSIZE_T_F", col "SSIZE_T_F"): %s",
                             (Py_ssize_t)(jsondata->ptr-jsondata->str), (Py_ssize_t)row, (Py_ssize_t)col,
                             reason ? PyBytes_AsString(reason) : "bad format");
                Py_XDECREF(reason);
            } else {
                int row, col;

                getRowAndCol(jsondata->str, jsondata->ptr, &row, &col);
                PyErr_Format(JSON_DecodeError, "invalid string starting at position "SSIZE_T_F"(row "SSIZE_T_F", col "SSIZE_T_F")",
                             (Py_ssize_t)(jsondata->ptr-jsondata->str), (Py_ssize_t)row, (Py_ssize_t)col);
            }
        }
        Py_XDECREF(type);
        Py_XDECREF(value);
        Py_XDECREF(tb);
    } else {
        jsondata->ptr = ptr+1;
    }

    return object;
}

static PyObject *decode_inf(JSONData *jsondata) {
    PyObject *object;
    ptrdiff_t left;

    left = jsondata->end-jsondata->ptr;

    if (left >= 8 && strncmp(jsondata->ptr, "Infinity", 8) == 0) {
        jsondata->ptr += 8;
        object = PyFloat_FromDouble(INFINITY);
        return object;
    } else if (left >= 9 && strncmp(jsondata->ptr, "+Infinity", 9) == 0) {
        jsondata->ptr += 9;
        object = PyFloat_FromDouble(INFINITY);
        return object;
    } else if (left >= 9 && strncmp(jsondata->ptr, "-Infinity", 9) == 0) {
        jsondata->ptr += 9;
        object = PyFloat_FromDouble(-INFINITY);
        return object;
    } else {
        int row, col;

        getRowAndCol(jsondata->str, jsondata->ptr, &row, &col);
        PyErr_Format(JSON_DecodeError, "cannot parse JSON description: %.20s (row "SSIZE_T_F", col "SSIZE_T_F")", jsondata->ptr, (Py_ssize_t)row, (Py_ssize_t)col);
        return NULL;
    }
}

static PyObject *decode_nan(JSONData *jsondata) {
    PyObject *object;
    ptrdiff_t left;

    left = jsondata->end-jsondata->ptr;

    if (left >= 3 && strncmp(jsondata->ptr, "NaN", 3) == 0) {
        jsondata->ptr += 3;
        object = PyFloat_FromDouble(NAN);
        return object;
    } else {
        int row, col;
        getRowAndCol(jsondata->str, jsondata->ptr, &row, &col);
        PyErr_Format(JSON_DecodeError, "cannot parse JSON description: %.20s(row "SSIZE_T_F", col "SSIZE_T_F")", jsondata->ptr, (Py_ssize_t)row, (Py_ssize_t)col);
        return NULL;
    }
}

static PyObject *decode_number(JSONData *jsondata) {
    PyObject *object, *str;
    int c, is_float, should_stop;
    char *ptr;

    /* check if we got a floating point number */
    ptr = jsondata->ptr;
    is_float = should_stop = False;
    while (True) {
        c = *ptr;
        if (c == 0)
            break;
        switch (c) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '-':
        case '+':
            break;

        case '.':
        case 'e':
        case 'E':
            is_float = True;
            break;

        default:
            should_stop = True;
        }
        if (should_stop) {
            break;
        }
        ptr++;
    }

    str = PyBytes_FromStringAndSize(jsondata->ptr, ptr-jsondata->ptr);
    if (str == NULL)
        return NULL;

    if (is_float) {
#ifdef IS_PY3K
        object = PyFloat_FromString(str);
#else
        object = PyFloat_FromString(str, NULL);
#endif
    } else {
#ifdef IS_PY3K
        object = PyLong_FromString(PyBytes_AS_STRING(str), NULL, 10);
#else
        object = PyInt_FromString(PyBytes_AS_STRING(str), NULL, 10);
#endif
    }

    Py_DECREF(str);

    if (object == NULL) {
        int row, col;

        getRowAndCol(jsondata->str, jsondata->ptr, &row, &col);
        PyErr_Format(JSON_DecodeError, "invalid number starting at position "SSIZE_T_F"(row "SSIZE_T_F", col "SSIZE_T_F")", (Py_ssize_t)(jsondata->ptr-jsondata->str), (Py_ssize_t)row, (Py_ssize_t)col);
    } else {
        jsondata->ptr = ptr;
    }

    return object;
}

static PyObject *decode_array(JSONData *jsondata) {
    PyObject *object, *item;
    int c, expect_item, items, result;
    char *start;

    object = PyList_New(0);

    start = jsondata->ptr;
    jsondata->ptr++;
    expect_item = True;
    items = 0;
    while (True) {
        skipSpaces(jsondata);
        c = *jsondata->ptr;
        if (c == 0) {
            int row, col;

            getRowAndCol(jsondata->str, jsondata->ptr, &row, &col);
            PyErr_Format(JSON_DecodeError, "unterminated array starting at position "SSIZE_T_F"(row "SSIZE_T_F", col "SSIZE_T_F")",
                         (Py_ssize_t)(start-jsondata->str), (Py_ssize_t)row, (Py_ssize_t)col);
            goto failure;
        } else if (c == ']') {
            if (expect_item && items > 0) {
                int row, col;

                getRowAndCol(jsondata->str, jsondata->ptr, &row, &col);
                PyErr_Format(JSON_DecodeError, "expecting array item at position "SSIZE_T_F"(row "SSIZE_T_F", col "SSIZE_T_F")",
                             (Py_ssize_t)(jsondata->ptr-jsondata->str), (Py_ssize_t)row, (Py_ssize_t)col);
                goto failure;
            }
            jsondata->ptr++;
            break;
        } else if (c == ',') {
            if (expect_item) {
                int row, col;

                getRowAndCol(jsondata->str, jsondata->ptr, &row, &col);
                PyErr_Format(JSON_DecodeError, "expecting array item at position "SSIZE_T_F"(row "SSIZE_T_F", col "SSIZE_T_F")",
                             (Py_ssize_t)(jsondata->ptr-jsondata->str), (Py_ssize_t)row, (Py_ssize_t)col);
                goto failure;
            }
            expect_item = True;
            jsondata->ptr++;
            continue;
        } else {
            item = decode_json(jsondata);
            if (item == NULL)
                goto failure;
            result = PyList_Append(object, item);
            Py_DECREF(item);
            if (result == -1)
                goto failure;
            expect_item = False;
            items++;
        }
    }

    return object;

failure:
    Py_DECREF(object);
    return NULL;
}

static PyObject *decode_object(JSONData *jsondata) {
    PyObject *object, *key, *value;
    int c, expect_key, items, result;
    char *start;

    object = PyDict_New();

    expect_key = True;
    items = 0;
    start = jsondata->ptr;
    jsondata->ptr++;

    while (True) {
        skipSpaces(jsondata);
        c = *jsondata->ptr;
        if (c == 0) {
            int row, col;

            getRowAndCol(jsondata->str, jsondata->ptr, &row, &col);
            PyErr_Format(JSON_DecodeError, "unterminated object starting at position "SSIZE_T_F"(row "SSIZE_T_F", col "SSIZE_T_F")",
                         (Py_ssize_t)(start-jsondata->str), (Py_ssize_t)row, (Py_ssize_t)col);
            goto failure;
        } else if (c == '}') {
            if (expect_key && items > 0) {
                int row, col;

                getRowAndCol(jsondata->str, jsondata->ptr, &row, &col);
                PyErr_Format(JSON_DecodeError, "expecting object property name at position "SSIZE_T_F"(row "SSIZE_T_F", col "SSIZE_T_F")",
                             (Py_ssize_t)(jsondata->ptr-jsondata->str), (Py_ssize_t)row, (Py_ssize_t)col);
                goto failure;
            }
            jsondata->ptr++;
            break;
        } else if (c == ',') {
            if (expect_key) {
                int row, col;

                getRowAndCol(jsondata->str, jsondata->ptr, &row, &col);
                PyErr_Format(JSON_DecodeError, "expecting object property name at position "SSIZE_T_F"(row "SSIZE_T_F", col "SSIZE_T_F")",
                             (Py_ssize_t)(jsondata->ptr-jsondata->str), (Py_ssize_t)row, (Py_ssize_t)col);
                goto failure;
            }
            expect_key = True;
            jsondata->ptr++;
            continue;
        } else {
            if (c != '"') {
                int row, col;

                getRowAndCol(jsondata->str, jsondata->ptr, &row, &col);
                PyErr_Format(JSON_DecodeError, "expecting property name in object at position "SSIZE_T_F"(row "SSIZE_T_F", col "SSIZE_T_F")",
                             (Py_ssize_t)(jsondata->ptr-jsondata->str), (Py_ssize_t)row, (Py_ssize_t)col);
                goto failure;
            }

            key = decode_json(jsondata);
            if (key == NULL)
                goto failure;

            skipSpaces(jsondata);
            if (*jsondata->ptr != ':') {
                int row, col;

                getRowAndCol(jsondata->str, jsondata->ptr, &row, &col);
                PyErr_Format(JSON_DecodeError, "missing colon after object property name at position "SSIZE_T_F"(row "SSIZE_T_F", col "SSIZE_T_F")",
                             (Py_ssize_t)(jsondata->ptr-jsondata->str), (Py_ssize_t)row, (Py_ssize_t)col);
                Py_DECREF(key);
                goto failure;
            } else {
                jsondata->ptr++;
            }

            value = decode_json(jsondata);
            if (value == NULL) {
                Py_DECREF(key);
                goto failure;
            }

            result = PyDict_SetItem(object, key, value);
            Py_DECREF(key);
            Py_DECREF(value);
            if (result == -1)
                goto failure;
            expect_key = False;
            items++;
        }
    }

    return object;

failure:
    Py_DECREF(object);
    return NULL;
}

static PyObject *decode_json(JSONData *jsondata) {
    PyObject *object;

    skipSpaces(jsondata);
    switch (*jsondata->ptr) {
    case 0:
        PyErr_SetString(JSON_DecodeError, "empty JSON description");
        return NULL;

    case '{':
        object = decode_object(jsondata);
        break;

    case '[':
        object = decode_array(jsondata);
        break;

    case '"':
        object = decode_string(jsondata);
        break;

    case 't':
    case 'f':
        object = decode_bool(jsondata);
        break;

    case 'n':
        object = decode_null(jsondata);
        break;

    case 'N':
        object = decode_nan(jsondata);
        break;

    case 'I':
        object = decode_inf(jsondata);
        break;

    case '+':
    case '-':
        if (*(jsondata->ptr+1) == 'I') {
            object = decode_inf(jsondata);
        } else {
            object = decode_number(jsondata);
        }
        break;

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        object = decode_number(jsondata);
        break;

    default:
        PyErr_SetString(JSON_DecodeError, "cannot parse JSON description");
        return NULL;
    }

    return object;
}

/* ------------------------------ Encoding ----------------------------- */

/*
 * This function is an almost verbatim copy of PyString_Repr() from
 * Python's stringobject.c with the following differences:
 *
 * - it always quotes the output using double quotes.
 * - it also quotes \b and \f
 * - it replaces any non ASCII character hh with \u00hh instead of \xhh
 */
static PyObject *encode_string(PyObject *string) {
    register PyBytesObject *op = (PyBytesObject *)string;
    size_t newsize = 2+6*Py_SIZE(op);
    PyObject *v;

    if (Py_SIZE(op) > (PY_SSIZE_T_MAX-2)/6) {
        PyErr_SetString(PyExc_OverflowError, "string is too large to make repr");
        return NULL;
    }
    v = PyBytes_FromStringAndSize((char *)NULL, newsize);
    if (v == NULL) {
        return NULL;
    } else {
        register Py_ssize_t i;
        register char c;
        register char *p;
        int quote;

        quote = '"';

        p = PyBytes_AS_STRING(v);
        *p++ = quote;
        for (i = 0; i < Py_SIZE(op); i++) {
            /* There's at least enough room for a hex escape
             and a closing quote. */
            assert(newsize-(p-PyBytes_AS_STRING(v)) >= 7);
            c = op->ob_sval[i];
            if (c == quote || c == '\\')
                *p++ = '\\', *p++ = c;
            else if (c == '\t')
                *p++ = '\\', *p++ = 't';
            else if (c == '\n')
                *p++ = '\\', *p++ = 'n';
            else if (c == '\r')
                *p++ = '\\', *p++ = 'r';
            else if (c == '\f')
                *p++ = '\\', *p++ = 'f';
            else if (c == '\b')
                *p++ = '\\', *p++ = 'b';
            else if (c < ' ' || c >= 0x7f) {
                /* For performance, we don't want to call
                 * PyOS_snprintf here (extra layers of
                 * function call). */
                sprintf(p, "\\u%04x", c&0xff);
                p += 6;
            }
            else
                *p++ = c;
        }
        assert(newsize-(p-PyBytes_AS_STRING(v)) >= 1);
        *p++ = quote;
        *p = '\0';
        _PyBytes_Resize(&v, (int) (p-PyBytes_AS_STRING(v)));
        return v;
    }
}

/*
 * This function is an almost verbatim copy of unicodeescape_string() from
 * Python's unicodeobject.c with the following differences:
 *
 * - it always quotes the output using double quotes.
 * - it uses \u00hh instead of \xhh in output.
 * - it also quotes \b and \f
 */
#if defined(IS_PY26) || defined(IS_PY3K)
static PyObject *encode_unicode(PyObject *unicode) {
    PyObject *repr;
    Py_UNICODE *s;
    Py_ssize_t size;
    char *p;
    static const char *hexdigit = "0123456789abcdef";
#ifdef Py_UNICODE_WIDE
    static const Py_ssize_t expandsize = 10;
#else
    static const Py_ssize_t expandsize = 6;
#endif

    s = PyUnicode_AS_UNICODE(unicode);
    size = PyUnicode_GET_SIZE(unicode);

    if (size > (PY_SSIZE_T_MAX-2-1)/expandsize) {
        PyErr_SetString(PyExc_OverflowError, "unicode object is too large to make repr");
        return NULL;
    }

    repr = PyByteArray_FromStringAndSize(NULL,
        2
        + expandsize*size
        + 1);
    if (repr == NULL)
        return NULL;

    p = PyByteArray_AS_STRING(repr);

    *p++ = '"';

    while (size-- > 0) {
        Py_UNICODE ch = *s++;

        /* Escape quotes */
        if ((ch == (Py_UNICODE)PyByteArray_AS_STRING(repr)[0] || ch == '\\')) {
            *p++ = '\\';
            *p++ = (char)ch;
            continue;
        }
#ifdef Py_UNICODE_WIDE
        /* Map 21-bit characters to '\U00xxxxxx' */
        else if (ch >= 0x10000) {
            *p++ = '\\';
            *p++ = 'U';
            *p++ = hexdigit[(ch>>28)&0x0000000F];
            *p++ = hexdigit[(ch>>24)&0x0000000F];
            *p++ = hexdigit[(ch>>20)&0x0000000F];
            *p++ = hexdigit[(ch>>16)&0x0000000F];
            *p++ = hexdigit[(ch>>12)&0x0000000F];
            *p++ = hexdigit[(ch>>8)&0x0000000F];
            *p++ = hexdigit[(ch>>4)&0x0000000F];
            *p++ = hexdigit[ch&0x0000000F];
            continue;
        }
#else
        /* Map UTF-16 surrogate pairs to Unicode \UXXXXXXXX escapes */
        else if (ch >= 0xD800 && ch < 0xDC00) {
            Py_UNICODE ch2;
            Py_UCS4 ucs;

            ch2 = *s++;
            size--;
            if (ch2 >= 0xDC00 && ch2 <= 0xDFFF) {
                ucs = (((ch&0x03FF)<<10)|(ch2&0x03FF))+0x00010000;
                *p++ = '\\';
                *p++ = 'U';
                *p++ = hexdigit[(ucs>>28)&0x0000000F];
                *p++ = hexdigit[(ucs>>24)&0x0000000F];
                *p++ = hexdigit[(ucs>>20)&0x0000000F];
                *p++ = hexdigit[(ucs>>16)&0x0000000F];
                *p++ = hexdigit[(ucs>>12)&0x0000000F];
                *p++ = hexdigit[(ucs>>8)&0x0000000F];
                *p++ = hexdigit[(ucs>>4)&0x0000000F];
                *p++ = hexdigit[ucs&0x0000000F];
                continue;
            }
            /* Fall through: isolated surrogates are copied as-is */
            s--;
            size++;
        }
#endif
        /* Map 16-bit characters to '\uxxxx' */
        if (ch >= 256) {
            *p++ = '\\';
            *p++ = 'u';
            *p++ = hexdigit[(ch>>12)&0x000F];
            *p++ = hexdigit[(ch>>8)&0x000F];
            *p++ = hexdigit[(ch>>4)&0x000F];
            *p++ = hexdigit[ch&0x000F];
        /* Map special whitespace to '\t', \n', '\r', '\f', '\b' */
        } else if (ch == '\t') {
            *p++ = '\\';
            *p++ = 't';
        } else if (ch == '\n') {
            *p++ = '\\';
            *p++ = 'n';
        } else if (ch == '\r') {
            *p++ = '\\';
            *p++ = 'r';
        } else if (ch == '\f') {
            *p++ = '\\';
            *p++ = 'f';
        } else if (ch == '\b') {
            *p++ = '\\';
            *p++ = 'b';
        }

        /* Map non-printable US ASCII to '\u00hh' */
        else if (ch < ' ' || ch >= 0x7F) {
            *p++ = '\\';
            *p++ = 'u';
            *p++ = '0';
            *p++ = '0';
            *p++ = hexdigit[(ch>>4)&0x000F];
            *p++ = hexdigit[ch&0x000F];
        }

        /* Copy everything else as-is */
        else
            *p++ = (char)ch;
    }

    *p++ = PyByteArray_AS_STRING(repr)[0];

    *p = '\0';
    {
        PyObject *result = PyBytes_FromStringAndSize(PyByteArray_AS_STRING(repr),
                                                     p - PyByteArray_AS_STRING(repr));
        Py_DECREF(repr);
        return result;
    }
}
#else
static PyObject *encode_unicode(PyObject *unicode) {
    PyObject *repr;
    Py_UNICODE *s;
    Py_ssize_t size;
    char *p;
    static const char *hexdigit = "0123456789abcdef";
#ifdef Py_UNICODE_WIDE
    static const Py_ssize_t expandsize = 10;
#else
    static const Py_ssize_t expandsize = 6;
#endif

    s = PyUnicode_AS_UNICODE(unicode);
    size = PyUnicode_GET_SIZE(unicode);

    if (size > (PY_SSIZE_T_MAX-2-1)/expandsize) {
        PyErr_SetString(PyExc_OverflowError, "unicode object is too large to make repr");
        return NULL;
    }

    repr = PyString_FromStringAndSize(NULL, 2+expandsize*size+1);
    if (repr == NULL)
        return NULL;

    p = PyString_AS_STRING(repr);

    *p++ = '"';

    while (size-- > 0) {
        Py_UNICODE ch = *s++;

        /* Escape quotes */
        if ((ch == (Py_UNICODE)PyString_AS_STRING(repr)[0] || ch == '\\')) {
            *p++ = '\\';
            *p++ = (char)ch;
            continue;
        }
#ifdef Py_UNICODE_WIDE
        /* Map 21-bit characters to '\U00xxxxxx' */
        else if (ch >= 0x10000) {
            int offset = p-PyString_AS_STRING(repr);

            /* Resize the string if necessary */
            if (offset+12 > PyString_GET_SIZE(repr)) {
                if (_PyString_Resize(&repr, PyString_GET_SIZE(repr)+100))
                    return NULL;
                p = PyString_AS_STRING(repr)+offset;
            }

            *p++ = '\\';
            *p++ = 'U';
            *p++ = hexdigit[(ch>>28)&0x0000000F];
            *p++ = hexdigit[(ch>>24)&0x0000000F];
            *p++ = hexdigit[(ch>>20)&0x0000000F];
            *p++ = hexdigit[(ch>>16)&0x0000000F];
            *p++ = hexdigit[(ch>>12)&0x0000000F];
            *p++ = hexdigit[(ch>>8)&0x0000000F];
            *p++ = hexdigit[(ch>>4)&0x0000000F];
            *p++ = hexdigit[ch&0x0000000F];
            continue;
        }
#endif
        /* Map UTF-16 surrogate pairs to Unicode \UXXXXXXXX escapes */
        else if (ch >= 0xD800 && ch < 0xDC00) {
            Py_UNICODE ch2;
            Py_UCS4 ucs;

            ch2 = *s++;
            size--;
            if (ch2 >= 0xDC00 && ch2 <= 0xDFFF) {
                ucs = (((ch&0x03FF)<<10)|(ch2&0x03FF))+0x00010000;
                *p++ = '\\';
                *p++ = 'U';
                *p++ = hexdigit[(ucs>>28)&0x0000000F];
                *p++ = hexdigit[(ucs>>24)&0x0000000F];
                *p++ = hexdigit[(ucs>>20)&0x0000000F];
                *p++ = hexdigit[(ucs>>16)&0x0000000F];
                *p++ = hexdigit[(ucs>>12)&0x0000000F];
                *p++ = hexdigit[(ucs>>8)&0x0000000F];
                *p++ = hexdigit[(ucs>>4)&0x0000000F];
                *p++ = hexdigit[ucs&0x0000000F];
                continue;
            }
            /* Fall through: isolated surrogates are copied as-is */
            s--;
            size++;
        /* Map 16-bit characters to '\uxxxx' */
        } if (ch >= 256) {
            *p++ = '\\';
            *p++ = 'u';
            *p++ = hexdigit[(ch>>12)&0x000F];
            *p++ = hexdigit[(ch>>8)&0x000F];
            *p++ = hexdigit[(ch>>4)&0x000F];
            *p++ = hexdigit[ch&0x000F];
        /* Map special whitespace to '\t', \n', '\r', '\f', '\b' */
        } else if (ch == '\t') {
            *p++ = '\\';
            *p++ = 't';
        } else if (ch == '\n') {
            *p++ = '\\';
            *p++ = 'n';
        } else if (ch == '\r') {
            *p++ = '\\';
            *p++ = 'r';
        } else if (ch == '\f') {
            *p++ = '\\';
            *p++ = 'f';
        } else if (ch == '\b') {
            *p++ = '\\';
            *p++ = 'b';
        }

        /* Map non-printable US ASCII to '\u00hh' */
        else if (ch < ' ' || ch >= 0x7F) {
            *p++ = '\\';
            *p++ = 'u';
            *p++ = '0';
            *p++ = '0';
            *p++ = hexdigit[(ch>>4)&0x000F];
            *p++ = hexdigit[ch&0x000F];
        }

        /* Copy everything else as-is */
        else
            *p++ = (char)ch;
    }

    *p++ = PyString_AS_STRING(repr)[0];

    *p = '\0';
    _PyString_Resize(&repr, p-PyString_AS_STRING(repr));
    return repr;
}
#endif

/*
 * This function is an almost verbatim copy of tuplerepr() from
 * Python's tupleobject.c with the following differences:
 *
 * - it uses encode_object() to get the object's JSON reprezentation.
 * - it uses [] as decorations isntead of () (to masquerade as a JSON array).
 */

static PyObject *encode_tuple(PyObject *tuple) {
    Py_ssize_t i, n;
    PyObject *s, *temp;
    PyObject *pieces, *result = NULL;
    PyTupleObject *v = (PyTupleObject *)tuple;

    n = Py_SIZE(v);
    if (n == 0)
        return PyBytes_FromString("[]");

    pieces = PyTuple_New(n);
    if (pieces == NULL)
        return NULL;

    /* Do repr() on each element. */
    for (i = 0; i < n; ++i) {
        s = encode_object(v->ob_item[i]);
        if (s == NULL)
            goto Done;
        PyTuple_SET_ITEM(pieces, i, s);
    }

    /* Add "[]" decorations to the first and last items. */
    assert(n > 0);
    s = PyBytes_FromString("[");
    if (s == NULL)
        goto Done;
    temp = PyTuple_GET_ITEM(pieces, 0);
    PyBytes_ConcatAndDel(&s, temp);
    PyTuple_SET_ITEM(pieces, 0, s);
    if (s == NULL)
        goto Done;

    s = PyBytes_FromString("]");
    if (s == NULL)
        goto Done;
    temp = PyTuple_GET_ITEM(pieces, n-1);
    PyBytes_ConcatAndDel(&temp, s);
    PyTuple_SET_ITEM(pieces, n-1, temp);
    if (temp == NULL)
        goto Done;

    /* Paste them all together with ", " between. */
    s = PyBytes_FromString(", ");
    if (s == NULL)
        goto Done;
    result = _PyBytes_Join(s, pieces);
    Py_DECREF(s);

Done:
    Py_DECREF(pieces);
    return result;
}

/*
 * This function is an almost verbatim copy of list_repr() from
 * Python's listobject.c with the following differences:
 *
 * - it uses encode_object() to get the object's JSON reprezentation.
 * - it doesn't use the ellipsis to represent a list with references
 *   to itself, instead it raises an exception as such lists cannot be
 *   represented in JSON.
 */
static PyObject *encode_list(PyObject *list) {
    Py_ssize_t i;
    PyObject *s, *temp;
    PyObject *pieces = NULL, *result = NULL;
    PyListObject *v = (PyListObject *)list;

    i = Py_ReprEnter((PyObject *)v);
    if (i != 0) {
        if (i > 0) {
            PyErr_SetString(JSON_EncodeError, "a list with references to itself is not JSON encodable");
        }
        return NULL;
    }

    if (Py_SIZE(v) == 0) {
        result = PyBytes_FromString("[]");
        goto Done;
    }

    pieces = PyList_New(0);
    if (pieces == NULL)
        goto Done;

    /* Do repr() on each element.  Note that this may mutate the list,
     * so must refetch the list size on each iteration. */
    for (i = 0; i < Py_SIZE(v); ++i) {
        int status;
        s = encode_object(v->ob_item[i]);
        if (s == NULL)
            goto Done;
        status = PyList_Append(pieces, s);
        Py_DECREF(s);  /* append created a new ref */
        if (status < 0)
            goto Done;
    }

    /* Add "[]" decorations to the first and last items. */
    assert(PyList_GET_SIZE(pieces) > 0);
    s = PyBytes_FromString("[");
    if (s == NULL)
        goto Done;
    temp = PyList_GET_ITEM(pieces, 0);
    PyBytes_ConcatAndDel(&s, temp);
    PyList_SET_ITEM(pieces, 0, s);
    if (s == NULL)
        goto Done;

    s = PyBytes_FromString("]");
    if (s == NULL)
        goto Done;
    temp = PyList_GET_ITEM(pieces, PyList_GET_SIZE(pieces)-1);
    PyBytes_ConcatAndDel(&temp, s);
    PyList_SET_ITEM(pieces, PyList_GET_SIZE(pieces)-1, temp);
    if (temp == NULL)
        goto Done;

    /* Paste them all together with ", " between. */
    s = PyBytes_FromString(", ");
    if (s == NULL)
        goto Done;
    result = _PyBytes_Join(s, pieces);
    Py_DECREF(s);

Done:
    Py_XDECREF(pieces);
    Py_ReprLeave((PyObject *)v);
    return result;
}

/*
 * This function is an almost verbatim copy of dict_repr() from
 * Python's dictobject.c with the following differences:
 *
 * - it uses encode_object() to get the object's JSON reprezentation.
 * - only accept strings for keys.
 * - it doesn't use the ellipsis to represent a dictionary with references
 *   to itself, instead it raises an exception as such dictionaries cannot
 *   be represented in JSON.
 */
static PyObject *encode_dict(PyObject *dict) {
    Py_ssize_t i;
    PyObject *s, *temp, *colon = NULL;
    PyObject *pieces = NULL, *result = NULL;
    PyObject *key, *value;
    PyDictObject *mp = (PyDictObject *)dict;

    i = Py_ReprEnter((PyObject *)mp);
    if (i != 0) {
        if (i > 0) {
            PyErr_SetString(JSON_EncodeError, "a dict with references to "
                            "itself is not JSON encodable");
        }
        return NULL;
    }

    if (mp->ma_used == 0) {
        result = PyBytes_FromString("{}");
        goto Done;
    }

    pieces = PyList_New(0);
    if (pieces == NULL)
        goto Done;

    colon = PyBytes_FromString(": ");
    if (colon == NULL)
        goto Done;

    /* Do repr() on each key+value pair, and insert ": " between them.
     * Note that repr may mutate the dict. */
    i = 0;
    while (PyDict_Next((PyObject *)mp, &i, &key, &value)) {
        int status;

        if (!PyBytes_Check(key) && !PyUnicode_Check(key)) {
            PyErr_SetString(JSON_EncodeError, "JSON encodable dictionaries must have string/unicode keys");
            goto Done;
        }

        /* Prevent repr from deleting value during key format. */
        Py_INCREF(value);
        s = encode_object(key);
        PyBytes_Concat(&s, colon);
        PyBytes_ConcatAndDel(&s, encode_object(value));
        Py_DECREF(value);
        if (s == NULL)
            goto Done;
        status = PyList_Append(pieces, s);
        Py_DECREF(s);  /* append created a new ref */
        if (status < 0)
            goto Done;
    }

    /* Add "{}" decorations to the first and last items. */
    assert(PyList_GET_SIZE(pieces) > 0);
    s = PyBytes_FromString("{");
    if (s == NULL)
        goto Done;
    temp = PyList_GET_ITEM(pieces, 0);
    PyBytes_ConcatAndDel(&s, temp);
    PyList_SET_ITEM(pieces, 0, s);
    if (s == NULL)
        goto Done;

    s = PyBytes_FromString("}");
    if (s == NULL)
        goto Done;
    temp = PyList_GET_ITEM(pieces, PyList_GET_SIZE(pieces)-1);
    PyBytes_ConcatAndDel(&temp, s);
    PyList_SET_ITEM(pieces, PyList_GET_SIZE(pieces)-1, temp);
    if (temp == NULL)
        goto Done;

    /* Paste them all together with ", " between. */
    s = PyBytes_FromString(", ");
    if (s == NULL)
        goto Done;
    result = _PyBytes_Join(s, pieces);
    Py_DECREF(s);

Done:
    Py_XDECREF(pieces);
    Py_XDECREF(colon);
    Py_ReprLeave((PyObject *)mp);
    return result;
}

static PyObject *encode_object(PyObject *object) {
    if (object == Py_True) {
        return PyBytes_FromString("true");
    } else if (object == Py_False) {
        return PyBytes_FromString("false");
    } else if (object == Py_None) {
        return PyBytes_FromString("null");
    } else if (PyBytes_Check(object)) {
        return encode_string(object);
    } else if (PyUnicode_Check(object)) {
        return encode_unicode(object);
    } else if (PyFloat_Check(object)) {
        double val = PyFloat_AS_DOUBLE(object);
        if (Py_IS_NAN(val)) {
            return PyBytes_FromString("NaN");
        } else if (Py_IS_INFINITY(val)) {
            if (val > 0) {
                return PyBytes_FromString("Infinity");
            } else {
                return PyBytes_FromString("-Infinity");
            }
        } else {
            return cjson_PyObject_Str(object);
        }
    }
#ifdef IS_PY3K
    else if (PyLong_Check(object)) {
        return cjson_PyObject_Str(object);
    }
#else
    else if (PyInt_Check(object) || PyLong_Check(object)) {
        return cjson_PyObject_Str(object);
    }
#endif
    else if (PyList_Check(object)) {
        return encode_list(object);
    } else if (PyTuple_Check(object)) {
        return encode_tuple(object);
    } else if (PyDict_Check(object)) { /* use PyMapping_Check(object) instead? -Dan */
        return encode_dict(object);
    } else {
        PyErr_SetString(JSON_EncodeError, "object is not JSON encodable");
        return NULL;
    }
}

/* Encode object into its JSON representation */

static PyObject *JSON_encode(PyObject *self, PyObject *object) {
    return encode_object(object);
}

/* Decode JSON representation into pyhton objects */

static PyObject *JSON_decode(PyObject *self, PyObject *args, PyObject *kwargs) {
    static char *kwlist[] = { "json", "all_unicode", NULL };
#ifdef IS_PY3K
    int all_unicode = True; /* by default return unicode always */
#else
    int all_unicode = False; /* by default return unicode only when needed */
#endif
    PyObject *object, *string, *str;
    JSONData jsondata;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|i:decode", kwlist, &string, &all_unicode))
        return NULL;

    if (PyUnicode_Check(string)) {
#ifdef IS_PY3K
        /* HACK: Workaround for crash bug in Python3's PyUnicode_AsRawUnicodeEscapeString... */
        str = PyUnicode_EncodeRawUnicodeEscape(PyUnicode_AS_UNICODE(string),
                                               PyUnicode_GET_SIZE(string));
#else
        str = PyUnicode_AsRawUnicodeEscapeString(string);
#endif
        if (str == NULL) {
            return NULL;
        }
    } else {
        Py_INCREF(string);
        str = string;
    }

    if (PyBytes_AsStringAndSize(str, &(jsondata.str), NULL) == -1) {
        Py_DECREF(str);
        return NULL; /* not a string object or it contains null bytes */
    }

    jsondata.ptr = jsondata.str;
    jsondata.end = jsondata.str+strlen(jsondata.str);
    jsondata.all_unicode = all_unicode;

    object = decode_json(&jsondata);

    if (object != NULL) {
        skipSpaces(&jsondata);
        if (jsondata.ptr < jsondata.end) {
            PyErr_Format(JSON_DecodeError, "extra data after JSON description at position "SSIZE_T_F, (Py_ssize_t)(jsondata.ptr-jsondata.str));
            Py_DECREF(str);
            Py_DECREF(object);
            return NULL;
        }
    }

    Py_DECREF(str);

    return object;
}

/* List of functions defined in the module */

static PyMethodDef cjson_methods[] = {
    { "encode", (PyCFunction)JSON_encode,  METH_O,
    PyDoc_STR("encode(object) -> generate the JSON representation for object.") },

/* On Python 3.x we normally want Unicode. */
    { "decode", (PyCFunction)JSON_decode,  METH_VARARGS|METH_KEYWORDS,
#ifdef IS_PY3K
    PyDoc_STR("decode(string, all_unicode=True) -> parse the JSON representation into\n"
              "python objects. The optional argument `all_unicode', specifies how to\n"
              "convert the strings in the JSON representation into python objects.\n"
              "If it is False (default on Python 2.x), it will return strings/bytes\n"
              "everywhere possible and unicode objects only where necessary, else\n"
              "it will return unicode objects everywhere (this is slower, but default\n"
              "on Python 3.x).")
#else
    PyDoc_STR("decode(string, all_unicode=False) -> parse the JSON representation into\n"
              "python objects. The optional argument `all_unicode', specifies how to\n"
              "convert the strings in the JSON representation into python objects.\n"
              "If it is False (default on Python 2.x), it will return strings/bytes\n"
              "everywhere possible and unicode objects only where necessary, else\n"
              "it will return unicode objects everywhere (this is slower, but default\n"
              "on Python 3.x).")
#endif
    },
    { NULL, NULL, 0, NULL }  /* sentinel */
};

PyDoc_STRVAR(module_doc, "Fast JSON encoder/decoder module.");

#define MODULE_VERSION "1.0.5"

static void initcjson_shared(PyObject *m) {
    JSON_Error = PyErr_NewException("cjson.Error", NULL, NULL);
    if (JSON_Error == NULL)
        return;
    Py_INCREF(JSON_Error);
    PyModule_AddObject(m, "Error", JSON_Error);

    JSON_EncodeError = PyErr_NewException("cjson.EncodeError", JSON_Error, NULL);
    if (JSON_EncodeError == NULL)
        return;
    Py_INCREF(JSON_EncodeError);
    PyModule_AddObject(m, "EncodeError", JSON_EncodeError);

    JSON_DecodeError = PyErr_NewException("cjson.DecodeError", JSON_Error, NULL);
    if (JSON_DecodeError == NULL)
        return;
    Py_INCREF(JSON_DecodeError);
    PyModule_AddObject(m, "DecodeError", JSON_DecodeError);

    /* Module version (the MODULE_VERSION macro is defined by setup.py) */
    PyModule_AddStringConstant(m, "__version__", MODULE_VERSION);
}


#ifdef IS_PY3K
static PyModuleDef cjsonModule = {
    PyModuleDef_HEAD_INIT,
    "cjson",           /* m_name     */
    module_doc,        /* m_doc      */
    -1,                /* m_size     */
    cjson_methods,     /* m_methods  */
    NULL,              /* m_reload   */
    NULL,              /* m_traverse */
    NULL,              /* m_clear    */
    NULL               /* m_free     */
};

PyObject* PyInit_cjson(void)
{
    PyObject *m;

    m = PyModule_Create(&cjsonModule);

    if (m == NULL)
        return NULL;

    initcjson_shared(m);

    return m;
}
#else
/* Initialization function for the module (*must *be called initcjson) */
PyMODINIT_FUNC initcjson(void) {
    PyObject *m;

    m = Py_InitModule3("cjson", cjson_methods, module_doc);

    if (m == NULL)
        return;

    initcjson_shared(m);
}
#endif
