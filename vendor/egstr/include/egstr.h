#pragma once
#include <flecs.h>



typedef struct {
	ecs_string_t value;
} EgText;

typedef struct {
	int32_t size;
	void * data;
} EgBuffer;



extern ECS_COMPONENT_DECLARE(EgText);
extern ECS_COMPONENT_DECLARE(EgBuffer);


void EgStrImport(ecs_world_t *world);