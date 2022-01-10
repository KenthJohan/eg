#include "eg_geometry.h"


ECS_COMPONENT_DECLARE(EgRectangleF32);
ECS_COMPONENT_DECLARE(EgRectangleI32);


void FlecsComponentsEgGeometryImport(ecs_world_t *world)
{
	ECS_MODULE(world, FlecsComponentsEgGeometry);

	ECS_COMPONENT_DEFINE(world, EgRectangleF32);
	ECS_COMPONENT_DEFINE(world, EgRectangleI32);

	ecs_set_name_prefix(world, "Eg");

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgRectangleF32),
	.members = {
	{ .name = "width", .type = ecs_id(ecs_f32_t) },
	{ .name = "height", .type = ecs_id(ecs_f32_t) }
	}
	});



}

