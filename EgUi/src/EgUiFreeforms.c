#include "EgUi.h"

#include <EgShapes.h>
#include <EgSpatials.h>
#include <ecsx.h>

#include "EgUi/EgUiFreeforms.h"

ECS_COMPONENT_DECLARE(EgUiFreeformsLayout);
ECS_COMPONENT_DECLARE(EgUiFreeformsAnchorKind);
ECS_COMPONENT_DECLARE(EgUiFreeformsAnchor);

static void EgUiFreeformsAnchorKind_Dir(EgUiFreeformsAnchorKind kind, float *x, float *y)
{
	static const float dir[][2] = {
	{0, 0}, {-1, 1}, {0, 1}, {1, 1}, {-1, 0}, {1, 0}, {-1, -1}, {0, -1}, {1, -1}};
	if ((int)kind < 0 || (int)kind > EgUiFreeformsAnchorKindBottomRight) {
		kind = EgUiFreeformsAnchorKindMiddle;
	}
	*x = dir[kind][0];
	*y = dir[kind][1];
}

static void EgUiFreeformsAnchor_Update(ecs_iter_t *it)
{
	EgUiFreeformsAnchor     *anchor = ecs_field_self(it, EgUiFreeformsAnchor, 0);
	EgShapesRectangle const *rect   = ecs_field_self(it, EgShapesRectangle, 1);
	Scale2 const            *scale  = ecs_field_self(it, Scale2, 2);
	Position2               *pos    = ecs_field_self(it, Position2, 3);
	EgShapesRectangle const *parent = ecs_field_is_set(it, 4) ? ecs_field_shared(it, EgShapesRectangle, 4) : NULL;

	float parent_half_w = parent ? parent->w * 0.5f : 0.0f;
	float parent_half_h = parent ? parent->h * 0.5f : 0.0f;
	for (int32_t i = 0; i < it->count; ++i) {
		float parent_x, parent_y, pivot_x, pivot_y;
		EgUiFreeformsAnchorKind_Dir(anchor[i].parent, &parent_x, &parent_y);
		EgUiFreeformsAnchorKind_Dir(anchor[i].pivot, &pivot_x, &pivot_y);
		pos[i].x = parent_x * parent_half_w + anchor[i].x - pivot_x * rect[i].w * 0.5f * scale[i].x;
		pos[i].y = parent_y * parent_half_h + anchor[i].y - pivot_y * rect[i].h * 0.5f * scale[i].y;
	}
}

void EgUiFreeformsImport(ecs_world_t *world)
{
	ECS_IMPORT(world, EgShapes);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgUiFlows);

	ECS_MODULE(world, EgUiFreeforms);
	ecs_set_name_prefix(world, "EgUiFreeforms");

	ECS_COMPONENT_DEFINE(world, EgUiFreeformsLayout);
	ECS_COMPONENT_DEFINE(world, EgUiFreeformsAnchorKind);
	ECS_COMPONENT_DEFINE(world, EgUiFreeformsAnchor);

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgUiFreeformsLayout),
	.members = {
	{.name = "dummy", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_enum_init(world,
	&(ecs_enum_desc_t){
	.entity    = ecs_id(EgUiFreeformsAnchorKind),
	.constants = {
	{.name = "Middle", .value = EgUiFreeformsAnchorKindMiddle},
	{.name = "TopLeft", .value = EgUiFreeformsAnchorKindTopLeft},
	{.name = "Top", .value = EgUiFreeformsAnchorKindTop},
	{.name = "TopRight", .value = EgUiFreeformsAnchorKindTopRight},
	{.name = "Left", .value = EgUiFreeformsAnchorKindLeft},
	{.name = "Right", .value = EgUiFreeformsAnchorKindRight},
	{.name = "BottomLeft", .value = EgUiFreeformsAnchorKindBottomLeft},
	{.name = "Bottom", .value = EgUiFreeformsAnchorKindBottom},
	{.name = "BottomRight", .value = EgUiFreeformsAnchorKindBottomRight},
	}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgUiFreeformsAnchor),
	.members = {
	{.name = "parent", .type = ecs_id(EgUiFreeformsAnchorKind)},
	{.name = "pivot", .type = ecs_id(EgUiFreeformsAnchorKind)},
	{.name = "x", .type = ecs_id(ecs_f32_t)},
	{.name = "y", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_entity_t after_bounds = ecs_entity(world, {.name = "EgUiFreeformsAfterBounds"});
	ecs_add_id(world, after_bounds, EcsPhase);
	ecs_add_pair(world, after_bounds, EcsDependsOn, EcsPreStore);

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiFreeformsAnchor_Update"}),
	.phase       = after_bounds,
	.callback    = EgUiFreeformsAnchor_Update,
	.query.terms = {
	{.id = ecs_id(EgUiFreeformsAnchor), .inout = EcsIn},
	{.id = ecs_id(EgShapesRectangle), .inout = EcsIn},
	{.id = ecs_id(Scale2), .inout = EcsIn},
	{.id = ecs_id(Position2), .src.id = EcsSelf, .inout = EcsOut},
	{.id = ecs_id(EgShapesRectangle), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn, .oper = EcsOptional},
	{.id = ecs_id(EgUiFlowsFlow), .trav = EcsChildOf, .src.id = EcsUp, .oper = EcsNot},
	}});
}
