#pragma once

#include <flecs.h>
#include <stdint.h>

// Atlas size the renderer bakes; the white texel at (0,0) gives solid fills.
#define EG_SHAPEDRAW_ATLAS_WIDTH  512
#define EG_SHAPEDRAW_ATLAS_HEIGHT 512

// Vertex layout shared with the egg renderer.
typedef struct {
	float   position[2];
	float   uv[2];
	uint8_t rgba[4];
} EgShapedrawVertex;

// Put on a root entity; rectangles that depend on it append triangles here each frame.
typedef struct {
	EgShapedrawVertex *data;
	int32_t            count;
	int32_t            capacity;
} EgShapedrawList;

extern ECS_COMPONENT_DECLARE(EgShapedrawList);

// Appends one solid triangle; color is 0xAARRGGBB with alpha 0 meaning opaque.
void EgShapedrawList_AddTriangle(EgShapedrawList *list, float x0, float y0, float x1, float y1, float x2, float y2, uint32_t color);

void EgShapedrawImport(ecs_world_t *world);
