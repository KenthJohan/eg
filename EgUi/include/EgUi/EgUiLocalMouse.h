#pragma once

#include <flecs.h>

typedef struct {
	float x; // Output: shared mouse position in the entity's local space (rotation and scale removed)
	float y;
	float pixel_x; // Output: local x units per screen pixel (1 when no camera is available)
	float pixel_y; // Output: local y units per screen pixel (1 when no camera is available)
	bool  inside;  // Output: the mouse is inside the entity's Rectangle (false without one)
} EgUiLocalMousePosition;

extern ECS_COMPONENT_DECLARE(EgUiLocalMousePosition);

void EgUiLocalMouseImport(ecs_world_t *world);
