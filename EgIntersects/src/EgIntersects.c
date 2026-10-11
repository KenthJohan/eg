#include "EgIntersects.h"
#include <EgSpatials.h>
#include <EgShapes.h>
#include <EgPhysics.h>
#include <math.h>
#include <ecsx.h>

ECS_COMPONENT_DECLARE(EgIntersectsRectangleBorder);

void EgIntersectsImport(ecs_world_t *world)
{
	ECS_IMPORT(world, EgPhysics);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgShapes);

	ECS_MODULE(world, EgIntersects);
	ecs_set_name_prefix(world, "EgIntersects");

	ECS_COMPONENT_DEFINE(world, EgIntersectsRectangleBorder);

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgIntersectsRectangleBorder),
	.members = {
	{.name = "left", .type = ecs_id(ecs_f32_t)},
	{.name = "right", .type = ecs_id(ecs_f32_t)},
	{.name = "bottom", .type = ecs_id(ecs_f32_t)},
	{.name = "top", .type = ecs_id(ecs_f32_t)},
	{.name = "minimum", .type = ecs_id(ecs_f32_t)},
	}});
}
