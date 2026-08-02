#pragma once
#include <flecs.h>

#include "EgFs/EgFsEpoll.h"
#include "EgFs/EgFsFanotify.h"
#include "EgFs/EgFsInotify.h"
#include "EgFs/EgFsSocket.h"

#define EGFS_FD_ENTITY_OFFSET 1000

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	ecs_entity_t path1;
	ecs_entity_t prefab;
} EgFsWatch;

typedef struct {
	int32_t fd;
} EgFsFd;

typedef struct {
	int32_t dummy;
} EgFsReady;

typedef struct {
	void    *data;
	uint32_t size;
} EgFsContent;

extern ECS_COMPONENT_DECLARE(EgFsWatch);
extern ECS_COMPONENT_DECLARE(EgFsFd);
extern ECS_COMPONENT_DECLARE(EgFsReady);
extern ECS_COMPONENT_DECLARE(EgFsContent);
extern ECS_COMPONENT_DECLARE(EgFsContent);

extern ECS_TAG_DECLARE(EgFs);
extern ECS_TAG_DECLARE(EgFsDir);
extern ECS_TAG_DECLARE(EgFsFile);
extern ECS_TAG_DECLARE(EgFsRoot);
extern ECS_TAG_DECLARE(EgFsCwd);
extern ECS_TAG_DECLARE(EgFsSockets);
extern ECS_TAG_DECLARE(EgFsDescriptors);
extern ECS_TAG_DECLARE(EgFsEventOpen);
extern ECS_TAG_DECLARE(EgFsEventModify);
extern ECS_TAG_DECLARE(EgFsDump);

void EgFsImport(ecs_world_t *world);

ecs_entity_t EgFs_create_path_entity(ecs_world_t *world, char const *path);

#ifdef __cplusplus
}
#endif
