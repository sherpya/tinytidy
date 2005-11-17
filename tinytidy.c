/*
 * Python Tiny TidyLib wrapper version 0.1
 * 
 * Inspired from http://utidylib.berlios.de/
 * but implements only parseString methods.
 * It can be used with python >= 2.1
 * Copyright (c) 2005 Gianluigi Tiesi <sherpya@netfarm.it>
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
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <Python.h>
#include <tidy/tidy.h>
#include <tidy/buffio.h>

#ifndef PyMODINIT_FUNC
#define PyMODINIT_FUNC void
#endif

#define TDOC_RETURN() \
{ \
    Py_DECREF(item); \
    Py_DECREF(value); \
    tidyBufFree(&errbuf); \
    tidyRelease(tdoc); \
    return res; \
}

#define TIDY_SET_PYERR() { PyErr_SetString(PyExc_ValueError, (const char *) errbuf.bp); }

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
            {
                if (!PyString_Check(value))
                {
                    PyErr_Format(PyExc_ValueError, "Option '%s' must be a String", PyString_AsString(item));
                    TDOC_RETURN();

                }
                if (!tidyOptSetValue(tdoc, tidyOptGetId(option), PyString_AsString(value)))
                {
                    TIDY_SET_PYERR();
                    TDOC_RETURN();
                }
                break;
            }
            case TidyInteger:
            {
                if (!PyInt_Check(value))
                {
                    PyErr_Format(PyExc_ValueError, "Option '%s' must be an Integer", PyString_AsString(item));
                    TDOC_RETURN();
                }

                if (!tidyOptSetInt(tdoc, tidyOptGetId(option), PyInt_AsLong(value)))
                {
                    TIDY_SET_PYERR();
                    TDOC_RETURN();
                }
                break;
            }

            case TidyBoolean:
            {
                if (!PyInt_Check(value))
                {
                    PyErr_Format(PyExc_ValueError, "Option '%s' must be a Boolean or integer",
                                 PyString_AsString(item));
                    TDOC_RETURN();
                }
                if (!tidyOptSetBool(tdoc, tidyOptGetId(option), PyInt_AsLong(value)))
                {
                    TIDY_SET_PYERR();
                    TDOC_RETURN();
                }
                break;
            }
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

    Py_INCREF(res);
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
