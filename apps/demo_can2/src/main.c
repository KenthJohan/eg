
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <errno.h>
#include <string.h>
#include "eg_can.h"

typedef struct {
	int s;
} bundled_data_per_socket_t;


#define MAXSOCK 16

typedef struct {
	int fd_epoll;
	bundled_data_per_socket_t socks[MAXSOCK];
} thread_data_t;


#define MAXSOCK 16
static void *thread_loop(thread_data_t * arg)
{
	struct epoll_event events[MAXSOCK] = {0};
	int timeout_ms = -1;
	int running = 1;
	while (running) {

		// Wait for events:
		int num_events = epoll_wait(arg->fd_epoll, events, MAXSOCK, timeout_ms);
		if (num_events < 0) {
			fprintf(stderr, "close(): %i: %s", num_events, strerror(errno));
			if (errno != EINTR) {
				running = 0;
			}
			continue;
		}

		// Check every event:
		for (int i = 0; i < num_events; i++) {
			bundled_data_per_socket_t *obj = events[i].data.ptr;
			if (events[i].events & EPOLLERR) {
				fprintf(stderr, "EPOLLERR\n");
			}
			eg_can_frame_t frame = {0};
			int rc = eg_can_recv(obj->s, &frame);
			if (rc < 0) {
				fprintf(stderr, "eg_can_recv(): %i: %s", rc, strerror(errno));
				continue;
			}

			printf("nodeod: %i, cmd:0x%03X\n", (frame.can_id >> 5), (frame.can_id & 0x01F));

		}
	}

	return NULL;
}


#define ODRIVE_CANID(node_id, cmd_id) (((node_id) << 5) | (cmd_id))
#define NODEID 2

static void send_command(int s)
{
	eg_can_frame_t frame = {0};
	frame.can_id = ODRIVE_CANID(NODEID, 0);
	eg_can_send(s, &frame);
}


static void send_vel(int s, float velocity, float torque)
{
	eg_can_frame_t frame = {0};
	frame.can_id = ODRIVE_CANID(NODEID, 0x0D);
	frame.len = 8;
	memcpy(frame.data + 0, &velocity, 4);
	memcpy(frame.data + 4, &torque, 4);
	eg_can_send(s, &frame);
}



int main(int argc, char const * argv[])
{
	thread_data_t tdata = {0};
	tdata.fd_epoll = epoll_create(1);
	if (tdata.fd_epoll < 0) {
		fprintf(stderr, "epoll_create(): %i: %s", tdata.fd_epoll, strerror(errno));
		return -1;
	}


	int index = 0;

	{
		printf("Create socket:\n");
		int s = socket_from_interace("can0");
		printf("socket %i\n", s);
		tdata.socks[index].s = s;
		//send_command(s);
		send_vel(s, 1.0f, 1.0f);
		//return 0;
		
	}


	{
		printf("Adding socket to epoll:\n");
		struct epoll_event setup = {0};
		setup.events = EPOLLIN;
		setup.data.ptr = tdata.socks + index;
		int rc = epoll_ctl(tdata.fd_epoll, EPOLL_CTL_ADD, tdata.socks[index].s, &setup);
		if (rc) {
			fprintf(stderr, "epoll_ctl(): %i: %s", rc, strerror(errno));
		}
	}

	{
		printf("Starting epoll thread!\n");
		thread_loop(&tdata);
	}
	
}

