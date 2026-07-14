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

static void System_Toggle(ecs_iter_t *it)
{
	ecs_log_set_level(1);
	EgButtonsState *keyboard = ecs_field(it, EgButtonsState, 0);                    // singleton
	EgButtonsActionToggleEntity *a = ecs_field(it, EgButtonsActionToggleEntity, 1); // self
	for (int i = 0; i < it->count; ++i, ++a) {
		uint8_t k = keyboard->state[a->key];
		if ((k & a->mask) == 0) {
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
	EgButtonsState *keyboard = ecs_field(it, EgButtonsState, 0); // singleton
	EgButtonsBinding *a = ecs_field(it, EgButtonsBinding, 1);    // self
	for (int i = 0; i < it->count; ++i, ++a) {
		void *ptr = ecs_get_mut_id(it->world, a->entity, a->comonent);
		if (ptr == NULL) {
			ecs_dbg("ecs_get_id(%s,%s) == NULL", ecs_get_name(it->world, a->entity), ecs_get_name(it->world, a->comonent));
			continue;
		}
		uint8_t k0 = keyboard->state[a->key0];
		uint8_t k1 = keyboard->state[a->key1];
		int delta = !!(k0 & a->mask) - !!(k1 & a->mask);
		float *f = (float *)((uint8_t *)ptr + a->byte_offset);
		(*f) = (float)delta * a->factor;
	}
	ecs_log_set_level(0);
}

void EgButtonsImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgButtons);
	ecs_set_name_prefix(world, "EgButtons");

	ECS_COMPONENT_DEFINE(world, EgButtonsDevice);
	ECS_COMPONENT_DEFINE(world, EgButtonsState);
	ECS_COMPONENT_DEFINE(world, EgButtonsBinding);
	ECS_COMPONENT_DEFINE(world, EgButtonsActionToggleEntity);

	ecs_struct(world,
	{.entity = ecs_id(EgButtonsState),
	.members = {
	{.name = "scancode", .type = ecs_id(ecs_u8_t), .count = EG_BUTTONS_KEYS_MAX},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgButtonsDevice),
	.members = {
	{.name = "id", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgButtonsBinding),
	.members = {
	{.name = "key0", .type = ecs_id(ecs_i32_t)},
	{.name = "key1", .type = ecs_id(ecs_i32_t)},
	{.name = "mask", .type = ecs_id(ecs_u8_t)},
	{.name = "entity", .type = ecs_id(ecs_entity_t)},
	{.name = "comonent", .type = ecs_id(ecs_id_t)},
	{.name = "byte_offset", .type = ecs_id(ecs_u8_t)},
	{.name = "factor", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgButtonsActionToggleEntity),
	.members = {
	{.name = "key", .type = ecs_id(ecs_i32_t)},
	{.name = "mask", .type = ecs_id(ecs_u8_t)},
	{.name = "entity", .type = ecs_id(ecs_entity_t)},
	{.name = "relation", .type = ecs_id(ecs_entity_t)},
	{.name = "toggle", .type = ecs_id(ecs_entity_t)},
	}});

	ecs_system(world,
	{.entity = ecs_entity(world, {.name = "System_Toggle", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_Toggle,
	.query.terms =
	{
	{.id = ecs_id(EgButtonsState), .src.id = ecs_id(EgButtonsState)},
	{.id = ecs_id(EgButtonsActionToggleEntity), .src.id = EcsSelf},
	}});

	ecs_system(world,
	{.entity = ecs_entity(world, {.name = "System_Bindings", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_Bindings,
	.query.terms =
	{
	{.id = ecs_id(EgButtonsState), .src.id = ecs_id(EgButtonsState)},
	{.id = ecs_id(EgButtonsBinding), .src.id = EcsSelf},
	}});
}
