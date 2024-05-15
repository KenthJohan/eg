#include "egquantities.h"


ECS_COMPONENT_DECLARE(EgQuantitiesIsq);
ECS_COMPONENT_DECLARE(EgQuantitiesProgress);
ECS_COMPONENT_DECLARE(EgQuantitiesRangedU8);
ECS_COMPONENT_DECLARE(EgQuantitiesRangedU16);
ECS_COMPONENT_DECLARE(EgQuantitiesRangedU32);
ECS_COMPONENT_DECLARE(EgQuantitiesRangedF32);

ECS_TAG_DECLARE(EgQuantitiesVoltage);


void EgQuantitiesImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgQuantities);
	ecs_set_name_prefix(world, "EgQuantities");

	ECS_COMPONENT_DEFINE(world, EgQuantitiesIsq);
	ECS_COMPONENT_DEFINE(world, EgQuantitiesProgress);
	ECS_COMPONENT_DEFINE(world, EgQuantitiesRangedU8);
	ECS_COMPONENT_DEFINE(world, EgQuantitiesRangedU16);
	ECS_COMPONENT_DEFINE(world, EgQuantitiesRangedU32);
	ECS_COMPONENT_DEFINE(world, EgQuantitiesRangedF32);
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

	ecs_struct(world,
	{.entity = ecs_id(EgQuantitiesProgress),
	.members = {
	{.name = "min", .type = ecs_id(ecs_f32_t)},
	{.name = "max", .type = ecs_id(ecs_f32_t)},
	{.name = "value", .type = ecs_id(ecs_f32_t)}
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgQuantitiesRangedU8),
	.members = {
	{.name = "min", .type = ecs_id(ecs_u8_t)},
	{.name = "max", .type = ecs_id(ecs_u8_t)},
	{.name = "tx", .type = ecs_id(ecs_u8_t)},
	{.name = "rx", .type = ecs_id(ecs_u8_t)}
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgQuantitiesRangedU16),
	.members = {
	{.name = "min", .type = ecs_id(ecs_u16_t)},
	{.name = "max", .type = ecs_id(ecs_u16_t)},
	{.name = "tx", .type = ecs_id(ecs_u16_t)},
	{.name = "rx", .type = ecs_id(ecs_u16_t)}
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgQuantitiesRangedU32),
	.members = {
	{.name = "min", .type = ecs_id(ecs_u32_t)},
	{.name = "max", .type = ecs_id(ecs_u32_t)},
	{.name = "tx", .type = ecs_id(ecs_u32_t)},
	{.name = "rx", .type = ecs_id(ecs_u32_t)}
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgQuantitiesRangedF32),
	.members = {
	{.name = "min", .type = ecs_id(ecs_f32_t)},
	{.name = "max", .type = ecs_id(ecs_f32_t)},
	{.name = "tx", .type = ecs_id(ecs_f32_t)},
	{.name = "rx", .type = ecs_id(ecs_f32_t)}
	}});


	ecs_set(world, EgQuantitiesVoltage, EgQuantitiesIsq, {.symbol = "V", .length = 2, .mass = 1, .time = -3, .current = -1});

	



}