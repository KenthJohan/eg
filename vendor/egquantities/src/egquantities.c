#include "egquantities.h"


ECS_COMPONENT_DECLARE(EgQuantitiesIsq);
ECS_TAG_DECLARE(EgQuantitiesVoltage);


void EgQuantitiesImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgQuantities);
	ecs_set_name_prefix(world, "EgQuantities");

	ECS_COMPONENT_DEFINE(world, EgQuantitiesIsq);
	ECS_TAG_DEFINE(world, EgQuantitiesVoltage);

	ecs_struct(world,
	{.entity = ecs_id(EgQuantitiesIsq),
	.members = {
	{.name = "symbol", .type = ecs_id(ecs_string_t)},
	{.name = "length", .type = ecs_id(ecs_i32_t)},
	{.name = "mass", .type = ecs_id(ecs_i32_t)},
	{.name = "time", .type = ecs_id(ecs_i32_t)},
	{.name = "current", .type = ecs_id(ecs_i32_t)},
	{.name = "temperature", .type = ecs_id(ecs_i32_t)},
	{.name = "substance", .type = ecs_id(ecs_i32_t)},
	{.name = "intensity", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_set(world, EgQuantitiesVoltage, EgQuantitiesIsq, {.symbol = "V", .length = 2, .mass = 1, .time = -3, .current = -1});

	



}