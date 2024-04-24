#include "lines.h"
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

void lines_init(lines_storage_t *storage)
{
	storage->capacity = 1024;
	storage->gpu_capacity = storage->capacity;

	storage->lines = ecs_os_calloc_n(line_t, storage->capacity);

	const sg_buffer_desc desc = {
	.type = SG_BUFFERTYPE_VERTEXBUFFER,
	.usage = SG_USAGE_STREAM,
	.data.ptr = NULL,
	.data.size = storage->gpu_capacity * sizeof(line_t)};
	storage->gpu_buffer = sg_make_buffer(&desc);
}

void lines_append(lines_storage_t *storage, line_t const *line)
{
	storage->count++;
	if (storage->count > storage->capacity) {
		storage->capacity = next_pow_of_2(storage->count);
		storage->lines = ecs_os_realloc_n(storage->lines, line_t, storage->capacity);
	}
	line_t *last = storage->lines + storage->count - 1;
	(*last) = (*line);
}

void lines_upload(lines_storage_t *storage)
{
	if (storage->capacity > storage->gpu_capacity) {
		sg_destroy_buffer(storage->gpu_buffer);
		const sg_buffer_desc desc = {
		.type = SG_BUFFERTYPE_VERTEXBUFFER,
		.usage = SG_USAGE_STREAM,
		.data.ptr = NULL,
		.data.size = storage->capacity * sizeof(line_t)};
		storage->gpu_buffer = sg_make_buffer(&desc);
		storage->gpu_capacity = storage->capacity;
	}
	int32_t size = storage->count * sizeof(line_t);
	sg_update_buffer(storage->gpu_buffer, &(sg_range const){.ptr = storage->lines, .size = size});
}

void lines_draw(lines_storage_t *storage)
{
	// storage->count is number of lines, one line consist of two vertices.
	int32_t vertex_count = storage->count * 2;
	sg_draw(0, vertex_count, 1);
}


void lines_flush(lines_storage_t *storage)
{
	storage->count = 0;
}