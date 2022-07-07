#pragma once
#include "flecs.h"

typedef struct
{
	ecs_f32_t x;
	ecs_f32_t y;
	void * buffer;
} EgFile;

typedef struct
{
	uint32_t fetched_offset;
	uint32_t fetched_size;
	void* buffer_ptr;
	uint32_t buffer_size;
} EgChunk;

typedef struct
{
	ecs_f32_t x;
	ecs_f32_t y;
	void * buffer;
} EgImage;

typedef struct
{
	ecs_string_t value;
} EgPath;

typedef struct
{
	ecs_world_t * world;
	ecs_entity_t entity;
} EgWorldEntity;

extern ECS_DECLARE(EgUpdating);
extern ECS_DECLARE(EgUpdate);

extern ECS_COMPONENT_DECLARE(EgImage);
extern ECS_COMPONENT_DECLARE(EgPath);
extern ECS_COMPONENT_DECLARE(EgChunk);
//extern ECS_DECLARE(EgPath);

void EgResourcesImport(ecs_world_t *world);
