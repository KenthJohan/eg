#pragma once

#include <flecs.h>
#include <EgSpatials.h>
#include <EgShapes.h>

#include "EgUi/EgUiButtons.h"
#include "EgUi/EgUiFlows.h"
#include "EgUi/EgUiBounds.h"
#include "EgUi/EgUiFreeforms.h"

typedef struct {
	float left;   // Signed distance from the child's left edge to the parent's left edge
	float right;  // Signed distance from the parent's right edge to the child's right edge
	float bottom; // Signed distance from the child's bottom edge to the parent's bottom edge
	float top;    // Signed distance from the parent's top edge to the child's top edge
} EgUiParentClearance;

extern ECS_COMPONENT_DECLARE(EgUiParentClearance);

void EgUiImport(ecs_world_t *world);
