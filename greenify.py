from ctypes import cdll, CFUNCTYPE, c_int
from ctypes.util import find_library

from gevent.hub import get_hub

libgreenify = cdll.LoadLibrary(find_library('greenify'))
callback_func_t = CFUNCTYPE(c_int, c_int, c_int)

@callback_func_t
def wait_gevent(fd, event):
    hub = get_hub()
    watcher = hub.loop.io(fd, event)
    hub.wait(watcher)

def greenify():
    libgreenify.greenify_set_wait_callback(wait_gevent)
