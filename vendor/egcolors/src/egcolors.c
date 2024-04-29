#include "egcolors.h"

ECS_COMPONENT_DECLARE(EgColorsRGBA8888);

void EgColorsImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgColors);
	ecs_set_name_prefix(world, "EgColors");


	ECS_COMPONENT_DEFINE(world, EgColorsRGBA8888);
	ecs_struct(world,
	{.entity = ecs_id(EgColorsRGBA8888),
	.members = {
	{.name = "r", .type = ecs_id(ecs_u8_t)},
	{.name = "g", .type = ecs_id(ecs_u8_t)},
	{.name = "b", .type = ecs_id(ecs_u8_t)},
	{.name = "a", .type = ecs_id(ecs_u8_t)}
	}});


}