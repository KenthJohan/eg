#include "EgUi/EgUiButtons.h"

#include <EgShapes.h>
#include <EgButtons.h>
#include <EgUi/EgUiLocalMouse.h>
#include <EgIntersects.h>
#include <EgSpatials.h>
#include <ecsx.h>
#include <math.h>

ECS_COMPONENT_DECLARE(EgUiButtonsButton);
ECS_COMPONENT_DECLARE(EgUiButtonsBorder);

static void EgUiButtonsButton_Update(ecs_iter_t *it)
{
	EgIntersectsRectangleBorder const *r = ecs_field_self(it, EgIntersectsRectangleBorder, 0);
	EgUiButtonsButton                 *b = ecs_field_self(it, EgUiButtonsButton, 1);
	EgButtonsState                    *s = ecs_field_shared(it, EgButtonsState, 2);
	for (int32_t i = 0; i < it->count; ++i, ++r, ++b) {
		bool held  = !!(EgButtonsState_get(s, b->key) & EG_BUTTONS_STATE_HELD);
		b->hovered = EG_INTERSECTS_RECTANGLE_BORDER_IS_INSIDE(*r);
		b->held    = (b->held && held) || (b->hovered && held);
		ecs_modified_id(it->world, it->entities[i], ecs_id(EgUiButtonsButton));
	}
}

static void EgUiButtonsButton_ClearDisabled(ecs_iter_t *it)
{
	EgUiButtonsButton *b = ecs_field_self(it, EgUiButtonsButton, 0);
	for (int32_t i = 0; i < it->count; ++i) {
		b[i].hovered = false;
		b[i].held    = false;
		ecs_modified_id(it->world, it->entities[i], ecs_id(EgUiButtonsButton));
	}
}

static uint8_t EgUiButtonsBorder_Hovered(EgIntersectsRectangleBorder const *border, float mx, float my)
{
	uint8_t edge = 0;
	if (border->bottom >= -my && border->top >= -my && fabsf(border->left) <= mx)
		edge |= EG_UI_EDGE_LEFT;
	if (border->bottom >= -my && border->top >= -my && fabsf(border->right) <= mx)
		edge |= EG_UI_EDGE_RIGHT;
	if (border->left >= -mx && border->right >= -mx && fabsf(border->bottom) <= my)
		edge |= EG_UI_EDGE_BOTTOM;
	if (border->left >= -mx && border->right >= -mx && fabsf(border->top) <= my)
		edge |= EG_UI_EDGE_TOP;
	return edge;
}

static void EgUiButtonsBorder_Update(ecs_iter_t *it)
{
	EgIntersectsRectangleBorder const *r = ecs_field_self(it, EgIntersectsRectangleBorder, 0);
	EgUiButtonsBorder                 *b = ecs_field_self(it, EgUiButtonsBorder, 1);
	EgButtonsState                    *s = ecs_field_shared(it, EgButtonsState, 2);
	EgUiLocalMousePosition const      *l = ecs_field_self(it, EgUiLocalMousePosition, 5);
	for (int32_t i = 0; i < it->count; ++i, ++r, ++b, ++l) {
		ecs_entity_t e       = it->entities[i];
		bool         pressed = !!(EgButtonsState_get(s, b->key) & EG_BUTTONS_STATE_PRESSED);
		bool         held    = !!(EgButtonsState_get(s, b->key) & EG_BUTTONS_STATE_HELD);
		uint8_t      hover = EgUiButtonsBorder_Hovered(r, b->margin * l->pixel_x, b->margin * l->pixel_y);
		uint8_t      grab    = b->grab;

		if (!held) {
			grab = 0;
		} else if (pressed && hover) {
			grab = hover;
		}

		if ((hover != b->hover) || (grab != b->grab)) {
			b->hover = hover;
			b->grab  = grab;
			ecs_modified_id(it->world, e, ecs_id(EgUiButtonsBorder));
		}
	}
}

static void EgUiButtonsBorder_ClearDisabled(ecs_iter_t *it)
{
	EgUiButtonsBorder *b = ecs_field_self(it, EgUiButtonsBorder, 0);
	for (int32_t i = 0; i < it->count; ++i, ++b) {
		b->hover = 0;
		b->grab  = 0;
		ecs_modified_id(it->world, it->entities[i], ecs_id(EgUiButtonsBorder));
	}
}

void EgUiButtonsImport(ecs_world_t *world)
{
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

	ECS_COMPONENT_DEFINE(world, EgUiButtonsBorder);

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgUiButtonsBorder),
	.members = {
	{.name = "key", .type = ecs_id(ecs_u32_t)},
	{.name = "margin", .type = ecs_id(ecs_f32_t)},
	{.name = "hover", .type = ecs_id(ecs_u8_t)},
	{.name = "grab", .type = ecs_id(ecs_u8_t)},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiButtonsBorder_Update"}),
	.phase       = EcsPreStore,
	.callback    = EgUiButtonsBorder_Update,
	.query.terms = {
	{.id = ecs_id(EgIntersectsRectangleBorder), .inout = EcsIn},
	{.id = ecs_id(EgUiButtonsBorder), .inout = EcsInOut},
	{.id = ecs_id(EgButtonsState), .src.id = ecs_id(EgButtonsState), .inout = EcsIn},
	{.id = ecs_id(EgShapesRectangle), .src.id = EcsSelf, .inout = EcsIn},
	{.id = EcsDisabled, .trav = EcsChildOf, .src.id = EcsUp, .oper = EcsNot},
	{.id = ecs_id(EgUiLocalMousePosition), .src.id = EcsSelf, .inout = EcsIn},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiButtonsBorder_ClearDisabled"}),
	.phase       = EcsPreStore,
	.callback    = EgUiButtonsBorder_ClearDisabled,
	.query.terms = {
	{.id = ecs_id(EgUiButtonsBorder), .inout = EcsInOut},
	{.id = EcsDisabled, .trav = EcsChildOf, .src.id = EcsUp},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiButtonsButton_Update"}),
	.phase       = EcsPreStore,
	.callback    = EgUiButtonsButton_Update,
	.query.terms = {
	{.id = ecs_id(EgIntersectsRectangleBorder), .inout = EcsIn},
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
