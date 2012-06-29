cdef extern from "libgreenify.h":
    struct greenify_watcher:
        int fd
        int events
    ctypedef int (*greenify_wait_callback_func_t) (greenify_watcher* watchers, int nwatchers, int timeout)
    cdef void greenify_set_wait_callback(greenify_wait_callback_func_t callback)

from gevent.hub import get_hub
from gevent.timeout import Timeout

cdef int wait_gevent(greenify_watcher* watchers, int nwatchers, int timeout_in_ms) with gil:
    cdef int fd, event
    cdef float timeout_in_s

    assert nwatchers == 1

    hub = get_hub()
    fd = watchers[0].fd;
    event = watchers[0].events;
    watcher = hub.loop.io(fd, event)

    if timeout_in_ms != 0:
        timeout_in_s = timeout_in_ms / 1000.0
        t = Timeout.start_new(timeout_in_s)
        try:
            hub.wait(watcher)
            return 0
        except Timeout:
            return -1
        finally:
            t.cancel()

    else:
        hub.wait(watcher)
        return 0

def greenify():
    greenify_set_wait_callback(wait_gevent)
