#include "MiscPoints.h"
#include <egsokol/Sg.h>
#include <eg/gmath.h>
#include <sokol/sokol_app.h>
#include <sokol/sokol_gfx.h>
#include <sokol/sokol_log.h>
#include <sokol/sokol_debugtext.h>
#include <sokol/sokol_glue.h>
#include <sokol/sokol_shape.h>
#include <eg/Components.h>
#include <stdlib.h>

ECS_COMPONENT_DECLARE(PointsBuffer);

typedef struct {
	m4f32 mvp;
	float extra[4];
} vs_params_t;

static void DrawPoints(ecs_iter_t *it)
{
	PointsBuffer *points = ecs_field(it, PointsBuffer, 1); // self
	SgPipeline *pipeline = ecs_field(it, SgPipeline, 2);   // self
	Camera *cam = ecs_field(it, Camera, 3);                // self
	Window *win = ecs_field(it, Window, 4);                // singleton

	for (int i = 0; i < it->count; i++) {
		points_upload(&points->storage);
		sg_apply_pipeline(pipeline->id);
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
	PointsBuffer *points = ecs_field(it, PointsBuffer, 1); // self
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

static void Flush(ecs_iter_t *it)
{
	PointsBuffer *points = ecs_field(it, PointsBuffer, 1); // self
	for (int i = 0; i < it->count; ++i, ++points) {
		points_reset(&points->storage);
	}
}

ECS_CTOR(PointsBuffer, ptr, {
	ecs_os_memset_t(ptr, 0, PointsBuffer);
})

void MiscPointsImport(ecs_world_t *world)
{
	ECS_MODULE(world, MiscPoints);
	ECS_IMPORT(world, Components);
	ECS_IMPORT(world, Sg);

	ECS_COMPONENT_DEFINE(world, PointsBuffer);

	ecs_set_hooks(world, PointsBuffer, {.ctor = ecs_ctor(PointsBuffer)});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = AppendExamplePoints,
	.query.filter.terms =
	{
	{.id = ecs_id(PointsBuffer), .src.flags = EcsSelf},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = DrawPoints,
	.query.filter.terms =
	{
	{.id = ecs_id(PointsBuffer), .src.flags = EcsSelf},
	{.id = ecs_id(SgPipeline), .src.trav = EgUse, .src.flags = EcsUp},
	{.id = ecs_id(Camera), .src.trav = EgUse, .src.flags = EcsUp},
	{.id = ecs_id(Window), .src.id = ecs_id(Window)},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = Flush,
	.query.filter.terms =
	{
	{.id = ecs_id(PointsBuffer), .src.flags = EcsSelf},
	}});
}