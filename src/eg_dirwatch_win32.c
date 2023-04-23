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
	eg_dirwatch_t public;
// Private:
	HANDLE file;
	OVERLAPPED overlapped;
	uint8_t change_buf[CHANGE_BUF_SIZE];
	FILE_NOTIFY_INFORMATION const* fni;
} _eg_direvent_t;

// Private
typedef struct
{
	eg_dirwatch_t public;
	HANDLE hIOCP;
	ecs_vec_t v;
	_eg_direvent_t * lastevent;
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







int eg_dirwatch_size()
{
	return sizeof(_eg_dirwatch_t);
}


void queue_next_event(_eg_direvent_t * e)
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
	HANDLE hDirectory = e->file;
	LPVOID lpBuffer = e->change_buf;
	DWORD nBufferLength = CHANGE_BUF_SIZE;
	WINBOOL bWatchSubtree = TRUE;
	LPDWORD lpBytesReturned = NULL;
	memset(&e->overlapped, 0, sizeof(e->overlapped));
	BOOL success = ReadDirectoryChangesW(hDirectory, lpBuffer, nBufferLength, bWatchSubtree, dwNotifyFilter, lpBytesReturned, &e->overlapped, NULL);
	assert(success); //FIXME: Handle this error
}

 

void _eg_dirwatch_init(_eg_dirwatch_t * dirwatch)
{
	memset(dirwatch, 0, sizeof(_eg_dirwatch_t));
	DWORD NumberOfConcurrentThreads = 0;
	dirwatch->hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)0, NumberOfConcurrentThreads);
	if((dirwatch->hIOCP == 0) || (dirwatch->hIOCP == INVALID_HANDLE_VALUE))
	{
		fprintf(stderr, "Error: CreateIoCompletionPort\n");
		win32_PrintCSBackupAPIErrorMessage(GetLastError());
		return;
	}
	ecs_vec_init_t(NULL, &dirwatch->v, _eg_direvent_t, 10);
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
	PostQueuedCompletionStatus(hIOCP, 0, (ULONG_PTR)e, 0);
	return;
fail:
	if(e->file)
	{
		CloseHandle(e->file);
	}
}









/*
int eg_dirwatch_wait_event(eg_dirwatch_t * monitor, int32_t timeout_ms, char out_path[EG_FS_PATH_LENGTH])
{
	assert(monitor);
	assert(out_path);
	eg_dirwatch_private_t * mon = (eg_dirwatch_private_t *)monitor;
	if(mon->event == NULL)
	{
		// Waits until the specified object is in the signaled state or the time-out interval elapses.
		DWORD dwMilliseconds = (timeout_ms == -1) ? INFINITE : (DWORD)timeout_ms;
		DWORD result = WaitForSingleObject(mon->overlapped.hEvent, dwMilliseconds);
		if (result != WAIT_OBJECT_0)
		{
			// No event found
			return 0;
		}
		// Retrieves the results of an overlapped operation on the specified file, named pipe, or communications device.
		DWORD bytes_transferred;
		GetOverlappedResult(mon->file, &mon->overlapped, &bytes_transferred, FALSE);
		mon->event = (FILE_NOTIFY_INFORMATION *)mon->change_buf;
	}
	//printf("%s: %.*ls\n", action_to_string(mon->event->Action), (int)name_len, mon->event->FileName);

	// Convert WCHAR to CHAR
	int name_len = mon->event->FileNameLength / sizeof(wchar_t);
	snprintf(out_path, EG_FS_PATH_LENGTH, "%s %.*ls", action_to_string(mon->event->Action), name_len, mon->event->FileName);
	//snprintf(path, EG_FS_PATH_LENGTH, "%.*ls", name_len, mon->event->FileName);

	// Are there more events to handle?
	assert(mon->event);
	if (mon->event->NextEntryOffset)
	{
		*((uint8_t **)&(mon->event)) += mon->event->NextEntryOffset;
	}
	else
	{
		// At this point the existing request has completed.
		// Only issue a new request after the existing request has completed.
		queue_next_event(mon);
		mon->event = NULL;
	}

	return 1;
}
*/






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
		dirwatch->lastevent->fni = NULL;
		if(NumberOfBytesTransferred > 0)
		{
			dirwatch->lastevent->fni = (FILE_NOTIFY_INFORMATION const*)dirwatch->lastevent->change_buf;
		}
	}

	assert(dirwatch->lastevent);
	if(dirwatch->lastevent->fni)
	{
		// Convert WCHAR to CHAR
		int name_len = dirwatch->lastevent->fni->FileNameLength / sizeof(wchar_t);
		snprintf(out_path, EG_DIRWATCH_PATH_LENGTH, "%s %.*ls", action_to_string(dirwatch->lastevent->fni->Action), name_len, dirwatch->lastevent->fni->FileName);
		//printf("%s", out_path);
		*((uint8_t **)&(dirwatch->lastevent->fni)) += dirwatch->lastevent->fni->NextEntryOffset;
		if(dirwatch->lastevent->fni->NextEntryOffset == 0)
		{
			dirwatch->lastevent->fni = NULL;
		}
		return 1;
	}

	queue_next_event(dirwatch->lastevent);
	dirwatch->lastevent = NULL;

	return 0;
}










void eg_dirwatch_init(eg_dirwatch_t * dirwatch)
{
	_eg_dirwatch_init((_eg_dirwatch_t*)dirwatch);
}

void eg_dirwatch_add(eg_dirwatch_t * dirwatch, char const * path)
{
	_eg_dirwatch_add((_eg_dirwatch_t*)dirwatch, path);
}


int eg_dirwatch_pull(eg_dirwatch_t * dirwatch, int32_t timeout_ms, char out_path[EG_DIRWATCH_PATH_LENGTH])
{
	return _eg_dirwatch_pull((_eg_dirwatch_t*)dirwatch, timeout_ms, out_path);
}