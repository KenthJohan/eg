#pragma once
#include <flecs.h>
#include <stdint.h>


typedef struct
{
	uint32_t id;
} EgDisplaysDevice;

typedef struct
{
	uint32_t bpp;
	uint32_t refresh_rate;
	uint32_t pixel_density;
} EgDisplaysMode;

typedef struct
{
	uint32_t dummy;
} EgDisplaysUpdate;


extern ECS_COMPONENT_DECLARE(EgDisplaysDevice);
extern ECS_COMPONENT_DECLARE(EgDisplaysMode);
extern ECS_COMPONENT_DECLARE(EgDisplaysUpdate);

void EgDisplaysImport(ecs_world_t *world);
