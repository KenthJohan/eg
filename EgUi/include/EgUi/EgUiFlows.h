#pragma once

#include <flecs.h>

typedef enum {
	EgUiDirectionNone,
	EgUiDirectionRight,
	EgUiDirectionLeft,
	EgUiDirectionUp,
	EgUiDirectionDown,
} EgUiDirection;

typedef struct {
	EgUiDirection direction;         // Direction children advance along the primary axis
	EgUiDirection wrap;              // Direction the flow advances when starting a new line
	float         cursor_primary;    // Parent-local position along the primary axis
	float         cursor_wrap;       // Parent-local position along the wrap axis
	float         line_wrap_extent;  // Largest child size along the wrap axis on this line
	bool          line_has_children; // Whether the current line already contains a child
} EgUiFlowsFlow;

extern ECS_COMPONENT_DECLARE(EgUiFlowsFlow);
extern ECS_COMPONENT_DECLARE(EgUiDirection);
extern ECS_TAG_DECLARE(EgUiFlowsFlowUnplaced);

void EgUiFlowsImport(ecs_world_t *world);
