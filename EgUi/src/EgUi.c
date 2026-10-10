#include "EgUi.h"

#include <EgPhysics.h>
#include <EgButtons.h>
#include <ecsx.h>

ECS_COMPONENT_DECLARE(EgUiParentClearance);

static EgUiParentClearance EgUiParentClearance_Compute(EgShapesRectangle const *parent, Position2 center,
	EgShapesRectangle const *child)
{
	float half_parent_w = parent->w * 0.5f;
	float half_parent_h = parent->h * 0.5f;
	float half_child_w  = child->w * 0.5f;
	float half_child_h  = child->h * 0.5f;
	return (EgUiParentClearance){
		.left   = -half_parent_w - (center.x - half_child_w),
		.right  = center.x + half_child_w - half_parent_w,
		.bottom = -half_parent_h - (center.y - half_child_h),
		.top    = center.y + half_child_h - half_parent_h,
	};
}

static void EgUiParentClearance_Update(ecs_iter_t *it)
{
	EgUiParentClearance     *clearance = ecs_field_self(it, EgUiParentClearance, 0);
	EgShapesRectangle const *child     = ecs_field_self(it, EgShapesRectangle, 1);
	Position2 const         *pos       = ecs_field_self(it, Position2, 2);
	EgShapesRectangle const *parent    = ecs_field_shared(it, EgShapesRectangle, 3);
	for (int32_t i = 0; i < it->count; ++i) {
		clearance[i] = EgUiParentClearance_Compute(parent, pos[i], &child[i]);
	}
}

void EgUiImport(ecs_world_t *world)
{
	ECS_IMPORT(world, EgPhysics);
	ECS_IMPORT(world, EgShapes);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgButtons);
	ECS_IMPORT(world, EgUiButtons);
	ECS_IMPORT(world, EgUiFlows);
	ECS_IMPORT(world, EgUiFreeforms);
	ECS_IMPORT(world, EgUiBounds);

	ECS_MODULE(world, EgUi);
	ecs_set_name_prefix(world, "EgUi");

	ECS_COMPONENT_DEFINE(world, EgUiParentClearance);
	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgUiParentClearance),
	.members = {
	{.name = "left", .type = ecs_id(ecs_f32_t)},
	{.name = "right", .type = ecs_id(ecs_f32_t)},
	{.name = "bottom", .type = ecs_id(ecs_f32_t)},
	{.name = "top", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiParentClearance_Update"}),
	.phase       = EcsPreStore,
	.callback    = EgUiParentClearance_Update,
	.query.terms = {
	{.id = ecs_id(EgUiParentClearance), .inout = EcsOut},
	{.id = ecs_id(EgShapesRectangle), .inout = EcsIn},
	{.id = ecs_id(Position2), .inout = EcsIn},
	{.id = ecs_id(EgShapesRectangle), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn},
	{.id = EcsDisabled, .oper = EcsOptional},
	}});
}
