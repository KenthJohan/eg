#pragma once
#include <flecs.h>

#define EG_GPUS_LOGTAG "[EgGpus] "

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

typedef struct
{
	int32_t location;
} EgGpusLocation;

extern ECS_COMPONENT_DECLARE(EgGpusDevice);
extern ECS_COMPONENT_DECLARE(EgGpusDeviceCreateInfo);
extern ECS_COMPONENT_DECLARE(EgGpusTexture);
extern ECS_COMPONENT_DECLARE(EgGpusTextureCreateInfo);
extern ECS_COMPONENT_DECLARE(EgGpusLocation);

void EgGpusImport(ecs_world_t *world);
