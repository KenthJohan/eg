#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "flecs.h"
#include "EgFs.h"
#include "EgStr.h"
#include "EgWin32.h"


int main (int argc, char * argv [])
{
	//_setmode(_fileno(stdout), _O_WTEXT);
	ecs_world_t *world = ecs_init_w_args(argc, argv);
	ECS_IMPORT(world, FlecsUnits);
	ECS_IMPORT(world, EgFs);
	ECS_IMPORT(world, EgWin32);

	//https://www.flecs.dev/explorer/?remote=true
	ecs_singleton_set(world, EcsRest, {0});

	/*
	ecs_entity_t w = ecs_new_entity(world, "EgFsMonitor");
	ecs_add(world, w, EgFsMonitor);

	ecs_entity_t d0 = ecs_new(world, 0);
	ecs_add_pair(world, d0, EcsChildOf, w);
	ecs_set(world, d0, EgFsMonitorDir, {false});
	ecs_set_pair(world, d0, EgText, EgFsPath, {"./"});
	*/


	ecs_entity_t a = ecs_new_entity(world, "fscwd");
	ecs_set_pair(world, a, EgText, EgFsPath, {"."});
	ecs_add(world, a, EgFsDir);
	ecs_add(world, a, EgFsList);
	ecs_add(world, a, EgFsCwd);


	while(1)
	{
		//printf("ecs_progress!\n");
		ecs_progress(world, 0);
		ecs_os_sleep(0,1000000);
	}

}
