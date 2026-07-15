#pragma once
#include <flecs.h>
#include <egmath.h>

typedef struct
{
	float fov;
	m4f32 view;
	m4f32 projection;
	m4f32 vp; // View-Projection Matrix
} EgCamerasState;

typedef struct
{
	ecs_i32_t dummy;
} EgCamerasUnproject;

extern ECS_COMPONENT_DECLARE(EgCamerasState);
extern ECS_COMPONENT_DECLARE(EgCamerasUnproject);

void EgCamerasImport(ecs_world_t *world);
