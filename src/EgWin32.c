#include "EgWin32.h"
#include "EgFs.h"
#include "EgStr.h"
#include "eg_win32.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <accctrl.h>
#include <aclapi.h>

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>


#define CHANGE_BUF_SIZE 1024*3

typedef struct
{
	int32_t timeout_ms;
	int32_t NumberOfBytesTransferred;
} EgWin32IOCP;

typedef struct
{
	HANDLE handle;
} EgWin32Handle;

extern ECS_COMPONENT_DECLARE(EgWin32IOCP);
extern ECS_COMPONENT_DECLARE(EgWin32Handle);


ECS_COMPONENT_DECLARE(EgWin32IOCP);
ECS_COMPONENT_DECLARE(EgWin32Handle);


#define FLOG(...) fprintf(__VA_ARGS__)




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



void request_notification(HANDLE * h, void * data)
{
	//EgWin32Handle const * hh = ecs_get(world, ev, EgWin32Handle);
	//EgBuffer const * bb = ecs_get(world, ev, EgBuffer);
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
	OVERLAPPED overlapped = {0};
	// Issue reqests to retrieves information that describes the changes within the specified directory.
	// The function does not report changes to the specified directory itself.
	FLOG(stdout, "ReadDirectoryChangesW\n");
	WINBOOL success = ReadDirectoryChangesW(h, data, CHANGE_BUF_SIZE, bWatchSubtree, dwNotifyFilter, NULL, &overlapped, NULL);
	if(success == FALSE)
	{
		FLOG(stderr, "Error: ReadDirectoryChangesW\n");
		eg_win32_PrintCSBackupAPIErrorMessage(GetLastError());
	}
}

void process_packet(ecs_world_t * world, ecs_entity_t ev, int32_t NumberOfBytesTransferred)
{
	assert(ecs_has(world, ev, EgWin32Handle));
	assert(ecs_has(world, ev, EgFsMonitorDir));
	assert(ecs_has(world, ev, EgBuffer)); // This contains the pointer to packet data buffer
	EgBuffer const * bb = ecs_get(world, ev, EgBuffer);
	EgWin32Handle const * hh = ecs_get(world, ev, EgWin32Handle);
	assert(bb->data);
	assert(hh->handle);
	if(NumberOfBytesTransferred > 0)
	{
		//ecs_set(it->world, ev, EgWin32DirNotification, {NumberOfBytesTransferred});
		FILE_NOTIFY_INFORMATION const * fni = (FILE_NOTIFY_INFORMATION const*)bb->data;
		while(fni)
		{
			//TODO: Stop duplicates path
			// Convert WCHAR to CHAR
			char pathbuf[MAX_PATH];
			int name_len = fni->FileNameLength / sizeof(wchar_t);
			snprintf(pathbuf, MAX_PATH, "%.*ls", name_len, fni->FileName);
			FLOG(stdout, "FILE_NOTIFY_INFORMATION %s %.*ls\n", action_to_string(fni->Action), name_len, fni->FileName);
			//snprintf(out_path, EG_DIRWATCH_PATH_LENGTH, "%s %.*ls", action_to_string(fni->Action), name_len, fni->FileName);
			ecs_entity_t evv = ecs_new(world, 0);
			ecs_add_pair(world, evv, EcsChildOf, ev);
			ecs_set_pair(world, evv, EgText, EgFsPath, {pathbuf});
			switch (fni->Action)
			{
				case FILE_ACTION_ADDED:ecs_add(world, evv, EgFsAdded);break;
				case FILE_ACTION_REMOVED:ecs_add(world, evv, EgFsRemoved);break;
				case FILE_ACTION_MODIFIED:ecs_add(world, evv, EgFsModified);break;
				case FILE_ACTION_RENAMED_OLD_NAME:ecs_add(world, evv, EgFsRenamedOld);break;
				case FILE_ACTION_RENAMED_NEW_NAME:ecs_add(world, evv, EgFsRenamedNew);break;
			}
			*((uint8_t **)&(fni)) += fni->NextEntryOffset;
			if(fni->NextEntryOffset == 0)
			{
				fni = NULL;
			}
		}
	}
	// We are only getting one dir notification if we stop making request.
	// To get dir notification again we need to request again:
	// TODO: Unwrapping this function here causes strange error:
	request_notification(hh->handle, bb->data);
}







void System_Dirwatcher(ecs_iter_t *it)
{
	EgWin32Handle *h = ecs_field(it, EgWin32Handle, 1);
	EgWin32IOCP *iocp = ecs_field(it, EgWin32IOCP, 2);
	for (int i = 0; i < it->count; i ++)
	{
		DWORD NumberOfBytesTransferred;
		ULONG_PTR CompletionKey;
		OVERLAPPED *Overlapped;
		DWORD dwMilliseconds = (iocp[i].timeout_ms < 0) ? INFINITE : (DWORD)iocp[i].timeout_ms;
		// Attempts to dequeue an I/O completion packet.
		// Retrieves information that describes the changes within the specified directory.
		WINBOOL got_packet = GetQueuedCompletionStatus(h[i].handle, &NumberOfBytesTransferred, &CompletionKey, &Overlapped, dwMilliseconds);
		iocp[i].NumberOfBytesTransferred = NumberOfBytesTransferred;
		if(got_packet == FALSE)
		{ 
			continue;
		}
		ecs_entity_t ev = (ecs_entity_t)CompletionKey;
		FLOG(stdout, "GetQueuedCompletionStatus: NumberOfBytesTransferred:%i, e:%ju\n", (int)NumberOfBytesTransferred, (uintmax_t)ev);
		process_packet(it->world, ev, (int32_t)NumberOfBytesTransferred);
	}
}






void System_Create_Watcher(ecs_iter_t *it)
{
	EgWin32Handle * h0 = ecs_field(it, EgWin32Handle, 1); // Parent
	EgWin32IOCP * iocp0 = ecs_field(it, EgWin32IOCP, 2); // Parent
	EgText *path = ecs_field(it, EgText, 3);
	EgFsMonitorDir *m = ecs_field(it, EgFsMonitorDir, 4);
	for (int i = 0; i < it->count; i ++)
	{
		iocp0->timeout_ms = 0;
		ecs_entity_t e = it->entities[i];
		FLOG(stdout, "CreateFile %ju\n", (uintmax_t)e);
		HANDLE * h = CreateFile(path[i].value,
			 FILE_LIST_DIRECTORY,
			 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			 NULL,
			 OPEN_EXISTING,
			 FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
			 NULL);
		ecs_set(it->world, e, EgWin32Handle, {h});
		EgBuffer * b = ecs_get_mut(it->world, e, EgBuffer);
		b->data = ecs_os_calloc(CHANGE_BUF_SIZE);
		DWORD NumberOfConcurrentThreads = 0;
		// Associating an instance of an opened file handle with an I/O completion port allows 
		// a process to receive notification of the completion of asynchronous I/O operations involving that file handle.
		h0[i].handle = CreateIoCompletionPort(h, h0[i].handle, (ULONG_PTR)e, NumberOfConcurrentThreads);
		if((h0[i].handle == 0) || (h0[i].handle == INVALID_HANDLE_VALUE))
		{
			eg_win32_PrintCSBackupAPIErrorMessage(GetLastError());
			FLOG(stderr, "Error: CreateIoCompletionPort\n");
		}
		WINBOOL success = PostQueuedCompletionStatus(h0[i].handle, 0, (ULONG_PTR)e, 0);
		if(success == FALSE)
		{
			eg_win32_PrintCSBackupAPIErrorMessage(GetLastError());
			FLOG(stderr, "Error: PostQueuedCompletionStatus\n");
		}
	}
}


void System_Dirwatch_Init(ecs_iter_t *it)
{
   for (int i = 0; i < it->count; i ++)
   {
      ecs_add(it->world, it->entities[i], EgWin32Handle);
      ecs_add(it->world, it->entities[i], EgWin32IOCP);
   }
}



int replacechar(char *str, char orig, char rep)
{
	char *ix = str;
	int n = 0;
	while((ix = strchr(ix, orig)) != NULL)
	{
		*ix++ = rep;
		n++;
	}
	return n;
}


BOOL GetLastWriteTime(HANDLE hFile, LPTSTR lpszString, DWORD dwSize)
{
    FILETIME ftCreate, ftAccess, ftWrite;
    SYSTEMTIME stUTC, stLocal;
    DWORD dwRet;

    // Retrieve the file times for the file.
    if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
        return FALSE;

    // Convert the last-write time to local time.
    FileTimeToSystemTime(&ftWrite, &stUTC);
    SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

    // Build a string showing the date and time.
    dwRet = snprintf(lpszString, dwSize, 
        TEXT("%02d/%02d/%d  %02d:%02d"),
        stLocal.wMonth, stLocal.wDay, stLocal.wYear,
        stLocal.wHour, stLocal.wMinute);

    if( S_OK == dwRet )
        return TRUE;
    else return FALSE;
}




void add_time2(ecs_world_t * world, ecs_entity_t e, ecs_entity_t tag, FILETIME * time)
{
	SYSTEMTIME stUTC, stLocal;
	FileTimeToSystemTime(time, &stUTC);
	SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
	char buf[128];
	snprintf(buf, 128, "%02d-%02d-%02d  %02d:%02d:%02d", stLocal.wYear, stLocal.wMonth, stLocal.wDay, stLocal.wHour, stLocal.wMinute, stLocal.wSecond);
	ecs_set_pair(world, e, EgText, tag, {buf});
}

void add_time(ecs_world_t * world, ecs_entity_t e, char const * filename)
{
	HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if((hFile == 0) || (hFile == INVALID_HANDLE_VALUE))
	{
		eg_win32_PrintCSBackupAPIErrorMessage(GetLastError());
		FLOG(stderr, "Error: CreateFile\n");
		return;
	}
	FILETIME ftCreate, ftAccess, ftWrite;
	if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
	{
		eg_win32_PrintCSBackupAPIErrorMessage(GetLastError());
		FLOG(stderr, "Error: GetFileTime\n");
		return;
	}
	add_time2(world, e, EgFsCreated, &ftCreate);
	add_time2(world, e, EgFsAccessed, &ftAccess);
	add_time2(world, e, EgFsModified, &ftWrite);
	CloseHandle(hFile);
}



void System_List_Files(ecs_iter_t *it)
{
	EgText *path = ecs_field(it, EgText, 1);
	ecs_entity_t old_scope = ecs_get_scope(it->world);
	for (int i = 0; i < it->count; i ++)
	{
		ecs_entity_t e = it->entities[i];
		WIN32_FIND_DATA ffd;
		LARGE_INTEGER filesize;
		char buf1[128];

		// Find files by adding a wildcard after the dir path:
		snprintf(buf1, 128, "%s/*", path[i].value);
		HANDLE hFind = FindFirstFile(buf1, &ffd);
		if(hFind == INVALID_HANDLE_VALUE)
		{
			eg_win32_PrintCSBackupAPIErrorMessage(GetLastError());
			FLOG(stderr, "Error: FindFirstFile\n"); 
			continue;
		}

		// Add new files under this entity later on:
		ecs_set_scope(it->world, e);
		do
		{
			// Ignore "." and ".." dirs:
			if(ffd.cFileName[0] == '.'){continue;}

			// Set full path in in EgFsPath and relative path in entity name:
			snprintf(buf1, 128, "%s/%s", path[i].value, ffd.cFileName);



			// flecs uses dot for scoping so replace it with comma:
			replacechar(ffd.cFileName, '.', ',');
			ecs_entity_t ee = ecs_new_entity(it->world, ffd.cFileName);
			ecs_set_pair(it->world, ee, EgText, EgFsPath, {buf1});


			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				printf("  %s   <DIR>\n", ffd.cFileName);
				ecs_add(it->world, ee, EgFsDir);
				ecs_doc_set_color(it->world, ee, "#ed930c");
			}
			else
			{
				filesize.LowPart = ffd.nFileSizeLow;
				filesize.HighPart = ffd.nFileSizeHigh;
				printf("  %s   %ji bytes\n", ffd.cFileName, (intmax_t)filesize.QuadPart);
				assert(filesize.QuadPart <= INTMAX_MAX);
				assert(filesize.QuadPart >= INTMAX_MIN);
				assert(filesize.QuadPart <= INT64_MAX);
				assert(filesize.QuadPart >= INT64_MIN);
				ecs_set(it->world, ee, EgFsSize, {(int64_t)filesize.QuadPart});
				ecs_add(it->world, ee, EgFsFile);
				ecs_doc_set_color(it->world, ee, "#0a0eff");
			}

			add_time(it->world, ee, buf1);

		}
		while (FindNextFile(hFind, &ffd) != 0);
		FindClose(hFind);
	}

	// Remove EgFsList command so this system is only ran once:
	for (int i = 0; i < it->count; i ++)
	{
		ecs_entity_t e = it->entities[i];
		ecs_remove(it->world, e, EgFsList);
	}

	// Restore scope:
	ecs_set_scope(it->world, old_scope);
}


//https://stackoverflow.com/questions/46024914/how-to-parse-exe-file-and-get-data-from-image-dos-header-structure-using-c-and
//eg_win32_get_sid(buf1);
//PIMAGE_DOS_HEADER a;
void System_Seq_Info(ecs_iter_t *it)
{
	EgText *path = ecs_field(it, EgText, 1);
	for (int i = 0; i < it->count; i ++)
	{
		ecs_entity_t e = it->entities[i];
		// Make sure that (EgText, EgFsOwner) is added so this system only runs once:
		EgText * text_owner = ecs_get_mut_pair(it->world, e, EgText, EgFsOwner);
		EgText * text_domain = ecs_get_mut_pair(it->world, e, EgText, EgFsDomain);
		text_owner->value = NULL;
		text_domain->value = NULL;
		HANDLE h = CreateFile(path[i].value, GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
		if (h == INVALID_HANDLE_VALUE)
		{
			eg_win32_PrintCSBackupAPIErrorMessage(GetLastError());
			continue;
		}
		PSID pSidOwner = NULL;
		PSECURITY_DESCRIPTOR pSD = NULL;
		{
			DWORD rv = GetSecurityInfo(h,SE_FILE_OBJECT,OWNER_SECURITY_INFORMATION,&pSidOwner,NULL,NULL,NULL,&pSD);
			CloseHandle(h);
			if (rv != ERROR_SUCCESS)
			{
				eg_win32_PrintCSBackupAPIErrorMessage(GetLastError());
				continue;
			}
		}
		{
			SID_NAME_USE eUse = SidTypeUnknown;
			DWORD dwAcctName = 0;
			DWORD dwDomainName = 0;
			BOOL rv;
			rv = LookupAccountSid(NULL,pSidOwner, NULL, (LPDWORD)&dwAcctName, NULL, (LPDWORD)&dwDomainName, &eUse);
			text_owner->value = ecs_os_calloc(dwAcctName);
			text_domain->value = ecs_os_calloc(dwDomainName);
			rv = LookupAccountSid(NULL,pSidOwner, text_owner->value, (LPDWORD)&dwAcctName, text_domain->value, (LPDWORD)&dwDomainName, &eUse);
			if (rv == FALSE)
			{
				eg_win32_PrintCSBackupAPIErrorMessage(GetLastError());
				continue;
			}
			continue;
		}

	}
}


















// The EgWin32Handle should free resources.
ECS_DTOR(EgWin32Handle, ptr, {
	FLOG(stdout, "EgWin32Handle::Dtor\n");
	if(ptr->handle){CloseHandle(ptr->handle);}
})

// The move hook should move resources from one location to another.
ECS_MOVE(EgWin32Handle, dst, src, {
	FLOG(stdout, "EgWin32Handle::Move\n");
	if(dst->handle){CloseHandle(dst->handle);}
	dst->handle = src->handle;
	src->handle = NULL;
})

// The copy hook should copy resources from one location to another.
ECS_COPY(EgWin32Handle, dst, src, {
	FLOG(stdout, "EgWin32Handle::Copy\n");
	dst->handle = src->handle;
})




void EgWin32Import(ecs_world_t *world)
{
	ECS_MODULE(world, EgWin32);
	ECS_IMPORT(world, FlecsUnits);
	ECS_IMPORT(world, EgFs);
	ECS_IMPORT(world, EgStr);
	ECS_COMPONENT_DEFINE(world, EgWin32Handle);
	ECS_COMPONENT_DEFINE(world, EgWin32IOCP);

	ecs_set_hooks(world, EgWin32Handle, {
		.ctor = ecs_default_ctor,
		.move = ecs_move(EgWin32Handle),
		.copy = ecs_copy(EgWin32Handle),
		.dtor = ecs_dtor(EgWin32Handle),
	});


	ecs_struct(world, {
	.entity = ecs_id(EgWin32Handle),
	.members = {
	{ .name = "handle", .type = ecs_id(ecs_uptr_t) }
	}
	});

	ecs_struct(world, {
	.entity = ecs_id(EgWin32IOCP),
	.members = {
	{ .name = "timeout_ms", .type = ecs_id(ecs_i32_t) },
	{ .name = "NumberOfBytesTransferred", .type = ecs_id(ecs_i32_t) },
	}
	});



	// This system will either add EgWin32ReadDirectoryChangesW or EgWin32DirNotification to an entity
	ecs_system(world, {
		.entity = ecs_entity(world, {
		.name = "System_Dirwatcher",
		.add = { ecs_dependson(EcsOnUpdate) }
		}),
		.query.filter.terms = {
		{ .id = ecs_id(EgWin32Handle), },
		{ .id = ecs_id(EgWin32IOCP), }
		},
		.callback = System_Dirwatcher
	});

	// This system will create a EgWin32Handle
	ecs_system(world, {
		.entity = ecs_entity(world, {
		.name = "System_Create_Watcher",
		.add = { ecs_dependson(EcsOnUpdate) }
		}),
		.query.filter.terms = {
		{.id = ecs_id(EgWin32Handle), .src.flags = EcsParent},
		{.id = ecs_id(EgWin32IOCP), .src.flags = EcsParent},
		{.id = ecs_pair(ecs_id(EgText), EgFsPath) },
		{.id = ecs_id(EgFsMonitorDir) },
		{.id = ecs_id(EgWin32Handle), .oper=EcsNot }, // Creates
		},
		.callback = System_Create_Watcher
	});

	ecs_system(world, {
		.entity = ecs_entity(world, {
		.name = "System_Dirwatch_Init",
		.add = { ecs_dependson(EcsOnUpdate) }
		}),
		.query.filter.terms = {
			{ .id = ecs_id(EgFsMonitor), },
			{ .id = ecs_id(EgWin32IOCP), .oper=EcsNot },
			{ .id = ecs_id(EgWin32Handle), .oper=EcsNot }
		},
		.callback = System_Dirwatch_Init
	});

	ecs_system(world, {
		.entity = ecs_entity(world, {
		.name = "System_List_Files",
		.add = { ecs_dependson(EcsOnUpdate) }
		}),
		.query.filter.terms = {
		{.id = ecs_pair(ecs_id(EgText), EgFsPath) },
		{.id = EgFsDir },
		{.id = EgFsList },
		},
		.callback = System_List_Files
	});

	ecs_system(world, {
		.entity = ecs_entity(world, {
		.name = "System_Seq_Info",
		.add = { ecs_dependson(EcsOnUpdate) }
		}),
		.query.filter.terms = {
		{.id = ecs_pair(ecs_id(EgText), EgFsPath) },
		{.id = EgFsFile },
		{.id = ecs_pair(ecs_id(EgText), EgFsOwner), .oper=EcsNot },
		},
		.callback = System_Seq_Info
	});


}