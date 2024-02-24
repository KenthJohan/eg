#include "MiscLines.h"
#include "egsokol/Sg.h"
#include "eg/gmath.h"
#include "sokol/sokol_app.h"
#include "sokol/sokol_gfx.h"
#include "sokol/sokol_log.h"
#include "sokol/sokol_debugtext.h"
#include "sokol/sokol_glue.h"
#include "sokol/sokol_shape.h"
#include "eg/Components.h"
#include "eg/Spatials.h"
#include "eg/Cameras.h"
#include "eg/Shapes.h"


ECS_COMPONENT_DECLARE(LinesBuffer);

typedef struct {
	m4f32 mvp;
	float extra[4];
} vs_params_t;

static void DrawLines(ecs_iter_t *it)
{
	LinesBuffer *lines = ecs_field(it, LinesBuffer, 1);  // self
	SgPipeline *pipeline = ecs_field(it, SgPipeline, 2); // self
	Camera *cam = ecs_field(it, Camera, 3);              // self

	for (int i = 0; i < it->count; i++) {
		if(lines->storage.count <= 0) {continue;}
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

/*
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
*/




#define COLOR_RGBA(r,g,b,a) ((r) << 0) | ((g) << 8) | ((b) << 16) | ((a) << 24)
#define COLOR_RED COLOR_RGBA(0xFF, 0x77, 0x77, 0xFF)
#define COLOR_GREEN COLOR_RGBA(0x77, 0xFF, 0x77, 0xFF)
#define COLOR_BLUE COLOR_RGBA(0x77, 0x77, 0xFF, 0xFF)


static void AppendLines2(ecs_iter_t *it)
{
	LinesBuffer *lines = ecs_field(it, LinesBuffer, 1); // up, shared
	OrientationWorld *o = ecs_field(it, OrientationWorld, 2); // self
	Position3World *p = ecs_field(it, Position3World, 3); // self
	for (int i = 0; i < it->count; ++i, ++o, ++p) {
		m3f32 r;
		qf32_unit_to_m3((float*)o, &r);
		line_t l1 = {.a = {.color = COLOR_RED, .pos = {p->x, p->y, p->z, 0}},.b = {.color = COLOR_RED, .pos = {p->x+r.c0[0], p->y+r.c0[1], p->z+r.c0[2], 0.0f}}};
		line_t l2 = {.a = {.color = COLOR_GREEN, .pos = {p->x, p->y, p->z, 0}},.b = {.color = COLOR_GREEN, .pos = {p->x+r.c1[0], p->y+r.c1[1], p->z+r.c1[2], 0.0f}}};
		line_t l3 = {.a = {.color = COLOR_BLUE, .pos = {p->x, p->y, p->z, 0}},.b = {.color = COLOR_BLUE, .pos = {p->x+r.c2[0], p->y+r.c2[1], p->z+r.c2[2], 0.0f}}};
		lines_append(&lines->storage, &l1);
		lines_append(&lines->storage, &l2);
		lines_append(&lines->storage, &l3);
	}
}










static void AppendLines(ecs_iter_t *it)
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


static void Flush(ecs_iter_t *it)
{
	LinesBuffer *lines = ecs_field(it, LinesBuffer, 1); // self
	for (int i = 0; i < it->count; ++i, ++lines) {
		lines_flush(&lines->storage);
	}
}


ECS_CTOR(LinesBuffer, ptr, {
	ecs_os_memset_t(ptr, 0, LinesBuffer);
})

void MiscLinesImport(ecs_world_t *world)
{
	ECS_MODULE(world, MiscLines);
	ECS_IMPORT(world, Components);
	ECS_IMPORT(world, Spatials);
	ECS_IMPORT(world, Cameras);
	ECS_IMPORT(world, Shapes);
	ECS_IMPORT(world, Sg);

	ECS_COMPONENT_DEFINE(world, LinesBuffer);

	ecs_set_hooks(world, LinesBuffer, {.ctor = ecs_ctor(LinesBuffer)});

	/*
	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = AppendExampleLines,
	.query.filter.terms =
	{
	{.id = ecs_id(LinesBuffer), .src.flags = EcsSelf},
	}});
	*/

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "AppendLines", .add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = AppendLines,
	.query.filter.terms =
	{
	{.id = ecs_id(LinesBuffer), .src.trav = EgUse, .src.flags = EcsUp},
	{.id = ecs_id(Line), .src.flags = EcsSelf},
	{.id = ecs_id(Color32), .src.flags = EcsSelf},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "AppendLines2", .add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = AppendLines2,
	.query.filter.terms =
	{
	{.id = ecs_id(LinesBuffer), .src.trav = EgUse, .src.flags = EcsUp},
	{.id = ecs_id(OrientationWorld), .src.flags = EcsSelf},
	{.id = ecs_id(Position3World), .src.flags = EcsSelf},
	{.id = ecs_id(Color32), .src.flags = EcsSelf},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "DrawLines", .add = {ecs_dependson(EcsPostUpdate)}}),
	.callback = DrawLines,
	.query.filter.terms =
	{
	{.id = ecs_id(LinesBuffer), .src.flags = EcsSelf},
	{.id = ecs_id(SgPipeline), .src.trav = EgUse, .src.flags = EcsUp},
	{.id = ecs_id(Camera), .src.trav = EgUse, .src.flags = EcsUp},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "Flush", .add = {ecs_dependson(EcsPostUpdate)}}),
	.callback = Flush,
	.query.filter.terms =
	{
	{.id = ecs_id(LinesBuffer), .src.flags = EcsSelf},
	}});
}