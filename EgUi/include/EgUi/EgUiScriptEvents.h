#pragma once

#include <flecs.h>

// Entities with this tag can be the target of flecs script mouse events
// (on.press, on.drag, on.release, on.click, on.move, on.enter, on.leave).
// Script name: eg.ui.scriptevents.Target
extern ECS_TAG_DECLARE(EgUiScriptEventsTarget);

// Return true when the provider handled the event, including a background hit.
// Return false when no picking target is available; no CPU fallback is used.
typedef bool (*EgUiScriptEventsPickFn)(ecs_world_t *world, ecs_entity_t *target,
	float *screen_x, float *screen_y, void *ctx);

typedef struct {
	EgUiScriptEventsPickFn pick;
	void *ctx;
} EgUiScriptEventsPicker;

extern ECS_COMPONENT_DECLARE(EgUiScriptEventsPicker);

void EgUiScriptEventsImport(ecs_world_t *world);
