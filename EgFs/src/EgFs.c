/*
https://wiki.libsdl.org/SDL3/SDL_ReadIO
https://github.com/SanderMertens/flecs/blob/master/examples/c/entities/hooks/src/main.c
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/src/datastructures/vec.c#L118
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/include/flecs/datastructures/vec.h
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/src/addons/alerts.c#L39
https://github.com/libsdl-org/SDL/blob/0fcaf47658be96816a851028af3e73256363a390/test/testautomation_iostream.c#L477
*/

#include "EgFs.h"
#include "fd.h"

ECS_COMPONENT_DECLARE(EgFsWatch);
ECS_COMPONENT_DECLARE(EgFsFd);
ECS_COMPONENT_DECLARE(EgFsReady);

ECS_ENTITY_DECLARE(EgFs);
ECS_ENTITY_DECLARE(EgFsFiles);
ECS_ENTITY_DECLARE(EgFsDescriptors);
ECS_ENTITY_DECLARE(EgFsEventOpen);
ECS_ENTITY_DECLARE(EgFsEventModify);

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



static void callback_newpath(const ecs_function_ctx_t *ctx, int argc, const ecs_value_t *argv, ecs_value_t *result)
{
	(void)ctx;
	(void)argc;
	ecs_world_t *world = ctx->world;
	const char *path = *(char **)argv[0].ptr;
	//char cwd[1024];
	//getcwd(cwd, sizeof(cwd));
	char fullpath[1024];
	if (path[0] == '.') {
		snprintf(fullpath, sizeof(fullpath), "%s%s", "$CWD", path + 1);
	} else {
		snprintf(fullpath, sizeof(fullpath), "%s", path);
	}
	printf("fullpath = '%s'\n", fullpath);
	ecs_entity_t e = ecs_entity_init(world, &(ecs_entity_desc_t){ .name = fullpath, .sep = "/", .parent = EgFsFiles });
	*(int64_t *)result->ptr = e;
}



void EgFsImport(ecs_world_t *world)
{
	ECS_MODULE_DEFINE(world, EgFs);
	ecs_set_name_prefix(world, "EgFs");

	ECS_COMPONENT_DEFINE(world, EgFsWatch);
	ECS_COMPONENT_DEFINE(world, EgFsFd);
	ECS_COMPONENT_DEFINE(world, EgFsReady);
	ECS_ENTITY_DEFINE(world, EgFsFiles);
	ECS_ENTITY_DEFINE(world, EgFsDescriptors);
	ECS_ENTITY_DEFINE(world, EgFsEventOpen);
	ECS_ENTITY_DEFINE(world, EgFsEventModify);

	ecs_set_hooks_id(world, ecs_id(EgFsFd),
	&(ecs_type_hooks_t){
	.flags = ECS_TYPE_HOOK_COPY_ILLEGAL,
	.move = ecs_move(EgFsFd),
	.dtor = ecs_dtor(EgFsFd),
	.ctor = ecs_ctor(EgFsFd),
	});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity = ecs_id(EgFsWatch),
	.members = {
	{.name = "fd", .type = ecs_id(ecs_i32_t)},
	{.name = "file", .type = ecs_id(ecs_entity_t)},
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

	/*
	ecs_entity_t e1 = ecs_entity_init(world, &(ecs_entity_desc_t){ .name = "a", .sep = "/" });
	ecs_entity_t e33 = ecs_entity_init(world, &(ecs_entity_desc_t){ .name = "a.c", .sep = "/" });
	ecs_entity_t e2 = ecs_entity_init(world, &(ecs_entity_desc_t){ .name = "a/b", .sep = "/" });
	ecs_entity_t e3 = ecs_entity_init(world, &(ecs_entity_desc_t){ .name = "a/a<>", .sep = "/" });
	ecs_entity_t e4 = ecs_entity_init(world, &(ecs_entity_desc_t){ .name = "a/a<>/banana.sasd", .sep = "/" });
	*/
}
