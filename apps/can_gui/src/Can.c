#include "Can.h"



ECS_COMPONENT_DECLARE(CanSignal);




void CanImport(ecs_world_t *world)
{
	ECS_MODULE(world, Can);
	ECS_COMPONENT_DEFINE(world, CanSignal);

	ecs_struct(world,
	{.entity = ecs_id(CanSignal),
	.members = {
	{.name = "value", .type = ecs_id(ecs_u8_t)},
	}});



}