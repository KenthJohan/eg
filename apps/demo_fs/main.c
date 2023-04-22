#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "flecs.h"
#include "eg_fs.h"
#include "EgFs.h"
#include "EgQuantities.h"


int main (int argc, char * argv [])
{
	ecs_world_t *world = ecs_init_w_args(argc, argv);
	ECS_IMPORT(world, FlecsUnits);
	ECS_IMPORT(world, EgFs);

	//eg_watch();a

	//https://www.flecs.dev/explorer/?remote=true
	ecs_singleton_set(world, EcsRest, {0});



	eg_fs_monitor_t * m = eg_fs_monitor_init("./", malloc(eg_fs_monitor_size()));

	while(1)
	{
		printf("ecs_progress!\n");
		ecs_progress(world, 0);
		char path[EG_FS_PATH_LENGTH];
		if(eg_fs_pull_changes(m, -1, path))
		{
			printf("Filechange: %s\n", path);
		}

	}


}
