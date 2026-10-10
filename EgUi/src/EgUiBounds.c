#include "EgUi.h"

#include <EgPhysics.h>
#include <EgButtons.h>
#include <EgIntersects.h>
#include <ecsx.h>
#include <math.h>

#include "EgUi/EgUiBounds.h"
#include "EgUi/EgUiFreeforms.h"

ECS_COMPONENT_DECLARE(EgUiBoundsResizable);

static void EgUiBoundsAnchorKind_Dir(EgUiFreeformsAnchorKind kind, float *x, float *y)
{
	static const float dir[][2] = {
	{0, 0}, {-1, 1}, {0, 1}, {1, 1}, {-1, 0}, {1, 0}, {-1, -1}, {0, -1}, {1, -1}};
	if ((int)kind < 0 || (int)kind > EgUiFreeformsAnchorKindBottomRight) {
		kind = EgUiFreeformsAnchorKindMiddle;
	}
	*x = dir[kind][0];
	*y = dir[kind][1];
}

static EgUiParentClearance EgUiBounds_ParentClearance(EgShapesRectangle const *parent, Position2 center,
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

static void EgUiBoundsResizable_ClearDisabled(ecs_iter_t *it)
{
	EgUiBoundsResizable *z = ecs_field_self(it, EgUiBoundsResizable, 0);
	for (int32_t i = 0; i < it->count; ++i) {
		bool disabled = false;
		for (ecs_entity_t current = it->entities[i]; current; current = ecs_get_parent(it->world, current)) {
			if (ecs_has_id(it->world, current, EcsDisabled)) {
				disabled = true;
				break;
			}
		}
		if (!disabled) continue;
		z[i].edge = 0;
		z[i].dragging = false;
		z[i].was_held = true;
		z[i].req_dw = 0.0f;
		z[i].req_dh = 0.0f;
		ecs_modified_id(it->world, it->entities[i], ecs_id(EgUiBoundsResizable));
	}
}

static void EgUiBoundsResizable_Request(ecs_iter_t *it)
{
	EgShapesRectangle const     *rect = ecs_field_self(it, EgShapesRectangle, 0);
	EgUiBoundsResizable         *z = ecs_field_self(it, EgUiBoundsResizable, 1);
	EgButtonsState              *state = ecs_field_shared(it, EgButtonsState, 2);
	EgUiFreeformsAnchor const   *anchor = ecs_field_is_set(it, 3) ? ecs_field_self(it, EgUiFreeformsAnchor, 3) : NULL;
	EgIntersectsRectangleBorder *border = ecs_field_self(it, EgIntersectsRectangleBorder, 4);

	for (int32_t i = 0; i < it->count; ++i) {
		float local_x = border[i].local_x;
		float local_y = border[i].local_y;
		float half_w = rect[i].w * 0.5f;
		float half_h = rect[i].h * 0.5f;
		bool held = !!(EgButtonsState_get(state, z[i].key) & EG_BUTTONS_STATE_HELD);
		uint8_t previous_edge = z[i].edge;
		bool previous_dragging = z[i].dragging;

		if (!z[i].dragging) {
			uint8_t edge = border[i].edges;
			if (anchor) {
				float pivot_x, pivot_y;
				EgUiBoundsAnchorKind_Dir(anchor[i].pivot, &pivot_x, &pivot_y);
				if (pivot_x > 0.0f) edge &= (uint8_t)~EG_UI_EDGE_RIGHT;
				if (pivot_x < 0.0f) edge &= (uint8_t)~EG_UI_EDGE_LEFT;
				if (pivot_y > 0.0f) edge &= (uint8_t)~EG_UI_EDGE_TOP;
				if (pivot_y < 0.0f) edge &= (uint8_t)~EG_UI_EDGE_BOTTOM;
			}
			z[i].edge = edge;
			if (held && !z[i].was_held && edge) {
				z[i].dragging = true;
				z[i].offset_x = (edge & EG_UI_EDGE_RIGHT) ? local_x - half_w :
					(edge & EG_UI_EDGE_LEFT) ? local_x + half_w : 0.0f;
				z[i].offset_y = (edge & EG_UI_EDGE_TOP) ? local_y - half_h :
					(edge & EG_UI_EDGE_BOTTOM) ? local_y + half_h : 0.0f;
			}
		}
		z[i].was_held = held;
		z[i].req_dw = 0.0f;
		z[i].req_dh = 0.0f;

		if (z[i].dragging && !held) {
			z[i].dragging = false;
			z[i].edge = 0;
		}

		if (z[i].dragging) {
			uint8_t edge = z[i].edge;
			float mouse_x = local_x - z[i].offset_x;
			float mouse_y = local_y - z[i].offset_y;
			float dw = 0.0f;
			float dh = 0.0f;
			if (edge & EG_UI_EDGE_RIGHT) dw = mouse_x - half_w;
			if (edge & EG_UI_EDGE_LEFT) dw = -(mouse_x + half_w);
			if (edge & EG_UI_EDGE_TOP) dh = mouse_y - half_h;
			if (edge & EG_UI_EDGE_BOTTOM) dh = -(mouse_y + half_h);
			if (anchor) {
				float pivot_x, pivot_y;
				EgUiBoundsAnchorKind_Dir(anchor[i].pivot, &pivot_x, &pivot_y);
				if (pivot_x == 0.0f) dw *= 2.0f;
				if (pivot_y == 0.0f) dh *= 2.0f;
			}
			dw = fmaxf(dw, z[i].min_w - rect[i].w);
			dh = fmaxf(dh, z[i].min_h - rect[i].h);
			z[i].req_dw = dw;
			z[i].req_dh = dh;
		}

		if (z[i].edge != previous_edge || z[i].dragging != previous_dragging) {
			ecs_modified_id(it->world, it->entities[i], ecs_id(EgUiBoundsResizable));
		}
	}
}

static void EgUiBounds_ClampToParent(EgShapesRectangle const *parent, Position2 pos, EgShapesRectangle const *rect,
	EgUiFreeformsAnchor const *anchor, uint8_t edge, float *dw, float *dh)
{
	EgUiParentClearance clearance = EgUiBounds_ParentClearance(parent, pos, rect);
	float pivot_x = 1.0f;
	float pivot_y = 1.0f;
	if (anchor) EgUiBoundsAnchorKind_Dir(anchor->pivot, &pivot_x, &pivot_y);
	if (*dw > 0.0f) {
		float available = 0.0f;
		if (anchor && pivot_x == 0.0f) {
			available = 2.0f * fminf(fmaxf(0.0f, -clearance.left), fmaxf(0.0f, -clearance.right));
		} else if (edge & EG_UI_EDGE_RIGHT) {
			available = fmaxf(0.0f, -clearance.right);
		} else if (edge & EG_UI_EDGE_LEFT) {
			available = fmaxf(0.0f, -clearance.left);
		}
		*dw = fminf(*dw, available);
	}
	if (*dh > 0.0f) {
		float available = 0.0f;
		if (anchor && pivot_y == 0.0f) {
			available = 2.0f * fminf(fmaxf(0.0f, -clearance.bottom), fmaxf(0.0f, -clearance.top));
		} else if (edge & EG_UI_EDGE_TOP) {
			available = fmaxf(0.0f, -clearance.top);
		} else if (edge & EG_UI_EDGE_BOTTOM) {
			available = fmaxf(0.0f, -clearance.bottom);
		}
		*dh = fminf(*dh, available);
	}
}

static void EgUiBoundsResizable_ApplyFlow(ecs_iter_t *it)
{
	EgUiBoundsResizable       *z = ecs_field_self(it, EgUiBoundsResizable, 0);
	EgShapesRectangle         *rect = ecs_field_self(it, EgShapesRectangle, 1);
	Position2 const           *pos = ecs_field_self(it, Position2, 2);
	EgShapesRectangle const   *parent = ecs_field_shared(it, EgShapesRectangle, 3);
	EgUiFreeformsAnchor const *anchor = ecs_field_is_set(it, 5) ? ecs_field_self(it, EgUiFreeformsAnchor, 5) : NULL;
	for (int32_t i = 0; i < it->count; ++i) {
		float dw = z[i].req_dw;
		float dh = z[i].req_dh;
		z[i].req_dw = 0.0f;
		z[i].req_dh = 0.0f;
		if (dw == 0.0f && dh == 0.0f) continue;
		EgUiBounds_ClampToParent(parent, pos[i], &rect[i], anchor ? &anchor[i] : NULL, z[i].edge, &dw, &dh);
		rect[i].w += dw;
		rect[i].h += dh;
	}
}

static void EgUiBoundsResizable_ApplyFreeforms(ecs_iter_t *it)
{
	EgUiBoundsResizable       *z = ecs_field_self(it, EgUiBoundsResizable, 0);
	EgShapesRectangle         *rect = ecs_field_self(it, EgShapesRectangle, 1);
	Position2                 *pos = ecs_field_self(it, Position2, 2);
	Rotation2 const           *rot = ecs_field_self(it, Rotation2, 3);
	Scale2 const              *scale = ecs_field_self(it, Scale2, 4);
	EgShapesRectangle const   *parent = ecs_field_is_set(it, 5) ? ecs_field_shared(it, EgShapesRectangle, 5) : NULL;
	EgUiFreeformsAnchor const *anchor = ecs_field_is_set(it, 7) ? ecs_field_self(it, EgUiFreeformsAnchor, 7) : NULL;
	for (int32_t i = 0; i < it->count; ++i) {
		float dw = z[i].req_dw;
		float dh = z[i].req_dh;
		uint8_t edge = z[i].edge;
		z[i].req_dw = 0.0f;
		z[i].req_dh = 0.0f;
		if (dw == 0.0f && dh == 0.0f) continue;
		if (parent) EgUiBounds_ClampToParent(parent, pos[i], &rect[i], anchor ? &anchor[i] : NULL, edge, &dw, &dh);

		float sx = (edge & EG_UI_EDGE_RIGHT) ? 0.5f : (edge & EG_UI_EDGE_LEFT) ? -0.5f : 0.0f;
		float sy = (edge & EG_UI_EDGE_TOP) ? 0.5f : (edge & EG_UI_EDGE_BOTTOM) ? -0.5f : 0.0f;
		float px = sx * dw * scale[i].x;
		float py = sy * dh * scale[i].y;
		float c = cosf(rot[i].radians);
		float sn = sinf(rot[i].radians);
		rect[i].w += dw;
		rect[i].h += dh;
		if (!anchor) {
			pos[i].x += px * c - py * sn;
			pos[i].y += px * sn + py * c;
		}
	}
}

void EgUiBoundsImport(ecs_world_t *world)
{
	ECS_IMPORT(world, EgShapes);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgPhysics);
	ECS_IMPORT(world, EgButtons);
	ECS_IMPORT(world, EgIntersects);
	ECS_IMPORT(world, EgUiFlows);
	ECS_IMPORT(world, EgUiFreeforms);

	ECS_MODULE(world, EgUiBounds);
	ecs_set_name_prefix(world, "EgUiBounds");
	ECS_COMPONENT_DEFINE(world, EgUiBoundsResizable);

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgUiBoundsResizable),
	.members = {
	{.name = "key", .type = ecs_id(ecs_u32_t)},
	{.name = "min_w", .type = ecs_id(ecs_f32_t)},
	{.name = "min_h", .type = ecs_id(ecs_f32_t)},
	{.name = "edge", .type = ecs_id(ecs_u8_t)},
	{.name = "dragging", .type = ecs_id(ecs_bool_t)},
	{.name = "was_held", .type = ecs_id(ecs_bool_t)},
	{.name = "offset_x", .type = ecs_id(ecs_f32_t)},
	{.name = "offset_y", .type = ecs_id(ecs_f32_t)},
	{.name = "req_dw", .type = ecs_id(ecs_f32_t)},
	{.name = "req_dh", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiBoundsResizable_Request"}),
	.phase       = EcsPreStore,
	.callback    = EgUiBoundsResizable_Request,
	.query.terms = {
	{.id = ecs_id(EgShapesRectangle), .inout = EcsIn},
	{.id = ecs_id(EgUiBoundsResizable), .inout = EcsInOut},
	{.id = ecs_id(EgButtonsState), .src.id = ecs_id(EgButtonsState), .inout = EcsIn},
	{.id = ecs_id(EgUiFreeformsAnchor), .src.id = EcsSelf, .inout = EcsIn, .oper = EcsOptional},
	{.id = ecs_id(EgIntersectsRectangleBorder), .src.id = EcsSelf, .inout = EcsInOut},
	{.id = EcsDisabled, .trav = EcsChildOf, .src.id = EcsUp, .oper = EcsNot},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiBoundsResizable_ApplyFlow"}),
	.phase       = EcsPreStore,
	.callback    = EgUiBoundsResizable_ApplyFlow,
	.query.terms = {
	{.id = ecs_id(EgUiBoundsResizable), .inout = EcsInOut},
	{.id = ecs_id(EgShapesRectangle), .inout = EcsInOut},
	{.id = ecs_id(Position2), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(EgShapesRectangle), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn},
	{.id = ecs_id(EgUiFlowsFlow), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn},
	{.id = ecs_id(EgUiFreeformsAnchor), .src.id = EcsSelf, .inout = EcsIn, .oper = EcsOptional},
	{.id = EcsDisabled, .trav = EcsChildOf, .src.id = EcsUp, .oper = EcsNot},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiBoundsResizable_ApplyFreeforms"}),
	.phase       = EcsPreStore,
	.callback    = EgUiBoundsResizable_ApplyFreeforms,
	.query.terms = {
	{.id = ecs_id(EgUiBoundsResizable), .inout = EcsInOut},
	{.id = ecs_id(EgShapesRectangle), .inout = EcsInOut},
	{.id = ecs_id(Position2), .src.id = EcsSelf, .inout = EcsInOut},
	{.id = ecs_id(Rotation2), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(Scale2), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(EgShapesRectangle), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn, .oper = EcsOptional},
	{.id = ecs_id(EgUiFreeformsLayout), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsInOutNone},
	{.id = ecs_id(EgUiFreeformsAnchor), .src.id = EcsSelf, .inout = EcsIn, .oper = EcsOptional},
	{.id = EcsDisabled, .trav = EcsChildOf, .src.id = EcsUp, .oper = EcsNot},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiBoundsResizable_ClearDisabled"}),
	.phase       = EcsPreStore,
	.callback    = EgUiBoundsResizable_ClearDisabled,
	.query.terms = {
	{.id = ecs_id(EgUiBoundsResizable), .inout = EcsInOut},
	{.id = EcsDisabled, .oper = EcsOptional},
	}});
}
