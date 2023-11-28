#include "points.h"
#include <flecs.h>

static int32_t next_pow_of_2(int32_t n)
{
	n--;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	n++;
	return n;
}

void points_init(points_storage_t *storage)
{
	storage->capacity = 1024;
	storage->gpu_capacity = storage->capacity;

	storage->points = ecs_os_calloc_n(point_vertex_t, storage->capacity);

	const sg_buffer_desc desc = {
	.type = SG_BUFFERTYPE_VERTEXBUFFER,
	.usage = SG_USAGE_STREAM,
	.data.ptr = NULL,
	.data.size = storage->gpu_capacity * sizeof(point_vertex_t)};
	storage->gpu_buffer = sg_make_buffer(&desc);
}

point_vertex_t * points_append(points_storage_t *storage, int32_t n)
{
	int32_t count = storage->count + n;
	if (count > storage->capacity) {
		storage->capacity = next_pow_of_2(count);
		storage->points = ecs_os_realloc_n(storage->points, point_vertex_t, storage->capacity);
	}
	point_vertex_t *last = storage->points + storage->count;
	storage->count = count;
	return last;
}

void points_upload(points_storage_t *storage)
{
	if (storage->capacity > storage->gpu_capacity) {
		sg_destroy_buffer(storage->gpu_buffer);
		const sg_buffer_desc desc = {
		.type = SG_BUFFERTYPE_VERTEXBUFFER,
		.usage = SG_USAGE_STREAM,
		.data.ptr = NULL,
		.data.size = storage->capacity * sizeof(point_vertex_t)};
		storage->gpu_buffer = sg_make_buffer(&desc);
		storage->gpu_capacity = storage->capacity;
	}
	int32_t size = storage->count * sizeof(point_vertex_t);
	sg_update_buffer(storage->gpu_buffer, &(sg_range const){.ptr = storage->points, .size = size});
}

void points_draw(points_storage_t *storage)
{
	int32_t vertex_count = storage->count;
	sg_draw(0, vertex_count, 1);
}

void points_reset(points_storage_t *storage)
{
	storage->count = 0;
}