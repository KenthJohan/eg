#include "EgResources.h"
#include "EgQuantities.h"

ECS_DECLARE(EgState);
ECS_DECLARE(EgUpdating);
ECS_DECLARE(EgUpdate);
ECS_DECLARE(EgValid);
ECS_DECLARE(EgMoving);
ECS_DECLARE(EgError);

ECS_COMPONENT_DECLARE(EgTexture);
ECS_COMPONENT_DECLARE(EgImage);
ECS_COMPONENT_DECLARE(EgPath);
ECS_COMPONENT_DECLARE(EgChunk);


static ECS_COPY(EgPath, dst, src, {
ecs_os_strset((char**)&dst->value, src->value);
})

static ECS_MOVE(EgPath, dst, src, {
ecs_os_free((char*)dst->value);
dst->value = src->value;
src->value = NULL;
})

static ECS_DTOR(EgPath, ptr, {
ecs_os_free((char*)ptr->value);
})


static void System1(ecs_iter_t *it)
{

}


void EgResourcesImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgResources);
	ECS_IMPORT(world, EgQuantities);
	ecs_set_name_prefix(world, "Eg");

	ECS_ENTITY_DEFINE(world, EgState, 0);
	ecs_add_id(world, EgState, EcsUnion);
	ECS_TAG_DEFINE(world, EgUpdating);
	ECS_TAG_DEFINE(world, EgUpdate);
	ECS_TAG_DEFINE(world, EgValid);
	ECS_TAG_DEFINE(world, EgMoving);
	ECS_TAG_DEFINE(world, EgError);

	ECS_COMPONENT_DEFINE(world, EgImage);
	ECS_COMPONENT_DEFINE(world, EgPath);
	ECS_COMPONENT_DEFINE(world, EgChunk);
	ECS_COMPONENT_DEFINE(world, EgTexture);


	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgPath),
	.members = {
	{ .name = "value", .type = ecs_id(ecs_string_t) }
	}
	});


	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgTexture),
	.members = {
	{ .name = "slot", .type = ecs_id(ecs_i32_t) },
	{ .name = "pixel_format", .type = ecs_id(ecs_i32_t) },
	{ .name = "min_filter", .type = ecs_id(ecs_i32_t) },
	{ .name = "mag_filter", .type = ecs_id(ecs_i32_t) },
	}
	});


	ecs_set_hooks(world, EgPath, {
	.ctor = ecs_default_ctor,
	.move = ecs_move(EgPath),
	.copy = ecs_copy(EgPath),
	.dtor = ecs_dtor(EgPath)
	});

}
