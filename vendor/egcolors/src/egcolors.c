#include "egcolors.h"

ECS_COMPONENT_DECLARE(EgColorsV4U8_RGBA);
ECS_COMPONENT_DECLARE(EgColorsV4F32_RGBA);

void EgColorsImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgColors);
	ecs_set_name_prefix(world, "EgColors");


	ECS_COMPONENT_DEFINE(world, EgColorsV4U8_RGBA);
	ECS_COMPONENT_DEFINE(world, EgColorsV4F32_RGBA);

	ecs_struct(world,
	{.entity = ecs_id(EgColorsV4U8_RGBA),
	.members = {
	{.name = "r", .type = ecs_id(ecs_u8_t)},
	{.name = "g", .type = ecs_id(ecs_u8_t)},
	{.name = "b", .type = ecs_id(ecs_u8_t)},
	{.name = "a", .type = ecs_id(ecs_u8_t)}
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgColorsV4F32_RGBA),
	.members = {
	{.name = "r", .type = ecs_id(ecs_f32_t)},
	{.name = "g", .type = ecs_id(ecs_f32_t)},
	{.name = "b", .type = ecs_id(ecs_f32_t)},
	{.name = "a", .type = ecs_id(ecs_f32_t)},
	}});


}