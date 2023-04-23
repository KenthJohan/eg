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

	eg_dirwatch_t * m = ecs_os_malloc(eg_dirwatch_size());
	eg_dirwatch_init(m);
	eg_dirwatch_add(m, "./");

	while(1)
	{
		printf("ecs_progress!\n");
		ecs_progress(world, 0);


		char path[EG_DIRWATCH_PATH_LENGTH];
		if(eg_dirwatch_pull(m, 0, path))
		{
			printf("Filechange: %s\n", path);
		}

		ecs_os_sleep(1,0);
	}

	ecs_os_free(m);


}
