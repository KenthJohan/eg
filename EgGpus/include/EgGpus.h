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

typedef struct
{
	void *object;
} EgGpusTexture;

typedef struct
{
	uint32_t sample_count;
} EgGpusTextureCreateInfo;

extern ECS_COMPONENT_DECLARE(EgGpusDevice);
extern ECS_COMPONENT_DECLARE(EgGpusDeviceCreateInfo);
extern ECS_COMPONENT_DECLARE(EgGpusTexture);
extern ECS_COMPONENT_DECLARE(EgGpusTextureCreateInfo);

void EgGpusImport(ecs_world_t *world);
