#pragma once
#include <flecs.h>

#include "EgFs/EgFsEpoll.h"
#include "EgFs/EgFsFanotify.h"
#include "EgFs/EgFsInotify.h"
#include "EgFs/EgFsPath.h"

#define EGFS_FD_ENTITY_OFFSET 1024

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  char const *value;
  ecs_size_t length;
  uint64_t hash;
} EgFsPath;

typedef struct {
  int fd;
  ecs_entity_t file;
} EgFsWatch;

typedef struct {
  ecs_hashmap_t hm;
} EgFsLookup;

extern ECS_COMPONENT_DECLARE(EgFsPath);
extern ECS_COMPONENT_DECLARE(EgFsWatch);
extern ECS_COMPONENT_DECLARE(EgFsLookup);
extern ECS_ENTITY_DECLARE(EgFsFiles);
extern ECS_ENTITY_DECLARE(EgFsDescriptors);

void EgFsImport(ecs_world_t *world);

#ifdef __cplusplus
}
#endif