#include "eg_quantity.h"
#include "eg_base.h"
#include <math.h>


ECS_COMPONENT_DECLARE(EgPosition2F32);
ECS_COMPONENT_DECLARE(EgPosition2I32);
ECS_COMPONENT_DECLARE(EgVelocity2F32);
ECS_COMPONENT_DECLARE(EgAcceleration2F32);
ECS_COMPONENT_DECLARE(EgForce2F32);
ECS_COMPONENT_DECLARE(EgDrag2F32);
ECS_COMPONENT_DECLARE(EgMomentum2F32);
ECS_COMPONENT_DECLARE(EgMassF32);
ECS_COMPONENT_DECLARE(EgTimeF32);
ECS_COMPONENT_DECLARE(EgTemperatureF32);
ECS_COMPONENT_DECLARE(EgLengthF32);
ECS_COMPONENT_DECLARE(EgDensityF32);


static void Kinematic1(ecs_iter_t *it)
{
	EgPosition2F32 *p = ecs_term(it, EgPosition2F32, 1); // [inout]
	EgVelocity2F32 *v = ecs_term(it, EgVelocity2F32, 2); // [in]
	for (int i = 0; i < it->count; i ++)
	{
		//printf("%f %f\n", v[i].x, v[i].y);
		p[i].x += v[i].x;
		p[i].y += v[i].y;
	}
}

static void Kinematic2(ecs_iter_t *it)
{
	EgVelocity2F32     *v = ecs_term(it, EgVelocity2F32,     1); // [in]
	EgAcceleration2F32 *a = ecs_term(it, EgAcceleration2F32, 2); // [inout]
	for (int i = 0; i < it->count; i ++)
	{
		v[i].x += a[i].x;
		v[i].y += a[i].y;
		//float k = 1.0f;
		//v[i].x = EG_CLAMP(v[i].x, -k, k);
		//v[i].y = EG_CLAMP(v[i].y, -k, k);
		EG_ASSERT(eg_isnan(v[i].x) == 0);
		EG_ASSERT(eg_isnan(v[i].y) == 0);
		EG_ASSERT(isinf(v[i].x) == 0);
		EG_ASSERT(isinf(v[i].y) == 0);
	}
}

static void Kinematic3(ecs_iter_t *it)
{
	EgMassF32          *m = ecs_term(it, EgMassF32,          1); // [in]  This
	EgForce2F32        *f = ecs_term(it, EgForce2F32,        2); // [in]  This
	EgDrag2F32         *d = ecs_term(it, EgDrag2F32,         3); // [in]  This
	EgAcceleration2F32 *a = ecs_term(it, EgAcceleration2F32, 4); // [out] This
	for (int i = 0; i < it->count; i ++)
	{
		EG_ASSERT(eg_isnan(f[i].x) == 0);
		EG_ASSERT(eg_isnan(f[i].y) == 0);
		EG_ASSERT(isinf(f[i].x) == 0);
		EG_ASSERT(isinf(f[i].y) == 0);
		float fx = f[i].x - d[i].x;
		float fy = f[i].y - d[i].y;
		a[i].x = fx / m[i].value;
		a[i].y = fy / m[i].value;
		EG_ASSERT(eg_isnan(a[i].x) == 0);
		EG_ASSERT(eg_isnan(a[i].y) == 0);
		EG_ASSERT(isinf(a[i].x) == 0);
		EG_ASSERT(isinf(a[i].y) == 0);
	}
}

static void Kinematic4(ecs_iter_t *it)
{
	EgMassF32      *m = ecs_term(it, EgMassF32,      1); // [in]  This
	EgVelocity2F32 *v = ecs_term(it, EgVelocity2F32, 2); // [in]  This
	EgMomentum2F32 *p = ecs_term(it, EgMomentum2F32, 3); // [out] This
	for (int i = 0; i < it->count; i ++)
	{
		p[i].x = v[i].x * m[i].value;
		p[i].y = v[i].y * m[i].value;
	}
}

static void Kinematic5(ecs_iter_t *it)
{
	EgDensityF32   *p = ecs_term(it, EgDensityF32,   1); // [in]  Parent
	EgVelocity2F32 *v = ecs_term(it, EgVelocity2F32, 2); // [in]  This
	EgDrag2F32     *d = ecs_term(it, EgDrag2F32,     3); // [out] This
	for (int i = 0; i < it->count; i ++)
	{
		float vx = v[i].x;
		float vy = v[i].y;
		float mag = sqrtf(vx*vx + vy*vy);
		d[i].x = 0.5f * p[0].value * vx * mag;
		d[i].y = 0.5f * p[0].value * vy * mag;
	}
}


void FlecsComponentsEgQuantityImport(ecs_world_t *world)
{
	ECS_MODULE(world, FlecsComponentsEgQuantity);


	ECS_COMPONENT_DEFINE(world, EgPosition2F32);
	ECS_COMPONENT_DEFINE(world, EgPosition2I32);
	ECS_COMPONENT_DEFINE(world, EgVelocity2F32);
	ECS_COMPONENT_DEFINE(world, EgAcceleration2F32);
	ECS_COMPONENT_DEFINE(world, EgForce2F32);
	ECS_COMPONENT_DEFINE(world, EgDrag2F32);
	ECS_COMPONENT_DEFINE(world, EgMomentum2F32);
	ECS_COMPONENT_DEFINE(world, EgMassF32);
	ECS_COMPONENT_DEFINE(world, EgTimeF32);
	ECS_COMPONENT_DEFINE(world, EgTemperatureF32);
	ECS_COMPONENT_DEFINE(world, EgLengthF32);
	ECS_COMPONENT_DEFINE(world, EgDensityF32);

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

	ecs_doc_set_brief(world, ecs_id(EgVelocity2F32), "Moved distance per unit time: the first time derivative of position. unit: m/s");
	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgVelocity2F32),
	.members = {
	{ .name = "x", .type = ecs_id(ecs_f32_t) },
	{ .name = "y", .type = ecs_id(ecs_f32_t) }
	}
	});

	ecs_doc_set_brief(world, ecs_id(EgAcceleration2F32), "Rate of change of velocity per unit time: the second time derivative of position. unit: m/s2");
	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgAcceleration2F32),
	.members = {
	{ .name = "x", .type = ecs_id(ecs_f32_t) },
	{ .name = "y", .type = ecs_id(ecs_f32_t) }
	}
	});

	ecs_doc_set_brief(world, ecs_id(EgForce2F32), "Transfer of momentum per unit time. unit: newton (N = kg⋅m⋅s−2)");
	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgForce2F32),
	.members = {
	{ .name = "x", .type = ecs_id(ecs_f32_t) },
	{ .name = "y", .type = ecs_id(ecs_f32_t) }
	}
	});

	ecs_doc_set_brief(world, ecs_id(EgMomentum2F32), "Product of an object's mass and velocity. unit: kg*m/s");
	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgMomentum2F32),
	.members = {
	{ .name = "x", .type = ecs_id(ecs_f32_t) },
	{ .name = "y", .type = ecs_id(ecs_f32_t) }
	}
	});

	ecs_doc_set_brief(world, ecs_id(EgMassF32), "A measure of resistance to acceleration. unit: kilogram (kg)");
	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgMassF32),
	.members = {
	{ .name = "value", .type = ecs_id(ecs_f32_t) }
	}
	});

	ecs_doc_set_brief(world, ecs_id(EgTimeF32), "The duration of an event. unit: second (s)");
	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgTimeF32),
	.members = {
	{ .name = "value", .type = ecs_id(ecs_f32_t) }
	}
	});

	ecs_doc_set_brief(world, ecs_id(EgTemperatureF32), "Average kinetic energy per degree of freedom of a system. unit: kelvin (K)");
	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgTemperatureF32),
	.members = {
	{ .name = "value", .type = ecs_id(ecs_f32_t) }
	}
	});

	ecs_doc_set_brief(world, ecs_id(EgLengthF32), "The one-dimensional extent of an object. unit: metre (m)");
	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgLengthF32),
	.members = {
	{ .name = "value", .type = ecs_id(ecs_f32_t) }
	}
	});

	ecs_doc_set_brief(world, ecs_id(EgDensityF32), "Substance is its mass per unit volume");
	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgDensityF32),
	.members = {
	{ .name = "value", .type = ecs_id(ecs_f32_t) }
	}
	});

	ECS_SYSTEM(world, Kinematic1, EcsOnUpdate, [inout] EgPosition2F32, [in] EgVelocity2F32);
	ECS_SYSTEM(world, Kinematic2, EcsOnUpdate, [inout] EgVelocity2F32, [in] EgAcceleration2F32);
	ECS_SYSTEM(world, Kinematic3, EcsOnUpdate, [in] EgMassF32, [in] EgForce2F32, [in] EgDrag2F32, [out] EgAcceleration2F32);
	ECS_SYSTEM(world, Kinematic4, EcsOnUpdate, [in] EgMassF32, [in] EgVelocity2F32, [out] EgMomentum2F32);
	ECS_SYSTEM(world, Kinematic5, EcsOnUpdate, [in] EgDensityF32(parent), [in] EgVelocity2F32, [out] EgDrag2F32);


}
