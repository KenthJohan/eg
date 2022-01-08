#pragma once
#include "flecs.h"

typedef struct
{
	ecs_f32_t x;
	ecs_f32_t y;
} EgPosition2F32;

typedef struct
{
	ecs_i32_t x;
	ecs_i32_t y;
} EgPosition2I32;

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
extern ECS_COMPONENT_DECLARE(EgPosition2F32);
extern ECS_COMPONENT_DECLARE(EgPosition2I32);


void FlecsComponentsEgGeometryImport(ecs_world_t *world);
