/*
https://wiki.libsdl.org/SDL3/SDL_ReadIO
https://github.com/SanderMertens/flecs/blob/master/examples/c/entities/hooks/src/main.c
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/src/datastructures/vec.c#L118
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/include/flecs/datastructures/vec.h
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/src/addons/alerts.c#L39
https://github.com/libsdl-org/SDL/blob/0fcaf47658be96816a851028af3e73256363a390/test/testautomation_iostream.c#L477
*/

#include "EgFs.h"

#include <stdlib.h>
#include <stdio.h>

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_iostream.h>

#include <ecsx.h>
#include <egmisc.h>

ECS_TAG_DECLARE(EgFsDir);
ECS_TAG_DECLARE(EgFsFile);
ECS_TAG_DECLARE(EgFsUpdate);
ECS_TAG_DECLARE(EgFsRead);
ECS_TAG_DECLARE(EgFsPrint);
ECS_TAG_DECLARE(EgFsEof);

ECS_COMPONENT_DECLARE(EgFsContent);
ECS_COMPONENT_DECLARE(EgFsStream);
typedef struct {
	ecs_world_t *world;
	ecs_entity_t entity;
} ecs_world_entity_t;

static SDL_EnumerationResult SDLCALL enum_callback(ecs_world_entity_t *we, const char *dirname, const char *fname)
{
	ecs_world_t *world = we->world;
	ecs_entity_t root = we->entity;
	char buf[1024];
	snprintf(buf, 1024, "%s%s", dirname, fname);
	ecs_entity_t e = ecs_entity_init(world,
	&(ecs_entity_desc_t){
	.name = NULL,
	.parent = root,
	});
	ecs_doc_set_name(world, e, buf);
	SDL_PathInfo info;
	if (!SDL_GetPathInfo(buf, &info)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't stat '%s': %s", buf, SDL_GetError());
		return SDL_ENUM_CONTINUE;
	}
	if (info.type == SDL_PATHTYPE_FILE) {
		ecs_doc_set_color(world, e, "#358844FF");
		ecs_doc_set_brief(world, e, "File");
		ecs_add_id(world, e, EgFsFile);
	} else if (info.type == SDL_PATHTYPE_DIRECTORY) {
		ecs_doc_set_color(world, e, "#888811FF");
		ecs_doc_set_brief(world, e, "Folder");
		ecs_add_id(world, e, EgFsDir);
		ecs_add_id(world, e, EgFsUpdate);
	} else {
		ecs_doc_set_color(world, e, "#880000FF");
		ecs_doc_set_brief(world, e, "Other");
	}
	return SDL_ENUM_CONTINUE;
}

static void System_enumerate_directory(ecs_iter_t *it)
{
	ecs_log_set_level(0);
	ecs_world_t *world = it->world;
	ecsx_trace_system_iter(it);
	ecs_log_push_(0);
	EcsDocDescription *d = ecs_field(it, EcsDocDescription, 0);
	for (int i = 0; i < it->count; ++i, ++d) {
		ecs_entity_t e = it->entities[i];
		char const *name = ecs_get_name(world, e);

		ecs_remove_id(world, e, EgFsUpdate);

		if (d->value == NULL) {
			ecs_err("No path set for directory entity '%s'", name);
			continue;
		}
		ecs_world_entity_t we = {.world = world, .entity = e};
		ecs_trace("Enumerate directory '%s' ('%s')", d->value, name);
		bool success = SDL_EnumerateDirectory(d->value, (SDL_EnumerateDirectoryCallback)enum_callback, &we);

		if (!success) {
			ecs_err("Base path enumeration failed!");
		}
	}
	ecs_log_pop_(0);
	ecs_log_set_level(0);
}

static void System_open(ecs_iter_t *it)
{
	ecs_log_set_level(0);
	EgFsStream *s = ecs_field(it, EgFsStream, 0);
	EcsDocDescription *d = ecs_field(it, EcsDocDescription, 1);
	for (int i = 0; i < it->count; ++i, ++s, ++d) {
		ecs_entity_t e = it->entities[i];
		printf("Open file '%s'\n", ecs_get_name(it->world, e));
		s->stream = SDL_IOFromFile(d->value, "rb");
		if (!s->stream) {
			ecs_err("Failed to open file '%s': %s", d->value, SDL_GetError());
			ecs_remove_id(it->world, e, ecs_id(EgFsStream));
		}
	}
}

static void System_read_file(ecs_iter_t *it)
{
	ecs_log_set_level(-1);
	ecs_world_t *world = it->world;
	ecsx_trace_system_iter(it);
	ecs_log_push_(0);
	EgFsContent *c = ecs_field(it, EgFsContent, 0);
	EgFsStream *s = ecs_field(it, EgFsStream, 1);
	for (int i = 0; i < it->count; ++i, ++c, ++s) {
		ecs_entity_t e = it->entities[i];
		char const *name = ecs_get_name(world, e);
		ecs_vec_t * vec = &c->buf;
		if (!vec) {
			ecs_err("Failed to allocate memory for file '%s'", name);
			ecs_remove_id(world, e, ecs_id(EgFsStream));
			ecs_remove_id(world, e, EgFsRead);
			continue;
		}
		char chunk[16];
		size_t r = SDL_ReadIO(s->stream, chunk, sizeof(chunk));
		if (r == 0) {
			ecs_trace("End of file reached for '%s'", name);
			ecs_remove_id(world, e, ecs_id(EgFsStream));
			ecs_remove_id(world, e, EgFsRead);
			ecs_add_id(world, e, EgFsEof);
		} else {
			char * ptr = ecs_vec_grow_t(NULL, vec, char, r);
			memcpy(ptr, chunk, r);
			ecs_trace("Read %zu bytes from file '%s'", r, name);
		}
	}
	ecs_log_pop_(0);
	ecs_log_set_level(0);
}

static void System_print(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	EgFsContent *c = ecs_field(it, EgFsContent, 0);
	for (int i = 0; i < it->count; ++i, ++c) {
		ecs_entity_t e = it->entities[i];
		char const *name = ecs_get_name(world, e);
		printf("Print file '%s' (%d bytes):\n", name, c->buf.count);
		fwrite(c->buf.array, 1, c->buf.count, stdout);
		ecs_remove_id(world, e, EgFsPrint);
	}
}

ECS_CTOR(EgFsStream, ptr, {
	// printf("Ctor\n");
	ptr->stream = NULL;
})

ECS_DTOR(EgFsStream, ptr, {
	// printf("Dtor\n");
	if (ptr->stream) {
		bool success = SDL_CloseIO(ptr->stream);
		ptr->stream = NULL;
		if (!success) {
			ecs_err("Failed to close file stream: %s", SDL_GetError());
		}
	}
})

ECS_MOVE(EgFsStream, dst, src, {
	// printf("Move\n");
	dst->stream = src->stream;
	src->stream = NULL;
})

ECS_COPY(EgFsStream, dst, src, {
	// printf("Copy\n");
	dst->stream = src->stream;
	// dst->stream = SDL_DuplicateFile(src->stream);
})

void EgFsContent_init(EgFsContent *c){
	ecs_vec_init_t(NULL, &c->buf, char, 0);
}

ECS_CTOR(EgFsContent, ptr, {
	//printf("Ctor\n");
	EgFsContent_init(ptr);
	//ecs_vec_init_t(NULL, &ptr->buf, char, 8);
})

ECS_DTOR(EgFsContent, ptr, {
	//printf("Dtor\n");
	ecs_vec_fini_t(NULL, &ptr->buf, char);
})

ECS_MOVE(EgFsContent, dst, src, {
	//printf("Move\n");
	ecs_vec_fini_t(NULL, &dst->buf, char);
	dst->buf = src->buf;
	src->buf = (ecs_vec_t){0};
})

ECS_COPY(EgFsContent, dst, src, {
	//printf("Copy\n");
	//ecs_vec_fini_t(NULL, &dst->buf, char);
	//dst->buf = ecs_vec_copy_t(NULL, &src->buf, char);
})

void hook_callback(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	ecs_entity_t event = it->event;

	for (int i = 0; i < it->count; i++) {
		ecs_entity_t e = it->entities[i];
		printf("%s: %s\n",ecs_get_name(world, event), ecs_get_name(world, e));
	}
}


void EgFsImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgFs);
	ecs_set_name_prefix(world, "EgFs");

	ECS_TAG_DEFINE(world, EgFsDir);
	ECS_TAG_DEFINE(world, EgFsFile);
	ECS_TAG_DEFINE(world, EgFsUpdate);
	ECS_TAG_DEFINE(world, EgFsRead);
	ECS_TAG_DEFINE(world, EgFsPrint);
	ECS_TAG_DEFINE(world, EgFsEof);
	ECS_COMPONENT_DEFINE(world, EgFsContent);
	ECS_COMPONENT_DEFINE(world, EgFsStream);

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity = ecs_id(EgFsContent),
	.members = {
	{.name = "buf_ptr", .type = ecs_id(ecs_uptr_t)},
	{.name = "buf_count", .type = ecs_id(ecs_i32_t)},
	{.name = "buf_cap", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_set_hooks_id(world, ecs_id(EgFsStream),
	&(ecs_type_hooks_t){
	.ctor = ecs_ctor(EgFsStream),
	.move = ecs_move(EgFsStream),
	.dtor = ecs_dtor(EgFsStream),
	.copy = ecs_copy(EgFsStream),

	/* Lifecycle hooks. These hooks should be used for application logic. */
	.on_add = hook_callback,
	.on_remove = hook_callback,
	.on_set = hook_callback});

	ecs_set_hooks_id(world, ecs_id(EgFsContent),
	&(ecs_type_hooks_t){
	.ctor = ecs_ctor(EgFsContent),
	.move = ecs_move(EgFsContent),
	.dtor = ecs_dtor(EgFsContent),
	.copy = ecs_copy(EgFsContent),

	/* Lifecycle hooks. These hooks should be used for application logic. */
	.on_add = hook_callback,
	.on_remove = hook_callback,
	.on_set = hook_callback});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "System_enumerate_directory", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_enumerate_directory,
	.immediate = true,
	.query.terms = {
	{.id = ecs_pair(ecs_id(EcsDocDescription), EcsName)},
	{.id = EgFsDir},
	{.id = EgFsUpdate},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "System_read_file", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_read_file,
	.immediate = true,
	.query.terms = {
	{.id = ecs_id(EgFsContent)},
	{.id = ecs_id(EgFsStream)},
	{.id = EgFsFile},
	{.id = EgFsRead}, // Removes this
	}});

	ecs_observer_init(world,
	&(ecs_observer_desc_t){
	.events = {EcsOnAdd},
	.callback = System_open,
	.query.terms = {
	{.id = ecs_id(EgFsStream)},
	{.id = ecs_pair(ecs_id(EcsDocDescription), EcsName), .inout = EcsInOutFilter},
	{.id = EgFsFile},
	}});


	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "System_print", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_print,
	.immediate = true,
	.query.terms = {
	{.id = ecs_id(EgFsContent)},
	{.id = EgFsPrint},
	}});

}
