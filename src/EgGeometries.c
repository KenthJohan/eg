#include "EgGeometries.h"


ECS_COMPONENT_DECLARE(EgRectangleF32);
ECS_COMPONENT_DECLARE(EgRectangleI32);
ECS_COMPONENT_DECLARE(EgBoxF32);
ECS_DECLARE(EgPrimitivePoint);
ECS_DECLARE(EgPrimitiveLine);
ECS_DECLARE(EgPrimitiveRectangle);






void EgGeometriesImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgGeometries);
	ecs_set_name_prefix(world, "Eg");

	ECS_COMPONENT_DEFINE(world, EgRectangleF32);
	ECS_COMPONENT_DEFINE(world, EgRectangleI32);
	ECS_COMPONENT_DEFINE(world, EgBoxF32);
	ECS_TAG_DEFINE(world, EgPrimitivePoint);
	ECS_TAG_DEFINE(world, EgPrimitiveLine);
	ECS_TAG_DEFINE(world, EgPrimitiveRectangle);


	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity = ecs_entity(world, {.id = ecs_id(EgRectangleF32)}),
	.members = {
	{ .name = "width", .type = ecs_id(ecs_f32_t) },
	{ .name = "height", .type = ecs_id(ecs_f32_t) }
	}
	});

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity = ecs_entity(world, {.id = ecs_id(EgRectangleI32)}),
	.members = {
	{ .name = "width", .type = ecs_id(ecs_i32_t) },
	{ .name = "height", .type = ecs_id(ecs_i32_t) }
	}
	});

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity = ecs_entity(world, {.id = ecs_id(EgBoxF32)}),
	.members = {
	{ .name = "x", .type = ecs_id(ecs_f32_t) },
	{ .name = "y", .type = ecs_id(ecs_f32_t) },
	{ .name = "z", .type = ecs_id(ecs_f32_t) }
	}
	});

}

