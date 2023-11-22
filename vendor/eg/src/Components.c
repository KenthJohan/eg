#include "eg/Components.h"

ECS_COMPONENT_DECLARE(Window);
ECS_COMPONENT_DECLARE(Pass);
ECS_COMPONENT_DECLARE(Position2);
ECS_COMPONENT_DECLARE(Position3);
ECS_COMPONENT_DECLARE(Velocity2);
ECS_COMPONENT_DECLARE(Velocity3);
ECS_COMPONENT_DECLARE(Color);
ECS_COMPONENT_DECLARE(String);
ECS_COMPONENT_DECLARE(Torus);
ECS_COMPONENT_DECLARE(ShapeBuffer);
ECS_COMPONENT_DECLARE(ShapeElement);
ECS_COMPONENT_DECLARE(UpdateBuffer);
ECS_COMPONENT_DECLARE(Camera);
ECS_COMPONENT_DECLARE(Orientation);
ECS_COMPONENT_DECLARE(Rotate3);
ECS_COMPONENT_DECLARE(Transformation);
ECS_TAG_DECLARE(Use);

ECS_CTOR(String, ptr, {
	ptr->value = NULL;
})

ECS_DTOR(String, ptr, {
	ecs_os_free(ptr->value);
})

ECS_MOVE(String, dst, src, {
	ecs_os_free(dst->value);
	dst->value = src->value;
	src->value = NULL;
})

ECS_COPY(String, dst, src, {
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

void ComponentsImport(ecs_world_t *world)
{
	ECS_MODULE(world, Components);
	ECS_COMPONENT_DEFINE(world, Window);
	ECS_COMPONENT_DEFINE(world, Pass);
	ECS_COMPONENT_DEFINE(world, Position2);
	ECS_COMPONENT_DEFINE(world, Position3);
	ECS_COMPONENT_DEFINE(world, Velocity2);
	ECS_COMPONENT_DEFINE(world, Velocity3);
	ECS_COMPONENT_DEFINE(world, Color);
	ECS_COMPONENT_DEFINE(world, String);
	ECS_COMPONENT_DEFINE(world, Torus);
	ECS_COMPONENT_DEFINE(world, ShapeBuffer);
	ECS_COMPONENT_DEFINE(world, ShapeElement);
	ECS_COMPONENT_DEFINE(world, UpdateBuffer);
	ECS_COMPONENT_DEFINE(world, Camera);
	ECS_COMPONENT_DEFINE(world, Orientation);
	ECS_COMPONENT_DEFINE(world, Rotate3);
	ECS_COMPONENT_DEFINE(world, Transformation);
	ECS_TAG_DEFINE(world, Use);
	ecs_add_id(world, Use, EcsTraversable);
	ecs_set_hooks(world, String, {.ctor = ecs_ctor(String), .move = ecs_move(String), .copy = ecs_copy(String), .dtor = ecs_dtor(String), .on_add = hook_callback, .on_remove = hook_callback, .on_set = hook_callback});
}