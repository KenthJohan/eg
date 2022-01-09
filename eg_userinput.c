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
	{ .name = "keyboard", .type = ecs_id(ecs_u64_t), 4 }
	}
	});

	
}

