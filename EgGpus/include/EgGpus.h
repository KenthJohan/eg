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
} EgGpusShaderVertex;

typedef struct
{
	char const *path;
} EgGpusShaderVertexCreateInfo;

typedef struct
{
	void *object;
} EgGpusShaderFragment;

typedef struct
{
	char const *path;
} EgGpusShaderFragmentCreateInfo;

extern ECS_COMPONENT_DECLARE(EgGpusDevice);
extern ECS_COMPONENT_DECLARE(EgGpusDeviceCreateInfo);
extern ECS_COMPONENT_DECLARE(EgGpusTexture);
extern ECS_COMPONENT_DECLARE(EgGpusTextureCreateInfo);
extern ECS_COMPONENT_DECLARE(EgGpusLocation);
extern ECS_COMPONENT_DECLARE(EgGpusGraphicsPipeline);
extern ECS_COMPONENT_DECLARE(EgGpusGraphicsPipelineCreateInfo);
extern ECS_COMPONENT_DECLARE(EgGpusShaderVertex);
extern ECS_COMPONENT_DECLARE(EgGpusShaderVertexCreateInfo);
extern ECS_COMPONENT_DECLARE(EgGpusShaderFragment);
extern ECS_COMPONENT_DECLARE(EgGpusShaderFragmentCreateInfo);

void EgGpusImport(ecs_world_t *world);
