#include "EgFs.h"
#include "EgFs/EgFsInotify.h"
#include "fd.h"

ECS_COMPONENT_DECLARE(EgFsInotifyFd);

ECS_CTOR(EgFsInotifyFd, ptr, {
	ptr->fd = fd_inotify_init1();
	ecs_map_init(&ptr->map, NULL);
})

// The destructor should free resources.
ECS_DTOR(EgFsInotifyFd, ptr, {
	fd_close_valid(ptr->fd);
	ecs_map_fini(&ptr->map);
})

ECS_MOVE(EgFsInotifyFd, dst, src, {
	fd_close_valid(dst->fd);
	dst->fd = src->fd;
	src->fd = -1; // Invalidate the source fd
	ecs_map_fini(&dst->map);
	dst->map = src->map;
	src->map = (ecs_map_t){0};
})

static void Observer_inotify_ctl(ecs_iter_t *it)
{
	ecs_log_set_level(0);
	ecs_world_t *world = it->world;
	EgFsWatch *w = ecs_field(it, EgFsWatch, 0);         // self
	EgFsInotifyFd *y = ecs_field(it, EgFsInotifyFd, 1); // shared
	for (int i = 0; i < it->count; ++i, ++w) {
		ecs_entity_t e = it->entities[i];
		ecs_entity_t parent = ecs_field_src(it, 1);
		int rv = 0;
		if (it->event == EcsOnRemove) {
			EcsIdentifier const *p = ecs_get_pair(world, w->file, EcsIdentifier, EcsName);
			ecs_trace("Removing watch '%s' fd=%i path='%s' from inotify '%s' fd=%i", ecs_get_name(world, parent), w->fd, p->value, ecs_get_name(world, e), y->fd);
			rv = fd_inotify_rm(y->fd, w->fd);
		} else if (it->event == EcsOnSet) {
			EcsIdentifier const *p = ecs_get_pair(world, w->file, EcsIdentifier, EcsName);
			ecs_trace("Adding watch '%s' path='%s' to inotify '%s' fd=%i", ecs_get_name(world, parent), p->value, ecs_get_name(world, e), y->fd);
			rv = fd_inotify_add(y->fd, p->value, FD_IN_ALL_EVENTS);
			if (rv >= 0) {
				w->fd = rv; // Store the watch descriptor
				ecs_map_insert(&y->map, w->fd, e);
			} else {
				w->fd = -1;
				ecs_enable(it->world, e, false);
			}
		}
		if (rv < 0) {
			ecs_enable(it->world, e, false);
		}
	} // END FOR LOOP
	ecs_log_set_level(-1);
}

static void Observer_epoll_ctl(ecs_iter_t *it)
{
	ecs_log_set_level(0);
	ecs_world_t *world = it->world;
	EgFsInotifyFd *y = ecs_field(it, EgFsInotifyFd, 0); // self
	EgFsEpollFd *o = ecs_field(it, EgFsEpollFd, 1);     // shared, parent
	for (int i = 0; i < it->count; ++i, ++y) {
		ecs_entity_t e = it->entities[i];
		ecs_entity_t parent = ecs_field_src(it, 1);
		int rv = 0;
		if (it->event == EcsOnRemove) {
			rv = fd_epoll_rm(o->fd, y->fd);
			ecs_trace("Removing inotify fd=%i (%s) from epoll fd=%i (%s)", y->fd, ecs_get_name(world, e), o->fd, ecs_get_name(world, parent));
			if (rv == 0) {
				ecs_map_remove(&o->map, y->fd);
			}
		} else if (it->event == EcsOnAdd) {
			ecs_trace("Adding inotify fd=%i (%s) to epoll fd=%i (%s)", y->fd, ecs_get_name(world, e), o->fd, ecs_get_name(world, parent));
			rv = fd_epoll_add(o->fd, y->fd);
			if (rv == 0) {
				ecs_map_insert(&o->map, y->fd, e);
			}
		}
		if (rv != 0) {
			ecs_enable(it->world, e, false);
		}
	}
	ecs_log_set_level(-1);
}

static void System_Read(ecs_iter_t *it)
{
	ecs_log_set_level(-1);
	ecs_world_t *world = it->world;
	EgFsInotifyFd *y = ecs_field(it, EgFsInotifyFd, 0); // self
	EgFsReady *r = ecs_field(it, EgFsReady, 1);         // self
	for (int i = 0; i < it->count; ++i, ++y, ++r) {
		ecs_entity_t e = it->entities[i];
		// ecs_trace("handle_fanotify_response fd=%d for entity '%s'", y->fd, ecs_get_name(world, e));
		//  Should be large enough to hold at least one full fanotify event and its associated info records.
		//  The kernel will return as many events as fit in the buffer,
		//  but you might not get all pending events if your buffer is too small.
		char buf[4096];
		int len = fd_read(y->fd, buf, sizeof(buf));
		ecs_trace("fd_read returned len=%d", len);
		if (len < 0) {
			ecs_enable(world, e, false);
			return;
		}
		fd_handle_inotify_events(world, EgFsEventModify, EgFsCwd, FD_IN_MODIFY, &y->map, buf, len);
		ecs_remove(world, e, EgFsReady);
	}
	ecs_log_set_level(-1);
}

void EgFsInotifyImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgFsInotify);
	ecs_set_name_prefix(world, "EgFsInotify");

	ECS_COMPONENT_DEFINE(world, EgFsInotifyFd);

	ecs_set_hooks_id(world, ecs_id(EgFsInotifyFd),
	&(ecs_type_hooks_t){
	.flags = ECS_TYPE_HOOK_COPY_ILLEGAL,
	.move = ecs_move(EgFsInotifyFd),
	.dtor = ecs_dtor(EgFsInotifyFd),
	.ctor = ecs_ctor(EgFsInotifyFd),
	});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity = ecs_id(EgFsInotifyFd),
	.members = {
	{.name = "fd", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_observer_init(world,
	&(ecs_observer_desc_t){
	.entity = ecs_entity(world, {.name = "Observer_epoll_ctl", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = Observer_epoll_ctl,
	.events = {EcsOnAdd, EcsOnRemove},
	.query.terms = {
	{.id = ecs_id(EgFsInotifyFd)},
	{.id = ecs_id(EgFsEpollFd), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsInOutFilter},
	}});

	ecs_observer_init(world,
	&(ecs_observer_desc_t){
	.entity = ecs_entity(world, {.name = "Observer_inotify_ctl", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = Observer_inotify_ctl,
	.events = {EcsOnSet, EcsOnRemove},
	.query.terms = {
	{.id = ecs_id(EgFsWatch)},
	{.id = ecs_id(EgFsInotifyFd), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsInOutFilter},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "System_Read", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_Read,
	.query.terms =
	{
	{.id = ecs_id(EgFsInotifyFd), .src.id = EcsSelf},
	{.id = ecs_id(EgFsReady), .src.id = EcsSelf},
	}});
}
