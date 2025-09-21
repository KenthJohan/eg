#pragma once
#include <flecs.h>

typedef struct {
	char const *value;
	ecs_size_t length;
	uint64_t hash;
} EgFsPathsHashed;

typedef struct {
	ecs_hashmap_t hm;
} EgFsPathsLookup;

extern ECS_COMPONENT_DECLARE(EgFsPathsHashed);
extern ECS_COMPONENT_DECLARE(EgFsPathsLookup);

void EgFsPathsImport(ecs_world_t *world);

ecs_entity_t EgFs_path_new(ecs_world_t *world, const char *path);
