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

#include "eg_dirwatch.h"
#include "EgFs.h"
#include "EgQuantities.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "win32error.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>




#define CHANGE_BUF_SIZE 1024*3


typedef struct
{
	HANDLE file;
	OVERLAPPED overlapped;
	uint8_t * change_buf;
	int dummy;
} _eg_direvent_t;

typedef struct
{
	HANDLE hIOCP;
	int timeout_ms;
} _eg_dirwatch_t;


ECS_TAG_DECLARE(EgDirReq);
ECS_TAG_DECLARE(EgDirRes);
ECS_COMPONENT_DECLARE(_eg_direvent_t);
ECS_COMPONENT_DECLARE(_eg_dirwatch_t);



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

void _eg_dirwatch_fini(_eg_dirwatch_t * dirwatch)
{
	if(dirwatch->hIOCP){CloseHandle(dirwatch->hIOCP);}
	ecs_os_free(dirwatch);
}
 

ecs_entity_t eg_dirwatch_add(ecs_world_t * world, ecs_entity_t scope, ecs_entity_t e, char const * path)
{
	ecs_entity_t old_scope = ecs_set_scope(world, scope);
	if(e == 0){e = ecs_new(world, _eg_direvent_t);}
	
	_eg_direvent_t * direvent = ecs_get_mut(world, e, _eg_direvent_t);
	direvent->change_buf = ecs_os_calloc(CHANGE_BUF_SIZE);

	_eg_direvent_t * ev = ecs_get_mut(world, e, _eg_direvent_t);
	ecs_os_memset_t(ev, 0, _eg_dirwatch_t);

	_eg_dirwatch_t * dirwatch = ecs_get_mut(world, scope, _eg_dirwatch_t);


	printf("Appending new direvent %jx\n", (uintmax_t)e);
	ev->file = CreateFile(path,
		 FILE_LIST_DIRECTORY,
		 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		 NULL,
		 OPEN_EXISTING,
		 FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
		 NULL);
	//e->overlapped.hEvent = CreateEvent(NULL, FALSE, 0, NULL);
	DWORD NumberOfConcurrentThreads = 0;
	HANDLE hIOCP = CreateIoCompletionPort(ev->file, dirwatch->hIOCP, (ULONG_PTR)e, NumberOfConcurrentThreads);
	assert(hIOCP == dirwatch->hIOCP);
	if((hIOCP == 0) || (hIOCP == INVALID_HANDLE_VALUE))
	{
		fprintf(stderr, "Error: CreateIoCompletionPort\n");
		goto fail;
	}
	WINBOOL success = PostQueuedCompletionStatus(hIOCP, 0, (ULONG_PTR)e, 0);
	if(success == FALSE)
	{
		fprintf(stderr, "Error: PostQueuedCompletionStatus\n");
		goto fail;
	}
	return e;
	
	ecs_set_scope(world, old_scope);

fail:
	_eg_dirwatch_fini(dirwatch);
	return 0;
}










ecs_entity_t eg_dirwatch_init(ecs_world_t * world, ecs_entity_t e)
{
	if(e == 0){e = ecs_new(world, _eg_dirwatch_t);}
	_eg_dirwatch_t * dirwatch = ecs_get_mut(world, e, _eg_dirwatch_t);
	ecs_os_memset_t(dirwatch, 0, _eg_dirwatch_t);
	DWORD NumberOfConcurrentThreads = 0;
	dirwatch->hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)0, NumberOfConcurrentThreads);
	if((dirwatch->hIOCP == 0) || (dirwatch->hIOCP == INVALID_HANDLE_VALUE))
	{
		fprintf(stderr, "Error: CreateIoCompletionPort\n");
		win32_PrintCSBackupAPIErrorMessage(GetLastError());
		return 0;
	}
	return e;
}







void System_Dirwatcher(ecs_iter_t *it)
{
	_eg_dirwatch_t *dw = ecs_field(it, _eg_dirwatch_t, 1);
	for (int i = 0; i < it->count; i ++)
	{
		DWORD NumberOfBytesTransferred;
		ULONG_PTR CompletionKey;
		OVERLAPPED *Overlapped;
		DWORD dwMilliseconds = (dw[i].timeout_ms < 0) ? INFINITE : (DWORD)dw[i].timeout_ms;
		WINBOOL got_packet = GetQueuedCompletionStatus(dw[i].hIOCP, &NumberOfBytesTransferred, &CompletionKey, &Overlapped, dwMilliseconds);
		if(got_packet == FALSE)
		{
			// no new packet - done   
			continue;;
		}
		//printf("Got packet! %i, %p\n", (int)NumberOfBytesTransferred, (void*)CompletionKey);
		ecs_entity_t ev = (ecs_entity_t)CompletionKey;
		if(NumberOfBytesTransferred > 0)
		{
			_eg_direvent_t * direvent = ecs_get_mut(it->world, ev, _eg_direvent_t);
			FILE_NOTIFY_INFORMATION const * fni = (FILE_NOTIFY_INFORMATION const *)direvent->change_buf;
			ecs_add(it->world, ev, EgDirRes);
		}
		else
		{
			ecs_add(it->world, ev, EgDirReq);
		}
	}
}




void System_Direvent(ecs_iter_t *it)
{
	_eg_dirwatch_t *dw0 = ecs_field(it, _eg_dirwatch_t, 1);
	_eg_direvent_t *de = ecs_field(it, _eg_direvent_t, 2);
	for (int i = 0; i < it->count; i ++)
	{
		FILE_NOTIFY_INFORMATION const * fni = (FILE_NOTIFY_INFORMATION const*)de[i].change_buf;
		while(fni)
		{
			// Convert WCHAR to CHAR
			int name_len = fni->FileNameLength / sizeof(wchar_t);
			//snprintf(out_path, EG_DIRWATCH_PATH_LENGTH, "%s %.*ls", action_to_string(fni->Action), name_len, fni->FileName);
			//printf("%s\n", out_path);
			//FIXME: filepath issuess
			char path[EG_DIRWATCH_PATH_LENGTH];
			snprintf(path, EG_DIRWATCH_PATH_LENGTH, "%.*ls", name_len, fni->FileName);
			ecs_entity_t evv = ecs_new(it->world, 0);
			ecs_add_pair(it->world, evv, EcsChildOf, it->entities[i]);
			ecs_set_pair(it->world, evv, EgText, EgFsPath, {path});
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
			fprintf(stderr, "Error: ReadDirectoryChangesW\n");
		}
		ecs_remove(it->world, it->entities[i], EgDirReq);
	}
}



void eg_dirwatch_bootstrap(ecs_world_t * world)
{
	ECS_TAG_DEFINE(world, EgDirReq);
	ECS_TAG_DEFINE(world, EgDirRes);
	ECS_COMPONENT_DEFINE(world, _eg_direvent_t);
	ECS_COMPONENT_DEFINE(world, _eg_dirwatch_t);

	ecs_system(world, {
	.entity = ecs_entity(world, {
	.name = "System_Dirwatcher",
	.add = { ecs_dependson(EcsOnUpdate) }
	}),
	.query.filter.terms = {
	{ .id = ecs_id(_eg_dirwatch_t), }
	},
	.callback = System_Dirwatcher
	});


	ecs_system(world, {
	.entity = ecs_entity(world, {
	.name = "System_Direvent",
	.add = { ecs_dependson(EcsOnUpdate) }
	}),
	.query.filter.terms = {
		{.id = ecs_id(_eg_dirwatch_t), .src.flags = EcsParent | EcsCascade},
		{.id = ecs_id(_eg_direvent_t) },
		{.id = ecs_id(EgDirRes) },
	},
	.callback = System_Direvent
	});

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