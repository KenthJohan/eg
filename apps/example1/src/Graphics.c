#include "Graphics.h"

#include <sokol/sokol_app.h>
#include <sokol/sokol_gfx.h>
#include <sokol/sokol_log.h>
#include <sokol/sokol_debugtext.h>
#include <sokol/sokol_glue.h>
#include <sokol/sokol_shape.h>
#include <eg/eg_fs.h>
#include <eg/Components.h>

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

static sg_pipeline global_pip;

void init_pipeline()
{
	char const *vs = eg_fs_readfile("shapes.vs.glsl");
	char const *fs = eg_fs_readfile("shapes.fs.glsl");
	assert(vs);
	assert(fs);

	static sg_shader_desc desc;
	desc.attrs[0].name = "position";
	desc.attrs[1].name = "normal";
	desc.attrs[2].name = "texcoord";
	desc.attrs[3].name = "color0";
	desc.vs.source = vs;
	desc.vs.entry = "main";
	desc.vs.uniform_blocks[0].size = 80;
	desc.vs.uniform_blocks[0].layout = SG_UNIFORMLAYOUT_STD140;
	desc.vs.uniform_blocks[0].uniforms[0].name = "vs_params";
	desc.vs.uniform_blocks[0].uniforms[0].type = SG_UNIFORMTYPE_FLOAT4;
	desc.vs.uniform_blocks[0].uniforms[0].array_count = 5;
	desc.fs.source = fs;
	desc.fs.entry = "main";
	desc.label = "shapes_shader1";
	sg_shader shd = sg_make_shader(&desc);

	// shader and pipeline object
	global_pip = sg_make_pipeline(&(sg_pipeline_desc){
	    .shader = shd,
	    .layout = {
	        .buffers[0] = sshape_vertex_buffer_layout_state(),
	        .attrs = {
	            [0] = sshape_position_vertex_attr_state(),
	            [1] = sshape_normal_vertex_attr_state(),
	            [2] = sshape_texcoord_vertex_attr_state(),
	            [3] = sshape_color_vertex_attr_state()}},
	    .index_type = SG_INDEXTYPE_UINT16,
	    .cull_mode = SG_CULLMODE_NONE,
	    .depth = {.compare = SG_COMPAREFUNC_LESS_EQUAL, .write_enabled = true},
	});
}

typedef enum {
	SSHAPE_TORUS,
	SSHAPE_BOX,
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

void AddShape(ecs_iter_t *it)
{
	ecs_entity_t parent = ecs_field_src(it, 1);
	ShapeBuffer *b = ecs_field(it, ShapeBuffer, 1); // shared
	Torus *torus = ecs_field(it, Torus, 2);

	for (int i = 0; i < it->count; ++i) {
		ShapeElement el;
		ShapeBuffer_append(b, &el, SSHAPE_TORUS, torus);
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
	float draw_mode;
	uint8_t _pad_4[12];
	float mvp[4 * 4];
} vs_params_t;

void DrawShape(ecs_iter_t *it)
{
	ShapeBuffer *b = ecs_field(it, ShapeBuffer, 1); // shared
	ShapeElement *element = ecs_field(it, ShapeElement, 2);
	Transformation *transformation = ecs_field(it, Transformation, 3);
	Camera *cam = ecs_field(it, Camera, 4);

	sg_apply_pipeline(global_pip);
	sg_apply_bindings(&(sg_bindings){
	    .vertex_buffers[0] = (sg_buffer){b->vbuf_id},
	    .index_buffer = (sg_buffer){b->ibuf_id}});

	for (int i = 0; i < it->count; i++) {
		// memcpy(state->vs_params.mvp, vp, sizeof(m4f32));
		// m4f32 model = M4_IDENTITY;
		// m4f32_translation3(&model, (float *)(state->positions + i));
		// m4f32_mul((m4f32 *)state->vs_params.mvp, vp, &model);
		vs_params_t params;
		m4f32 t;
		m4f32_mul(&t, &cam->vp, &transformation->matrix);
		//m4f32_print(&t);
		memcpy(params.mvp, &t, sizeof(m4f32));
		sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &SG_RANGE(params));
		sg_draw(element->base_element, element->num_elements, 1);
	}
}

void DrawStuff(ecs_iter_t *it)
{
	Window *window = ecs_field(it, Window, 1);
}

void GraphicsImport(ecs_world_t *world)
{
	ECS_MODULE(world, Graphics);
	ECS_IMPORT(world, Components);

	init_pipeline();

	ECS_SYSTEM(world, DrawText, EcsOnUpdate, Window($), Position2, Color, String);

	ECS_SYSTEM(world, AddShape, EcsOnUpdate, ShapeBuffer(parent), Torus, !ShapeElement);
	ECS_SYSTEM(world, DrawShape, EcsOnUpdate, ShapeBuffer(parent), ShapeElement, Transformation, Camera(up(components.Use)), !UpdateBuffer(parent));

	ECS_SYSTEM(world, Update_GPU_Buffer, EcsOnUpdate, ShapeBuffer, UpdateBuffer);

	ECS_SYSTEM(world, DrawStuff, EcsOnUpdate, Window);
}