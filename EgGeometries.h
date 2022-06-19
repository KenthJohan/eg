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

typedef struct
{
	ecs_f32_t x;
	ecs_f32_t y;
	ecs_f32_t z;
} EgBoxF32;



extern ECS_COMPONENT_DECLARE(EgRectangleF32);
extern ECS_COMPONENT_DECLARE(EgRectangleI32);
extern ECS_COMPONENT_DECLARE(EgBoxF32);

extern ECS_DECLARE(EgPrimitivePoint);
extern ECS_DECLARE(EgPrimitiveLine);
extern ECS_DECLARE(EgPrimitiveRectangle);


void EgGeometriesImport(ecs_world_t *world);
