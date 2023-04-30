#pragma once
#include "flecs.h"



typedef struct
{
	ecs_bool_t subtree;
} EgFsMonitorDir;


extern ECS_TAG_DECLARE(EgFsAdded);
extern ECS_TAG_DECLARE(EgFsModified);
extern ECS_TAG_DECLARE(EgFsRemoved);
extern ECS_TAG_DECLARE(EgFsRenamedOld);
extern ECS_TAG_DECLARE(EgFsRenamedNew);
extern ECS_TAG_DECLARE(EgFsPath);
extern ECS_TAG_DECLARE(EgFsMonitor);
extern ECS_COMPONENT_DECLARE(EgFsMonitorDir);





void EgFsImport(ecs_world_t *world);



