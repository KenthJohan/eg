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

sshape_buffer_t ShapeBuffer_convert(EgBaseMemory2 * b)
{
	sshape_buffer_t buf = {
	.indices.buffer.ptr = b->mem[0].ptr,
	.indices.buffer.size = b->mem[0].cap,
	.indices.data_size = b->mem[0].size,
	.indices.shape_offset = b->mem[0].last,
	.vertices.buffer.ptr = b->mem[1].ptr,
	.vertices.buffer.size = b->mem[1].cap,
	.vertices.data_size = b->mem[1].size,
	.vertices.shape_offset = b->mem[1].last,
	};
	return buf;
}

void ShapeBuffer_append(EgBaseMemory2 *b, MyGraphicsDrawCommand *el, sshape_t shape, void *data, uint32_t color)
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
		Memory_grow(b->mem + 0, sizes.indices.size);
		Memory_grow(b->mem + 1, sizes.vertices.size);
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
		Memory_grow(b->mem + 0, sizes.indices.size);
		Memory_grow(b->mem + 1, sizes.vertices.size);
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
		Memory_grow(b->mem + 0, sizes.indices.size);
		Memory_grow(b->mem + 1, sizes.vertices.size);
		buf = ShapeBuffer_convert(b);
		buf = sshape_build_sphere(&buf, &info);
		break;
	}

	default:
		break;
	}

	b->mem[0].size = buf.indices.data_size;
	b->mem[0].last = buf.indices.shape_offset;
	b->mem[1].size = buf.vertices.data_size;
	b->mem[1].last = buf.vertices.shape_offset;

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

void ShapeBuffer_upload(EgBaseMemory2 *storage, EgBaseShapeBuffer * gbuf)
{
	upload(storage->mem + 0, &gbuf->ibuf, SG_BUFFERTYPE_INDEXBUFFER);
	upload(storage->mem + 1, &gbuf->vbuf, SG_BUFFERTYPE_VERTEXBUFFER);
}

void ShapeBuffer_reset(EgBaseMemory2 *storage)
{
	storage->mem[0].size = 0;
	storage->mem[0].last = 0;
	storage->mem[1].size = 0;
	storage->mem[1].last = 0;
}