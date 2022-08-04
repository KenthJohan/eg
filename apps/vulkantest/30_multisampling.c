#include "flecs.h"
#include "renderer.h"

#include "EgWindows.h"

int main()
{
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
