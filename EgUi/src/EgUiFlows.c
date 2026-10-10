#include "EgUi.h"

#include <EgShapes.h>
#include <EgSpatials.h>
#include <ecsx.h>

ECS_COMPONENT_DECLARE(EgUiFlowsFlow);
ECS_COMPONENT_DECLARE(EgUiDirection);
ECS_TAG_DECLARE(EgUiFlowsFlowUnplaced);

#define EG_UI_FLOWS_LAYOUT_EPSILON 1e-4f

static EgUiParentClearance EgUiFlows_ParentClearanceCompute(EgShapesRectangle const *parent,
	Position2 center, EgShapesRectangle const *child)
{
	float half_parent_w = parent->w * 0.5f;
	float half_parent_h = parent->h * 0.5f;
	float half_child_w = child->w * 0.5f;
	float half_child_h = child->h * 0.5f;
	return (EgUiParentClearance){
		.left = -half_parent_w - (center.x - half_child_w),
		.right = center.x + half_child_w - half_parent_w,
		.bottom = -half_parent_h - (center.y - half_child_h),
		.top = center.y + half_child_h - half_parent_h,
	};
}

static bool EgUiFlows_ParentClearanceFits(EgUiParentClearance clearance)
{
	return clearance.left <= EG_UI_FLOWS_LAYOUT_EPSILON && clearance.right <= EG_UI_FLOWS_LAYOUT_EPSILON &&
		clearance.bottom <= EG_UI_FLOWS_LAYOUT_EPSILON && clearance.top <= EG_UI_FLOWS_LAYOUT_EPSILON;
}

static bool EgUiFlowsFlow_PrimaryOverflows(EgUiParentClearance clearance, int axis, float sign)
{
	if (axis == 0) {
		return sign > 0.0f ? clearance.right > EG_UI_FLOWS_LAYOUT_EPSILON
					   : clearance.left > EG_UI_FLOWS_LAYOUT_EPSILON;
	}
	return sign > 0.0f ? clearance.top > EG_UI_FLOWS_LAYOUT_EPSILON
				   : clearance.bottom > EG_UI_FLOWS_LAYOUT_EPSILON;
}

static Position2 EgUiFlowsFlow_GetPosition(int primary_axis, float primary_sign, int wrap_axis,
	float wrap_sign, bool can_wrap, float cursor_primary, float cursor_wrap,
	EgShapesRectangle const *child)
{
	float size[2] = {child->w, child->h};
	Position2 pos = {0.0f, 0.0f};
	if (primary_axis == 0) {
		pos.x = cursor_primary + primary_sign * size[0] * 0.5f;
	} else {
		pos.y = cursor_primary + primary_sign * size[1] * 0.5f;
	}
	if (can_wrap) {
		if (wrap_axis == 0) {
			pos.x = cursor_wrap + wrap_sign * size[0] * 0.5f;
		} else {
			pos.y = cursor_wrap + wrap_sign * size[1] * 0.5f;
		}
	}
	return pos;
}

static bool EgUiFlowsFlow_GetAxis(EgUiDirection direction, int *axis, float *sign)
{
	switch (direction) {
	case EgUiDirectionRight:
		*axis = 0;
		*sign = 1.0f;
		return true;
	case EgUiDirectionLeft:
		*axis = 0;
		*sign = -1.0f;
		return true;
	case EgUiDirectionUp:
		*axis = 1;
		*sign = 1.0f;
		return true;
	case EgUiDirectionDown:
		*axis = 1;
		*sign = -1.0f;
		return true;
	default:
		return false;
	}
}

static void EgUiFlowsFlow_Reset(ecs_iter_t *it)
{
	EgUiFlowsFlow     *flow   = ecs_field_self(it, EgUiFlowsFlow, 0);
	EgShapesRectangle *bounds = ecs_field_self(it, EgShapesRectangle, 1);
	for (int i = 0; i < it->count; ++i) {
		int   primary_axis = 0;
		float primary_sign = 0.0f;
		int   wrap_axis    = 0;
		float wrap_sign    = 0.0f;
		if (!EgUiFlowsFlow_GetAxis(flow[i].direction, &primary_axis, &primary_sign)) {
			flow[i].cursor_primary    = 0.0f;
			flow[i].cursor_wrap       = 0.0f;
			flow[i].line_wrap_extent  = 0.0f;
			flow[i].line_has_children = false;
			continue;
		}
		bool  can_wrap            = EgUiFlowsFlow_GetAxis(flow[i].wrap, &wrap_axis, &wrap_sign) && wrap_axis != primary_axis;
		float half_width          = bounds[i].w * 0.5f;
		float half_height         = bounds[i].h * 0.5f;
		float primary_limit       = primary_axis == 0 ? half_width : half_height;
		float wrap_limit          = wrap_axis == 0 ? half_width : half_height;
		flow[i].cursor_primary    = primary_sign > 0.0f ? -primary_limit : primary_limit;
		flow[i].cursor_wrap       = can_wrap ? (wrap_sign > 0.0f ? -wrap_limit : wrap_limit) : 0.0f;
		flow[i].line_wrap_extent  = 0.0f;
		flow[i].line_has_children = false;
	}
}

static void EgUiFlowsFlow_Update(ecs_iter_t *it)
{
	EgUiFlowsFlow      *flow            = ecs_field_shared(it, EgUiFlowsFlow, 0);
	EgShapesRectangle  *parent_rect     = ecs_field_shared(it, EgShapesRectangle, 1);
	Position2          *child_positions = ecs_field_self(it, Position2, 2);
	EgShapesRectangle  *child_rects     = ecs_field_self(it, EgShapesRectangle, 3);

	int   primary_axis;
	float primary_sign;
	int   wrap_axis = 0;
	float wrap_sign = 0.0f;
	if (!EgUiFlowsFlow_GetAxis(flow->direction, &primary_axis, &primary_sign)) {
		for (int i = 0; i < it->count; ++i) {
			if (ecs_has_id(it->world, it->entities[i], ecs_id(EgUiFlowsFlowUnplaced))) {
				ecs_remove_id(it->world, it->entities[i], ecs_id(EgUiFlowsFlowUnplaced));
				ecs_enable(it->world, it->entities[i], true);
			}
		}
		return;
	}
	bool  can_wrap      = EgUiFlowsFlow_GetAxis(flow->wrap, &wrap_axis, &wrap_sign) && wrap_axis != primary_axis;
	float primary_limit = (primary_axis == 0 ? parent_rect->w : parent_rect->h) * 0.5f;

	for (int i = 0; i < it->count; ++i) {
		bool flow_unplaced = ecs_has_id(it->world, it->entities[i], ecs_id(EgUiFlowsFlowUnplaced));
		if (ecs_has_id(it->world, it->entities[i], EcsDisabled) && !flow_unplaced) {
			continue;
		}

		float child_size[2] = {child_rects[i].w, child_rects[i].h};
		float primary_size = child_size[primary_axis];
		float candidate_primary = flow->cursor_primary;
		float candidate_wrap = flow->cursor_wrap;
		float candidate_extent = flow->line_wrap_extent;
		bool candidate_has_children = flow->line_has_children;
		Position2 candidate_pos = EgUiFlowsFlow_GetPosition(primary_axis, primary_sign, wrap_axis,
			wrap_sign, can_wrap, candidate_primary, candidate_wrap, &child_rects[i]);
		EgUiParentClearance clearance = EgUiFlows_ParentClearanceCompute(parent_rect,
			candidate_pos, &child_rects[i]);
		if (can_wrap && candidate_has_children &&
			EgUiFlowsFlow_PrimaryOverflows(clearance, primary_axis, primary_sign)) {
			candidate_wrap += wrap_sign * candidate_extent;
			candidate_primary = primary_sign > 0.0f ? -primary_limit : primary_limit;
			candidate_extent = 0.0f;
			candidate_has_children = false;
			candidate_pos = EgUiFlowsFlow_GetPosition(primary_axis, primary_sign, wrap_axis,
				wrap_sign, can_wrap, candidate_primary, candidate_wrap, &child_rects[i]);
			clearance = EgUiFlows_ParentClearanceCompute(parent_rect, candidate_pos, &child_rects[i]);
		}

		if (!EgUiFlows_ParentClearanceFits(clearance)) {
			if (!flow_unplaced) {
				ecs_add_id(it->world, it->entities[i], ecs_id(EgUiFlowsFlowUnplaced));
				ecs_enable(it->world, it->entities[i], false);
			}
			continue;
		}

		if (flow_unplaced) {
			ecs_remove_id(it->world, it->entities[i], ecs_id(EgUiFlowsFlowUnplaced));
			ecs_enable(it->world, it->entities[i], true);
		}
		child_positions[i] = candidate_pos;
		candidate_primary += primary_sign * primary_size;
		if (can_wrap && child_size[wrap_axis] > candidate_extent) {
			candidate_extent = child_size[wrap_axis];
		}
		flow->cursor_primary = candidate_primary;
		flow->cursor_wrap = candidate_wrap;
		flow->line_wrap_extent = candidate_extent;
		flow->line_has_children = true;
	}
}

void EgUiFlowsImport(ecs_world_t *world)
{
	ECS_IMPORT(world, EgShapes);
	ECS_IMPORT(world, EgSpatials);

	ECS_MODULE(world, EgUiFlows);
	ecs_set_name_prefix(world, "EgUiFlows");

	ECS_COMPONENT_DEFINE(world, EgUiFlowsFlow);
	ECS_COMPONENT_DEFINE(world, EgUiDirection);
	ECS_TAG_DEFINE(world, EgUiFlowsFlowUnplaced);

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
	.entity  = ecs_id(EgUiFlowsFlow),
	.members = {
	{.name = "direction", .type = ecs_id(EgUiDirection)},
	{.name = "wrap", .type = ecs_id(EgUiDirection)},
	{.name = "cursor_primary", .type = ecs_id(ecs_f32_t)},
	{.name = "cursor_wrap", .type = ecs_id(ecs_f32_t)},
	{.name = "line_wrap_extent", .type = ecs_id(ecs_f32_t)},
	{.name = "line_has_children", .type = ecs_id(ecs_bool_t)},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiFlowsFlow_Reset"}),
	.phase       = EcsOnUpdate,
	.callback    = EgUiFlowsFlow_Reset,
	.query.terms = {
	{.id = ecs_id(EgUiFlowsFlow), .inout = EcsInOut},
	{.id = ecs_id(EgShapesRectangle), .inout = EcsIn},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiFlowsFlow_Update"}),
	.phase       = EcsOnUpdate,
	.callback    = EgUiFlowsFlow_Update,
	.query.terms = {
	{.id = ecs_id(EgUiFlowsFlow), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsInOut},
	{.id = ecs_id(EgShapesRectangle), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn},
	{.id = ecs_id(Position2), .inout = EcsOut},
	{.id = ecs_id(EgShapesRectangle), .inout = EcsIn},
	{.id = ecs_id(EgUiFlowsFlowUnplaced), .oper = EcsOptional},
	{.id = EcsDisabled, .oper = EcsOptional},
	{.id = EcsDisabled, .trav = EcsChildOf, .src.id = EcsUp, .oper = EcsNot},
	}});
}
