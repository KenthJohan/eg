#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "flecs.h"
#include "EgFs.h"
#include "EgQuantities.h"
#include "EgDirwatch.h"


int main (int argc, char * argv [])
{
	//_setmode(_fileno(stdout), _O_WTEXT);
	ecs_world_t *world = ecs_init_w_args(argc, argv);
	ECS_IMPORT(world, FlecsUnits);
	ECS_IMPORT(world, EgFs);
	ECS_IMPORT(world, EgDirwatch);

	//eg_watch();a

	//https://www.flecs.dev/explorer/?remote=true
	ecs_singleton_set(world, EcsRest, {0});

	ecs_entity_t w = ecs_new_entity(world, "Car");
	ecs_add(world, w, EgFsMonitorInstance);
	ecs_entity_t d0 = ecs_new(world, 0);
	ecs_add(world, d0, EgFsMonitorDir);
	ecs_add_pair(world, d0, EcsChildOf, w);
	ecs_set_pair(world, d0, EgText, EgFsPath, {"./"});
	//ecs_entity_t d1 = ecs_new(world, 0);
	//ecs_add_pair(world, d1, EcsIsA, w);
	//ecs_set_pair(world, d1, EgText, EgFsPath, {"./"});


	//ecs_entity_t e1 = eg_dirwatch_add(world, w, 0, "./");


	/*
	ecs_entity_t e1 = ecs_new(world, 0);
	ecs_set_pair(world, e1, EgText, EgFsPath, {"./"});
	ecs_set_pair(world, e1, EgMonitor, EgFsPath, {"./"});
	*/


	while(1)
	{
		//printf("ecs_progress!\n");
		ecs_progress(world, 0);
		ecs_os_sleep(0,1000000);
	}

	//eg_dirwatch_fini(dw);


}
