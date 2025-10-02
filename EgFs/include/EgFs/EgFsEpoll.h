#pragma once
#include <flecs.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  ecs_map_t map;
} EgFsEpollCreate;

extern ECS_COMPONENT_DECLARE(EgFsEpollCreate);

void EgFsEpollImport(ecs_world_t *world);

#ifdef __cplusplus
}
#endif