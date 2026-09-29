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
	const float half_sqrt_two = 0.70710678f;
	const ecs_entity_t parent = ecs_new(world);
	const ecs_entity_t child = ecs_new_w_pair(world, EcsChildOf, parent);

	ecs_set(world, parent, Position3, {0.0f, 0.0f, 0.0f});
	ecs_set(world, parent, Position3World, {0.0f, 0.0f, 0.0f});
	ecs_set(world, parent, Orientation, {0.0f, 0.0f, half_sqrt_two, half_sqrt_two});
	ecs_set(world, parent, OrientationWorld, {0.0f, 0.0f, 0.0f, 1.0f});
	ecs_set(world, parent, Scale3, {1.0f, 1.0f, 1.0f});
	ecs_set(world, parent, Scale3World, {1.0f, 1.0f, 1.0f});
	ecs_set(world, parent, Transformation, {{0}});

	ecs_set(world, child, Position3, {1.0f, 0.0f, 0.0f});
	ecs_set(world, child, Position3World, {0.0f, 0.0f, 0.0f});
	ecs_set(world, child, Orientation, {0.0f, 0.0f, 0.0f, 1.0f});
	ecs_set(world, child, OrientationWorld, {0.0f, 0.0f, 0.0f, 1.0f});
	ecs_set(world, child, Scale3, {1.0f, 1.0f, 1.0f});
	ecs_set(world, child, Scale3World, {1.0f, 1.0f, 1.0f});
	ecs_set(world, child, Transformation, {{0}});

	ecs_progress(world, 0.0f);
	ecs_progress(world, 0.0f);

	const Position3World *child_position = ecs_get(world, child, Position3World);
	const Transformation *child_transform = ecs_get(world, child, Transformation);
	test_assert(child_position != NULL);
	test_assert(child_transform != NULL);
	test_assert(fabsf(child_position->x) < EPSILON);
	test_assert(fabsf(child_position->y - 1.0f) < EPSILON);
	test_assert(fabsf(child_transform->matrix.c0[0]) < EPSILON);
	test_assert(fabsf(child_transform->matrix.c0[1] - 1.0f) < EPSILON);
}
