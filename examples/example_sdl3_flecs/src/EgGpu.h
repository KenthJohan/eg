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
	void * device;
} EgGpuShader;

typedef struct
{
	void * device;
} EgGpuShaderCreateInfo;



extern ECS_COMPONENT_DECLARE(EgGpuDevice);
extern ECS_COMPONENT_DECLARE(EgGpuDeviceCreateInfo);
extern ECS_COMPONENT_DECLARE(EgGpuShader);
extern ECS_COMPONENT_DECLARE(EgGpuShaderCreateInfo);

void EgGpuImport(ecs_world_t *world);


