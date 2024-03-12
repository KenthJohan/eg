#pragma once


#include <stdint.h>

int do_bind_socket(char const * host, uint16_t port);

int net_accept(int sock, int timeout_ms);

void net_get_address(int sock, char ip[1025], char port[32]);