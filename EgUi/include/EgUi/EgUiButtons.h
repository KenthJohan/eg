#pragma once

#include <flecs.h>

typedef struct {
	uint32_t key;     // The key code associated with the button
	bool     hovered; // Indicates if the button is currently hovered over
	bool     held;    // Indicates if the button is currently held down
} EgUiButtonsButton;

void EgUiButtonsImport(ecs_world_t *world);
