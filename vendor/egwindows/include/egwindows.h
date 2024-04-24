#pragma once
#include <flecs.h>


typedef struct {
	float w;
	float h;
	float dt;
	float mouse_x;
	float mouse_y;
	float mouse_dx;
	float mouse_dy;
	uint8_t mouse_left;
	uint8_t mouse_right;
	uint8_t mouse_left_edge;
	uint8_t mouse_right_edge;
	uint8_t keys[512];
	uint8_t keys_edge[512];
} Window;

typedef struct {
	int32_t dummy;
} KeyboardController;

typedef struct {
	int32_t keycode;
	ecs_entity_t entity;
} KeyActionToggleEntity;


extern ECS_COMPONENT_DECLARE(Window);
extern ECS_COMPONENT_DECLARE(KeyActionToggleEntity);
extern ECS_COMPONENT_DECLARE(KeyboardController);


void EgWindowsImport(ecs_world_t *world);