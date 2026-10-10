#include "EgIntersects.h"
#include <EgSpatials.h>
#include <EgShapes.h>
#include <ecsx.h>

ECS_COMPONENT_DECLARE(EgIntersectsOverlap);

static void EgIntersectsOverlap_Test(ecs_iter_t *it)
{
	Position2               *p0 = ecs_field_shared(it, Position2, 0);
	EgShapesRectangle const *r  = ecs_field_self(it, EgShapesRectangle, 1);
	WorldTransform3 const   *x  = ecs_field_self(it, WorldTransform3, 2);
	EgIntersectsOverlap     *b  = ecs_field_self(it, EgIntersectsOverlap, 3);
	for (int32_t i = 0; i < it->count; ++i, ++r, ++x, ++b) {
		char const *name = ecs_get_name(it->world, it->entities[i]);
		float       dx   = p0->x - x->matrix.c2[0];
		float       dy   = p0->y - x->matrix.c2[1];
		float       det  = x->matrix.c0[0] * x->matrix.c1[1] - x->matrix.c1[0] * x->matrix.c0[1];
		if (fabsf(det) < 1e-8f) {
			continue;
		}
		float local_x = (dx * x->matrix.c1[1] - dy * x->matrix.c1[0]) / det;
		float local_y = (dy * x->matrix.c0[0] - dx * x->matrix.c0[1]) / det;
		b->overlap    = (fabsf(local_x) <= fabsf(r->w) * 0.5f) && (fabsf(local_y) <= fabsf(r->h) * 0.5f);
	}
}

void EgIntersectsImport(ecs_world_t *world)
{
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgShapes);

	ECS_MODULE(world, EgIntersects);
	ecs_set_name_prefix(world, "EgIntersects");

	ECS_COMPONENT_DEFINE(world, EgIntersectsOverlap);
	ecs_add_id(world, ecs_id(EgIntersectsOverlap), EcsTraversable);

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgIntersectsOverlap),
	.members = {
	{.name = "overlap", .type = ecs_id(ecs_bool_t)},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgIntersectsOverlap_Test"}),
	.phase       = EcsPreStore,
	.callback    = EgIntersectsOverlap_Test,
	.query.terms = {
	{.id = ecs_id(Position2), .trav = ecs_id(EgIntersectsOverlap), .src.id = EcsUp, .inout = EcsIn},
	{.id = ecs_id(EgShapesRectangle), .inout = EcsIn},
	{.id = ecs_id(WorldTransform3), .inout = EcsIn},
	{.id = ecs_id(EgIntersectsOverlap), .inout = EcsOut},
	}});
}
