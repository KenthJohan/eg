#include "EgFs.h"
#include "EgFs/EgFsSocket.h"
#include "fd.h"
#include <ecsx.h>

ECS_COMPONENT_DECLARE(EgFsSocketCreate);

static void System_Create_Socket_Udp(ecs_iter_t *it)
{
	ecs_log_set_level(0);
	ecs_world_t *world = it->world;
	EgFsSocketCreate *y = ecs_field_self(it, EgFsSocketCreate, 0); // self
	for (int i = 0; i < it->count; ++i) {
		ecs_entity_t e = it->entities[i];
		char const *name = ecs_get_name(world, y->path1);
		ecs_trace("System_Create_Socket_Udp for entity '%s' path='%s'", ecs_get_name(world, e), name);
		(void)e;

		int s = fd_create_udp_socket("0.0.0.0", 9999);
		if (s < 0) {
			ecs_err("fd_create_udp_socket failed for entity '%s'", ecs_get_name(world, e));
			ecs_enable(it->world, e, false);
			continue;
		} else {
			ecs_trace("Created UDP socket fd=%d for entity '%s'", s, ecs_get_name(world, e));
			ecs_set_pair(world, e, EgFsFd, ecs_id(EgFsSocketCreate), {.fd = s});
		}
	}
	ecs_log_set_level(-1);
}

static void System_Recv(ecs_iter_t *it)
{
	ecs_log_set_level(-1);
	ecs_world_t *world = it->world;
	EgFsFd *y = ecs_field_self(it, EgFsFd, 0);                     // self
	EgFsSocketCreate *c = ecs_field_self(it, EgFsSocketCreate, 1); // self
	EgFsReady *r = ecs_field_self(it, EgFsReady, 2);               // self
	for (int i = 0; i < it->count; ++i, ++y, ++r, ++c) {
		ecs_entity_t e = it->entities[i];
		fd_udp_test_recv_send(y->fd);
		ecs_remove(world, e, EgFsReady);
	}
	ecs_log_set_level(-1);
}

void EgFsSocketImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgFsSocket);
	ecs_set_name_prefix(world, "EgFsSocket");

	ECS_COMPONENT_DEFINE(world, EgFsSocketCreate);

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity = ecs_id(EgFsSocketCreate),
	.members = {
	{.name = "path1", .type = ecs_id(ecs_entity_t)},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "System_Recv", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_Recv,
	.query.terms =
	{
	{.id = ecs_pair(ecs_id(EgFsFd), ecs_id(EgFsSocketCreate)), .src.id = EcsSelf},
	{.id = ecs_id(EgFsSocketCreate), .src.id = EcsSelf},
	{.id = ecs_id(EgFsReady), .src.id = EcsSelf},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "System_Create_Socket_Udp", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_Create_Socket_Udp,
	.query.terms =
	{
	{.id = ecs_id(EgFsSocketCreate), .src.id = EcsSelf},
	{.id = ecs_pair(ecs_id(EgFsFd), ecs_id(EgFsSocketCreate)), .oper = EcsNot},
	}});
}
