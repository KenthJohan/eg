#include "EgWin32.h"
#include "EgFs.h"
#include "EgQuantities.h"

ECS_COMPONENT_DECLARE(EgWin32IOCP);
ECS_COMPONENT_DECLARE(EgWin32Handle);
ECS_COMPONENT_DECLARE(EgWin32ReadDirectoryChangesW);


#define FLOG(...) fprintf(__VA_ARGS__)
#define EG_DIRWATCH_PATH_LENGTH 1024



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
		//printf("Got packet! %i, %p\n", (int)NumberOfBytesTransferred, (void*)CompletionKey);
		ecs_entity_t ev = (ecs_entity_t)CompletionKey;
		assert(ecs_has(it->world, ev, EgFsMonitorDir));
		assert(ecs_has(it->world, ev, EgBuffer));
		if(NumberOfBytesTransferred > 0)
		{
			//ecs_set(it->world, ev, EgWin32DirNotification, {NumberOfBytesTransferred});
			EgBuffer const * b = ecs_get(it->world, ev, EgBuffer);
			FILE_NOTIFY_INFORMATION const * fni = (FILE_NOTIFY_INFORMATION const*)b->data;
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
				ecs_add_pair(it->world, evv, EcsChildOf, ev);
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
					ecs_add(it->world, ev, EgWin32ReadDirectoryChangesW);
				}
			}
		}
		else
		{
			ecs_set(it->world, ev, EgWin32ReadDirectoryChangesW, {false});
		}
	}
}



void System_DirRequest(ecs_iter_t *it)
{
	EgWin32Handle *h = ecs_field(it, EgWin32Handle, 1);
	EgBuffer *buf = ecs_field(it, EgBuffer, 2);
	for (int i = 0; i < it->count; i ++)
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
		OVERLAPPED overlapped = {0};
		HANDLE * handle = h[i].handle;
		assert(buf[i].data);
		// Issue reqests to retrieves information that describes the changes within the specified directory.
		// The function does not report changes to the specified directory itself.
		WINBOOL success = ReadDirectoryChangesW(handle, buf[i].data, CHANGE_BUF_SIZE, bWatchSubtree, dwNotifyFilter, NULL, &overlapped, NULL);
		if(success == FALSE)
		{
			FLOG(stderr, "Error: ReadDirectoryChangesW\n");
			win32_PrintCSBackupAPIErrorMessage(GetLastError());
		}
		ecs_remove(it->world, it->entities[i], EgWin32ReadDirectoryChangesW);
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
			win32_PrintCSBackupAPIErrorMessage(GetLastError());
			FLOG(stderr, "Error: CreateIoCompletionPort\n");
		}
		WINBOOL success = PostQueuedCompletionStatus(h0[i].handle, 0, (ULONG_PTR)e, 0);
		if(success == FALSE)
		{
			win32_PrintCSBackupAPIErrorMessage(GetLastError());
			FLOG(stderr, "Error: PostQueuedCompletionStatus\n");
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
	ECS_IMPORT(world, EgFs);
	ECS_IMPORT(world, EgQuantities);
	ECS_COMPONENT_DEFINE(world, EgWin32Handle);
	ECS_COMPONENT_DEFINE(world, EgWin32IOCP);
	ECS_COMPONENT_DEFINE(world, EgWin32ReadDirectoryChangesW);

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

	ecs_struct(world, {
	.entity = ecs_id(EgWin32ReadDirectoryChangesW),
	.members = {
	{ .name = "bWatchSubtree", .type = ecs_id(ecs_bool_t) },
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



	// This system will remove EgWin32ReadDirectoryChangesW
	ecs_system(world, {
		.entity = ecs_entity(world, {
		.name = "System_DirRequest",
		.add = { ecs_dependson(EcsOnUpdate) }
		}),
		.query.filter.terms = {
			{.id = ecs_id(EgWin32Handle) },
			{.id = ecs_id(EgBuffer) },
			{.id = ecs_id(EgWin32ReadDirectoryChangesW) }, // Removes
		},
		.callback = System_DirRequest
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



}