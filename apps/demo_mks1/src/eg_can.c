#include "eg_can.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/epoll.h>
#include <errno.h>
#include <assert.h>

/* CAN CC/FD/XL frame union */
typedef union {
	struct can_frame cc;
	struct canfd_frame fd;
	//struct canxl_frame xl;
} cu_t;

#define EG_CAN_CTRLMSG_LEN CMSG_SPACE(sizeof(struct timeval)) + CMSG_SPACE(3 * sizeof(struct timespec)) + CMSG_SPACE(sizeof(__u32))

int eg_can_addr_from_interface(struct sockaddr_can *addr, int s, char const *interface)
{
	int rc = 0;
	struct ifreq ifr = {0};
	strcpy(ifr.ifr_name, interface);
	rc = ioctl(s, SIOCGIFINDEX, &ifr);
	if (rc < 0) {
		return rc;
	}
	memset(addr, 0, sizeof(struct sockaddr_can));
	addr->can_family = AF_CAN;
	addr->can_ifindex = ifr.ifr_ifindex;
	return rc;
}

int eg_can_create_socket(char const *interface)
{
	printf("CAN Sockets Demo\r\n");
	int s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (s < 0) {
		perror("socket()");
		return 1;
	}

	struct sockaddr_can addr = {0};
	eg_can_addr_from_interface(&addr, s, interface);

	if (bind(s, (struct sockaddr *)&addr, sizeof(struct sockaddr_can)) < 0) {
		perror("bind()");
		return 1;
	}
	return s;
}

void eg_can_print(cu_t *cu, int nbytes)
{
	printf("recvmsglen = %i, id = %04X, len = %i, payload: ", nbytes, cu->fd.can_id, cu->fd.len);
	for (int i = 0; i < cu->fd.len; ++i) {
		printf("%02X", cu->fd.data[i]);
	}
	printf("\n");
}

int eg_can_recv(int s, eg_can_frame_t * frame)
{
	cu_t cu = {0};
	struct iovec iov[1] = {0};
	struct msghdr msg = {0};
	char ctrlmsg[EG_CAN_CTRLMSG_LEN] = {0};
	struct sockaddr_can addr = {0};

	iov[0].iov_base = &cu;

	msg.msg_name = &addr;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	msg.msg_control = &ctrlmsg;
	/* these settings may be modified by recvmsg() */
	msg.msg_iov[0].iov_len = sizeof(cu_t);
	msg.msg_namelen = sizeof(struct sockaddr_can);
	msg.msg_controllen = EG_CAN_CTRLMSG_LEN;
	msg.msg_flags = 0;
	int nbytes = recvmsg(s, &msg, 0);
	if (nbytes < 0) {
		perror("recvmsg()");
		return nbytes;
	}
	//eg_can_print((cu_t *)msg.msg_iov->iov_base, nbytes);

	frame->can_id = cu.cc.can_id;
	frame->len = cu.cc.len;
	memcpy(frame->data, cu.cc.data, CAN_MAX_DLEN);

	return nbytes;
}


int eg_can_send(int s, eg_can_frame_t *frame)
{
	assert(sizeof(eg_can_frame_t) == sizeof(struct can_frame));
	int n = write(s, frame, sizeof(eg_can_frame_t));
	if (n != sizeof(eg_can_frame_t)) {
		perror("write()");
		return n;
	}
	return n;
}













void eg_can_book_init(eg_can_book_t * book)
{
	book->count = EG_CAN_MAX_ID;
	book->v = malloc(sizeof(eg_can_book_t) * book->count);
}


void eg_can_book_set(eg_can_book_t * book, eg_can_frame_t * frame)
{
	int index = frame->can_id;
	book->v[index] = *frame;
}


void eg_can_book_fini(eg_can_book_t * book)
{
	free(book->v);
	book->v = NULL;
	book->count = 0;
}