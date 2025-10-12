#include "EgFs.h"
#include "fd.h"
#include "fs.h"
#include <ecsx/ecsx_trace.h>
#include <stdio.h>

ECS_COMPONENT_DECLARE(EgFsWatch);
ECS_COMPONENT_DECLARE(EgFsFd);
ECS_COMPONENT_DECLARE(EgFsReady);
ECS_COMPONENT_DECLARE(EgFsContent);

ECS_ENTITY_DECLARE(EgFs);
ECS_ENTITY_DECLARE(EgFsFile);
ECS_ENTITY_DECLARE(EgFsDir);
ECS_ENTITY_DECLARE(EgFsRoot);
ECS_ENTITY_DECLARE(EgFsCwd);
ECS_ENTITY_DECLARE(EgFsSockets);
ECS_ENTITY_DECLARE(EgFsDescriptors);
ECS_ENTITY_DECLARE(EgFsEventOpen);
ECS_ENTITY_DECLARE(EgFsEventModify);
ECS_ENTITY_DECLARE(EgFsDump);

ECS_CTOR(EgFsFd, ptr, {
	ptr->fd = -1;
})

ECS_DTOR(EgFsFd, ptr, {
	fd_close_valid(ptr->fd);
})

ECS_MOVE(EgFsFd, dst, src, {
	fd_close_valid(dst->fd);
	dst->fd = src->fd;
	src->fd = -1; // Invalidate the source fd
})

/*
static ECS_COPY(EgFsContent, dst, src, {
    ecs_log_set_level(0);
    ecs_trace("COPY EgFsContent %i -> %i", dst->size, src->size);
    if (dst) {
        ecs_os_free(dst->data);
    }
    if (src->data && (src->size > 0)) {
        dst->data = ecs_os_malloc(src->size);
        ecs_os_memcpy_n(dst->data, src->data, char, src->size);
        dst->size = src->size;
    } else {
        dst->data = NULL;
        dst->size = 0;
    }
    ecs_log_set_level(-1);
})
*/

ECS_CTOR(EgFsContent, ptr, {
	ecs_log_set_level(0);
	ecs_trace("CTOR EgFsContent");
	ptr->data = NULL;
	ptr->size = 0;
	ecs_log_set_level(-1);
})

static ECS_MOVE(EgFsContent, dst, src, {
	ecs_log_set_level(0);
	ecs_trace("MOVE EgFsContent src=%i, dst=%i", src->size, dst->size);
	if (dst) {
		ecs_os_free(dst->data);
	}
	if (src->data) {
		dst->data = src->data;
		dst->size = src->size;
		src->data = NULL;
		src->size = 0;
	} else {
		dst->data = NULL;
		dst->size = 0;
	}
	ecs_log_set_level(-1);
})

static ECS_DTOR(EgFsContent, ptr, {
	ecs_log_set_level(0);
	ecs_trace("DTOR EgFsContent %i", ptr->size);
	ecs_os_free(ptr->data);
	ecs_log_set_level(-1);
})

ecs_entity_t EgFs_create_path_entity(ecs_world_t *world, char const *path)
{
	ecs_entity_t parent = 0;
	uint32_t flags = 0;
	if ((path[0] == '.') && (path[1] == '/')) {
		parent = EgFsCwd;
		flags = fs_get_path_flags(path);
		path += 2;
	} else if (path[0] == '/') {
		parent = EgFsRoot;
		flags = fs_get_path_flags(path);
		path += 1;
	} else {
		return 0;
	}
	ecs_id_t f = 0;
	if (flags & FS_PATH_FILE) {
		f = ecs_id(EgFsFile);
	} else if (flags & FS_PATH_DIR) {
		f = ecs_id(EgFsDir);
	} else {
		return 0;
	}
	ecs_entity_t e = ecs_entity_init(world,
	&(ecs_entity_desc_t){
	.name = path,
	.sep = "/",
	.parent = parent,
	.add = (ecs_id_t[]){f, 0},
	});
	return e;
}

/*
https://github.com/nanomsg/nng
https://nng.nanomsg.org/ref/tran/udp.html
https://nng.nanomsg.org/man/v1.10.0/index.html
https://www.flecs.dev/flecs/group__liveliness.html#ga7995e931b0f8b7588f0519ae88b6e4c0
https://github.com/copilot/c/caff1387-5a8d-4db9-a850-e167ba83926d
*/
static void callback_newpath(const ecs_function_ctx_t *ctx, int argc, const ecs_value_t *argv, ecs_value_t *result)
{
	ecs_log_set_level(0);
	(void)ctx;
	(void)argc;
	ecs_world_t *world = ctx->world;
	const char *path = *(char **)argv[0].ptr;
	// char cwd[1024];
	// getcwd(cwd, sizeof(cwd));
	ecs_entity_t e = 0;
	e = EgFs_create_path_entity(world, path);
	if (e) {
		char *p = ecs_get_path_w_sep(world, EgFsSockets, e, ":", NULL);
		ecs_trace("path '%s' -> '%s' %16i", path, p, e);
		ecs_os_free(p);
	} else if (strncmp(path, "udp://", 6) == 0) {
		// fd_create_udp_socket
		e = ecs_lookup_path_w_sep(world, EgFsSockets, path, "", NULL, false);
		if (e == 0) {
			int fd = fd_create_udp_socket(NULL, 5000);
			if (fd < 0) {
				e = 0;
			} else {
				e = fd + EGFS_FD_ENTITY_OFFSET;
				ecs_make_alive(world, e);
				ecs_add_path_w_sep(world, e, EgFsSockets, path, "", NULL);
			}
		}
	}
	*(int64_t *)result->ptr = e;
	ecs_log_set_level(-1);
}

static void Observer_OnOpen(ecs_iter_t *it)
{
	ecs_log_set_level(0);
	ecs_world_t *world = it->world;
	EcsIdentifier *p = ecs_field(it, EcsIdentifier, 0); // self
	for (int i = 0; i < it->count; ++i) {
		ecs_entity_t e = it->entities[i];
		ecs_trace("EgFsEventOpen received for entity '%s' %s", ecs_get_name(world, e), p->value);
	}
	ecs_log_set_level(-1);
}

static void Observer_OnModify_extra(ecs_world_t * world, ecs_entity_t e)
{
    ecs_iter_t it = ecs_each_id(world, ecs_pair(EgFsEventModify, e));
    while (ecs_each_next(&it)) {
        for (int i = 0; i < it.count; i ++) {
            printf("%s\n", ecs_get_name(world, it.entities[i]));
        }
    }
}

static void Observer_OnModify(ecs_iter_t *it)
{
	ecs_log_set_level(0);
	ecs_world_t *world = it->world;
	EgFsContent *c = ecs_field(it, EgFsContent, 1); // self
	for (int i = 0; i < it->count; ++i) {
		ecs_entity_t e = it->entities[i];
		if (c[i].data) {
			// Free previous content
			ecs_trace("free %i bytes previous EgFsContent for entity '%s'", c[i].size, ecs_get_name(world, e));
			ecs_os_free(c[i].data);
			c[i].data = NULL;
			c[i].size = 0;
		}
		size_t size = 0;
		void *content = NULL;
		char *path = ecs_get_path_w_sep(world, EgFsCwd, e, "/", "./"); // Allocates
		uint32_t flags = fs_get_path_flags(path);
		if (flags & FS_PATH_FILE) {
			content = fs_load_from_file(path, &size);
		}
		ecs_os_free(path);
		if (!content) {
			ecs_err("failed to load file for entity '%s'", ecs_get_name(world, e));
		} else {
			c[i].data = content;
			c[i].size = (uint32_t)size;
			ecs_trace("loaded %u bytes into EgFsContent for entity '%s'", c[i].size, ecs_get_name(world, e));
			ecs_enqueue(world,
			&(ecs_event_desc_t){
			.event = ecs_id(EgFsContent),
			.entity = e,
			.ids = &(ecs_type_t){(ecs_id_t[]){ecs_id(EgFsContent)}, 1},
			});
			Observer_OnModify_extra(world, e);
			// Add EgFsDump to dump content in System_Dump
			// ecs_add(world, e, EgFsDump);
		}
	}
	ecs_log_set_level(-1);
}

static void System_Dump(ecs_iter_t *it)
{
	ecs_log_set_level(0);
	ecs_world_t *world = it->world;
	EgFsContent *c = ecs_field(it, EgFsContent, 0);
	(void)world;
	for (int i = 0; i < it->count; ++i) {
		ecs_entity_t e = it->entities[i];
		ecs_remove(world, e, EgFsDump);
		ecs_trace("System_Dump for entity '%s'", ecs_get_name(world, e));
		if (c[i].data && (c[i].size > 0)) {
			printf("---- content (%u bytes) ----\n", c[i].size);
			fwrite(c[i].data, 1, c[i].size, stdout);
			printf("\n---- end of content ----\n");
		} else {
			printf("---- no content ----\n");
		}
	}
	ecs_log_set_level(-1);
}

static void System_Dump1(ecs_iter_t *it)
{
	ecs_log_set_level(0);
	ecs_world_t *world = it->world;
	for (int i = 0; i < it->count; ++i) {
		ecs_entity_t e = it->entities[i];
		ecs_remove(world, e, EgFsDump);
		// print entity integer and name
		printf("Entity %16i '%s'\n", (uint32_t)e, ecs_get_name(world, e));
	}
	ecs_log_set_level(-1);
}

void EgFsImport(ecs_world_t *world)
{
	ECS_MODULE_DEFINE(world, EgFs);
	ecs_set_name_prefix(world, "EgFs");

	ECS_COMPONENT_DEFINE(world, EgFsWatch);
	ECS_COMPONENT_DEFINE(world, EgFsFd);
	ECS_COMPONENT_DEFINE(world, EgFsReady);
	ECS_COMPONENT_DEFINE(world, EgFsContent);

	ECS_ENTITY_DEFINE(world, EgFsCwd);
	ECS_ENTITY_DEFINE(world, EgFsRoot);
	ECS_ENTITY_DEFINE(world, EgFsSockets);
	ECS_ENTITY_DEFINE(world, EgFsDescriptors);
	ECS_ENTITY_DEFINE(world, EgFsEventOpen);
	ECS_ENTITY_DEFINE(world, EgFsEventModify);
	ECS_ENTITY_DEFINE(world, EgFsDump);
	ECS_ENTITY_DEFINE(world, EgFsFile);
	ECS_ENTITY_DEFINE(world, EgFsDir);

	ecs_set_hooks_id(world, ecs_id(EgFsFd),
	&(ecs_type_hooks_t){
	.move = ecs_move(EgFsFd),
	.dtor = ecs_dtor(EgFsFd),
	.ctor = ecs_ctor(EgFsFd),
	});

	ecs_set_hooks_id(world, ecs_id(EgFsContent),
	&(ecs_type_hooks_t){
	.move = ecs_move(EgFsContent),
	.dtor = ecs_dtor(EgFsContent),
	.ctor = ecs_ctor(EgFsContent),
	});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity = ecs_id(EgFsFd),
	.members = {
	{.name = "fd", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity = ecs_id(EgFsWatch),
	.members = {
	{.name = "path1", .type = ecs_id(ecs_entity_t)},
	{.name = "prefab", .type = ecs_id(ecs_entity_t)},
	}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity = ecs_id(EgFsContent),
	.members = {
	{.name = "data", .type = ecs_id(ecs_uptr_t)},
	{.name = "size", .type = ecs_id(ecs_u32_t)},
	}});

	{
		ecs_entity_t m = ecs_function_init(world,
		&(ecs_function_desc_t){
		.name = "path1",
		.return_type = ecs_id(ecs_entity_t),
		.params = {{.name = "name", .type = ecs_id(ecs_string_t)}},
		.callback = callback_newpath});
		ecs_doc_set_brief(world, m, "Lookup child by name11");
	}

	ecs_observer_init(world,
	&(ecs_observer_desc_t){
	.entity = ecs_entity(world, {.name = "Observer_OnOpen"}),
	.callback = Observer_OnOpen,
	.events = {EgFsEventOpen},
	.query.terms = {
	{.id = ecs_pair(ecs_id(EcsIdentifier), EcsName)},
	}});

	ecs_observer_init(world,
	&(ecs_observer_desc_t){
	.entity = ecs_entity(world, {.name = "Observer_OnModify"}),
	.callback = Observer_OnModify,
	.events = {EgFsEventModify},
	.query.terms = {
	{.id = EgFsFile},
	{.id = ecs_id(EgFsContent), .inout = EcsInOutFilter},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "System_Dump", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_Dump,
	.query.terms = {
	{.id = ecs_id(EgFsContent), .src.id = EcsSelf},
	{.id = EgFsDump, .src.id = EcsSelf},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "System_Dump1", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_Dump1,
	.query.terms = {
	{.id = EgFsDump, .src.id = EcsSelf},
	}});
}
