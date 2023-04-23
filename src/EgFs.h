#pragma once
#include "flecs.h"

extern ECS_TAG_DECLARE(EgFsAdded);
extern ECS_TAG_DECLARE(EgFsModified);
extern ECS_TAG_DECLARE(EgFsPath);
extern ECS_TAG_DECLARE(EgFsMonitor);

void EgFsImport(ecs_world_t *world);



