#include "EgButtons.h"

#include <stdlib.h>
#include <stdio.h>
#include <EgShapes.h>
#include <EgSpatials.h>
#include <ecsx.h>

ECS_COMPONENT_DECLARE(EgButtonsDevice);
ECS_COMPONENT_DECLARE(EgButtonsState);
ECS_COMPONENT_DECLARE(EgButtonsBinding);
ECS_COMPONENT_DECLARE(EgButtonsActionToggleEntity);
ECS_COMPONENT_DECLARE(EgButtonsObliqueBinding);

/**
 * Get the state of a button.
 *
 * @param key The key code.
 * @param buttons The buttons state.
 * @return The state of the button, or -1 if out of range.
 */
static int32_t get_button_state(int32_t key, uint8_t mask, EgButtonsState *buttons)
{
	uint8_t  b     = (key >> 0) & 0xFF;
	uint8_t  s     = (key >> 16) & 0xFF;
	int32_t  max[] = {EG_BUTTONS_SCANCODES_MAX, EG_BUTTONS_MOUSE_MAX};
	uint8_t *set[] = {buttons->scancode, buttons->mouse};
	if ((s >= 2) || (b >= max[s])) {
		return 0;
	}
	return (set[s][b] & mask) ? 1 : 0;
}

static void System_Toggle(ecs_iter_t *it)
{
	ecs_log_set_level(1);
	EgButtonsState              *buttons = ecs_field(it, EgButtonsState, 0);              // singleton
	EgButtonsActionToggleEntity *a        = ecs_field(it, EgButtonsActionToggleEntity, 1); // self
	for (int i = 0; i < it->count; ++i, ++a) {
		bool state = get_button_state(a->button, a->mask, buttons);
		if (!state) {
			continue;
		}
		printf("toggle(%s,%s)\n", ecs_get_name(it->world, a->entity), ecs_get_name(it->world, a->toggle));
		ecsx_toggle(it->world, a->entity, a->relation, a->toggle);
	}
	ecs_log_set_level(0);
}

/*
char *str = ecs_ptr_to_expr(it->world, a->comonent, ptr);
printf("%s\n", str); // {x: 10, y: 20}
ecs_os_free(str);
*/

static void System_Bindings(ecs_iter_t *it)
{
	ecs_log_set_level(1);
	EgButtonsState   *buttons = ecs_field(it, EgButtonsState, 0);   // singleton
	EgButtonsBinding *a       = ecs_field(it, EgButtonsBinding, 1); // self
	for (int i = 0; i < it->count; ++i, ++a) {
		void *ptr = ecs_get_mut_id(it->world, a->entity, a->component);
		if (ptr == NULL) {
			ecs_dbg("ecs_get_id(%s,%s) == NULL", ecs_get_name(it->world, a->entity), ecs_get_name(it->world, a->component));
			continue;
		}
		int32_t v0    = get_button_state(a->button0, a->mask, buttons);
		int32_t v1    = get_button_state(a->button1, a->mask, buttons);
		int32_t delta = v0 - v1;
		float  *f     = (float *)((uint8_t *)ptr + a->byte_offset);
		(*f)          = (float)delta * a->factor;
	}
	ecs_log_set_level(0);
}

void EgButtonsObliqueBinding_System_Clear(ecs_iter_t *it)
{
	EgButtonsObliqueBinding *o = ecs_field_shared(it, EgButtonsObliqueBinding, 0);
	for (int i = 0; i < it->count; i++) {
		ecs_remove_id(it->world, it->entities[i], o->tag);
	}
}

void EgButtonsObliqueBinding_System_Update(ecs_iter_t *it)
{
	EgButtonsObliqueBinding *o       = ecs_field_shared(it, EgButtonsObliqueBinding, 0);
	EgButtonsState          *buttons = ecs_field_shared(it, EgButtonsState, 1); // singleton
	for (int i = 0; i < it->count; i++) {
		int32_t button_enable = get_button_state(o->button, o->mask, buttons);
		if (button_enable) {
			ecs_add_id(it->world, it->entities[i], o->tag);
		}
	}
}

void EgButtonsObliqueBinding_Observer(ecs_iter_t *it)
{
	EgButtonsObliqueBinding *o = ecs_field_self(it, EgButtonsObliqueBinding, 0);

	for (int i = 0; i < it->count; i++) {
		ecs_entity_t e = it->entities[i];
		if (it->event == EcsOnSet) {
			ecs_system(it->world,
			{.entity  = ecs_entity(it->world, {.add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
			.callback = EgButtonsObliqueBinding_System_Clear,
			.query.terms =
			{
			{.id = ecs_id(EgButtonsObliqueBinding), .src.id = e},
			{.id = o->term, .src.id = EcsSelf},
			}});
			ecs_system(it->world,
			{.entity  = ecs_entity(it->world, {.add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
			.callback = EgButtonsObliqueBinding_System_Update,
			.query.terms =
			{
			{.id = ecs_id(EgButtonsObliqueBinding), .src.id = e},
			{.id = ecs_id(EgButtonsState), .src.id = ecs_id(EgButtonsState)},
			{.id = o->term, .src.id = EcsSelf},
			}});
		}
	}
}

void EgButtonsImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgButtons);
	ecs_set_name_prefix(world, "EgButtons");

	ECS_COMPONENT_DEFINE(world, EgButtonsDevice);
	ECS_COMPONENT_DEFINE(world, EgButtonsState);
	ECS_COMPONENT_DEFINE(world, EgButtonsBinding);
	ECS_COMPONENT_DEFINE(world, EgButtonsActionToggleEntity);
	ECS_COMPONENT_DEFINE(world, EgButtonsObliqueBinding);

	ecs_struct(world,
	{.entity = ecs_id(EgButtonsState),
	.members = {
	{.name = "scancode", .type = ecs_id(ecs_u8_t), .count = EG_BUTTONS_SCANCODES_MAX},
	{.name = "mouse", .type = ecs_id(ecs_u8_t), .count = EG_BUTTONS_MOUSE_MAX},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgButtonsDevice),
	.members = {
	{.name = "id", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgButtonsBinding),
	.members = {
	{.name = "button0", .type = ecs_id(ecs_i32_t)},
	{.name = "button1", .type = ecs_id(ecs_i32_t)},
	{.name = "mask", .type = ecs_id(ecs_u8_t)},
	{.name = "entity", .type = ecs_id(ecs_entity_t)},
	{.name = "component", .type = ecs_id(ecs_id_t)},
	{.name = "byte_offset", .type = ecs_id(ecs_u8_t)},
	{.name = "factor", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgButtonsActionToggleEntity),
	.members = {
	{.name = "button", .type = ecs_id(ecs_i32_t)},
	{.name = "mask", .type = ecs_id(ecs_u8_t)},
	{.name = "entity", .type = ecs_id(ecs_entity_t)},
	{.name = "relation", .type = ecs_id(ecs_entity_t)},
	{.name = "toggle", .type = ecs_id(ecs_entity_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgButtonsObliqueBinding),
	.members = {
	{.name = "term", .type = ecs_id(ecs_id_t)},
	{.name = "button", .type = ecs_id(ecs_i32_t)},
	{.name = "mask", .type = ecs_id(ecs_u8_t)},
	{.name = "tag", .type = ecs_id(ecs_entity_t)},
	}});

	ecs_system(world,
	{.entity  = ecs_entity(world, {.name = "System_Toggle", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_Toggle,
	.query.terms =
	{
	{.id = ecs_id(EgButtonsState), .src.id = ecs_id(EgButtonsState)},
	{.id = ecs_id(EgButtonsActionToggleEntity), .src.id = EcsSelf},
	}});

	ecs_system(world,
	{.entity  = ecs_entity(world, {.name = "System_Bindings", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_Bindings,
	.query.terms =
	{
	{.id = ecs_id(EgButtonsState), .src.id = ecs_id(EgButtonsState)},
	{.id = ecs_id(EgButtonsBinding), .src.id = EcsSelf},
	}});

	ecs_observer(world,
	{.query   = {.terms = {{.id = ecs_id(EgButtonsObliqueBinding)}}},
	.events   = {EcsOnSet},
	.callback = EgButtonsObliqueBinding_Observer});
}
