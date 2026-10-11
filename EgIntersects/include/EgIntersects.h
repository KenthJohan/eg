#pragma once

#include <flecs.h>

typedef struct {
	float left;    // Output: signed distance from the point to the rectangle's left edge
	float right;   // Output: signed distance from the point to the rectangle's right edge
	float bottom;  // Output: signed distance from the point to the rectangle's bottom edge
	float top;     // Output: signed distance from the point to the rectangle's top edge
	float minimum; // Output: smallest signed distance to any rectangle edge
} EgIntersectsRectangleBorder;

#define EG_INTERSECTS_RECTANGLE_BORDER_IS_INSIDE(border) \
	((border).left >= 0.0f && (border).right >= 0.0f &&  \
	 (border).bottom >= 0.0f && (border).top >= 0.0f)

extern ECS_COMPONENT_DECLARE(EgIntersectsRectangleBorder);

void EgIntersectsImport(ecs_world_t *world);
