
// #define _GNU_SOURCE /* To get defns of NI_MAXSERV and NI_MAXHOST */

// Fixes: error: variable ‘ifr’ has initializer but incomplete type
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include "utils.h"

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/epoll.h>
#include <errno.h>
#include <assert.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>
#include <string.h>
#include <assert.h>

int interface_index_from_name(int s, char const *interface)
{
	int rc = 0;
	struct ifreq ifr = {0};
	strcpy(ifr.ifr_name, interface);
	rc = ioctl(s, SIOCGIFINDEX, &ifr);
	if (rc < 0) {
		return rc;
	}
	return ifr.ifr_ifindex;
}

int can_socket_from_interface(char const *interface)
{
	int s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (s < 0) {
		return s;
	}

	int index = interface_index_from_name(s, interface);
	if (index < 0) {
		return index;
	}

	struct sockaddr_can addr = {0};
	addr.can_family = AF_CAN;
	addr.can_ifindex = index;

	int rc = bind(s, (struct sockaddr *)&addr, sizeof(struct sockaddr_can));
	if (rc < 0) {
		return rc;
	}
	return s;
}

int can_send(int fd, can_frame_t *frame)
{
	assert(sizeof(can_frame_t) == sizeof(struct can_frame));
	int n = write(fd, frame, sizeof(can_frame_t));
	return n;
}


int can_close(int fd)
{
	int rc = close(fd);
	return rc;
}


int can_recv(int fd, can_frame_t *frame)
{
	int n = read(fd, frame, sizeof(can_frame_t));
	return n;
}


