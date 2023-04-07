#pragma once
#include "flecs.h"

typedef struct
{
	ecs_f32_t left;
	ecs_f32_t right;
	ecs_f32_t botton;
	ecs_f32_t top;
} EgMargin4;

typedef struct
{
	ecs_u8_t z;
} EgZIndex;


typedef struct {
	ecs_entity_t entity;
	ecs_u32_t zindex;
} EgHover;


extern ECS_DECLARE(EgHover1);
extern ECS_COMPONENT_DECLARE(EgMargin4);
extern ECS_COMPONENT_DECLARE(EgZIndex);
extern ECS_COMPONENT_DECLARE(EgHover);



void EgGuiImport(ecs_world_t *world);
