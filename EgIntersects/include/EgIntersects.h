#pragma once

#include <flecs.h>

typedef struct {
	bool overlap;
} EgIntersectsOverlap;

enum {
	EG_INTERSECTS_EDGE_LEFT   = 1,
	EG_INTERSECTS_EDGE_RIGHT  = 2,
	EG_INTERSECTS_EDGE_BOTTOM = 4,
	EG_INTERSECTS_EDGE_TOP    = 8,
};

typedef struct {
	float   grab;    // Input: edge hit width in rectangle-local units
	uint8_t edges;   // Output: overlapped border sides, EG_INTERSECTS_EDGE_* bits
	float   local_x; // Output: point in rectangle-local space (rotation and scale removed)
	float   local_y;
} EgIntersectsRectangleBorder;

extern ECS_COMPONENT_DECLARE(EgIntersectsOverlap);
extern ECS_COMPONENT_DECLARE(EgIntersectsRectangleBorder);

void EgIntersectsImport(ecs_world_t *world);
