#include "shapes.h"

#include <sokol_gfx.h>
#include <sokol_shape.h>

int32_t Memory_next_pow_of_2(int32_t n)
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

void Memory_grow(EgBaseMemory *m, int32_t inc)
{
	int32_t cap = m->size + inc;
	if (cap > m->cap) {
		m->cap = Memory_next_pow_of_2(cap);
		m->ptr = ecs_os_realloc(m->ptr, m->cap);
	}
}

sshape_buffer_t ShapeBuffer_convert(EgBaseShapeBuffer *b)
{
	sshape_buffer_t buf = {
	.vertices.buffer.ptr = b->vertices.ptr,
	.vertices.buffer.size = b->vertices.cap,
	.vertices.data_size = b->vertices.size,
	.vertices.shape_offset = b->vertices.last,

	.indices.buffer.ptr = b->indices.ptr,
	.indices.buffer.size = b->indices.cap,
	.indices.data_size = b->indices.size,
	.indices.shape_offset = b->indices.last,
	};
	return buf;
}

void ShapeBuffer_append(EgBaseShapeBuffer *b, MyGraphicsDrawCommand *el, sshape_t shape, void *data, uint32_t color)
{
	sshape_buffer_t buf;
	switch (shape) {
	case SSHAPE_TORUS: {
		Torus *torus = data;
		sshape_torus_t info = {
		.radius = torus->radius,
		.ring_radius = torus->ring_radius,
		.rings = torus->rings,
		.sides = torus->sides,
		.color = color,
		.random_colors = torus->random_colors};
		sshape_sizes_t sizes = sshape_torus_sizes(torus->sides, torus->rings);
		Memory_grow(&b->vertices, sizes.vertices.size);
		Memory_grow(&b->indices, sizes.indices.size);
		buf = ShapeBuffer_convert(b);
		buf = sshape_build_torus(&buf, &info);
		break;
	}

	case SSHAPE_CYLINDER: {
		Cylinder *cylinder = data;
		sshape_cylinder_t info = {
		.radius = cylinder->radius,
		.height = cylinder->height,
		.slices = cylinder->slices,
		.stacks = cylinder->stacks,
		.color = color,
		.random_colors = cylinder->random_colors};
		sshape_sizes_t sizes = sshape_cylinder_sizes(cylinder->slices, cylinder->stacks);
		Memory_grow(&b->vertices, sizes.vertices.size);
		Memory_grow(&b->indices, sizes.indices.size);
		buf = ShapeBuffer_convert(b);
		buf = sshape_build_cylinder(&buf, &info);
		break;
	}

	case SSHAPE_SPHERE: {
		Sphere *sphere = data;
		sshape_sphere_t info = {
		.radius = sphere->radius,
		.slices = sphere->slices,
		.stacks = sphere->stacks,
		.color = color,
		.random_colors = sphere->random_colors};
		sshape_sizes_t sizes = sshape_sphere_sizes(sphere->slices, sphere->stacks);
		Memory_grow(&b->vertices, sizes.vertices.size);
		Memory_grow(&b->indices, sizes.indices.size);
		buf = ShapeBuffer_convert(b);
		buf = sshape_build_sphere(&buf, &info);
		break;
	}

	default:
		break;
	}

	b->indices.size = buf.indices.data_size;
	b->indices.last = buf.indices.shape_offset;
	b->vertices.size = buf.vertices.data_size;
	b->vertices.last = buf.vertices.shape_offset;

	sshape_element_range_t element = sshape_element_range(&buf);
	el->offset = element.base_element;
	el->count = element.num_elements;
	el->instances = 1;
}

static void upload(EgBaseMemory *mem, EgBaseMemoryGPU *gpu, sg_buffer_type type)
{
	if (mem->cap > gpu->cap) {
		sg_destroy_buffer((sg_buffer){gpu->id});
		const sg_buffer_desc desc = {
		.type = type,
		.usage = SG_USAGE_STREAM,
		.data.ptr = NULL,
		.data.size = mem->cap};
		gpu->id = sg_make_buffer(&desc).id;
		gpu->cap = mem->cap;
	}
	sg_update_buffer((sg_buffer){gpu->id}, &(sg_range const){.ptr = mem->ptr, .size = mem->size});
}

void ShapeBuffer_upload(EgBaseShapeBuffer *storage)
{
	upload(&storage->vertices, &storage->vbuf, SG_BUFFERTYPE_VERTEXBUFFER);
	upload(&storage->indices, &storage->ibuf, SG_BUFFERTYPE_INDEXBUFFER);
}

void ShapeBuffer_reset(EgBaseShapeBuffer *storage)
{
	storage->indices.size = 0;
	storage->indices.last = 0;
	storage->vertices.size = 0;
	storage->vertices.last = 0;
}