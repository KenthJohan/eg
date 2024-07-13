#pragma once
#include <flecs.h>



typedef struct {
	uint32_t pipeline;
	uint32_t ibuf;
	uint32_t vbuf;
	int32_t offset;
	int32_t count;
	int32_t instances;
} MyGraphicsDrawCommand;


extern ECS_COMPONENT_DECLARE(MyGraphicsDrawCommand);

void MyGraphicsImport(ecs_world_t *world);