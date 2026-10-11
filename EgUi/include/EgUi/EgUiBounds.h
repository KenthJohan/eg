#pragma once

#include <flecs.h>

#include "EgUi/EgUiButtons.h"

typedef struct {
	float   min_w;
	float   min_h;
	uint8_t edge; // Edges grabbed at drag start; zero when not dragging
	float   offset_x;
	float   offset_y;
} EgUiBoundsResizable;

// Requested outward growth per edge (negative shrinks); consumers decide whether and how much to honor.
typedef struct {
	float left;
	float right;
	float bottom;
	float top;
} EgUiBoundsEdgeRequest;

extern ECS_COMPONENT_DECLARE(EgUiBoundsResizable);
extern ECS_COMPONENT_DECLARE(EgUiBoundsEdgeRequest);

void EgUiBoundsImport(ecs_world_t *world);
