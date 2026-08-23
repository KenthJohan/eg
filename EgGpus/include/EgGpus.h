#pragma once
#include <flecs.h>

typedef struct
{
	void *device;
} EgGpusDevice;

typedef struct
{
	bool debug;
} EgGpusDeviceCreateInfo;

extern ECS_COMPONENT_DECLARE(EgGpusDevice);
extern ECS_COMPONENT_DECLARE(EgGpusDeviceCreateInfo);

void EgGpusImport(ecs_world_t *world);
