#include "EgIntersects.h"
#include <EgSpatials.h>
#include <EgShapes.h>
#include <EgPhysics.h>
#include <math.h>
#include <ecsx.h>

ECS_COMPONENT_DECLARE(EgIntersectsRectangleBorder);

static void EgIntersectsRectangleBorder_Test(ecs_iter_t *it)
{
	Position2 const         *p0 = ecs_field_shared(it, Position2, 0);
	EgShapesRectangle const *r  = ecs_field_self(it, EgShapesRectangle, 1);
	WorldTransform3 const   *x  = ecs_field_self(it, WorldTransform3, 2);
	EgIntersectsRectangleBorder *b = ecs_field_self(it, EgIntersectsRectangleBorder, 3);
	for (int32_t i = 0; i < it->count; ++i) {
		float dx  = p0->x - x[i].matrix.c2[0];
		float dy  = p0->y - x[i].matrix.c2[1];
		float det = m3f32_det(&x[i].matrix);
		if (fabsf(det) < 1e-8f) {
			continue;
		}
		float lx = (dx * x[i].matrix.c1[1] - dy * x[i].matrix.c1[0]) / det;
		float ly = (dy * x[i].matrix.c0[0] - dx * x[i].matrix.c0[1]) / det;
		float hw = fabsf(r[i].w) * 0.5f;
		float hh = fabsf(r[i].h) * 0.5f;
		float m  = b[i].margin;

		//printf("lx: %f, ly: %f, hw: %f, hh: %f, margin: %f\n", lx, ly, hw, hh, m);

		bool    in_x = fabsf(lx) <= hw + m;
		bool    in_y = fabsf(ly) <= hh + m;
		uint8_t e    = 0;
		if (in_y && fabsf(lx + hw) <= m)
			e |= EG_INTERSECTS_EDGE_LEFT;
		if (in_y && fabsf(lx - hw) <= m)
			e |= EG_INTERSECTS_EDGE_RIGHT;
		if (in_x && fabsf(ly + hh) <= m)
			e |= EG_INTERSECTS_EDGE_BOTTOM;
		if (in_x && fabsf(ly - hh) <= m)
			e |= EG_INTERSECTS_EDGE_TOP;
		b[i].edges   = e;
		b[i].local_x = lx;
		b[i].local_y = ly;
		b[i].left    = lx + hw;
		b[i].right   = hw - lx;
		b[i].bottom  = ly + hh;
		b[i].top     = hh - ly;
	}
}

void EgIntersectsImport(ecs_world_t *world)
{
	ECS_IMPORT(world, EgPhysics);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgShapes);

	ECS_MODULE(world, EgIntersects);
	ecs_set_name_prefix(world, "EgIntersects");

	ECS_COMPONENT_DEFINE(world, EgIntersectsRectangleBorder);

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgIntersectsRectangleBorder),
	.members = {
	{.name = "margin", .type = ecs_id(ecs_f32_t)},
	{.name = "edges", .type = ecs_id(ecs_u8_t)},
	{.name = "local_x", .type = ecs_id(ecs_f32_t)},
	{.name = "local_y", .type = ecs_id(ecs_f32_t)},
	{.name = "left", .type = ecs_id(ecs_f32_t)},
	{.name = "right", .type = ecs_id(ecs_f32_t)},
	{.name = "bottom", .type = ecs_id(ecs_f32_t)},
	{.name = "top", .type = ecs_id(ecs_f32_t)},
	}});
	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgIntersectsRectangleBorder_Test"}),
	.phase       = EcsPreStore,
	.callback    = EgIntersectsRectangleBorder_Test,
	.query.terms = {
	{.id = ecs_id(Position2), .trav = ecs_id(EgPhysicsOverlapChecking), .src.id = EcsUp, .inout = EcsIn},
	{.id = ecs_id(EgShapesRectangle), .inout = EcsIn},
	{.id = ecs_id(WorldTransform3), .inout = EcsIn},
	{.id = ecs_id(EgIntersectsRectangleBorder), .inout = EcsInOut},
	}});
}
