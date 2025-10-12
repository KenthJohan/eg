#pragma once
#include <flecs.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	ecs_map_t map;
} EgFsEpollCreate;

typedef struct {
	int32_t events; // Bitmask of events that occurred
} EgFsEpollEvent;

extern ECS_COMPONENT_DECLARE(EgFsEpollCreate);

void EgFsEpollImport(ecs_world_t *world);

#ifdef __cplusplus
}
#endif