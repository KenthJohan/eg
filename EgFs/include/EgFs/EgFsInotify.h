#pragma once
#include <flecs.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int fd;
  ecs_map_t map;
} EgFsInotifyFd;

extern ECS_COMPONENT_DECLARE(EgFsInotifyFd);

void EgFsInotifyImport(ecs_world_t *world);

#ifdef __cplusplus
}
#endif