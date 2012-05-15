#include <Python.h>
#include <libgreenify.h>

static PyObject *get_hub = NULL;

static int
wait_gevent(int fd, int event)
{
	PyGILState_STATE gstate;
	PyObject *hub=NULL, *loop=NULL, *io=NULL, *watcher=NULL;
	int retval = -1;

	gstate = PyGILState_Ensure();

	/* hub = get_hub()
	 * watcher = hub.loop.io(fd, event)
	 * hub.wait(watcher)
	 */

	if (!(hub = PyObject_CallObject(get_hub, NULL)))
		goto error;
	if (!(loop = PyObject_GetAttrString(hub, "loop")))
		goto error;
	if (!(io = PyObject_GetAttrString(loop, "io")))
		goto error;
	if (!(watcher = PyObject_CallFunction(io, "ii", fd, event)))
		goto error;
	if (!(PyObject_CallMethod(hub, "wait", "O", watcher)))
		goto error;

	retval = 0;
	goto exit;

error:
	PyErr_Print();

exit:
	Py_XDECREF(watcher);
	Py_XDECREF(io);
	Py_XDECREF(loop);
	Py_XDECREF(hub);

	PyGILState_Release(gstate);
	return retval;
}

static PyObject *
greenify(PyObject *self, PyObject *args)
{
	greenify_set_wait_callback(wait_gevent);
	Py_RETURN_NONE;
}

static PyMethodDef GreenifyMethods[] = {
	{"greenify", greenify, METH_VARARGS, "set greenify callback"},
	{NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
initgreenify(void)
{
	PyObject *m;

	m = Py_InitModule("greenify", GreenifyMethods);
	if (m == NULL)
		return;

	/* from gevent.hub import get_hub */
	if (!(m = PyImport_ImportModule("gevent.hub"))) {
		PyErr_Print();
		return;
	}
	if (!(get_hub = PyObject_GetAttrString(m, "get_hub"))) {
		PyErr_Print();
		Py_DECREF(m);
		return;
	}

	Py_DECREF(m);
}
