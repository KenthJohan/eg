#pragma once
#include <flecs.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	ecs_entity_t path1;
} EgFsSocketCreate;

extern ECS_COMPONENT_DECLARE(EgFsSocketCreate);

void EgFsSocketImport(ecs_world_t *world);

#ifdef __cplusplus
}
#endif