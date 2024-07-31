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
	ecs_trace("MyGraphicsDrawCommand::Ctor");
	ecs_os_memset_t(ptr, 0, MyGraphicsDrawCommand);
})

ECS_DTOR(MyGraphicsDrawCommand, ptr, {
	ecs_trace("MyGraphicsDrawCommand::Dtor");
	ecs_os_free(ptr->transforms_data);
})

ECS_MOVE(MyGraphicsDrawCommand, dst, src, {
	ecs_trace("MyGraphicsDrawCommand::Move");
	ecs_os_free(dst->transforms_data);
	dst->transforms_data = src->transforms_data;
	src->transforms_data = NULL;
})

ECS_COPY(MyGraphicsDrawCommand, dst, src, {
	ecs_trace("MyGraphicsDrawCommand::Copy");
	ecs_os_free(dst->transforms_data);
	dst->transforms_data = ecs_os_memdup_n(src->transforms_data, m4f32, src->transforms_count);
})

typedef struct {
	float mvp[16];
	float extra[4];
} vs_params_t;

static void DrawShape(ecs_iter_t *it)
{
	Transformation *transformation = ecs_field(it, Transformation, 0);        // self
	MyGraphicsDrawCommand *drawcmd = ecs_field(it, MyGraphicsDrawCommand, 1); // up, shared
	SgPipeline *pipeline = ecs_field(it, SgPipeline, 2);                      // up, shared
	EgBaseShapeBuffer *ivbuf = ecs_field(it, EgBaseShapeBuffer, 3);           // up, shared
	Camera *cam = ecs_field(it, Camera, 4);                                   // up, shared
	if (ivbuf->ibuf.id == 0) {
		return;
	}
	if (ivbuf->vbuf.id == 0) {
		return;
	}
	sg_apply_pipeline((sg_pipeline){pipeline->id});
	sg_apply_bindings(&(sg_bindings){
	.vertex_buffers[0] = (sg_buffer){ivbuf->vbuf.id},
	.index_buffer = (sg_buffer){ivbuf->ibuf.id},
	.fs.images[0] = (sg_image){0},
	.fs.samplers[0] = (sg_sampler){0},
	});

	for (int i = 0; i < it->count; ++i, ++transformation) {
		//EgColorsV4F32_RGBA color[1] = {{255,255,255,255}};
		//vs_params_t params = {.extra = {color->r, color->b, color->b, color->a}};
		vs_params_t params = {0};
		params.extra[3] = -3;
		m4f32_mul((m4f32 *)params.mvp, &cam->vp, &transformation->matrix);
		// m4f32_print(&t);
		sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &SG_RANGE(params));
		sg_draw(drawcmd->offset, drawcmd->count, 1);
	}
	
}




static void AppendTransforms(ecs_iter_t *it)
{
	Transformation *t = ecs_field(it, Transformation, 0);               // self
	MyGraphicsDrawCommand *d = ecs_field(it, MyGraphicsDrawCommand, 1); // up, shared

	int32_t sum = d->transforms_count + it->count;
	if (sum > d->transforms_cap) {
		d->transforms_cap = sum * 2;
		ecs_trace("AppendTransforms::ecs_os_realloc_n");
		d->transforms_data = ecs_os_realloc_n(d->transforms_data, m4f32, d->transforms_cap);
	}

	for (int i = 0; i < it->count; ++i, ++t) {
		d->transforms_data[d->transforms_count] = t->matrix;
		d->transforms_count++;
	}
}


static void Draw(ecs_iter_t *it)
{
	MyGraphicsDrawCommand *d = ecs_field(it, MyGraphicsDrawCommand, 0);
	for (int i = 0; i < it->count; ++i, ++d) {
		// Upload

		// Reset
		d->transforms_total = d->transforms_count;
		d->transforms_count = 0;
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

	ecs_set_hooks(world, MyGraphicsDrawCommand, {
	                                            .ctor = ecs_ctor(MyGraphicsDrawCommand),
	                                            .move = ecs_move(MyGraphicsDrawCommand),
	                                            .copy = ecs_copy(MyGraphicsDrawCommand),
	                                            .dtor = ecs_dtor(MyGraphicsDrawCommand),
	                                            });
	
	ecs_struct(world,
	{.entity = ecs_id(MyGraphicsDrawCommand),
	.members = {
	{.name = "offset", .type = ecs_id(ecs_i32_t)},
	{.name = "count", .type = ecs_id(ecs_i32_t)},
	{.name = "instances", .type = ecs_id(ecs_i32_t)},
	{.name = "transforms_data", .type = ecs_id(ecs_uptr_t)},
	{.name = "transforms_count", .type = ecs_id(ecs_i32_t)},
	{.name = "transforms_cap", .type = ecs_id(ecs_i32_t)},
	{.name = "transforms_total", .type = ecs_id(ecs_i32_t)},
	}});
	

	ecs_system(world, {.entity = ecs_entity(world, {.name = "DrawShape", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	                  .callback = DrawShape,
	                  //.query.instanced = true,
	                  .query.terms =
	                  {
	                  {.id = ecs_id(Transformation), .src.id = EcsSelf},
	                  {.id = ecs_id(MyGraphicsDrawCommand), .trav = EgBaseUse, .src.id = EcsUp},
	                  {.id = ecs_id(SgPipeline), .trav = EgBaseUse, .src.id = EcsUp},
	                  {.id = ecs_id(EgBaseShapeBuffer), .trav = EgBaseUse, .src.id = EcsUp},
	                  {.id = ecs_id(Camera), .trav = EgBaseUse, .src.id = EcsUp},
	                  }});


	ecs_system(world, {.entity = ecs_entity(world, {.name = "AppendTransforms", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	                  .callback = AppendTransforms,
	                  .query.terms =
	                  {
	                  {.id = ecs_id(Transformation), .src.id = EcsSelf},
	                  {.id = ecs_id(MyGraphicsDrawCommand), .trav = EgBaseUse, .src.id = EcsUp},
	                  }});

	ecs_system(world, {.entity = ecs_entity(world, {.name = "Draw", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	                  .callback = Draw,
	                  .query.terms =
	                  {
	                  {.id = ecs_id(MyGraphicsDrawCommand), .src.id = EcsSelf},
	                  }});
					  
					  
}