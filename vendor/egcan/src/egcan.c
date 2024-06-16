// #define _GNU_SOURCE /* To get defns of NI_MAXSERV and NI_MAXHOST */

// Fixes: error: variable ‘ifr’ has initializer but incomplete type
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include "egcan.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "eg_can.h"

ECS_DECLARE(EgCanRx);
ECS_DECLARE(EgCanTx);
ECS_COMPONENT_DECLARE(EgCanRxThread);
ECS_COMPONENT_DECLARE(EgCanBusDescription);
ECS_COMPONENT_DECLARE(EgCanBus);
ECS_COMPONENT_DECLARE(EgCanSignal);
ECS_COMPONENT_DECLARE(EgCanId);
ECS_COMPONENT_DECLARE(EgCanRxThreadMember);

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
	EgCanBusDescription *desc = ecs_field(it, EgCanBusDescription, 1);
	for (int i = 0; i < it->count; ++i, ++desc) {
		ecs_entity_t e = it->entities[i];

		if (desc->error != 0) {
			continue;
		}

		int sock = socket_from_interace(desc->interface);
		if (sock < 0) {
			desc->error = sock;
			continue;
		}

		EgCanBus *bus = ecs_ensure(world, e, EgCanBus);
		bus->socket = sock;
		bus->ptr = ecs_os_calloc_t(eg_can_book_t);
		bus->ptr->sock = bus->socket;
		bus->ptr->tx = ecs_os_calloc_n(eg_can_book_packet8_t, EG_CAN_BOOK_CAP);
		bus->ptr->rx = ecs_os_calloc_n(eg_can_book_packet8_t, EG_CAN_BOOK_CAP);
		bus->ptr->cap = EG_CAN_BOOK_CAP;
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
				ecs_err("EPOLLERR");
				// TODO: How to handle this error?
				int rc = close(book->sock);
				if (rc < 0) {
					ecs_err("close(): %i: %s", rc, strerror(rc));
				}
				continue;
			}
			eg_can_frame_t frame = {0};
			int rc = eg_can_recv(book->sock, &frame);
			if (rc < 0) {
				ecs_err("eg_can_recv(): %i", rc);
				continue;
			}
			if (frame.can_id >= book->cap) {
				ecs_warn("canid=%i must be less than cap=%i", frame.can_id, book->cap);
				continue;
			}
			ecs_os_mutex_lock(stuff->lock);
			eg_can_book_packet8_t *rx = book->rx + frame.can_id;
			rx->dirty = 1;
			rx->len = frame.len;
			rx->stats_count++;
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

static void eg_can_book_send(eg_can_book_t *book)
{
	eg_can_book_packet8_t *tx = book->tx;
	for (uint32_t canid = 0; canid < book->cap; ++canid, ++tx) {
		if (tx->dirty == 0) {
			continue;
		}
		tx->dirty = 0;
		if (tx->len > 8) {
			ecs_warn("can frame len=%i must not be larger than 8 bytes", tx->len);
			continue;
		}
		eg_can_frame_t frame = {0};
		frame.can_id = tx->can_id;
		frame.len = tx->len;
		frame.data[0] = tx->payload[0];
		frame.data[1] = tx->payload[1];
		frame.data[2] = tx->payload[2];
		frame.data[3] = tx->payload[3];
		frame.data[4] = tx->payload[4];
		frame.data[5] = tx->payload[5];
		frame.data[6] = tx->payload[6];
		frame.data[7] = tx->payload[7];
		eg_can_send(book->sock, &frame);
		int lvl = ecs_log_get_level();
		ecs_log_set_level(0);
		ecs_trace("cansend: socket=%i, id=%i, len=%i, data=[%02X %02X %02X %02X %02X %02X %02X %02X]", book->sock, frame.can_id, frame.len, frame.data[0], frame.data[1], frame.data[2], frame.data[3], frame.data[4], frame.data[5], frame.data[6], frame.data[7]);
		ecs_log_set_level(lvl);
	}
}

static void EgCanBusBook_System_Sender(ecs_iter_t *it)
{
	EgCanBus *bus = ecs_field(it, EgCanBus, 1);
	for (int i = 0; i < it->count; ++i, ++bus) {
		eg_can_book_send(bus->ptr);
	}
}

static void EgCanSignal_parse(ecs_world_t *world, ecs_entity_t e, EgCanSignal *signal, EgCanId *channel, eg_can_book_t const *book, EgQuantitiesRangedGeneric *val)
{
	uint32_t id = channel->id;
	if (id >= book->cap) {
		ecs_warn("canid=%i must be less than cap=%i", id, book->cap);
		return;
	}
	int o = signal->byte_offset;
	if ((o >= 8) || (o < 0)) {
		ecs_warn("offset=%i must be in range of 0..8", o);
		return;
	}
	// TODO: Support all types and bit offsets
	eg_can_book_packet8_t *rx = book->rx + id;
	channel->n = rx->stats_count;
	void *out = &val->rx;
	switch (val->kind) {
	case EcsI8:
	case EcsU8:
		memcpy(out, rx->payload + o, 1);
		break;
	case EcsU16:
		memcpy(out, rx->payload + o, 2);
		break;
	case EcsU32:
		memcpy(out, rx->payload + o, 4);
		break;
	case EcsF32:
		memcpy(out, rx->payload + o, 4);
		break;
	default:
		break;
	}

	/*
	if (signal->component_rep) {
	    EcsComponent const * com = ecs_get(world, signal->component_rep, EcsComponent);
	    ecs_set_id(world, e, signal->component_rep, com->size, out);
	}
	*/

	// TODO: This is temporary code:
	memset(&val->max, 0, sizeof(val->max));
	memset(&val->min, 0, sizeof(val->min));
	switch (val->kind) {
	case EcsU8:
		val->max.val_u8 = val->max_u64;
		val->min.val_u8 = val->min_u64;
		break;
	case EcsU16:
		val->max.val_u16 = val->max_u64;
		val->min.val_u16 = val->min_u64;
		break;
	case EcsI16:
		val->max.val_i16 = val->max_i64;
		val->min.val_i16 = val->min_i64;
		break;
	case EcsU32:
		val->max.val_u32 = val->max_u64;
		val->min.val_u32 = val->min_u64;
		break;
	case EcsU64:
		val->max.val_u64 = val->max_u64;
		val->min.val_u64 = val->min_u64;
		break;
	case EcsF32:
		val->max.val_f32 = val->max_f32;
		val->min.val_f32 = val->min_f32;
		break;
	case EcsF64:
		val->max.val_f64 = val->max_f64;
		val->min.val_f64 = val->min_f64;
		break;
	default:
		break;
	}
}

static void System_Rx(ecs_iter_t *it)
{
	EgCanRxThread *rxt = ecs_field(it, EgCanRxThread, 1);                         // shared
	EgCanBus *bus = ecs_field(it, EgCanBus, 2);                                   // shared
	EgCanId *channel = ecs_field(it, EgCanId, 3);                                 // shared
	EgCanSignal *signal = ecs_field(it, EgCanSignal, 4);                          // self
	EgQuantitiesRangedGeneric *val = ecs_field(it, EgQuantitiesRangedGeneric, 5); // self
	thread_stuff_t const *impl = rxt->impl;
	eg_can_book_t const *book = bus->ptr;
	ecs_os_mutex_lock(impl->lock);
	for (int i = 0; i < it->count; ++i, ++val, ++signal) {
		EgCanSignal_parse(it->world, it->entities[i], signal, channel, book, val);
	}
	ecs_os_mutex_unlock(impl->lock);
}

static void System_Value(ecs_iter_t *it)
{
	EgQuantitiesRangedGeneric *s = ecs_field(it, EgQuantitiesRangedGeneric, 1);
	EgQuantitiesProgress *q = ecs_field(it, EgQuantitiesProgress, 2);
	for (int i = 0; i < it->count; ++i, ++s, ++q) {
		switch (s->kind) {
		case EcsU8:
			q->value = (float)s->rx.val_u8;
			q->min = (float)s->min.val_u8;
			q->max = (float)s->max.val_u8;
			break;
		case EcsU16:
			q->value = (float)s->rx.val_u16;
			q->min = (float)s->min.val_u16;
			q->max = (float)s->max.val_u16;
			break;
		case EcsU32:
			q->value = (float)s->rx.val_u32;
			q->min = (float)s->min.val_u32;
			q->max = (float)s->max.val_u32;
			break;
		case EcsF32:
			q->value = (float)s->rx.val_f32;
			q->min = (float)s->min.val_f32;
			q->max = (float)s->max.val_f32;
			break;

		default:
			break;
		}
	}
}

static void System_EpollAdditions(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	EgCanRxThread *rxt = ecs_field(it, EgCanRxThread, 1); // shared
	EgCanBus *bus = ecs_field(it, EgCanBus, 2);           // self
	for (int i = 0; i < it->count; ++i, ++bus) {
		ecs_entity_t e = it->entities[i];
		thread_stuff_t *impl = rxt->impl;
		// https://stackoverflow.com/questions/7058737/is-epoll-thread-safe
		struct epoll_event event_setup = {
		.events = EPOLLIN,
		.data.ptr = bus->ptr};
		if (epoll_ctl(impl->fd_epoll, EPOLL_CTL_ADD, bus->socket, &event_setup)) {
			ecs_warn("failed to add socket to epoll %s", ecs_get_name(world, e));
			continue;
		}
		EgCanRxThreadMember *m = ecs_ensure(world, e, EgCanRxThreadMember);
		m->dummy = 0;
	}
}

#define CAN_RTR_FLAG 0x40000000U /* remote transmission request */

static int ecs_primitive_kind_size(ecs_primitive_kind_t kind)
{
	switch (kind) {
	case EcsBool:
		return sizeof(ecs_bool_t);
	case EcsChar:
		return sizeof(ecs_char_t);
	case EcsByte:
		return sizeof(ecs_byte_t);
	case EcsU8:
		return sizeof(ecs_u8_t);
	case EcsU16:
		return sizeof(ecs_u16_t);
	case EcsU32:
		return sizeof(ecs_u32_t);
	case EcsU64:
		return sizeof(ecs_u64_t);
	case EcsI8:
		return sizeof(ecs_i8_t);
	case EcsI16:
		return sizeof(ecs_i16_t);
	case EcsI32:
		return sizeof(ecs_i32_t);
	case EcsI64:
		return sizeof(ecs_i64_t);
	case EcsF32:
		return sizeof(ecs_f32_t);
	case EcsF64:
		return sizeof(ecs_f64_t);
	case EcsUPtr:
		return sizeof(ecs_uptr_t);
	case EcsIPtr:
		return sizeof(ecs_iptr_t);
	case EcsString:
		return sizeof(ecs_string_t);
	case EcsEntity:
		return sizeof(ecs_entity_t);
	case EcsId:
		return sizeof(ecs_id_t);
	}
	return 0;
}

void EgCan_book_prepare_send(eg_can_book_t *book, EgCanSignal *signal, EgCanId *channel, EgQuantitiesRangedGeneric *value)
{
	// printf("Send can packet canid=%i, value=%i\n", (int)signal->canid, signal->value);
	uint32_t id = channel->id;
	int32_t o = signal->byte_offset;
	if (id > EG_CAN_BOOK_CAP) {
		return;
	}
	if ((o >= 8) || (o < 0)) {
		return;
	}
	eg_can_book_packet8_t *tx = book->tx + id;
	if (value) {
		memcpy(tx->payload + o, &value->tx, ecs_primitive_kind_size(value->kind));
	}
	tx->can_id = id;
	tx->dirty = 1;
	if (signal->rxtx == 5) {
		tx->len = 0;
		tx->can_id |= CAN_RTR_FLAG;
	} else {
		tx->can_id = id;
		tx->len = signal->len;
	}
}

static ECS_COPY(EgCanRxThread, dst, src, {
	thread_stuff_t *impl = src->impl;
	dst->impl = impl;
})

static ECS_MOVE(EgCanRxThread, dst, src, {
	*dst = *src;
	src->impl = NULL;
})

static ECS_DTOR(EgCanRxThread, ptr, {
                                    // thread_stuff_t *impl = ptr->impl;
                                    // TODO: fini thread stuff
                                    })

static void EgCanRxThread_on_add(ecs_iter_t *it)
{
	EgCanRxThread *t = it->ptrs[0];
	for (int i = 0; i < it->count; ++i) {
		thread_stuff_t *impl = ecs_os_calloc_t(thread_stuff_t);
		impl->lock = ecs_os_mutex_new();
		impl->fd_epoll = epoll_create(1);
		if (impl->fd_epoll < 0) {
			ecs_err(0, "epoll_create");
		} else {
			ecs_log(0, "Starting epoll thread!\n");
			ecs_os_thread_new((ecs_os_thread_callback_t)thread_loop, impl);
		}
		t[i].impl = impl;
	}
}

void EgCanImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgCan);
	ECS_IMPORT(world, FlecsUnits);
	ECS_IMPORT(world, EgQuantities);
	ecs_set_name_prefix(world, "EgCan");

	ECS_ENTITY_DEFINE(world, EgCanRx, Union);
	ECS_ENTITY_DEFINE(world, EgCanTx, Union);
	ECS_COMPONENT_DEFINE(world, EgCanRxThread);
	ECS_COMPONENT_DEFINE(world, EgCanBusDescription);
	ECS_COMPONENT_DEFINE(world, EgCanBus);
	ECS_COMPONENT_DEFINE(world, EgCanSignal);
	ECS_COMPONENT_DEFINE(world, EgCanId);
	ECS_COMPONENT_DEFINE(world, EgCanRxThreadMember);

	// clang-format off
	ecs_set_hooks(world, EgCanRxThread, {
		.ctor = ecs_default_ctor, 
		.move = ecs_move(EgCanRxThread), 
		.copy = ecs_copy(EgCanRxThread), 
		.dtor = ecs_dtor(EgCanRxThread), 
		.on_add = EgCanRxThread_on_add
	});

	ecs_set_hooks(world, EgCanBusDescription, {
	.ctor = ecs_ctor(EgCanBusDescription),
	.move = ecs_move(EgCanBusDescription),
	.copy = ecs_copy(EgCanBusDescription),
	.dtor = ecs_dtor(EgCanBusDescription)
	});

	ecs_struct(world,
	{.entity = ecs_id(EgCanRxThread),
	.members = {
	{.name = "dummy", .type = ecs_id(ecs_i32_t)},
	{.name = "impl", .type = ecs_id(ecs_uptr_t)},
	}});

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
	{.name = "len", .type = ecs_id(ecs_u8_t)},
	{.name = "byte_offset", .type = ecs_id(ecs_u8_t)},
	{.name = "rxtx", .type = ecs_id(ecs_u8_t)}
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgCanId),
	.members = {
	{.name = "id", .type = ecs_id(ecs_u32_t)},
	{.name = "n", .type = ecs_id(ecs_u32_t)},
	}});


	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = System_EgCanBusDescription1,
	.query.filter.terms =
	{
	{.id = ecs_id(EgCanBusDescription)},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = System_EgCanBusDescription2,
	.query.filter.terms =
	{
	{.id = ecs_id(EgCanBusDescription), .src.flags = EcsUp, .src.trav = EcsChildOf},
	{.id = ecs_id(EgCanSignal)}
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = CanBusDescription_System,
	.query.filter.terms =
	{
	{.id = ecs_id(EgCanBusDescription), .src.flags = EcsSelf},
	{.id = ecs_id(EgCanBus), .oper = EcsNot}, // Adds this
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = System_EpollAdditions,
	.query.filter.terms =
	{
	{.id = ecs_id(EgCanRxThread), .src.flags = EcsUp, .src.trav = EcsChildOf},
	{.id = ecs_id(EgCanBus), .src.flags = EcsSelf},
	{.id = ecs_id(EgCanRxThreadMember), .oper = EcsNot}, // Adds this
	}});


	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = EgCanBusBook_System_Sender,
	.query.filter.terms =
	{
	{.id = ecs_id(EgCanBus)},
	}});





	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = System_Rx,
	.query.filter.terms =
	{
	{.id = ecs_id(EgCanRxThread), .src.flags = EcsUp, .src.trav = EcsChildOf},
	{.id = ecs_id(EgCanBus), .src.flags = EcsUp, .src.trav = EcsChildOf},
	{.id = ecs_id(EgCanId), .src.flags = EcsUp, .src.trav = EcsChildOf},
	{.id = ecs_id(EgCanSignal)},
	{.id = ecs_id(EgQuantitiesRangedGeneric)}
	}});








	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = System_Value,
	.query.filter.terms =
	{
	{.id = ecs_id(EgQuantitiesRangedGeneric)},
	{.id = ecs_id(EgQuantitiesProgress)}
	}
	});
	// clang-format on
}