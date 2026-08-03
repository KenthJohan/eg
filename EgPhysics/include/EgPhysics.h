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
	float density;
	float friction;
} EgPhysicsShapesDef;

typedef struct {
	float linear_hertz;
	float linear_damping;
} EgPhysicsJointDef;

typedef struct {
	ecs_entity_t body_a;
	ecs_entity_t body_b_filter;
} EgPhysicsJointApplyRule;

typedef struct
{
	ecs_entity_t tag; ///< Adds this tag to the entity when an overlap is detected and removes it when no overlap is detected
} EgPhysicsOverlapChecking;

extern ECS_COMPONENT_DECLARE(EgPhysicsWorldDef);
extern ECS_COMPONENT_DECLARE(EgPhysicsBodyDef);
extern ECS_COMPONENT_DECLARE(EgPhysicsShapesDef);
extern ECS_COMPONENT_DECLARE(EgPhysicsJointDef);
extern ECS_COMPONENT_DECLARE(EgPhysicsJointApplyRule);
extern ECS_COMPONENT_DECLARE(EgPhysicsOverlapChecking);
extern ECS_TAG_DECLARE(EgPhysicsTargetTransform);

void EgPhysicsImport(ecs_world_t *world);
