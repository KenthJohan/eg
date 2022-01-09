#pragma once
#include "flecs.h"

typedef struct
{
	ecs_string_t title;
	ecs_u64_t counter;
} EgWindow;

typedef struct
{
	int dummy;
} EgDraw;

extern ECS_COMPONENT_DECLARE(EgWindow);
extern ECS_COMPONENT_DECLARE(EgDraw);


void FlecsComponentsEgWindowImport(ecs_world_t *world);
