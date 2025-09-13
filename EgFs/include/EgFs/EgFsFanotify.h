#pragma once
#include <flecs.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int fd;
} EgFsFanotifyFd;

extern ECS_COMPONENT_DECLARE(EgFsFanotifyFd);

void EgFsFanotifyImport(ecs_world_t *world);

#ifdef __cplusplus
}
#endif