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
	ecs_f32_t x;
	ecs_f32_t y;
} EgForce2F32;

typedef struct
{
	ecs_f32_t x;
	ecs_f32_t y;
} EgDrag2F32;

typedef struct
{
	ecs_f32_t x;
	ecs_f32_t y;
} EgMomentum2F32;

typedef struct
{
	ecs_f32_t value;
} EgMassF32;

typedef struct
{
	ecs_f32_t value;
} EgTimeF32;

typedef struct
{
	ecs_f32_t value;
} EgTemperatureF32;

typedef struct
{
	ecs_f32_t value;
} EgLengthF32;

typedef struct
{
	ecs_f32_t value;
} EgDensityF32;


extern ECS_COMPONENT_DECLARE(EgPosition2F32);
extern ECS_COMPONENT_DECLARE(EgPosition2I32);
extern ECS_COMPONENT_DECLARE(EgVelocity2F32);
extern ECS_COMPONENT_DECLARE(EgAcceleration2F32);
extern ECS_COMPONENT_DECLARE(EgForce2F32);
extern ECS_COMPONENT_DECLARE(EgDrag2F32);
extern ECS_COMPONENT_DECLARE(EgMomentum2F32);
extern ECS_COMPONENT_DECLARE(EgMassF32);
extern ECS_COMPONENT_DECLARE(EgTimeF32);
extern ECS_COMPONENT_DECLARE(EgTemperatureF32);
extern ECS_COMPONENT_DECLARE(EgLengthF32);
extern ECS_COMPONENT_DECLARE(EgDensityF32);



void EgQuantitiesImport(ecs_world_t *world);
