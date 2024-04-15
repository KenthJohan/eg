#pragma once

#include <sokol_gfx.h>

typedef struct
{
	float pos[4];
	uint32_t color;
} point_vertex_t;


typedef struct
{
	point_vertex_t *points;
	int32_t count;
	int32_t capacity;

	int32_t gpu_capacity;
	sg_buffer gpu_buffer;
} points_storage_t;

void points_init(points_storage_t *storage);
point_vertex_t * points_ensure(points_storage_t *storage, int32_t n);
point_vertex_t * points_append(points_storage_t *storage, int32_t n);
void points_upload(points_storage_t *storage);
void points_draw(points_storage_t *storage);
void points_reset(points_storage_t *storage);