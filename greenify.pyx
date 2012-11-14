cdef extern from "libgreenify.h":
    struct greenify_watcher:
        int fd
        int events
    ctypedef int (*greenify_wait_callback_func_t) (greenify_watcher* watchers, int nwatchers, int timeout)
    cdef void greenify_set_wait_callback(greenify_wait_callback_func_t callback)

from gevent.hub import get_hub, getcurrent, Waiter
from gevent.timeout import Timeout

cdef int wait_gevent(greenify_watcher* watchers, int nwatchers, int timeout_in_ms) with gil:
    cdef int fd, event
    cdef float timeout_in_s
    cdef int i

    hub = get_hub()
    watchers_list = []
    for i in range(nwatchers):
        fd = watchers[i].fd;
        event = watchers[i].events;
        watcher = hub.loop.io(fd, event)
        watchers_list.append(watcher)

    if timeout_in_ms != 0:
        timeout_in_s = timeout_in_ms / 1000.0
        t = Timeout.start_new(timeout_in_s)
        try:
            for item in wait(watchers_list):
                return 0
        except Timeout:
            return -1
        finally:
            t.cancel()

    else:
        for item in wait(watchers_list):
            return 0

def greenify():
    greenify_set_wait_callback(wait_gevent)

def wait(watchers):
    waiter = Waiter()
    switch = waiter.switch
    objs = []
    try:
        count = len(watchers)
        for watcher in watchers:
            obj = object()
            watcher.start(switch, obj)
            objs.append(obj)

        result = waiter.get()
        assert result in objs, 'Invalid switch into %s: %r' % (getcurrent(), result)
        waiter.clear()
        yield result
    finally:
        for watcher in watchers:
            watcher.stop()
