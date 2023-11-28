#include "MiscShapes.h"

#include <sokol/sokol_app.h>
#include <sokol/sokol_gfx.h>
#include <sokol/sokol_log.h>
#include <sokol/sokol_debugtext.h>
#include <sokol/sokol_glue.h>
#include <sokol/sokol_shape.h>
#include <eg/eg_fs.h>
#include <eg/Components.h>
#include <egsokol/Sg.h>

#include "shapes.h"


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
	SgPipeline *pipeline = ecs_field(it, SgPipeline, 3);               // up, shared
	ShapeBuffer *b = ecs_field(it, ShapeBuffer, 4);                    // up, shared
	Camera *cam = ecs_field(it, Camera, 5);                            // up, shared

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















void MiscShapesImport(ecs_world_t *world)
{
	ECS_MODULE(world, MiscShapes);
	ECS_IMPORT(world, Components);
	ECS_IMPORT(world, Sg);

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = AddShapeTorus,
	.query.filter.terms =
	{
	{.id = ecs_id(Torus), .src.flags = EcsSelf},
	{.id = ecs_id(ShapeBuffer), .src.trav = Use, .src.flags = EcsUp},
	{.id = ecs_id(ShapeElement), .oper = EcsNot}, // Adds this
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = AddShapeCylinder,
	.query.filter.terms =
	{
	{.id = ecs_id(Cylinder), .src.flags = EcsSelf},
	{.id = ecs_id(ShapeBuffer), .src.trav = Use, .src.flags = EcsUp},
	{.id = ecs_id(ShapeElement), .oper = EcsNot}, // Adds this
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
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

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = Update_GPU_Buffer,
	.query.filter.terms =
	{
	{.id = ecs_id(ShapeBuffer), .src.flags = EcsSelf},
	{.id = ecs_id(UpdateBuffer), .src.flags = EcsSelf},
	}});
}