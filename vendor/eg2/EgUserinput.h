#pragma once
#include "flecs.h"

#define EG_EDGE_UP      0x01
#define EG_EDGE_DOWN    0x02
#define EG_EDGE_RISING  0x10
#define EG_EDGE_FALLING 0x20
#define EG_KEYBOARD_SIZE 512


typedef struct {
	ecs_u8_t keys[EG_KEYBOARD_SIZE];
} EgKeyboard;


typedef struct {
	ecs_u8_t left;
	ecs_u8_t right;
	ecs_f32_t scroll_x;
	ecs_f32_t scroll_y;
} EgMouse;


extern ECS_TAG_DECLARE(EgUserinput);
extern ECS_COMPONENT_DECLARE(EgMouse);
extern ECS_COMPONENT_DECLARE(EgKeyboard);



void EgUserinputImport(ecs_world_t *world);
