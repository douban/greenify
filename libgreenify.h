#ifndef _GREENIFY_H
#define _GREENIFY_H

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/socket.h>

int green_connect(int socket, const struct sockaddr *address, socklen_t address_len);
ssize_t green_read(int fildes, void *buf, size_t nbyte);
ssize_t green_write(int fildes, const void *buf, size_t nbyte);
ssize_t green_recv(int socket, void *buffer, size_t length, int flags);
ssize_t green_send(int socket, const void *buffer, size_t length, int flags);

typedef int (*greenify_wait_callback_func_t) (int fd, int event);
void greenify_set_wait_callback(greenify_wait_callback_func_t callback);

#endif
