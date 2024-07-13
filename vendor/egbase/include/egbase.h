#pragma once
#include <flecs.h>

typedef struct {
	void *ptr;
	int32_t cap; // Capacity
	int32_t size; // Current size
	int32_t last; // Starting byte position of last item
} EgBaseMemory;


typedef struct {
	EgBaseMemory mem[2];
} EgBaseMemory2;


typedef struct {
	int32_t id;
	int32_t cap;
} EgBaseMemoryGPU;



typedef struct {
	EgBaseMemoryGPU vbuf;
	EgBaseMemoryGPU ibuf;
} EgBaseShapeBuffer;

typedef struct {
	uint32_t flags;
} EgBaseShowDrawReference;

extern ECS_COMPONENT_DECLARE(EgBaseShapeBuffer);
extern ECS_COMPONENT_DECLARE(EgBaseShowDrawReference);
extern ECS_COMPONENT_DECLARE(EgBaseMemory);
extern ECS_COMPONENT_DECLARE(EgBaseMemory2);

extern ECS_TAG_DECLARE(EgBaseUse);
extern ECS_TAG_DECLARE(EgBaseUpdate);
extern ECS_TAG_DECLARE(EgBaseDraw);

void EgBaseImport(ecs_world_t *world);