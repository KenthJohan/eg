#include "MiscLines.h"
#include <egsokol/Sg.h>
#include <eg/gmath.h>
#include <sokol/sokol_app.h>
#include <sokol/sokol_gfx.h>
#include <sokol/sokol_log.h>
#include <sokol/sokol_debugtext.h>
#include <sokol/sokol_glue.h>
#include <sokol/sokol_shape.h>
#include <eg/Components.h>

typedef struct {
	m4f32 mvp;
	float extra[4];
} vs_params_t;

void DrawLines(ecs_iter_t *it)
{
	LinesBuffer *lines = ecs_field(it, LinesBuffer, 1);  // self
	SgPipeline *pipeline = ecs_field(it, SgPipeline, 2); // self
	Camera *cam = ecs_field(it, Camera, 3);              // self

	for (int i = 0; i < it->count; i++) {
		lines_upload(&lines->storage);
		sg_apply_pipeline(pipeline->id);
		sg_apply_bindings(&(sg_bindings){
		.vertex_buffers[0] = lines->storage.gpu_buffer});
		vs_params_t params = {0};
		params.mvp = cam->vp;
		sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &SG_RANGE(params));
		lines_draw(&lines->storage);
	}
}

void AppendExampleLines(ecs_iter_t *it)
{
	LinesBuffer *lines = ecs_field(it, LinesBuffer, 1); // self

	for (int i = 0; i < 100; i++) {
		line_t line =
		{
		.a = {.color = 0xFFFFFFFF, .pos = {0, 0, 0, 0}},
		.b = {.color = 0xFFFFFFFF, .pos = {sin(i/10.0f)*100.0f, 100, 300, 0}}};
		//lines_append(&lines->storage, &line);
	}
}

void AppendLines(ecs_iter_t *it)
{
	LinesBuffer *lines = ecs_field(it, LinesBuffer, 1); // up, shared
	Line *line = ecs_field(it, Line, 2); // self
	Color32 *color = ecs_field(it, Color32, 3); // self

	for (int i = 0; i < it->count; ++i, ++line) {
		uint32_t c = (color->r << 0) | (color->g << 8) | (color->b << 16) | (color->a << 24);
		line_t l =
		{
		.a = {.color = c, .pos = {line->a[0], line->a[1], line->a[2], 0.0f}},
		.b = {.color = c, .pos = {line->b[0], line->b[1], line->b[2], 0.0f}}};
		lines_append(&lines->storage, &l);
	}
}

ECS_CTOR(LinesBuffer, ptr, {
	ecs_os_memset_t(ptr, 0, LinesBuffer);
})

void MiscLinesImport(ecs_world_t *world)
{
	ECS_MODULE(world, MiscLines);
	ECS_IMPORT(world, Components);
	ECS_IMPORT(world, Sg);

	ECS_COMPONENT_DEFINE(world, LinesBuffer);

	ecs_set_hooks(world, LinesBuffer, {.ctor = ecs_ctor(LinesBuffer)});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = AppendExampleLines,
	.query.filter.terms =
	{
	{.id = ecs_id(LinesBuffer), .src.flags = EcsSelf},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = AppendLines,
	.query.filter.terms =
	{
	{.id = ecs_id(LinesBuffer), .src.trav = Use, .src.flags = EcsUp},
	{.id = ecs_id(Line), .src.flags = EcsSelf},
	{.id = ecs_id(Color32), .src.flags = EcsSelf},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = DrawLines,
	.query.filter.terms =
	{
	{.id = ecs_id(LinesBuffer), .src.flags = EcsSelf},
	{.id = ecs_id(SgPipeline), .src.trav = Use, .src.flags = EcsUp},
	{.id = ecs_id(Camera), .src.trav = Use, .src.flags = EcsUp},
	}});
}