#include "egshapes.h"



ECS_COMPONENT_DECLARE(Torus);
ECS_COMPONENT_DECLARE(Cylinder);
ECS_COMPONENT_DECLARE(Sphere);
ECS_COMPONENT_DECLARE(Line);
ECS_COMPONENT_DECLARE(Rectangle);

void EgShapesImport(ecs_world_t *world)
{
	ECS_MODULE(world, Shapes);
	ecs_set_name_prefix(world, "EgShapes");

	ECS_COMPONENT_DEFINE(world, Torus);
	ECS_COMPONENT_DEFINE(world, Cylinder);
	ECS_COMPONENT_DEFINE(world, Sphere);
	ECS_COMPONENT_DEFINE(world, Line);

	ecs_struct(world,
	{.entity = ecs_id(Line),
	.members = {
	{.name = "a", .type = ecs_id(ecs_f32_t), .count = 3},
	{.name = "b", .type = ecs_id(ecs_f32_t), .count = 3},
	}});

	ecs_struct(world,
	{.entity = ecs_id(Torus),
	.members = {
	{.name = "radius", .type = ecs_id(ecs_f32_t)},
	{.name = "ring_radius", .type = ecs_id(ecs_f32_t)},
	{.name = "rings", .type = ecs_id(ecs_i32_t)},
	{.name = "sides", .type = ecs_id(ecs_i32_t)},
	{.name = "random_colors", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(Cylinder),
	.members = {
	{.name = "radius", .type = ecs_id(ecs_f32_t)},
	{.name = "height", .type = ecs_id(ecs_f32_t)},
	{.name = "slices", .type = ecs_id(ecs_i32_t)},
	{.name = "stacks", .type = ecs_id(ecs_i32_t)},
	{.name = "random_colors", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(Sphere),
	.members = {
	{.name = "radius", .type = ecs_id(ecs_f32_t)},
	{.name = "slices", .type = ecs_id(ecs_i32_t)},
	{.name = "stacks", .type = ecs_id(ecs_i32_t)},
	{.name = "random_colors", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(Rectangle),
	.members = {
	{.name = "w", .type = ecs_id(ecs_f32_t)},
	{.name = "h", .type = ecs_id(ecs_f32_t)},
	}});

}