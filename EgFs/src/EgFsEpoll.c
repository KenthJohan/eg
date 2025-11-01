/*
https://wiki.libsdl.org/SDL3/SDL_ReadIO
https://github.com/SanderMertens/flecs/blob/master/examples/c/entities/hooks/src/main.c
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/src/datastructures/vec.c#L118
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/include/flecs/datastructures/vec.h
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/src/addons/alerts.c#L39
https://github.com/libsdl-org/SDL/blob/0fcaf47658be96816a851028af3e73256363a390/test/testautomation_iostream.c#L477
*/

#include "EgFs.h"
#include "EgFs/EgFsEpoll.h"
#include "fd.h"
#include "ecsx.h"

ECS_COMPONENT_DECLARE(EgFsEpollCreate);
ECS_COMPONENT_DECLARE(EgFsEpollEvent);

/*
The epoll API performs a similar task to poll(2): monitoring
multiple file descriptors to see if I/O is possible on any of
them.  The epoll API can be used either as an edge-triggered or a
level-triggered interface and scales well to large numbers of
watched file descriptors.
*/

ECS_CTOR(EgFsEpollCreate, ptr, {
	ecs_map_init(&ptr->map, NULL);
})

ECS_DTOR(EgFsEpollCreate, ptr, {
	ecs_map_fini(&ptr->map);
})

ECS_MOVE(EgFsEpollCreate, dst, src, {
	ecs_map_fini(&dst->map);
	dst->map = src->map;
	src->map = (ecs_map_t){0};
})

#define MAX_EVENTS 10

static void System_wait_epoll(ecs_iter_t *it)
{
	ecs_log_set_level(-1);
	ecs_world_t *world = it->world;
	EgFsFd *o = ecs_field(it, EgFsFd, 0);                   // self
	EgFsEpollCreate *c = ecs_field(it, EgFsEpollCreate, 1); // self
	for (int i = 0; i < it->count; ++i, ++o, ++c) {
		fd_epoll_ecs_wait(world, o->fd, &c->map, ecs_id(EgFsReady), sizeof(EgFsReady), &(EgFsReady){0});
	} // END FOR LOOP
	ecs_log_set_level(-1);
}

static void System_Create(ecs_iter_t *it)
{
	ecs_log_set_level(0);
	ecs_world_t *world = it->world;
	EgFsEpollCreate *c = ecs_field_self(it, EgFsEpollCreate, 0); // self
	for (int i = 0; i < it->count; ++i, ++c) {
		ecs_entity_t e = it->entities[i];
		int fd = fd_epoll_create();
		if (fd >= 0) {
			ecs_set_pair(world, e, EgFsFd, ecs_id(EgFsEpollCreate), {fd});
		} else {
			ecs_enable(world, e, false);
		}
	}
	ecs_log_set_level(-1);
}

static void Observer_epoll_ctl(ecs_iter_t *it)
{
	ecs_log_set_level(0);
	// ecsx_trace_system_iter(it);
	// ecs_world_t *world = it->world;
	EgFsFd *parent_fd = ecs_field_shared(it, EgFsFd, 0);                  // shared, parent, epoll fd
	EgFsEpollCreate *parent_c = ecs_field_shared(it, EgFsEpollCreate, 1); // shared, parent, epoll info
	EgFsFd *y = ecs_field_self(it, EgFsFd, 2);                            // self, fd to be added/removed
	for (int i = 0; i < it->count; ++i, ++y) {
		ecs_entity_t e = it->entities[i];
		// ecs_entity_t parent = ecs_field_src(it, 0);
		int rv = 0;
		if (it->event == EcsOnRemove) {
			rv = fd_epoll_rm(parent_fd->fd, y->fd);
			if (rv == 0) {
				// Successfully removed from epoll instance, remove from map
				ecs_map_remove(&parent_c->map, y->fd);
			}
		} else if (it->event == EcsOnSet) {
			rv = fd_epoll_add(parent_fd->fd, y->fd);
			if (rv == 0) {
				// Successfully added to epoll instance, add to map
				ecs_map_insert(&parent_c->map, y->fd, e);
			}
		}
		if (rv != 0) {
			ecs_enable(it->world, e, false);
		}
	}
	ecs_log_set_level(-1);
}

void EgFsEpollImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgFsEpoll);
	ecs_set_name_prefix(world, "EgFsEpoll");

	ECS_COMPONENT_DEFINE(world, EgFsEpollCreate);
	ECS_COMPONENT_DEFINE(world, EgFsEpollEvent);

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity = ecs_id(EgFsEpollEvent),
	.members = {
	{.name = "event", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_set_hooks_id(world, ecs_id(EgFsEpollCreate),
	&(ecs_type_hooks_t){
	.flags = ECS_TYPE_HOOK_COPY_ILLEGAL,
	.move = ecs_move(EgFsEpollCreate),
	.dtor = ecs_dtor(EgFsEpollCreate),
	.ctor = ecs_ctor(EgFsEpollCreate),
	});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "System_wait_epoll", .add = ecs_ids(ecs_dependson(EcsOnValidate))}),
	.callback = System_wait_epoll,
	.query.terms =
	{
	{.id = ecs_pair(ecs_id(EgFsFd), ecs_id(EgFsEpollCreate)), .src.id = EcsSelf},
	{.id = ecs_id(EgFsEpollCreate), .src.id = EcsSelf},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "System_Create", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_Create,
	.query.terms =
	{
	{.id = ecs_id(EgFsEpollCreate), .src.id = EcsSelf},
	{.id = ecs_pair(ecs_id(EgFsFd), ecs_id(EgFsEpollCreate)), .src.id = EcsSelf, .oper = EcsNot}, // Creates this
	}});

	ecs_observer_init(world,
	&(ecs_observer_desc_t){
	.entity = ecs_entity(world, {.name = "Observer_epoll_ctl", .add = ecs_ids(ecs_dependson(EcsPostFrame))}),
	.callback = Observer_epoll_ctl,
	.events = {EcsOnSet, EcsOnRemove},
	.query.terms = {
	{.id = ecs_pair(ecs_id(EgFsFd), ecs_id(EgFsEpollCreate)), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsInOutFilter},
	{.id = ecs_id(EgFsEpollCreate), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsInOutFilter},
	{.id = ecs_pair(ecs_id(EgFsFd), EcsWildcard), .src.id = EcsSelf},
	{.id = ecs_id(EgFsEpollEvent), .src.id = EcsSelf, .inout = EcsInOutFilter},
	}});
}
