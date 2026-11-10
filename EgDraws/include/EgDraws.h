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

// Collector colors are 0xAARRGGBB; alpha 0 is treated as opaque.
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

// Shape generation is internal; the former EgDraws_List_Add* functions are no longer provided.
// Returns the vertex vector for z, growing the list as needed. Negative z clamps to 0. NULL on allocation failure.
ecs_vec_t *EgDraws_List_GetLayer(EgDraws_List *list, int32_t z);

// Appends every layer of src to the same z level in dst.
void EgDraws_List_Append(EgDraws_List *dst, const EgDraws_List *src);

void EgDraws_List_SetPixelScale(EgDraws_List *list, float pixelScale);

// Single-channel EG_DRAWS_ATLAS_WIDTH x EG_DRAWS_ATLAS_HEIGHT glyph bitmap, baked once; NULL if no font was found.
const unsigned char *EgDraws_Font_GetBitmap(void);

void EgDraws_Import(ecs_world_t *world);
