#include "EgWin32.h"
#include "EgFs.h"

ECS_COMPONENT_DECLARE(EgWin32IOCP);
#define FLOG(...) fprintf(__VA_ARGS__)


void System_Dirwatcher(ecs_iter_t *it)
{
	EgWin32IOCP *dw = ecs_field(it, EgWin32IOCP, 1);
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
			ecs_add(it->world, ev, EgDirRes);
		}
		else
		{
			ecs_add(it->world, ev, EgDirReq);
		}
	}
}

ECS_CTOR(EgWin32IOCP, ptr, {
    FLOG(stdout, "EgWin32IOCP::Ctor\n");
	ecs_os_memset_t(ptr, 0, EgWin32IOCP);
	DWORD NumberOfConcurrentThreads = 0;
	ptr->hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)0, NumberOfConcurrentThreads);
	if((ptr->hIOCP == 0) || (ptr->hIOCP == INVALID_HANDLE_VALUE))
	{
		FLOG(stderr, "Error: CreateIoCompletionPort\n");
		win32_PrintCSBackupAPIErrorMessage(GetLastError());
	}
})

// The destructor should free resources.
ECS_DTOR(EgWin32IOCP, ptr, {
    FLOG(stdout, "EgWin32IOCP::Dtor\n");
	if(ptr->hIOCP){CloseHandle(ptr->hIOCP);}
})

// The move hook should move resources from one location to another.
ECS_MOVE(EgWin32IOCP, dst, src, {
    FLOG(stdout, "EgWin32IOCP::Move\n");
	if(dst->hIOCP){ecs_os_free(dst->hIOCP);}
    dst->hIOCP = src->hIOCP;
    src->hIOCP = NULL;
})

// The copy hook should copy resources from one location to another.
ECS_COPY(EgWin32IOCP, dst, src, {
    FLOG(stdout, "EgWin32IOCP::Copy\n");
})


void EgWin32Import(ecs_world_t *world)
{
	ECS_MODULE(world, EgWin32);
	ECS_COMPONENT_DEFINE(world, EgWin32IOCP);


	ecs_set_hooks(world, EgWin32IOCP, {
		/* Resource management hooks. These hooks should primarily be used for
		* managing memory used by the component. */
		.ctor = ecs_ctor(EgWin32IOCP),
		.move = ecs_move(EgWin32IOCP),
		.copy = ecs_copy(EgWin32IOCP),
		.dtor = ecs_dtor(EgWin32IOCP),
	});

	// Attempts to dequeue an I/O completion packet.
	// Retrieves information that describes the changes within the specified directory.
	ecs_system(world, {
		.entity = ecs_entity(world, {
		.name = "System_Dirwatcher",
		.add = { ecs_dependson(EcsOnUpdate) }
		}),
		.query.filter.terms = {
		{ .id = ecs_id(EgWin32IOCP), }
		},
		.callback = System_Dirwatcher
	});
}