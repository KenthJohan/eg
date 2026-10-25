#pragma once

#include <flecs.h>

#include "EgUi/EgUiButtons.h"

typedef struct {
	float   min_w;
	float   min_h;
} EgUiBoundsResizable;

// One-shot request to move edges to absolute positions in the entity's local space (the space of its
// Rectangle, origin at the center). Only edges set in `edges` (EG_UI_EDGE_* mask) are read; consumers
// decide whether and how much to honor. Producers write it, EgUiBoundsEdgeRequest_Reset clears it
// after the Apply systems ran.
typedef struct {
	uint32_t edges;
	float    left;
	float    right;
	float    bottom;
	float    top;
} EgUiBoundsEdgeRequest;

extern ECS_COMPONENT_DECLARE(EgUiBoundsResizable);
extern ECS_COMPONENT_DECLARE(EgUiBoundsEdgeRequest);

void EgUiBoundsImport(ecs_world_t *world);
