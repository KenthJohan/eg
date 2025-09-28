#pragma once
#include <flecs.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  char * addr;
  int port;
} EgFsSocketInfo;

typedef struct {
  ecs_entity_t s;
} EgFsSocketUdp;

extern ECS_COMPONENT_DECLARE(EgFsSocketUdp);
extern ECS_COMPONENT_DECLARE(EgFsSocketInfo);

void EgFsSocketImport(ecs_world_t *world);

#ifdef __cplusplus
}
#endif