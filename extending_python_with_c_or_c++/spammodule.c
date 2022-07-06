#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdlib.h>

static PyObject* spam_system(PyObject* self, PyObject* args) {
    char const* command;
    int sts;

    if (!PyArg_ParseTuple(args, "s", &command)) {
        return NULL;
    }
    sts = system(command);
    return PyLong_FromLong(sts);
}

static PyMethodDef SpamMethods[] = {
    {"system", spam_system, METH_VARARGS, "Execute a shell command"},
    {NULL, NULL, 0, NULL}  // Sentinel
};

static PyModuleDef spammodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "spam",        // name of module
    .m_doc = "spam module",  // module documentation, may be NULL
    .m_size = -1,  // size of per-interpreter state of the module, or -1 if
                   // the module keeps state in global variables
    .m_methods = SpamMethods,
};

PyMODINIT_FUNC PyInit_spam() { return PyModule_Create(&spammodule); }