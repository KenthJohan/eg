#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <flecs.h>

#include "eg_log.h"



static void main_init()
{
#ifdef _WIN32
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	GetConsoleMode (hOut, &dwMode);
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode (hOut, dwMode);
#endif
}


static void * thread2(void *arg)
{
	int i = 0;
	while(1)
	{
		ecs_os_sleep(1,0);
		ecs_trace("Testing %i %i", i++, stall);
	}
}

static void * thread3(void *arg)
{
	int i = 0;
	while(1)
	{
		ecs_os_sleep(0,50000);
		ecs_trace("Testing %i %i", i++, stall);
	}
}


// https://www.flecs.dev/explorer/?remote=true
int main(int argc, char *argv[])
{
	main_init();
	ecs_log_set_level(0);
	//ecs_world_t * world = ecs_init_w_args(argc, argv);
	ecs_world_t *world = ecs_init_w_args(1, (char*[]){
	"rest_test", NULL // Application name, optional
	});

	ECS_IMPORT(world, FlecsComponentsEgLog);

	ecs_trace("Testing %p", world);
	ecs_trace("Testing %p", world);
	ecs_trace("Testing %p", world);
	ecs_trace("Testing %p", world);
	ecs_trace("Testing %p", world);


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
