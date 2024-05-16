#include <flecs.h>
#include <stdio.h>

#include "cancode.h"

int main(int argc, char const * argv[])
{

	ecs_world_t *world = ecs_init();
	// https://www.flecs.dev/explorer/?remote=true
	ecs_set(world, EcsWorld, EcsRest, {.port = 0});


	ecs_os_thread_new(cancode_thread, NULL);

	while(1) {
		ecs_os_sleep(0.0f, 100000.0f);
		//printf("ecs_progress\n");
		ecs_progress(world, 0.0f);
	}

}
