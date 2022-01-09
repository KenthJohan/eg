#include "eg_geometry.h"


ECS_COMPONENT_DECLARE(EgRectangleF32);
ECS_COMPONENT_DECLARE(EgRectangleI32);
ECS_COMPONENT_DECLARE(EgPosition2F32);
ECS_COMPONENT_DECLARE(EgPosition2I32);
ECS_COMPONENT_DECLARE(EgVelocity2F32);


void FlecsComponentsEgGeometryImport(ecs_world_t *world)
{
	ECS_MODULE(world, FlecsComponentsEgGeometry);

	ECS_COMPONENT_DEFINE(world, EgRectangleF32);
	ECS_COMPONENT_DEFINE(world, EgRectangleI32);
	ECS_COMPONENT_DEFINE(world, EgPosition2F32);
	ECS_COMPONENT_DEFINE(world, EgPosition2I32);
	ECS_COMPONENT_DEFINE(world, EgVelocity2F32);

	ecs_set_name_prefix(world, "Eg");

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgPosition2F32),
	.members = {
	{ .name = "x", .type = ecs_id(ecs_f32_t) },
	{ .name = "y", .type = ecs_id(ecs_f32_t) }
	}
	});
	
	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgPosition2I32),
	.members = {
	{ .name = "x", .type = ecs_id(ecs_i32_t) },
	{ .name = "y", .type = ecs_id(ecs_i32_t) }
	}
	});

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgRectangleF32),
	.members = {
	{ .name = "width", .type = ecs_id(ecs_f32_t) },
	{ .name = "height", .type = ecs_id(ecs_f32_t) }
	}
	});
	
	/*
	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgRectangleI32),
	.members = {
	{ .name = "width", .type = ecs_id(ecs_i32_t) },
	{ .name = "height", .type = ecs_id(ecs_i32_t) }
	}
	});
	*/


}

