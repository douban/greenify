#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>


int c_http_head(const char *host, const char* port)
{
    struct addrinfo *p, hints, *res;
    char request[64];
    char response[2049];
    int socket_fd;

    sprintf(request, "HEAD / HTTP/1.1\nhost: %s\n\n", host);

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    getaddrinfo(host, port, &hints, &res);
    for(p = res; p; p = p->ai_next)
    {
        socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (connect(socket_fd, p->ai_addr, p->ai_addrlen) == 0) {
            // fprintf(stderr, "[%s] connected %s\n", __FUNCTION__, host);
            break;
        }
    }

    send(socket_fd, request, strlen(request), 0);
    recv(socket_fd, response, 2048, 0);
    close(socket_fd);
    // fprintf(stderr, "[%s] closed %s\n", __FUNCTION__, host);
    // fprintf(stderr, "%s\n", response);
    if (strstr(response, "HTTP/1.") != NULL) {  // HTTP/1.1 or HTTP/1.0
        return 1;
    } else {
      fprintf(stderr, "[%s] unexpected response: %s\n", __FUNCTION__, response);
    }
    return 0;
}


static PyObject*
http_head(PyObject* self, PyObject* args)
{
    const char* host, *port;
    if (!PyArg_ParseTuple(args, "ss", &host, &port))
    {
        return NULL;
    }
    long ret = c_http_head(host, port);
#if PY_MAJOR_VERSION >= 3
    return PyLong_FromLong(ret);
#else
    return PyInt_FromLong(ret);
#endif
}


static PyMethodDef ModHttpHeadMethods[] =
{
    {"http_head", http_head, METH_VARARGS, "A naive http head test."},
    {NULL, NULL}
};

struct module_state {
    PyObject *error;
};

#if PY_MAJOR_VERSION >= 3
#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))
#else
#define GETSTATE(m) (&_state)
static struct module_state _state;
#endif


#if PY_MAJOR_VERSION >= 3
static int myextension_traverse(PyObject *m, visitproc visit, void *arg) {
    Py_VISIT(GETSTATE(m)->error);
    return 0;
}

static int myextension_clear(PyObject *m) {
    Py_CLEAR(GETSTATE(m)->error);
    return 0;
}

    static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "mod_http_head",     /* m_name */
        "for test greenify",  /* m_doc */
        sizeof(struct module_state),                  /* m_size */
        ModHttpHeadMethods,  /* m_methods */
        NULL,                /* m_reload */
        myextension_traverse,                /* m_traverse */
        myextension_clear,                /* m_clear */
        NULL,                /* m_free */
    };
    #define INITERROR return NULL
#else
    #define INITERROR return
#endif

#if PY_MAJOR_VERSION >=3
PyObject *
PyInit_mod_http_head(void)
#else
PyMODINIT_FUNC
initmod_http_head(void)
#endif
{
#if PY_MAJOR_VERSION >= 3
    PyObject *module = PyModule_Create(&moduledef);
#else
    PyObject *module = (void) Py_InitModule("mod_http_head", ModHttpHeadMethods);
#endif

    if (module == NULL)
        INITERROR;
    struct module_state *st = GETSTATE(module);

    st->error = PyErr_NewException("myextension.Error", NULL, NULL);
    if (st->error == NULL) {
        Py_DECREF(module);
        INITERROR;
    }

#if PY_MAJOR_VERSION >= 3
    return module;
#endif
}
