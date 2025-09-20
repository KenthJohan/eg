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

#include "fd.h"

ECS_COMPONENT_DECLARE(EgFsPath);
ECS_COMPONENT_DECLARE(EgFsWatch);
ECS_COMPONENT_DECLARE(EgFsLookup);
ECS_COMPONENT_DECLARE(EgFsFd);
ECS_COMPONENT_DECLARE(EgFsReady);

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






ECS_CTOR(EgFsFd, ptr, {
	ptr->fd = -1;
})

// The destructor should free resources.
ECS_DTOR(EgFsFd, ptr, {
	fd_close_valid(ptr->fd);
})

ECS_MOVE(EgFsFd, dst, src, {
	fd_close_valid(dst->fd);
	dst->fd = src->fd;
	src->fd = -1; // Invalidate the source fd
})


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
		a = ecs_entity_init(world,
		&(ecs_entity_desc_t){
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
	ECS_COMPONENT_DEFINE(world, EgFsWatch);
	ECS_COMPONENT_DEFINE(world, EgFsLookup);
	ECS_COMPONENT_DEFINE(world, EgFsFd);
	ECS_COMPONENT_DEFINE(world, EgFsReady);
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

	ecs_set_hooks_id(world, ecs_id(EgFsFd),
	&(ecs_type_hooks_t){
	.flags = ECS_TYPE_HOOK_COPY_ILLEGAL,
	.move = ecs_move(EgFsFd),
	.dtor = ecs_dtor(EgFsFd),
	.ctor = ecs_ctor(EgFsFd),
	});

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
	.entity = ecs_id(EgFsWatch),
	.members = {
	{.name = "fd", .type = ecs_id(ecs_i32_t)},
	{.name = "file", .type = ecs_id(ecs_entity_t)},
	}});

	ecs_singleton_add(world, EgFsLookup);
}
