/*
https://wiki.libsdl.org/SDL3/SDL_ReadIO
https://github.com/SanderMertens/flecs/blob/master/examples/c/entities/hooks/src/main.c
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/src/datastructures/vec.c#L118
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/include/flecs/datastructures/vec.h
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/src/addons/alerts.c#L39
https://github.com/libsdl-org/SDL/blob/0fcaf47658be96816a851028af3e73256363a390/test/testautomation_iostream.c#L477
*/

// Define _GNU_SOURCE, Otherwise we don't get O_LARGEFILE
#define _GNU_SOURCE

#include "EgFs.h"
#include "EgFs/EgFsPath.h"
#include "EgFs/EgFsInotify.h"

#include <stdlib.h>
#include <stdio.h>
#include <ecsx.h>
#include <egmisc.h>
#include <sys/inotify.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <sys/fanotify.h>
#include <linux/limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/fanotify.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "fd.h"

ECS_COMPONENT_DECLARE(EgFsInotifyFd);

/*
The inotify API provides a mechanism for monitoring filesystem
events.  Inotify can be used to monitor individual files, or to
monitor directories.  When a directory is monitored, inotify will
return events for the directory itself, and for files inside the
directory.

The following system calls are used with this API:

•  inotify_init(2) creates an inotify instance and returns a file
    descriptor referring to the inotify instance.  The more recent
    inotify_init1(2) is like inotify_init(2), but has a flags
    argument that provides access to some extra functionality.

•  inotify_add_watch(2) manipulates the "watch list" associated
    with an inotify instance.  Each item ("watch") in the watch
    list specifies the pathname of a file or directory, along with
    some set of events that the kernel should monitor for the file
    referred to by that pathname.  inotify_add_watch(2) either
    creates a new watch item, or modifies an existing watch.  Each
    watch has a unique "watch descriptor", an integer returned by
    inotify_add_watch(2) when the watch is created.

•  When events occur for monitored files and directories, those
    events are made available to the application as structured data
    that can be read from the inotify file descriptor using read(2)
    (see below).

•  inotify_rm_watch(2) removes an item from an inotify watch list.

•  When all file descriptors referring to an inotify instance have
    been closed (using close(2)), the underlying object and its
    resources are freed for reuse by the kernel; all associated
    watches are automatically freed.
*/

ECS_CTOR(EgFsInotifyFd, ptr, {
	ptr->fd = fd_inotify_init1();
})

// The destructor should free resources.
ECS_DTOR(EgFsInotifyFd, ptr, {
	fd_close_valid(ptr->fd);
})

ECS_MOVE(EgFsInotifyFd, dst, src, {
	fd_close_valid(dst->fd);
	dst->fd = src->fd;
	src->fd = -1; // Invalidate the source fd
})

static void EgFsWatch_EcsOnSet_inotify_add_watch(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	EgFsWatch *w = ecs_field(it, EgFsWatch, 0);         // self
	EgFsInotifyFd *y = ecs_field(it, EgFsInotifyFd, 1); // shared
	for (int i = 0; i < it->count; ++i, ++w) {
		ecs_entity_t e = it->entities[i];
		// print entity
		ecs_trace("Adding watch for entity %s", ecs_get_name(world, e));
		ecs_assert(w->file != 0, ECS_INVALID_PARAMETER, NULL);
		EgFsPath const *p = ecs_get(world, w->file, EgFsPath);
		if (!p) {
			ecs_err("Failed to get filename for watch");
			continue;
		}
		ecs_doc_set_name(world, e, p->value);
		ecs_trace("Adding watch fd (%s) to epoll (%s)", ecs_get_name(world, e), ecs_get_name(world, ecs_field_src(it, 1)));
		int wd = inotify_add_watch(y->fd, p->value, IN_ALL_EVENTS);
		if (w->fd >= 0) {
			// Remove previous watch
			int rv = inotify_rm_watch(y->fd, w->fd);
			if (rv < 0) {
				perror("inotify_rm_watch");
			}
		}
		if (wd < 0) {
			perror("inotify_add_watch");
			w->fd = -1;
		} else {
			w->fd = wd;
		}
	} // END FOR LOOP
}

static void Observer_inotify_rm_watch(ecs_iter_t *it)
{
	// ecs_world_t *world = it->world;
	EgFsWatch *w = ecs_field(it, EgFsWatch, 0); // self
	EgFsEpollFd *l = ecs_field(it, EgFsEpollFd, 1); // shared
	for (int i = 0; i < it->count; ++i, ++w) {
		int rv = inotify_rm_watch(l->fd, w->fd);
		if (rv < 0) {
			perror("inotify_rm_watch");
		}
	} // END FOR LOOP
}



static void Observer_epoll_ctl(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	EgFsInotifyFd *y = ecs_field(it, EgFsInotifyFd, 0); // self
	EgFsEpollFd *o = ecs_field(it, EgFsEpollFd, 1);     // shared
	for (int i = 0; i < it->count; ++i, ++y) {
		ecs_entity_t e = it->entities[i];
		if (it->event == EcsOnRemove) {
			ecs_map_remove(&o->map, y->fd);
		} else if (it->event == EcsOnAdd) {
			ecs_trace("Adding inotify fd (%s) to epoll (%s)", ecs_get_name(world, e), ecs_get_name(world, ecs_field_src(it, 1)));
			struct epoll_event event;
			event.events = EPOLLIN;
			event.data.fd = y->fd;
			if (epoll_ctl(o->fd, EPOLL_CTL_ADD, y->fd, &event) == 0) {
				ecs_map_insert(&o->map, y->fd, e);
			} else {
				perror("epoll_ctl");
			}
		}
	}
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
	.entity = ecs_entity(world, {.name = "EgFsWatch_EcsOnSet_inotify_add_watch", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = EgFsWatch_EcsOnSet_inotify_add_watch,
	.events = {EcsOnSet},
	.query.terms = {
	{.id = ecs_id(EgFsWatch)},
	{.id = ecs_id(EgFsInotifyFd), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsInOutFilter},
	}});

	ecs_observer_init(world,
	&(ecs_observer_desc_t){
	.entity = ecs_entity(world, {.name = "Observer_inotify_rm_watch", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = Observer_inotify_rm_watch,
	.events = {EcsOnRemove},
	.query.terms = {
	{.id = ecs_id(EgFsWatch)},
	{.id = ecs_id(EgFsInotifyFd), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsInOutFilter},
	}});
}
