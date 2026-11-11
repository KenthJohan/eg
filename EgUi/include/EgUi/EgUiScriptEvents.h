#pragma once

#include <flecs.h>

// Entities with this tag can be the target of flecs script mouse events
// (on.press, on.drag, on.release, on.click, on.move, on.enter, on.leave).
// Script name: eg.ui.scriptevents.Target
extern ECS_TAG_DECLARE(EgUiScriptEventsTarget);

// Opt in to left-button capture on press. Drag and release continue to target
// this entity until release or cancellation, even when the pointer leaves it.
extern ECS_TAG_DECLARE(EgUiScriptEventsCapture);

// Return true when the provider handled the event, including a background hit.
// Return false when no picking target is available; no CPU fallback is used.
typedef bool (*EgUiScriptEventsPickFn)(ecs_world_t *world, ecs_entity_t *target,
	float *screen_x, float *screen_y, void *ctx);

// Optional platform hooks for capture outside the application window.
typedef bool (*EgUiScriptEventsCaptureBeginFn)(ecs_world_t *world, void *ctx);
typedef bool (*EgUiScriptEventsCaptureValidFn)(ecs_world_t *world, void *ctx);
typedef void (*EgUiScriptEventsCaptureEndFn)(ecs_world_t *world, void *ctx);

typedef struct {
	EgUiScriptEventsPickFn pick;
	void *ctx;
	EgUiScriptEventsCaptureBeginFn capture_begin;
	EgUiScriptEventsCaptureValidFn capture_valid;
	EgUiScriptEventsCaptureEndFn capture_end;
} EgUiScriptEventsPicker;

extern ECS_COMPONENT_DECLARE(EgUiScriptEventsPicker);

void EgUiScriptEventsImport(ecs_world_t *world);
