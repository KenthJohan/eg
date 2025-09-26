#include "EgFs.h"
#include "fd.h"
#include <stdio.h>
#include <errno.h>

ECS_COMPONENT_DECLARE(EgFsWatch);
ECS_COMPONENT_DECLARE(EgFsFd);
ECS_COMPONENT_DECLARE(EgFsReady);
ECS_COMPONENT_DECLARE(EgFsContent);



ECS_ENTITY_DECLARE(EgFs);
ECS_ENTITY_DECLARE(EgFsCwd);
ECS_ENTITY_DECLARE(EgFsFiles);
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

static ECS_COPY(EgFsContent, dst, src, {
	ecs_trace("COPY EgFsContent");
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
})

ECS_CTOR(EgFsContent, ptr, {
	ecs_trace("CTOR EgFsContent");
	ptr->data = NULL;
	ptr->size = 0;
})

static ECS_MOVE(EgFsContent, dst, src, {
	ecs_trace("MOVE EgFsContent");
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
})

static ECS_DTOR(EgFsContent, ptr, {
	ecs_log_set_level(0);
	ecs_trace("DTOR EgFsContent %i", ptr->size);
	ecs_os_free(ptr->data);
	ecs_log_set_level(-1);
})


char* flecs_load_from_file( const char *filename, size_t *size )
{
    FILE* file;
    char* content = NULL;
    int32_t bytes;

    /* Open file for reading */
    ecs_os_fopen(&file, filename, "r");
    if (!file) {
        ecs_err("%s (%s)", ecs_os_strerror(errno), filename);
        goto error;
    }

    /* Determine file size */
    fseek(file, 0, SEEK_END);
    bytes = (int32_t)ftell(file);
    if (bytes == -1) {
        goto error;
    }
    fseek(file, 0, SEEK_SET);

    /* Load contents in memory */
    content = ecs_os_malloc(bytes + 1);
    *size = (size_t)bytes;
    if (!(*size = fread(content, 1, *size, file)) && bytes) {
        ecs_err("%s: read zero bytes instead of %d", filename, *size);
        ecs_os_free(content);
        content = NULL;
        goto error;
    } else {
        content[*size] = '\0';
    }

    fclose(file);

    return content;
error:
    if (file) {
        fclose(file);
    }
    ecs_os_free(content);
    return NULL;
}



static void callback_newpath(const ecs_function_ctx_t *ctx, int argc, const ecs_value_t *argv, ecs_value_t *result)
{
	(void)ctx;
	(void)argc;
	ecs_world_t *world = ctx->world;
	const char *path = *(char **)argv[0].ptr;
	// char cwd[1024];
	// getcwd(cwd, sizeof(cwd));
	char fullpath[1024];
	if (path[0] == '.') {
		ecs_os_snprintf(fullpath, sizeof(fullpath), "%s%s", "$CWD", path + 1);
	} else {
		ecs_os_snprintf(fullpath, sizeof(fullpath), "%s", path);
	}
	ecs_trace("fullpath = '%s'", fullpath);
	ecs_entity_t e = ecs_entity_init(world, &(ecs_entity_desc_t){.name = fullpath, .sep = "/", .parent = EgFsCwd});
	*(int64_t *)result->ptr = e;
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


char * load_file(ecs_world_t *world, ecs_entity_t e, size_t * size)
{
	ecs_trace("load_file from path entity '%s'", ecs_get_name(world, e));
	char * path = ecs_get_path_w_sep(world, EgFsCwd, e, "/", NULL); // a a
	path[3] = '.'; // $CWD/src/main.c
	void * content = flecs_load_from_file(path+3, size);
	ecs_os_free(path);
	return content;
}


static void Observer_OnModify(ecs_iter_t *it)
{
	ecs_log_set_level(0);
	ecs_world_t *world = it->world;
	//EcsIdentifier *p = ecs_field(it, EcsIdentifier, 0); // self
	for (int i = 0; i < it->count; ++i) {
		ecs_entity_t e = it->entities[i];
		size_t size = 0;
		void * b = load_file(world, e, &size);
		if (!b) {
			ecs_err("failed to load file for entity '%s'", ecs_get_name(world, e));
			continue;
		} else {
			ecs_set(world, e, EgFsContent, {b, (uint32_t)size});
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

void EgFsImport(ecs_world_t *world)
{
	ECS_MODULE_DEFINE(world, EgFs);
	ecs_set_name_prefix(world, "EgFs");

	ECS_COMPONENT_DEFINE(world, EgFsWatch);
	ECS_COMPONENT_DEFINE(world, EgFsFd);
	ECS_COMPONENT_DEFINE(world, EgFsReady);
	ECS_COMPONENT_DEFINE(world, EgFsContent);

	ECS_ENTITY_DEFINE(world, EgFsCwd);
	ECS_ENTITY_DEFINE(world, EgFsFiles);
	ECS_ENTITY_DEFINE(world, EgFsDescriptors);
	ECS_ENTITY_DEFINE(world, EgFsEventOpen);
	ECS_ENTITY_DEFINE(world, EgFsEventModify);
	ECS_ENTITY_DEFINE(world, EgFsDump);

	ecs_set_hooks_id(world, ecs_id(EgFsFd),
	&(ecs_type_hooks_t){
	.flags = ECS_TYPE_HOOK_COPY_ILLEGAL,
	.move = ecs_move(EgFsFd),
	.dtor = ecs_dtor(EgFsFd),
	.ctor = ecs_ctor(EgFsFd),
	});

	ecs_set_hooks_id(world, ecs_id(EgFsContent),
	&(ecs_type_hooks_t){
	.copy = ecs_copy(EgFsContent),
	.move = ecs_move(EgFsContent),
	.dtor = ecs_dtor(EgFsContent),
	.ctor = ecs_ctor(EgFsContent),
	});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity = ecs_id(EgFsWatch),
	.members = {
	{.name = "fd", .type = ecs_id(ecs_i32_t)},
	{.name = "file", .type = ecs_id(ecs_entity_t)},
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
		.name = "new",
		.return_type = ecs_id(ecs_entity_t),
		.params = {{.name = "path", .type = ecs_id(ecs_string_t)}},
		.callback = callback_newpath});
		ecs_doc_set_brief(world, m, "Lookup child by name");
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
	{ .id = EcsAny },
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "System_Dump", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_Dump,
	.query.terms =
	{
	{.id = ecs_id(EgFsContent), .src.id = EcsSelf},
	{.id = EgFsDump, .src.id = EcsSelf},
	}});

	/*

ecs_pair(ecs_id(EcsIdentifier), EcsName)

	ecs_entity_t e1 = ecs_entity_init(world, &(ecs_entity_desc_t){ .name = "a", .sep = "/" });
	ecs_entity_t e33 = ecs_entity_init(world, &(ecs_entity_desc_t){ .name = "a.c", .sep = "/" });
	ecs_entity_t e2 = ecs_entity_init(world, &(ecs_entity_desc_t){ .name = "a/b", .sep = "/" });
	ecs_entity_t e3 = ecs_entity_init(world, &(ecs_entity_desc_t){ .name = "a/a<>", .sep = "/" });
	ecs_entity_t e4 = ecs_entity_init(world, &(ecs_entity_desc_t){ .name = "a/a<>/banana.sasd", .sep = "/" });
	*/
}
