#pragma once
#include <flecs.h>





typedef struct {
	float r;
	float g;
	float b;
	float a;
} Color;

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
} Color32;


typedef struct {
	char *value;
} Pass;


typedef struct {
	int32_t base_element;
	int32_t num_elements;
} ShapeElement;

typedef struct {
	void *ptr;
	int32_t size;
	int32_t cap;
} Memory;

typedef struct {
	int32_t id;
	int32_t cap;
} MemoryGPU;

typedef struct {
	Memory buffer;        // pointer/size pair of output buffer
	int32_t data_size;    // size in bytes of valid data in buffer
	int32_t shape_offset; // data offset of the most recent shape
} ShapeBufferItem;

typedef struct {
	ShapeBufferItem vertices;
	ShapeBufferItem indices;
	MemoryGPU vbuf;
	MemoryGPU ibuf;
} ShapeBuffer;







typedef struct {
	int32_t dummy;
} ShowDrawReference;




extern ECS_COMPONENT_DECLARE(Pass);
extern ECS_COMPONENT_DECLARE(Color);
extern ECS_COMPONENT_DECLARE(Color32);
extern ECS_COMPONENT_DECLARE(ShapeElement);
extern ECS_COMPONENT_DECLARE(ShapeBuffer);
extern ECS_COMPONENT_DECLARE(ShowDrawReference);
extern ECS_COMPONENT_DECLARE(Memory);


extern ECS_TAG_DECLARE(EgRotateOrder1);
extern ECS_TAG_DECLARE(EgRotateOrder2);
extern ECS_TAG_DECLARE(EgUse);
extern ECS_TAG_DECLARE(EgUpdate);

void ComponentsImport(ecs_world_t *world);