#pragma once
#include <flecs.h>


typedef struct
{
	void * device;
} EgGpuDevice;

typedef struct
{
	bool debug;
} EgGpuDeviceCreateInfo;

typedef struct
{
	void * object;
} EgGpuShader;

typedef struct
{
	uint32_t stage;
} EgGpuShaderCreateInfo;

typedef struct
{
	void * object;
} EgGpuPipeline;

typedef struct
{
	bool target_info_has_depth_stencil_target;
} EgGpuPipelineCreateInfo;



extern ECS_COMPONENT_DECLARE(EgGpuDevice);
extern ECS_COMPONENT_DECLARE(EgGpuDeviceCreateInfo);
extern ECS_COMPONENT_DECLARE(EgGpuShader);
extern ECS_COMPONENT_DECLARE(EgGpuShaderCreateInfo);
extern ECS_COMPONENT_DECLARE(EgGpuPipeline);
extern ECS_COMPONENT_DECLARE(EgGpuPipelineCreateInfo);

void EgGpuImport(ecs_world_t *world);


