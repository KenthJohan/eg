#include "EgEvents.h"


ECS_COMPONENT_DECLARE(EgUserEvent);



void EgEventsImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgEvents);
	ECS_COMPONENT_DEFINE(world, EgUserEvent);

	ecs_set_name_prefix(world, "Eg");

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgUserEvent),
	.members = {
	{ .name = "keyboard", .type = ecs_id(ecs_u64_t), EG_NUM_KEYS64 },
	{ .name = "keyboard_up", .type = ecs_id(ecs_u64_t), EG_NUM_KEYS64 },
	{ .name = "keyboard_down", .type = ecs_id(ecs_u64_t), EG_NUM_KEYS64 },
	{ .name = "mouse_x", .type = ecs_id(ecs_i32_t)},
	{ .name = "mouse_y", .type = ecs_id(ecs_i32_t)},
	{ .name = "mouse_dx", .type = ecs_id(ecs_i32_t)},
	{ .name = "mouse_dy", .type = ecs_id(ecs_i32_t)}
	}
	});

	
}

