#pragma once

#include <flecs.h>

#include "EgUiCommon.h"

typedef struct {
	EgUiDir direction;               // Direction children advance along the primary axis
	EgUiDir wrap;                    // Direction the flow advances when starting a new line
	float         cursor_primary;    // Parent-local position along the primary axis
	float         cursor_wrap;       // Parent-local position along the wrap axis
	float         line_wrap_extent;  // Largest child size along the wrap axis on this line
	bool          line_has_children; // Whether the current line already contains a child
} EgUiFlowsFlow;

extern ECS_COMPONENT_DECLARE(EgUiFlowsFlow);
extern ECS_COMPONENT_DECLARE(EgUiDir);
extern ECS_TAG_DECLARE(EgUiFlowsFlowUnplaced);

void EgUiFlowsImport(ecs_world_t *world);
