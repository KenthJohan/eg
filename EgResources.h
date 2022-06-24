#pragma once
#include "flecs.h"


typedef struct
{
	ecs_f32_t x;
	ecs_f32_t y;
} EgImage;

typedef struct
{
	ecs_string_t value;
} EgPath;


extern ECS_COMPONENT_DECLARE(EgImage);
extern ECS_COMPONENT_DECLARE(EgPath);
//extern ECS_DECLARE(EgPath);

void EgResourcesImport(ecs_world_t *world);
