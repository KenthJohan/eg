#pragma once
#include "flecs.h"

typedef struct
{
	ecs_f32_t x;
} EgV1F32;

typedef struct
{
	ecs_f32_t x;
	ecs_f32_t y;
} EgV2F32;

typedef struct
{
	ecs_f32_t x;
	ecs_f32_t y;
	ecs_f32_t z;
} EgV3F32;

typedef struct
{
	ecs_f32_t x;
	ecs_f32_t y;
	ecs_f32_t z;
	ecs_f32_t w;
} EgV4F32;


typedef struct
{
	ecs_u8_t x;
	ecs_u8_t y;
	ecs_u8_t z;
	ecs_u8_t w;
} EgV4U8;

typedef struct {
	ecs_string_t value;
} EgText;





extern ECS_DECLARE(EgUserinput);
extern ECS_DECLARE(EgPosition);
extern ECS_DECLARE(EgVelocity);
extern ECS_DECLARE(EgPositionRelative);
extern ECS_DECLARE(EgRectangle);
extern ECS_DECLARE(EgColor);
extern ECS_COMPONENT_DECLARE(EgV1F32);
extern ECS_COMPONENT_DECLARE(EgV2F32);
extern ECS_COMPONENT_DECLARE(EgV3F32);
extern ECS_COMPONENT_DECLARE(EgV4F32);
extern ECS_COMPONENT_DECLARE(EgV4U8);
extern ECS_COMPONENT_DECLARE(EgText);



void EgQuantitiesImport(ecs_world_t *world);
