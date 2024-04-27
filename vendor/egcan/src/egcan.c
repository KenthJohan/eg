// #define _GNU_SOURCE /* To get defns of NI_MAXSERV and NI_MAXHOST */

// Fixes: error: variable ‘ifr’ has initializer but incomplete type
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include "egcan.h"
#include <egquantities.h>

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

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>

#include "iface.h"

ECS_COMPONENT_DECLARE(EgCanEpoll);
ECS_COMPONENT_DECLARE(EgCanBusDescription);
ECS_COMPONENT_DECLARE(EgCanBus);
ECS_COMPONENT_DECLARE(EgCanSignal);
ECS_COMPONENT_DECLARE(EgCanBusBook);
ECS_COMPONENT_DECLARE(EgCanInterface);

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

static void System_EgCanBusDescription1(ecs_iter_t *it)
{
	EgCanBusDescription *d = ecs_field(it, EgCanBusDescription, 1);
	for (int i = 0; i < it->count; ++i, ++d) {
		d->signals_amount = 0;
	}
}

static void System_EgCanBusDescription2(ecs_iter_t *it)
{
	EgCanBusDescription *d = ecs_field(it, EgCanBusDescription, 1);
	// EgCanSignal *s = ecs_field(it, EgCanSignal, 2);
	for (int i = 0; i < it->count; ++i) {
		d->signals_amount++;
	}
}

typedef struct {
	int fd_epoll;
	ecs_os_mutex_t lock;
} thread_stuff_t;

static void CanBusDescription_System(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	thread_stuff_t *stuff = it->ctx;
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

#define MAXSOCK 16
static void *thread_loop(thread_stuff_t *stuff)
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
			if (events[i].events & EPOLLERR) {
				printf("EPOLLERR\n");
				// TODO: How to handle this error?
				continue;
			}
			EgCanBusBook *book = events[i].data.ptr;
			eg_can_frame_t frame = {0};
			int rc = eg_can_recv(book->socket, &frame);
			if (rc < 0) {
				/*
				if (epoll_ctl(stuff->fd_epoll, EPOLL_CTL_DEL, book->socket, NULL)) {
				    perror("failed to delete socket from epoll");
				    continue;
				}
				*/
				/*
				rc = close(book->socket);
				if(rc < 0) {
				    perror("failed to close socket from epoll");
				    continue;
				}
				*/
				continue;
			}
			ecs_os_mutex_lock(stuff->lock);
			eg_can_book8_t *rx = book->rx + frame.can_id;
			rx->dirty = 1;
			rx->len = frame.len;
			rx->packet[0] = frame.data[0];
			rx->packet[1] = frame.data[1];
			rx->packet[2] = frame.data[2];
			rx->packet[3] = frame.data[3];
			rx->packet[4] = frame.data[4];
			rx->packet[5] = frame.data[5];
			rx->packet[6] = frame.data[6];
			rx->packet[7] = frame.data[7];
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
	// ecs_world_t *world = it->world;
	// thread_stuff_t * stuff = it->ctx;
	EgCanBus *bus = ecs_field(it, EgCanBus, 1);
	EgCanBusBook *book = ecs_field(it, EgCanBusBook, 2);
	for (int i = 0; i < it->count; ++i, ++bus, ++book) {
		// ecs_entity_t e = it->entities[i];
		for (int canid = 0; canid < book->cap; ++canid) {
			if (book->tx[canid].dirty) {
				eg_can_frame_t frame = {0};
				frame.can_id = canid;
				frame.len = book->tx[canid].len;
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
	// ecs_world_t *world = it->world;
	thread_stuff_t *stuff = it->ctx;
	EgCanBusBook *book = ecs_field(it, EgCanBusBook, 1);
	EgCanSignal *signal = ecs_field(it, EgCanSignal, 2);
	for (int i = 0; i < it->count; ++i, ++signal) {
		int canid = signal->canid;
		if (canid >= book->cap) {
			continue;
		}
		int byte_offset = signal->byte_offset;
		if (byte_offset >= 8) {
			continue;
		}
		if (byte_offset < 0) {
			continue;
		}

		ecs_os_mutex_lock(stuff->lock);
		int32_t value = book->rx[canid].packet[byte_offset];
		ecs_os_mutex_unlock(stuff->lock);
		signal->rx = value;
	}
}

static void System_Value(ecs_iter_t *it)
{
	EgCanSignal *s = ecs_field(it, EgCanSignal, 1);
	EgQuantitiesProgress *q = ecs_field(it, EgQuantitiesProgress, 2);
	for (int i = 0; i < it->count; ++i, ++s, ++q) {
		q->value = (float)s->rx;
		q->min = (float)s->min;
		q->max = (float)s->max;
	}
}

void EgCanBusBook_prepare_send(EgCanBusBook *book, EgCanSignal *signal)
{
	// printf("Send can packet canid=%i, value=%i\n", (int)signal->canid, signal->value);
	if (signal->canid > EG_CAN_BOOK_CAP) {
		return;
	}
	if (signal->byte_offset >= 8) {
		return;
	}
	if (signal->byte_offset < 0) {
		return;
	}
	uint8_t value = signal->tx;
	book->tx[signal->canid].len = signal->len;
	book->tx[signal->canid].packet[signal->byte_offset] = value;
	book->tx[signal->canid].dirty = 1;
}

void Tick(ecs_iter_t *it)
{
	struct ifaddrs *ifaddr;
	int rc = getifaddrs(&ifaddr);
	if (rc < 0) {
		return;
	}
	for (struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		iface_info_t info = {0};
		interface_details(ifa->ifa_name, &info);
		char buf[128];
		snprintf(buf, sizeof(buf), "interfaces.%s", ifa->ifa_name);
		ecs_entity_t a = ecs_new_entity(it->world, buf);
		EgCanInterface ptr = {
		.can_bitrate = info.can_bitrate,
		.can_clock = info.can_clock,
		.index = info.index,
		.mtu = info.mtu,
		.tso_max_size = info.tso_max_size,
		.numtxqueues = info.numtxqueues,
		.numrxqueues = info.numrxqueues,
		.minmtu = info.minmtu,
		.maxmtu = info.maxmtu,
		};
		ecs_set_ptr(it->world, a, EgCanInterface, &ptr);
	}
	freeifaddrs(ifaddr);
}

void EgCanImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgCan);
	ECS_IMPORT(world, FlecsUnits);
	ECS_IMPORT(world, EgQuantities);
	ecs_set_name_prefix(world, "EgCan");

	ECS_COMPONENT_DEFINE(world, EgCanEpoll);
	ECS_COMPONENT_DEFINE(world, EgCanBusDescription);
	ECS_COMPONENT_DEFINE(world, EgCanBus);
	ECS_COMPONENT_DEFINE(world, EgCanSignal);
	ECS_COMPONENT_DEFINE(world, EgCanBusBook);
	ECS_COMPONENT_DEFINE(world, EgCanInterface);

	thread_stuff_t *stuff = ecs_os_calloc_t(thread_stuff_t);
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
	{.name = "signals_amount", .type = ecs_id(ecs_i32_t)},
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
	{.name = "len", .type = ecs_id(ecs_i32_t)},
	{.name = "rx", .type = ecs_id(ecs_i32_t)},
	{.name = "tx", .type = ecs_id(ecs_i32_t)},
	{.name = "byte_offset", .type = ecs_id(ecs_i32_t)},
	{.name = "min", .type = ecs_id(ecs_i32_t)},
	{.name = "max", .type = ecs_id(ecs_i32_t)},
	{.name = "gui_index", .type = ecs_id(ecs_i32_t)},
	}});


	ecs_struct(world,
	{.entity = ecs_id(EgCanBusBook),
	.members = {
	{.name = "len", .type = ecs_id(ecs_i32_t)},
	{.name = "book", .type = ecs_id(ecs_uptr_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgCanInterface),
	.members = {
	{.name = "index", .type = ecs_id(ecs_i32_t)},
	{.name = "bitrate", .type = ecs_id(ecs_i32_t)},
	{.name = "clock", .type = ecs_id(ecs_i32_t)},
	{.name = "tso_max_size", .type = ecs_id(ecs_i32_t)},
	{.name = "numtxqueues", .type = ecs_id(ecs_i32_t)},
	{.name = "numrxqueues", .type = ecs_id(ecs_i32_t)},
	{.name = "minmtu", .type = ecs_id(ecs_i32_t)},
	{.name = "maxmtu", .type = ecs_id(ecs_i32_t)},
	}});

    ecs_system(world, {
        .entity = ecs_entity(world, { 
            .name = "Tick",
            .add = { ecs_dependson(EcsOnUpdate) } // run in OnUpdate phase
        }),
        .callback = Tick,
        .interval = 1.0  // time in seconds
    });

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = System_EgCanBusDescription1,
	.ctx = stuff,
	.query.filter.terms =
	{
	{.id = ecs_id(EgCanBusDescription)},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = System_EgCanBusDescription2,
	.ctx = stuff,
	.query.filter.terms =
	{
	{.id = ecs_id(EgCanBusDescription), .src.flags = EcsUp, .src.trav = EcsIsA},
	{.id = ecs_id(EgCanSignal)}
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
	{.id = ecs_id(EgCanBusBook), .src.flags = EcsUp, .src.trav = EcsIsA},
	{.id = ecs_id(EgCanSignal)}
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = System_Value,
	.query.filter.terms =
	{
	{.id = ecs_id(EgCanSignal)},
	{.id = ecs_id(EgQuantitiesProgress)}
	}
	});
	// clang-format on
}