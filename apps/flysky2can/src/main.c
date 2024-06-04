#include "utils.h"
#include "ibus.h"
#include "argparse.h"

#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/epoll.h>

#define MAXSOCK 16


static int send_channels_to_can(int fd_can, int can_id, uint16_t ch[], int n_ch)
{
	assert(n_ch <= 4);
	assert(n_ch >= 0);
	can_frame_t frame = {0};
	frame.can_id = can_id;
	frame.len = 2 * n_ch;
	memcpy(frame.data, ch, frame.len);
	int rc = can_send(fd_can, &frame);
	return rc;
}




int main(int argc, char const *argv[])
{

	char const *ibus_serial_port_name;
	char const *can_interface_name;
	int can_id_start;

	{
		static const char *const usages[] = {
		"flysky2can [options] [[--] args]",
		"flysky2can [options]",
		NULL,
		};
		struct argparse_option options[] = {
		OPT_HELP(),
		OPT_GROUP("Basic options"),
		OPT_STRING('i', "ibus", &ibus_serial_port_name, "The IBus serial port name, eg RPI5: </dev/ttyAMA0>, RPI4: </dev/serial0>", NULL, 0, 0),
		OPT_STRING('c', "can", &can_interface_name, "The CAN network interface name, eg: <can0> or <vcan0>", NULL, 0, 0),
		OPT_INTEGER('u', "canid", &can_id_start, "The CAN id, eg: <0> or <10>", NULL, 0, 0),
		OPT_END(),
		};

		struct argparse argparse;
		argparse_init(&argparse, options, usages, 0);
		argparse_describe(&argparse, "\nFlysky to CAN description\n", "Flysky to CAN epilog");
		argc = argparse_parse(&argparse, argc, argv);
	}

	int fd_ibus = ibus_open(ibus_serial_port_name);
	if (fd_ibus < 0) {
		perror("error: ibus_open");
		return fd_ibus;
	}

	int fd_can = can_socket_from_interface(can_interface_name);
	if (fd_can < 0) {
		perror("error: can_socket_from_interace");
		return fd_can;
	}

	int fd_epoll = epoll_create(1);
	if (fd_epoll < 0) {
		perror("error: epoll_create");
		return fd_epoll;
	}

	{
		struct epoll_event e;
		e.events = EPOLLIN;
		e.data.fd = fd_ibus;
		int rc = epoll_ctl(fd_epoll, EPOLL_CTL_ADD, fd_ibus, &e);
		if (rc) {
			perror("error: epoll_ctl");
			return rc;
		}
	}

	{
		struct epoll_event e;
		e.events = EPOLLIN;
		e.data.fd = fd_can;
		int rc = epoll_ctl(fd_epoll, EPOLL_CTL_ADD, fd_can, &e);
		if (rc) {
			perror("error: epoll_ctl");
			return rc;
		}
	}

	/*
	can_frame_t frame = {0};
	while (1) {
		frame.can_id++;
		frame.len = 8;
		can_send(fd_can, &frame);
		usleep(1000);
	}
	*/

	uint16_t ch[10];
	while (1) {

		struct epoll_event e[MAXSOCK] = {0};
		int timeout_ms = 1000;
		int n = epoll_wait(fd_epoll, e, MAXSOCK, timeout_ms);
		if (n < 0) {
			perror("error: epoll_wait");
			continue;
		}
		if (n == 0) {
			printf("epoll_wait: timeout\n");
			continue;
		}
		for (int i = 0; i < n; i++) {
			if (e[i].events & EPOLLERR) {
				printf("error: EPOLLERR\n");
				continue;
			}
			if (e[i].data.fd == fd_ibus) {
				int rc = ibus_read(fd_ibus, ch);
				/*
				printf("%04i: ", rc);
				printf("%04i ", ch[0]);
				printf("%04i ", ch[1]);
				printf("%04i ", ch[2]);
				printf("%04i ", ch[3]);
				printf("%04i ", ch[4]);
				printf("%04i ", ch[5]);
				printf("%04i ", ch[6]);
				printf("%04i ", ch[7]);
				printf("%04i ", ch[8]);
				printf("%04i ", ch[9]);
				printf("\n");
				*/
				if (rc == 0) {
					send_channels_to_can(fd_can, can_id_start + 0, ch + 0, 4);
					send_channels_to_can(fd_can, can_id_start + 1, ch + 4, 4);
					send_channels_to_can(fd_can, can_id_start + 2, ch + 8, 2);
				}
				/*
				 */
			} else if (e[i].data.fd == fd_can) {
				can_frame_t frame = {0};
				int rc = can_recv(fd_can, &frame);
				if (rc < 0) {
					perror("error: can_recv");
				} else {
					printf("canid %i\n", frame.can_id);
				}
			}
		}
	}

	ibus_close(fd_ibus);
	can_close(fd_can);
	return 0;
};
