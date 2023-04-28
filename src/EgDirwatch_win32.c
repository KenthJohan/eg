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
#include "win32error.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>



#define FLOG(...) fprintf(__VA_ARGS__)
#define CHANGE_BUF_SIZE 1024*3


typedef struct
{
	HANDLE file;
	OVERLAPPED overlapped;
	uint8_t * change_buf;
	int dummy;
} _eg_direvent_t;





ECS_COMPONENT_DECLARE(_eg_direvent_t);



char const * action_to_string(DWORD Action)
{
	switch (Action)
	{
		case FILE_ACTION_ADDED:return "FILE_ACTION_ADDED";
		case FILE_ACTION_REMOVED:return "FILE_ACTION_REMOVED";
		case FILE_ACTION_MODIFIED:return "FILE_ACTION_MODIFIED";
		case FILE_ACTION_RENAMED_OLD_NAME:return "FILE_ACTION_RENAMED_OLD_NAME";
		case FILE_ACTION_RENAMED_NEW_NAME:return "FILE_ACTION_RENAMED_NEW_NAME";
		default: return "Unknown action";
	}
}




void System_Dir_Init(ecs_iter_t *it)
{
	EgWin32IOCP * dirwatch = ecs_field(it, EgWin32IOCP, 1);
	EgText *path = ecs_field(it, EgText, 2);
	for (int i = 0; i < it->count; i ++)
	{
		FLOG(stdout, "Appending new direvent %jx\n", (uintmax_t)it->entities[i]);
		_eg_direvent_t * dir = ecs_get_mut(it->world, it->entities[i], _eg_direvent_t);
		dir->file = CreateFile(path[i].value,
			 FILE_LIST_DIRECTORY,
			 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			 NULL,
			 OPEN_EXISTING,
			 FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
			 NULL);
		//e->overlapped.hEvent = CreateEvent(NULL, FALSE, 0, NULL);
		DWORD NumberOfConcurrentThreads = 0;
		HANDLE hIOCP = CreateIoCompletionPort(dir->file, dirwatch->hIOCP, (ULONG_PTR)it->entities[i], NumberOfConcurrentThreads);
		assert(hIOCP == dirwatch->hIOCP);
		if((hIOCP == 0) || (hIOCP == INVALID_HANDLE_VALUE))
		{
			FLOG(stderr, "Error: CreateIoCompletionPort\n");
		}
		WINBOOL success = PostQueuedCompletionStatus(hIOCP, 0, (ULONG_PTR)it->entities[i], 0);
		if(success == FALSE)
		{
			FLOG(stderr, "Error: PostQueuedCompletionStatus\n");
		}
	}
}








void System_Direvent(ecs_iter_t *it)
{
	EgWin32IOCP *dw0 = ecs_field(it, EgWin32IOCP, 1);
	_eg_direvent_t *de = ecs_field(it, _eg_direvent_t, 2);
	for (int i = 0; i < it->count; i ++)
	{
		FILE_NOTIFY_INFORMATION const * fni = (FILE_NOTIFY_INFORMATION const*)de[i].change_buf;
		while(fni)
		{
			//snprintf(out_path, EG_DIRWATCH_PATH_LENGTH, "%s %.*ls", action_to_string(fni->Action), name_len, fni->FileName);
			//printf("%s\n", out_path);
			//FIXME: Stop duplicates path
			// Convert WCHAR to CHAR
			int name_len = fni->FileNameLength / sizeof(wchar_t);
			char path[EG_DIRWATCH_PATH_LENGTH];
			snprintf(path, EG_DIRWATCH_PATH_LENGTH, "%.*ls", name_len, fni->FileName);
			ecs_entity_t evv = ecs_new(it->world, 0);
			ecs_add_pair(it->world, evv, EcsChildOf, it->entities[i]);
			ecs_set_pair(it->world, evv, EgText, EgFsPath, {path});

			switch (fni->Action)
			{
				case FILE_ACTION_ADDED:ecs_add(it->world, evv, EgFsAdded);break;
				case FILE_ACTION_REMOVED:ecs_add(it->world, evv, EgFsRemoved);break;
				case FILE_ACTION_MODIFIED:ecs_add(it->world, evv, EgFsModified);break;
				case FILE_ACTION_RENAMED_OLD_NAME:ecs_add(it->world, evv, EgFsRenamedOld);break;
				case FILE_ACTION_RENAMED_NEW_NAME:ecs_add(it->world, evv, EgFsRenamedNew);break;
			}

			*((uint8_t **)&(fni)) += fni->NextEntryOffset;
			if(fni->NextEntryOffset == 0)
			{
				fni = NULL;
				ecs_add(it->world, it->entities[i], EgDirReq);
				ecs_remove(it->world, it->entities[i], EgDirRes);
			}
		}
	}
}


void System_DirRequest(ecs_iter_t *it)
{
	_eg_direvent_t *de = ecs_field(it, _eg_direvent_t, 1);
	for (int i = 0; i < it->count; i ++)
	{
		de[i].dummy = 42;
		if(de[i].change_buf == NULL)
		{
			ecs_os_abort();
			continue;
			//ecs_os_memset(de[i].change_buf, 0, CHANGE_BUF_SIZE);
		}
		DWORD dwNotifyFilter =
			FILE_NOTIFY_CHANGE_FILE_NAME |
			FILE_NOTIFY_CHANGE_DIR_NAME |
			FILE_NOTIFY_CHANGE_ATTRIBUTES |
			FILE_NOTIFY_CHANGE_SIZE |
			FILE_NOTIFY_CHANGE_LAST_WRITE |
			FILE_NOTIFY_CHANGE_LAST_ACCESS |
			FILE_NOTIFY_CHANGE_CREATION |
			FILE_NOTIFY_CHANGE_SECURITY |
			0;
		WINBOOL bWatchSubtree = TRUE;
		ecs_os_memset_t(&de[i].overlapped, 0, OVERLAPPED);
		WINBOOL success = ReadDirectoryChangesW(de[i].file, de[i].change_buf, CHANGE_BUF_SIZE, bWatchSubtree, dwNotifyFilter, NULL, &de[i].overlapped, NULL);
		if(success == FALSE)
		{
			FLOG(stderr, "Error: ReadDirectoryChangesW\n");
		}
		ecs_remove(it->world, it->entities[i], EgDirReq);
	}
}


void System_Dirwatch_Init(ecs_iter_t *it)
{
	for (int i = 0; i < it->count; i ++)
	{
		ecs_add(it->world, it->entities[i], EgWin32IOCP);
	}
}




ECS_CTOR(_eg_direvent_t, ptr, {
    FLOG(stdout, "_eg_direvent_t::Ctor\n");
	ptr->change_buf = ecs_os_calloc(CHANGE_BUF_SIZE);
})

// The destructor should free resources.
ECS_DTOR(_eg_direvent_t, ptr, {
    FLOG(stdout, "_eg_direvent_t::Dtor\n");
	if(ptr->change_buf){ecs_os_free(ptr->change_buf);}
	if(ptr->file){CloseHandle(ptr->file);}
})

// The move hook should move resources from one location to another.
ECS_MOVE(_eg_direvent_t, dst, src, {
    FLOG(stdout, "_eg_direvent_t::Move\n");
	if(dst->change_buf){ecs_os_free(dst->change_buf);}
	if(dst->file){CloseHandle(dst->file);}
    dst->change_buf = src->change_buf;
    dst->file = src->file;
    src->change_buf = NULL;
    src->file = NULL;
})

// The copy hook should copy resources from one location to another.
ECS_COPY(_eg_direvent_t, dst, src, {
    FLOG(stdout, "_eg_direvent_t::Copy\n");
})


// This callback is used for the add, remove and set hooks. Note that the
// signature is the same as systems, triggers, observers.
void hook_callback(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	ecs_entity_t event = it->event;
	for (int i = 0; i < it->count; i ++)
	{
		ecs_entity_t e = it->entities[i];
		FLOG(stdout, "%s: %s\n", ecs_get_name(world, event), ecs_get_name(world, e));
	}
}

void EgDirwatchImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgDirwatch);
	ECS_IMPORT(world, EgWin32);
	ECS_COMPONENT_DEFINE(world, _eg_direvent_t);



	ecs_set_hooks(world, _eg_direvent_t, {
		/* Resource management hooks. These hooks should primarily be used for
		* managing memory used by the component. */
		.ctor = ecs_ctor(_eg_direvent_t),
		.move = ecs_move(_eg_direvent_t),
		.copy = ecs_copy(_eg_direvent_t),
		.dtor = ecs_dtor(_eg_direvent_t),
        /* Lifecycle hooks. These hooks should be used for application logic. */
        .on_add = hook_callback,
        .on_remove = hook_callback,
        .on_set = hook_callback
	});

	// Creates an I/O completion port that is not yet associated with a file handle, allowing association at a later time.
	ecs_system(world, {
		.entity = ecs_entity(world, {
		.name = "System_Dirwatch_Init",
		.add = { ecs_dependson(EcsOnUpdate) }
		}),
		.query.filter.terms = {
		{ .id = ecs_id(EgFsMonitorInstance), },
		{ .id = ecs_id(EgWin32IOCP), .oper=EcsNot }
		},
		.callback = System_Dirwatch_Init
	});

	// Associating an instance of an opened file handle with an I/O completion port allows 
	// a process to receive notification of the completion of asynchronous I/O operations involving that file handle.
	ecs_system(world, {
		.entity = ecs_entity(world, {
		.name = "System_Dir_Init",
		.add = { ecs_dependson(EcsOnUpdate) }
		}),
		.query.filter.terms = {
		{.id = ecs_id(EgWin32IOCP), .src.flags = EcsParent},
		{.id = ecs_pair(ecs_id(EgText), EgFsPath) },
		{.id = EgFsMonitorDir },
		{.id = ecs_id(_eg_direvent_t), .oper=EcsNot },
		},
		.callback = System_Dir_Init
	});

	// Retrieves information that describes the changes within the specified directory.
	ecs_system(world, {
		.entity = ecs_entity(world, {
		.name = "System_Direvent",
		.add = { ecs_dependson(EcsOnUpdate) }
		}),
		.query.filter.terms = {
			{.id = ecs_id(EgWin32IOCP), .src.flags = EcsParent | EcsCascade},
			{.id = ecs_id(_eg_direvent_t) },
			{.id = ecs_id(EgDirRes) },
		},
		.callback = System_Direvent
	});

	// Issue reqests to retrieves information that describes the changes within the specified directory.
	// The function does not report changes to the specified directory itself.
	ecs_system(world, {
		.entity = ecs_entity(world, {
		.name = "System_DirRequest",
		.add = { ecs_dependson(EcsOnUpdate) }
		}),
		.query.filter.terms = {
			{.id = ecs_id(_eg_direvent_t) },
			{.id = ecs_id(EgDirReq) },
		},
		.callback = System_DirRequest
	});
}

























#endif