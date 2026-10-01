#include "EgUi.h"

#include <EgShapes.h>
#include <ecsx.h>
#include <math.h>

ECS_COMPONENT_DECLARE(EgUiFlow);

static void EgUiFlow_Reset(ecs_iter_t *it)
{
	EgUiFlow *flow = ecs_field_self(it, EgUiFlow, 0);
	for (int i = 0; i < it->count; ++i, ++flow) {
		flow->cursor = (Position2){0};
		flow->max    = 0;
	}
}

static void EgUiFlow_Update(ecs_iter_t *it)
{
	EgUiFlow          *pf = ecs_field_shared(it, EgUiFlow, 0);
	EgShapesRectangle *pr = ecs_field_shared(it, EgShapesRectangle, 1);
	Position2         *cp = ecs_field_self(it, Position2, 2);
	EgShapesRectangle *cr = ecs_field_self(it, EgShapesRectangle, 3);

	bool   horizontal = pf->direction.x != 0;
	float  sign       = horizontal ? pf->direction.x : pf->direction.y;
	float  bound      = horizontal ? pr->w : pr->h;
	float *along      = horizontal ? &pf->cursor.x : &pf->cursor.y;
	float *across     = horizontal ? &pf->cursor.y : &pf->cursor.x;

	// a reversed flow starts at the far edge of the parent instead of at zero
	if (*along == 0.0f && sign < 0.0f) *along = bound;

	for (int i = 0; i < it->count; ++i, ++cp, ++cr) {
		float childAlong  = horizontal ? cr->w : cr->h;
		float childAcross = horizontal ? cr->h : cr->w;

		// wrap to the next line once the child no longer fits along the flow axis
		if (*along != (sign < 0.0f ? bound : 0.0f) &&
		    (sign >= 0.0f ? *along + childAlong > bound : *along - childAlong < 0.0f)) {
			*along   = sign < 0.0f ? bound : 0.0f;
			*across += pf->max;
			pf->max  = 0;
		}

		float alongPos = sign >= 0.0f ? *along : *along - childAlong;
		if (horizontal) {
			cp->x = alongPos;
			cp->y = *across;
		} else {
			cp->y = alongPos;
			cp->x = *across;
		}

		*along += sign >= 0.0f ? childAlong : -childAlong;
		if (childAcross > pf->max) pf->max = childAcross;
	}
}

void EgUiImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgUi);
	ecs_set_name_prefix(world, "EgUi");

	ECS_COMPONENT_DEFINE(world, EgUiFlow);

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgUiFlow),
	.members = {
	{.name = "cursor", .type = ecs_id(Position2)},
	{.name = "direction", .type = ecs_id(V2f32)},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiFlow_Reset"}),
	.phase       = EcsOnUpdate,
	.callback    = EgUiFlow_Reset,
	.query.terms = {
	{.id = ecs_id(EgUiFlow), .inout = EcsInOut},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiFlow_Update"}),
	.phase       = EcsOnUpdate,
	.callback    = EgUiFlow_Update,
	.query.terms = {
	{.id = ecs_id(EgUiFlow), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsInOut},
	{.id = ecs_id(EgShapesRectangle), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn},
	{.id = ecs_id(Position2), .inout = EcsOut},
	{.id = ecs_id(EgShapesRectangle), .inout = EcsIn},
	}});
}
