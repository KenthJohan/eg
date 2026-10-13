#pragma once

#include <flecs.h>

enum {
	EG_UI_EDGE_LEFT   = 1,
	EG_UI_EDGE_RIGHT  = 2,
	EG_UI_EDGE_BOTTOM = 4,
	EG_UI_EDGE_TOP    = 8,
};

typedef struct {
	uint32_t key;     // The key code associated with the button
	bool     hovered; // Indicates if the button is currently hovered over
	bool     held;    // Indicates if the button is currently held down
} EgUiButtonsButton;

typedef struct {
	uint32_t key;    // The key code that grabs a border
	float    margin; // Border hit thickness around each edge in screen pixels
	uint8_t  hover;  // EG_UI_EDGE_* mask of borders under the pointer
	uint8_t  grab;   // EG_UI_EDGE_* mask of borders grabbed while the key is held
} EgUiButtonsBorder;

extern ECS_COMPONENT_DECLARE(EgUiButtonsButton);
extern ECS_COMPONENT_DECLARE(EgUiButtonsBorder);

void EgUiButtonsImport(ecs_world_t *world);
