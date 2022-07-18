#include "flecs.h"
#include "renderer.h"


int main()
{
	ecs_world_t * world = ecs_init();
	renderer_init(world);
	ecs_fini(world);
}
