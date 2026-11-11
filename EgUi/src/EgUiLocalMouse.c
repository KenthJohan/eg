#include "EgUi/EgUiLocalMouse.h"
#include <EgUi.h>

#include <EgSpatials.h>
#include <EgCameras.h>
#include <ecsx.h>
#include <math.h>

ECS_COMPONENT_DECLARE(EgUiLocalMousePosition);

// Transforms the shared mouse position into each entity's local space once for other systems.
static void EgUiLocalMousePosition_Update(ecs_iter_t *it)
{
	Position2 const          *p0          = ecs_field_shared(it, Position2, 1);
	WorldTransform3 const    *x           = ecs_field_self(it, WorldTransform3, 2);
	EgUiLocalMousePosition   *l           = ecs_field_self(it, EgUiLocalMousePosition, 3);
	EgCamerasUnproject const *u           = ecs_field_is_set(it, 4) ? ecs_field_shared(it, EgCamerasUnproject, 4) : NULL;
	float                     pixel_scale = (u != NULL && u->pixel_scale > 0.0f) ? u->pixel_scale : 1.0f;
	for (int32_t i = 0; i < it->count; ++i) {
		float dx     = p0->x - x[i].matrix.c2[0];
		float dy     = p0->y - x[i].matrix.c2[1];
		float det    = m3f32_det(&x[i].matrix);
		float sx     = hypotf(x[i].matrix.c0[0], x[i].matrix.c0[1]);
		float sy     = hypotf(x[i].matrix.c1[0], x[i].matrix.c1[1]);
		l[i].pixel_x = sx > 1e-8f ? pixel_scale / sx : pixel_scale;
		l[i].pixel_y = sy > 1e-8f ? pixel_scale / sy : pixel_scale;
		if (fabsf(det) < 1e-8f) {
			continue;
		}
		l[i].x      = (dx * x[i].matrix.c1[1] - dy * x[i].matrix.c1[0]) / det;
		l[i].y      = (dy * x[i].matrix.c0[0] - dx * x[i].matrix.c0[1]) / det;
	}
}

void EgUiLocalMouseImport(ecs_world_t *world)
{
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgCameras);

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

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiLocalMousePosition_Update"}),
	.phase       = EcsPreStore,
	.callback    = EgUiLocalMousePosition_Update,
	.query.terms = {
	// The GUI root holds (EgUiMouse, camera mouse); descendants find it by traversing up.
	{.first.id = EgUiMouse, .second.name = "$mouse", .src.id = EcsSelf | EcsUp, .trav = EcsChildOf, .inout = EcsInOutNone},
	{.id = ecs_id(Position2), .src.name = "$mouse", .inout = EcsIn},
	{.id = ecs_id(WorldTransform3), .inout = EcsIn},
	{.id = ecs_id(EgUiLocalMousePosition), .inout = EcsInOut},
	{.id = ecs_id(EgCamerasUnproject), .src.name = "$mouse", .oper = EcsOptional, .inout = EcsIn},
	}});
}
