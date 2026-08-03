#pragma once
#include <box2d/box2d.h>
#include <flecs.h>


extern ECS_COMPONENT_DECLARE(b2WorldId);
extern ECS_COMPONENT_DECLARE(b2BodyId);
extern ECS_COMPONENT_DECLARE(b2ShapeId);
extern ECS_TAG_DECLARE(EgB2TargetTransform);

void EgPhysicsBox2dImport(ecs_world_t *world);
