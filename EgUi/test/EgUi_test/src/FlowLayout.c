#include <bake_test.h>
#include <EgShapes.h>
#include <EgSpatials.h>
#include <EgUi.h>
#include <math.h>

#define EPSILON 1e-4f

static ecs_world_t *world;

void FlowLayout_setup(void)
{
	world = ecs_init();
	ECS_IMPORT(world, EgShapes);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgUi);
}

void FlowLayout_teardown(void)
{
	ecs_fini(world);
	world = NULL;
}

static ecs_entity_t make_child(ecs_entity_t parent, float w, float h)
{
	ecs_entity_t e = ecs_new(world);
	ecs_add_pair(world, e, EcsChildOf, parent);
	ecs_set(world, e, EgShapesRectangle, {.w = w, .h = h});
	ecs_set(world, e, Position2, {0.0f, 0.0f});
	return e;
}

static void assert_pos(ecs_entity_t e, float x, float y)
{
	const Position2 *p = ecs_get(world, e, Position2);
	test_assert(p != NULL);
	test_assert(fabsf(p->x - x) < EPSILON);
	test_assert(fabsf(p->y - y) < EPSILON);
}

void FlowLayout_test_children_flow_right_then_wrap_down(void)
{
	ecs_entity_t parent = ecs_new(world);
	ecs_set(world, parent, EgShapesRectangle, {.w = 100, .h = 100});
	ecs_set(world, parent, EgUiFlowsFlow, {.direction = EgUiDirectionRight, .wrap = EgUiDirectionDown});

	ecs_entity_t a = make_child(parent, 40, 40);
	ecs_entity_t b = make_child(parent, 40, 40);
	ecs_entity_t c = make_child(parent, 40, 40);

	ecs_progress(world, 0);

	assert_pos(a, -30.0f, 30.0f);
	assert_pos(b, 10.0f, 30.0f);
	assert_pos(c, -30.0f, -10.0f);
}

void FlowLayout_test_none_direction_leaves_children_untouched(void)
{
	ecs_entity_t parent = ecs_new(world);
	ecs_set(world, parent, EgShapesRectangle, {.w = 100, .h = 100});
	ecs_set(world, parent, EgUiFlowsFlow, {.direction = EgUiDirectionNone});

	ecs_entity_t a = make_child(parent, 40, 40);
	ecs_set(world, a, Position2, {7.0f, 9.0f});

	ecs_progress(world, 0);

	assert_pos(a, 7.0f, 9.0f);
}

void FlowLayout_test_parent_clearance_is_signed(void)
{
	ecs_entity_t parent = ecs_new(world);
	ecs_set(world, parent, EgShapesRectangle, {.w = 100, .h = 80});
	ecs_entity_t child = make_child(parent, 20, 10);
	ecs_set(world, child, EgUiParentClearance, {0});

	ecs_progress(world, 0);
	const EgUiParentClearance *clearance = ecs_get(world, child, EgUiParentClearance);
	test_assert(clearance != NULL);
	test_assert(fabsf(clearance->left + 40.0f) < EPSILON);
	test_assert(fabsf(clearance->right + 40.0f) < EPSILON);
	test_assert(fabsf(clearance->bottom + 35.0f) < EPSILON);
	test_assert(fabsf(clearance->top + 35.0f) < EPSILON);

	ecs_set(world, child, Position2, {45.0f, 0.0f});
	ecs_progress(world, 0);
	clearance = ecs_get(world, child, EgUiParentClearance);
	test_assert(fabsf(clearance->right - 5.0f) < EPSILON);
	test_assert(clearance->left < 0.0f);
}

void FlowLayout_test_oversized_child_is_skipped_and_retried(void)
{
	ecs_entity_t parent = ecs_new(world);
	ecs_set(world, parent, EgShapesRectangle, {.w = 100, .h = 100});
	ecs_set(world, parent, EgUiFlowsFlow, {.direction = EgUiDirectionRight, .wrap = EgUiDirectionDown});
	ecs_entity_t first = make_child(parent, 40, 40);
	ecs_entity_t oversized = make_child(parent, 120, 40);
	ecs_set(world, oversized, EgUiParentClearance, {0});
	ecs_entity_t last = make_child(parent, 40, 40);

	ecs_progress(world, 0);
	test_assert(ecs_has_id(world, oversized, EcsDisabled));
	const EgUiParentClearance *clearance = ecs_get(world, oversized, EgUiParentClearance);
	test_assert(clearance != NULL);
	test_assert(clearance->left > 0.0f || clearance->right > 0.0f);
	assert_pos(first, -30.0f, 30.0f);
	assert_pos(last, 10.0f, 30.0f);

	ecs_set(world, parent, EgShapesRectangle, {.w = 150, .h = 100});
	ecs_progress(world, 0);
	test_assert(!ecs_has_id(world, oversized, EcsDisabled));
	assert_pos(oversized, -15.0f, -10.0f);
	clearance = ecs_get(world, oversized, EgUiParentClearance);
	test_assert(clearance->left <= EPSILON && clearance->right <= EPSILON);
}

void FlowLayout_test_manually_disabled_child_stays_disabled(void)
{
	ecs_entity_t parent = ecs_new(world);
	ecs_set(world, parent, EgShapesRectangle, {.w = 100, .h = 100});
		ecs_set(world, parent, EgUiFlowsFlow, {.direction = EgUiDirectionRight, .wrap = EgUiDirectionDown});
	ecs_entity_t disabled = make_child(parent, 40, 40);
	ecs_enable(world, disabled, false);
	ecs_entity_t visible = make_child(parent, 40, 40);

	ecs_progress(world, 0);
	test_assert(ecs_has_id(world, disabled, EcsDisabled));
	test_assert(!ecs_has_id(world, disabled, ecs_id(EgUiFlowsFlowUnplaced)));
	assert_pos(visible, -30.0f, 30.0f);
}
