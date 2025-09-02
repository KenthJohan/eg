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

ECS_COMPONENT_DECLARE(EgFsPath);
ECS_COMPONENT_DECLARE(EgFsEpoll);
ECS_COMPONENT_DECLARE(EgFsInotify);
ECS_COMPONENT_DECLARE(EgFsFanotify);
ECS_COMPONENT_DECLARE(EgFsWatch);
ECS_COMPONENT_DECLARE(EgFsLookup);
ECS_ENTITY_DECLARE(EgFs);
ECS_ENTITY_DECLARE(EgFsFiles);
ECS_ENTITY_DECLARE(EgFsDescriptors);

/*
The epoll API performs a similar task to poll(2): monitoring
multiple file descriptors to see if I/O is possible on any of
them.  The epoll API can be used either as an edge-triggered or a
level-triggered interface and scales well to large numbers of
watched file descriptors.

The destructor should free resources.
The move hook should move resources from one location to another.
*/

ECS_CTOR(EgFsLookup, ptr, {
	flecs_hashmap_init_(
	&ptr->hm,
	ECS_SIZEOF(EgFsPath),
	ECS_SIZEOF(uint64_t),
	EgFsPath_name_index_hash,
	EgFsPath_name_index_compare,
	NULL);
})

ECS_DTOR(EgFsLookup, ptr, {
	flecs_hashmap_fini(&ptr->hm);
})

ECS_MOVE(EgFsLookup, dst, src, {
	flecs_hashmap_fini(&dst->hm);
	dst->hm = src->hm;
	src->hm = (ecs_hashmap_t){0};
})

/*
The destructor should free resources.
The move hook should move resources from one location to another.
The copy hook should copy resources from one location to another.
*/

ECS_CTOR(EgFsPath, ptr, {
	ecs_os_zeromem(ptr);
})

ECS_DTOR(EgFsPath, ptr, {
	ecs_os_free((void *)ptr->value);
})

ECS_MOVE(EgFsPath, dst, src, {
	ecs_os_free((void *)dst->value);
	*dst = *src;
	ecs_os_zeromem(src);
})

ECS_COPY(EgFsPath, dst, src, {
	ecs_os_free((void *)dst->value);
	dst->value = ecs_os_strdup(src->value);
})

/*
The epoll API performs a similar task to poll(2): monitoring
multiple file descriptors to see if I/O is possible on any of
them.  The epoll API can be used either as an edge-triggered or a
level-triggered interface and scales well to large numbers of
watched file descriptors.

The destructor should free resources.
The move hook should move resources from one location to another.
*/

ECS_CTOR(EgFsEpoll, ptr, {
	int epoll_fd = epoll_create1(0);
	if (epoll_fd < 0) {
		perror("epoll_create1");
		ptr->fd = -1;
	} else {
		ptr->fd = epoll_fd;
		ecs_map_init(&ptr->map, NULL);
	}
})

ECS_DTOR(EgFsEpoll, ptr, {
	if (ptr->fd > 0) {
		close(ptr->fd);
	}
	ecs_map_fini(&ptr->map);
})

ECS_MOVE(EgFsEpoll, dst, src, {
	if (dst->fd > 0) {
		close(dst->fd);
	}
	dst->fd = src->fd;
	src->fd = -1; // Invalidate the source fd

	ecs_map_fini(&dst->map);
	dst->map = src->map;
	src->map = (ecs_map_t){0};
})

/*
The fanotify API provides notification and interception of
    filesystem events.  Use cases include virus scanning and
    hierarchical storage management.  In the original fanotify API,
    only a limited set of events was supported.  In particular, there
    was no support for create, delete, and move events.  The support
    for those events was added in Linux 5.1.  (See inotify(7) for
    details of an API that did notify those events pre Linux 5.1.)

    Additional capabilities compared to the inotify(7) API include the
    ability to monitor all of the objects in a mounted filesystem, the
    ability to make access permission decisions, and the possibility
    to read or modify files before access by other applications.

    The following system calls are used with this API:
    fanotify_init(2), fanotify_mark(2), read(2), write(2), and
    close(2).
*/

int fanotify_init1 ()
{
    int fd = fanotify_init(FAN_CLASS_NOTIF, O_RDONLY);
	if (fd < 0) {
		perror("fanotify_init");
		return -1;
	}
	return fd;
}

ECS_CTOR(EgFsFanotify, ptr, {
	int fan_fd = fanotify_init1();
	if (fan_fd < 0) {
		perror("fanotify_init");
		ptr->fd = -1;
	} else {
		ptr->fd = fan_fd;
	}
})

// The destructor should free resources.
ECS_DTOR(EgFsFanotify, ptr, {
	if (ptr->fd > 0) {
		close(ptr->fd);
	}
})

ECS_MOVE(EgFsFanotify, dst, src, {
	if (dst->fd > 0) {
		close(dst->fd);
	}
	dst->fd = src->fd;
	src->fd = -1; // Invalidate the source fd
})

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

ECS_CTOR(EgFsInotify, ptr, {
	int inotify_fd = inotify_init1(IN_NONBLOCK);
	if (inotify_fd < 0) {
		perror("inotify_init1");
		ptr->fd = -1;
	} else {
		ptr->fd = inotify_fd;
	}
})

// The destructor should free resources.
ECS_DTOR(EgFsInotify, ptr, {
	if (ptr->fd > 0) {
		close(ptr->fd);
	}
})

ECS_MOVE(EgFsInotify, dst, src, {
	if (dst->fd > 0) {
		close(dst->fd);
	}
	dst->fd = src->fd;
	src->fd = -1; // Invalidate the source fd
})

static void Observer_epoll_ctl(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	EgFsInotify *y = ecs_field(it, EgFsInotify, 0); // self
	EgFsEpoll *o = ecs_field(it, EgFsEpoll, 1);     // shared
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

static void EgFsWatch_EcsOnSet_inotify_add_watch(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	EgFsWatch *w = ecs_field(it, EgFsWatch, 0);     // self
	EgFsInotify *y = ecs_field(it, EgFsInotify, 1); // shared
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

static void EgFsWatch_EcsOnSet_fannotify_add_watch(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	EgFsWatch *w = ecs_field(it, EgFsWatch, 0);       // self
	EgFsFanotify *y = ecs_field(it, EgFsFanotify, 1); // shared
	for (int i = 0; i < it->count; ++i, ++w) {
		ecs_entity_t e = it->entities[i];
		ecs_assert(w->file != 0, ECS_INVALID_PARAMETER, NULL);
		EgFsPath const *p = ecs_get(world, w->file, EgFsPath);
		int r = fanotify_mark(y->fd,
		FAN_MARK_ADD | FAN_MARK_MOUNT, FAN_CREATE | FAN_DELETE | FAN_MODIFY | FAN_ACCESS, AT_FDCWD,
		p->value);
		if (r) {
			perror("fanotify_mark");
		} else {
			ecs_trace("Added fanotify watch for %s on entity %s", p->value, ecs_get_name(world, e));
		}
	} // END FOR LOOP
}

static void Observer_inotify_rm_watch(ecs_iter_t *it)
{
	// ecs_world_t *world = it->world;
	EgFsWatch *w = ecs_field(it, EgFsWatch, 0); // self
	EgFsEpoll *l = ecs_field(it, EgFsEpoll, 1); // shared
	for (int i = 0; i < it->count; ++i, ++w) {
		int rv = inotify_rm_watch(l->fd, w->fd);
		if (rv < 0) {
			perror("inotify_rm_watch");
		}
	} // END FOR LOOP
}

#define MAX_EVENTS 10
#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (MAX_EVENTS * (EVENT_SIZE + NAME_MAX + 1))

static void enqueue_inotify_event(ecs_world_t *world, int fd)
{
	char buffer[BUF_LEN];
	ssize_t len = read(fd, buffer, BUF_LEN);
	if (len < 0 && errno != EAGAIN) {
		perror("read");
		return;
	}
	ssize_t pos = 0;
	while (pos < len) {
		struct inotify_event *ev = (struct inotify_event *)&buffer[pos];
		printf("Event: wd=%d mask=0x%x cookie=%u len=%u, ", ev->wd, ev->mask, ev->cookie, ev->len);
		if (ev->len > 0)
			printf(" name=%s", ev->name);
		printf("\n");
		pos += EVENT_SIZE + ev->len;
	}
}

#define FANOTIFY_BUF_SIZE 4096

static void enqueue_fanotify_event(ecs_world_t *world, int fd)
{
	char buf[FANOTIFY_BUF_SIZE];
	ssize_t len = read(fd, buf, sizeof(buf));
	if (len < 0) {
		perror("read from fanotify fd");
		return;
	}

	struct fanotify_event_metadata *metadata;
	for (metadata = (struct fanotify_event_metadata *)buf;
	     FAN_EVENT_OK(metadata, len);
	     metadata = FAN_EVENT_NEXT(metadata, len)) {

		if (metadata->fd >= 0) {
			char path[PATH_MAX];
			snprintf(path, sizeof(path), "/proc/self/fd/%d", metadata->fd);
			ssize_t path_len = readlink(path, path, sizeof(path) - 1);
			if (path_len > 0)
				path[path_len] = '\0';
			else
				strcpy(path, "unknown");

			printf("Event: ");
			if (metadata->mask & FAN_CREATE)
				printf("CREATE ");
			if (metadata->mask & FAN_DELETE)
				printf("DELETE ");
			if (metadata->mask & FAN_MODIFY)
				printf("MODIFY ");
			if (metadata->mask & FAN_ACCESS)
				printf("ACCESS ");
			printf("on %s\n", path);

			close(metadata->fd);
		}
	}
}

static void System_epoll(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	EgFsEpoll *o = ecs_field(it, EgFsEpoll, 0); // self
	for (int i = 0; i < it->count; ++i, ++o) {
		struct epoll_event events[MAX_EVENTS];
		int nfds = epoll_wait(o->fd, events, MAX_EVENTS, 100);
		if (nfds == -1) {
			if (errno == EINTR)
				continue;
			perror("epoll_wait");
			break;
		}

		for (int j = 0; j < nfds; ++j) {
			ecs_entity_t *e = ecs_map_get(&o->map, events[j].data.fd);
			if (e == NULL) {
				continue;
			}
			if (ecs_has(world, *e, EgFsInotify)) {
				enqueue_inotify_event(world, events[j].data.fd);
			}
			if (ecs_has(world, *e, EgFsFanotify)) {
				enqueue_fanotify_event(world, events[j].data.fd);
			}
		}
	} // END FOR LOOP
}

static void lookup(const ecs_function_ctx_t *ctx, int argc, const ecs_value_t *argv, ecs_value_t *result)
{
	(void)ctx;
	(void)argc;
	ecs_world_t *world = ctx->world;
	const char *path = *(char **)argv[0].ptr;
	EgFsLookup const *lookup = ecs_singleton_get(world, EgFsLookup);
	ecs_entity_t a = EgFsPath_name_index_find(&lookup->hm, path, 0, 0);
	if (a == 0) {
		// ecs_entity_t parent = ecs_pair(ecs_id(EgFsLookup), ecs_id(EgFsLookup));
		a = ecs_entity_init(world, &(ecs_entity_desc_t){
		                           .parent = EgFsFiles,
		                           });
		ecs_set(world, a, EgFsPath, {.value = path});
		ecs_doc_set_name(world, a, path);
	}
	*(int64_t *)result->ptr = a;
}

void EgFsImport(ecs_world_t *world)
{
	ECS_MODULE_DEFINE(world, EgFs);
	ecs_set_name_prefix(world, "EgFs");

	ECS_COMPONENT_DEFINE(world, EgFsPath);
	ECS_COMPONENT_DEFINE(world, EgFsEpoll);
	ECS_COMPONENT_DEFINE(world, EgFsFanotify);
	ECS_COMPONENT_DEFINE(world, EgFsInotify);
	ECS_COMPONENT_DEFINE(world, EgFsWatch);
	ECS_COMPONENT_DEFINE(world, EgFsLookup);
	ECS_ENTITY_DEFINE(world, EgFsFiles);
	ECS_ENTITY_DEFINE(world, EgFsDescriptors);

	{
		ecs_entity_t m = ecs_function_init(world,
		&(ecs_function_desc_t){
		.name = "entfile",
		.return_type = ecs_id(ecs_entity_t),
		.params = {
		{.name = "path", .type = ecs_id(ecs_string_t)}},
		.callback = lookup});
		ecs_doc_set_brief(world, m, "Lookup child by name");
	}

	ecs_set_hooks_id(world, ecs_id(EgFsLookup),
	&(ecs_type_hooks_t){
	.ctor = ecs_ctor(EgFsLookup),
	.move = ecs_move(EgFsLookup),
	.dtor = ecs_dtor(EgFsLookup),
	});

	ecs_set_hooks_id(world, ecs_id(EgFsPath),
	&(ecs_type_hooks_t){
	.ctor = ecs_ctor(EgFsPath),
	.move = ecs_move(EgFsPath),
	.copy = ecs_copy(EgFsPath),
	.dtor = ecs_dtor(EgFsPath),
	.on_set = ecs_on_set(EgFsPath),
	});

	ecs_set_hooks_id(world, ecs_id(EgFsEpoll),
	&(ecs_type_hooks_t){
	.flags = ECS_TYPE_HOOK_COPY_ILLEGAL,
	.move = ecs_move(EgFsEpoll),
	.dtor = ecs_dtor(EgFsEpoll),
	.ctor = ecs_ctor(EgFsEpoll),
	});

	ecs_set_hooks_id(world, ecs_id(EgFsFanotify),
	&(ecs_type_hooks_t){
	.flags = ECS_TYPE_HOOK_COPY_ILLEGAL,
	.move = ecs_move(EgFsFanotify),
	.dtor = ecs_dtor(EgFsFanotify),
	.ctor = ecs_ctor(EgFsFanotify),
	});

	ecs_set_hooks_id(world, ecs_id(EgFsInotify),
	&(ecs_type_hooks_t){
	.flags = ECS_TYPE_HOOK_COPY_ILLEGAL,
	.move = ecs_move(EgFsInotify),
	.dtor = ecs_dtor(EgFsInotify),
	.ctor = ecs_ctor(EgFsInotify),
	});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity = ecs_id(EgFsPath),
	.members = {
	{.name = "value", .type = ecs_id(ecs_string_t)},
	{.name = "length", .type = ecs_id(ecs_i32_t)},
	{.name = "hash", .type = ecs_id(ecs_u64_t)},
	}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity = ecs_id(EgFsEpoll),
	.members = {
	{.name = "fd", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity = ecs_id(EgFsInotify),
	.members = {
	{.name = "fd", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity = ecs_id(EgFsWatch),
	.members = {
	{.name = "fd", .type = ecs_id(ecs_i32_t)},
	{.name = "file", .type = ecs_id(ecs_entity_t)},
	}});

	ecs_observer_init(world,
	&(ecs_observer_desc_t){
	.entity = ecs_entity(world, {.name = "Observer_epoll_ctl", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = Observer_epoll_ctl,
	.events = {EcsOnAdd, EcsOnRemove},
	.query.terms = {
	{.id = ecs_id(EgFsInotify)},
	{.id = ecs_id(EgFsEpoll), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsInOutFilter},
	}});

	ecs_observer_init(world,
	&(ecs_observer_desc_t){
	.entity = ecs_entity(world, {.name = "EgFsWatch_EcsOnSet_inotify_add_watch", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = EgFsWatch_EcsOnSet_inotify_add_watch,
	.events = {EcsOnSet},
	.query.terms = {
	{.id = ecs_id(EgFsWatch)},
	{.id = ecs_id(EgFsInotify), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsInOutFilter},
	}});

	ecs_observer_init(world,
	&(ecs_observer_desc_t){
	.entity = ecs_entity(world, {.name = "EgFsWatch_EcsOnSet_fannotify_add_watch", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = EgFsWatch_EcsOnSet_fannotify_add_watch,
	.events = {EcsOnSet},
	.query.terms = {
	{.id = ecs_id(EgFsWatch)},
	{.id = ecs_id(EgFsInotify), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsInOutFilter},
	}});

	ecs_observer_init(world,
	&(ecs_observer_desc_t){
	.entity = ecs_entity(world, {.name = "Observer_inotify_rm_watch", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = Observer_inotify_rm_watch,
	.events = {EcsOnRemove},
	.query.terms = {
	{.id = ecs_id(EgFsWatch)},
	{.id = ecs_id(EgFsInotify), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsInOutFilter},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "System_epoll", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_epoll,
	.query.terms =
	{
	{.id = ecs_id(EgFsEpoll), .src.id = EcsSelf},
	}});

	ecs_singleton_add(world, EgFsLookup);
}
