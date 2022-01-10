#pragma once
#include "flecs.h"

typedef struct
{
	ecs_u64_t counter;
	ecs_bool_t should_destroy;
} EgWindow;

typedef struct
{
	int dummy;
} EgDraw;

typedef struct
{
	ecs_string_t value;
} EgTitle;


extern ECS_COMPONENT_DECLARE(EgWindow);
extern ECS_COMPONENT_DECLARE(EgDraw);
extern ECS_COMPONENT_DECLARE(EgTitle);


void FlecsComponentsEgWindowImport(ecs_world_t *world);
