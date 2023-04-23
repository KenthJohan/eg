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

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>


#define CHANGE_BUF_SIZE 1024*3



typedef struct
{
	eg_dirwatch_t public;
	HANDLE file;
	OVERLAPPED overlapped;
	uint8_t change_buf[CHANGE_BUF_SIZE];
	FILE_NOTIFY_INFORMATION *event;
	HANDLE hIOCP;
} eg_dirwatch_private_t;



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





// Display error message text, given an error code.
// Typically, the parameter passed to this function is retrieved
// from GetLastError().
void PrintCSBackupAPIErrorMessage(DWORD dwErr)
{
	LPTSTR a;
    char   wszMsgBuff[512];  // Buffer for text.

    DWORD   dwChars;  // Number of chars returned.

    // Try to get the message from the system errors.
    dwChars = FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM |
                             FORMAT_MESSAGE_IGNORE_INSERTS,
                             NULL,
                             dwErr,
                             0,
                             wszMsgBuff,
                             512,
                             NULL );

    if (0 == dwChars)
    {
        // The error code did not exist in the system errors.
        // Try Ntdsbmsg.dll for the error code.

        HINSTANCE hInst;

        // Load the library.
        hInst = LoadLibrary("Ntdsbmsg.dll");
        if ( NULL == hInst )
        {
            printf("cannot load Ntdsbmsg.dll\n");
            exit(1);  // Could 'return' instead of 'exit'.
        }

        // Try getting message text from ntdsbmsg.
        dwChars = FormatMessageA( FORMAT_MESSAGE_FROM_HMODULE |
                                 FORMAT_MESSAGE_IGNORE_INSERTS,
                                 hInst,
                                 dwErr,
                                 0,
                                 wszMsgBuff,
                                 512,
                                 NULL );

        // Free the library.
        FreeLibrary( hInst );

    }

    // Display the error message, or generic text if not found.
    printf("Error value: %ld Message: %s\n",
            dwErr,
            dwChars ? wszMsgBuff : "Error message not found." );

}











int eg_dirwatch_size()
{
	return sizeof(eg_dirwatch_private_t);
}


void queue_next_event(eg_dirwatch_private_t * mon)
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
	HANDLE hDirectory = mon->file;
	LPVOID lpBuffer = mon->change_buf;
	DWORD nBufferLength = CHANGE_BUF_SIZE;
	WINBOOL bWatchSubtree = TRUE;
	LPDWORD lpBytesReturned = NULL;
	memset(&mon->overlapped, 0, sizeof(mon->overlapped));
	LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine = NULL;
	BOOL success = ReadDirectoryChangesW(hDirectory, lpBuffer, nBufferLength, bWatchSubtree, dwNotifyFilter, lpBytesReturned, &mon->overlapped, lpCompletionRoutine);
	assert(success); //FIXME: Handle this error
}

 

void eg_dirwatch_init(eg_dirwatch_t * dirwatch, char const * path)
{
	eg_dirwatch_private_t * internal = (eg_dirwatch_private_t *)dirwatch;
	memset(internal, 0, sizeof(eg_dirwatch_private_t));
	internal->file = CreateFile(path,
		 FILE_LIST_DIRECTORY,
		 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		 NULL,
		 OPEN_EXISTING,
		 FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
		 NULL);
	//internal->overlapped.hEvent = CreateEvent(NULL, FALSE, 0, NULL);
	internal->event = NULL;

	// Test:
	{
		HANDLE FileHandle = internal->file;
		HANDLE ExistingCompletionPort = 0;
		ULONG_PTR CompletionKey = (uintptr_t)internal;
		DWORD NumberOfConcurrentThreads = 0;
		internal->hIOCP = CreateIoCompletionPort(FileHandle, ExistingCompletionPort, CompletionKey, NumberOfConcurrentThreads);
		if((internal->hIOCP == 0) || (internal->hIOCP == INVALID_HANDLE_VALUE))
		{
			printf("CreateIoCompletionPort!\n");
			goto fail;
		}
		PostQueuedCompletionStatus(internal->hIOCP, 0, CompletionKey, 0);
	}

	//queue_next_event(internal);
	return;
fail:
	if(internal->file)
	{
		CloseHandle(internal->file);
	}
	PrintCSBackupAPIErrorMessage(GetLastError());
}


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







// if a packet is pending, extract its events, post them in the queue and
// re-issue its watch.
void eg_dirwatch_get_packet(eg_dirwatch_t * dirwatch)
{
	eg_dirwatch_private_t * internal = (eg_dirwatch_private_t *)dirwatch;
	// poll for change notifications from all pending watches

	
	{
		HANDLE CompletionPort = internal->hIOCP;
		DWORD NumberOfBytesTransferred;
		ULONG_PTR CompletionKey;
		OVERLAPPED *Overlapped;
		DWORD dwMilliseconds = 0;
		WINBOOL got_packet = GetQueuedCompletionStatus(CompletionPort, &NumberOfBytesTransferred, &CompletionKey, &Overlapped, dwMilliseconds);
		if(got_packet == FALSE)
		{
			// no new packet - done   a
			return;
		}
		printf("Got packet! %i, %p === %p\n", (int)NumberOfBytesTransferred, internal, (void*)CompletionKey);
		queue_next_event(internal);
	}


	/*

	// this is an actual packet, not just a kickoff for issuing the watch.
	// extract the events and push them onto AppState's queue.
	if(bytes_transferred != 0)
	extract_events(w);


	// (re-)issue change notification request.
	// it's safe to reuse Watch.change_buf, because we copied out all events.
	const DWORD filter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
	FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE |
	FILE_NOTIFY_CHANGE_CREATION;
	const DWORD buf_size = sizeof(w->change_buf);
	memset(&w->ovl, 0, sizeof(w->ovl));
	BOOL watch_subtree = TRUE;
	// much faster than watching every dir separately. see dir_add_watch.
	BOOL ok = ReadDirectoryChangesW(w->hDir, w->change_buf, buf_size, watch_subtree, filter, &w->dummy_nbytes, &w->ovl, 0);
	WARN_IF_FALSE(ok);
	*/
}







