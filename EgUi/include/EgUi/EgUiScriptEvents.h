#pragma once

#include <flecs.h>

// Entities with this tag can be the target of flecs script mouse events
// (on.press, on.drag, on.release, on.click, on.move, on.enter, on.leave).
// Script name: eg.ui.scriptevents.Target
extern ECS_TAG_DECLARE(EgUiScriptEventsTarget);

void EgUiScriptEventsImport(ecs_world_t *world);
