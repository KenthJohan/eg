#include <bake_test.h>
#include <EgButtons.h>
#include <EgPhysics.h>
#include <EgShapes.h>
#include <EgSpatials.h>
#include <EgUi.h>

#define MOUSE_LEFT_KEY ((1u << 16) | 1u)
#define EPSILON 1e-4f

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
	ecs_entity_t child = ecs_new_w_pair(world, EcsChildOf, parent);
	ecs_entity_t hover_tag = ecs_new(world);
	ecs_set_pair(world, child, EgPhysicsOverlapChecking, mouse, {.tag = hover_tag});
	ecs_set(world, child, EgShapesRectangle, {.w = 80.0f, .h = 40.0f});
	ecs_set(world, child, Position2, {0.0f, 0.0f});
	ecs_set(world, child, Rotation2, {0.0f});
	ecs_set(world, child, Scale2, {1.0f, 1.0f});
	ecs_set(world, child, WorldTransform3, {
		.matrix = {
			{1.0f, 0.0f, 0.0f},
			{0.0f, 1.0f, 0.0f},
			{0.0f, 0.0f, 1.0f}
		}});
	ecs_set(world, child, EgUiResizable, {.key = MOUSE_LEFT_KEY, .grab = 5.0f, .min_w = 20.0f, .min_h = 10.0f});

	ecs_progress(world, 0.0f);
	ecs_set(world, mouse, Position2, {60.0f, 0.0f});
	ecs_progress(world, 0.0f);

	const EgShapesRectangle *rect = ecs_get(world, child, EgShapesRectangle);
	const Position2 *pos = ecs_get(world, child, Position2);
	test_assert(fabsf(rect->w - 90.0f) < EPSILON);
	test_assert(fabsf(pos->x - 5.0f) < EPSILON);
}

void Resizable_test_disabled_entity_cancels_drag(void)
{
	ecs_entity_t child = ecs_new(world);
	ecs_set(world, child, EgUiResizable, {
		.key = MOUSE_LEFT_KEY,
		.edge = EG_UI_EDGE_RIGHT,
		.dragging = true,
		.was_held = true
	});
	ecs_enable(world, child, false);

	ecs_progress(world, 0.0f);

	const EgUiResizable *resizable = ecs_get(world, child, EgUiResizable);
	test_assert(!resizable->dragging);
	test_assert(resizable->edge == 0);
}
