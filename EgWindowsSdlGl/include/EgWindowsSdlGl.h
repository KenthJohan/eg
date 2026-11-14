#pragma once
#include <flecs.h>

// Optional window component, invoked after swap or after a context/swap failure.
typedef struct {
	void (*complete)(ecs_world_t *world, ecs_entity_t window, bool displayed);
} EgWindowsSdlGlFrameCallback;

extern ECS_COMPONENT_DECLARE(EgWindowsSdlGlFrameCallback);

bool EgWindowsSdlGlCleanup(void *window, void *context, void (*destroy)(void *), void *resource);
void EgWindowsSdlGlImport(ecs_world_t *world);
