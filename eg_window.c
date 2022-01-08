#include "eg_window.h"


ECS_COMPONENT_DECLARE(EgWindow);



void FlecsComponentsEgWindowImport(ecs_world_t *world)
{
	ECS_MODULE(world, FlecsComponentsEgWindow);
	ECS_COMPONENT_DEFINE(world, EgWindow);

	ecs_set_name_prefix(world, "Eg");

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgWindow),
	.members = {
	{ .name = "title", .type = ecs_id(ecs_string_t) },
	{ .name = "userinput", .type = ecs_id(ecs_u64_t), 4 }
	}
	});

}

