#pragma once
#include "flecs.h"



#define EG_NUM_KEYS 512
#define EG_NUM_KEYS64 (EG_NUM_KEYS/64)

typedef struct
{
	ecs_u64_t keyboard[EG_NUM_KEYS64];      // ___|¨¨¨¨|__
	ecs_u64_t keyboard_up[EG_NUM_KEYS64];   // ________|__
	ecs_u64_t keyboard_down[EG_NUM_KEYS64]; // ___|_______
	ecs_i32_t mouse_x;
	ecs_i32_t mouse_y;
	ecs_i32_t mouse_dx;
	ecs_i32_t mouse_dy;
} EgUserEvent;

extern ECS_COMPONENT_DECLARE(EgUserEvent);


void FlecsComponentsEgUserinputImport(ecs_world_t *world);
