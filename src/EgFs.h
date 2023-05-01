#pragma once
#include "flecs.h"



typedef struct
{
	ecs_bool_t subtree;
} EgFsMonitorDir;


// Filesystem events for files being added, removed, modified:
extern ECS_TAG_DECLARE(EgFsAdded);
extern ECS_TAG_DECLARE(EgFsModified);
extern ECS_TAG_DECLARE(EgFsRemoved);
extern ECS_TAG_DECLARE(EgFsRenamedOld);
extern ECS_TAG_DECLARE(EgFsRenamedNew);

// A filesystem path
extern ECS_TAG_DECLARE(EgFsPath);

// Root instance of a filesystem monitor:
extern ECS_TAG_DECLARE(EgFsMonitor);

// List files and dirs
extern ECS_TAG_DECLARE(EgFsList);


extern ECS_TAG_DECLARE(EgFsDir);
extern ECS_TAG_DECLARE(EgFsFile);

// Must be child of EgFsMonitor to be able to monitor a directory:
extern ECS_COMPONENT_DECLARE(EgFsMonitorDir);





void EgFsImport(ecs_world_t *world);



