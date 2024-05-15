
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <errno.h>
#include <string.h>
#include "eg_can.h"

/*
https://github.com/belovictor/odrive_can_ros_driver/blob/main/src/odrive_axis.hpp
https://docs.odriverobotics.com/v/latest/manual/can-protocol.html#cyclic-messages
*/
typedef enum {
	CANOPEN_NMT_MESSAGE = 0x000,
	HEARTBEAT_MESSAGE = 0x001,
	ESTOP_MESSAGE = 0x002,
	GETERROR = 0x003,
	SET_AXIS_NODE_ID = 0x006,
	SET_AXIS_REQUESTED_STATE = 0x007,
	SET_AXIS_STARTUP_CONFIG = 0x008,
	GET_ENCODER_ESTIMATE = 0x009,
	GET_ENCODER_COUNT = 0x00A,
	GET_TORQ = 0x01C,
	SET_CONTROLLER_MODES = 0x00B,
	SET_INPUT_POS = 0x00C,
	SET_INPUT_VELOCITY = 0x00D,
	SET_INPUT_TORQUE = 0x00E,
	SET_LIMITS = 0x00F,
	START_ANTI_COGGING = 0x010,
	SET_TRAJ_VEL_LIMIT = 0x011,
	SET_TRAJ_ACCEL_LIMITS = 0x012,
	SET_TRAJ_INERTIA = 0x013,
	GET_IQ = 0x014,
	GET_TEMPERATURE = 0x015,
	REBOOT_ODRIVE = 0x016,
	GET_BUS_VOLTAGE_AND_CURRENT = 0x017,
	CLEAR_ERRORS = 0x018,
	SET_ABSOLUTE_POSITION = 0x019,
	SET_POSITION_GAIN = 0x01A,
	SET_VEL_GAINS = 0x01B,
	CANOPEN_HEARTBEAT_MESSAGE = 0x700
} ODriveCommandId_t;

static char const *get_cmd_str(ODriveCommandId_t cmd)
{
	switch (cmd) {
	case CANOPEN_NMT_MESSAGE:
		return "CANOPEN_NMT_MESSAGE";
	case HEARTBEAT_MESSAGE:
		return "HEARTBEAT_MESSAGE";
	case ESTOP_MESSAGE:
		return "ESTOP_MESSAGE";
	case GETERROR:
		return "GETERROR";
	case SET_AXIS_NODE_ID:
		return "SET_AXIS_NODE_ID";
	case SET_AXIS_REQUESTED_STATE:
		return "SET_AXIS_REQUESTED_STATE";
	case SET_AXIS_STARTUP_CONFIG:
		return "SET_AXIS_STARTUP_CONFIG";
	case GET_ENCODER_ESTIMATE:
		return "GET_ENCODER_ESTIMATE";
	case GET_ENCODER_COUNT:
		return "GET_ENCODER_COUNT";
	case GET_TORQ:
		return "GET_TORQ";
	case SET_CONTROLLER_MODES:
		return "SET_CONTROLLER_MODES";
	case SET_INPUT_POS:
		return "SET_INPUT_POS";
	case SET_INPUT_VELOCITY:
		return "SET_INPUT_VELOCITY";
	case SET_INPUT_TORQUE:
		return "SET_INPUT_TORQUE";
	case SET_LIMITS:
		return "SET_LIMITS";
	case START_ANTI_COGGING:
		return "START_ANTI_COGGING";
	case SET_TRAJ_VEL_LIMIT:
		return "SET_TRAJ_VEL_LIMIT";
	case SET_TRAJ_ACCEL_LIMITS:
		return "SET_TRAJ_ACCEL_LIMITS";
	case SET_TRAJ_INERTIA:
		return "SET_TRAJ_INERTIA";
	case GET_IQ:
		return "GET_IQ";
	case GET_TEMPERATURE:
		return "GET_TEMPERATURE";
	case REBOOT_ODRIVE:
		return "REBOOT_ODRIVE";
	case GET_BUS_VOLTAGE_AND_CURRENT:
		return "GET_BUS_VOLTAGE_AND_CURRENT";
	case CLEAR_ERRORS:
		return "CLEAR_ERRORS";
	case SET_ABSOLUTE_POSITION:
		return "SET_ABSOLUTE_POSITION";
	case SET_POSITION_GAIN:
		return "SET_POSITION_GAIN";
	case SET_VEL_GAINS:
		return "SET_VEL_GAINS";
	case CANOPEN_HEARTBEAT_MESSAGE:
		return "CANOPEN_HEARTBEAT_MESSAG";
	}
	return "";
}

typedef struct {
	int s;
} bundled_data_per_socket_t;

#define MAXSOCK 16

typedef struct {
	int fd_epoll;
	bundled_data_per_socket_t socks[MAXSOCK];
} thread_data_t;

#define MAXSOCK 16
static void *thread_loop(thread_data_t *arg)
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

			printf("nodeod: %i, cmd:%03X, %-30s, len:%i\n", (frame.can_id >> 5), (frame.can_id & 0x01F), get_cmd_str(frame.can_id & 0x01F), frame.len);
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

int main(int argc, char const *argv[])
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
		// send_command(s);
		send_vel(s, 1.0f, 1.0f);
		// return 0;
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
