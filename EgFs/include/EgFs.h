#pragma once
#include <flecs.h>

#include "EgFs/EgFsEpoll.h"
#include "EgFs/EgFsFanotify.h"
#include "EgFs/EgFsInotify.h"

#define EGFS_FD_ENTITY_OFFSET 1024

#ifdef __cplusplus
extern "C" {
#endif



typedef struct {
  int fd;
  ecs_entity_t file;
} EgFsWatch;

typedef struct {
  int fd;
} EgFsFd;

typedef struct {
  int dummy;
} EgFsReady;


extern ECS_COMPONENT_DECLARE(EgFsWatch);
extern ECS_COMPONENT_DECLARE(EgFsFd);
extern ECS_COMPONENT_DECLARE(EgFsReady);

extern ECS_ENTITY_DECLARE(EgFsFiles);
extern ECS_ENTITY_DECLARE(EgFsDescriptors);
extern ECS_ENTITY_DECLARE(EgFsEventOpen);
extern ECS_ENTITY_DECLARE(EgFsEventModify);

void EgFsImport(ecs_world_t *world);



#ifdef __cplusplus
}
#endif