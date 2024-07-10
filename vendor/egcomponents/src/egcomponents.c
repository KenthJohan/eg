#include "egcomponents.h"

ECS_COMPONENT_DECLARE(Pass);
ECS_COMPONENT_DECLARE(ShapeBuffer);
ECS_COMPONENT_DECLARE(ShapeElement);
ECS_COMPONENT_DECLARE(ShowDrawReference);
ECS_COMPONENT_DECLARE(Memory);

ECS_TAG_DECLARE(EgUse);
ECS_TAG_DECLARE(EgUpdate);
ECS_TAG_DECLARE(EgComponentsDraw);



ECS_CTOR(ShapeBuffer, ptr, {
	ecs_os_memset_t(ptr, 0, ShapeBuffer);
})









void EgComponentsImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgComponents);

	ecs_set_name_prefix(world, "EgComponents");

	ECS_COMPONENT_DEFINE(world, Pass);

	ECS_COMPONENT_DEFINE(world, ShapeBuffer);
	ECS_COMPONENT_DEFINE(world, ShapeElement);
	ECS_COMPONENT_DEFINE(world, ShowDrawReference);
	ECS_COMPONENT_DEFINE(world, Memory);
	

	ECS_TAG_DEFINE(world, EgUse);
	ECS_TAG_DEFINE(world, EgUpdate);
	ECS_TAG_DEFINE(world, EgComponentsDraw);

	ecs_add_id(world, EgUse, EcsTraversable);
	ecs_add_id(world, EgUpdate, EcsTraversable);
	

	ecs_set_hooks(world, ShapeBuffer, {.ctor = ecs_ctor(ShapeBuffer)});





	ecs_struct(world,
	{.entity = ecs_id(ShapeElement),
	.members = {
	{.name = "x", .type = ecs_id(ecs_i32_t)},
	{.name = "y", .type = ecs_id(ecs_i32_t)},
	}});


	ecs_struct(world,
	{.entity = ecs_id(Memory),
	.members = {
	{.name = "ptr", .type = ecs_id(ecs_uptr_t)},
	{.name = "size", .type = ecs_id(ecs_i32_t)},
	{.name = "cap", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(ShowDrawReference),
	.members = {
	{.name = "flags", .type = ecs_id(ecs_u32_t)},
	}});


}