#pragma once

#include <flecs.h>
#include <stdint.h>
#include <egmath.h>

// Atlas size the renderer bakes; the white texel at (0,0) gives solid fills.
#define EG_SHAPEDRAW_ATLAS_WIDTH  512
#define EG_SHAPEDRAW_ATLAS_HEIGHT 512

// Vertex layout shared with the egg renderer.
typedef struct {
	float   position[2];
	float   uv[2];
	uint8_t rgba[4];
} EgShapedrawVertex;

// Triangles drawn at one z level.
typedef struct {
	EgShapedrawVertex *data;
	int32_t            count;
	int32_t            capacity;
} EgShapedrawLayer;

// Put on a root entity; shapes that depend on it append triangles here each frame. Layer index is z, drawn in ascending order.
typedef struct {
	EgShapedrawLayer *layers;
	int32_t           layerCount;
	int32_t           layerCapacity;
	float             pixelScale; // World units per pixel; scales line thickness, point size and outlines.
} EgShapedrawList;

// Optional per-entity z level for collected shapes; defaults to 0.
typedef struct {
	int32_t z;
} EgShapedrawZ;

typedef struct {
	float x;
	float y;
} EgShapedrawVec2;

extern ECS_COMPONENT_DECLARE(EgShapedrawList);
extern ECS_COMPONENT_DECLARE(EgShapedrawZ);

// Standalone list (not an ECS component); free with EgShapedrawList_Destroy.
EgShapedrawList *EgShapedrawList_Create(void);

void EgShapedrawList_Destroy(EgShapedrawList *list);

// Drops all vertices but keeps the allocated capacity.
void EgShapedrawList_Clear(EgShapedrawList *list);

// Returns the layer for z, growing the list as needed. Negative z clamps to 0. NULL on allocation failure.
EgShapedrawLayer *EgShapedrawList_GetLayer(EgShapedrawList *list, int32_t z);

// Appends every layer of src to the same z level in dst.
void EgShapedrawList_Append(EgShapedrawList *dst, const EgShapedrawList *src);

void EgShapedrawList_SetPixelScale(EgShapedrawList *list, float pixelScale);

// Colors are 0xAARRGGBB with alpha 0 meaning opaque.
void EgShapedrawList_AddTriangle(EgShapedrawList *list, int32_t z, float x0, float y0, float x1, float y1, float x2, float y2, uint32_t color);

void EgShapedrawList_AddText(EgShapedrawList *list, int32_t z, const m3f32 *transform, float fontSize, uint32_t color, const char *string);

void EgShapedrawList_AddLine(EgShapedrawList *list, int32_t z, float x1, float y1, float x2, float y2, float thickness, uint32_t color);

void EgShapedrawList_AddPoint(EgShapedrawList *list, int32_t z, float x, float y, float size, uint32_t color);

void EgShapedrawList_AddCircle(EgShapedrawList *list, int32_t z, float x, float y, float radius, uint32_t color);

void EgShapedrawList_AddCircleOutline(EgShapedrawList *list, int32_t z, float x, float y, float radius, float thickness, uint32_t color);

void EgShapedrawList_AddCapsuleOutline(EgShapedrawList *list, int32_t z, float x1, float y1, float x2, float y2, float radius, float thickness, uint32_t color);

void EgShapedrawList_AddTransform(EgShapedrawList *list, int32_t z, float x, float y, float rotationCos, float rotationSin, float scale, uint32_t color);

void EgShapedrawList_AddRectangle(EgShapedrawList *list, int32_t z, float x, float y, float rotationCos, float rotationSin, float width, float height, uint32_t color);

void EgShapedrawList_AddRectangleOutline(EgShapedrawList *list, int32_t z, float x, float y, float rotationCos, float rotationSin, float width, float height, float thickness, uint32_t color);

void EgShapedrawList_AddBounds(EgShapedrawList *list, int32_t z, float minX, float minY, float maxX, float maxY, uint32_t color);

void EgShapedrawList_AddPolygon(EgShapedrawList *list, int32_t z, const EgShapedrawVec2 *vertices, int vertexCount, float tx, float ty, float rotationCos, float rotationSin, uint32_t color);

// Single-channel EG_SHAPEDRAW_ATLAS_WIDTH x EG_SHAPEDRAW_ATLAS_HEIGHT glyph bitmap, baked once; NULL if no font was found.
const unsigned char *EgShapedrawFont_GetBitmap(void);

void EgShapedrawImport(ecs_world_t *world);
