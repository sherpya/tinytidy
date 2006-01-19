/*
 * Python TinyTidy: a minimal TidyLib wrapper version 0.1
 * Copyright (c) 2005-2006 Gianluigi Tiesi <sherpya@netfarm.it>
 * 
 * Inspired by http://utidylib.berlios.de/
 * but only parseString method implemented.
 * It can be used also with python 2.1
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this software; if not, write to the
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <Python.h>
#include <tidy/tidy.h>
#include <tidy/buffio.h>

#ifndef PyMODINIT_FUNC
#define PyMODINIT_FUNC void
#endif

/* Yes I like to abuse of gcc preprocessor */
#define TIDY_SET_PYERR() { PyErr_SetString(PyExc_ValueError, (const char *) errbuf.bp); }

#define TDOC_RETURN() \
{ \
    Py_DECREF(item); \
    Py_DECREF(value); \
    tidyBufFree(&errbuf); \
    tidyRelease(tdoc); \
    return res; \
}

#define PY_TO_TIDY(py_check, tidy_type, py_converter, mustbe) \
{ \
    if (!Py##py_check(value)) \
    { \
        PyErr_Format(PyExc_ValueError, "Option '%s' must be " mustbe, PyString_AsString(item)); \
        TDOC_RETURN(); \
    } \
    if (!tidyOptSet##tidy_type(tdoc, tidyOptGetId(option), Py##py_converter(value))) \
    { \
        TIDY_SET_PYERR(); \
        TDOC_RETURN(); \
    } \
}

static PyObject *TinyTidyError;

static PyObject *parseString(PyObject *self, PyObject *args)
{
    char *cp;
    int i, len, list_size;
    TidyDoc tdoc;
    TidyOption option = TidyUnknownOption;
    PyObject *res = NULL, *arglist = NULL;
    PyObject *key_list = NULL, *item = NULL, *value = NULL;
    TidyBuffer output = {0};
    TidyBuffer errbuf = {0};

    if (!PyArg_ParseTuple(args, "s#|O", &cp, &len, &arglist))
        return NULL;

    if (arglist && !PyDict_Check(arglist))
    {
        PyErr_SetString(PyExc_TypeError, "Second argument must be a dictionary!");
        return NULL;
    }

    tdoc = tidyCreate();
    tidySetErrorBuffer(tdoc, &errbuf);

    if (!arglist) goto im_so_lazy; /* Options not provided */;

    key_list = PyDict_Keys(arglist);
    list_size = PyList_Size(key_list);

    for (i = 0; i < list_size; i++)
    {
        item = PyList_GetItem(key_list, i);
        value = PyDict_GetItem(arglist, item);
        Py_INCREF(item);
        Py_INCREF(value);

        option = tidyGetOptionByName(tdoc, PyString_AsString(item));

        if (option == TidyUnknownOption)
        {
            PyErr_Format(PyExc_KeyError, "Unknown tidy option '%s'", PyString_AsString(item));
            TDOC_RETURN();
        }

        switch (tidyOptGetType(option))
        {
            case TidyString:
                PY_TO_TIDY(String_Check, Value, String_AsString, "a String");
                break;
            case TidyInteger:
                PY_TO_TIDY(Int_Check, Int, Int_AsLong, "an Integer");
                break;
            case TidyBoolean:
                PY_TO_TIDY(Int_Check, Bool, Int_AsLong, "a Boolean or an Integer");
                break;
            default:
            {
                PyErr_Format(PyExc_RuntimeError,
                             "Something strange happened there is no option type %d",
                             tidyOptGetType(option));
                TDOC_RETURN();
            }          
        }
        Py_DECREF(item);
        Py_DECREF(value);
    }

 im_so_lazy:   
    tidyParseString(tdoc, cp);
    tidyCleanAndRepair(tdoc);
    tidySaveBuffer(tdoc, &output);

    res = Py_BuildValue("s#", output.bp, output.size);
    tidyBufFree(&output);
    tidyBufFree(&errbuf);
    tidyRelease(tdoc);
    return res;
}

static PyMethodDef tinytidy_methods[] =
{
    { "parseString", (PyCFunction) parseString, METH_VARARGS, "TinyTidy 4 Python" },
    { NULL, NULL}
};

PyMODINIT_FUNC inittinytidy(void)
{
    PyObject *m;
    m = Py_InitModule("tinytidy", tinytidy_methods);

    TinyTidyError = PyErr_NewException("tinytidy.error", NULL, NULL);
    Py_INCREF(TinyTidyError);
    PyModule_AddObject(m, "error", TinyTidyError);
}
