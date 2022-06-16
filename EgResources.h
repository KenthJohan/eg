#pragma once
#include "flecs.h"


typedef struct
{
	ecs_f32_t x;
	ecs_f32_t y;
} EgImage;

extern ECS_COMPONENT_DECLARE(EgImage);


void EgResourcesImport(ecs_world_t *world);
