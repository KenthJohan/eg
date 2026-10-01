#include "EgUi.h"

#include <EgShapes.h>
#include <ecsx.h>
#include <math.h>

ECS_COMPONENT_DECLARE(EgUiFlow);

static void EgUiFlow_Reset(ecs_iter_t *it)
{
	EgUiFlow *flow = ecs_field_self(it, EgUiFlow, 0);
	for (int i = 0; i < it->count; ++i, ++flow) {
		flow->cursor  = (Position2){0};
		flow->max     = 0;
		flow->started = false;
	}
}

static void EgUiFlow_Update(ecs_iter_t *it)
{
	EgUiFlow          *pf = ecs_field_shared(it, EgUiFlow, 0);
	EgShapesRectangle *pr = ecs_field_shared(it, EgShapesRectangle, 1);
	Position2         *cp = ecs_field_self(it, Position2, 2);
	EgShapesRectangle *cr = ecs_field_self(it, EgShapesRectangle, 3);

	// direction is a unit vector (a 2d rotation applied to the x axis); only its
	// dominant axis is used to pick the flow axis, its sign picks the flow's start corner
	bool   horizontal  = pf->direction.x != 0;
	float  sign        = horizontal ? pf->direction.x : pf->direction.y;
	float  halfBound   = (horizontal ? pr->w : pr->h) * 0.5f;
	float  halfAcross  = (horizontal ? pr->h : pr->w) * 0.5f;
	float *edge        = horizontal ? &pf->cursor.x : &pf->cursor.y; // leading edge along the flow axis
	float *line        = horizontal ? &pf->cursor.y : &pf->cursor.x; // near edge of the current line

	if (!pf->started) {
		*edge      = sign < 0.0f ? halfBound : -halfBound;
		*line      = -halfAcross;
		pf->started = true;
	}

	for (int i = 0; i < it->count; ++i, ++cp, ++cr) {
		float childAlong  = horizontal ? cr->w : cr->h;
		float childAcross = horizontal ? cr->h : cr->w;

		// wrap to a new line once the child's far edge no longer fits along the flow axis
		bool atStart = *edge == (sign < 0.0f ? halfBound : -halfBound);
		bool fits    = sign >= 0.0f ? *edge + childAlong <= halfBound : *edge - childAlong >= -halfBound;
		if (!atStart && !fits) {
			*edge  = sign < 0.0f ? halfBound : -halfBound;
			*line += pf->max;
			pf->max = 0;
		}

		// positions are centers, so offset the edge by half the child's size
		float alongCenter  = *edge + (sign >= 0.0f ? childAlong : -childAlong) * 0.5f;
		float acrossCenter = *line + childAcross * 0.5f;
		if (horizontal) {
			cp->x = alongCenter;
			cp->y = acrossCenter;
		} else {
			cp->y = alongCenter;
			cp->x = acrossCenter;
		}

		*edge += sign >= 0.0f ? childAlong : -childAlong;
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
