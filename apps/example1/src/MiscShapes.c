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
	Torus *torus = ecs_field(it, Torus, 1);            // self
	ShapeElement *el = ecs_field(it, ShapeElement, 2); // self
	ShapeBuffer *b = ecs_field(it, ShapeBuffer, 3);    // shared
	for (int i = 0; i < it->count; ++i) {
		ShapeBuffer_append(b, el, SSHAPE_TORUS, torus);
	}
}

void AddShapeCylinder(ecs_iter_t *it)
{
	Cylinder *cylinder = ecs_field(it, Cylinder, 1);   // self
	ShapeElement *el = ecs_field(it, ShapeElement, 2); // self
	ShapeBuffer *b = ecs_field(it, ShapeBuffer, 3);    // shared
	for (int i = 0; i < it->count; ++i) {
		ShapeBuffer_append(b, el, SSHAPE_CYLINDER, cylinder);
	}
}

static void Flush(ecs_iter_t *it)
{
	ShapeBuffer *b = ecs_field(it, ShapeBuffer, 1);
	for (int i = 0; i < it->count; ++i, ++b) {
		//printf("%i %i, %i %i\n", b->ibuf.cap, b->vertices.buffer.cap, b->vbuf.cap, b->indices.buffer.cap);
		ShapeBuffer_upload(b);
		ShapeBuffer_reset(b);
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
	.vertex_buffers[0] = (sg_buffer){b->vbuf.id},
	.index_buffer = (sg_buffer){b->ibuf.id}});


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
	{.id = ecs_id(ShapeElement), .src.flags = EcsSelf},
	{.id = ecs_id(ShapeBuffer), .src.trav = Use, .src.flags = EcsUp},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = AddShapeCylinder,
	.query.filter.terms =
	{
	{.id = ecs_id(Cylinder), .src.flags = EcsSelf},
	{.id = ecs_id(ShapeElement), .src.flags = EcsSelf},
	{.id = ecs_id(ShapeBuffer), .src.trav = Use, .src.flags = EcsUp},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = Flush,
	.query.filter.terms =
	{
	{.id = ecs_id(ShapeBuffer), .src.flags = EcsSelf},
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
	}});


}