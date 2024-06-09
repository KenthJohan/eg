#include <stdio.h>
#include <stdlib.h>

#include <flecs.h>
#include <flecs_extra.h>
#include <egquantities.h>



int main(int argc, char const * argv[])
{
	ecs_world_t *world = ecs_init();
	ECS_IMPORT(world, EgQuantities);

	ecs_plecs_from_dir(world, "config");

	ecs_log_set_level(0);
	ecs_log(0, "https://www.flecs.dev/explorer/?remote=true");
	ecs_set(world, EcsWorld, EcsRest, {.port = 0});
	ecs_log_set_level(-1);
	
	while(1) {
		ecs_os_sleep(0.0f, 100000.0f);
		//printf("ecs_progress\n");
		ecs_progress(world, 0.0f);
	}
}

