#include "EgUi.h"

#include <EgShapes.h>
#include <ecsx.h>

ECS_COMPONENT_DECLARE(EgUiFlow);
ECS_COMPONENT_DECLARE(EgUiDirection);

static bool EgUiFlow_GetAxis(EgUiDirection direction, int *axis, float *sign)
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

static void EgUiFlow_Reset(ecs_iter_t *it)
{
	EgUiFlow *flow = ecs_field_self(it, EgUiFlow, 0);
	EgShapesRectangle *bounds = ecs_field_self(it, EgShapesRectangle, 1);
	for (int i = 0; i < it->count; ++i) {
		int primary_axis = 0;
		float primary_sign = 0.0f;
		int wrap_axis = 0;
		float wrap_sign = 0.0f;
		if (!EgUiFlow_GetAxis(flow[i].direction, &primary_axis, &primary_sign)) {
			flow[i].cursor_primary = 0.0f;
			flow[i].cursor_wrap = 0.0f;
			flow[i].line_wrap_extent = 0.0f;
			flow[i].line_has_children = false;
			continue;
		}
		bool can_wrap = EgUiFlow_GetAxis(flow[i].wrap, &wrap_axis, &wrap_sign) && wrap_axis != primary_axis;
		float half_width = bounds[i].w * 0.5f;
		float half_height = bounds[i].h * 0.5f;
		float primary_limit = primary_axis == 0 ? half_width : half_height;
		float wrap_limit = wrap_axis == 0 ? half_width : half_height;
		flow[i].cursor_primary = primary_sign > 0.0f ? -primary_limit : primary_limit;
		flow[i].cursor_wrap = can_wrap ? (wrap_sign > 0.0f ? -wrap_limit : wrap_limit) : 0.0f;
		flow[i].line_wrap_extent = 0.0f;
		flow[i].line_has_children = false;
	}
}

static void EgUiFlow_Update(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	EgUiFlow *flow = ecs_field_shared(it, EgUiFlow, 0);
	EgShapesRectangle *parent_rect = ecs_field_shared(it, EgShapesRectangle, 1);
	Position2 *child_positions = ecs_field_self(it, Position2, 2);
	EgShapesRectangle *child_rects = ecs_field_self(it, EgShapesRectangle, 3);

	int primary_axis;
	float primary_sign;
	int wrap_axis = 0;
	float wrap_sign = 0.0f;
	if (!EgUiFlow_GetAxis(flow->direction, &primary_axis, &primary_sign)) {
		return;
	}
	bool can_wrap = EgUiFlow_GetAxis(flow->wrap, &wrap_axis, &wrap_sign) && wrap_axis != primary_axis;
	float primary_limit = (primary_axis == 0 ? parent_rect->w : parent_rect->h) * 0.5f;

	for (int i = 0; i < it->count; ++i) {
		float child_size[2] = {child_rects[i].w, child_rects[i].h};
		float primary_size = child_size[primary_axis];
		float next_primary = flow->cursor_primary + primary_sign * primary_size;
		bool exceeds_primary = primary_sign > 0.0f ? next_primary > primary_limit : next_primary < -primary_limit;
		if (can_wrap && flow->line_has_children && exceeds_primary) {
			flow->cursor_wrap += wrap_sign * flow->line_wrap_extent;
			flow->cursor_primary = primary_sign > 0.0f ? -primary_limit : primary_limit;
			flow->line_wrap_extent = 0.0f;
			flow->line_has_children = false;
		}

		child_positions[i].x = 0.0f;
		child_positions[i].y = 0.0f;
		if (primary_axis == 0) {
			child_positions[i].x = flow->cursor_primary + primary_sign * child_size[0] * 0.5f;
		} else {
			child_positions[i].y = flow->cursor_primary + primary_sign * child_size[1] * 0.5f;
		}
		if (can_wrap) {
			if (wrap_axis == 0) {
				child_positions[i].x = flow->cursor_wrap + wrap_sign * child_size[0] * 0.5f;
			} else {
				child_positions[i].y = flow->cursor_wrap + wrap_sign * child_size[1] * 0.5f;
			}
			if (child_size[wrap_axis] > flow->line_wrap_extent) {
				flow->line_wrap_extent = child_size[wrap_axis];
			}
		}
		flow->cursor_primary += primary_sign * primary_size;
		flow->line_has_children = true;
	}
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
	{.name = "cursor_primary", .type = ecs_id(ecs_f32_t)},
	{.name = "cursor_wrap", .type = ecs_id(ecs_f32_t)},
	{.name = "line_wrap_extent", .type = ecs_id(ecs_f32_t)},
	{.name = "line_has_children", .type = ecs_id(ecs_bool_t)},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiFlow_Reset"}),
	.phase       = EcsOnUpdate,
	.callback    = EgUiFlow_Reset,
	.query.terms = {
	{.id = ecs_id(EgUiFlow), .inout = EcsInOut},
	{.id = ecs_id(EgShapesRectangle), .inout = EcsIn},
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
