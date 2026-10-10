#pragma once

#include <flecs.h>

enum {
	EG_INTERSECTS_EDGE_LEFT   = 1,
	EG_INTERSECTS_EDGE_RIGHT  = 2,
	EG_INTERSECTS_EDGE_BOTTOM = 4,
	EG_INTERSECTS_EDGE_TOP    = 8,
};

typedef struct {
	float   margin;  // Input: edge hit width in rectangle-local units
	uint8_t edges;   // Output: overlapped border sides, EG_INTERSECTS_EDGE_* bits
	float   local_x; // Output: point in rectangle-local space (rotation and scale removed)
	float   local_y;
	float   left;    // Output: signed distance from the point to the rectangle's left edge
	float   right;   // Output: signed distance from the point to the rectangle's right edge
	float   bottom;  // Output: signed distance from the point to the rectangle's bottom edge
	float   top;     // Output: signed distance from the point to the rectangle's top edge
} EgIntersectsRectangleBorder;

#define EG_INTERSECTS_RECTANGLE_BORDER_IS_INSIDE(border) \
	((border).left >= 0.0f && (border).right >= 0.0f && \
	 (border).bottom >= 0.0f && (border).top >= 0.0f)

extern ECS_COMPONENT_DECLARE(EgIntersectsRectangleBorder);

void EgIntersectsImport(ecs_world_t *world);
