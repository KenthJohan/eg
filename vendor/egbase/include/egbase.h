#pragma once
#include <flecs.h>




typedef struct {
	char *value;
} EgBasePass;




typedef struct {
	void *ptr;
	int32_t size;
	int32_t cap;
} EgBaseMemory;

typedef struct {
	int32_t id;
	int32_t cap;
} EgBaseMemoryGPU;

typedef struct {
	EgBaseMemory buffer;        // pointer/size pair of output buffer
	int32_t data_size;    // size in bytes of valid data in buffer
	int32_t shape_offset; // data offset of the most recent shape
} EgBaseShapeBufferItem;

typedef struct {
	EgBaseShapeBufferItem vertices;
	EgBaseShapeBufferItem indices;
	EgBaseMemoryGPU vbuf;
	EgBaseMemoryGPU ibuf;
} EgBaseShapeBuffer;







typedef struct {
	uint32_t flags;
} EgBaseShowDrawReference;




extern ECS_COMPONENT_DECLARE(EgBasePass);
extern ECS_COMPONENT_DECLARE(EgBaseShapeElement);
extern ECS_COMPONENT_DECLARE(EgBaseShapeBuffer);
extern ECS_COMPONENT_DECLARE(EgBaseShowDrawReference);
extern ECS_COMPONENT_DECLARE(EgBaseMemory);



extern ECS_TAG_DECLARE(EgBaseUse);
extern ECS_TAG_DECLARE(EgBaseUpdate);
extern ECS_TAG_DECLARE(EgBaseDraw);

void EgBaseImport(ecs_world_t *world);