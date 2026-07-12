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

extern ECS_COMPONENT_DECLARE(EgCamerasState);

void EgCamerasImport(ecs_world_t *world);
