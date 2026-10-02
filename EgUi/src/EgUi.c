#include "EgUi.h"

#include <EgShapes.h>
#include <ecsx.h>
#include <math.h>

ECS_COMPONENT_DECLARE(EgUiFlow);
ECS_COMPONENT_DECLARE(EgUiDirection);

static void EgUiFlow_Reset(ecs_iter_t *it)
{
	EgUiFlow *flow = ecs_field_self(it, EgUiFlow, 0);
	for (int i = 0; i < it->count; ++i, ++flow) {
	}
}

static void EgUiFlow_Update(ecs_iter_t *it)
{
	EgUiFlow          *pf = ecs_field_shared(it, EgUiFlow, 0);
	EgShapesRectangle *pr = ecs_field_shared(it, EgShapesRectangle, 1);
	Position2         *cp = ecs_field_self(it, Position2, 2);
	EgShapesRectangle *cr = ecs_field_self(it, EgShapesRectangle, 3);
}

void EgUiImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgUi);
	ecs_set_name_prefix(world, "EgUi");

	ECS_COMPONENT_DEFINE(world, EgUiFlow);
	ECS_COMPONENT_DEFINE(world, EgUiDirection);

	ecs_enum_init(world,
	&(ecs_enum_desc_t){
	.entity    = ecs_id(EgUiDirection),
	.constants = {
	{.name = "None", .value = EgUiDirectionNone},
	{.name = "Right", .value = EgUiDirectionRight},
	{.name = "Left", .value = EgUiDirectionLeft},
	{.name = "Up", .value = EgUiDirectionUp},
	{.name = "Down", .value = EgUiDirectionDown},
	}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgUiFlow),
	.members = {
	{.name = "direction", .type = ecs_id(EgUiDirection)},
	{.name = "wrap", .type = ecs_id(EgUiDirection)},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiFlow_Reset"}),
	.phase       = EcsOnUpdate,
	.callback    = EgUiFlow_Reset,
	.query.terms = {
	{.id = ecs_id(EgUiFlow), .inout = EcsInOut},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiFlow_Update"}),
	.phase       = EcsOnUpdate,
	.callback    = EgUiFlow_Update,
	.query.terms = {
	{.id = ecs_id(EgUiFlow), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsInOut},
	{.id = ecs_id(EgShapesRectangle), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn},
	{.id = ecs_id(Position2), .inout = EcsOut},
	{.id = ecs_id(EgShapesRectangle), .inout = EcsIn},
	}});
}
