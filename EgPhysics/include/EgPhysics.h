#pragma once
#include <flecs.h>

typedef struct {
	float gravity_x;
	float gravity_y;
} EgPhysicsWorldDef;

typedef struct {
	ecs_i32_t type; // b2BodyType
} EgPhysicsBodyDef;

typedef struct {
	float half_width;
	float half_height;
	float density;
	float friction;
} EgPhysicsBox;

extern ECS_COMPONENT_DECLARE(EgPhysicsWorldDef);
extern ECS_COMPONENT_DECLARE(EgPhysicsBodyDef);
extern ECS_COMPONENT_DECLARE(EgPhysicsBox);

void EgPhysicsImport(ecs_world_t *world);
