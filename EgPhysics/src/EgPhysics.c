#include "EgPhysics.h"

ECS_COMPONENT_DECLARE(EgPhysicsWorldDef);
ECS_COMPONENT_DECLARE(EgPhysicsBodyDef);
ECS_COMPONENT_DECLARE(EgPhysicsBox);

void EgPhysicsImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgPhysics);
	ecs_set_name_prefix(world, "EgPhysics");

	ECS_COMPONENT_DEFINE(world, EgPhysicsWorldDef);
	ECS_COMPONENT_DEFINE(world, EgPhysicsBodyDef);
	ECS_COMPONENT_DEFINE(world, EgPhysicsBox);

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgPhysicsWorldDef),
	.members = {
	{.name = "gravity_x", .type = ecs_id(ecs_f32_t)},
	{.name = "gravity_y", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgPhysicsBodyDef),
	.members = {
	{.name = "type", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgPhysicsBox),
	.members = {
	{.name = "half_width", .type = ecs_id(ecs_f32_t)},
	{.name = "half_height", .type = ecs_id(ecs_f32_t)},
	{.name = "density", .type = ecs_id(ecs_f32_t)},
	{.name = "friction", .type = ecs_id(ecs_f32_t)},
	}});
}
