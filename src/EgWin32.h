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

typedef struct
{
	HANDLE hIOCP;
	int32_t timeout_ms;
} EgWin32IOCP;


extern ECS_COMPONENT_DECLARE(EgWin32IOCP);

void EgWin32Import(ecs_world_t *world);