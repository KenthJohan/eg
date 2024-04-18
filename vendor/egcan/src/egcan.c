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

static void CanBusDescription_System(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	EgCanBusDescription *d = ecs_field(it, EgCanBusDescription, 1);

	for (int i = 0; i < it->count; ++i, ++d) {
		ecs_entity_t e = it->entities[i];
		EgCanBus *b = ecs_ensure(world, e, EgCanBus);
		int fd = socket_from_interace(d->interface);
		b->socket = fd;
		EgCanBusBook *book = ecs_ensure(world, e, EgCanBusBook);
		book->book = ecs_os_calloc_n(eg_can_book8_t, EG_CAN_BOOK_CAP);
		book->cap = EG_CAN_BOOK_CAP;
	}
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
	ecs_world_t *world = it->world;
	EgCanBus *bus = ecs_field(it, EgCanBus, 1);
	EgCanBusBook *book = ecs_field(it, EgCanBusBook, 2);
	for (int i = 0; i < it->count; ++i, ++bus, ++book) {
		ecs_entity_t e = it->entities[i];
		for(int canid = 0; canid < book->cap; ++canid) {
			if (book->book[canid].dirty) {
				eg_can_frame_t frame = {0};
				frame.can_id = canid;
				frame.len = 4;
				frame.data[0] = book->book[canid].packet[0];
				frame.data[1] = book->book[canid].packet[1];
				frame.data[2] = book->book[canid].packet[2];
				frame.data[3] = book->book[canid].packet[3];
				eg_can_send(bus->socket, &frame);
				printf("Send can packet socket=%i\n", bus->socket);
				book->book[canid].dirty = 0;
			}
		}
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
	book->book[signal->canid].len = 4;
	uint8_t value = signal->value;
	book->book[signal->canid].packet[signal->byte_offset] = value;
	book->book[signal->canid].dirty = 1;
}



void EgCanImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgCan);
	ECS_IMPORT(world, FlecsUnits);
	ecs_set_name_prefix(world, "EgCan");

	ECS_COMPONENT_DEFINE(world, EgCanBusDescription);
	ECS_COMPONENT_DEFINE(world, EgCanBus);
	ECS_COMPONENT_DEFINE(world, EgCanSignal);
	ECS_COMPONENT_DEFINE(world, EgCanBusBook);

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
	{.name = "value", .type = ecs_id(ecs_i32_t)},
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
	.query.filter.terms =
	{
	{.id = ecs_id(EgCanBusDescription), .src.flags = EcsSelf},
	{.id = ecs_id(EgCanBus), .oper = EcsNot}, // Adds this
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = EgCanBusBook_System_Sender,
	.query.filter.terms =
	{
	{.id = ecs_id(EgCanBus)},
	{.id = ecs_id(EgCanBusBook)},
	}});
	// clang-format on
}