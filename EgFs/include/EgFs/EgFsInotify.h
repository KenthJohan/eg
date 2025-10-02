#pragma once
#include <flecs.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  ecs_map_t map;
} EgFsInotifyCreate;

extern ECS_COMPONENT_DECLARE(EgFsInotifyCreate);

void EgFsInotifyImport(ecs_world_t *world);

#ifdef __cplusplus
}
#endif