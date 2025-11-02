#pragma once
#include <flecs.h>



typedef struct
{
	ecs_vec_t data;
} EgBaseVec;

typedef struct
{
	ecs_vec_t vertices;
	ecs_vec_t indices;
	uint32_t stride_vertices;
} EgBaseVertexIndexVec;

typedef struct
{
	uint32_t offset;
	uint32_t count;
} EgBaseOffsetCount;


extern ECS_COMPONENT_DECLARE(EgBaseVec);
extern ECS_COMPONENT_DECLARE(EgBaseVertexIndexVec);
extern ECS_COMPONENT_DECLARE(EgBaseOffsetCount);


extern ECS_TAG_DECLARE(EgBaseLoad);
extern ECS_TAG_DECLARE(EgBaseCopyTo);
extern ECS_TAG_DECLARE(EgBaseUpdate);
extern ECS_TAG_DECLARE(EgBaseError);

void EgBaseImport(ecs_world_t *world);
