#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "flecs.h"
#include "EgWindows.h"

void my_abort(void)
{
	printf("Aborted\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	ecs_os_set_api_defaults();
    ecs_os_api_t os_api = ecs_os_api;
    os_api.abort_ = my_abort;
    ecs_os_set_api(&os_api);

	ecs_world_t * world = ecs_init();

	ecs_log_set_level(1);
	ECS_IMPORT(world, EgWindows);

	//https://www.flecs.dev/explorer/?remote=true
    ecs_singleton_set(world, EcsRest, {0});
	
    while(true)
    {
		ecs_os_sleep(0, 1000000);
        ecs_progress(world, 0);
    }


    ecs_fini(world);

	return 0;
}



