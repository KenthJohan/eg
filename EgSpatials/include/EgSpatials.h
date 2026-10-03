#pragma once

#include <flecs.h>
#include "egmath.h"

typedef struct {
	float x;
	float y;
	float z;
	float w;
} V4f32;

typedef struct {
	float x;
	float y;
	float z;
} V3f32;

typedef struct {
	float x;
	float y;
} V2f32;

typedef struct {
	float x;
	float y;
} Position2;

typedef struct {
	float x;
	float y;
} Scale2;

typedef struct {
	float radians;
} Rotation2;

typedef struct {
	float x;
	float y;
	float z;
} Position3;

typedef struct {
	float r;
	float g;
	float b;
} Color3;

typedef struct {
	float x;
	float y;
	float z;
} Ray3;

typedef struct {
	float x;
	float y;
	float z;
} Scale3;

typedef struct {
	float x;
	float y;
} Velocity2;

typedef struct {
	float x;
	float y;
	float z;
} Velocity3;

typedef struct {
	m4f32 matrix;
} Transform4;

typedef struct {
	m3f32 matrix;
} Transform3;

typedef struct {
	m4f32 matrix;
} WorldTransform4;

typedef struct {
	m3f32 matrix;
} WorldTransform3;

typedef struct {
	float x;
	float y;
	float z;
	float w;
} Orientation;

typedef struct {
	float pitch;
	float yaw;
	float roll;
} EulerAngles;

typedef struct {
	float dx;
	float dy;
	float dz;
} Rotate3;

typedef struct {
	float frequency;
	float amplitude;
} Sinewave;

extern ECS_COMPONENT_DECLARE(V4f32);
extern ECS_COMPONENT_DECLARE(V3f32);
extern ECS_COMPONENT_DECLARE(V2f32);
extern ECS_COMPONENT_DECLARE(Position2);
extern ECS_COMPONENT_DECLARE(Scale2);
extern ECS_COMPONENT_DECLARE(Rotation2);
extern ECS_COMPONENT_DECLARE(Position3);
extern ECS_COMPONENT_DECLARE(Ray3);
extern ECS_COMPONENT_DECLARE(Scale3);
extern ECS_COMPONENT_DECLARE(Velocity2);
extern ECS_COMPONENT_DECLARE(Velocity3);
extern ECS_COMPONENT_DECLARE(Orientation);
extern ECS_COMPONENT_DECLARE(EulerAngles);
extern ECS_COMPONENT_DECLARE(Rotate3);
extern ECS_COMPONENT_DECLARE(Transform4);
extern ECS_COMPONENT_DECLARE(Transform3);
extern ECS_COMPONENT_DECLARE(WorldTransform4);
extern ECS_COMPONENT_DECLARE(WorldTransform3);
extern ECS_COMPONENT_DECLARE(Sinewave);
extern ECS_COMPONENT_DECLARE(Color3);

extern ECS_TAG_DECLARE(RotateOrder1);
extern ECS_TAG_DECLARE(RotateOrder2);
extern ECS_TAG_DECLARE(Normalized);

void EgSpatialsImport(ecs_world_t *world);
