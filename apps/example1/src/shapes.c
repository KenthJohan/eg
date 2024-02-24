#include "shapes.h"

#include "sokol/sokol_gfx.h"
#include "sokol/sokol_shape.h"

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

void Memory_grow(Memory *m, int32_t inc)
{
	m->size = m->size + inc;
	if (m->size > m->cap) {
		m->cap = Memory_next_pow_of_2(m->size);
		m->ptr = ecs_os_realloc(m->ptr, m->cap);
	}
}

sshape_buffer_t ShapeBuffer_convert(ShapeBuffer *b)
{
	sshape_buffer_t buf = {
	.vertices.buffer.ptr = b->vertices.buffer.ptr,
	.vertices.buffer.size = b->vertices.buffer.size,
	.vertices.data_size = b->vertices.data_size,
	.vertices.shape_offset = b->vertices.shape_offset,

	.indices.buffer.ptr = b->indices.buffer.ptr,
	.indices.buffer.size = b->indices.buffer.size,
	.indices.data_size = b->indices.data_size,
	.indices.shape_offset = b->indices.shape_offset,
	};
	return buf;
}

void ShapeBuffer_append(ShapeBuffer *b, ShapeElement *el, sshape_t shape, void *data, uint32_t color)
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
		.random_colors = false
		};
		sshape_sizes_t sizes = sshape_torus_sizes(torus->sides, torus->rings);
		Memory_grow(&b->vertices.buffer, sizes.vertices.size);
		Memory_grow(&b->indices.buffer, sizes.indices.size);
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
		.random_colors = false
		};
		sshape_sizes_t sizes = sshape_cylinder_sizes(cylinder->slices, cylinder->stacks);
		Memory_grow(&b->vertices.buffer, sizes.vertices.size);
		Memory_grow(&b->indices.buffer, sizes.indices.size);
		buf = ShapeBuffer_convert(b);
		buf = sshape_build_cylinder(&buf, &info);
		break;
	}

	case SSHAPE_SPHERE: {
		Sphere *cylinder = data;
		sshape_sphere_t info = {
		.radius = cylinder->radius,
		.slices = cylinder->slices,
		.stacks = cylinder->stacks,
		.color = color,
		.random_colors = false
		};
		sshape_sizes_t sizes = sshape_sphere_sizes(cylinder->slices, cylinder->stacks);
		Memory_grow(&b->vertices.buffer, sizes.vertices.size);
		Memory_grow(&b->indices.buffer, sizes.indices.size);
		buf = ShapeBuffer_convert(b);
		buf = sshape_build_sphere(&buf, &info);
		break;
	}

	default:
		break;
	}

	b->indices.data_size = buf.indices.data_size;
	b->indices.shape_offset = buf.indices.shape_offset;
	b->vertices.data_size = buf.vertices.data_size;
	b->vertices.shape_offset = buf.vertices.shape_offset;

	sshape_element_range_t element = sshape_element_range(&buf);
	el->base_element = element.base_element;
	el->num_elements = element.num_elements;
}

static void upload(Memory *mem, MemoryGPU *gpu, sg_buffer_type type)
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

void ShapeBuffer_upload(ShapeBuffer *storage)
{
	upload(&storage->vertices.buffer, &storage->vbuf, SG_BUFFERTYPE_VERTEXBUFFER);
	upload(&storage->indices.buffer, &storage->ibuf, SG_BUFFERTYPE_INDEXBUFFER);
}


void ShapeBuffer_reset(ShapeBuffer *storage)
{
	storage->indices.data_size = 0;
	storage->indices.shape_offset = 0;
	storage->vertices.data_size = 0;
	storage->vertices.shape_offset = 0;
	storage->indices.buffer.size = 0;
	storage->vertices.buffer.size = 0;
}