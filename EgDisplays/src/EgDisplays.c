#include "EgDisplays.h"

#include <stdlib.h>
#include <stdio.h>


ECS_COMPONENT_DECLARE(EgDisplaysDevice);
ECS_COMPONENT_DECLARE(EgDisplaysMode);
ECS_COMPONENT_DECLARE(EgDisplaysUpdate);


void EgDisplaysImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgDisplays);
	ecs_set_name_prefix(world, "EgDisplays");

	ECS_COMPONENT_DEFINE(world, EgDisplaysDevice);
	ECS_COMPONENT_DEFINE(world, EgDisplaysMode);
	ECS_COMPONENT_DEFINE(world, EgDisplaysUpdate);


	ecs_struct(world,
	{.entity = ecs_id(EgDisplaysDevice),
	.members = {
	{.name = "id", .type = ecs_id(ecs_u32_t), .count = 1},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgDisplaysMode),
	.members = {
	{.name = "bpp", .type = ecs_id(ecs_u32_t), .count = 1},
	{.name = "refresh_rate", .type = ecs_id(ecs_u32_t), .count = 1},
	{.name = "pixel_density", .type = ecs_id(ecs_u32_t), .count = 1},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgDisplaysUpdate),
	.members = {
	{.name = "dummy", .type = ecs_id(ecs_u32_t), .count = 1},
	}});
}
