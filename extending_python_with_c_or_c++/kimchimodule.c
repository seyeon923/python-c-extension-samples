#define PY_SSIZE_T_CLEAN
#include <Python.h>

static PyObject* kimchi_system(PyObject* self, PyObject* args) {
    char const* command;
    int sts;

    if (!PyArg_ParseTuple(args, "s", &command)) {
        return NULL;
    }
    sts = system(command);
    return PyLong_FromLong(sts);
}

static PyMethodDef KimchiMethods[] = {
    {"system", kimchi_system, METH_VARARGS, "Execute a shell command"},
    {NULL, NULL, 0, NULL}  // Sentinel
};

static PyModuleDef kimchimodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "kimchi",        // name of module
    .m_doc = "kimchi module",  // module documentation, may be NULL
    .m_size = -1,  // size of per-interpreter state of the module, or -1 if
                   // the module keeps state in global variables
    .m_methods = KimchiMethods,
};

PyMODINIT_FUNC PyInit_kimchi() { return PyModule_Create(&kimchimodule); }