#include "egwindows.h"

ECS_COMPONENT_DECLARE(Window);
ECS_COMPONENT_DECLARE(KeyActionToggleEntity);
ECS_COMPONENT_DECLARE(KeyboardController);


void EgWindowsImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgWindows);
	ecs_set_name_prefix(world, "EgWindows");


	ECS_COMPONENT_DEFINE(world, Window);
	ECS_COMPONENT_DEFINE(world, KeyActionToggleEntity);
	ECS_COMPONENT_DEFINE(world, KeyboardController);

	ecs_struct(world,
	{.entity = ecs_id(Window),
	.members = {
	{.name = "w", .type = ecs_id(ecs_f32_t)},
	{.name = "h", .type = ecs_id(ecs_f32_t)},
	{.name = "mouse_x", .type = ecs_id(ecs_f32_t)},
	{.name = "mouse_y", .type = ecs_id(ecs_f32_t)},
	{.name = "mouse_dx", .type = ecs_id(ecs_f32_t)},
	{.name = "mouse_dy", .type = ecs_id(ecs_f32_t)},
	{.name = "mouse_left", .type = ecs_id(ecs_u8_t)},
	{.name = "mouse_right", .type = ecs_id(ecs_u8_t)},
	{.name = "mouse_left_edge", .type = ecs_id(ecs_u8_t)},
	{.name = "mouse_right_edge", .type = ecs_id(ecs_u8_t)},
	{.name = "keys", .type = ecs_id(ecs_u8_t), .count = 512},
	{.name = "keys_edge", .type = ecs_id(ecs_u8_t), .count = 512},
	}});
	
	ecs_struct(world,
	{.entity = ecs_id(KeyActionToggleEntity),
	.members = {
	{.name = "keycode", .type = ecs_id(ecs_i32_t)},
	{.name = "entity", .type = ecs_id(ecs_entity_t)},
	}});


}