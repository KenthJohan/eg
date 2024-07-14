#include "MiscShapes.h"

#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_log.h>
#include <sokol_debugtext.h>
#include <sokol_glue.h>
#include <sokol_shape.h>
#include <egbase.h>
#include <egcameras.h>
#include <egspatials.h>
#include <egshapes.h>
#include <egsokol.h>
#include <egcolors.h>

#include "lines.h"
#include "shapes.h"
#include "MyGraphics.h"

static void AddShapeTorus(ecs_iter_t *it)
{
	Torus *torus = ecs_field(it, Torus, 0);                              // self
	EgColorsV4U8_RGBA *color = ecs_field(it, EgColorsV4U8_RGBA, 1);      // self
	MyGraphicsDrawCommand *el = ecs_field(it, MyGraphicsDrawCommand, 2); // self
	EgBaseMemory2 *b = ecs_field(it, EgBaseMemory2, 3);                  // shared
	for (int i = 0; i < it->count; ++i, ++torus, ++color, ++el) {
		uint32_t c = (color->r << 0) | (color->g << 8) | (color->b << 16) | (color->a << 24);
		ShapeBuffer_append(b, el, SSHAPE_TORUS, torus, c);
	}
}

static void AddShapeCylinder(ecs_iter_t *it)
{
	Cylinder *cylinder = ecs_field(it, Cylinder, 0);                     // self
	EgColorsV4U8_RGBA *color = ecs_field(it, EgColorsV4U8_RGBA, 1);      // self
	MyGraphicsDrawCommand *el = ecs_field(it, MyGraphicsDrawCommand, 2); // self
	EgBaseMemory2 *b = ecs_field(it, EgBaseMemory2, 3);                  // shared
	for (int i = 0; i < it->count; ++i, ++cylinder, ++color, ++el) {
		uint32_t c = (color->r << 0) | (color->g << 8) | (color->b << 16) | (color->a << 24);
		ShapeBuffer_append(b, el, SSHAPE_CYLINDER, cylinder, c);
	}
}

static void AddShapeSphere(ecs_iter_t *it)
{
	Sphere *sphere = ecs_field(it, Sphere, 0);                           // self
	EgColorsV4U8_RGBA *color = ecs_field(it, EgColorsV4U8_RGBA, 1);      // self
	MyGraphicsDrawCommand *el = ecs_field(it, MyGraphicsDrawCommand, 2); // self
	EgBaseMemory2 *b = ecs_field(it, EgBaseMemory2, 3);                  // shared
	for (int i = 0; i < it->count; ++i, ++sphere, ++color, ++el) {
		uint32_t c = (color->r << 0) | (color->g << 8) | (color->b << 16) | (color->a << 24);
		ShapeBuffer_append(b, el, SSHAPE_SPHERE, sphere, c);
	}
}

static void Flush(ecs_iter_t *it)
{
	EgBaseMemory2 *b = ecs_field(it, EgBaseMemory2, 0);
	EgBaseShapeBuffer *g = ecs_field(it, EgBaseShapeBuffer, 1);
	for (int i = 0; i < it->count; ++i, ++b) {
		if (b->mem[0].size <= 0) {
			continue;
		}
		if (b->mem[1].size <= 0) {
			continue;
		}
		// printf("EgBaseShapeBuffer %s %i %i\n", ecs_get_name(it->world, it->entities[i]), b->indices.size, b->indices.cap);
		//  printf("%i %i, %i %i\n", b->ibuf.cap, b->vertices.buffer.cap, b->vbuf.cap, b->indices.buffer.cap);
		ShapeBuffer_upload(b, g);
		ShapeBuffer_reset(b);
	}
}

void MiscShapesImport(ecs_world_t *world)
{
	ECS_MODULE(world, MiscShapes);
	ECS_IMPORT(world, EgBase);
	ECS_IMPORT(world, EgCameras);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgShapes);
	ECS_IMPORT(world, EgColors);
	ECS_IMPORT(world, Sg);
	ECS_IMPORT(world, MyGraphics);

	ecs_system(world,{
	.entity = ecs_entity(world, {.name = "AddShapeTorus", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = AddShapeTorus,
	.query.terms =
	{
	{.id = ecs_id(Torus), .src.id = EcsSelf},
	{.id = ecs_id(EgColorsV4U8_RGBA), .src.id = EcsSelf},
	{.id = ecs_id(MyGraphicsDrawCommand), .src.id = EcsSelf},
	{.id = ecs_id(EgBaseMemory2), .trav = EcsChildOf, .src.id = EcsUp},
	}});

	ecs_system(world,{
	.entity = ecs_entity(world, {.name = "AddShapeCylinder", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = AddShapeCylinder,
	.query.terms =
	{
	{.id = ecs_id(Cylinder), .src.id = EcsSelf},
	{.id = ecs_id(EgColorsV4U8_RGBA), .src.id = EcsSelf},
	{.id = ecs_id(MyGraphicsDrawCommand), .src.id = EcsSelf},
	{.id = ecs_id(EgBaseMemory2), .trav = EcsChildOf, .src.id = EcsUp},
	}});

	ecs_system(world,{
	.entity = ecs_entity(world, {.name = "AddShapeSphere", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = AddShapeSphere,
	.query.terms =
	{
	{.id = ecs_id(Sphere), .src.id = EcsSelf},
	{.id = ecs_id(EgColorsV4U8_RGBA), .src.id = EcsSelf},
	{.id = ecs_id(MyGraphicsDrawCommand), .src.id = EcsSelf},
	{.id = ecs_id(EgBaseMemory2), .trav = EcsChildOf, .src.id = EcsUp},
	}});

	ecs_system(world,{
	.entity = ecs_entity(world, {.name = "Flush", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = Flush,
	.query.terms =
	{
	{.id = ecs_id(EgBaseMemory2), .src.id = EcsSelf},
	{.id = ecs_id(EgBaseShapeBuffer), .src.id = EcsSelf},
	}});
}