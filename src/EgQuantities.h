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

typedef struct
{
	ecs_f32_t dx;
	ecs_f32_t dy;
} EgVelocity_V2F32;

typedef struct
{
	ecs_f32_t x;
	ecs_f32_t y;
} EgPosition_V2F32;

typedef struct
{
	ecs_f32_t x;
	ecs_f32_t y;
} EgPositionGlobal_V2F32;

typedef struct
{
	ecs_f32_t w;
	ecs_f32_t h;
} EgRectangle_V2F32;

typedef struct {
	ecs_string_t value;
} EgText;

typedef struct
{
	ecs_u8_t r;
	ecs_u8_t g;
	ecs_u8_t b;
	ecs_u8_t a;
} EgColorRGBA_V4U8;



extern ECS_DECLARE(EgUserinput);
//extern ECS_DECLARE(EgPosition);
extern ECS_DECLARE(EgVelocity);
extern ECS_DECLARE(EgPositionRelative);
//extern ECS_DECLARE(EgRectangle);
extern ECS_DECLARE(EgColor);
extern ECS_DECLARE(EgColorRandom);
extern ECS_DECLARE(EgViewport);
extern ECS_COMPONENT_DECLARE(EgV1F32);
//extern ECS_COMPONENT_DECLARE(EgV2F32);
extern ECS_COMPONENT_DECLARE(EgV3F32);
extern ECS_COMPONENT_DECLARE(EgV4F32);
extern ECS_COMPONENT_DECLARE(EgVelocity_V2F32);
extern ECS_COMPONENT_DECLARE(EgPosition_V2F32);
extern ECS_COMPONENT_DECLARE(EgPositionGlobal_V2F32);
extern ECS_COMPONENT_DECLARE(EgRectangle_V2F32);
extern ECS_COMPONENT_DECLARE(EgV4U8);
extern ECS_COMPONENT_DECLARE(EgText);
extern ECS_COMPONENT_DECLARE(EgColorRGBA_V4U8);



void EgQuantitiesImport(ecs_world_t *world);
