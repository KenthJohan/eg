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
	ecs_f32_t left;
	ecs_f32_t right;
	ecs_f32_t botton;
	ecs_f32_t top;
} EgGuiBorder4;

typedef struct
{
	ecs_u8_t z;
} EgZIndex;


typedef struct {
	ecs_entity_t entity;
	ecs_u32_t zindex;
	ecs_entity_t entity1;
} EgHover;

typedef struct {
	ecs_entity_t entity;
} EgGuiDrag;

extern ECS_DECLARE(EgGuiWatchMouseOver);
extern ECS_DECLARE(EgGuiWatchDragging);

extern ECS_DECLARE(EgGuiMouseOver);
extern ECS_DECLARE(EgGuiMouseOver1);
extern ECS_DECLARE(EgGuiDragging);
extern ECS_COMPONENT_DECLARE(EgMargin4);
extern ECS_COMPONENT_DECLARE(EgZIndex);
extern ECS_COMPONENT_DECLARE(EgHover);
extern ECS_COMPONENT_DECLARE(EgGuiDrag);
extern ECS_COMPONENT_DECLARE(EgGuiBorder4);



void EgGuiImport(ecs_world_t *world);
