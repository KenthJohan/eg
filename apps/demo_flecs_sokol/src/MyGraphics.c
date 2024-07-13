#include "MyGraphics.h"

#include <egsokol.h>
#include <egmath.h>
#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_log.h>
#include <sokol_debugtext.h>
#include <sokol_glue.h>
#include <sokol_shape.h>
#include <egbase.h>
#include <egspatials.h>
#include <egcameras.h>
#include <egshapes.h>
#include <egcolors.h>

ECS_COMPONENT_DECLARE(MyGraphicsDrawCommand);



ECS_CTOR(MyGraphicsDrawCommand, ptr, {
	ecs_os_memset_t(ptr, 0, MyGraphicsDrawCommand);
})

typedef struct {
	float mvp[16];
	float extra[4];
} vs_params_t;

static void DrawShape(ecs_iter_t *it)
{
	Transformation *transformation = ecs_field(it, Transformation, 1);        // self
	EgColorsV4F32_RGBA *color = ecs_field(it, EgColorsV4F32_RGBA, 2);         // self
	MyGraphicsDrawCommand *drawcmd = ecs_field(it, MyGraphicsDrawCommand, 3); // up, shared
	Camera *cam = ecs_field(it, Camera, 4);                                   // up, shared
	if (drawcmd->ibuf == 0) {
		return;
	}
	if (drawcmd->vbuf == 0) {
		return;
	}
	sg_apply_pipeline((sg_pipeline){drawcmd->pipeline});
	sg_apply_bindings(&(sg_bindings){
	.vertex_buffers[0] = (sg_buffer){drawcmd->vbuf},
	.index_buffer = (sg_buffer){drawcmd->ibuf}});

	if (color) {
		for (int i = 0; i < it->count; ++i, ++transformation) {
			vs_params_t params = {.extra = {color->r, color->b, color->b, color->a}};
			m4f32_mul((m4f32 *)params.mvp, &cam->vp, &transformation->matrix);
			// m4f32_print(&t);
			sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &SG_RANGE(params));
			sg_draw(drawcmd->offset, drawcmd->count, drawcmd->instances);
		}
	} else {
		for (int i = 0; i < it->count; ++i, ++transformation) {
			vs_params_t params = {0};
			params.extra[3] = -3;
			m4f32_mul((m4f32 *)params.mvp, &cam->vp, &transformation->matrix);
			// m4f32_print(&t);
			sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &SG_RANGE(params));
			sg_draw(drawcmd->offset, drawcmd->count, 1);
		}
	}
}

static void CopyDrawInfo(ecs_iter_t *it)
{
	SgPipeline *p = ecs_field(it, SgPipeline, 1);                       // up, shared
	EgBaseShapeBuffer *b = ecs_field(it, EgBaseShapeBuffer, 2);         // up, shared
	MyGraphicsDrawCommand *d = ecs_field(it, MyGraphicsDrawCommand, 3); // self

	for (int i = 0; i < it->count; ++i, ++d) {
		d->ibuf = b->ibuf.id;
		d->vbuf = b->vbuf.id;
		d->pipeline = p->id.id;
	}
}

void MyGraphicsImport(ecs_world_t *world)
{
	ECS_MODULE(world, MyGraphics);
	ECS_IMPORT(world, EgBase);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgCameras);
	ECS_IMPORT(world, EgShapes);
	ECS_IMPORT(world, Sg);
	ecs_set_name_prefix(world, "MyGraphics");

	ECS_COMPONENT_DEFINE(world, MyGraphicsDrawCommand);

	ecs_set_hooks(world, MyGraphicsDrawCommand, {.ctor = ecs_ctor(MyGraphicsDrawCommand)});

	ecs_struct(world,
	{.entity = ecs_id(MyGraphicsDrawCommand),
	.members = {
	{.name = "pipeline", .type = ecs_id(ecs_u32_t)},
	{.name = "ibuf", .type = ecs_id(ecs_u32_t)},
	{.name = "vbuf", .type = ecs_id(ecs_u32_t)},
	{.name = "offset", .type = ecs_id(ecs_i32_t)},
	{.name = "count", .type = ecs_id(ecs_i32_t)},
	{.name = "instances", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "DrawShape", .add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = DrawShape,
	.query.filter.instanced = true,
	.query.filter.terms =
	{
	{.id = ecs_id(Transformation), .src.flags = EcsSelf},
	{.id = ecs_id(EgColorsV4F32_RGBA), .src.flags = EcsSelf, .oper = EcsOptional},
	{.id = ecs_id(MyGraphicsDrawCommand), .src.trav = EgBaseUse, .src.flags = EcsUp},
	{.id = ecs_id(Camera), .src.trav = EgBaseUse, .src.flags = EcsUp},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "CopyDrawInfo", .add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = CopyDrawInfo,
	.query.filter.instanced = true,
	.query.filter.terms =
	{
	{.id = ecs_id(SgPipeline), .src.trav = EcsChildOf, .src.flags = EcsUp},
	{.id = ecs_id(EgBaseShapeBuffer), .src.trav = EcsChildOf, .src.flags = EcsUp},
	{.id = ecs_id(MyGraphicsDrawCommand), .src.flags = EcsSelf},
	}});
}