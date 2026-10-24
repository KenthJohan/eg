#pragma once

#include <flecs.h>
#include <EgSpatials.h>
#include <EgShapes.h>

#include "EgUi/EgUiLocalMouse.h"
#include "EgUi/EgUiButtons.h"
#include "EgUi/EgUiFlows.h"
#include "EgUi/EgUiBounds.h"
#include "EgUi/EgUiFreeforms.h"
#include "EgUi/EgUiScriptEvents.h"

// Relationship to the entity that provides the shared mouse Position2.
extern ECS_TAG_DECLARE(EgUiMouse);
extern ECS_COMPONENT_DECLARE(EgUiAt);
extern ECS_COMPONENT_DECLARE(EgUiDir);

void EgUiImport(ecs_world_t *world);
