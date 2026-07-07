#include "EgSpatials.h"

ECS_COMPONENT_DECLARE(Position2);
ECS_COMPONENT_DECLARE(Position3);
ECS_COMPONENT_DECLARE(V4f32);
ECS_COMPONENT_DECLARE(Ray3);
ECS_COMPONENT_DECLARE(Scale3);
ECS_COMPONENT_DECLARE(Position3World);
ECS_COMPONENT_DECLARE(Velocity2);
ECS_COMPONENT_DECLARE(Velocity3);
ECS_COMPONENT_DECLARE(Rotate3);
ECS_COMPONENT_DECLARE(RotMat3);
ECS_COMPONENT_DECLARE(Orientation);
ECS_COMPONENT_DECLARE(OrientationWorld);
ECS_COMPONENT_DECLARE(EulerAngles);
ECS_COMPONENT_DECLARE(Transformation);
ECS_COMPONENT_DECLARE(TransformationCollector);
ECS_COMPONENT_DECLARE(Sinewave);
ECS_COMPONENT_DECLARE(Color3);
ECS_TAG_DECLARE(EgRotateOrder1);
ECS_TAG_DECLARE(EgRotateOrder2);
ECS_TAG_DECLARE(EgPositionWorldNoReset);



ECS_CTOR(TransformationCollector, ptr, {
	ecs_trace("TransformationCollector::Ctor");
	ecs_os_memset_t(ptr, 0, TransformationCollector);
})

ECS_DTOR(TransformationCollector, ptr, {
	ecs_trace("TransformationCollector::Dtor");
	ecs_os_free(ptr->data);
})

ECS_MOVE(TransformationCollector, dst, src, {
	ecs_trace("TransformationCollector::Move");
	ecs_os_free(dst->data);
	dst->data = src->data;
	src->data = NULL;
})

ECS_COPY(TransformationCollector, dst, src, {
	ecs_trace("TransformationCollector::Copy");
	ecs_os_free(dst->data);
	dst->data = ecs_os_memdup_n(src->data, m4f32, src->count);
})

ECS_CTOR(Transformation, ptr, {
	ptr->matrix = (m4f32)M4_IDENTITY;
})

ECS_CTOR(RotMat3, ptr, {
	ptr->x1 = 1.0f;
	ptr->y1 = 0.0f;
	ptr->z1 = 0.0f;
	ptr->x2 = 0.0f;
	ptr->y2 = 1.0f;
	ptr->z2 = 0.0f;
	ptr->x3 = 0.0f;
	ptr->y3 = 0.0f;
	ptr->z3 = 1.0f;
})

ECS_CTOR(Orientation, ptr, {
	// QF32_IDENTITY;
	// printf("Orientation::ECS_CTOR\n");
	ptr->x = 0.0f;
	ptr->y = 0.0f;
	ptr->z = 0.0f;
	ptr->w = 1.0f;
})

ECS_CTOR(OrientationWorld, ptr, {
	// QF32_IDENTITY;
	// printf("Orientation::ECS_CTOR\n");
	ptr->x = 0.0f;
	ptr->y = 0.0f;
	ptr->z = 0.0f;
	ptr->w = 1.0f;
})

ECS_CTOR(Scale3, ptr, {
	ptr->x = 1.0f;
	ptr->y = 1.0f;
	ptr->z = 1.0f;
})

void EgSpatialsImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgSpatials);
	ecs_set_name_prefix(world, "EgSpatials");

	ECS_COMPONENT_DEFINE(world, Position2);
	ECS_COMPONENT_DEFINE(world, Position3);
	ECS_COMPONENT_DEFINE(world, V4f32);
	ECS_COMPONENT_DEFINE(world, Ray3);
	ECS_COMPONENT_DEFINE(world, Scale3);
	ECS_COMPONENT_DEFINE(world, Position3World);
	ECS_COMPONENT_DEFINE(world, Velocity2);
	ECS_COMPONENT_DEFINE(world, Velocity3);
	ECS_COMPONENT_DEFINE(world, Orientation);
	ECS_COMPONENT_DEFINE(world, OrientationWorld);
	ECS_COMPONENT_DEFINE(world, EulerAngles);
	ECS_COMPONENT_DEFINE(world, Rotate3);
	ECS_COMPONENT_DEFINE(world, Transformation);
	ECS_COMPONENT_DEFINE(world, TransformationCollector);
	ECS_COMPONENT_DEFINE(world, RotMat3);
	ECS_COMPONENT_DEFINE(world, Sinewave);
	ECS_COMPONENT_DEFINE(world, Color3);

	ECS_TAG_DEFINE(world, EgRotateOrder1);
	ECS_TAG_DEFINE(world, EgRotateOrder2);
	ECS_TAG_DEFINE(world, EgPositionWorldNoReset);

	ecs_set_hooks(world, Orientation, {.ctor = ecs_ctor(Orientation)});
	ecs_set_hooks(world, OrientationWorld, {.ctor = ecs_ctor(OrientationWorld)});
	ecs_set_hooks(world, Transformation, {.ctor = ecs_ctor(Transformation)});
	ecs_set_hooks(world, RotMat3, {.ctor = ecs_ctor(RotMat3)});
	ecs_set_hooks(world, Scale3, {.ctor = ecs_ctor(Scale3)});
	ecs_set_hooks(world, TransformationCollector,
	{
	.ctor = ecs_ctor(TransformationCollector),
	.move = ecs_move(TransformationCollector),
	.copy = ecs_copy(TransformationCollector),
	.dtor = ecs_dtor(TransformationCollector),
	});

	ecs_struct(world,
	{.entity = ecs_id(Position2),
	.members = {
	{.name = "x", .type = ecs_id(ecs_f32_t)},
	{.name = "y", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(Position3),
	.members = {
	{.name = "x", .type = ecs_id(ecs_f32_t)},
	{.name = "y", .type = ecs_id(ecs_f32_t)},
	{.name = "z", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(Color3),
	.members = {
	{.name = "r", .type = ecs_id(ecs_f32_t)},
	{.name = "g", .type = ecs_id(ecs_f32_t)},
	{.name = "b", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(Ray3),
	.members = {
	{.name = "x", .type = ecs_id(ecs_f32_t)},
	{.name = "y", .type = ecs_id(ecs_f32_t)},
	{.name = "z", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(Scale3),
	.members = {
	{.name = "x", .type = ecs_id(ecs_f32_t)},
	{.name = "y", .type = ecs_id(ecs_f32_t)},
	{.name = "z", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(Position3World),
	.members = {
	{.name = "x", .type = ecs_id(ecs_f32_t)},
	{.name = "y", .type = ecs_id(ecs_f32_t)},
	{.name = "z", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(Velocity2),
	.members = {
	{.name = "x", .type = ecs_id(ecs_f32_t)},
	{.name = "y", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(Velocity3),
	.members = {
	{.name = "x", .type = ecs_id(ecs_f32_t)},
	{.name = "y", .type = ecs_id(ecs_f32_t)},
	{.name = "z", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(Orientation),
	.members = {
	{.name = "x", .type = ecs_id(ecs_f32_t)},
	{.name = "y", .type = ecs_id(ecs_f32_t)},
	{.name = "z", .type = ecs_id(ecs_f32_t)},
	{.name = "w", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(OrientationWorld),
	.members = {
	{.name = "x", .type = ecs_id(ecs_f32_t)},
	{.name = "y", .type = ecs_id(ecs_f32_t)},
	{.name = "z", .type = ecs_id(ecs_f32_t)},
	{.name = "w", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EulerAngles),
	.members = {
	{.name = "pitch", .type = ecs_id(ecs_f32_t)},
	{.name = "yaw", .type = ecs_id(ecs_f32_t)},
	{.name = "roll", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(Rotate3),
	.members = {
	{.name = "dx", .type = ecs_id(ecs_f32_t)},
	{.name = "dy", .type = ecs_id(ecs_f32_t)},
	{.name = "dz", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(V4f32),
	.members = {
	{.name = "x", .type = ecs_id(ecs_f32_t)},
	{.name = "y", .type = ecs_id(ecs_f32_t)},
	{.name = "z", .type = ecs_id(ecs_f32_t)},
	{.name = "w", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(Transformation),
	.members = {
	//{.name = "matrix", .type = ecs_id(ecs_f32_t), .count = 16},
	{.name = "c0", .type = ecs_id(V4f32)},
	{.name = "c1", .type = ecs_id(V4f32)},
	{.name = "c2", .type = ecs_id(V4f32)},
	{.name = "c3", .type = ecs_id(V4f32)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(TransformationCollector),
	.members = {
	{.name = "data", .type = ecs_id(ecs_uptr_t)},
	{.name = "count", .type = ecs_id(ecs_i32_t)},
	{.name = "cap", .type = ecs_id(ecs_i32_t)},
	{.name = "total", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(RotMat3),
	.members = {
	{.name = "x1", .type = ecs_id(ecs_f32_t)},
	{.name = "y1", .type = ecs_id(ecs_f32_t)},
	{.name = "z1", .type = ecs_id(ecs_f32_t)},
	{.name = "x2", .type = ecs_id(ecs_f32_t)},
	{.name = "y2", .type = ecs_id(ecs_f32_t)},
	{.name = "z2", .type = ecs_id(ecs_f32_t)},
	{.name = "x3", .type = ecs_id(ecs_f32_t)},
	{.name = "y3", .type = ecs_id(ecs_f32_t)},
	{.name = "z3", .type = ecs_id(ecs_f32_t)}}});

	ecs_struct(world,
	{.entity = ecs_id(Sinewave),
	.members = {
	{.name = "frequency", .type = ecs_id(ecs_f32_t)},
	{.name = "amplitude", .type = ecs_id(ecs_f32_t)},
	}});
}
