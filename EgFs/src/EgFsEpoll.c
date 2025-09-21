/*
https://wiki.libsdl.org/SDL3/SDL_ReadIO
https://github.com/SanderMertens/flecs/blob/master/examples/c/entities/hooks/src/main.c
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/src/datastructures/vec.c#L118
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/include/flecs/datastructures/vec.h
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/src/addons/alerts.c#L39
https://github.com/libsdl-org/SDL/blob/0fcaf47658be96816a851028af3e73256363a390/test/testautomation_iostream.c#L477
*/


#include "EgFs.h"
#include "EgFs/EgFsPaths.h"
#include "EgFs/EgFsEpoll.h"
#include "fd.h"

ECS_COMPONENT_DECLARE(EgFsEpollFd);

/*
The epoll API performs a similar task to poll(2): monitoring
multiple file descriptors to see if I/O is possible on any of
them.  The epoll API can be used either as an edge-triggered or a
level-triggered interface and scales well to large numbers of
watched file descriptors.
*/

ECS_CTOR(EgFsEpollFd, ptr, {
	ptr->fd = fd_epoll_create();
	ecs_map_init(&ptr->map, NULL);
})

ECS_DTOR(EgFsEpollFd, ptr, {
	fd_close_valid(ptr->fd);
	ecs_map_fini(&ptr->map);
})

ECS_MOVE(EgFsEpollFd, dst, src, {
	fd_close_valid(dst->fd);
	dst->fd = src->fd;
	src->fd = -1; // Invalidate the source fd

	ecs_map_fini(&dst->map);
	dst->map = src->map;
	src->map = (ecs_map_t){0};
})

#define MAX_EVENTS 10

static void System_epoll(ecs_iter_t *it)
{
	ecs_log_set_level(-1);
	ecs_world_t *world = it->world;
	EgFsEpollFd *o = ecs_field(it, EgFsEpollFd, 0); // self
	for (int i = 0; i < it->count; ++i, ++o) {
		fd_epoll_ecs_wait(world, o->fd, &o->map, ecs_id(EgFsReady), sizeof(EgFsReady), &(EgFsReady){0});
	} // END FOR LOOP
	ecs_log_set_level(-1);
}

void EgFsEpollImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgFsEpoll);
	ecs_set_name_prefix(world, "EgFsEpoll");

	ECS_COMPONENT_DEFINE(world, EgFsEpollFd);

	ecs_set_hooks_id(world, ecs_id(EgFsEpollFd),
	&(ecs_type_hooks_t){
	.flags = ECS_TYPE_HOOK_COPY_ILLEGAL,
	.move = ecs_move(EgFsEpollFd),
	.dtor = ecs_dtor(EgFsEpollFd),
	.ctor = ecs_ctor(EgFsEpollFd),
	});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity = ecs_id(EgFsEpollFd),
	.members = {
	{.name = "fd", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "System_epoll", .add = ecs_ids(ecs_dependson(EcsOnValidate))}),
	.callback = System_epoll,
	.query.terms =
	{
	{.id = ecs_id(EgFsEpollFd), .src.id = EcsSelf},
	}});
}
