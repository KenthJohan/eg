#include "MiscLines.h"
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

ECS_COMPONENT_DECLARE(LinesBuffer);

typedef struct {
	m4f32 mvp;
	float extra[4];
} vs_params_t;

static void DrawLines(ecs_iter_t *it)
{
	LinesBuffer *lines = ecs_field(it, LinesBuffer, 0);  // up
	SgPipeline const *pipeline = ecs_field(it, SgPipeline, 1); // up, BUG: Does not match even when SgPipeline is added
	Camera const *cam = ecs_field(it, Camera, 2);              // up

	if (pipeline->id == 0) {
		ecs_warn("pipeline id is 0");
		return;
	}

	for (int i = 0; i < it->count; i++) {
		// printf("DrawLines: %s\n", ecs_get_name(it->world, it->entities[i]));
		if (lines->storage.count <= 0) {
			continue;
		}
		lines_upload(&lines->storage);
		sg_apply_pipeline((sg_pipeline){pipeline->id});
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

#define COLOR_RGBA(r, g, b, a) ((r) << 0) | ((g) << 8) | ((b) << 16) | ((a) << 24)
#define COLOR_RED COLOR_RGBA(0xFF, 0x77, 0x77, 0xFF)
#define COLOR_GREEN COLOR_RGBA(0x77, 0xFF, 0x77, 0xFF)
#define COLOR_BLUE COLOR_RGBA(0x77, 0x77, 0xFF, 0xFF)

static void AppendLines2(ecs_iter_t *it)
{
	LinesBuffer *lines = ecs_field(it, LinesBuffer, 0);                     // up, shared
	OrientationWorld *o = ecs_field(it, OrientationWorld, 1);               // self
	Position3World *p = ecs_field(it, Position3World, 2);                   // self
	EgBaseShowDrawReference *d = ecs_field(it, EgBaseShowDrawReference, 3); // self
	for (int i = 0; i < it->count; ++i, ++o, ++p, ++d) {
		if ((d->flags & 0x01) == 0) {
			continue;
		}
		m3f32 r;
		qf32_unit_to_m3((float *)o, &r);
		line_t l1 = {.a = {.color = COLOR_RED, .pos = {p->x, p->y, p->z, 0}}, .b = {.color = COLOR_RED, .pos = {p->x + r.c0[0], p->y + r.c0[1], p->z + r.c0[2], 0.0f}}};
		line_t l2 = {.a = {.color = COLOR_GREEN, .pos = {p->x, p->y, p->z, 0}}, .b = {.color = COLOR_GREEN, .pos = {p->x + r.c1[0], p->y + r.c1[1], p->z + r.c1[2], 0.0f}}};
		line_t l3 = {.a = {.color = COLOR_BLUE, .pos = {p->x, p->y, p->z, 0}}, .b = {.color = COLOR_BLUE, .pos = {p->x + r.c2[0], p->y + r.c2[1], p->z + r.c2[2], 0.0f}}};
		lines_append(&lines->storage, &l1);
		lines_append(&lines->storage, &l2);
		lines_append(&lines->storage, &l3);
	}
}

static void Ray2Line(ecs_iter_t *it)
{
	Position3 *p = ecs_field(it, Position3, 0); // self|shared
	Ray3 *r = ecs_field(it, Ray3, 1);           // self|shared
	Line *l = ecs_field(it, Line, 2);           // self
	int self_p = ecs_field_is_self(it, 0);
	int self_r = ecs_field_is_self(it, 1);
	// int self_l = ecs_field_is_self(it, 3);
	for (int i = 0; i < it->count; ++i, ++l, p += self_p, p += self_r) {
		float length = 100.0f;
		l->a[0] = p->x;
		l->a[1] = p->y;
		l->a[2] = p->z;
		l->b[0] = p->x + r->x * length;
		l->b[1] = p->y + r->y * length;
		l->b[2] = p->z + r->z * length;
		// printf("%s: %f %f %f\n", ecs_get_name(it->world, it->entities[i]), l->a[0], l->a[1], l->a[2]);
		/*
		Line line = {
		    // TODO:
		    // Camera position flipped, hmm?
		    // Shoot ray from mouse position or camera position?
		    .a = {-pos->x, -pos->y, -pos->z},
		    .b = {-pos->x+ ray_world[0]*length, -pos->y+ ray_world[1]*length, -pos->z+ ray_world[2]*length}
		};
		*/
	}
}

static void AppendLines(ecs_iter_t *it)
{
	LinesBuffer *lines = ecs_field(it, LinesBuffer, 0);             // up, shared
	Line *line = ecs_field(it, Line, 1);                            // self
	EgColorsV4U8_RGBA *color = ecs_field(it, EgColorsV4U8_RGBA, 2); // self

	for (int i = 0; i < it->count; ++i, ++line, ++color) {
		// printf("AppendLines: %s\n", ecs_get_name(it->world, it->entities[i]));
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
	LinesBuffer *lines = ecs_field(it, LinesBuffer, 0); // self
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
	ECS_IMPORT(world, EgBase);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgCameras);
	ECS_IMPORT(world, EgShapes);
	ECS_IMPORT(world, Sg);
	ecs_set_name_prefix(world, "MiscLines");


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

	ecs_system(world,{
	.entity = ecs_entity(world, {.name = "AppendLines", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = AppendLines,
	.query.terms =
	{
	{.id = ecs_id(LinesBuffer), .trav = EgBaseUse, .src.id = EcsUp},
	{.id = ecs_id(Line), .src.id = EcsSelf},
	{.id = ecs_id(EgColorsV4U8_RGBA), .src.id = EcsSelf},
	}});

	ecs_system(world,{
	.entity = ecs_entity(world, {.name = "AppendLines2", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = AppendLines2,
	.query.terms =
	{
	{.id = ecs_id(LinesBuffer), .trav = EgBaseUse, .src.id = EcsUp},
	{.id = ecs_id(OrientationWorld), .src.id = EcsSelf},
	{.id = ecs_id(Position3World), .src.id = EcsSelf},
	{.id = ecs_id(EgBaseShowDrawReference), .src.id = EcsSelf},
	}});

	ecs_system(world,{
	.entity = ecs_entity(world, {.name = "Ray2Line", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = Ray2Line,
	.query.terms =
	{
	{.id = ecs_id(Position3)},
	{.id = ecs_id(Ray3)},
	{.id = ecs_id(Line), .src.id = EcsSelf},
	}});

	ecs_system(world,{
	.entity = ecs_entity(world, {.name = "DrawLines", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = DrawLines,
	.query.terms =
	{
	{.id = ecs_id(LinesBuffer), .trav = EgBaseUse, .src.id = EcsUp, .inout = EcsInOut},
	{.id = ecs_id(SgPipeline), .trav = EgBaseUse, .src.id = EcsUp, .inout = EcsIn},
	{.id = ecs_id(Camera), .trav = EgBaseUse, .src.id = EcsUp, .inout = EcsIn},
	{.id = EgBaseDraw},
	}});

	ecs_system(world,{
	.entity = ecs_entity(world, {.name = "Flush", .add = ecs_ids(ecs_dependson(EcsPostUpdate))}),
	.callback = Flush,
	.query.terms =
	{
	{.id = ecs_id(LinesBuffer), .src.id = EcsSelf},
	}});
}