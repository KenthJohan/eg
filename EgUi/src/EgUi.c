#include "EgUi.h"

#include <EgShapes.h>
#include <EgPhysics.h>
#include <EgButtons.h>
#include <EgIntersects.h>
#include <ecsx.h>
#include <math.h>

ECS_COMPONENT_DECLARE(EgUiFlow);
ECS_COMPONENT_DECLARE(EgUiDirection);
ECS_COMPONENT_DECLARE(EgUiResizable);
ECS_COMPONENT_DECLARE(EgUiAnchorKind);
ECS_COMPONENT_DECLARE(EgUiAnchor);
ECS_COMPONENT_DECLARE(EgUiParentClearance);
ECS_TAG_DECLARE(EgUiFlowUnplaced);
ECS_TAG_DECLARE(EgUiContainer);

#define EG_UI_LAYOUT_EPSILON 1e-4f

static EgUiParentClearance EgUiParentClearance_Compute(EgShapesRectangle const *parent,
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

static bool EgUiParentClearance_Fits(EgUiParentClearance clearance)
{
	return clearance.left <= EG_UI_LAYOUT_EPSILON && clearance.right <= EG_UI_LAYOUT_EPSILON &&
		clearance.bottom <= EG_UI_LAYOUT_EPSILON && clearance.top <= EG_UI_LAYOUT_EPSILON;
}

static bool EgUiFlow_PrimaryOverflows(EgUiParentClearance clearance, int axis, float sign)
{
	if (axis == 0) {
		return sign > 0.0f ? clearance.right > EG_UI_LAYOUT_EPSILON
					   : clearance.left > EG_UI_LAYOUT_EPSILON;
	}
	return sign > 0.0f ? clearance.top > EG_UI_LAYOUT_EPSILON
				   : clearance.bottom > EG_UI_LAYOUT_EPSILON;
}

static Position2 EgUiFlow_GetPosition(int primary_axis, float primary_sign, int wrap_axis,
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

// Unit direction of an anchor kind, +y is up.
static void EgUiAnchorKind_Dir(EgUiAnchorKind k, float *x, float *y)
{
	static const float dir[][2] = {
	{0, 0}, {-1, 1}, {0, 1}, {1, 1}, {-1, 0}, {1, 0}, {-1, -1}, {0, -1}, {1, -1}};
	if ((int)k < 0 || (int)k > EgUiAnchorKindBottomRight) {
		k = EgUiAnchorKindMiddle;
	}
	*x = dir[k][0];
	*y = dir[k][1];
}

static void EgUiAnchor_Update(ecs_iter_t *it)
{
	EgUiAnchor              *a      = ecs_field_self(it, EgUiAnchor, 0);
	EgShapesRectangle const *r      = ecs_field_self(it, EgShapesRectangle, 1);
	Scale2 const            *scl    = ecs_field_self(it, Scale2, 2);
	Position2               *pos    = ecs_field_self(it, Position2, 3);
	EgShapesRectangle const *parent = ecs_field_is_set(it, 4) ? ecs_field_shared(it, EgShapesRectangle, 4) : NULL;

	float pw = parent ? parent->w * 0.5f : 0.0f;
	float ph = parent ? parent->h * 0.5f : 0.0f;
	for (int32_t i = 0; i < it->count; ++i) {
		float ax, ay, vx, vy;
		EgUiAnchorKind_Dir(a[i].parent, &ax, &ay);
		EgUiAnchorKind_Dir(a[i].pivot, &vx, &vy);
		pos[i].x = ax * pw + a[i].x - vx * r[i].w * 0.5f * scl[i].x;
		pos[i].y = ay * ph + a[i].y - vy * r[i].h * 0.5f * scl[i].y;
	}
}

static void EgUiParentClearance_Update(ecs_iter_t *it)
{
	EgUiParentClearance  *clearance = ecs_field_self(it, EgUiParentClearance, 0);
	EgShapesRectangle const *child = ecs_field_self(it, EgShapesRectangle, 1);
	Position2 const         *pos = ecs_field_self(it, Position2, 2);
	EgShapesRectangle const *parent = ecs_field_shared(it, EgShapesRectangle, 3);
	for (int32_t i = 0; i < it->count; ++i) {
		clearance[i] = EgUiParentClearance_Compute(parent, pos[i], &child[i]);
	}
}





static bool EgUiEntity_IsDisabledInHierarchy(ecs_world_t *world, ecs_entity_t entity)
{
	for (ecs_entity_t current = entity; current; current = ecs_get_parent(world, current)) {
		if (ecs_has_id(world, current, EcsDisabled)) {
			return true;
		}
	}
	return false;
}

static void EgUiResizable_ClearDisabled(ecs_iter_t *it)
{
	EgUiResizable *z = ecs_field_self(it, EgUiResizable, 0);
	for (int32_t i = 0; i < it->count; ++i) {
		if (!EgUiEntity_IsDisabledInHierarchy(it->world, it->entities[i])) {
			continue;
		}
		z[i].edge = 0;
		z[i].dragging = false;
		z[i].was_held = true;
		z[i].req_dw = 0.0f;
		z[i].req_dh = 0.0f;
		ecs_modified_id(it->world, it->entities[i], ecs_id(EgUiResizable));
	}
}

static void EgUiResizable_Request(ecs_iter_t *it)
{
	Position2 const         *mouse  = ecs_field_shared(it, Position2, 0);
	EgShapesRectangle const *r      = ecs_field_self(it, EgShapesRectangle, 1);
	EgUiResizable           *z      = ecs_field_self(it, EgUiResizable, 3);
	EgButtonsState          *s      = ecs_field_shared(it, EgButtonsState, 4);
	EgUiAnchor const        *anc    = ecs_field_is_set(it, 5) ? ecs_field_self(it, EgUiAnchor, 5) : NULL;
	EgIntersectsRectangleBorder *border = ecs_field_self(it, EgIntersectsRectangleBorder, 7);

	for (int32_t i = 0; i < it->count; ++i) {
		// The border system reads the margin on the next frame
		border[i].margin = z[i].grab;
		float lx = border[i].local_x;
		float ly = border[i].local_y;

		float hw   = r[i].w * 0.5f;
		float hh   = r[i].h * 0.5f;
		bool  held = !!(EgButtonsState_get(s, z[i].key) & EG_BUTTONS_STATE_HELD);

		uint8_t prev_edge = z[i].edge;
		bool    prev_drag = z[i].dragging;

		if (!z[i].dragging) {
			uint8_t e = border[i].edges;			// The side pinned by the anchor pivot is not draggable, otherwise the inset would change
			if (anc) {
				float vx, vy;
				EgUiAnchorKind_Dir(anc[i].pivot, &vx, &vy);
				if (vx > 0.0f)
					e &= (uint8_t)~EG_UI_EDGE_RIGHT;
				if (vx < 0.0f)
					e &= (uint8_t)~EG_UI_EDGE_LEFT;
				if (vy > 0.0f)
					e &= (uint8_t)~EG_UI_EDGE_TOP;
				if (vy < 0.0f)
					e &= (uint8_t)~EG_UI_EDGE_BOTTOM;
			}
			z[i].edge = e;
			// Only a fresh press starts a drag
			if (held && !z[i].was_held && e) {
				z[i].dragging = true;
				z[i].offset_x = (e & EG_UI_EDGE_RIGHT) ? lx - hw : (e & EG_UI_EDGE_LEFT) ? lx + hw
				                                                                         : 0.0f;
				z[i].offset_y = (e & EG_UI_EDGE_TOP) ? ly - hh : (e & EG_UI_EDGE_BOTTOM) ? ly + hh
				                                                                         : 0.0f;
			}
		}
		z[i].was_held = held;
		z[i].req_dw   = 0.0f;
		z[i].req_dh   = 0.0f;

		if (z[i].dragging && !held) {
			z[i].dragging = false;
			z[i].edge     = 0;
		}

		if (z[i].dragging) {
			uint8_t e  = z[i].edge;
			float   mx = lx - z[i].offset_x;
			float   my = ly - z[i].offset_y;
			float   dw = 0.0f;
			float   dh = 0.0f;
			if (e & EG_UI_EDGE_RIGHT)
				dw = mx - hw;
			if (e & EG_UI_EDGE_LEFT)
				dw = -(mx + hw);
			if (e & EG_UI_EDGE_TOP)
				dh = my - hh;
			if (e & EG_UI_EDGE_BOTTOM)
				dh = -(my + hh);

			// Middle pivot keeps the center, so both sides move and the edge follows the mouse
			if (anc) {
				float vx, vy;
				EgUiAnchorKind_Dir(anc[i].pivot, &vx, &vy);
				if (vx == 0.0f)
					dw *= 2.0f;
				if (vy == 0.0f)
					dh *= 2.0f;
			}

			dw = fmaxf(dw, z[i].min_w - r[i].w);
			dh = fmaxf(dh, z[i].min_h - r[i].h);

			z[i].req_dw = dw;
			z[i].req_dh = dh;
		}

		if (z[i].edge != prev_edge || z[i].dragging != prev_drag) {
			ecs_modified_id(it->world, it->entities[i], ecs_id(EgUiResizable));
		}
	}
}

// Limits the requested growth to the free space left inside the parent.
static void EgUiResizable_ClampToParent(EgShapesRectangle const *parent, Position2 pos,
	EgShapesRectangle const *r, EgUiAnchor const *anc, uint8_t e, float *dw, float *dh)
{
	EgUiParentClearance clearance = EgUiParentClearance_Compute(parent, pos, r);
	float vx = 1.0f;
	float vy = 1.0f;
	if (anc) {
		EgUiAnchorKind_Dir(anc->pivot, &vx, &vy);
	}
	if (*dw > 0.0f) {
		float available = 0.0f;
		if (anc && vx == 0.0f) {
			available = 2.0f * fminf(fmaxf(0.0f, -clearance.left), fmaxf(0.0f, -clearance.right));
		} else if (e & EG_UI_EDGE_RIGHT) {
			available = fmaxf(0.0f, -clearance.right);
		} else if (e & EG_UI_EDGE_LEFT) {
			available = fmaxf(0.0f, -clearance.left);
		}
		*dw = fminf(*dw, available);
	}
	if (*dh > 0.0f) {
		float available = 0.0f;
		if (anc && vy == 0.0f) {
			available = 2.0f * fminf(fmaxf(0.0f, -clearance.bottom), fmaxf(0.0f, -clearance.top));
		} else if (e & EG_UI_EDGE_TOP) {
			available = fmaxf(0.0f, -clearance.top);
		} else if (e & EG_UI_EDGE_BOTTOM) {
			available = fmaxf(0.0f, -clearance.bottom);
		}
		*dh = fminf(*dh, available);
	}
}

static void EgUiResizable_ApplyFlow(ecs_iter_t *it)
{
	EgUiResizable           *z      = ecs_field_self(it, EgUiResizable, 0);
	EgShapesRectangle       *r      = ecs_field_self(it, EgShapesRectangle, 1);
	Position2 const         *pos    = ecs_field_self(it, Position2, 2);
	EgShapesRectangle const *parent = ecs_field_shared(it, EgShapesRectangle, 3);
	EgUiAnchor const        *anc    = ecs_field_is_set(it, 5) ? ecs_field_self(it, EgUiAnchor, 5) : NULL;

	for (int32_t i = 0; i < it->count; ++i) {
		float dw = z[i].req_dw;
		float dh = z[i].req_dh;
		z[i].req_dw = 0.0f;
		z[i].req_dh = 0.0f;
		if (dw == 0.0f && dh == 0.0f) {
			continue;
		}
		EgUiResizable_ClampToParent(parent, pos[i], &r[i], anc ? &anc[i] : NULL, z[i].edge, &dw, &dh);
		r[i].w += dw;
		r[i].h += dh;
	}
}

static void EgUiResizable_ApplyContainer(ecs_iter_t *it)
{
	EgUiResizable           *z      = ecs_field_self(it, EgUiResizable, 0);
	EgShapesRectangle       *r      = ecs_field_self(it, EgShapesRectangle, 1);
	Position2               *pos    = ecs_field_self(it, Position2, 2);
	Rotation2 const         *rot    = ecs_field_self(it, Rotation2, 3);
	Scale2 const            *scl    = ecs_field_self(it, Scale2, 4);
	EgShapesRectangle const *parent = ecs_field_is_set(it, 5) ? ecs_field_shared(it, EgShapesRectangle, 5) : NULL;
	EgUiAnchor const        *anc    = ecs_field_is_set(it, 7) ? ecs_field_self(it, EgUiAnchor, 7) : NULL;

	for (int32_t i = 0; i < it->count; ++i) {
		float   dw = z[i].req_dw;
		float   dh = z[i].req_dh;
		uint8_t e  = z[i].edge;
		z[i].req_dw = 0.0f;
		z[i].req_dh = 0.0f;
		if (dw == 0.0f && dh == 0.0f) {
			continue;
		}
		if (parent) {
			EgUiResizable_ClampToParent(parent, pos[i], &r[i], anc ? &anc[i] : NULL, e, &dw, &dh);
		}

		// Opposite side stays fixed: size changes by d, center by d/2
		float sx = (e & EG_UI_EDGE_RIGHT) ? 0.5f : (e & EG_UI_EDGE_LEFT) ? -0.5f : 0.0f;
		float sy = (e & EG_UI_EDGE_TOP) ? 0.5f : (e & EG_UI_EDGE_BOTTOM) ? -0.5f : 0.0f;
		float px = sx * dw * scl[i].x;
		float py = sy * dh * scl[i].y;
		float c  = cosf(rot[i].radians);
		float sn = sinf(rot[i].radians);

		r[i].w += dw;
		r[i].h += dh;
		// An anchor rebuilds the position, so only the size changes
		if (!anc) {
			pos[i].x += px * c - py * sn;
			pos[i].y += px * sn + py * c;
		}
	}
}

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
	EgUiFlow          *flow   = ecs_field_self(it, EgUiFlow, 0);
	EgShapesRectangle *bounds = ecs_field_self(it, EgShapesRectangle, 1);
	for (int i = 0; i < it->count; ++i) {
		int   primary_axis = 0;
		float primary_sign = 0.0f;
		int   wrap_axis    = 0;
		float wrap_sign    = 0.0f;
		if (!EgUiFlow_GetAxis(flow[i].direction, &primary_axis, &primary_sign)) {
			flow[i].cursor_primary    = 0.0f;
			flow[i].cursor_wrap       = 0.0f;
			flow[i].line_wrap_extent  = 0.0f;
			flow[i].line_has_children = false;
			continue;
		}
		bool  can_wrap            = EgUiFlow_GetAxis(flow[i].wrap, &wrap_axis, &wrap_sign) && wrap_axis != primary_axis;
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

static void EgUiFlow_Update(ecs_iter_t *it)
{
	EgUiFlow          *flow            = ecs_field_shared(it, EgUiFlow, 0);
	EgShapesRectangle *parent_rect     = ecs_field_shared(it, EgShapesRectangle, 1);
	Position2         *child_positions = ecs_field_self(it, Position2, 2);
	EgShapesRectangle *child_rects     = ecs_field_self(it, EgShapesRectangle, 3);

	int   primary_axis;
	float primary_sign;
	int   wrap_axis = 0;
	float wrap_sign = 0.0f;
	if (!EgUiFlow_GetAxis(flow->direction, &primary_axis, &primary_sign)) {
		for (int i = 0; i < it->count; ++i) {
			if (ecs_has_id(it->world, it->entities[i], ecs_id(EgUiFlowUnplaced))) {
				ecs_remove_id(it->world, it->entities[i], ecs_id(EgUiFlowUnplaced));
				ecs_enable(it->world, it->entities[i], true);
			}
		}
		return;
	}
	bool  can_wrap      = EgUiFlow_GetAxis(flow->wrap, &wrap_axis, &wrap_sign) && wrap_axis != primary_axis;
	float primary_limit = (primary_axis == 0 ? parent_rect->w : parent_rect->h) * 0.5f;

	for (int i = 0; i < it->count; ++i) {
		bool flow_unplaced = ecs_has_id(it->world, it->entities[i], ecs_id(EgUiFlowUnplaced));
		if (ecs_has_id(it->world, it->entities[i], EcsDisabled) && !flow_unplaced) {
			continue;
		}

		float child_size[2]   = {child_rects[i].w, child_rects[i].h};
		float primary_size    = child_size[primary_axis];
		float candidate_primary = flow->cursor_primary;
		float candidate_wrap    = flow->cursor_wrap;
		float candidate_extent  = flow->line_wrap_extent;
		bool  candidate_has_children = flow->line_has_children;
		Position2 candidate_pos = EgUiFlow_GetPosition(primary_axis, primary_sign, wrap_axis,
			wrap_sign, can_wrap, candidate_primary, candidate_wrap, &child_rects[i]);
		EgUiParentClearance clearance = EgUiParentClearance_Compute(parent_rect, candidate_pos, &child_rects[i]);
		if (can_wrap && candidate_has_children && EgUiFlow_PrimaryOverflows(clearance, primary_axis, primary_sign)) {
			candidate_wrap += wrap_sign * candidate_extent;
			candidate_primary = primary_sign > 0.0f ? -primary_limit : primary_limit;
			candidate_extent = 0.0f;
			candidate_has_children = false;
			candidate_pos = EgUiFlow_GetPosition(primary_axis, primary_sign, wrap_axis,
				wrap_sign, can_wrap, candidate_primary, candidate_wrap, &child_rects[i]);
			clearance = EgUiParentClearance_Compute(parent_rect, candidate_pos, &child_rects[i]);
		}

		if (!EgUiParentClearance_Fits(clearance)) {
			if (!flow_unplaced) {
				ecs_add_id(it->world, it->entities[i], ecs_id(EgUiFlowUnplaced));
				ecs_enable(it->world, it->entities[i], false);
			}
			continue;
		}

		if (flow_unplaced) {
			ecs_remove_id(it->world, it->entities[i], ecs_id(EgUiFlowUnplaced));
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

void EgUiImport(ecs_world_t *world)
{
	ECS_IMPORT(world, EgPhysics);
	ECS_IMPORT(world, EgShapes);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgButtons);
	ECS_IMPORT(world, EgIntersects);
	ECS_IMPORT(world, EgUiButtons);

	ECS_MODULE(world, EgUi);
	ecs_set_name_prefix(world, "EgUi");

	ECS_COMPONENT_DEFINE(world, EgUiFlow);
	ECS_COMPONENT_DEFINE(world, EgUiDirection);
	ECS_COMPONENT_DEFINE(world, EgUiResizable);
	ecs_add_pair(world, ecs_id(EgUiResizable), EcsWith, ecs_id(EgIntersectsRectangleBorder));

	ECS_COMPONENT_DEFINE(world, EgUiAnchorKind);
	ECS_COMPONENT_DEFINE(world, EgUiAnchor);
	ECS_COMPONENT_DEFINE(world, EgUiParentClearance);
	ECS_TAG_DEFINE(world, EgUiFlowUnplaced);
	ECS_TAG_DEFINE(world, EgUiContainer);



	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgUiResizable),
	.members = {
	{.name = "key", .type = ecs_id(ecs_u32_t)},
	{.name = "grab", .type = ecs_id(ecs_f32_t)},
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

	ecs_enum_init(world,
	&(ecs_enum_desc_t){
	.entity    = ecs_id(EgUiAnchorKind),
	.constants = {
	{.name = "Middle", .value = EgUiAnchorKindMiddle},
	{.name = "TopLeft", .value = EgUiAnchorKindTopLeft},
	{.name = "Top", .value = EgUiAnchorKindTop},
	{.name = "TopRight", .value = EgUiAnchorKindTopRight},
	{.name = "Left", .value = EgUiAnchorKindLeft},
	{.name = "Right", .value = EgUiAnchorKindRight},
	{.name = "BottomLeft", .value = EgUiAnchorKindBottomLeft},
	{.name = "Bottom", .value = EgUiAnchorKindBottom},
	{.name = "BottomRight", .value = EgUiAnchorKindBottomRight},
	}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgUiAnchor),
	.members = {
	{.name = "parent", .type = ecs_id(EgUiAnchorKind)},
	{.name = "pivot", .type = ecs_id(EgUiAnchorKind)},
	{.name = "x", .type = ecs_id(ecs_f32_t)},
	{.name = "y", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgUiParentClearance),
	.members = {
	{.name = "left", .type = ecs_id(ecs_f32_t)},
	{.name = "right", .type = ecs_id(ecs_f32_t)},
	{.name = "bottom", .type = ecs_id(ecs_f32_t)},
	{.name = "top", .type = ecs_id(ecs_f32_t)},
	}});

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
	{.id = ecs_id(EgUiFlowUnplaced), .oper = EcsOptional},
	{.id = EcsDisabled, .oper = EcsOptional},
	{.id = EcsDisabled, .trav = EcsChildOf, .src.id = EcsUp, .oper = EcsNot},
	}});



	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiResizable_Request"}),
	.phase       = EcsPreStore,
	.callback    = EgUiResizable_Request,
	.query.terms = {
	{.id = ecs_id(Position2), .trav = ecs_id(EgPhysicsOverlapChecking), .src.id = EcsUp, .inout = EcsIn},
	{.id = ecs_id(EgShapesRectangle), .inout = EcsIn},
	{.id = ecs_id(WorldTransform3), .inout = EcsIn},
	{.id = ecs_id(EgUiResizable), .inout = EcsInOut},
	{.id = ecs_id(EgButtonsState), .src.id = ecs_id(EgButtonsState), .inout = EcsIn},
	{.id = ecs_id(EgUiAnchor), .src.id = EcsSelf, .inout = EcsIn, .oper = EcsOptional},
	{.id = EcsDisabled, .trav = EcsChildOf, .src.id = EcsUp, .oper = EcsNot},
	{.id = ecs_id(EgIntersectsRectangleBorder), .src.id = EcsSelf, .inout = EcsInOut},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiResizable_ApplyFlow"}),
	.phase       = EcsPreStore,
	.callback    = EgUiResizable_ApplyFlow,
	.query.terms = {
	{.id = ecs_id(EgUiResizable), .inout = EcsInOut},
	{.id = ecs_id(EgShapesRectangle), .inout = EcsInOut},
	{.id = ecs_id(Position2), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(EgShapesRectangle), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn},
	{.id = ecs_id(EgUiFlow), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn},
	{.id = ecs_id(EgUiAnchor), .src.id = EcsSelf, .inout = EcsIn, .oper = EcsOptional},
	{.id = EcsDisabled, .trav = EcsChildOf, .src.id = EcsUp, .oper = EcsNot},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiResizable_ApplyContainer"}),
	.phase       = EcsPreStore,
	.callback    = EgUiResizable_ApplyContainer,
	.query.terms = {
	{.id = ecs_id(EgUiResizable), .inout = EcsInOut},
	{.id = ecs_id(EgShapesRectangle), .inout = EcsInOut},
	{.id = ecs_id(Position2), .src.id = EcsSelf, .inout = EcsInOut},
	{.id = ecs_id(Rotation2), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(Scale2), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(EgShapesRectangle), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn, .oper = EcsOptional},
	{.id = ecs_id(EgUiContainer), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsInOutNone},
	{.id = ecs_id(EgUiAnchor), .src.id = EcsSelf, .inout = EcsIn, .oper = EcsOptional},
	{.id = EcsDisabled, .trav = EcsChildOf, .src.id = EcsUp, .oper = EcsNot},
	}});

	// Declared after the apply systems so a drag is applied before Position2 is rebuilt.
	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiAnchor_Update"}),
	.phase       = EcsPreStore,
	.callback    = EgUiAnchor_Update,
	.query.terms = {
	{.id = ecs_id(EgUiAnchor), .inout = EcsIn},
	{.id = ecs_id(EgShapesRectangle), .inout = EcsIn},
	{.id = ecs_id(Scale2), .inout = EcsIn},
	{.id = ecs_id(Position2), .src.id = EcsSelf, .inout = EcsOut},
	{.id = ecs_id(EgShapesRectangle), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn, .oper = EcsOptional},
	{.id = ecs_id(EgUiFlow), .trav = EcsChildOf, .src.id = EcsUp, .oper = EcsNot},
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



	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiResizable_ClearDisabled"}),
	.phase       = EcsPreStore,
	.callback    = EgUiResizable_ClearDisabled,
	.query.terms = {
	{.id = ecs_id(EgUiResizable), .inout = EcsInOut},
	{.id = EcsDisabled, .oper = EcsOptional},
	}});
}
