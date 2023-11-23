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


ECS_CTOR(Orientation, ptr, {
	//QF32_IDENTITY;
	//printf("Orientation::ECS_CTOR\n");
	ptr->x = 0.0f;
	ptr->y = 0.0f;
	ptr->z = 0.0f;
	ptr->w = 1.0f;
})

ECS_CTOR(ShapeBuffer, ptr, {
	ecs_os_memset_t(ptr, 0, ShapeBuffer);
})

ECS_CTOR(Camera, ptr, {
	ptr->fov = 45;
})


void ComponentsImport(ecs_world_t *world)
{
	ECS_MODULE(world, Components);
	ecs_set_name_prefix(world, "Components");

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
	ecs_set_hooks(world, Orientation, {.ctor = ecs_ctor(Orientation)});
	ecs_set_hooks(world, ShapeBuffer, {.ctor = ecs_ctor(ShapeBuffer)});
	ecs_set_hooks(world, Camera, {.ctor = ecs_ctor(Camera)});


	ecs_struct(world, {.entity = ecs_id(Position2),
	                   .members = {
	                       {.name = "x", .type = ecs_id(ecs_f32_t)},
	                       {.name = "y", .type = ecs_id(ecs_f32_t)},
	                   }});

	ecs_struct(world, {.entity = ecs_id(Position3),
	                   .members = {
	                       {.name = "x", .type = ecs_id(ecs_f32_t)},
	                       {.name = "y", .type = ecs_id(ecs_f32_t)},
	                       {.name = "z", .type = ecs_id(ecs_f32_t)},
	                   }});

	ecs_struct(world, {.entity = ecs_id(Velocity2),
	                   .members = {
	                       {.name = "x", .type = ecs_id(ecs_f32_t)},
	                       {.name = "y", .type = ecs_id(ecs_f32_t)},
	                   }});

	ecs_struct(world, {.entity = ecs_id(Velocity3),
	                   .members = {
	                       {.name = "x", .type = ecs_id(ecs_f32_t)},
	                       {.name = "y", .type = ecs_id(ecs_f32_t)},
	                       {.name = "z", .type = ecs_id(ecs_f32_t)},
	                   }});

	ecs_struct(world, {.entity = ecs_id(Orientation),
	                   .members = {
	                       {.name = "x", .type = ecs_id(ecs_f32_t)},
	                       {.name = "y", .type = ecs_id(ecs_f32_t)},
	                       {.name = "z", .type = ecs_id(ecs_f32_t)},
	                       {.name = "w", .type = ecs_id(ecs_f32_t)},
	                   }});
					
	ecs_struct(world, {.entity = ecs_id(Rotate3),
	                   .members = {
	                       {.name = "dx", .type = ecs_id(ecs_f32_t)},
	                       {.name = "dy", .type = ecs_id(ecs_f32_t)},
	                       {.name = "dz", .type = ecs_id(ecs_f32_t)},
	                   }});

	ecs_struct(world, {.entity = ecs_id(Torus),
	                   .members = {
	                       {.name = "radius", .type = ecs_id(ecs_f32_t)},
	                       {.name = "ring_radius", .type = ecs_id(ecs_f32_t)},
	                       {.name = "rings", .type = ecs_id(ecs_i32_t)},
	                       {.name = "sides", .type = ecs_id(ecs_i32_t)},
	                       {.name = "random_colors", .type = ecs_id(ecs_i32_t)},
	                   }});

	ecs_struct(world, {.entity = ecs_id(Torus),
	                   .members = {
	                       {.name = "radius", .type = ecs_id(ecs_f32_t)},
	                       {.name = "ring_radius", .type = ecs_id(ecs_f32_t)},
	                       {.name = "rings", .type = ecs_id(ecs_i32_t)},
	                       {.name = "sides", .type = ecs_id(ecs_i32_t)},
	                       {.name = "random_colors", .type = ecs_id(ecs_i32_t)},
	                   }});

	ecs_struct(world, {.entity = ecs_id(ShapeElement),
	                   .members = {
	                       {.name = "x", .type = ecs_id(ecs_i32_t)},
	                       {.name = "y", .type = ecs_id(ecs_i32_t)},
	                   }});

	ecs_struct(world, {.entity = ecs_id(Camera),
	                   .members = {
	                       {.name = "fov", .type = ecs_id(ecs_f32_t)},
	                       {.name = "vp", .type = ecs_id(ecs_f32_t), .count = 16},
	                   }});
}