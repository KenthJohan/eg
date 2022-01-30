#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "flecs.h"
#include "eg_geometry.h"
#include "eg_window.h"
#include "eg_userevent.h"
#include "eg_base.h"
#include "eg_quantity.h"

// If SDL is used as backend
#if 1
#include "eg_sdl.h"
#include "eg_sokol_gfx.h"
#endif


// https://www.flecs.dev/explorer/?remote=true
int main(int argc, char *argv[])
{
	ecs_log_set_level(0);
	//ecs_world_t * world = ecs_init_w_args(argc, argv);
	ecs_world_t *world = ecs_init_w_args(1, (char*[]){
	"rest_test", NULL // Application name, optional
	});

	ECS_IMPORT(world, FlecsComponentsEgWindow);
	ECS_IMPORT(world, FlecsComponentsEgSdl);
	ECS_IMPORT(world, FlecsComponentsEgSokolGfx);
	ECS_IMPORT(world, FlecsComponentsEgGeometry);
	ECS_IMPORT(world, FlecsComponentsEgQuantity);

	ecs_entity_t window1 = ecs_new(world, 0);
	ecs_set_name(world, window1, "Window1");
	ecs_set(world, window1, EgRectangleI32, {800, 800});
	ecs_set(world, window1, EgWindow, {EG_WINDOW_OPENGL|EG_WINDOW_RESIZABLE, 0, false});
	ecs_set(world, window1, EgTitle, {"Window1"});

	/*
	ecs_entity_t window2 = ecs_new(world, 0);
	ecs_set_name(world, window2, "Window2");
	ecs_set(world, window2, EgRectangleI32, {800, 800});
	ecs_set(world, window2, EgWindow, {EG_WINDOW_OPENGL|EG_WINDOW_RESIZABLE, 0, false});
	ecs_set(world, window2, EgTitle, {"Window2"});
	*/

	while (1)
	{
		ecs_os_sleep(0,100000);
		ecs_progress(world, 0);

	}

	return 0;
}
