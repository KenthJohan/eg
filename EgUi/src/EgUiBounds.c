#include "EgUi.h"

#include <EgButtons.h>
#include <EgIntersects.h>
#include <ecsx.h>
#include <math.h>

#include "EgUi/EgUiBounds.h"
#include "EgUi/EgUiFreeforms.h"

ECS_COMPONENT_DECLARE(EgUiBoundsResizable);
ECS_COMPONENT_DECLARE(EgUiBoundsEdgeRequest);

static void EgUiBoundsAt_Dir(EgUiAt kind, float *x, float *y)
{
	static const float dir[][2] = {{0, 0}, {-1, 1}, {0, 1}, {1, 1}, {-1, 0}, {1, 0}, {-1, -1}, {0, -1}, {1, -1}};
	if ((int)kind < 0 || (int)kind > EgUiAtBottomRight) {
		kind = EgUiAtMiddle;
	}
	*x = dir[kind][0];
	*y = dir[kind][1];
}

static EgUiParentClearance EgUiBounds_ParentClearance(EgShapesRectangle const *parent, Position2 center, EgShapesRectangle const *child)
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

// Requests are one-shot absolute edge targets: producers write them, the Apply systems read them, this clears them.
static void EgUiBoundsEdgeRequest_Reset(ecs_iter_t *it)
{
	EgUiBoundsEdgeRequest *req = ecs_field_self(it, EgUiBoundsEdgeRequest, 0);
	for (int32_t i = 0; i < it->count; ++i) {
		req[i] = (EgUiBoundsEdgeRequest){0};
	}
}

static void EgUiBounds_ResolveRequest(EgUiBoundsResizable const *z, EgShapesRectangle const *rect, EgUiFreeformsAnchor const *anchor, EgUiBoundsEdgeRequest const *req, uint8_t *edge, float *dw, float *dh)
{
	// Outward growth needed per requested edge to reach its target
	float hw      = rect->w * 0.5f;
	float hh      = rect->h * 0.5f;
	float l       = (req->edges & EG_UI_EDGE_LEFT) ? -hw - req->left : 0.0f;
	float r       = (req->edges & EG_UI_EDGE_RIGHT) ? req->right - hw : 0.0f;
	float b       = (req->edges & EG_UI_EDGE_BOTTOM) ? -hh - req->bottom : 0.0f;
	float t       = (req->edges & EG_UI_EDGE_TOP) ? req->top - hh : 0.0f;
	float scale_w = 1.0f;
	float scale_h = 1.0f;
	if (anchor) {
		float pivot_x, pivot_y;
		EgUiBoundsAt_Dir(anchor->pivot, &pivot_x, &pivot_y);
		if (pivot_x > 0.0f)
			r = 0.0f;
		if (pivot_x < 0.0f)
			l = 0.0f;
		if (pivot_y > 0.0f)
			t = 0.0f;
		if (pivot_y < 0.0f)
			b = 0.0f;
		if (pivot_x == 0.0f)
			scale_w = 2.0f;
		if (pivot_y == 0.0f)
			scale_h = 2.0f;
	}
	*edge = (uint8_t)((l != 0.0f ? EG_UI_EDGE_LEFT : 0) | (r != 0.0f ? EG_UI_EDGE_RIGHT : 0) |
	                  (b != 0.0f ? EG_UI_EDGE_BOTTOM : 0) | (t != 0.0f ? EG_UI_EDGE_TOP : 0));
	*dw   = 0.0f;
	*dh   = 0.0f;
	if (*edge & (EG_UI_EDGE_LEFT | EG_UI_EDGE_RIGHT))
		*dw = fmaxf((l + r) * scale_w, z->min_w - rect->w);
	if (*edge & (EG_UI_EDGE_BOTTOM | EG_UI_EDGE_TOP))
		*dh = fmaxf((b + t) * scale_h, z->min_h - rect->h);
}

static void EgUiBounds_ClampToParent(EgShapesRectangle const *parent, Position2 pos, EgShapesRectangle const *rect, EgUiFreeformsAnchor const *anchor, uint8_t edge, float *dw, float *dh)
{
	EgUiParentClearance clearance = EgUiBounds_ParentClearance(parent, pos, rect);
	float               pivot_x   = 1.0f;
	float               pivot_y   = 1.0f;
	if (anchor)
		EgUiBoundsAt_Dir(anchor->pivot, &pivot_x, &pivot_y);
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
	EgUiBoundsResizable const   *z      = ecs_field_self(it, EgUiBoundsResizable, 0);
	EgShapesRectangle           *rect   = ecs_field_self(it, EgShapesRectangle, 1);
	Position2 const             *pos    = ecs_field_self(it, Position2, 2);
	EgShapesRectangle const     *parent = ecs_field_shared(it, EgShapesRectangle, 3);
	EgUiFreeformsAnchor const   *anchor = ecs_field_is_set(it, 5) ? ecs_field_self(it, EgUiFreeformsAnchor, 5) : NULL;
	EgUiBoundsEdgeRequest const *req    = ecs_field_self(it, EgUiBoundsEdgeRequest, 6);
	EgUiFlowsFlow const         *flow   = ecs_field(it, EgUiFlowsFlow, 4);
	for (int32_t i = 0; i < it->count; ++i) {
		uint8_t edge;
		float   dw, dh;
		// Edges facing the flow origin touch the previous sibling; a grab there is shared with it
		// and would grow this child while the sibling is being shrunk.
		EgUiBoundsEdgeRequest flow_req = req[i];
		EgUiDir               dirs[2]  = {flow->direction, flow->wrap};
		for (int d = 0; d < 2; ++d) {
			switch (dirs[d]) {
			case EgUiDirRight:
				flow_req.edges &= ~EG_UI_EDGE_LEFT;
				break;
			case EgUiDirLeft:
				flow_req.edges &= ~EG_UI_EDGE_RIGHT;
				break;
			case EgUiDirUp:
				flow_req.edges &= ~EG_UI_EDGE_BOTTOM;
				break;
			case EgUiDirDown:
				flow_req.edges &= ~EG_UI_EDGE_TOP;
				break;
			default:
				break;
			}
		}
		EgUiBounds_ResolveRequest(&z[i], &rect[i], anchor ? &anchor[i] : NULL, &flow_req, &edge, &dw, &dh);
		if (dw == 0.0f && dh == 0.0f)
			continue;
		EgUiBounds_ClampToParent(parent, pos[i], &rect[i], anchor ? &anchor[i] : NULL, edge, &dw, &dh);
		rect[i].w += dw;
		rect[i].h += dh;
		ecs_modified_id(it->world, it->entities[i], ecs_id(EgShapesRectangle));
	}
}

static void EgUiBoundsResizable_ApplyFreeforms(ecs_iter_t *it)
{
	EgUiBoundsResizable const   *z      = ecs_field_self(it, EgUiBoundsResizable, 0);
	EgShapesRectangle           *rect   = ecs_field_self(it, EgShapesRectangle, 1);
	Position2                   *pos    = ecs_field_self(it, Position2, 2);
	Rotation2 const             *rot    = ecs_field_self(it, Rotation2, 3);
	Scale2 const                *scale  = ecs_field_self(it, Scale2, 4);
	EgShapesRectangle const     *parent = ecs_field_is_set(it, 5) ? ecs_field_shared(it, EgShapesRectangle, 5) : NULL;
	EgUiFreeformsAnchor const   *anchor = ecs_field_is_set(it, 7) ? ecs_field_self(it, EgUiFreeformsAnchor, 6) : NULL;
	EgUiBoundsEdgeRequest const *req    = ecs_field_self(it, EgUiBoundsEdgeRequest, 7);
	for (int32_t i = 0; i < it->count; ++i) {
		uint8_t edge;
		float   dw, dh;
		EgUiBounds_ResolveRequest(&z[i], &rect[i], anchor ? &anchor[i] : NULL, &req[i], &edge, &dw, &dh);
		if (dw == 0.0f && dh == 0.0f)
			continue;
		if (parent)
			EgUiBounds_ClampToParent(parent, pos[i], &rect[i], anchor ? &anchor[i] : NULL, edge, &dw, &dh);

		float sx = (edge & EG_UI_EDGE_RIGHT) ? 0.5f : (edge & EG_UI_EDGE_LEFT) ? -0.5f :
		                                                                         0.0f;
		float sy = (edge & EG_UI_EDGE_TOP) ? 0.5f : (edge & EG_UI_EDGE_BOTTOM) ? -0.5f :
		                                                                         0.0f;
		float px = sx * dw * scale[i].x;
		float py = sy * dh * scale[i].y;
		float c  = cosf(rot[i].radians);
		float sn = sinf(rot[i].radians);
		rect[i].w += dw;
		rect[i].h += dh;
		ecs_modified_id(it->world, it->entities[i], ecs_id(EgShapesRectangle)); // Lets script reading the size re-evaluate
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
	ECS_IMPORT(world, EgButtons);
	ECS_IMPORT(world, EgIntersects);
	ECS_IMPORT(world, EgUiLocalMouse);
	ECS_IMPORT(world, EgUiButtons);
	ECS_IMPORT(world, EgUiFlows);
	ECS_IMPORT(world, EgUiFreeforms);

	ECS_MODULE(world, EgUiBounds);
	ecs_set_name_prefix(world, "EgUiBounds");
	ECS_COMPONENT_DEFINE(world, EgUiBoundsResizable);
	ECS_COMPONENT_DEFINE(world, EgUiBoundsEdgeRequest);
	ecs_add_pair(world, ecs_id(EgUiBoundsResizable), EcsWith, ecs_id(EgUiBoundsEdgeRequest));

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgUiBoundsEdgeRequest),
	.members = {
	{.name = "edges", .type = ecs_id(ecs_u32_t)},
	{.name = "left", .type = ecs_id(ecs_f32_t)},
	{.name = "right", .type = ecs_id(ecs_f32_t)},
	{.name = "bottom", .type = ecs_id(ecs_f32_t)},
	{.name = "top", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgUiBoundsResizable),
	.members = {
	{.name = "min_w", .type = ecs_id(ecs_f32_t)},
	{.name = "min_h", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiBoundsResizable_ApplyFlow"}),
	.phase       = EcsPreStore,
	.callback    = EgUiBoundsResizable_ApplyFlow,
	.immediate   = true,
	.query.terms = {
	{.id = ecs_id(EgUiBoundsResizable), .inout = EcsInOut},
	{.id = ecs_id(EgShapesRectangle), .inout = EcsInOut},
	{.id = ecs_id(Position2), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(EgShapesRectangle), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn},
	{.id = ecs_id(EgUiFlowsFlow), .src = {.id = EcsIsVariable, .name = "parent"}, .inout = EcsIn},
	{.id = ecs_id(EgUiFreeformsAnchor), .src.id = EcsSelf, .inout = EcsIn, .oper = EcsOptional},
	{.id = ecs_id(EgUiBoundsEdgeRequest), .inout = EcsIn},
	{.id = EcsDisabled, .trav = EcsChildOf, .src.id = EcsUp, .oper = EcsNot},
	{.first.id = EcsChildOf, .second = {.id = EcsIsVariable, .name = "parent"}, .inout = EcsInOutNone},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiBoundsResizable_ApplyFreeforms"}),
	.phase       = EcsPreStore,
	.callback    = EgUiBoundsResizable_ApplyFreeforms,
	.immediate   = true,
	.query.terms = {
	{.id = ecs_id(EgUiBoundsResizable), .inout = EcsInOut},
	{.id = ecs_id(EgShapesRectangle), .inout = EcsInOut},
	{.id = ecs_id(Position2), .src.id = EcsSelf, .inout = EcsInOut},
	{.id = ecs_id(Rotation2), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(Scale2), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(EgShapesRectangle), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn, .oper = EcsOptional},
	{.id = ecs_id(EgUiFreeformsAnchor), .src.id = EcsSelf, .inout = EcsIn, .oper = EcsOptional},
	{.id = ecs_id(EgUiBoundsEdgeRequest), .inout = EcsIn},
	{.id = ecs_id(EgUiFreeformsLayout), .src = {.id = EcsIsVariable, .name = "parent"}, .inout = EcsInOutNone},
	{.id = EcsDisabled, .trav = EcsChildOf, .src.id = EcsUp, .oper = EcsNot},
	{.first.id = EcsChildOf, .second = {.id = EcsIsVariable, .name = "parent"}, .inout = EcsInOutNone},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiBoundsEdgeRequest_Reset"}),
	.phase       = EcsPreStore,
	.callback    = EgUiBoundsEdgeRequest_Reset,
	.query.terms = {
	{.id = ecs_id(EgUiBoundsEdgeRequest), .inout = EcsInOut},
	{.id = EcsDisabled, .oper = EcsOptional},
	}});
}
