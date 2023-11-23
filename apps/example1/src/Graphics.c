#include "Graphics.h"

#include <sokol/sokol_app.h>
#include <sokol/sokol_gfx.h>
#include <sokol/sokol_log.h>
#include <sokol/sokol_debugtext.h>
#include <sokol/sokol_glue.h>
#include <sokol/sokol_shape.h>
#include <eg/eg_fs.h>
#include <eg/Components.h>
#include <egsokol/Sg.h>

void DrawText(ecs_iter_t *it)
{
	Window *win = ecs_field(it, Window, 1);
	Position2 *p = ecs_field(it, Position2, 2);
	Color *c = ecs_field(it, Color, 3);
	String *s = ecs_field(it, String, 4);

	sdtx_canvas(win->w * 0.5f, win->h * 0.5f);
	for (int i = 0; i < it->count; i++) {
		sdtx_origin(p->x, p->y);
		sdtx_color3f(c->r, c->g, c->b);
		sdtx_puts(s->value);
	}
}

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

typedef enum {
	SSHAPE_TORUS,
	SSHAPE_BOX,
	SSHAPE_CYLINDER,
	SSHAPE_COUNT,
} sshape_t;

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

void ShapeBuffer_append(ShapeBuffer *b, ShapeElement *el, sshape_t shape, void *data)
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
		    .random_colors = true,
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
		    .random_colors = true,
		};
		sshape_sizes_t sizes = sshape_cylinder_sizes(cylinder->slices, cylinder->stacks);
		Memory_grow(&b->vertices.buffer, sizes.vertices.size);
		Memory_grow(&b->indices.buffer, sizes.indices.size);
		buf = ShapeBuffer_convert(b);
		buf = sshape_build_cylinder(&buf, &info);
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

void AddShapeTorus(ecs_iter_t *it)
{
	Torus *torus = ecs_field(it, Torus, 1);         // self
	ecs_entity_t parent = ecs_field_src(it, 2);     // shared
	ShapeBuffer *b = ecs_field(it, ShapeBuffer, 2); // shared
	for (int i = 0; i < it->count; ++i) {
		ShapeElement el;
		ShapeBuffer_append(b, &el, SSHAPE_TORUS, torus);
		ecs_set(it->world, it->entities[i], ShapeElement, {.base_element = el.base_element, .num_elements = el.num_elements});
	}
	ecs_add(it->world, parent, UpdateBuffer);
}

void AddShapeCylinder(ecs_iter_t *it)
{
	Cylinder *cylinder = ecs_field(it, Cylinder, 1); // self
	ecs_entity_t parent = ecs_field_src(it, 2);      // shared
	ShapeBuffer *b = ecs_field(it, ShapeBuffer, 2);  // shared
	for (int i = 0; i < it->count; ++i) {
		ShapeElement el;
		ShapeBuffer_append(b, &el, SSHAPE_CYLINDER, cylinder);
		ecs_set(it->world, it->entities[i], ShapeElement, {.base_element = el.base_element, .num_elements = el.num_elements});
	}
	ecs_add(it->world, parent, UpdateBuffer);
}

void Update_GPU_Buffer(ecs_iter_t *it)
{
	ShapeBuffer *b = ecs_field(it, ShapeBuffer, 1);
	for (int i = 0; i < it->count; ++i, ++b) {
		sg_destroy_buffer((sg_buffer){b->vbuf_id});
		sg_destroy_buffer((sg_buffer){b->ibuf_id});
		const sg_buffer_desc vbuf_desc = {
		    .type = SG_BUFFERTYPE_VERTEXBUFFER,
		    .usage = SG_USAGE_IMMUTABLE,
		    .data.ptr = b->vertices.buffer.ptr,
		    .data.size = b->vertices.data_size};
		const sg_buffer_desc ibuf_desc = {
		    .type = SG_BUFFERTYPE_INDEXBUFFER,
		    .usage = SG_USAGE_IMMUTABLE,
		    .data.ptr = b->indices.buffer.ptr,
		    .data.size = b->indices.data_size};
		b->vbuf_id = sg_make_buffer(&vbuf_desc).id;
		b->ibuf_id = sg_make_buffer(&ibuf_desc).id;
		ecs_remove(it->world, it->entities[i], UpdateBuffer);
	}
}

typedef struct {
	float mvp[16];
	float extra[4];
} vs_params_t;

void DrawShape(ecs_iter_t *it)
{
	ShapeElement *element = ecs_field(it, ShapeElement, 1);            // self
	Transformation *transformation = ecs_field(it, Transformation, 2); // self
	SgPipeline *pipeline = ecs_field(it, SgPipeline, 3);               // shared
	ShapeBuffer *b = ecs_field(it, ShapeBuffer, 4);                    // shared
	Camera *cam = ecs_field(it, Camera, 5);                            // shared

	sg_apply_pipeline(pipeline->id);
	sg_apply_bindings(&(sg_bindings){
	    .vertex_buffers[0] = (sg_buffer){b->vbuf_id},
	    .index_buffer = (sg_buffer){b->ibuf_id}});

	for (int i = 0; i < it->count; i++) {
		vs_params_t params = {0};
		m4f32_mul((m4f32 *)params.mvp, &cam->vp, &transformation->matrix);
		// m4f32_print(&t);
		sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &SG_RANGE(params));
		sg_draw(element->base_element, element->num_elements, 1);
	}
}

void GraphicsImport(ecs_world_t *world)
{
	ECS_MODULE(world, Graphics);
	ECS_IMPORT(world, Components);
	ECS_IMPORT(world, Sg);

	ECS_SYSTEM(world, DrawText, EcsOnUpdate, Window($), Position2, Color, String);

	ecs_system_init(world, &(ecs_system_desc_t){
	                           .entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	                           .callback = AddShapeTorus,
	                           .query.filter.terms =
	                               {
	                                   {.id = ecs_id(Torus), .src.flags = EcsSelf},
	                                   {.id = ecs_id(ShapeBuffer), .src.trav = Use, .src.flags = EcsUp},
	                                   {.id = ecs_id(ShapeElement), .oper = EcsNot}, // Adds this
	                               }});

	ecs_system_init(world, &(ecs_system_desc_t){
	                           .entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	                           .callback = AddShapeCylinder,
	                           .query.filter.terms =
	                               {
	                                   {.id = ecs_id(Cylinder), .src.flags = EcsSelf},
	                                   {.id = ecs_id(ShapeBuffer), .src.trav = Use, .src.flags = EcsUp},
	                                   {.id = ecs_id(ShapeElement), .oper = EcsNot}, // Adds this
	                               }});

	ecs_system_init(world, &(ecs_system_desc_t){
	                           .entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	                           .callback = DrawShape,
	                           .query.filter.terms =
	                               {
	                                   {.id = ecs_id(ShapeElement), .src.flags = EcsSelf},
	                                   {.id = ecs_id(Transformation), .src.flags = EcsSelf},
	                                   {.id = ecs_id(SgPipeline), .src.trav = Use, .src.flags = EcsUp},
	                                   {.id = ecs_id(ShapeBuffer), .src.trav = Use, .src.flags = EcsUp},
	                                   {.id = ecs_id(Camera), .src.trav = Use, .src.flags = EcsUp},
	                                   {.id = ecs_id(UpdateBuffer), .src.trav = Use, .src.flags = EcsUp, .oper = EcsNot},
	                               }});

	ECS_SYSTEM(world, Update_GPU_Buffer, EcsOnUpdate, ShapeBuffer, UpdateBuffer);
}