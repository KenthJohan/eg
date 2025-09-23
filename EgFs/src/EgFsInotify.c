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
			ecs_trace("Removing watch fd=%i (%s) from inotify fd=%i (%s)", w->fd, ecs_get_name(world, parent), y->fd, ecs_get_name(world, e));
			rv = fd_inotify_rm(y->fd, w->fd);
		} else if (it->event == EcsOnSet) {
			EcsIdentifier const *p = ecs_get_pair(world, w->file, EcsIdentifier, EcsName);
			ecs_trace("Adding watch '%s' (%s) to epoll fd=%i (%s)", p->value, ecs_get_name(world, parent), y->fd, ecs_get_name(world, e));
			rv = fd_inotify_add(y->fd, p->value, IN_ALL_EVENTS);
			if (rv >= 0) {
				w->fd = rv; // Store the watch descriptor
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
	EgFsEpollFd *o = ecs_field(it, EgFsEpollFd, 1);     // shared
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

static void /* Display information from inotify_event structure */
displayInotifyEvent(struct inotify_event *i)
{
	printf("    wd =%2d; ", i->wd);
	if (i->cookie > 0)
		printf("cookie =%4d; ", i->cookie);

	printf("mask = ");
	if (i->mask & IN_ACCESS)
		printf("IN_ACCESS ");
	if (i->mask & IN_ATTRIB)
		printf("IN_ATTRIB ");
	if (i->mask & IN_CLOSE_NOWRITE)
		printf("IN_CLOSE_NOWRITE ");
	if (i->mask & IN_CLOSE_WRITE)
		printf("IN_CLOSE_WRITE ");
	if (i->mask & IN_CREATE)
		printf("IN_CREATE ");
	if (i->mask & IN_DELETE)
		printf("IN_DELETE ");
	if (i->mask & IN_DELETE_SELF)
		printf("IN_DELETE_SELF ");
	if (i->mask & IN_IGNORED)
		printf("IN_IGNORED ");
	if (i->mask & IN_ISDIR)
		printf("IN_ISDIR ");
	if (i->mask & IN_MODIFY)
		printf("IN_MODIFY ");
	if (i->mask & IN_MOVE_SELF)
		printf("IN_MOVE_SELF ");
	if (i->mask & IN_MOVED_FROM)
		printf("IN_MOVED_FROM ");
	if (i->mask & IN_MOVED_TO)
		printf("IN_MOVED_TO ");
	if (i->mask & IN_OPEN)
		printf("IN_OPEN ");
	if (i->mask & IN_Q_OVERFLOW)
		printf("IN_Q_OVERFLOW ");
	if (i->mask & IN_UNMOUNT)
		printf("IN_UNMOUNT ");
	printf("\n");

	if (i->len > 0)
		printf("        name = %s\n", i->name);
}

static void System_Read(ecs_iter_t *it)
{
	ecs_log_set_level(0);
	ecs_world_t *world = it->world;
	EgFsFanotifyFd *y = ecs_field(it, EgFsFanotifyFd, 0); // self
	EgFsReady *r = ecs_field(it, EgFsReady, 1);           // shared
	for (int i = 0; i < it->count; ++i, ++y) {
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

		for (char *p = buf; p < buf + len;) {
			struct inotify_event *event = (struct inotify_event *)p;
			displayInotifyEvent(event);
			p += sizeof(struct inotify_event) + event->len;
		}

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
