#pragma once
#include <flecs.h>

#define EG_GPUS_LOGTAG "[EgGpus] "

typedef struct
{
	void *object;
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
	uint32_t info_num_vertex_attributes;
} EgGpusGraphicsPipeline;

typedef struct
{
	int32_t sample_count;
} EgGpusGraphicsPipelineCreateInfo;

typedef struct
{
	void *object;
} EgGpusShader;

typedef struct
{
	int32_t stage;
} EgGpusShaderCreateInfo;

extern ECS_COMPONENT_DECLARE(EgGpusDevice);
extern ECS_COMPONENT_DECLARE(EgGpusDeviceCreateInfo);
extern ECS_COMPONENT_DECLARE(EgGpusTexture);
extern ECS_COMPONENT_DECLARE(EgGpusTextureCreateInfo);
extern ECS_COMPONENT_DECLARE(EgGpusLocation);
extern ECS_COMPONENT_DECLARE(EgGpusGraphicsPipeline);
extern ECS_COMPONENT_DECLARE(EgGpusGraphicsPipelineCreateInfo);
extern ECS_COMPONENT_DECLARE(EgGpusShader);
extern ECS_COMPONENT_DECLARE(EgGpusShaderCreateInfo);

void EgGpusImport(ecs_world_t *world);
