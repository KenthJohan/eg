#pragma once


#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>


int do_bind_socket(char const * host, uint16_t port);

int net_accept(int sock, int timeout_ms);

void net_get_address(int sock, char ip[1025], char port[32]);

void net_close_socket(int fd);

void net_listen(int sock, int n);

ssize_t net_send(int fd, const void *buf, size_t n, int flags);
ssize_t net_send_all(int fd, const void *buf, size_t len, int flags);

ssize_t net_recv(int fd, void *buf, size_t n, int flags);