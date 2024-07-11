#include "egbase.h"

ECS_COMPONENT_DECLARE(EgBasePass);
ECS_COMPONENT_DECLARE(EgBaseShapeBuffer);
ECS_COMPONENT_DECLARE(EgBaseShapeElement);
ECS_COMPONENT_DECLARE(EgBaseShowDrawReference);
ECS_COMPONENT_DECLARE(EgBaseMemory);

ECS_TAG_DECLARE(EgBaseUse);
ECS_TAG_DECLARE(EgBaseUpdate);
ECS_TAG_DECLARE(EgBaseDraw);



ECS_CTOR(EgBaseShapeBuffer, ptr, {
	ecs_os_memset_t(ptr, 0, EgBaseShapeBuffer);
})









void EgBaseImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgBase);

	ecs_set_name_prefix(world, "EgBase");

	ECS_COMPONENT_DEFINE(world, EgBasePass);
	ECS_COMPONENT_DEFINE(world, EgBaseShapeBuffer);
	ECS_COMPONENT_DEFINE(world, EgBaseShapeElement);
	ECS_COMPONENT_DEFINE(world, EgBaseShowDrawReference);
	ECS_COMPONENT_DEFINE(world, EgBaseMemory);
	ECS_TAG_DEFINE(world, EgBaseUse);
	ECS_TAG_DEFINE(world, EgBaseUpdate);
	ECS_TAG_DEFINE(world, EgBaseDraw);

	ecs_add_id(world, EgBaseUse, EcsTraversable);
	ecs_add_id(world, EgBaseUpdate, EcsTraversable);
	

	ecs_set_hooks(world, EgBaseShapeBuffer, {.ctor = ecs_ctor(EgBaseShapeBuffer)});





	ecs_struct(world,
	{.entity = ecs_id(EgBaseShapeElement),
	.members = {
	{.name = "x", .type = ecs_id(ecs_i32_t)},
	{.name = "y", .type = ecs_id(ecs_i32_t)},
	}});


	ecs_struct(world,
	{.entity = ecs_id(EgBaseMemory),
	.members = {
	{.name = "ptr", .type = ecs_id(ecs_uptr_t)},
	{.name = "size", .type = ecs_id(ecs_i32_t)},
	{.name = "cap", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgBaseShowDrawReference),
	.members = {
	{.name = "flags", .type = ecs_id(ecs_u32_t)},
	}});


}