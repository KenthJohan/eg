#pragma once
#include "flecs.h"

typedef struct
{
	ecs_i32_t dummy;
} EgSokolGfxConfig;

extern ECS_DECLARE(EgSokolGfxState);
extern ECS_COMPONENT_DECLARE(EgSokolGfxConfig);

void EgSokolGfxImport(ecs_world_t *world);
