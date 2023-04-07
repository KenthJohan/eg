#pragma once
#include "flecs.h"


#define EG_KEYBOARD_SIZE 128
typedef struct {
	ecs_u8_t keys[EG_KEYBOARD_SIZE];
} EgKeyboard;


typedef struct {
	ecs_u8_t left;
	ecs_u8_t right;
} EgMouse;


extern ECS_COMPONENT_DECLARE(EgMouse);
extern ECS_COMPONENT_DECLARE(EgKeyboard);



void EgUserinputImport(ecs_world_t *world);
