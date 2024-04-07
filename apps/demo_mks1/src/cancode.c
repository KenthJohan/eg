
#define _DEFAULT_SOURCE

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

#include "eg_can/eg_can.h"
#include "mks_servo/mks_servo.h"

#define MAXSOCK 16 /* max. number of CAN interfaces given on the cmdline */

#define CMD_SPEED_MODE 0x01
#define CMD_HOME 0x03
#define CMD_CURRENT 0x04
#define CMD_STOP_SAVE_SPEED 0x05
#define CMD_STOP_CLEAR_SPEED 0x06

static void eg_can_frame_print(char const * pre, eg_can_frame_t const *f)
{
	printf("%s id=%i, len=%i, data=[ ", pre, f->can_id, f->len);
	for (int i = 0; i < f->len; ++i) {
		printf("%02X ", f->data[i]);
	}
	printf("]\n");
}

static void create_command(eg_can_frame_t const *cmd, eg_can_frame_t *frame)
{
	frame->can_id = 0x1;
	switch (cmd->data[0]) {
	case CMD_HOME:
		frame->len = mks_cmd(frame->data, frame->can_id, 0x91);
		break;
	case CMD_SPEED_MODE:
		frame->len = mks_speed_mode(frame->data, frame->can_id, 0, (cmd->data[2] << 8) + cmd->data[1], 0x02);
		break;
	case CMD_STOP_SAVE_SPEED:
		frame->len = mks_save_speed(frame->data, frame->can_id, MKS_CMD_STOP_AND_SAVE_SPEED);
		break;
	case CMD_STOP_CLEAR_SPEED:
		frame->len = mks_save_speed(frame->data, frame->can_id, MKS_CMD_STOP_AND_CLEAR_SPEED);
		break;
	case CMD_CURRENT:
		frame->len = mks_cmd(frame->data, frame->can_id, cmd->data[1]);
		break;
	default:
		break;
	}
}

static void can_callback(int s)
{
	eg_can_frame_t frame = {0};
	eg_can_recv(s, &frame);
	eg_can_frame_print("recv: ", &frame);

	if ((frame.can_id == 3) && (frame.len >= 2)) {
		eg_can_frame_t f = {0};
		create_command(&frame, &f);
		eg_can_frame_print("send: ", &f);
		eg_can_send(s, &f);
	}

	if (frame.can_id == 1) {
		mks_parse(frame.can_id, frame.data);
	}
}

static void event_loop(int fd_can, int fd_epoll)
{
	struct epoll_event events[MAXSOCK] = {0};
	int timeout_ms = -1;
	int running = 1;

	while (running) {

		// Wait for events:
		int num_events = epoll_wait(fd_epoll, events, MAXSOCK, timeout_ms);
		if (num_events == -1) {
			if (errno != EINTR) {
				running = 0;
			}
			continue;
		}

		// Check every event:
		for (int i = 0; i < num_events; i++) {
			int fd = events[i].data.fd;

			// Check for CAN-bus event:
			if (fd_can == fd) {
				can_callback(fd_can);
			}
		}
	}
}

void * cancode_thread(void * arg)
{
	printf("cancode_thread started!\n");

	int fd_can = eg_can_create_socket("can5");

	int fd_epoll = epoll_create(1);
	if (fd_epoll < 0) {
		perror("epoll_create()");
		return NULL;
	}

	struct epoll_event event_setup = {
	.events = EPOLLIN,
	.data.fd = fd_can};
	if (epoll_ctl(fd_epoll, EPOLL_CTL_ADD, fd_can, &event_setup)) {
		perror("failed to add socket to epoll");
		return NULL;
	}

	event_loop(fd_can, fd_epoll);

	if (close(fd_can) < 0) {
		perror("close()");
		return NULL;
	}

	return NULL;
}
