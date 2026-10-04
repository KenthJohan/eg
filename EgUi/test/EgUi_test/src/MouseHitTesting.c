#include <bake_test.h>
#include <EgPhysics.h>
#include <EgShapes.h>
#include <EgSpatials.h>
#include <EgUi.h>

static ecs_world_t *world;

void MouseHitTesting_setup(void)
{
	world = ecs_init();
	ECS_IMPORT(world, EgShapes);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgUi);
}

void MouseHitTesting_teardown(void)
{
	ecs_fini(world);
	world = NULL;
}

static ecs_entity_t make_root(ecs_entity_t *mouse, ecs_entity_t *hover_tag)
{
	*mouse = ecs_new(world);
	*hover_tag = ecs_new(world);
	ecs_set(world, *mouse, Position2, {0.0f, 0.0f});

	return ecs_new(world);
}

static ecs_entity_t make_rect(ecs_entity_t root, ecs_entity_t mouse, ecs_entity_t hover_tag,
	float w, float h, float center_x, float center_y, float basis_xx, float basis_yx,
	float basis_xy, float basis_yy)
{
	ecs_entity_t rect = ecs_new_w_pair(world, EcsChildOf, root);
	ecs_set_pair(world, rect, EgPhysicsOverlapChecking, mouse, {.tag = hover_tag});
	ecs_set(world, rect, EgShapesRectangle, {.w = w, .h = h});
	ecs_set(world, rect, WorldTransform3, {
	.matrix = {
	{basis_xx, basis_xy, 0.0f},
	{basis_yx, basis_yy, 0.0f},
	{center_x, center_y, 1.0f}
	}});
	return rect;
}

void MouseHitTesting_test_axis_aligned_hit_and_clear(void)
{
	ecs_entity_t mouse;
	ecs_entity_t hover_tag;
	ecs_entity_t root = make_root(&mouse, &hover_tag);
	ecs_entity_t rect = make_rect(root, mouse, hover_tag, 4.0f, 2.0f, 10.0f, 20.0f,
		1.0f, 0.0f, 0.0f, 1.0f);
	ecs_set(world, mouse, Position2, {12.0f, 21.0f});

	ecs_progress(world, 0.0f);
	test_assert(ecs_has_id(world, rect, hover_tag));

	ecs_set(world, mouse, Position2, {12.01f, 21.0f});
	ecs_progress(world, 0.0f);
	test_assert(!ecs_has_id(world, rect, hover_tag));

	ecs_set(world, mouse, Position2, {12.0f, 21.0f});
	ecs_progress(world, 0.0f);
	test_assert(ecs_has_id(world, rect, hover_tag));
}

void MouseHitTesting_test_rotated_scaled_hit(void)
{
	ecs_entity_t mouse;
	ecs_entity_t hover_tag;
	ecs_entity_t root = make_root(&mouse, &hover_tag);
	ecs_entity_t rect = make_rect(root, mouse, hover_tag, 4.0f, 2.0f, 10.0f, 20.0f,
		0.0f, -3.0f, 2.0f, 0.0f);
	ecs_set(world, mouse, Position2, {7.0f, 22.0f});

	ecs_progress(world, 0.0f);
	test_assert(ecs_has_id(world, rect, hover_tag));

	ecs_set(world, mouse, Position2, {7.0f, 24.1f});
	ecs_progress(world, 0.0f);
	test_assert(!ecs_has_id(world, rect, hover_tag));
}

void MouseHitTesting_test_overlapping_rectangles_all_hover(void)
{
	ecs_entity_t mouse;
	ecs_entity_t hover_tag;
	ecs_entity_t root = make_root(&mouse, &hover_tag);
	ecs_entity_t first = make_rect(root, mouse, hover_tag, 10.0f, 10.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 1.0f);
	ecs_entity_t second = make_rect(root, mouse, hover_tag, 4.0f, 4.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 1.0f);

	ecs_progress(world, 0.0f);
	test_assert(ecs_has_id(world, first, hover_tag));
	test_assert(ecs_has_id(world, second, hover_tag));
}
