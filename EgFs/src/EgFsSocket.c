#include "EgFs.h"
#include "EgFs/EgFsSocket.h"
#include "fd.h"

ECS_COMPONENT_DECLARE(EgFsSocketUdp);
ECS_COMPONENT_DECLARE(EgFsSocketInfo);


/*
static void System_Create_Socket_Udp(ecs_iter_t *it)
{
    ecs_log_set_level(-1);
    ecs_world_t *world = it->world;
    EgFsSocketInfo *y = ecs_field(it, EgFsSocketInfo, 0); // self
    for (int i = 0; i < it->count; ++i) {
        ecs_entity_t e = it->entities[i];
        int s = fd_create_udp_socket(y->addr, y->port);
        if (s < 0) {
            ecs_err("fd_create_udp_socket failed for entity '%s'", ecs_get_name(world, e));
            ecs_enable(it->world, e, false);
            continue;
        } else {
            ecs_trace("Created UDP socket fd=%d for entity '%s'", s, ecs_get_name(world, e));
            ecs_set(world, e, EgFsSocketUdp, {.fd = s});
        }
    }
    ecs_log_set_level(-1);
}
*/

void EgFsSocketImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgFsSocket);
	ecs_set_name_prefix(world, "EgFsSocket");

	ECS_COMPONENT_DEFINE(world, EgFsSocketUdp);
	ECS_COMPONENT_DEFINE(world, EgFsSocketInfo);

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity = ecs_id(EgFsSocketUdp),
	.members = {
	{.name = "s", .type = ecs_id(ecs_entity_t)},
	}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity = ecs_id(EgFsSocketInfo),
	.members = {
	{.name = "addr", .type = ecs_id(ecs_string_t)},
	{.name = "port", .type = ecs_id(ecs_i32_t)},
	}});


	/*
	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "System_Create_Socket_Udp", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_Create_Socket_Udp,
	.query.terms =
	{
	{.id = ecs_id(EgFsSocketInfo), .src.id = EcsSelf},
	{.id = ecs_id(EgFsSocketUdp), .oper = EcsNot},
	}});
	*/

}
