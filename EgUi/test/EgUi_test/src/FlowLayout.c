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
