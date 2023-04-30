/*
https://gist.github.com/nickav/a57009d4fcc3b527ed0f5c9cf30618f8
http://old.zaynar.co.uk/cppdoc/latest/projects/lib.sysdep.win/wdir_watch.cpp.html
https://github.com/tresorit/rdcfswatcherexample/blob/master/rdc_fs_watcher.cpp
https://github.com/0ad/0ad/blob/master/source/lib/sysdep/os/win/wiocp.cpp
https://github.com/0ad/0ad/blob/master/source/lib/sysdep/os/win/wdir_watch.cpp
https://stackoverflow.com/questions/339776/asynchronous-readdirectorychangesw
*/

#include "flecs.h"

#if defined(ECS_TARGET_WINDOWS)

#include "EgDirwatch.h"
#include "EgFs.h"
#include "EgQuantities.h"
#include "EgWin32.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>



#define FLOG(...) fprintf(__VA_ARGS__)


void System_Dirwatch_Init(ecs_iter_t *it)
{
	for (int i = 0; i < it->count; i ++)
	{
		ecs_add(it->world, it->entities[i], EgWin32Handle);
		ecs_add(it->world, it->entities[i], EgWin32IOCP);
	}
}


void EgDirwatchImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgDirwatch);
	ECS_IMPORT(world, EgWin32);
	
	ecs_system(world, {
		.entity = ecs_entity(world, {
		.name = "System_Dirwatch_Init",
		.add = { ecs_dependson(EcsOnUpdate) }
		}),
		.query.filter.terms = {
		{ .id = ecs_id(EgFsMonitorInstance), },
		{ .id = ecs_id(EgWin32IOCP), .oper=EcsNot },
		{ .id = ecs_id(EgWin32Handle), .oper=EcsNot }
		},
		.callback = System_Dirwatch_Init
	});

}

























#endif