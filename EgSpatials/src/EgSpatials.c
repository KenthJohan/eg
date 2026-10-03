#include "EgSpatials.h"

ECS_COMPONENT_DECLARE(Position2);
ECS_COMPONENT_DECLARE(Scale2);
ECS_COMPONENT_DECLARE(Rotation2);
ECS_COMPONENT_DECLARE(Position3);
ECS_COMPONENT_DECLARE(V4f32);
ECS_COMPONENT_DECLARE(V3f32);
ECS_COMPONENT_DECLARE(V2f32);
ECS_COMPONENT_DECLARE(Ray3);
ECS_COMPONENT_DECLARE(Scale3);
ECS_COMPONENT_DECLARE(Velocity2);
ECS_COMPONENT_DECLARE(Velocity3);
ECS_COMPONENT_DECLARE(Rotate3);
ECS_COMPONENT_DECLARE(RotMat3);
ECS_COMPONENT_DECLARE(Orientation);
ECS_COMPONENT_DECLARE(EulerAngles);
ECS_COMPONENT_DECLARE(Transform4);
ECS_COMPONENT_DECLARE(Transform3);
ECS_COMPONENT_DECLARE(WorldTransform4);
ECS_COMPONENT_DECLARE(WorldTransform3);
ECS_COMPONENT_DECLARE(Sinewave);
ECS_COMPONENT_DECLARE(Color3);
ECS_TAG_DECLARE(RotateOrder1);
ECS_TAG_DECLARE(RotateOrder2);
ECS_TAG_DECLARE(Normalized);

ECS_CTOR(Transform4, ptr, {
	ptr->matrix = (m4f32)M4_IDENTITY;
})

ECS_CTOR(Transform3, ptr, {
	ptr->matrix = (m3f32)M3_IDENTITY;
})

ECS_CTOR(WorldTransform4, ptr, {
	ptr->matrix = (m4f32)M4_IDENTITY;
})

ECS_CTOR(WorldTransform3, ptr, {
	ptr->matrix = (m3f32)M3_IDENTITY;
})

ECS_CTOR(Orientation, ptr, {
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

ECS_CTOR(Scale2, ptr, {
	ptr->x = 1.0f;
	ptr->y = 1.0f;
})

void EgSpatialsImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgSpatials);
	ecs_set_name_prefix(world, "EgSpatials");

	ECS_COMPONENT_DEFINE(world, Position2);
	ECS_COMPONENT_DEFINE(world, Scale2);
	ECS_COMPONENT_DEFINE(world, Rotation2);
	ECS_COMPONENT_DEFINE(world, Position3);
	ECS_COMPONENT_DEFINE(world, V4f32);
	ECS_COMPONENT_DEFINE(world, V3f32);
	ECS_COMPONENT_DEFINE(world, V2f32);
	ECS_COMPONENT_DEFINE(world, Ray3);
	ECS_COMPONENT_DEFINE(world, Scale3);
	ECS_COMPONENT_DEFINE(world, Velocity2);
	ECS_COMPONENT_DEFINE(world, Velocity3);
	ECS_COMPONENT_DEFINE(world, Orientation);
	ECS_COMPONENT_DEFINE(world, EulerAngles);
	ECS_COMPONENT_DEFINE(world, Rotate3);
	ECS_COMPONENT_DEFINE(world, Transform4);
	ECS_COMPONENT_DEFINE(world, Transform3);
	ECS_COMPONENT_DEFINE(world, WorldTransform4);
	ECS_COMPONENT_DEFINE(world, WorldTransform3);
	ECS_COMPONENT_DEFINE(world, Sinewave);
	ECS_COMPONENT_DEFINE(world, Color3);

	ECS_TAG_DEFINE(world, RotateOrder1);
	ECS_TAG_DEFINE(world, RotateOrder2);
	ECS_TAG_DEFINE(world, Normalized);

	ecs_set_hooks(world, Orientation, {.ctor = ecs_ctor(Orientation)});
	ecs_set_hooks(world, Transform4, {.ctor = ecs_ctor(Transform4)});
	ecs_set_hooks(world, Transform3, {.ctor = ecs_ctor(Transform3)});
	ecs_set_hooks(world, WorldTransform4, {.ctor = ecs_ctor(WorldTransform4)});
	ecs_set_hooks(world, WorldTransform3, {.ctor = ecs_ctor(WorldTransform3)});
	ecs_set_hooks(world, Scale3, {.ctor = ecs_ctor(Scale3)});
	ecs_set_hooks(world, Scale2, {.ctor = ecs_ctor(Scale2)});

	ecs_struct(world,
	{.entity = ecs_id(Position2),
	.members = {
	{.name = "x", .type = ecs_id(ecs_f32_t)},
	{.name = "y", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(Scale2),
	.members = {
	{.name = "x", .type = ecs_id(ecs_f32_t)},
	{.name = "y", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(Rotation2),
	.members = {
	{.name = "radians", .type = ecs_id(ecs_f32_t)},
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
	{.entity = ecs_id(V3f32),
	.members = {
	{.name = "x", .type = ecs_id(ecs_f32_t)},
	{.name = "y", .type = ecs_id(ecs_f32_t)},
	{.name = "z", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(V2f32),
	.members = {
	{.name = "x", .type = ecs_id(ecs_f32_t)},
	{.name = "y", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(Transform4),
	.members = {
	//{.name = "matrix", .type = ecs_id(ecs_f32_t), .count = 16},
	{.name = "c0", .type = ecs_id(V4f32)},
	{.name = "c1", .type = ecs_id(V4f32)},
	{.name = "c2", .type = ecs_id(V4f32)},
	{.name = "c3", .type = ecs_id(V4f32)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(Transform3),
	.members = {
	{.name = "c0", .type = ecs_id(V3f32)},
	{.name = "c1", .type = ecs_id(V3f32)},
	{.name = "c2", .type = ecs_id(V3f32)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(WorldTransform4),
	.members = {
	{.name = "c0", .type = ecs_id(V4f32)},
	{.name = "c1", .type = ecs_id(V4f32)},
	{.name = "c2", .type = ecs_id(V4f32)},
	{.name = "c3", .type = ecs_id(V4f32)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(WorldTransform3),
	.members = {
	{.name = "c0", .type = ecs_id(V3f32)},
	{.name = "c1", .type = ecs_id(V3f32)},
	{.name = "c2", .type = ecs_id(V3f32)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(Sinewave),
	.members = {
	{.name = "frequency", .type = ecs_id(ecs_f32_t)},
	{.name = "amplitude", .type = ecs_id(ecs_f32_t)},
	}});
}
