#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "flecs.h"
#include "eg_geometry.h"
#include "eg_window.h"
#include "eg_userevent.h"
#include "eg_base.h"
#include "eg_quantity.h"


// If Sokol is used as backend
#if 1
#include "eg_sokol.h"
#endif



// https://www.flecs.dev/explorer/?remote=true
int main(int argc, char *argv[])
{
	ecs_log_set_level(0);
	//ecs_world_t * world = ecs_init_w_args(argc, argv);
	ecs_world_t *world = ecs_init_w_args(1, (char*[]){
	"rest_test", NULL // Application name, optional
	});

	ECS_IMPORT(world, FlecsComponentsEgSokol);
	ECS_IMPORT(world, FlecsComponentsEgGeometry);
	ECS_IMPORT(world, FlecsComponentsEgWindow);
	ECS_IMPORT(world, FlecsComponentsEgQuantity);



	return ecs_app_run(world, &(ecs_app_desc_t) {
	.target_fps = 60, .enable_rest = true
	});
}
