#include "MiscPoints.h"
#include <egsokol.h>
#include <egmath.h>
#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_log.h>
#include <sokol_debugtext.h>
#include <sokol_glue.h>
#include <sokol_shape.h>
#include <egbase.h>
#include <egcameras.h>
#include <stdlib.h>

ECS_COMPONENT_DECLARE(PointsBuffer);

typedef struct {
	m4f32 mvp;
	float extra[4];
} vs_params_t;

static void DrawPoints(ecs_iter_t *it)
{
	PointsBuffer *points = ecs_field(it, PointsBuffer, 0); // self
	SgPipeline *pipeline = ecs_field(it, SgPipeline, 1);   // self
	Camera *cam = ecs_field(it, Camera, 2);                // self
	Window *win = ecs_field(it, Window, 3);                // singleton

	for (int i = 0; i < it->count; i++) {
		if(points->storage.count <= 0) {continue;}
		//printf("points->storage.count %i\n", points->storage.count);
 		points_upload(&points->storage);
		sg_apply_pipeline((sg_pipeline){pipeline->id});
		sg_apply_bindings(&(sg_bindings){
		.vertex_buffers[0] = points->storage.gpu_buffer});
		vs_params_t params = {0};
		params.mvp = cam->vp;
		params.extra[0] = win->w;
		params.extra[1] = win->h;
		sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &SG_RANGE(params));
		points_draw(&points->storage);
	}
}

static void AppendExamplePoints(ecs_iter_t *it)
{
	PointsBuffer *points = ecs_field(it, PointsBuffer, 0); // self
	int32_t n = 100;
	point_vertex_t * p = points_append(&points->storage, n);
	for (int i = 0; i < n; ++i, ++p) {
		point_vertex_t point = {
			.color = 0xFFFFFFFF,
			.pos = {
			((float)rand() / (float)RAND_MAX)*100.0f,
			((float)rand() / (float)RAND_MAX)*100.0f,
			((float)rand() / (float)RAND_MAX)*100.0f,
			1.0f}
		};
		*p = point;
	}
}



ECS_CTOR(PointsBuffer, ptr, {
	ecs_os_memset_t(ptr, 0, PointsBuffer);
})

void MiscPointsImport(ecs_world_t *world)
{
	ECS_MODULE(world, MiscPoints);
	ECS_IMPORT(world, EgBase);
	ECS_IMPORT(world, EgCameras);
	ECS_IMPORT(world, Sg);

	ECS_COMPONENT_DEFINE(world, PointsBuffer);

	ecs_set_hooks(world, PointsBuffer, {.ctor = ecs_ctor(PointsBuffer)});

	ecs_system(world,{
	.entity = ecs_entity(world, {.name = "AppendExamplePoints", .add = ecs_ids(ecs_dependson(EcsOnUpdate), EcsDisabled)}),
	.callback = AppendExamplePoints,
	.query.terms =
	{
	{.id = ecs_id(PointsBuffer), .src.id = EcsSelf},
	}});

	ecs_system(world,{
	.entity = ecs_entity(world, {.name = "DrawPoints", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = DrawPoints,
	.query.terms =
	{
	{.id = ecs_id(PointsBuffer), .src.id = EcsSelf},
	{.id = ecs_id(SgPipeline), .trav = EgBaseUse, .src.id = EcsUp},
	{.id = ecs_id(Camera), .trav = EgBaseUse, .src.id = EcsUp},
	{.id = ecs_id(Window), .src.id = ecs_id(Window)},
	}});
}