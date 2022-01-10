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
	ecs_f32_t x;
	ecs_f32_t y;
} EgVelocity2F32;

typedef struct
{
	ecs_f32_t x;
	ecs_f32_t y;
} EgAcceleration2F32;

typedef struct
{
	float value;
} EgMassF32;

typedef struct
{
	float x;
	float y;
} EgForce2F32;


extern ECS_COMPONENT_DECLARE(EgPosition2F32);
extern ECS_COMPONENT_DECLARE(EgPosition2I32);
extern ECS_COMPONENT_DECLARE(EgVelocity2F32);
extern ECS_COMPONENT_DECLARE(EgAcceleration2F32);
extern ECS_COMPONENT_DECLARE(EgMassF32);
extern ECS_COMPONENT_DECLARE(EgForceF32);


void FlecsComponentsEgQuantityImport(ecs_world_t *world);
