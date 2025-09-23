/*
https://wiki.libsdl.org/SDL3/SDL_ReadIO
https://github.com/SanderMertens/flecs/blob/master/examples/c/entities/hooks/src/main.c
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/src/datastructures/vec.c#L118
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/include/flecs/datastructures/vec.h
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/src/addons/alerts.c#L39
https://github.com/libsdl-org/SDL/blob/0fcaf47658be96816a851028af3e73256363a390/test/testautomation_iostream.c#L477
*/

#include "EgFs.h"
#include "EgFs/EgFsFanotify.h"
#include "fd.h"

ECS_COMPONENT_DECLARE(EgFsFanotifyFd);

ECS_CTOR(EgFsFanotifyFd, ptr, {
	ptr->fd = fd_fanotify_init();
})

// The destructor should free resources.
ECS_DTOR(EgFsFanotifyFd, ptr, {
	fd_close_valid(ptr->fd);
})

ECS_MOVE(EgFsFanotifyFd, dst, src, {
	fd_close_valid(dst->fd);
	dst->fd = src->fd;
	src->fd = -1; // Invalidate the source fd
})

static void Observer_fanotify_mark(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	EgFsWatch *w = ecs_field(it, EgFsWatch, 0);           // self
	EgFsFanotifyFd *y = ecs_field(it, EgFsFanotifyFd, 1); // shared
	for (int i = 0; i < it->count; ++i, ++w) {
		ecs_assert(w->file != 0, ECS_INVALID_PARAMETER, NULL);
		EcsIdentifier const *p = ecs_get_pair(world, w->file, EcsIdentifier, EcsName);
		ecs_entity_t e = it->entities[i];
		int r = 0;
		if (it->event == EcsOnRemove) {
			fd_fanotify_mark_rm(y->fd, p->value);
		} else if (it->event == EcsOnSet) {
			fd_fanotify_mark_add(y->fd, p->value);
		}
		if (r) {
			ecs_enable(world, e, false);
		}
	} // END FOR LOOP
}

static void Observer_epoll_ctl(ecs_iter_t *it)
{
	ecs_log_set_level(0);
	ecs_world_t *world = it->world;
	EgFsFanotifyFd *y = ecs_field(it, EgFsFanotifyFd, 0); // self
	EgFsEpollFd *o = ecs_field(it, EgFsEpollFd, 1);       // shared
	for (int i = 0; i < it->count; ++i, ++y) {
		ecs_entity_t e = it->entities[i];
		int r = 0;
		if (it->event == EcsOnRemove) {
			ecs_trace("epoll '%s'(%i) rm '%s'(%d)", ecs_get_name(world, ecs_field_src(it, 1)), o->fd, ecs_get_name(world, e), y->fd);
			r = fd_epoll_rm(o->fd, y->fd);
			if (r == 0) {
				ecs_map_remove(&o->map, y->fd);
			}
		} else if (it->event == EcsOnAdd) {
			ecs_trace("epoll '%s'(%i) add '%s'(%d)", ecs_get_name(world, ecs_field_src(it, 1)), o->fd, ecs_get_name(world, e), y->fd);
			r = fd_epoll_add(o->fd, y->fd);
			if (r == 0) {
				ecs_map_insert(&o->map, y->fd, e);
			}
		}
		if (r) {
			ecs_enable(world, e, false);
		}
	}
	ecs_log_set_level(-1);
}

static void System_Read(ecs_iter_t *it)
{
	ecs_log_set_level(0);
	ecs_world_t *world = it->world;
	EgFsFanotifyFd *y = ecs_field(it, EgFsFanotifyFd, 0); // self
	EgFsReady *r = ecs_field(it, EgFsReady, 1);           // shared
	for (int i = 0; i < it->count; ++i, ++y) {
		ecs_entity_t e = it->entities[i];
		//ecs_trace("handle_fanotify_response fd=%d for entity '%s'", y->fd, ecs_get_name(world, e));
		// Should be large enough to hold at least one full fanotify event and its associated info records. 
		// The kernel will return as many events as fit in the buffer, 
		// but you might not get all pending events if your buffer is too small.
		char buf[4096];
		int len = fd_read(y->fd, buf, sizeof(buf));
		ecs_trace("fd_read returned len=%d", len);
		if (len < 0) {
			ecs_enable(world, e, false);
			return;
		}
		fan_handle_notifications(world, EgFsEventOpen, e, FD_FAN_OPEN, buf, len);
		fan_handle_notifications(world, EgFsEventModify, e, FD_FAN_MODIFY, buf, len);
		ecs_remove(world, e, EgFsReady);
	}
	ecs_log_set_level(-1);
}



void EgFsFanotifyImport(ecs_world_t *world)
{
	ECS_IMPORT(world, EgFsEpoll);

	ECS_MODULE(world, EgFsFanotify);
	ecs_set_name_prefix(world, "EgFsFanotify");

	ECS_COMPONENT_DEFINE(world, EgFsFanotifyFd);

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity = ecs_id(EgFsFanotifyFd),
	.members = {
	{.name = "fd", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_set_hooks_id(world, ecs_id(EgFsFanotifyFd),
	&(ecs_type_hooks_t){
	.flags = ECS_TYPE_HOOK_COPY_ILLEGAL,
	.move = ecs_move(EgFsFanotifyFd),
	.dtor = ecs_dtor(EgFsFanotifyFd),
	.ctor = ecs_ctor(EgFsFanotifyFd),
	});

	ecs_observer_init(world,
	&(ecs_observer_desc_t){
	.entity = ecs_entity(world, {.name = "EgFsWatch_EcsOnSet_fannotify_mark", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = Observer_fanotify_mark,
	.events = {EcsOnSet, EcsOnRemove},
	.query.terms = {
	{.id = ecs_id(EgFsWatch)},
	{.id = ecs_id(EgFsFanotifyFd), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsInOutFilter},
	}});

	ecs_observer_init(world,
	&(ecs_observer_desc_t){
	.entity = ecs_entity(world, {.name = "Observer_epoll_ctl", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = Observer_epoll_ctl,
	.events = {EcsOnAdd, EcsOnRemove},
	.query.terms = {
	{.id = ecs_id(EgFsFanotifyFd)},
	{.id = ecs_id(EgFsEpollFd), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsInOutFilter},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "System_Read", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_Read,
	.query.terms =
	{
	{.id = ecs_id(EgFsFanotifyFd), .src.id = EcsSelf},
	{.id = ecs_id(EgFsReady), .src.id = EcsSelf},
	}});



}
