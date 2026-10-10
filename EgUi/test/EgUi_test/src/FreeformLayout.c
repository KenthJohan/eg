#include <bake_test.h>
#include <EgButtons.h>
#include <EgIntersects.h>
#include <EgPhysics.h>
#include <EgShapes.h>
#include <EgSpatials.h>
#include <EgUi.h>
#include <math.h>

#define EPSILON 1e-4f
#define MOUSE_LEFT_KEY ((1u << 16) | 1u)

static ecs_world_t *world;
static ecs_entity_t mouse;

void FreeformLayout_setup(void)
{
	world = ecs_init();
	ECS_IMPORT(world, EgShapes);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgUi);
	ecs_singleton_set(world, EgButtonsState, {.mouse = {[1] = EG_BUTTONS_STATE_HELD}});

	mouse = ecs_new(world);
	ecs_set(world, mouse, Position2, {0.0f, 0.0f});
}

void FreeformLayout_teardown(void)
{
	ecs_fini(world);
	world = NULL;
}

void FreeformLayout_test_anchor_positions_child_from_parent_pivot(void)
{
	ecs_entity_t parent = ecs_new(world);
	ecs_set(world, parent, EgShapesRectangle, {.w = 100.0f, .h = 80.0f});
	ecs_set(world, parent, EgUiFreeformsLayout, {.dummy = 0});

	ecs_entity_t child = ecs_new_w_pair(world, EcsChildOf, parent);
	ecs_set(world, child, EgShapesRectangle, {.w = 20.0f, .h = 10.0f});
	ecs_set(world, child, Scale2, {1.0f, 1.0f});
	ecs_set(world, child, Position2, {0.0f, 0.0f});
	ecs_set(world, child, EgUiFreeformsAnchor, {
		.parent = EgUiFreeformsAnchorKindTopRight,
		.pivot = EgUiFreeformsAnchorKindTopRight,
		.x = -10.0f,
		.y = -5.0f
	});

	ecs_progress(world, 0.0f);
	const Position2 *pos = ecs_get(world, child, Position2);
	test_assert(fabsf(pos->x - 30.0f) < EPSILON);
	test_assert(fabsf(pos->y - 30.0f) < EPSILON);
}

void FreeformLayout_test_flow_parent_does_not_apply_freeform_anchor(void)
{
	ecs_entity_t parent = ecs_new(world);
	ecs_set(world, parent, EgShapesRectangle, {.w = 100.0f, .h = 80.0f});
	ecs_set(world, parent, EgUiFlowsFlow, {.direction = EgUiDirectionNone});

	ecs_entity_t child = ecs_new_w_pair(world, EcsChildOf, parent);
	ecs_set(world, child, EgShapesRectangle, {.w = 20.0f, .h = 10.0f});
	ecs_set(world, child, Scale2, {1.0f, 1.0f});
	ecs_set(world, child, Position2, {7.0f, 9.0f});
	ecs_set(world, child, EgUiFreeformsAnchor, {
		.parent = EgUiFreeformsAnchorKindTopRight,
		.pivot = EgUiFreeformsAnchorKindTopRight
	});

	ecs_progress(world, 0.0f);
	const Position2 *pos = ecs_get(world, child, Position2);
	test_assert(fabsf(pos->x - 7.0f) < EPSILON);
	test_assert(fabsf(pos->y - 9.0f) < EPSILON);
}

void FreeformLayout_test_resizing_anchored_child_rebuilds_position(void)
{
	ecs_entity_t parent = ecs_new(world);
	ecs_set(world, parent, EgShapesRectangle, {.w = 100.0f, .h = 100.0f});
	ecs_set(world, parent, EgUiFreeformsLayout, {.dummy = 0});

	ecs_entity_t child = ecs_new_w_pair(world, EcsChildOf, parent);
	ecs_entity_t hover_tag = ecs_new(world);
	ecs_set_pair(world, child, EgPhysicsOverlapChecking, mouse, {.tag = hover_tag});
	ecs_set(world, child, EgShapesRectangle, {.w = 40.0f, .h = 40.0f});
	ecs_set(world, child, EgIntersectsRectangleBorder, {.margin = 5.0f});
	ecs_set(world, child, Position2, {0.0f, 0.0f});
	ecs_set(world, child, Rotation2, {0.0f});
	ecs_set(world, child, Scale2, {1.0f, 1.0f});
	ecs_set(world, child, WorldTransform3, {
		.matrix = {
			{1.0f, 0.0f, 0.0f},
			{0.0f, 1.0f, 0.0f},
			{20.0f, 20.0f, 1.0f}
		}});
	ecs_set(world, child, EgUiFreeformsAnchor, {
		.parent = EgUiFreeformsAnchorKindTopRight,
		.pivot = EgUiFreeformsAnchorKindTopRight,
		.x = -10.0f,
		.y = -10.0f
	});
	ecs_set(world, child, EgUiBoundsResizable, {.key = MOUSE_LEFT_KEY, .min_w = 20.0f, .min_h = 20.0f});
	ecs_set(world, mouse, Position2, {0.0f, 20.0f});

	ecs_progress(world, 0.0f);
	ecs_set(world, mouse, Position2, {10.0f, 20.0f});
	ecs_progress(world, 0.0f);

	const EgShapesRectangle *rect = ecs_get(world, child, EgShapesRectangle);
	const Position2 *pos = ecs_get(world, child, Position2);
	test_assert(fabsf(rect->w - 30.0f) < EPSILON);
	test_assert(fabsf(pos->x - 25.0f) < EPSILON);
}
