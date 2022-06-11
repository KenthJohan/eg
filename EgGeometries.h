#pragma once
#include "flecs.h"



typedef struct
{
	ecs_f32_t width;
	ecs_f32_t height;
} EgRectangleF32;

typedef struct
{
	ecs_i32_t width;
	ecs_i32_t height;
} EgRectangleI32;




extern ECS_COMPONENT_DECLARE(EgRectangleF32);
extern ECS_COMPONENT_DECLARE(EgRectangleI32);


void EgGeometriesImport(ecs_world_t *world);
