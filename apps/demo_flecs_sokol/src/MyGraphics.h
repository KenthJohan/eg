#pragma once
#include <flecs.h>



typedef struct {
	int32_t offset;
	int32_t count;
	int32_t instances;
} MyGraphicsDrawCommand;


extern ECS_COMPONENT_DECLARE(MyGraphicsDrawCommand);

void MyGraphicsImport(ecs_world_t *world);