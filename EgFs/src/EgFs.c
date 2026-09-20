#include "EgFs.h"
#include "EgFs/fd.h"
#include <stdio.h>
#include <ecsx.h>
#include <ecsx/ecsx_file.h>
#include <ecsx/ecsx_pathkind.h>
#include <ecsx/ecsx_trace.h>
#include "EgFsContent.h"

ECS_COMPONENT_DECLARE(EgFsWatch);
ECS_COMPONENT_DECLARE(EgFsFd);
ECS_COMPONENT_DECLARE(EgFsReady);
ECS_COMPONENT_DECLARE(EgFsContent);

ECS_TAG_DECLARE(EgFs);
ECS_TAG_DECLARE(EgFsFile);
ECS_TAG_DECLARE(EgFsDir);
ECS_TAG_DECLARE(EgFsRoot);
ECS_TAG_DECLARE(EgFsSync);
ECS_TAG_DECLARE(EgFsCwd);
ECS_TAG_DECLARE(EgFsSockets);
ECS_TAG_DECLARE(EgFsDescriptors);
ECS_TAG_DECLARE(EgFsEventOpen);
ECS_TAG_DECLARE(EgFsEventModify);
ECS_TAG_DECLARE(EgFsDump);

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
	ecs_entity_t    parent    = 0;
	ecsx_pathkind_t path_type = ecsx_pathkind_get_path_type(path);
	switch (path_type) {
	case ECSX_PATHKIND_DIR:
	case ECSX_PATHKIND_FILE:
	case ECSX_PATHKIND_LINK:
		if ((path[0] == '.') && (path[1] == '/')) {
			parent = EgFsCwd;
			path += 2;
		} else if (path[0] == '/') {
			parent = EgFsRoot;
			path += 1;
		} else {
			return 0;
		}
		break;
	case ECSX_PATHKIND_TCP:
	case ECSX_PATHKIND_UDP:
	case ECSX_PATHKIND_HTTP:
		parent = EgFsSockets;
		break;
	default:
		break;
	}

	ecs_id_t f = 0;
	switch (path_type) {
	case ECSX_PATHKIND_FILE:
		f = ecs_id(EgFsFile);
		break;
	case ECSX_PATHKIND_DIR:
		f = ecs_id(EgFsDir);
		break;
	default:
		break;
	}

	if (path_type == ECSX_PATHKIND_FILE) {
	} else if (path_type == ECSX_PATHKIND_DIR) {
		f = ecs_id(EgFsDir);
	} else {
		return 0;
	}
	ecs_entity_t e = ecs_entity_init(world,
	&(ecs_entity_desc_t){
	.name   = path,
	.sep    = "/",
	.parent = parent});
	ecs_add_id(world, e, f);
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
	int loglvl = 0;

	if (argc < 1) {
		ecs_err("callback_newpath: insufficient arguments");
		return;
	}

	ecs_world_t *world = ctx->world;
	const char  *path  = *(char **)argv[0].ptr;

	char cwd[1024];
	ecsx_os_getcwd(cwd, sizeof(cwd));
	ecs_log(loglvl, "callback_newpath: current working directory: '%s'", cwd);

	ecs_entity_t e = EgFs_create_path_entity(world, path);
	if (e) {
		char *p = ecs_get_path_w_sep(world, 0, e, "/", NULL);
		ecs_log(loglvl, "newpath '%s' -> '%s' entity:0x%jX", path, p, (uintmax_t)e);
		ecs_os_free(p);
	} else {
		ecs_err("failed to create path entity for path '%s'", path);
	}
	*(int64_t *)result->ptr = e;
}

static void Observer_OnOpen(ecs_iter_t *it)
{
	ecs_log_set_level(0);
	ecs_world_t   *world = it->world;
	EcsIdentifier *p     = ecs_field(it, EcsIdentifier, 0); // self
	for (int i = 0; i < it->count; ++i) {
		ecs_entity_t e = it->entities[i];
		ecs_trace("EgFsEventOpen received for entity '%s' %s", ecs_get_name(world, e), p->value);
	}
	ecs_log_set_level(-1);
}

static void Observer_OnModify_extra(ecs_world_t *world, ecs_entity_t e)
{
	ecs_id_t   comp = ecs_pair(EgFsEventModify, e);
	ecs_iter_t it   = ecs_each_id(world, comp);
	while (ecs_each_next(&it)) {
		for (int i = 0; i < it.count; i++) {
			printf("%s\n", ecs_get_name(world, it.entities[i]));
			ecs_add(world, it.entities[i], EgFsEventModify);
		}
	}
}



static void System_Dump(ecs_iter_t *it)
{
	int32_t loglvl = 0;

	ecs_world_t *world = it->world;

	EgFsContent *c = ecs_field_self(it, EgFsContent, 0);

	for (int i = 0; i < it->count; ++i) {
		ecs_entity_t e = it->entities[i];
		ecs_remove(world, e, EgFsDump);
		ecs_log(loglvl, "System_Dump for entity '%s'", ecs_get_name(world, e));
		if (c[i].data && (c[i].size > 0)) {
			ecs_log(loglvl, "---- content (%u bytes) ----", c[i].size);
			fwrite(c[i].data, 1, c[i].size, stdout);
			ecs_log(loglvl, "---- end of content ----");
		} else {
			ecs_log(loglvl, "---- no content ----");
		}
	}
}

static void System_Dump1(ecs_iter_t *it)
{
	int32_t loglvl = 0;

	ecs_world_t *world = it->world;

	for (int i = 0; i < it->count; ++i) {
		ecs_entity_t e = it->entities[i];
		ecs_remove(world, e, EgFsDump);
		ecs_log(loglvl, "Entity %16i '%s'", (uint32_t)e, ecs_get_name(world, e));
	}
}

void EgFsImport(ecs_world_t *world)
{
	ECS_MODULE_DEFINE(world, EgFs);
	ecs_set_name_prefix(world, "EgFs");

	ECS_COMPONENT_DEFINE(world, EgFsWatch);
	ECS_COMPONENT_DEFINE(world, EgFsFd);
	ECS_COMPONENT_DEFINE(world, EgFsReady);
	ECS_COMPONENT_DEFINE(world, EgFsContent);

	ECS_TAG_DEFINE(world, EgFsCwd);
	ECS_TAG_DEFINE(world, EgFsSync);
	ECS_TAG_DEFINE(world, EgFsRoot);
	ECS_TAG_DEFINE(world, EgFsSockets);
	ECS_TAG_DEFINE(world, EgFsDescriptors);
	ECS_TAG_DEFINE(world, EgFsEventOpen);
	ECS_TAG_DEFINE(world, EgFsEventModify);
	ECS_TAG_DEFINE(world, EgFsDump);
	ECS_TAG_DEFINE(world, EgFsFile);
	ECS_TAG_DEFINE(world, EgFsDir);

	ecs_add_id(world, EgFsEventModify, EcsTraversable);
	ecs_add_id(world, EgFsEventOpen, EcsTraversable);

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
	.entity  = ecs_id(EgFsFd),
	.members = {
	{.name = "fd", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgFsWatch),
	.members = {
	{.name = "path1", .type = ecs_id(ecs_entity_t)},
	{.name = "prefab", .type = ecs_id(ecs_entity_t)},
	}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgFsContent),
	.members = {
	{.name = "data", .type = ecs_id(ecs_uptr_t)},
	{.name = "size", .type = ecs_id(ecs_u32_t)},
	}});

	{
		ecs_entity_t m = ecs_function_init(world,
		&(ecs_function_desc_t){
		.name        = "path1",
		.return_type = ecs_id(ecs_entity_t),
		.params      = {{.name = "name", .type = ecs_id(ecs_string_t)}},
		.callback    = callback_newpath});
		ecs_doc_set_brief(world, m, "Lookup child by name11");
	}

	ecs_observer_init(world,
	&(ecs_observer_desc_t){
	.entity      = ecs_entity(world, {.name = "Observer_OnOpen"}),
	.callback    = Observer_OnOpen,
	.events      = {EgFsEventOpen},
	.query.terms = {
	{.id = ecs_pair(ecs_id(EcsIdentifier), EcsName)},
	}});

	ecs_observer_init(world,
	&(ecs_observer_desc_t){
	.entity      = ecs_entity(world, {.name = "EgFsContent_Load_Observer"}),
	.callback    = EgFsContent_Load,
	.events      = {EgFsEventModify},
	.query.terms = {
	{.id = ecs_id(EgFsContent), .inout = EcsInOutFilter},
	{.id = EgFsFile},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgFsContent_Load_System"}),
	.phase       = EcsOnUpdate,
	.callback    = EgFsContent_Load,
	.query.terms = {
	{.id = ecs_id(EgFsContent), .src.id = EcsSelf},
	{.id = EgFsFile},
	{.id = EgFsSync},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "System_Dump"}),
	.phase       = EcsOnUpdate,
	.callback    = System_Dump,
	.query.terms = {
	{.id = ecs_id(EgFsContent), .src.id = EcsSelf},
	{.id = EgFsDump, .src.id = EcsSelf},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "System_Dump1"}),
	.phase       = EcsOnUpdate,
	.callback    = System_Dump1,
	.query.terms = {
	{.id = EgFsDump, .src.id = EcsSelf},
	}});
}
