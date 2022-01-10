#include "eg_quantity.h"



ECS_COMPONENT_DECLARE(EgPosition2F32);
ECS_COMPONENT_DECLARE(EgPosition2I32);
ECS_COMPONENT_DECLARE(EgVelocity2F32);
ECS_COMPONENT_DECLARE(EgAcceleration2F32);
ECS_COMPONENT_DECLARE(EgMassF32);
ECS_COMPONENT_DECLARE(EgForce2F32);



static void Kinematic1(ecs_iter_t *it)
{
	EgPosition2F32 *p = ecs_term(it, EgPosition2F32, 1);
	EgVelocity2F32 *v = ecs_term(it, EgVelocity2F32, 2);
	for (int i = 0; i < it->count; i ++)
	{
		p[i].x += v[i].x;
		p[i].y += v[i].y;
	}
}

static void Kinematic2(ecs_iter_t *it)
{
	EgVelocity2F32 *v = ecs_term(it, EgVelocity2F32, 1);
	EgAcceleration2F32 *a = ecs_term(it, EgAcceleration2F32, 2);
	for (int i = 0; i < it->count; i ++)
	{
		v[i].x += a[i].x;
		v[i].y += a[i].y;
	}
}

static void Kinematic3(ecs_iter_t *it)
{
	EgMassF32 *m = ecs_term(it, EgMassF32, 1);
	EgForce2F32 *f = ecs_term(it, EgForce2F32, 2);
	EgAcceleration2F32 *a = ecs_term(it, EgAcceleration2F32, 2);
	for (int i = 0; i < it->count; i ++)
	{
		a[i].x = f[i].x / m[i].value;
		a[i].y = f[i].y / m[i].value;
	}
}


void FlecsComponentsEgQuantityImport(ecs_world_t *world)
{
	ECS_MODULE(world, FlecsComponentsEgQuantity);


	ECS_COMPONENT_DEFINE(world, EgPosition2F32);
	ECS_COMPONENT_DEFINE(world, EgPosition2I32);
	ECS_COMPONENT_DEFINE(world, EgVelocity2F32);
	ECS_COMPONENT_DEFINE(world, EgAcceleration2F32);
	ECS_COMPONENT_DEFINE(world, EgMassF32);
	ECS_COMPONENT_DEFINE(world, EgForce2F32);

	ecs_set_name_prefix(world, "Eg");

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgPosition2F32),
	.members = {
	{ .name = "x", .type = ecs_id(ecs_f32_t) },
	{ .name = "y", .type = ecs_id(ecs_f32_t) }
	}
	});

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgPosition2I32),
	.members = {
	{ .name = "x", .type = ecs_id(ecs_i32_t) },
	{ .name = "y", .type = ecs_id(ecs_i32_t) }
	}
	});

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgMassF32),
	.members = {
	{ .name = "value", .type = ecs_id(ecs_i32_t) }
	}
	});

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgForce2F32),
	.members = {
	{ .name = "x", .type = ecs_id(ecs_i32_t) },
	{ .name = "y", .type = ecs_id(ecs_i32_t) }
	}
	});



	ECS_SYSTEM(world, Kinematic1, EcsOnUpdate, EgPosition2F32, EgVelocity2F32);
	ECS_SYSTEM(world, Kinematic2, EcsOnUpdate, EgVelocity2F32, EgAcceleration2F32);


}
