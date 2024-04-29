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
#include "iplink.h"

ECS_COMPONENT_DECLARE(EgCanEpoll);
ECS_COMPONENT_DECLARE(EgCanBusDescription);
ECS_COMPONENT_DECLARE(EgCanBus);
ECS_COMPONENT_DECLARE(EgCanSignal);
ECS_COMPONENT_DECLARE(EgCanInterface);

ECS_CTOR(EgCanBusDescription, ptr, {
	ptr->interface = NULL;
	ptr->signals_amount = 0;
	ptr->error = 0;
})

ECS_DTOR(EgCanBusDescription, ptr, {
	ecs_os_free((char *)ptr->interface);
})

ECS_COPY(EgCanBusDescription, dst, src, {
	ecs_os_strset((char **)&dst->interface, src->interface);
	dst->signals_amount = src->signals_amount;
	dst->error = src->error;
})

ECS_MOVE(EgCanBusDescription, dst, src, {
	ecs_os_free((char *)dst->interface);
	dst->interface = src->interface;
	dst->signals_amount = src->signals_amount;
	dst->error = src->error;
	src->interface = NULL;
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
		if (d->error != 0) {
			continue;
		}

		int sock = socket_from_interace(d->interface);
		if (sock < 0) {
			d->error = sock;
			continue;
		}

		EgCanBus *bus = ecs_ensure(world, e, EgCanBus);
		bus->socket = sock;
		bus->ptr = ecs_os_calloc_t(eg_can_book_t);
		bus->ptr->sock = bus->socket;
		bus->ptr->tx = ecs_os_calloc_n(eg_can_book_packet8_t, EG_CAN_BOOK_CAP);
		bus->ptr->rx = ecs_os_calloc_n(eg_can_book_packet8_t, EG_CAN_BOOK_CAP);
		bus->ptr->cap = EG_CAN_BOOK_CAP;

		// https://stackoverflow.com/questions/7058737/is-epoll-thread-safe
		struct epoll_event event_setup = {
		.events = EPOLLIN,
		.data.ptr = bus->ptr};
		if (epoll_ctl(stuff->fd_epoll, EPOLL_CTL_ADD, bus->socket, &event_setup)) {
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
			eg_can_book_t *book = events[i].data.ptr;
			if (events[i].events & EPOLLERR) {
				printf("EPOLLERR\n");
				// TODO: How to handle this error?
				int rc = close(book->sock);
				if (rc < 0) {
					perror("failed to close socket from epoll");
				}
				continue;
			}
			eg_can_frame_t frame = {0};
			int rc = eg_can_recv(book->sock, &frame);
			if (rc < 0) {
				continue;
			}
			ecs_os_mutex_lock(stuff->lock);
			eg_can_book_packet8_t *rx = book->rx + frame.can_id;
			rx->dirty = 1;
			rx->len = frame.len;
			rx->payload[0] = frame.data[0];
			rx->payload[1] = frame.data[1];
			rx->payload[2] = frame.data[2];
			rx->payload[3] = frame.data[3];
			rx->payload[4] = frame.data[4];
			rx->payload[5] = frame.data[5];
			rx->payload[6] = frame.data[6];
			rx->payload[7] = frame.data[7];
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

static void eg_can_book_send(eg_can_book_t *book)
{
	for (int canid = 0; canid < book->cap; ++canid) {
		if (book->tx[canid].dirty) {
			eg_can_frame_t frame = {0};
			frame.can_id = canid;
			frame.len = book->tx[canid].len;
			frame.data[0] = book->tx[canid].payload[0];
			frame.data[1] = book->tx[canid].payload[1];
			frame.data[2] = book->tx[canid].payload[2];
			frame.data[3] = book->tx[canid].payload[3];
			frame.data[4] = book->tx[canid].payload[4];
			frame.data[5] = book->tx[canid].payload[5];
			frame.data[6] = book->tx[canid].payload[6];
			frame.data[7] = book->tx[canid].payload[7];
			eg_can_send(book->sock, &frame);
			printf("Send can packet socket=%i\n", book->sock);
			book->tx[canid].dirty = 0;
		}
	}
}

static void EgCanBusBook_System_Sender(ecs_iter_t *it)
{
	// ecs_world_t *world = it->world;
	// thread_stuff_t * stuff = it->ctx;
	EgCanBus *bus = ecs_field(it, EgCanBus, 1);
	for (int i = 0; i < it->count; ++i, ++bus) {
		// ecs_entity_t e = it->entities[i];
		eg_can_book_send(bus->ptr);
	}
}

static void EgCanSignal_parse(EgCanSignal *signal, eg_can_book_t *book)
{
	int id = signal->canid;
	if (id >= book->cap) {
		return;
	}
	int o = signal->byte_offset;
	if ((o >= 8) || (o < 0)) {
		return;
	}
	// TODO: Support all types and bit offsets
	eg_can_book_packet8_t *rx = book->rx + id;
	int32_t value = (int8_t)rx->payload[o];
	signal->rx = value;
}

static void System_Rx(ecs_iter_t *it)
{
	thread_stuff_t *stuff = it->ctx;
	EgCanBus *bus = ecs_field(it, EgCanBus, 1);          // shared
	EgCanSignal *signal = ecs_field(it, EgCanSignal, 2); // self
	for (int i = 0; i < it->count; ++i) {
		ecs_os_mutex_lock(stuff->lock);
		EgCanSignal_parse(signal + i, bus->ptr);
		ecs_os_mutex_unlock(stuff->lock);
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

void Tick(ecs_iter_t *it)
{
	iplink_info_t info[5] = {0};
	int n = 0;
	//n = iplink_parse(info, 5);

	for (int i = 0; i < n; ++i) {
		char buf[256];
		snprintf(buf, sizeof(buf), "interfaces.%s", info[i].ifname);
		ecs_entity_t a = ecs_new_entity(it->world, buf);
		EgCanInterface ptr = {
		.can_bitrate = info[i].can_bitrate,
		.can_clock = info[i].can_clock,
		.index = info[i].ifindex,
		.mtu = info[i].mtu,
		.tso_max_size = info[i].tso_max_size,
		.numtxqueues = info[i].numtxqueues,
		.numrxqueues = info[i].numrxqueues,
		.minmtu = info[i].minmtu,
		.maxmtu = info[i].maxmtu,
		.stats64_rx_bytes = info[i].stats64_rx_bytes,
		.stats64_rx_packets = info[i].stats64_rx_packets,
		.stats64_rx_errors = info[i].stats64_rx_errors,
		.stats64_tx_bytes = info[i].stats64_tx_bytes,
		.stats64_tx_packets = info[i].stats64_tx_packets,
		.stats64_tx_errors = info[i].stats64_tx_errors,
		};
		ecs_set_ptr(it->world, a, EgCanInterface, &ptr);
	}

	return;
}

void Observer(ecs_iter_t *it)
{
	// EgCanBook *b = ecs_field(it, EgCanBook, 1);
	for (int i = 0; i < it->count; ++i) {
		// ecs_entity_t e = it->entities[i];
	}
}

void eg_can_book_prepare_send(eg_can_book_t *book, EgCanSignal *signal)
{
	// printf("Send can packet canid=%i, value=%i\n", (int)signal->canid, signal->value);
	uint32_t id = signal->canid;
	int32_t o = signal->byte_offset;
	if (id > EG_CAN_BOOK_CAP) {
		return;
	}
	if ((o >= 8) || (o < 0)) {
		return;
	}
	uint8_t value = signal->tx;
	book->tx[id].len = signal->len;
	book->tx[id].payload[o] = value;
	book->tx[id].dirty = 1;
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
	.ctor = ecs_ctor(EgCanBusDescription),
	.move = ecs_move(EgCanBusDescription),
	.copy = ecs_copy(EgCanBusDescription),
	.dtor = ecs_dtor(EgCanBusDescription)
	});

    ecs_observer(world, {
        .filter = { .terms = {{ .id = ecs_id(EgCanBus) }}},
        .events = { EcsOnAdd },
        .callback = Observer
    });

	ecs_struct(world,
	{.entity = ecs_id(EgCanBusDescription),
	.members = {
	{.name = "interface", .type = ecs_id(ecs_string_t)},
	{.name = "signals_amount", .type = ecs_id(ecs_i32_t)},
	{.name = "error", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgCanBus),
	.members = {
	{.name = "socket", .type = ecs_id(ecs_i32_t)},
	{.name = "book", .type = ecs_id(ecs_uptr_t)},
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
	{.name = "stats64_rx_bytes", .type = ecs_id(ecs_i32_t)},
	{.name = "stats64_rx_packets", .type = ecs_id(ecs_i32_t)},
	{.name = "stats64_rx_errors", .type = ecs_id(ecs_i32_t)},
	{.name = "stats64_tx_bytes", .type = ecs_id(ecs_i32_t)},
	{.name = "stats64_tx_packets", .type = ecs_id(ecs_i32_t)},
	{.name = "stats64_tx_errors", .type = ecs_id(ecs_i32_t)},
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
	}});


	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = System_Rx,
	.ctx = stuff,
	.query.filter.terms =
	{
	{.id = ecs_id(EgCanBus), .src.flags = EcsUp, .src.trav = EcsIsA},
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