#include "EgUserinput.h"
#include "eg_basics.h"
#include <math.h>
#include <stdio.h>


ECS_TAG_DECLARE(EgUserinput);
ECS_COMPONENT_DECLARE(EgMouse);
ECS_COMPONENT_DECLARE(EgKeyboard);



void EgUserinputImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgUserinput);
	ecs_set_name_prefix(world, "Eg");
	
	
	ECS_TAG_DEFINE(world, EgUserinput);
	ECS_COMPONENT_DEFINE(world, EgMouse);
	ECS_COMPONENT_DEFINE(world, EgKeyboard);


	ecs_struct(world, {
	.entity = ecs_id(EgKeyboard),
	.members = {
	{ .name = "keys", .type = ecs_id(ecs_u8_t), .count = EG_KEYBOARD_SIZE }
	}
	});

	ecs_struct(world, {
	.entity = ecs_id(EgMouse),
	.members = {
	{ .name = "left", .type = ecs_id(ecs_u8_t) },
	{ .name = "right", .type = ecs_id(ecs_u8_t) },
	{ .name = "scroll_x", .type = ecs_id(ecs_f32_t) },
	{ .name = "scroll_y", .type = ecs_id(ecs_f32_t) }
	}
	});

}
