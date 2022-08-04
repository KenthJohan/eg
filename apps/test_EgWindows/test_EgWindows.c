#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "flecs.h"
#include "EgWindows.h"


int main(int argc, char **argv)
{
	ecs_world_t * world = ecs_init();
	ECS_IMPORT(world, EgWindows);
	ecs_fini(world);

	return 0;
}



