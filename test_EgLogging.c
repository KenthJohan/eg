#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <flecs.h>

#include "EgLogging.h"
#include "eg_basics.h"






static void * thread1(void *arg)
{
	int i = INT_MAX;
	while(1)
	{
		ecs_os_sleep(0,100000000);
		ecs_trace("thread1 %i", i--);
	}
}

static void * thread2(void *arg)
{
	int i = 0;
	while(1)
	{
		ecs_os_sleep(0,500000000);
		ecs_trace("thread2 %i", i++);
	}
}

static void * thread3(void *arg)
{
	int i = 0;
	while(1)
	{
		ecs_os_sleep(0,800000000);
		ecs_trace("thread3 %x", i++);
	}
}

// https://www.flecs.dev/explorer/?remote=true
int main(int argc, char *argv[])
{
	eg_basics_enable_ternminal_color();
	ecs_log_set_level(0);
	ecs_world_t *world = ecs_init();
	ecs_singleton_set(world, EcsRest, {0});
	ECS_IMPORT(world, FlecsMonitor);
	ECS_IMPORT(world, FlecsUnits);
	ECS_IMPORT(world, EgLogging);

	ecs_trace("Testing %p", world);
	ecs_trace("Testing %p", world);
	ecs_trace("Testing %p", world);
	ecs_trace("Testing %p", world);
	ecs_trace("Testing %p", world);


	ecs_os_thread_t t1 = ecs_os_thread_new(thread1, NULL);
	ecs_os_thread_t t2 = ecs_os_thread_new(thread2, NULL);
	ecs_os_thread_t t3 = ecs_os_thread_new(thread3, NULL);


	ecs_set(world, EcsWorld, EcsRest, {0});
	while (1)
	{
		ecs_os_sleep(1,0);
		ecs_progress(world, 0);
	}

	return 0;
}
