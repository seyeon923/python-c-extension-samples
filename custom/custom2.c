#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"

#include <stddef.h>

typedef struct {
    // clang-format off
    PyObject_HEAD
    PyObject* first;  // first name
    // clang-format on
    PyObject* last;  // last name
    int number;
} CustomObject;

static void Custom_dealloc(CustomObject* self) {
    Py_XDECREF(self->first);
    Py_XDECREF(self->last);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

// Custom.__new__
static PyObject* Custom_new(PyTypeObject* type, PyObject* args,
                            PyObject* kwargs) {
    CustomObject* self;
    self = (CustomObject*)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->first = PyUnicode_FromString("");
        if (self->first == NULL) {
            Py_DECREF(self);
            return NULL;
        }
        self->last = PyUnicode_FromString("");
        if (self->last == NULL) {
            Py_DECREF(self);
            return NULL;
        }
        self->number = 0;
    }
    return (PyObject*)self;
}

// Custom.__init__
// return 0 on success, -1 on error
static int Custom_init(CustomObject* self, PyObject* args, PyObject* kwargs) {
    static char* kwlist[] = {"first", "last", "number", NULL};
    PyObject* first = NULL;
    PyObject* last = NULL;
    PyObject* tmp;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|OOi", kwlist, &first,
                                     &last, &self->number)) {
        return -1;
    }

    if (first) {
        tmp = self->first;
        Py_INCREF(first);
        self->first = first;
        Py_XDECREF(tmp);
    }
    if (last) {
        tmp = self->last;
        Py_INCREF(last);
        self->last = last;
        Py_XDECREF(tmp);
    }
    return 0;
}

static PyMemberDef Custom_members[] = {
    {"first", T_OBJECT_EX, offsetof(CustomObject, first), 0, "first name"},
    {"last", T_OBJECT_EX, offsetof(CustomObject, last), 0, "last name"},
    {"number", T_INT, offsetof(CustomObject, number), 0, "custom number"},
    {NULL}  // Sentienl
};

// Custom.name
static PyObject* Custom_name(CustomObject* self, PyObject* Py_UNUSED(ignore)) {
    if (self->first == NULL) {
        PyErr_SetString(PyExc_AttributeError, "No 'first' attritbute");
        return NULL;
    }
    if (self->last == NULL) {
        PyErr_SetString(PyExc_AttributeError, "No 'last' attribute");
        return NULL;
    }
    return PyUnicode_FromFormat("%S %S", self->first, self->last);
}

static PyMethodDef Custom_methods[] = {
    {"name", (PyCFunction)Custom_name, METH_NOARGS,
     "Retrun the name, combining the first and last name"},
    {NULL}  // Sentinel
};

static PyTypeObject CustomType = {
    // clang-format off
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "custom2.Custom",
    // clang-format on
    .tp_doc = PyDoc_STR("Custom objects"),
    .tp_basicsize = sizeof(CustomObject),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = Custom_new,
    .tp_init = (initproc)Custom_init,
    .tp_dealloc = (destructor)Custom_dealloc,
    .tp_members = Custom_members,
    .tp_methods = Custom_methods,
};

static PyModuleDef custommodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "custom2",
    .m_doc = "Example module that creates an extension type",
    .m_size = -1,
};

PyMODINIT_FUNC PyInit_custom2() {
    PyObject* m;
    if (PyType_Ready(&CustomType) < 0) {
        return NULL;
    }

    m = PyModule_Create(&custommodule);
    if (m == NULL) {
        return NULL;
    }

    Py_INCREF(&CustomType);
    if (PyModule_AddObject(m, "Custom", (PyObject*)&CustomType) < 0) {
        Py_DECREF(&CustomType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}
