#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <flecs.h>

#include "eg_sdl.h"
#include "eg_geometry.h"
#include "eg_window.h"





// https://www.flecs.dev/explorer/?remote=true
int main(int argc, char *argv[])
{
	ecs_log_set_level(0);
	//ecs_world_t * world = ecs_init_w_args(argc, argv);
	ecs_world_t *world = ecs_init_w_args(1, (char*[]){
	"rest_test", NULL // Application name, optional
	});

	ECS_IMPORT(world, FlecsComponentsEgSDL);
	ECS_IMPORT(world, FlecsComponentsEgGeometry);
	ECS_IMPORT(world, FlecsComponentsEgWindow);

	ecs_entity_t e1 = ecs_new(world, 0);
	ecs_set(world, e1, EgWindow, {NULL});
	ecs_set(world, e1, EgRectangleI32, {400, 300});
	
	ecs_entity_t e2 = ecs_new_w_pair(world, EcsChildOf, e1);
	ecs_set(world, e2, EgDraw, {1});
	ecs_set(world, e2, EgRectangleF32, {200, 200});

	ecs_entity_t e3 = ecs_new_w_pair(world, EcsChildOf, e1);
	ecs_set(world, e3, EgDraw, {1});
	ecs_set(world, e3, EgRectangleF32, {100, 100});

	ecs_set(world, EcsWorld, EcsRest, {0});
	
	while (1)
	{
		ecs_os_sleep(0,100000);
		ecs_progress(world, 0);
	}

	return 0;
}
