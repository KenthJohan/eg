#include "egcan.h"

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

ECS_COMPONENT_DECLARE(EgCanEpoll);
ECS_COMPONENT_DECLARE(EgCanBusDescription);
ECS_COMPONENT_DECLARE(EgCanBus);
ECS_COMPONENT_DECLARE(EgCanSignal);
ECS_COMPONENT_DECLARE(EgCanBusBook);

ECS_COPY(EgCanBusDescription, dst, src, {
	ecs_os_strset((char **)&dst->interface, src->interface);
})

ECS_MOVE(EgCanBusDescription, dst, src, {
	ecs_os_free((char *)dst->interface);
	dst->interface = src->interface;
	src->interface = NULL;
})

ECS_DTOR(EgCanBusDescription, ptr, {
	ecs_os_free((char *)ptr->interface);
})

static int sockaddr_can_from_interface(struct sockaddr_can *addr, int s, char const *interface)
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

static int socket_from_interace(char const *interface)
{
	printf("CAN Sockets Demo\r\n");
	int s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (s < 0) {
		perror("socket()");
		return 1;
	}

	struct sockaddr_can addr = {0};
	int rc = sockaddr_can_from_interface(&addr, s, interface);

	if (rc < 0) {
		return rc;
	}

	if (bind(s, (struct sockaddr *)&addr, sizeof(struct sockaddr_can)) < 0) {
		perror("bind()");
		return 1;
	}
	return s;
}






typedef struct {
	int fd_epoll;
	ecs_os_mutex_t lock;
} thread_stuff_t;


static void CanBusDescription_System(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	thread_stuff_t * stuff = it->ctx;
	EgCanBusDescription *d = ecs_field(it, EgCanBusDescription, 1);

	for (int i = 0; i < it->count; ++i, ++d) {
		ecs_entity_t e = it->entities[i];

		EgCanBus *b = ecs_ensure(world, e, EgCanBus);
		b->socket = socket_from_interace(d->interface);
		if (b->socket < 0) {
			continue;
		}

		EgCanBusBook *book = ecs_ensure(world, e, EgCanBusBook);
		book->socket = b->socket;
		book->tx = ecs_os_calloc_n(eg_can_book8_t, EG_CAN_BOOK_CAP);
		book->rx = ecs_os_calloc_n(eg_can_book8_t, EG_CAN_BOOK_CAP);
		book->cap = EG_CAN_BOOK_CAP;

		// https://stackoverflow.com/questions/7058737/is-epoll-thread-safe
		struct epoll_event event_setup = {
		.events = EPOLLIN,
		.data.ptr = book};
		if (epoll_ctl(stuff->fd_epoll, EPOLL_CTL_ADD, b->socket, &event_setup)) {
			perror("failed to add socket to epoll");
			continue;
		}


		// ecs_os_mutex_lock(stuff->lock);
		// ecs_os_mutex_unlock(stuff->lock);


	}
}

#define EG_CAN_CTRLMSG_LEN CMSG_SPACE(sizeof(struct timeval)) + CMSG_SPACE(3 * sizeof(struct timespec)) + CMSG_SPACE(sizeof(__u32))
/* CAN CC/FD/XL frame union */
typedef union {
	struct can_frame cc;
	struct canfd_frame fd;
	//struct canxl_frame xl;
} cu_t;


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



#define MAXSOCK 16
static void * thread_loop(thread_stuff_t * stuff)
{
	struct epoll_event events[MAXSOCK] = {0};
	int timeout_ms = -1;
	int running = 1;

	while (running) {

		// Wait for events:
		int num_events = epoll_wait(stuff->fd_epoll, events, MAXSOCK, timeout_ms);
		if (num_events == -1) {
			if (errno != EINTR) {
				running = 0;
			}
			continue;
		}

		// Check every event:
		for (int i = 0; i < num_events; i++) {
			EgCanBusBook *book = events[i].data.ptr;
			printf("epoll event: %i\n", book->socket);
			eg_can_frame_t frame = {0};
			eg_can_recv(book->socket, &frame);
			ecs_os_mutex_lock(stuff->lock);
			book->rx[frame.can_id].dirty = 1;
			book->rx[frame.can_id].len = frame.len;
			book->rx[frame.can_id].packet[0] = frame.data[0];
			book->rx[frame.can_id].packet[1] = frame.data[1];
			book->rx[frame.can_id].packet[2] = frame.data[2];
			book->rx[frame.can_id].packet[3] = frame.data[3];
			book->rx[frame.can_id].packet[4] = frame.data[4];
			book->rx[frame.can_id].packet[5] = frame.data[5];
			book->rx[frame.can_id].packet[6] = frame.data[6];
			book->rx[frame.can_id].packet[7] = frame.data[7];
			ecs_os_mutex_unlock(stuff->lock);
		}
	}

	return NULL;
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


static void EgCanBusBook_System_Sender(ecs_iter_t *it)
{
	//ecs_world_t *world = it->world;
	thread_stuff_t * stuff = it->ctx;
	EgCanBus *bus = ecs_field(it, EgCanBus, 1);
	EgCanBusBook *book = ecs_field(it, EgCanBusBook, 2);
	for (int i = 0; i < it->count; ++i, ++bus, ++book) {
		//ecs_entity_t e = it->entities[i];
		for(int canid = 0; canid < book->cap; ++canid) {
			if (book->tx[canid].dirty) {
				eg_can_frame_t frame = {0};
				frame.can_id = canid;
				frame.len = 4;
				frame.data[0] = book->tx[canid].packet[0];
				frame.data[1] = book->tx[canid].packet[1];
				frame.data[2] = book->tx[canid].packet[2];
				frame.data[3] = book->tx[canid].packet[3];
				eg_can_send(bus->socket, &frame);
				printf("Send can packet socket=%i\n", bus->socket);
				book->tx[canid].dirty = 0;
			}
		}
	}
}


static void System_Rx(ecs_iter_t *it)
{
	//ecs_world_t *world = it->world;
	thread_stuff_t * stuff = it->ctx;
	EgCanBusBook *book = ecs_field(it, EgCanBusBook, 1);
	EgCanSignal *signal = ecs_field(it, EgCanSignal, 2);
	for (int i = 0; i < it->count; ++i, ++signal) {
		int canid = signal->canid;
		if (canid >= book->cap) {
			continue;
		}
		int byte_offset = signal->byte_offset;
		if(byte_offset >= 8) {
			continue;
		}
		if(byte_offset < 0) {
			continue;
		}
		ecs_os_mutex_lock(stuff->lock);
		signal->rx = book->rx[canid].packet[byte_offset];
		ecs_os_mutex_unlock(stuff->lock);
	}
}



void EgCanBusBook_prepare_send(EgCanBusBook * book, EgCanSignal * signal) {
	//printf("Send can packet canid=%i, value=%i\n", (int)signal->canid, signal->value);
	if(signal->canid > EG_CAN_BOOK_CAP) {
		return;
	}
	if(signal->byte_offset >= 8) {
		return;
	}
	if(signal->byte_offset < 0) {
		return;
	}
	uint8_t value = signal->tx;
	book->tx[signal->canid].len = 4;
	book->tx[signal->canid].packet[signal->byte_offset] = value;
	book->tx[signal->canid].dirty = 1;
}








void EgCanImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgCan);
	ECS_IMPORT(world, FlecsUnits);
	ecs_set_name_prefix(world, "EgCan");

	ECS_COMPONENT_DEFINE(world, EgCanEpoll);
	ECS_COMPONENT_DEFINE(world, EgCanBusDescription);
	ECS_COMPONENT_DEFINE(world, EgCanBus);
	ECS_COMPONENT_DEFINE(world, EgCanSignal);
	ECS_COMPONENT_DEFINE(world, EgCanBusBook);




	thread_stuff_t * stuff = ecs_os_calloc_t(thread_stuff_t);
	stuff->lock = ecs_os_mutex_new();
	stuff->fd_epoll = epoll_create(1);
	if (stuff->fd_epoll < 0) {
		perror("epoll_create()");
	} else {
		printf("Starting epoll thread!\n");
		ecs_os_thread_new((ecs_os_thread_callback_t)thread_loop, stuff);
	}


	// clang-format off
	ecs_set_hooks(world, EgCanBusDescription, {
	.ctor = ecs_default_ctor,
	.move = ecs_move(EgCanBusDescription),
	.copy = ecs_copy(EgCanBusDescription),
	.dtor = ecs_dtor(EgCanBusDescription)
	});

	ecs_struct(world,
	{.entity = ecs_id(EgCanBusDescription),
	.members = {
	{.name = "interface", .type = ecs_id(ecs_string_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgCanBus),
	.members = {
	{.name = "socket", .type = ecs_id(ecs_i32_t)},
	}});


	ecs_struct(world,
	{.entity = ecs_id(EgCanSignal),
	.members = {
	{.name = "canid", .type = ecs_id(ecs_u32_t)},
	{.name = "rx", .type = ecs_id(ecs_i32_t)},
	{.name = "tx", .type = ecs_id(ecs_i32_t)},
	{.name = "byte_offset", .type = ecs_id(ecs_i32_t)},
	}});


	ecs_struct(world,
	{.entity = ecs_id(EgCanBusBook),
	.members = {
	{.name = "len", .type = ecs_id(ecs_i32_t)},
	{.name = "book", .type = ecs_id(ecs_uptr_t)},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = CanBusDescription_System,
	.ctx = stuff,
	.query.filter.terms =
	{
	{.id = ecs_id(EgCanBusDescription), .src.flags = EcsSelf},
	{.id = ecs_id(EgCanBus), .oper = EcsNot}, // Adds this
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = EgCanBusBook_System_Sender,
	.ctx = stuff,
	.query.filter.terms =
	{
	{.id = ecs_id(EgCanBus)},
	{.id = ecs_id(EgCanBusBook)},
	}});


	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = System_Rx,
	.ctx = stuff,
	.query.filter.terms =
	{
	{.id = ecs_id(EgCanBusBook), .src.flags = EcsUp, .src.trav = EcsChildOf},
	{.id = ecs_id(EgCanSignal)}
	}});
	// clang-format on
}