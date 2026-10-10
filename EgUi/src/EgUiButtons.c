#include "EgUi/EgUiButtons.h"

#include <EgShapes.h>
#include <EgPhysics.h>
#include <EgButtons.h>
#include <EgIntersects.h>
#include <EgSpatials.h>
#include <ecsx.h>
#include <math.h>

ECS_COMPONENT_DECLARE(EgUiButtonsButton);

static void EgUiButtonsButton_Update(ecs_iter_t *it)
{
	EgIntersectsOverlap const *o = ecs_field_self(it, EgIntersectsOverlap, 0);
	WorldTransform3 const     *x = ecs_field_self(it, WorldTransform3, 2);
	EgUiButtonsButton         *b = ecs_field_self(it, EgUiButtonsButton, 3);
	EgButtonsState            *s = ecs_field_shared(it, EgButtonsState, 4);
	for (int32_t i = 0; i < it->count; ++i, ++o, ++x, ++b) {
		bool mouse_held = !!(EgButtonsState_get(s, b->key) & EG_BUTTONS_STATE_HELD);
		b->hovered      = o->overlap;
		b->held         = (b->held && mouse_held) || (b->hovered && mouse_held);
		ecs_modified_id(it->world, it->entities[i], ecs_id(EgUiButtonsButton));
	}
}

static void EgUiButtonsButton_ClearDisabled(ecs_iter_t *it)
{
	EgUiButtonsButton *b = ecs_field_self(it, EgUiButtonsButton, 0);
	for (int32_t i = 0; i < it->count; ++i) {
		b[i].hovered = false;
		b[i].held = false;
		ecs_modified_id(it->world, it->entities[i], ecs_id(EgUiButtonsButton));
	}
}

void EgUiButtonsImport(ecs_world_t *world)
{
	ECS_IMPORT(world, EgPhysics);
	ECS_IMPORT(world, EgShapes);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgButtons);
	ECS_IMPORT(world, EgIntersects);

	ECS_MODULE(world, EgUiButtons);
	ecs_set_name_prefix(world, "EgUiButtons");


	ECS_COMPONENT_DEFINE(world, EgUiButtonsButton);

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgUiButtonsButton),
	.members = {
	{.name = "key", .type = ecs_id(ecs_u32_t)},
	{.name = "hovered", .type = ecs_id(ecs_bool_t)},
	{.name = "held", .type = ecs_id(ecs_bool_t)},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiButtonsButton_Update"}),
	.phase       = EcsPreStore,
	.callback    = EgUiButtonsButton_Update,
	.query.terms = {
	{.id = ecs_id(EgIntersectsOverlap), .inout = EcsIn},
	{.id = ecs_id(EgUiButtonsButton), .inout = EcsInOut},
	{.id = ecs_id(EgButtonsState), .src.id = ecs_id(EgButtonsState), .inout = EcsIn},
	{.id = EcsDisabled, .trav = EcsChildOf, .src.id = EcsUp, .oper = EcsNot},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiButtonsButton_ClearDisabled"}),
	.phase       = EcsPreStore,
	.callback    = EgUiButtonsButton_ClearDisabled,
	.query.terms = {
	{.id = ecs_id(EgUiButtonsButton), .inout = EcsInOut},
	{.id = EcsDisabled, .trav = EcsChildOf, .src.id = EcsUp},
	}});
}
