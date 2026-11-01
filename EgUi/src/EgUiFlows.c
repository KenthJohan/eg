#include "EgUi/EgUiFlows.h"
#include "EgUi/EgUiFreeforms.h"
#include <EgShapes.h>
#include <EgSpatials.h>
#include <ecsx.h>

ECS_COMPONENT_DECLARE(EgUiFlowsFlow);
ECS_TAG_DECLARE(EgUiFlowsFlowUnplaced);

#define EG_UI_FLOWS_LAYOUT_EPSILON 1e-4f

static EgUiParentClearance EgUiFlows_ParentClearanceCompute(EgShapesRectangle const *parent,
Position2 center, EgShapesRectangle const *child)
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

static bool EgUiFlows_ParentClearanceFits(EgUiParentClearance clearance)
{
	return clearance.left <= EG_UI_FLOWS_LAYOUT_EPSILON && clearance.right <= EG_UI_FLOWS_LAYOUT_EPSILON &&
	       clearance.bottom <= EG_UI_FLOWS_LAYOUT_EPSILON && clearance.top <= EG_UI_FLOWS_LAYOUT_EPSILON;
}

static bool EgUiFlowsFlow_PrimaryOverflows(EgUiParentClearance clearance, int axis, float sign)
{
	if (axis == 0) {
		return sign > 0.0f ? clearance.right > EG_UI_FLOWS_LAYOUT_EPSILON : clearance.left > EG_UI_FLOWS_LAYOUT_EPSILON;
	}
	return sign > 0.0f ? clearance.top > EG_UI_FLOWS_LAYOUT_EPSILON : clearance.bottom > EG_UI_FLOWS_LAYOUT_EPSILON;
}

static Position2 EgUiFlowsFlow_GetPosition(int primary_axis, float primary_sign, int wrap_axis,
float wrap_sign, bool can_wrap, float cursor_primary, float cursor_wrap,
EgShapesRectangle const *child)
{
	float     size[2] = {child->w, child->h};
	Position2 pos     = {0.0f, 0.0f};
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

static bool EgUiFlowsFlow_GetAxis(EgUiDir direction, int *axis, float *sign)
{
	switch (direction) {
	case EgUiDirRight:
		*axis = 0;
		*sign = 1.0f;
		return true;
	case EgUiDirLeft:
		*axis = 0;
		*sign = -1.0f;
		return true;
	case EgUiDirUp:
		*axis = 1;
		*sign = 1.0f;
		return true;
	case EgUiDirDown:
		*axis = 1;
		*sign = -1.0f;
		return true;
	default:
		return false;
	}
}

static void EgUiFlowsFlow_Update(ecs_iter_t *it)
{
	EgUiFlowsFlow     *flows          = ecs_field_self(it, EgUiFlowsFlow, 0);
	EgShapesRectangle *bounds         = ecs_field_self(it, EgShapesRectangle, 1);
	ecs_query_t       *children_query = it->ctx;
	if (children_query == NULL) {
		ecs_err("Flow child query is not initialized");
		return;
	}

	for (int i = 0; i < it->count; ++i) {
		int   primary_axis = 0;
		float primary_sign = 0.0f;
		int   wrap_axis    = 0;
		float wrap_sign    = 0.0f;

		if (!EgUiFlowsFlow_GetAxis(flows[i].direction, &primary_axis, &primary_sign)) {
			flows[i].cursor_primary    = 0.0f;
			flows[i].cursor_wrap       = 0.0f;
			flows[i].line_wrap_extent  = 0.0f;
			flows[i].line_has_children = false;

			ecs_iter_t children = ecs_query_iter(it->stage, children_query);
			ecs_iter_set_group(&children, it->entities[i]);
			while (ecs_query_next(&children)) {
				for (int j = 0; j < children.count; ++j) {
					ecs_entity_t child = children.entities[j];
					if (ecs_has_id(it->world, child, ecs_id(EgUiFlowsFlowUnplaced))) {
						ecs_remove_id(it->stage, child, ecs_id(EgUiFlowsFlowUnplaced));
						ecs_enable(it->stage, child, true);
					}
				}
			}
			continue;
		}

		bool  can_wrap      = EgUiFlowsFlow_GetAxis(flows[i].wrap, &wrap_axis, &wrap_sign) && wrap_axis != primary_axis;
		float primary_limit = (primary_axis == 0 ? bounds[i].w : bounds[i].h) * 0.5f;
		float wrap_limit    = (wrap_axis == 0 ? bounds[i].w : bounds[i].h) * 0.5f;
		float cursor_primary = primary_sign > 0.0f ? -primary_limit : primary_limit;
		float cursor_wrap    = can_wrap ? (wrap_sign > 0.0f ? -wrap_limit : wrap_limit) : 0.0f;
		float line_extent    = 0.0f;
		bool  line_has_children = false;

		ecs_iter_t children = ecs_query_iter(it->stage, children_query);
		ecs_iter_set_group(&children, it->entities[i]);
		while (ecs_query_next(&children)) {
			Position2         *child_positions = ecs_field_w_size(&children, sizeof(Position2), 0);
			EgShapesRectangle *child_rects     = ecs_field_w_size(&children, sizeof(EgShapesRectangle), 1);
			for (int j = 0; j < children.count; ++j) {
				ecs_entity_t child = children.entities[j];
				bool flow_unplaced = ecs_has_id(it->world, child, ecs_id(EgUiFlowsFlowUnplaced));
				if (ecs_has_id(it->world, child, EcsDisabled) && !flow_unplaced) {
					continue;
				}

				float child_size[2] = {child_rects[j].w, child_rects[j].h};
				float candidate_primary = cursor_primary;
				float candidate_wrap = cursor_wrap;
				float candidate_extent = line_extent;
				bool candidate_has_children = line_has_children;
				Position2 candidate_pos = EgUiFlowsFlow_GetPosition(primary_axis, primary_sign,
				wrap_axis, wrap_sign, can_wrap, candidate_primary, candidate_wrap, &child_rects[j]);
				EgUiParentClearance clearance = EgUiFlows_ParentClearanceCompute(
				&bounds[i], candidate_pos, &child_rects[j]);

				if (can_wrap && candidate_has_children &&
				    EgUiFlowsFlow_PrimaryOverflows(clearance, primary_axis, primary_sign)) {
					candidate_wrap += wrap_sign * candidate_extent;
					candidate_primary = primary_sign > 0.0f ? -primary_limit : primary_limit;
					candidate_extent = 0.0f;
					candidate_has_children = false;
					candidate_pos = EgUiFlowsFlow_GetPosition(primary_axis, primary_sign, wrap_axis,
					wrap_sign, can_wrap, candidate_primary, candidate_wrap, &child_rects[j]);
					clearance = EgUiFlows_ParentClearanceCompute(
					&bounds[i], candidate_pos, &child_rects[j]);
				}

				if (!EgUiFlows_ParentClearanceFits(clearance)) {
					if (!flow_unplaced) {
						ecs_add_id(it->stage, child, ecs_id(EgUiFlowsFlowUnplaced));
						ecs_enable(it->stage, child, false);
					}
					continue;
				}

				if (flow_unplaced) {
					ecs_remove_id(it->stage, child, ecs_id(EgUiFlowsFlowUnplaced));
					ecs_enable(it->stage, child, true);
				}

				child_positions[j] = candidate_pos;
				candidate_primary += primary_sign * child_size[primary_axis];
				if (can_wrap && child_size[wrap_axis] > candidate_extent) {
					candidate_extent = child_size[wrap_axis];
				}

				cursor_primary = candidate_primary;
				cursor_wrap = candidate_wrap;
				line_extent = candidate_extent;
				line_has_children = true;
			}
		}

		flows[i].cursor_primary = cursor_primary;
		flows[i].cursor_wrap = cursor_wrap;
		flows[i].line_wrap_extent = line_extent;
		flows[i].line_has_children = line_has_children;
	}
}

void EgUiFlowsImport(ecs_world_t *world)
{
	ECS_IMPORT(world, EgShapes);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgUiFreeforms);

	ECS_MODULE(world, EgUiFlows);
	ecs_set_name_prefix(world, "EgUiFlows");

	ECS_COMPONENT_DEFINE(world, EgUiFlowsFlow);
	ECS_TAG_DEFINE(world, EgUiFlowsFlowUnplaced);

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgUiFlowsFlow),
	.members = {
	{.name = "direction", .type = ecs_id(EgUiDir)},
	{.name = "wrap", .type = ecs_id(EgUiDir)},
	{.name = "cursor_primary", .type = ecs_id(ecs_f32_t)},
	{.name = "cursor_wrap", .type = ecs_id(ecs_f32_t)},
	{.name = "line_wrap_extent", .type = ecs_id(ecs_f32_t)},
	{.name = "line_has_children", .type = ecs_id(ecs_bool_t)},
	}});

	ecs_query_t *children_query = ecs_query(world,
	{
		.cache_kind = EcsQueryCacheAuto,
		.group_by = EcsChildOf,
		.terms = {
			{.id = ecs_id(Position2), .inout = EcsInOut},
			{.id = ecs_id(EgShapesRectangle), .inout = EcsIn},
			{ecs_childof(EcsWildcard), .inout = EcsInOutFilter},
			{.id = ecs_id(EgUiFlowsFlowUnplaced), .oper = EcsOptional},
			{.id = EcsDisabled, .oper = EcsOptional},
			{.id = EcsDisabled, .trav = EcsChildOf, .src.id = EcsUp, .oper = EcsNot},
			{.id = ecs_id(EgUiFreeformsAnchor), .oper = EcsNot},
		},
	});
	if (children_query == NULL) {
		ecs_err("Failed to create grouped flow child query");
		return;
	}

	ecs_system_init(world,
	&(ecs_system_desc_t){
		.entity      = ecs_entity(world, {.name = "EgUiFlowsFlow_Update"}),
		.phase       = EcsOnUpdate,
		.callback    = EgUiFlowsFlow_Update,
		.ctx         = children_query,
		.query.terms = {
			{.id = ecs_id(EgUiFlowsFlow), .src.id = EcsSelf, .inout = EcsInOut},
			{.id = ecs_id(EgShapesRectangle), .src.id = EcsSelf, .inout = EcsIn},
		}});
}
