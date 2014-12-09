cdef extern from "hook.h":
    void* hook(char* library_filename, char* function_name, void* substitution_address)

cdef extern from "libgreenify.h":
    struct sockaddr:
        pass
    struct msghdr:
        pass
    ctypedef unsigned long socklen_t
    int green_connect(int socket, const sockaddr *address, socklen_t address_len)
    ssize_t green_read(int fildes, void *buf, size_t nbyte)
    ssize_t green_write(int fildes, void *buf, size_t nbyte)
    ssize_t green_recv(int socket, void *buffer, size_t length, int flags)
    ssize_t green_send(int socket, void *buffer, size_t length, int flags)
    ssize_t green_sendmsg(int socket, const msghdr* message, int flags)
    struct fd_set:
        pass
    struct timeval:
        pass
    int green_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, timeval *timeout)
    struct pollfd:
        pass
    ctypedef unsigned long nfds_t
    int green_poll(pollfd *fds, nfds_t nfds, int timeout)

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
            wait(watchers_list)
            return 0
        except Timeout:
            return -1
        finally:
            t.cancel()
    else:
        wait(watchers_list)
        return 0

def greenify():
    greenify_set_wait_callback(wait_gevent)

def wait(watchers):
    waiter = Waiter()
    switch = waiter.switch
    unique = object()
    try:
        count = len(watchers)
        for watcher in watchers:
            watcher.start(switch, unique)
        result = waiter.get()
        assert result is unique, 'Invalid switch into %s: %r' % (getcurrent(), result)
        waiter.clear()
        return result
    finally:
        for watcher in watchers:
            watcher.stop()

cpdef patch_lib(bytes library_path):
    cdef char* path = library_path
    cdef bint result = False
    if NULL != hook(path, "connect", <void*>green_connect):
        result = True
    if NULL != hook(path, "read", <void*>green_read):
        result = True
    if NULL != hook(path, "write", <void*>green_write):
        result = True
    if NULL != hook(path, "recv", <void*>green_recv):
        result = True
    if NULL != hook(path, "send", <void*>green_send):
        result = True
    if NULL != hook(path, "sendmsg", <void*>green_sendmsg):
        result = True
    if NULL != hook(path, "select", <void*>green_select):
        result = True
    if NULL != hook(path, "poll", <void*>green_poll):
        result = True

    return result
