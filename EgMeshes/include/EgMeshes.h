#pragma once

#include <flecs.h>

typedef struct
{
	int32_t size;
	int32_t alignment;
	int32_t offset_pos;
	int32_t offset_col;
} EgMeshesVertexInfo;

typedef struct
{
	ecs_vec_t vertices;
	ecs_vec_t indices;
	uint32_t index_counter;
	EgMeshesVertexInfo info;
} EgMeshesVertices;

typedef struct
{
	uint32_t offset;
	uint32_t count;
} EgMeshesSection;

extern ECS_COMPONENT_DECLARE(EgMeshesVertexInfo);
extern ECS_COMPONENT_DECLARE(EgMeshesVertices);
extern ECS_COMPONENT_DECLARE(EgMeshesSection);

void EgMeshesImport(ecs_world_t *world);
