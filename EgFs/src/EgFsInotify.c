#include "EgFs.h"
#include "EgFs/EgFsInotify.h"
#include "fd.h"
#include <stdio.h>

ECS_COMPONENT_DECLARE(EgFsInotifyCreate);

ECS_CTOR(EgFsInotifyCreate, ptr, {
	ecs_map_init(&ptr->map, NULL);
})

// The destructor should free resources.
ECS_DTOR(EgFsInotifyCreate, ptr, {
	ecs_map_fini(&ptr->map);
})

ECS_MOVE(EgFsInotifyCreate, dst, src, {
	ecs_map_fini(&dst->map);
	dst->map = src->map;
	src->map = (ecs_map_t){0};
})

void get_entity_path(ecs_world_t *world, ecs_entity_t e, char *buf, size_t size)
{
	char *p = ecs_get_path_w_sep(world, EgFsCwd, e, "/", NULL);
	// Skip the $CWD/ part
	ecs_os_snprintf(buf, size, "./%s", p);
	ecs_os_free(p);
}

static void Observer_inotify_ctl(ecs_iter_t *it)
{
	ecs_log_set_level(0);
	ecs_world_t *world = it->world;
	EgFsWatch *w = ecs_field(it, EgFsWatch, 0);                 // self, string path
	EgFsFd *y = ecs_field(it, EgFsFd, 1);                       // shared, inotify fd
	EgFsInotifyCreate *c = ecs_field(it, EgFsInotifyCreate, 2); // shared
	// ecs_entity_t parent = ecs_field_src(it, 1);
	for (int i = 0; i < it->count; ++i, ++w) {
		ecs_entity_t e = it->entities[i];
		char path[1024];
		get_entity_path(world, w->path1, path, sizeof(path));
		int rv = 0;
		if (it->event == EcsOnRemove) {
			rv = fd_inotify_rm(y->fd, w->fd);
		} else if (it->event == EcsOnSet) {
			rv = fd_inotify_add(y->fd, path, FD_IN_ALL_EVENTS);
			// On success, this function returns a non-negative watch descriptor.
			if (rv >= 0) {
				w->fd = rv; // Store the watch descriptor
				ecs_map_insert(&c->map, w->fd, e);
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

void handle_events(ecs_world_t *world, ecs_map_t *map, char *buffer, int len)
{
	uint32_t mask = FD_IN_MODIFY;
	fd_inotify_event *i;
	for (char *p = buffer; p < buffer + len; p += sizeof(fd_inotify_event) + i->len) {
		i = (fd_inotify_event *)p;
		if (i->len <= 0) {
			continue;
		}
		if ((i->mask & mask) == 0) {
			continue;
		}
		// displayInotifyEvent(i);
		ecs_entity_t *ew = ecs_map_get(map, i->wd);
		if (ew == NULL) {
			continue;
		}
		EgFsWatch const *w = ecs_get(world, *ew, EgFsWatch);
		if (w == NULL) {
			continue;
		}
		if (w->path1 == 0) {
			continue;
		}
		char const *parent_path = ecs_get_name(world, w->path1);

		char fullpath[1024];
		snprintf(fullpath, sizeof(fullpath), "./%s/%s", parent_path, i->name);
		ecs_trace("fullpath = '%s'\n", fullpath);

		ecs_entity_t e = EgFs_create_path_entity(world, fullpath);
		if (e == 0) {
			continue;
		}
		if (w->prefab) {
			ecs_add_pair(world, e, EcsIsA, w->prefab);
		}

		if (i->mask & FD_IN_MODIFY) {
			ecs_enqueue(world,
			&(ecs_event_desc_t){
			.event = EgFsEventModify,
			.entity = e,
			.ids = &(ecs_type_t){(ecs_id_t[]){EgFsFile}, 1},
			});
		}
	}
}

static void System_Read(ecs_iter_t *it)
{
	ecs_log_set_level(-1);
	ecs_world_t *world = it->world;
	EgFsFd *y = ecs_field(it, EgFsFd, 0);                       // self
	EgFsInotifyCreate *c = ecs_field(it, EgFsInotifyCreate, 1); // self
	EgFsReady *r = ecs_field(it, EgFsReady, 2);                 // self
	for (int i = 0; i < it->count; ++i, ++y, ++r, ++c) {
		ecs_entity_t e = it->entities[i];
		// ecs_trace("handle_fanotify_response fd=%d for entity '%s'", y->fd, ecs_get_name(world, e));
		//  Should be large enough to hold at least one full fanotify event and its associated info records.
		//  The kernel will return as many events as fit in the buffer,
		//  but you might not get all pending events if your buffer is too small.
		char buf[4096];
		int len = fd_read(y->fd, buf, sizeof(buf));
		if (len < 0) {
			ecs_enable(world, e, false);
			return;
		}
		handle_events(world, &c->map, buf, len);
		ecs_remove(world, e, EgFsReady);
	}
	ecs_log_set_level(-1);
}

static void System_Create(ecs_iter_t *it)
{
	ecs_log_set_level(0);
	ecs_world_t *world = it->world;
	EgFsInotifyCreate *c = ecs_field(it, EgFsInotifyCreate, 0); // self
	for (int i = 0; i < it->count; ++i, ++c) {
		ecs_entity_t e = it->entities[i];
		int fd = fd_inotify_init1();
		if (fd >= 0) {
			ecs_set_pair(world, e, EgFsFd, ecs_id(EgFsInotifyCreate), {fd});
		} else {
			ecs_enable(world, e, false);
		}
	}
	ecs_log_set_level(-1);
}

void System_test(ecs_iter_t *it)
{
	ecs_log_set_level(0);
	ecs_world_t *world = it->world;
	ecs_entity_t parent = ecs_field_src(it, 2);
	// print only parent name
	ecs_trace("System_test for parent '%s'", ecs_get_name(world, parent));
	for (int i = 0; i < it->count; ++i) {
		ecs_entity_t e = it->entities[i];
		ecs_trace("System_test entity '%s'", ecs_get_name(world, e));
	}
	ecs_log_set_level(-1);
}

void EgFsInotifyImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgFsInotify);
	ecs_set_name_prefix(world, "EgFsInotify");

	ECS_COMPONENT_DEFINE(world, EgFsInotifyCreate);

	ecs_set_hooks_id(world, ecs_id(EgFsInotifyCreate),
	&(ecs_type_hooks_t){
	.flags = ECS_TYPE_HOOK_COPY_ILLEGAL,
	.move = ecs_move(EgFsInotifyCreate),
	.dtor = ecs_dtor(EgFsInotifyCreate),
	.ctor = ecs_ctor(EgFsInotifyCreate),
	});

	/*
	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "System_test", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_test,
	.query.expr = "eg.fs.Watch, ChildOf($this, $parent), eg.fs.Fd($parent), eg.fs.inotify.Create($parent)"
	});
	*/

	ecs_doc_set_detail(world, ecs_id(EgFsInotifyCreate), "Component that keeps track of inotify watches. ");

	ecs_log_set_level(0);
	// test_inotify("./config");
	ecs_log_set_level(-1);

	ecs_observer_init(world,
	&(ecs_observer_desc_t){
	.entity = ecs_entity(world, {.name = "Observer_inotify_ctl", .add = ecs_ids(ecs_dependson(EcsOnLoad))}),
	.callback = Observer_inotify_ctl,
	.events = {EcsOnSet, EcsOnRemove},
	.query.terms = {
	{.id = ecs_id(EgFsWatch)},
	{.id = ecs_pair(ecs_id(EgFsFd), ecs_id(EgFsInotifyCreate)), .trav = EcsChildOf, .src.id = EcsUp},
	{.id = ecs_id(EgFsInotifyCreate), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsInOutFilter},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "System_Read", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_Read,
	.query.terms =
	{
	{.id = ecs_pair(ecs_id(EgFsFd), ecs_id(EgFsInotifyCreate)), .src.id = EcsSelf},
	{.id = ecs_id(EgFsInotifyCreate), .src.id = EcsSelf},
	{.id = ecs_id(EgFsReady), .src.id = EcsSelf},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "System_Create", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_Create,
	.query.terms =
	{
	{.id = ecs_id(EgFsInotifyCreate), .src.id = EcsSelf},
	{.id = ecs_pair(ecs_id(EgFsFd), ecs_id(EgFsInotifyCreate)), .src.id = EcsSelf, .oper = EcsNot}, // Creates this
	}});
}
