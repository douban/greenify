#include <sys/socket.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include "libgreenify.h"

#define EVENT_READ 1
#define EVENT_WRITE 2

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

int
connect(int socket, const struct sockaddr *address, socklen_t address_len)
{
	int flags_changed, flags, s_err;
	ssize_t retval;

	if (g_wait_callback == NULL || !set_nonblock(socket, &flags))
		return connect(socket, address, address_len);

	do {
		retval = connect(socket, address, address_len);
		s_err = errno;
	} while(retval < 0 && (s_err == EWOULDBLOCK || s_err == EALREADY || s_err == EINPROGRESS)
			&& !(retval = g_wait_callback(socket, EVENT_READ)));

	restore_flags(socket, flags);
	errno = s_err;
	return retval;
}

ssize_t
green_read(int fildes, void *buf, size_t nbyte)
{
	int flags_changed, flags, s_err;
	ssize_t retval;

	if (g_wait_callback == NULL || !set_nonblock(fildes, &flags))
		return read(fildes, buf, nbyte);

	do {
		retval = read(fildes, buf, nbyte);
		s_err = errno;
	} while(retval < 0 && (s_err == EWOULDBLOCK || s_err == EAGAIN)
			&& !(retval = g_wait_callback(fildes, EVENT_READ)));

	restore_flags(fildes, flags);
	errno = s_err;
	return retval;
}

ssize_t
green_write(int fildes, const void *buf, size_t nbyte)
{
	int flags, flags_changed, s_err;
	ssize_t retval;

	if (g_wait_callback == NULL || !set_nonblock(fildes, &flags))
		return write(fildes, buf, nbyte);

	do {
		retval = write(fildes, buf, nbyte);
		s_err = errno;
	} while(retval < 0 && (s_err == EWOULDBLOCK || s_err == EAGAIN)
			&& !(retval = g_wait_callback(fildes, EVENT_WRITE)));

	restore_flags(fildes, flags);
	errno = s_err;
	return retval;
}

ssize_t
green_recv(int socket, void *buffer, size_t length, int flags)
{
	int sock_flags, sock_flags_changed, s_err;
	ssize_t retval;

	if (g_wait_callback == NULL || !set_nonblock(socket, &sock_flags))
		return recv(socket, buffer, length, flags);

	do {
		retval = recv(socket, buffer, length, flags);
		s_err = errno;
	} while(retval < 0 && (s_err == EWOULDBLOCK || s_err == EAGAIN)
			&& !(retval = g_wait_callback(socket, EVENT_READ)));

	restore_flags(socket, sock_flags);
	errno = s_err;
	return retval;
}

ssize_t
green_send(int socket, const void *buffer, size_t length, int flags)
{
	int sock_flags, sock_flags_changed, s_err;
	ssize_t retval;

	if (g_wait_callback == NULL || !set_nonblock(socket, &sock_flags))
		return send(socket, buffer, length, flags);

	do {
		retval = send(socket, buffer, length, flags);
		s_err = errno;
	} while(retval < 0 && (s_err == EWOULDBLOCK || s_err == EAGAIN)
			&& !(retval = g_wait_callback(socket, EVENT_WRITE)));

	restore_flags(socket, sock_flags);
	errno = s_err;
	return retval;
}

