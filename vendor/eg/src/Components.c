#include "eg/Components.h"

ECS_COMPONENT_DECLARE(Pass);
ECS_COMPONENT_DECLARE(Color);
ECS_COMPONENT_DECLARE(Color32);
ECS_COMPONENT_DECLARE(String);
ECS_COMPONENT_DECLARE(ShapeBuffer);
ECS_COMPONENT_DECLARE(ShapeElement);
ECS_COMPONENT_DECLARE(ShowDrawReference);

ECS_TAG_DECLARE(EgUse);
ECS_TAG_DECLARE(EgUpdate);

ECS_CTOR(String, ptr, {
	ecs_trace("String::CTOR");
	ptr->value = NULL;
})

ECS_DTOR(String, ptr, {
	ecs_trace("String::DTOR");
	ecs_os_free(ptr->value);
})

ECS_MOVE(String, dst, src, {
	ecs_trace("String::MOVE");
	ecs_os_free(dst->value);
	dst->value = src->value;
	src->value = NULL;
})

ECS_COPY(String, dst, src, {
	ecs_trace("String::COPY");
	ecs_os_free(dst->value);
	dst->value = ecs_os_strdup(src->value);
})

void hook_callback(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	ecs_entity_t event = it->event;

	for (int i = 0; i < it->count; i++) {
		ecs_entity_t e = it->entities[i];
		ecs_trace("%s: %s", ecs_get_name(world, event), ecs_get_name(world, e));
	}
}



ECS_CTOR(ShapeBuffer, ptr, {
	ecs_os_memset_t(ptr, 0, ShapeBuffer);
})















void ComponentsImport(ecs_world_t *world)
{
	ECS_MODULE(world, Components);
	ecs_set_name_prefix(world, "Components");

	ECS_COMPONENT_DEFINE(world, Pass);



	ECS_COMPONENT_DEFINE(world, Color);
	ECS_COMPONENT_DEFINE(world, Color32);
	ECS_COMPONENT_DEFINE(world, String);
	ECS_COMPONENT_DEFINE(world, ShapeBuffer);
	ECS_COMPONENT_DEFINE(world, ShapeElement);
	ECS_COMPONENT_DEFINE(world, ShowDrawReference);
	

	ECS_TAG_DEFINE(world, EgUse);
	ECS_TAG_DEFINE(world, EgUpdate);
	ecs_add_id(world, EgUse, EcsTraversable);
	ecs_add_id(world, EgUpdate, EcsTraversable);

	ecs_set_hooks(world, String, {.ctor = ecs_ctor(String), .move = ecs_move(String), .copy = ecs_copy(String), .dtor = ecs_dtor(String), .on_add = hook_callback, .on_remove = hook_callback, .on_set = hook_callback});
	ecs_set_hooks(world, ShapeBuffer, {.ctor = ecs_ctor(ShapeBuffer)});



	ecs_struct(world,
	{.entity = ecs_id(Color32),
	.members = {
	{.name = "r", .type = ecs_id(ecs_u8_t)},
	{.name = "g", .type = ecs_id(ecs_u8_t)},
	{.name = "b", .type = ecs_id(ecs_u8_t)},
	{.name = "a", .type = ecs_id(ecs_u8_t)},
	}});



	ecs_struct(world,
	{.entity = ecs_id(ShapeElement),
	.members = {
	{.name = "x", .type = ecs_id(ecs_i32_t)},
	{.name = "y", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(String),
	.members = {
	{.name = "value", .type = ecs_id(ecs_string_t)},
	}});

}