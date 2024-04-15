#include "Can.h"

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

ECS_COMPONENT_DECLARE(CanBusDescription);
ECS_COMPONENT_DECLARE(CanBus);
ECS_COMPONENT_DECLARE(CanSignal);

ECS_COPY(CanBusDescription, dst, src, {
	ecs_os_strset((char **)&dst->interface, src->interface);
});

ECS_MOVE(CanBusDescription, dst, src, {
	ecs_os_free((char *)dst->interface);
	dst->interface = src->interface;
	src->interface = NULL;
});

ECS_DTOR(CanBusDescription, ptr, {
	ecs_os_free((char *)ptr->interface);
});

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

int socket_from_interace(char const *interface)
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
	CanBusDescription *d = ecs_field(it, CanBusDescription, 1);

	for (int i = 0; i < it->count; ++i, ++d) {
		ecs_entity_t e = it->entities[i];
		CanBus *b = ecs_ensure(world, e, CanBus);
		int fd = socket_from_interace(d->interface);
		b->socket = fd;
	}
}

void CanImport(ecs_world_t *world)
{
	ECS_MODULE(world, Can);
	ECS_COMPONENT_DEFINE(world, CanBusDescription);
	ECS_COMPONENT_DEFINE(world, CanBus);
	ECS_COMPONENT_DEFINE(world, CanSignal);

	// clang-format off
	ecs_set_hooks(world, CanBusDescription, {
	.ctor = ecs_default_ctor,
	.move = ecs_move(CanBusDescription),
	.copy = ecs_copy(CanBusDescription),
	.dtor = ecs_dtor(CanBusDescription)
	});

	ecs_struct(world,
	{.entity = ecs_id(CanBusDescription),
	.members = {
	{.name = "interface", .type = ecs_id(ecs_string_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(CanBus),
	.members = {
	{.name = "socket", .type = ecs_id(ecs_i32_t)},
	}});


	ecs_struct(world,
	{.entity = ecs_id(CanSignal),
	.members = {
	{.name = "canid", .type = ecs_id(ecs_u32_t)},
	{.name = "value", .type = ecs_id(ecs_i32_t)},
	}});


	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = CanBusDescription_System,
	.query.filter.terms =
	{
	{.id = ecs_id(CanBusDescription), .src.flags = EcsSelf},
	{.id = ecs_id(CanBus), .oper = EcsNot}, // Adds this
	}});
	// clang-format on
}