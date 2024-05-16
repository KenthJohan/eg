#include <flecs.h>
#include <egcan.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char const * argv[])
{
	ecs_world_t *world = ecs_init();


	ECS_IMPORT(world, EgCan);
	// https://www.flecs.dev/explorer/?remote=true
	ecs_set(world, EcsWorld, EcsRest, {.port = 0});
	while(1) {
		ecs_os_sleep(0.0f, 100000.0f);
		//printf("ecs_progress\n");
		ecs_progress(world, 0.0f);
	}
}

