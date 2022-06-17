#pragma once
#include "flecs.h"



typedef struct
{
	ecs_i32_t dx[2];
	ecs_i32_t dy[2];
	ecs_i32_t dz[2];
	ecs_i32_t pitch[2];
	ecs_i32_t yaw[2];
	ecs_i32_t roll[2];
} EgCamera3DKeyBindings;

typedef struct
{
	ecs_f32_t move[3];
	ecs_f32_t look[3];
} EgCamera3D;

typedef struct
{
	ecs_i32_t forward;
	ecs_i32_t backward;
	ecs_i32_t left;
	ecs_i32_t right;
} EgCamera2D;

typedef struct
{
	ecs_string_t a;
	ecs_bool_t enabled_depth;
	ecs_bool_t enabled_facecull;
	ecs_query_t * query;
} EgScene;



extern ECS_COMPONENT_DECLARE(EgCamera3DKeyBindings);
extern ECS_COMPONENT_DECLARE(EgCamera3D);
extern ECS_COMPONENT_DECLARE(EgCamera2D);
extern ECS_COMPONENT_DECLARE(EgScene);


void EgCameraImport(ecs_world_t *world);
