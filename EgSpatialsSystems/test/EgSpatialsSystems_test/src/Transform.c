#include <bake_test.h>
#include <EgSpatials.h>
#include <EgSpatialsSystems.h>
#include <math.h>

#define EPSILON 1e-4f

static ecs_world_t *world;

void Transform_setup(void)
{
	world = ecs_init();
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgSpatialsSystems);
}

void Transform_teardown(void)
{
	ecs_fini(world);
	world = NULL;
}

void Transform_test_positive_z_rotation_moves_child_toward_positive_y(void)
{
	const float        half_sqrt_two = 0.70710678f;
	const float        sin_pi_eighth = 0.38268343f;
	const float        cos_pi_eighth = 0.92387953f;
	const ecs_entity_t parent        = ecs_new(world);
	const ecs_entity_t child         = ecs_new_w_pair(world, EcsChildOf, parent);

	ecs_set(world, parent, Position3, {0.0f, 0.0f, 0.0f});
	ecs_set(world, parent, Position3World, {0.0f, 0.0f, 0.0f});
	ecs_set(world, parent, Orientation, {0.0f, 0.0f, half_sqrt_two, half_sqrt_two});
	ecs_set(world, parent, OrientationWorld, {0.0f, 0.0f, 0.0f, 1.0f});
	ecs_set(world, parent, Scale3, {2.0f, 1.0f, 1.0f});
	ecs_set(world, parent, Matrix4, {{0}});

	ecs_set(world, child, Position3, {1.0f, 0.0f, 0.0f});
	ecs_set(world, child, Position3World, {0.0f, 0.0f, 0.0f});
	ecs_set(world, child, Position3WorldOffset, {0.0f, 0.0f, 0.0f});
	ecs_set(world, child, Orientation, {0.0f, 0.0f, sin_pi_eighth, cos_pi_eighth});
	ecs_set(world, child, OrientationWorld, {0.0f, 0.0f, 0.0f, 1.0f});
	ecs_set(world, child, Scale3, {1.0f, 1.0f, 1.0f});
	ecs_set(world, child, Matrix4, {{0}});
	ecs_set(world, child, Sinewave, {0.0f, 3.0f});

	ecs_progress(world, 0.0f);

	const Position3World       *child_position  = ecs_get(world, child, Position3World);
	const Position3WorldOffset *child_offset    = ecs_get(world, child, Position3WorldOffset);
	const Matrix4              *child_transform = ecs_get(world, child, Matrix4);
	test_assert(child_position != NULL);
	test_assert(child_offset != NULL);
	test_assert(child_transform != NULL);
	test_assert(fabsf(child_position->x) < EPSILON);
	test_assert(fabsf(child_position->y - 5.0f) < EPSILON);
	test_assert(fabsf(child_offset->y - 3.0f) < EPSILON);
	test_assert(fabsf(child_transform->matrix.c0[0] + 0.70710678f) < EPSILON);
	test_assert(fabsf(child_transform->matrix.c0[1] - 1.41421356f) < EPSILON);
	test_assert(fabsf(child_transform->matrix.c3[1] - 5.0f) < EPSILON);
}

void Transform_test_three_level_scene_graph_composes_scale(void)
{
	const ecs_entity_t root   = ecs_new(world);
	const ecs_entity_t middle = ecs_new_w_pair(world, EcsChildOf, root);
	const ecs_entity_t leaf   = ecs_new_w_pair(world, EcsChildOf, middle);

	ecs_set(world, root, Position3, {0.0f, 0.0f, 0.0f});
	ecs_set(world, root, Orientation, {0.0f, 0.0f, 0.0f, 1.0f});
	ecs_set(world, root, Scale3, {2.0f, 2.0f, 2.0f});
	ecs_set(world, root, Position3World, {0.0f, 0.0f, 0.0f});
	ecs_set(world, root, Matrix4, {{0}});

	ecs_set(world, middle, Position3, {1.0f, 0.0f, 0.0f});
	ecs_set(world, middle, Orientation, {0.0f, 0.0f, 0.0f, 1.0f});
	ecs_set(world, middle, Scale3, {3.0f, 3.0f, 3.0f});
	ecs_set(world, middle, Position3World, {0.0f, 0.0f, 0.0f});
	ecs_set(world, middle, Matrix4, {{0}});

	ecs_set(world, leaf, Position3, {1.0f, 0.0f, 0.0f});
	ecs_set(world, leaf, Orientation, {0.0f, 0.0f, 0.0f, 1.0f});
	ecs_set(world, leaf, Scale3, {0.5f, 0.5f, 0.5f});
	ecs_set(world, leaf, Position3World, {0.0f, 0.0f, 0.0f});
	ecs_set(world, leaf, Matrix4, {{0}});

	ecs_progress(world, 0.0f);

	const Matrix4 *middle_transform = ecs_get(world, middle, Matrix4);
	const Matrix4 *leaf_transform   = ecs_get(world, leaf, Matrix4);
	test_assert(middle_transform != NULL);
	test_assert(leaf_transform != NULL);
	test_assert(fabsf(middle_transform->matrix.c0[0] - 6.0f) < EPSILON);
	test_assert(fabsf(middle_transform->matrix.c3[0] - 2.0f) < EPSILON);
	test_assert(fabsf(leaf_transform->matrix.c0[0] - 3.0f) < EPSILON);
	test_assert(fabsf(leaf_transform->matrix.c3[0] - 8.0f) < EPSILON);
}
