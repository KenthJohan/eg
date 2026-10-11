#pragma once

#include <flecs.h>

typedef struct {
	float x; // Output: shared mouse position in the entity's local space (rotation and scale removed)
	float y;
} EgUiLocalMousePosition;

extern ECS_COMPONENT_DECLARE(EgUiLocalMousePosition);

void EgUiLocalMouseImport(ecs_world_t *world);
