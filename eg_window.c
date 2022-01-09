#include "eg_window.h"


ECS_COMPONENT_DECLARE(EgWindow);
ECS_COMPONENT_DECLARE(EgDraw);



void FlecsComponentsEgWindowImport(ecs_world_t *world)
{
	ECS_MODULE(world, FlecsComponentsEgWindow);
	ECS_COMPONENT_DEFINE(world, EgWindow);
	ECS_COMPONENT_DEFINE(world, EgDraw);

	ecs_set_name_prefix(world, "Eg");

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgWindow),
	.members = {
	{ .name = "title", .type = ecs_id(ecs_string_t) },
	{ .name = "counter", .type = ecs_id(ecs_u64_t) },
	{ .name = "should_destroy", .type = ecs_id(ecs_bool_t) }
	}
	});

	
}

