#pragma once

#include <sokol_gfx.h>

typedef struct
{
	float pos[4];
	uint32_t color;
} line_vertex_t;

typedef struct
{
	line_vertex_t a;
	line_vertex_t b;
} line_t;

typedef struct
{
	line_t *lines;
	int32_t count;
	int32_t capacity;

	int32_t gpu_capacity;
	sg_buffer gpu_buffer;
} lines_storage_t;

void lines_init(lines_storage_t *storage);
void lines_append(lines_storage_t *storage, line_t const *line);
void lines_upload(lines_storage_t *storage);
void lines_draw(lines_storage_t *storage);
void lines_flush(lines_storage_t *storage);