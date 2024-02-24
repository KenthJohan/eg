#include "MiscShapes.h"

#include "sokol/sokol_app.h"
#include "sokol/sokol_gfx.h"
#include "sokol/sokol_log.h"
#include "sokol/sokol_debugtext.h"
#include "sokol/sokol_glue.h"
#include "sokol/sokol_shape.h"
#include "eg/eg_fs.h"
#include "eg/Components.h"
#include "eg/Cameras.h"
#include "eg/Spatials.h"
#include "eg/Shapes.h"
#include "egsokol/Sg.h"

#include "shapes.h"

static void AddShapeTorus(ecs_iter_t *it)
{
	Torus *torus = ecs_field(it, Torus, 1);            // self
	Color32 *color = ecs_field(it, Color32, 2);        // self
	ShapeElement *el = ecs_field(it, ShapeElement, 3); // self
	ShapeBuffer *b = ecs_field(it, ShapeBuffer, 4);    // shared
	for (int i = 0; i < it->count; ++i, ++torus, ++color, ++el) {
		uint32_t c = (color->r << 0) | (color->g << 8) | (color->b << 16) | (color->a << 24);
		ShapeBuffer_append(b, el, SSHAPE_TORUS, torus, c);
	}
}

static void AddShapeCylinder(ecs_iter_t *it)
{
	Cylinder *cylinder = ecs_field(it, Cylinder, 1);   // self
	Color32 *color = ecs_field(it, Color32, 2);        // self
	ShapeElement *el = ecs_field(it, ShapeElement, 3); // self
	ShapeBuffer *b = ecs_field(it, ShapeBuffer, 4);    // shared
	for (int i = 0; i < it->count; ++i, ++cylinder, ++color, ++el) {
		uint32_t c = (color->r << 0) | (color->g << 8) | (color->b << 16) | (color->a << 24);
		ShapeBuffer_append(b, el, SSHAPE_CYLINDER, cylinder, c);
	}
}

static void AddShapeSphere(ecs_iter_t *it)
{
	Sphere *sphere = ecs_field(it, Sphere, 1);   // self
	Color32 *color = ecs_field(it, Color32, 2);        // self
	ShapeElement *el = ecs_field(it, ShapeElement, 3); // self
	ShapeBuffer *b = ecs_field(it, ShapeBuffer, 4);    // shared
	for (int i = 0; i < it->count; ++i, ++sphere, ++color, ++el) {
		uint32_t c = (color->r << 0) | (color->g << 8) | (color->b << 16) | (color->a << 24);
		ShapeBuffer_append(b, el, SSHAPE_SPHERE, sphere, c);
	}
}

static void Flush(ecs_iter_t *it)
{
	ShapeBuffer *b = ecs_field(it, ShapeBuffer, 1);
	for (int i = 0; i < it->count; ++i, ++b) {
		if(b->indices.buffer.size <= 0) {continue;}
		if(b->vertices.buffer.size <= 0) {continue;}
		//printf("ShapeBuffer %s\n", ecs_get_name(it->world, it->entities[i]));
		//printf("%i %i, %i %i\n", b->ibuf.cap, b->vertices.buffer.cap, b->vbuf.cap, b->indices.buffer.cap);
		ShapeBuffer_upload(b);
		ShapeBuffer_reset(b);
	}
}

typedef struct {
	float mvp[16];
	float extra[4];
} vs_params_t;

static void DrawShape(ecs_iter_t *it)
{
	ShapeElement *element = ecs_field(it, ShapeElement, 1);            // self
	Transformation *transformation = ecs_field(it, Transformation, 2); // self
	SgPipeline *pipeline = ecs_field(it, SgPipeline, 3);               // up, shared
	ShapeBuffer *b = ecs_field(it, ShapeBuffer, 4);                    // up, shared
	Camera *cam = ecs_field(it, Camera, 5);                            // up, shared

	//int sizeofcolor = sizeof(Color32);
	//printf("entbuf: %s %i %i\n", ecs_get_name(it->world, it->entities[0]), element->num_elements, sizeofcolor);

	if(b->ibuf.id == 0){return;}
	if(b->vbuf.id == 0){return;}

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
	ECS_IMPORT(world, Cameras);
	ECS_IMPORT(world, Spatials);
	ECS_IMPORT(world, Shapes);
	ECS_IMPORT(world, Sg);

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "AddShapeTorus", .add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = AddShapeTorus,
	.query.filter.terms =
	{
	{.id = ecs_id(Torus), .src.flags = EcsSelf},
	{.id = ecs_id(Color32), .src.flags = EcsSelf},
	{.id = ecs_id(ShapeElement), .src.flags = EcsSelf},
	{.id = ecs_id(ShapeBuffer), .src.trav = EgUse, .src.flags = EcsUp},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "AddShapeCylinder", .add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = AddShapeCylinder,
	.query.filter.terms =
	{
	{.id = ecs_id(Cylinder), .src.flags = EcsSelf},
	{.id = ecs_id(Color32), .src.flags = EcsSelf},
	{.id = ecs_id(ShapeElement), .src.flags = EcsSelf},
	{.id = ecs_id(ShapeBuffer), .src.trav = EgUse, .src.flags = EcsUp},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "AddShapeSphere", .add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = AddShapeSphere,
	.query.filter.terms =
	{
	{.id = ecs_id(Sphere), .src.flags = EcsSelf},
	{.id = ecs_id(Color32), .src.flags = EcsSelf},
	{.id = ecs_id(ShapeElement), .src.flags = EcsSelf},
	{.id = ecs_id(ShapeBuffer), .src.trav = EgUse, .src.flags = EcsUp},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "Flush", .add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = Flush,
	.query.filter.terms =
	{
	{.id = ecs_id(ShapeBuffer), .src.flags = EcsSelf},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "DrawShape", .add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = DrawShape,
	.query.filter.terms =
	{
	{.id = ecs_id(ShapeElement), .src.flags = EcsSelf},
	{.id = ecs_id(Transformation), .src.flags = EcsSelf},
	{.id = ecs_id(SgPipeline), .src.trav = EgUse, .src.flags = EcsUp},
	{.id = ecs_id(ShapeBuffer), .src.trav = EgUse, .src.flags = EcsUp},
	{.id = ecs_id(Camera), .src.trav = EgUse, .src.flags = EcsUp},
	}});


}