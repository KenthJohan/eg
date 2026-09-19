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

typedef struct
{
	void *object;
} EgGpusGraphicsPipeline;

typedef struct
{
	int32_t sample_count;
} EgGpusGraphicsPipelineCreateInfo;

typedef struct
{
	void *object;
} EgGpusShaderVertex;

typedef struct
{
	void *object;
} EgGpusShaderFragment;

extern ECS_COMPONENT_DECLARE(EgGpusDevice);
extern ECS_COMPONENT_DECLARE(EgGpusDeviceCreateInfo);
extern ECS_COMPONENT_DECLARE(EgGpusTexture);
extern ECS_COMPONENT_DECLARE(EgGpusTextureCreateInfo);
extern ECS_COMPONENT_DECLARE(EgGpusLocation);
extern ECS_COMPONENT_DECLARE(EgGpusGraphicsPipeline);
extern ECS_COMPONENT_DECLARE(EgGpusGraphicsPipelineCreateInfo);
extern ECS_COMPONENT_DECLARE(EgGpusShaderVertex);
extern ECS_COMPONENT_DECLARE(EgGpusShaderFragment);

void EgGpusImport(ecs_world_t *world);
