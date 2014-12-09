#ifndef _GREENIFY_H
#define _GREENIFY_H

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#ifndef NO_POLL
#include <poll.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

int green_connect(int socket, const struct sockaddr *address, socklen_t address_len);
ssize_t green_read(int fildes, void *buf, size_t nbyte);
ssize_t green_write(int fildes, const void *buf, size_t nbyte);
ssize_t green_recv(int socket, void *buffer, size_t length, int flags);
ssize_t green_send(int socket, const void *buffer, size_t length, int flags);
int green_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
#ifndef NO_POLL
int green_poll(struct pollfd *fds, nfds_t nfds, int timeout);
#endif

struct greenify_watcher {
	int fd;
	int events;
};

/* return 0 for events occurred, -1 for timeout.
 * timeout in milliseconds */
typedef int (*greenify_wait_callback_func_t) (struct greenify_watcher watchers[], int nwatchers, int timeout);

void greenify_set_wait_callback(greenify_wait_callback_func_t callback);

#ifdef __cplusplus
}
#endif

#endif
