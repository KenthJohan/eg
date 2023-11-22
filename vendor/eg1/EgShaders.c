#include "EgShaders.h"


ECS_COMPONENT_DECLARE(EgShader);






void EgShadersImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgShaders);

	ECS_COMPONENT_DEFINE(world, EgShader);

	ecs_set_name_prefix(world, "Eg");

	/*
	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgRectangleF32),
	.members = {
	{ .name = "width", .type = ecs_id(ecs_f32_t) },
	{ .name = "height", .type = ecs_id(ecs_f32_t) }
	}
	});
	*/


}

