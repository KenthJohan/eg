#pragma once
#include <flecs.h>

#include <egmath.h>


typedef struct {
	int32_t offset;
	int32_t count;
	int32_t instances;
	m4f32 * transforms_data;
	int32_t transforms_count;
	int32_t transforms_cap;
	int32_t transforms_total;
} MyGraphicsDrawCommand;


extern ECS_COMPONENT_DECLARE(MyGraphicsDrawCommand);

void MyGraphicsImport(ecs_world_t *world);