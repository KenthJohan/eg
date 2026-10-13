#include "EgUi/EgUiLocalMouse.h"
#include <EgUi.h>

#include <EgSpatials.h>
#include <EgCameras.h>
#include <EgShapes.h>
#include <EgIntersects.h>
#include <ecsx.h>
#include <math.h>

ECS_COMPONENT_DECLARE(EgUiLocalMousePosition);

// Transforms the shared mouse position into each entity's local space once for other systems.
static void EgUiLocalMousePosition_Update(ecs_iter_t *it)
{
	Position2 const        *p0 = ecs_field_shared(it, Position2, 0);
	WorldTransform3 const  *x  = ecs_field_self(it, WorldTransform3, 1);
	EgUiLocalMousePosition *l  = ecs_field_self(it, EgUiLocalMousePosition, 2);
	EgCamerasUnproject const *u = ecs_field_is_set(it, 3) ? ecs_field_shared(it, EgCamerasUnproject, 3) : NULL;
	float pixelScale = (u != NULL && u->pixelScale > 0.0f) ? u->pixelScale : 1.0f;
	for (int32_t i = 0; i < it->count; ++i) {
		float dx  = p0->x - x[i].matrix.c2[0];
		float dy  = p0->y - x[i].matrix.c2[1];
		float det = m3f32_det(&x[i].matrix);
		float sx  = hypotf(x[i].matrix.c0[0], x[i].matrix.c0[1]);
		float sy  = hypotf(x[i].matrix.c1[0], x[i].matrix.c1[1]);
		l[i].pixel_x = sx > 1e-8f ? pixelScale / sx : pixelScale;
		l[i].pixel_y = sy > 1e-8f ? pixelScale / sy : pixelScale;
		if (fabsf(det) < 1e-8f) {
			continue;
		}
		l[i].x = (dx * x[i].matrix.c1[1] - dy * x[i].matrix.c1[0]) / det;
		l[i].y = (dy * x[i].matrix.c0[0] - dx * x[i].matrix.c0[1]) / det;
	}
}

// Derives edge distances from the local mouse position.
static void EgUiRectangleBorder_Update(ecs_iter_t *it)
{
	EgShapesRectangle const      *r = ecs_field_self(it, EgShapesRectangle, 0);
	EgUiLocalMousePosition const *l = ecs_field_self(it, EgUiLocalMousePosition, 1);
	EgIntersectsRectangleBorder  *b = ecs_field_self(it, EgIntersectsRectangleBorder, 2);
	for (int32_t i = 0; i < it->count; ++i) {
		float hw     = fabsf(r[i].w) * 0.5f;
		float hh     = fabsf(r[i].h) * 0.5f;
		b[i].left    = l[i].x + hw;
		b[i].right   = hw - l[i].x;
		b[i].bottom  = l[i].y + hh;
		b[i].top     = hh - l[i].y;
		b[i].minimum = fminf(fminf(b[i].left, b[i].right), fminf(b[i].bottom, b[i].top));
	}
}

void EgUiLocalMouseImport(ecs_world_t *world)
{
	ECS_IMPORT(world, EgShapes);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgCameras);
	ECS_IMPORT(world, EgIntersects);

	ECS_MODULE(world, EgUiLocalMouse);
	ecs_set_name_prefix(world, "EgUiLocalMouse");

	ECS_COMPONENT_DEFINE(world, EgUiLocalMousePosition);
	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgUiLocalMousePosition),
	.members = {
	{.name = "x", .type = ecs_id(ecs_f32_t)},
	{.name = "y", .type = ecs_id(ecs_f32_t)},
	{.name = "pixel_x", .type = ecs_id(ecs_f32_t)},
	{.name = "pixel_y", .type = ecs_id(ecs_f32_t)},
	}});
	// Rectangle borders always need the local mouse position.
	ecs_add_pair(world, ecs_id(EgIntersectsRectangleBorder), EcsWith, ecs_id(EgUiLocalMousePosition));

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiLocalMousePosition_Update"}),
	.phase       = EcsPreStore,
	.callback    = EgUiLocalMousePosition_Update,
	.query.terms = {
	{.id = ecs_id(Position2), .trav = EgUiMouse, .src.id = EcsUp, .inout = EcsIn},
	{.id = ecs_id(WorldTransform3), .inout = EcsIn},
	{.id = ecs_id(EgUiLocalMousePosition), .inout = EcsInOut},
	{.id = ecs_id(EgCamerasUnproject), .trav = EgUiMouse, .src.id = EcsUp, .oper = EcsOptional, .inout = EcsIn},
	}});
	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiRectangleBorder_Update"}),
	.phase       = EcsPreStore,
	.callback    = EgUiRectangleBorder_Update,
	.query.terms = {
	{.id = ecs_id(EgShapesRectangle), .inout = EcsIn},
	{.id = ecs_id(EgUiLocalMousePosition), .inout = EcsIn},
	{.id = ecs_id(EgIntersectsRectangleBorder), .inout = EcsInOut},
	}});
}
