#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "flecs.h"
#include "eg_dirwatch.h"
#include "EgFs.h"
#include "EgQuantities.h"


int main (int argc, char * argv [])
{
	//_setmode(_fileno(stdout), _O_WTEXT);
	ecs_world_t *world = ecs_init_w_args(argc, argv);
	ECS_IMPORT(world, FlecsUnits);
	ECS_IMPORT(world, EgFs);

	//eg_watch();a

	//https://www.flecs.dev/explorer/?remote=true
	ecs_singleton_set(world, EcsRest, {0});

	eg_dirwatch_bootstrap(world);
	ecs_entity_t w = eg_dirwatch_init(world, 0);
	ecs_entity_t e1 = eg_dirwatch_add(world, w, 0, "./");

	/*
	ecs_entity_t e1 = ecs_new(world, 0);
	ecs_set_pair(world, e1, EgText, EgFsPath, {"./"});
	ecs_set_pair(world, e1, EgMonitor, EgFsPath, {"./"});
	*/


	while(1)
	{
		printf("ecs_progress!\n");
		ecs_progress(world, 0);

		
		char path[EG_DIRWATCH_PATH_LENGTH];
		while(eg_dirwatch_pull(world, w, 0, path) > 0)
		{
			printf("Filechange: %s\n", path);
		}
		

		ecs_os_sleep(0,1000000);
	}

	//eg_dirwatch_fini(dw);


}
