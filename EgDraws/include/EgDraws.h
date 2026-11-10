#pragma once

#include <flecs.h>
#include <stdint.h>
#include <egmath.h>

// Atlas size the renderer bakes; the white texel at (0,0) gives solid fills.
#define EG_DRAWS_ATLAS_WIDTH  512
#define EG_DRAWS_ATLAS_HEIGHT 512

// Vertex layout shared with the egg renderer.
typedef struct {
	float   position[2];
	float   uv[2];
	uint8_t rgba[4];
} EgDraws_Vertex;

// Put on a root entity; shapes that depend on it append triangles here each frame. Layer index is z, drawn in ascending order.
typedef struct {
	ecs_vec_t layers; // ecs_vec_t<ecs_vec_t<EgDraws_Vertex>>
	float             pixelScale; // World units per pixel; scales line thickness, point size and outlines.
} EgDraws_List;

// Optional per-entity z level for collected shapes; defaults to 0.
typedef struct {
	int32_t z;
} EgDraws_Z;

typedef struct {
	float x;
	float y;
} EgDraws_Vec2;

typedef struct {
	float    thickness;
	uint32_t color;
} EgDraws_Border;

typedef struct {
	uint32_t color;
} EgDraws_Solid;

extern ECS_COMPONENT_DECLARE(EgDraws_List);
extern ECS_COMPONENT_DECLARE(EgDraws_Z);
extern ECS_COMPONENT_DECLARE(EgDraws_Border);
extern ECS_COMPONENT_DECLARE(EgDraws_Solid);

// Standalone list (not an ECS component); free with EgDraws_List_Destroy.
EgDraws_List *EgDraws_List_Create(void);

void EgDraws_List_Destroy(EgDraws_List *list);

// Drops all vertices but keeps the allocated capacity.
void EgDraws_List_Clear(EgDraws_List *list);

// Returns the vertex vector for z, growing the list as needed. Negative z clamps to 0. NULL on allocation failure.
ecs_vec_t *EgDraws_List_GetLayer(EgDraws_List *list, int32_t z);

// Appends every layer of src to the same z level in dst.
void EgDraws_List_Append(EgDraws_List *dst, const EgDraws_List *src);

void EgDraws_List_SetPixelScale(EgDraws_List *list, float pixelScale);

// Colors are 0xAARRGGBB with alpha 0 meaning opaque.
void EgDraws_List_AddTriangle(EgDraws_List *list, int32_t z, const m3f32 *transform, float x0, float y0, float x1, float y1, float x2, float y2, uint32_t color);

void EgDraws_List_AddText(EgDraws_List *list, int32_t z, const m3f32 *transform, float fontSize, uint32_t color, const char *string); // fontSize in pixels

void EgDraws_List_AddLine(EgDraws_List *list, int32_t z, const m3f32 *transform, float x1, float y1, float x2, float y2, float thickness, uint32_t color);

void EgDraws_List_AddPoint(EgDraws_List *list, int32_t z, const m3f32 *transform, float size, uint32_t color);

void EgDraws_List_AddCircle(EgDraws_List *list, int32_t z, const m3f32 *transform, float radius, uint32_t color);

void EgDraws_List_AddCircleOutline(EgDraws_List *list, int32_t z, const m3f32 *transform, float radius, float thickness, uint32_t color);

void EgDraws_List_AddCapsuleOutline(EgDraws_List *list, int32_t z, const m3f32 *transform, float x1, float y1, float x2, float y2, float radius, float thickness, uint32_t color);

void EgDraws_List_AddTransform(EgDraws_List *list, int32_t z, const m3f32 *transform, float scale, uint32_t color);

void EgDraws_List_AddRectangle(EgDraws_List *list, int32_t z, const m3f32 *transform, float width, float height, uint32_t color);

void EgDraws_List_AddRectangleOutline(EgDraws_List *list, int32_t z, const m3f32 *transform, float width, float height, float thickness, uint32_t color);

void EgDraws_List_AddBounds(EgDraws_List *list, int32_t z, const m3f32 *transform, float minX, float minY, float maxX, float maxY, uint32_t color);

void EgDraws_List_AddPolygon(EgDraws_List *list, int32_t z, const m3f32 *transform, const EgDraws_Vec2 *vertices, int vertexCount, uint32_t color);

// Single-channel EG_DRAWS_ATLAS_WIDTH x EG_DRAWS_ATLAS_HEIGHT glyph bitmap, baked once; NULL if no font was found.
const unsigned char *EgDraws_Font_GetBitmap(void);

void EgDraws_Import(ecs_world_t *world);
