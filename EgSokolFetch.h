#pragma once
#include "flecs.h"


typedef struct
{
	ecs_i32_t max_requests;
	ecs_i32_t num_channels;
	ecs_i32_t num_lanes;
} EgSokolFetchConfig;

extern ECS_COMPONENT_DECLARE(EgSokolFetchConfig);

void EgSokolFetchImport(ecs_world_t *world);
