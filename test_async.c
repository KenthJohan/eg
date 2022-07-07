#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "flecs.h"
#include "eg_basics.h"

#include <signal.h>
typedef struct {
  float x, y;
} Position, Velocity;
ECS_COMPONENT_DECLARE(Position);
ECS_COMPONENT_DECLARE(Velocity);
static void * My_Thread(void * arg)
{
	ecs_world_t * world = arg;
	ecs_world_t *async = ecs_async_stage_new(world);
	while(1)
	{
		printf("ecs_stage_is_readonly(async) %i\n", ecs_stage_is_readonly(async)); //prints 0
		printf("ecs_stage_is_readonly(world) %i\n", ecs_stage_is_readonly(world)); //prints 1
		ecs_entity_t e = ecs_new(async, 0);
		ecs_add(async, e, Position);
		ecs_set(async, e, Velocity, {(float)rand()/(float)RAND_MAX, (float)rand()/(float)RAND_MAX});
		ecs_merge(async);
		ecs_os_sleep(((float)rand()/(float)RAND_MAX),0);
	}
	ecs_async_stage_free(async);
	return NULL;
}

void myabort()
{
	raise(SIGINT);
}

int main(int argc, char *argv[])
{
	ecs_os_set_api_defaults();
	ecs_os_api_t os_api = ecs_os_api;
	os_api.abort_ = myabort;
	ecs_os_set_api(&os_api);

	ecs_world_t *world = ecs_init_w_args(argc, argv);
	//ecs_singleton_set(world, EcsRest, {0});
	ECS_COMPONENT_DEFINE(world, Position);
	ECS_COMPONENT_DEFINE(world, Velocity);
	ecs_os_thread_new(My_Thread, world);
	ecs_entity_t e = ecs_new(world, 0);
	ecs_add(world, e, Position);
	ecs_set(world, e, Velocity, {1, 1});
	while (1)
	{
		ecs_os_sleep(0,100000);
		ecs_progress(world, 0);
	}
	return 0;
}
