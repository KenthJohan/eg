#pragma once

#include "EgDraws.h"

typedef struct {
	float pixscale;
	m3f32 transform;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
} verts_config_t;

/* Append EgDraws_Vertex values to an initialized vector; RGBA is used verbatim.
 * A non-positive pixscale is treated as 1.0 world unit per pixel. */
void verts_add_triangle(ecs_vec_t *v, verts_config_t const *config, float x0, float y0, float x1, float y1, float x2, float y2);
void verts_add_text(ecs_vec_t *v, verts_config_t const *config, float fontSize, const char *string);
void verts_add_line(ecs_vec_t *v, verts_config_t const *config, float x1, float y1, float x2, float y2, float thickness);
void verts_add_point(ecs_vec_t *v, verts_config_t const *config, float size);
void verts_add_circle(ecs_vec_t *v, verts_config_t const *config, float radius);
void verts_add_circle_outline(ecs_vec_t *v, verts_config_t const *config, float radius, float thickness);
void verts_add_capsule_outline(ecs_vec_t *v, verts_config_t const *config, float x1, float y1, float x2, float y2, float radius, float thickness);
void verts_add_transform(ecs_vec_t *v, verts_config_t const *config, float scale);
void verts_add_rectangle(ecs_vec_t *v, verts_config_t const *config, float width, float height);
void verts_add_rectangle_outline(ecs_vec_t *v, verts_config_t const *config, float width, float height, float thickness);
void verts_add_bounds(ecs_vec_t *v, verts_config_t const *config, float minX, float minY, float maxX, float maxY);
void verts_add_polygon(ecs_vec_t *v, verts_config_t const *config, const EgDraws_Vec2 *vertices, int vertexCount);
