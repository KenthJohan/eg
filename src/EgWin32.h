#pragma once

#include <flecs.h>

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
	int32_t timeout_ms;
	int32_t NumberOfBytesTransferred;
} EgWin32IOCP;

typedef struct
{
	HANDLE handle;
} EgWin32Handle;

typedef struct
{
	int32_t NumberOfBytesTransferred;
} EgWin32DirNotification;

typedef struct
{
	bool bWatchSubtree;
} EgWin32ReadDirectoryChangesW;



extern ECS_COMPONENT_DECLARE(EgWin32IOCP);
extern ECS_COMPONENT_DECLARE(EgWin32Handle);
extern ECS_COMPONENT_DECLARE(EgWin32DirNotification);
extern ECS_COMPONENT_DECLARE(EgWin32ReadDirectoryChangesW);

void EgWin32Import(ecs_world_t *world);