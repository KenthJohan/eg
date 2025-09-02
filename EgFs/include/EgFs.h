#pragma once
#include <flecs.h>


#define EGFS_FD_ENTITY_OFFSET 1024

typedef struct
{
	char const *value;
	ecs_size_t length;
	uint64_t hash;
} EgFsPath;

typedef struct
{
	int fd;
	ecs_map_t map;
} EgFsEpoll;

typedef struct
{
	int fd;
} EgFsFanotify;

typedef struct
{
	int fd;
} EgFsInotify;

typedef struct
{
	int fd;
	ecs_entity_t file;
} EgFsWatch;

typedef struct
{
	ecs_hashmap_t hm;
} EgFsLookup;



extern ECS_COMPONENT_DECLARE(EgFsPath);
extern ECS_COMPONENT_DECLARE(EgFsEpoll);
extern ECS_COMPONENT_DECLARE(EgFsFanotify);
extern ECS_COMPONENT_DECLARE(EgFsInotify);
extern ECS_COMPONENT_DECLARE(EgFsWatch);
extern ECS_COMPONENT_DECLARE(EgFsLookup);
extern ECS_ENTITY_DECLARE(EgFsFiles);
extern ECS_ENTITY_DECLARE(EgFsDescriptors);

void EgFsImport(ecs_world_t *world);
