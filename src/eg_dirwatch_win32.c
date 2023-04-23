/*
https://gist.github.com/nickav/a57009d4fcc3b527ed0f5c9cf30618f8
http://old.zaynar.co.uk/cppdoc/latest/projects/lib.sysdep.win/wdir_watch.cpp.html
https://github.com/tresorit/rdcfswatcherexample/blob/master/rdc_fs_watcher.cpp
https://github.com/0ad/0ad/blob/master/source/lib/sysdep/os/win/wiocp.cpp
https://github.com/0ad/0ad/blob/master/source/lib/sysdep/os/win/wdir_watch.cpp
https://stackoverflow.com/questions/339776/asynchronous-readdirectorychangesw
*/

#include "eg_dirwatch.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "win32error.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "flecs.h"


#define CHANGE_BUF_SIZE 1024*3


typedef struct
{
	HANDLE file;
	OVERLAPPED overlapped;
	uint8_t change_buf[CHANGE_BUF_SIZE];
} _eg_direvent_t;

typedef struct
{
	eg_dirwatch_t public;
	HANDLE hIOCP;
	ecs_vec_t v;
	_eg_direvent_t * lastevent;
	FILE_NOTIFY_INFORMATION const* fni;
} _eg_dirwatch_t;



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
	for (int32_t i = 0; i < dirwatch->v.count; i++)
	{
		_eg_direvent_t *v = ecs_vec_get_t(&dirwatch->v, _eg_direvent_t, i);
		if(v->file){CloseHandle(v->file);}
	}
	if(dirwatch->hIOCP){CloseHandle(dirwatch->hIOCP);}
	ecs_vec_fini_t(NULL, &dirwatch->v, _eg_direvent_t);
	ecs_os_free(dirwatch);
}
 

_eg_dirwatch_t * _eg_dirwatch_init(eg_dirwatch_desc_t * desc)
{
	(void)desc;
	_eg_dirwatch_t * dirwatch = ecs_os_calloc_t(_eg_dirwatch_t);
	DWORD NumberOfConcurrentThreads = 0;
	dirwatch->hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)0, NumberOfConcurrentThreads);
	if((dirwatch->hIOCP == 0) || (dirwatch->hIOCP == INVALID_HANDLE_VALUE))
	{
		fprintf(stderr, "Error: CreateIoCompletionPort\n");
		win32_PrintCSBackupAPIErrorMessage(GetLastError());
		return NULL;
	}
	ecs_vec_init_t(NULL, &dirwatch->v, _eg_direvent_t, 10);
	return dirwatch;
}




void _eg_dirwatch_add(_eg_dirwatch_t * dirwatch, char const * path)
{
	_eg_direvent_t * e = ecs_vec_append_t(NULL, &dirwatch->v, _eg_direvent_t);
	printf("Appending new direvent %p\n", e);
	e->file = CreateFile(path,
		 FILE_LIST_DIRECTORY,
		 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		 NULL,
		 OPEN_EXISTING,
		 FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
		 NULL);
	//e->overlapped.hEvent = CreateEvent(NULL, FALSE, 0, NULL);
	DWORD NumberOfConcurrentThreads = 0;
	HANDLE hIOCP = CreateIoCompletionPort(e->file, dirwatch->hIOCP, (ULONG_PTR)e, NumberOfConcurrentThreads);
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
	return;
fail:
	_eg_dirwatch_fini(dirwatch);
}



// if a packet is pending, extract its events, post them in the queue and
// re-issue its watch.
int _eg_dirwatch_pull(_eg_dirwatch_t * dirwatch, int32_t timeout_ms, char out_path[EG_DIRWATCH_PATH_LENGTH])
{
	DWORD NumberOfBytesTransferred;
	if(dirwatch->lastevent == NULL)
	{
		ULONG_PTR CompletionKey;
		OVERLAPPED *Overlapped;
		DWORD dwMilliseconds = (timeout_ms < 0) ? INFINITE : (DWORD)timeout_ms;
		WINBOOL got_packet = GetQueuedCompletionStatus(dirwatch->hIOCP, &NumberOfBytesTransferred, &CompletionKey, &Overlapped, dwMilliseconds);
		if(got_packet == FALSE)
		{
			// no new packet - done   
			return 0;
		}
		//printf("Got packet! %i, %p\n", (int)NumberOfBytesTransferred, (void*)CompletionKey);
		dirwatch->lastevent = (_eg_direvent_t*)CompletionKey;
		dirwatch->fni = NULL;
		if(NumberOfBytesTransferred > 0)
		{
			dirwatch->fni = (FILE_NOTIFY_INFORMATION const*)dirwatch->lastevent->change_buf;
		}
	}

	assert(dirwatch->lastevent);
	if(dirwatch->fni)
	{
		// Convert WCHAR to CHAR
		int name_len = dirwatch->fni->FileNameLength / sizeof(wchar_t);
		snprintf(out_path, EG_DIRWATCH_PATH_LENGTH, "%s %.*ls", action_to_string(dirwatch->fni->Action), name_len, dirwatch->fni->FileName);
		//printf("%s", out_path);
		*((uint8_t **)&(dirwatch->fni)) += dirwatch->fni->NextEntryOffset;
		if(dirwatch->fni->NextEntryOffset == 0)
		{
			dirwatch->fni = NULL;
		}
		return 1;
	}

	{
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
		memset(&dirwatch->lastevent->overlapped, 0, sizeof(dirwatch->lastevent->overlapped));
		WINBOOL success = ReadDirectoryChangesW(dirwatch->lastevent->file, dirwatch->lastevent->change_buf, CHANGE_BUF_SIZE, bWatchSubtree, dwNotifyFilter, NULL, &dirwatch->lastevent->overlapped, NULL);
		if(success == FALSE)
		{
			fprintf(stderr, "Error: ReadDirectoryChangesW\n");
			goto fail;
		}
	}
	dirwatch->lastevent = NULL;
	return 0;

fail:
	_eg_dirwatch_fini(dirwatch);
	return -1;
}









void eg_dirwatch_fini(eg_dirwatch_t * dirwatch)
{
	_eg_dirwatch_fini((_eg_dirwatch_t*)dirwatch);
}
eg_dirwatch_t * eg_dirwatch_init(eg_dirwatch_desc_t * desc)
{
	return (eg_dirwatch_t*)_eg_dirwatch_init(desc);
}

void eg_dirwatch_add(eg_dirwatch_t * dirwatch, char const * path)
{
	_eg_dirwatch_add((_eg_dirwatch_t*)dirwatch, path);
}


int eg_dirwatch_pull(eg_dirwatch_t * dirwatch, int32_t timeout_ms, char out_path[EG_DIRWATCH_PATH_LENGTH])
{
	return _eg_dirwatch_pull((_eg_dirwatch_t*)dirwatch, timeout_ms, out_path);
}