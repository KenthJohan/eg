#include "eg_userinput.h"


ECS_COMPONENT_DECLARE(EgUserinput);



void FlecsComponentsEgUserinputImport(ecs_world_t *world)
{
	ECS_MODULE(world, FlecsComponentsEgUserinput);
	ECS_COMPONENT_DEFINE(world, EgUserinput);

	ecs_set_name_prefix(world, "Eg");

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgUserinput),
	.members = {
	{ .name = "keyboard", .type = ecs_id(ecs_u64_t), 4 },
	{ .name = "mouse_x", .type = ecs_id(ecs_i32_t)},
	{ .name = "mouse_y", .type = ecs_id(ecs_i32_t)},
	{ .name = "mouse_dx", .type = ecs_id(ecs_i32_t)},
	{ .name = "mouse_dy", .type = ecs_id(ecs_i32_t)}
	}
	});

	
}

