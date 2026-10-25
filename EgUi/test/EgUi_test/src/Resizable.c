#include <bake_test.h>
#include <EgButtons.h>
#include <EgShapes.h>
#include <EgSpatials.h>
#include <EgUi.h>

#define MOUSE_LEFT_KEY ((1u << 16) | 1u)
#define EPSILON        1e-4f

static ecs_world_t *world;
static ecs_entity_t mouse;

void Resizable_setup(void)
{
	world = ecs_init();
	ECS_IMPORT(world, EgShapes);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgUi);
	ecs_singleton_set(world, EgButtonsState, {.mouse = {[1] = EG_BUTTONS_STATE_HELD}});

	mouse = ecs_new(world);
	ecs_set(world, mouse, Position2, {40.0f, 0.0f});
}

void Resizable_teardown(void)
{
	ecs_fini(world);
	world = NULL;
}

void Resizable_test_right_edge_stops_at_parent_boundary(void)
{
	ecs_entity_t parent = ecs_new(world);
	ecs_set(world, parent, EgShapesRectangle, {.w = 100.0f, .h = 100.0f});
	ecs_add(world, parent, EgUiFreeformsLayout);
	ecs_entity_t child     = ecs_new_w_pair(world, EcsChildOf, parent);
	ecs_add_pair(world, child, EgUiMouse, mouse);
	ecs_set(world, child, EgShapesRectangle, {.w = 80.0f, .h = 40.0f});
	ecs_set(world, child, Position2, {0.0f, 0.0f});
	ecs_set(world, child, Rotation2, {0.0f});
	ecs_set(world, child, Scale2, {1.0f, 1.0f});
	ecs_set(world, child, WorldTransform3, {.matrix = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}});
	ecs_set(world, child, EgUiBoundsResizable, {.min_w = 20.0f, .min_h = 10.0f});

	ecs_progress(world, 0.0f);
	ecs_set(world, child, EgUiBoundsEdgeRequest, {.edges = EG_UI_EDGE_RIGHT, .right = 60.0f});
	ecs_progress(world, 0.0f);

	const EgShapesRectangle *rect = ecs_get(world, child, EgShapesRectangle);
	const Position2         *pos  = ecs_get(world, child, Position2);
	test_assert(fabsf(rect->w - 90.0f) < EPSILON);
	test_assert(fabsf(pos->x - 5.0f) < EPSILON);
}

void Resizable_test_request_is_cleared_after_use(void)
{
	ecs_entity_t child = ecs_new(world);
	ecs_set(world, child, EgUiBoundsResizable, {.min_w = 20.0f, .min_h = 10.0f});
	ecs_set(world, child, EgUiBoundsEdgeRequest, {.edges = EG_UI_EDGE_RIGHT | EG_UI_EDGE_TOP, .right = 5.0f, .top = 3.0f});

	ecs_progress(world, 0.0f);

	const EgUiBoundsEdgeRequest *req = ecs_get(world, child, EgUiBoundsEdgeRequest);
	test_assert(req->edges == 0 && req->left == 0.0f && req->right == 0.0f && req->bottom == 0.0f && req->top == 0.0f);
}

void Resizable_test_disabled_entity_request_is_cleared(void)
{
	ecs_entity_t child = ecs_new(world);
	ecs_set(world, child, EgUiBoundsResizable, {.min_w = 20.0f, .min_h = 10.0f});
	ecs_set(world, child, EgUiBoundsEdgeRequest, {.edges = EG_UI_EDGE_RIGHT, .right = 5.0f});
	ecs_enable(world, child, false);

	ecs_progress(world, 0.0f);

	const EgUiBoundsEdgeRequest *req = ecs_get(world, child, EgUiBoundsEdgeRequest);
	test_assert(req->edges == 0 && req->right == 0.0f);
}

static ecs_entity_t make_flow_resizable(ecs_entity_t parent, float w, float h)
{
	ecs_entity_t e = ecs_new_w_pair(world, EcsChildOf, parent);
	ecs_add_pair(world, e, EgUiMouse, mouse);
	ecs_set(world, e, EgShapesRectangle, {.w = w, .h = h});
	ecs_set(world, e, Position2, {0.0f, 0.0f});
	ecs_set(world, e, Rotation2, {0.0f});
	ecs_set(world, e, Scale2, {1.0f, 1.0f});
	ecs_set(world, e, WorldTransform3, {.matrix = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}});
	ecs_set(world, e, EgUiBoundsResizable, {.min_w = 20.0f, .min_h = 10.0f});
	return e;
}

// Stand-in for the script-driven world transform: parent sits at the origin
static void sync_world_transform(ecs_entity_t e)
{
	const Position2 *p = ecs_get(world, e, Position2);
	WorldTransform3 wt = {.matrix = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {p->x, p->y, 1.0f}}};
	ecs_set_id(world, e, ecs_id(WorldTransform3), sizeof(wt), &wt);
}

void Resizable_test_flow_shrink_to_min_does_not_expand_next_sibling(void)
{
	ecs_entity_t parent = ecs_new(world);
	ecs_set(world, parent, EgShapesRectangle, {.w = 200.0f, .h = 100.0f});
	ecs_set(world, parent, EgUiFlowsFlow, {.direction = EgUiDirRight, .wrap = EgUiDirDown});
	ecs_set(world, parent, Position2, {0.0f, 0.0f});
	ecs_set(world, parent, Rotation2, {0.0f});
	ecs_set(world, parent, Scale2, {1.0f, 1.0f});
	ecs_set(world, parent, WorldTransform3, {.matrix = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}});
	ecs_entity_t a = make_flow_resizable(parent, 40.0f, 40.0f);
	ecs_entity_t b = make_flow_resizable(parent, 60.0f, 30.0f);

	// A spans x -100..-60 in the flow; shrink its right edge well past its minimum width.
	ecs_progress(world, 0.0f);
	sync_world_transform(a);
	sync_world_transform(b);
	for (int i = 0; i < 12; ++i) {
		ecs_set(world, a, EgUiBoundsEdgeRequest, {.edges = EG_UI_EDGE_RIGHT, .right = -100.0f});
		sync_world_transform(a);
		sync_world_transform(b);
		ecs_progress(world, 0.0f);
	}

	const EgShapesRectangle *ra = ecs_get(world, a, EgShapesRectangle);
	const EgShapesRectangle *rb = ecs_get(world, b, EgShapesRectangle);
	test_assert(fabsf(ra->w - 20.0f) < EPSILON);
	test_assert(fabsf(rb->w - 60.0f) < EPSILON);
}

