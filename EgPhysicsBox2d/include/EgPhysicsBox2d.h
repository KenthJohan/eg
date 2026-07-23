#pragma once
#include <box2d/box2d.h>
#include <flecs.h>

typedef struct {
	ecs_entity_t tag; ///< Adds this tag to the entity when an overlap is detected and removes it when no overlap is detected
} EgPhysicsBox2dOverlapChecking;

extern ECS_COMPONENT_DECLARE(b2WorldId);
extern ECS_COMPONENT_DECLARE(b2BodyId);
extern ECS_COMPONENT_DECLARE(b2ShapeId);
extern ECS_COMPONENT_DECLARE(EgPhysicsBox2dOverlapChecking);
extern ECS_TAG_DECLARE(EgB2TargetTransform);

void EgPhysicsBox2dImport(ecs_world_t *world);
