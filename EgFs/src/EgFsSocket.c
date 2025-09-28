#include "EgFs.h"
#include "EgFs/EgFsSocket.h"
#include "fd.h"

ECS_COMPONENT_DECLARE(EgFsSocketUdp);
ECS_COMPONENT_DECLARE(EgFsSocketInfo);

static void Observer_epoll_ctl(ecs_iter_t *it)
{
	ecs_log_set_level(0);
	ecs_world_t *world = it->world;
	EgFsSocketUdp *y = ecs_field(it, EgFsSocketUdp, 0); // self
	EgFsEpollFd *o = ecs_field(it, EgFsEpollFd, 1);     // shared
	for (int i = 0; i < it->count; ++i, ++y) {
		ecs_entity_t e = it->entities[i];
		ecs_entity_t parent = ecs_field_src(it, 1);
		int rv = 0;
		if (it->event == EcsOnRemove) {
			rv = fd_epoll_rm(o->fd, y->s);
			ecs_trace("Removing inotify fd=%i (%s) from epoll fd=%i (%s)", y->s, ecs_get_name(world, e), o->fd, ecs_get_name(world, parent));
			if (rv == 0) {
				ecs_map_remove(&o->map, y->s);
			}
		} else if (it->event == EcsOnAdd) {
			ecs_trace("Adding inotify fd=%i (%s) to epoll fd=%i (%s)", y->s, ecs_get_name(world, e), o->fd, ecs_get_name(world, parent));
			rv = fd_epoll_add(o->fd, y->s);
			if (rv == 0) {
				ecs_map_insert(&o->map, y->s, e);
			}
		}
		if (rv != 0) {
			ecs_enable(it->world, e, false);
		}
	}
	ecs_log_set_level(-1);
}

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

	ecs_observer_init(world,
	&(ecs_observer_desc_t){
	.entity = ecs_entity(world, {.name = "Observer_epoll_ctl"}),
	.callback = Observer_epoll_ctl,
	.events = {EcsOnAdd, EcsOnRemove},
	.query.terms = {
	{.id = ecs_id(EgFsSocketUdp)},
	{.id = ecs_id(EgFsEpollFd), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsInOutFilter},
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
