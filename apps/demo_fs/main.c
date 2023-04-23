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

	eg_dirwatch_desc_t desc = {0};
	eg_dirwatch_t * dw = eg_dirwatch_init(&desc);
	eg_dirwatch_add(dw, "./");

	while(1)
	{
		printf("ecs_progress!\n");
		ecs_progress(world, 0);


		char path[EG_DIRWATCH_PATH_LENGTH];
		while(eg_dirwatch_pull(dw, 0, path) > 0)
		{
			printf("Filechange: %s\n", path);
		}

		ecs_os_sleep(1,0);
	}

	eg_dirwatch_fini(dw);


}
