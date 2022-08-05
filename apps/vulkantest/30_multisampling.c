#include "flecs.h"
#include "renderer.h"

#include "EgWindows.h"
#include <stdio.h>
#include <stdlib.h>

void myabort()
{
	printf("myabort\n");
	abort();
}


int main()
{
	ecs_os_set_api_defaults();
	ecs_os_api_t os_api = ecs_os_api;
	os_api.abort_ = myabort;
	ecs_os_set_api(&os_api);


	ecs_world_t * world = ecs_init();

	renderer_init();
	int r;
	do
	{
		r = renderer_update();
		ecs_progress(world, 0);
		ecs_os_sleep(0,100000);
	}
	while(r == 0);
	renderer_fini();

	ecs_fini(world);
}
