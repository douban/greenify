#include <sys/socket.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>
#include <time.h>

#ifdef DEBUG
#include <unistd.h>
#define debug(...) \
	do { \
		time_t now = time(NULL); \
		char t[30]; \
		ctime_r(&now, t); \
		t[24] = '\0'; \
		fprintf(stderr, "[greenify] [%s] [%d] ", t, getpid()); \
		fprintf(stderr, __VA_ARGS__); \
	} while(0);
#else  /* #define DEBUG */
#define debug(...)
#endif /* #define DEBUG */

#include "libgreenify.h"

#define EVENT_READ 0x01
#define EVENT_WRITE 0x02

static greenify_wait_callback_func_t g_wait_callback = NULL;

/* return 1 means the flags changed */
static int
set_nonblock(int fd, int *old_flags)
{
    *old_flags = fcntl(fd, F_GETFL, 0);
    if ((*old_flags) & O_NONBLOCK) {
        return 0;
    } else {
        fcntl(fd, F_SETFL, *old_flags | O_NONBLOCK);
        return 1;
    }
}

static void
restore_flags(int fd, int flags)
{
    fcntl(fd, F_SETFL, flags);
}

void greenify_set_wait_callback(greenify_wait_callback_func_t callback)
{
    g_wait_callback = callback;
}

int callback_multiple_watchers(struct greenify_watcher* watchers, int nwatchers, int timeout)
{
    int retval;
    assert(g_wait_callback != NULL);
    retval = g_wait_callback(watchers, nwatchers, timeout);
    return retval;
}

int callback_single_watcher(int fd, int events, int timeout)
{
    struct greenify_watcher watchers[1];
    int retval;

    assert(g_wait_callback != NULL);

    watchers[0].fd = fd;
    watchers[0].events = events;
    retval = g_wait_callback(watchers, 1, timeout);
    return retval;
}

int
green_connect(int socket, const struct sockaddr *address, socklen_t address_len)
{
    int flags_changed, flags, s_err, retval;

    debug("Enter green_connect\n");

    if (g_wait_callback == NULL || !set_nonblock(socket, &flags)) {
        retval = connect(socket, address, address_len);
        return retval;
    }

    retval = connect(socket, address, address_len);
    s_err = errno;
    if (retval < 0 && (s_err == EWOULDBLOCK || s_err == EALREADY || s_err == EINPROGRESS)) {
        callback_single_watcher(socket, EVENT_READ, 0);
        getsockopt(socket, SOL_SOCKET, SO_ERROR, &s_err, &address_len);
        retval = s_err ? -1 : 0;
    }

    restore_flags(socket, flags);
    errno = s_err;
    return retval;
}

ssize_t
green_read(int fildes, void *buf, size_t nbyte)
{
    int flags_changed, flags, s_err;
    ssize_t retval;

    debug("Enter green_read\n");

    if (g_wait_callback == NULL || !set_nonblock(fildes, &flags))
        return read(fildes, buf, nbyte);

    do {
        retval = read(fildes, buf, nbyte);
        s_err = errno;
    } while(retval < 0 && (s_err == EWOULDBLOCK || s_err == EAGAIN)
            && !(retval = callback_single_watcher(fildes, EVENT_READ, 0)));

    restore_flags(fildes, flags);
    errno = s_err;
    return retval;
}

ssize_t
green_write(int fildes, const void *buf, size_t nbyte)
{
    int flags, flags_changed, s_err;
    ssize_t retval;

    debug("Enter green_write\n");

    if (g_wait_callback == NULL || !set_nonblock(fildes, &flags))
        return write(fildes, buf, nbyte);

    do {
        retval = write(fildes, buf, nbyte);
        s_err = errno;
	debug("write %zuB@%p to fd %d, return %zu, errno %d\n",
			nbyte, buf, fildes, retval, s_err);
    } while(retval < 0 && (s_err == EWOULDBLOCK || s_err == EAGAIN)
            && !(retval = callback_single_watcher(fildes, EVENT_WRITE, 0)));

    restore_flags(fildes, flags);
    errno = s_err;
    return retval;
}

ssize_t
green_recv(int socket, void *buffer, size_t length, int flags)
{
    int sock_flags, sock_flags_changed, s_err;
    ssize_t retval;

    debug("Enter green_recv\n");

    if (g_wait_callback == NULL || !set_nonblock(socket, &sock_flags))
        return recv(socket, buffer, length, flags);

    do {
        retval = recv(socket, buffer, length, flags);
        s_err = errno;
    } while(retval < 0 && (s_err == EWOULDBLOCK || s_err == EAGAIN)
            && !(retval = callback_single_watcher(socket, EVENT_READ, 0)));

    restore_flags(socket, sock_flags);
    errno = s_err;
    return retval;
}

ssize_t
green_send(int socket, const void *buffer, size_t length, int flags)
{
    int sock_flags, sock_flags_changed, s_err;
    ssize_t retval;

    debug("Enter green_send\n");

    if (g_wait_callback == NULL || !set_nonblock(socket, &sock_flags))
        return send(socket, buffer, length, flags);

    do {
        retval = send(socket, buffer, length, flags);
        s_err = errno;
	debug("send %zuB@%p to fd %d, return %zu, errno %d\n",
			length, buffer, socket, retval, s_err);
    } while(retval < 0 && (s_err == EWOULDBLOCK || s_err == EAGAIN)
            && !(retval = callback_single_watcher(socket, EVENT_WRITE, 0)));

    restore_flags(socket, sock_flags);
    errno = s_err;
    return retval;
}

int
green_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
    struct greenify_watcher watchers[nfds];
    int count = 0, i = 0;

    debug("Enter green_select\n");
    if (g_wait_callback == NULL)
        return select(nfds, readfds, writefds, exceptfds, timeout);

    for (i = 0; i < nfds; ++i) {
        if (FD_ISSET(i, readfds) || FD_ISSET(i, exceptfds)) {
            watchers[count].fd = i;
            watchers[count].events = EVENT_READ;
            count++;
        }
        if (FD_ISSET(i, writefds)) {
            watchers[count].fd = i;
            watchers[count].events = EVENT_WRITE;
            count++;
        }
    }

    float timeout_in_ms = timeout->tv_usec / 1000.0;
    callback_multiple_watchers(watchers, count, timeout_in_ms);
    return select(nfds, readfds, writefds, exceptfds, timeout);
}

#if defined(HAVE_POLL) || defined(HAVE_POLL_H)
int
green_poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
    int retval;
    nfds_t i;
    struct greenify_watcher watchers[nfds];

    debug("Enter green_poll\n");

    if (g_wait_callback == NULL || timeout == 0)
        return poll(fds, nfds, timeout);

    for (i = 0; i < nfds; i++) {
        if (fds[i].events & ~(POLLIN | POLLPRI | POLLOUT)) {
            fprintf(stderr, "[greenify] support POLLIN|POLLPRI|POLLOUT only, got 0x%x, may block.\n",
                    fds[i].events);
            return poll(fds, nfds, timeout);
        }

        watchers[i].fd = fds[i].fd;
        watchers[i].events = 0;

        if (fds[i].events & POLLIN || fds[i].events & POLLPRI) {
            watchers[i].events |= EVENT_READ;
        }

        if (fds[i].events & POLLOUT) {
            watchers[i].events |= EVENT_WRITE;
        }
    }

    callback_multiple_watchers(watchers, nfds, timeout);
    return poll(fds, nfds, 0);
}
#endif
