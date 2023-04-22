/*
https://gist.github.com/nickav/a57009d4fcc3b527ed0f5c9cf30618f8
*/

#include "eg_fs.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>


#define CHANGE_BUF_SIZE 1024*3



typedef struct
{
	eg_fs_monitor_t public;
	HANDLE file;
	OVERLAPPED overlapped;
	uint8_t change_buf[CHANGE_BUF_SIZE];
	FILE_NOTIFY_INFORMATION *event;
} eg_fs_monitor_win32_t;



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




int eg_fs_monitor_size()
{
	return sizeof(eg_fs_monitor_win32_t);
}


void queue_next_event(eg_fs_monitor_win32_t * mon)
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
	BOOL success = ReadDirectoryChangesW(mon->file, mon->change_buf, CHANGE_BUF_SIZE, TRUE, dwNotifyFilter, NULL, &mon->overlapped, NULL);
	assert(success); //FIXME: Handle this error
}



eg_fs_monitor_t * eg_fs_monitor_init(char const * path, void * memory)
{
	eg_fs_monitor_win32_t * ptr = (eg_fs_monitor_win32_t *)memory;
	ptr->file = CreateFile(path,
		 FILE_LIST_DIRECTORY,
		 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		 NULL,
		 OPEN_EXISTING,
		 FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
		 NULL);
	ptr->overlapped.hEvent = CreateEvent(NULL, FALSE, 0, NULL);
	ptr->event = NULL;
	queue_next_event(ptr);
	return (eg_fs_monitor_t*)ptr;
}

//aaaa6


int eg_fs_pull_changes(eg_fs_monitor_t * monitor, int32_t delay, char out_path[EG_FS_PATH_LENGTH])
{
	assert(monitor);
	assert(out_path);

	eg_fs_monitor_win32_t * mon = (eg_fs_monitor_win32_t *)monitor;
	if(mon->event == NULL)
	{
		//printf("WaitForSingleObject\n");
		DWORD dwMilliseconds = (delay == -1) ? INFINITE : (DWORD)delay;
		DWORD result = WaitForSingleObject(mon->overlapped.hEvent, dwMilliseconds);
		if (result != WAIT_OBJECT_0) {return 0;}
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
		queue_next_event(mon);
		mon->event = NULL;
	}

	return 1;
}

