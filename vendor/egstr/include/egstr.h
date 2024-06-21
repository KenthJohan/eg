#pragma once
#include <flecs.h>



typedef struct {
	ecs_string_t value;
} EgStrText;

typedef struct {
	int32_t size;
	void * data;
} EgStrBuffer;



extern ECS_COMPONENT_DECLARE(EgStrText);
extern ECS_COMPONENT_DECLARE(EgStrBuffer);


void EgStrImport(ecs_world_t *world);