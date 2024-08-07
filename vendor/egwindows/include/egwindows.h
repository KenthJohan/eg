#pragma once
#include <flecs.h>


typedef struct {
	float w;
	float h;
	float dt;
	float pos[3];
	float fps;
	float canvas_mouse_x;
	float canvas_mouse_y;
	float canvas_width;
	float canvas_height;
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
	uint16_t key_rotate_dx_plus;
	uint16_t key_rotate_dx_minus;
	uint16_t key_rotate_dy_plus;
	uint16_t key_rotate_dy_minus;
	uint16_t key_rotate_dz_plus;
	uint16_t key_rotate_dz_minus;
	uint16_t key_move_dx_plus;
	uint16_t key_move_dx_minus;
	uint16_t key_move_dy_plus;
	uint16_t key_move_dy_minus;
	uint16_t key_move_dz_plus;
	uint16_t key_move_dz_minus;
	uint16_t key_fov_reset;
	uint16_t key_fov_plus;
	uint16_t key_fov_minus;
} KeyboardController;

typedef struct {
	int32_t keycode;
	ecs_entity_t entity;
} KeyActionToggleEntity;

typedef struct {
	int32_t dummy;
} EgWindowsMouseRay;


extern ECS_COMPONENT_DECLARE(Window);
extern ECS_COMPONENT_DECLARE(KeyActionToggleEntity);
extern ECS_COMPONENT_DECLARE(KeyboardController);
extern ECS_COMPONENT_DECLARE(EgWindowsMouseRay);


void EgWindowsImport(ecs_world_t *world);