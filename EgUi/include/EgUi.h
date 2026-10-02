#pragma once

#include <flecs.h>
#include <EgSpatials.h>

typedef enum {
	EgUiDirectionNone,
	EgUiDirectionRight,
	EgUiDirectionLeft,
	EgUiDirectionUp,
	EgUiDirectionDown,
} EgUiDirection;

extern ECS_COMPONENT_DECLARE(EgUiDirection);

typedef struct {
	EgUiDirection direction; // The primary direction of the UI flow
	EgUiDirection wrap;      // The direction to wrap the UI flow when reaching the end
	float cursor_primary;
	float cursor_wrap;
	float line_wrap_extent;
	bool line_has_children;
} EgUiFlow;

extern ECS_COMPONENT_DECLARE(EgUiFlow);

void EgUiImport(ecs_world_t *world);
