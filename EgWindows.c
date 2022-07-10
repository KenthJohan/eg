#include "EgWindows.h"


ECS_DECLARE(EgOpenGLContext);
ECS_COMPONENT_DECLARE(EgWindow);
ECS_COMPONENT_DECLARE(EgDraw);
ECS_COMPONENT_DECLARE(EgTitle);


ECS_CTOR(EgTitle, ptr, {
ptr->value = NULL;
})

static ECS_COPY(EgTitle, dst, src, {
ecs_os_strset((char**)&dst->value, src->value);
})

static ECS_MOVE(EgTitle, dst, src, {
ecs_os_free((char*)dst->value);
dst->value = src->value;
src->value = NULL;
})

static ECS_DTOR(EgTitle, ptr, {
ecs_os_free((char*)ptr->value);
})


void EgWindowsImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgWindows);
	ecs_set_name_prefix(world, "Eg");

	ECS_ENTITY_DEFINE(world, EgOpenGLContext, 0);
	//ECS_TAG_DEFINE(world, EgOpenGLContext);
	ecs_add_id(world, EgOpenGLContext, EcsUnion);

	ECS_COMPONENT_DEFINE(world, EgWindow);
	ECS_COMPONENT_DEFINE(world, EgDraw);
	ECS_COMPONENT_DEFINE(world, EgTitle);



	ecs_set_hooks(world, EgTitle, {
	.ctor = ecs_ctor(EgTitle),
	.move = ecs_move(EgTitle),
	.copy = ecs_copy(EgTitle),
	.dtor = ecs_dtor(EgTitle),
	});

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgTitle),
	.members = {
	{ .name = "value", .type = ecs_id(ecs_string_t) }
	}
	});

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgWindow),
	.members = {
	{ .name = "flags", .type = ecs_id(ecs_u64_t) },
	{ .name = "counter", .type = ecs_id(ecs_u64_t) },
	{ .name = "should_destroy", .type = ecs_id(ecs_bool_t) },
	}
	});

	
}

