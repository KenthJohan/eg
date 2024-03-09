#pragma once
#include <flecs.h>



typedef struct
{
	// Enable subtree monitoring:
	ecs_bool_t subtree;
} EgFsMonitorDir;

typedef struct
{
	// Enable subtree monitoring:
	int64_t size;
} EgFsSize;

// Filesystem events for files being added, removed, modified:
extern ECS_TAG_DECLARE(EgFsAdded);
extern ECS_TAG_DECLARE(EgFsModified);
extern ECS_TAG_DECLARE(EgFsRemoved);
extern ECS_TAG_DECLARE(EgFsRenamedOld);
extern ECS_TAG_DECLARE(EgFsRenamedNew);

// A filesystem path
extern ECS_TAG_DECLARE(EgFsPath);
extern ECS_TAG_DECLARE(EgFsPathReal);

// Root instance of a filesystem monitor:
extern ECS_TAG_DECLARE(EgFsMonitor);

// List files and dirs
extern ECS_TAG_DECLARE(EgFsList);


extern ECS_TAG_DECLARE(EgFsDir);
extern ECS_TAG_DECLARE(EgFsFile);

// Current working directory:
extern ECS_TAG_DECLARE(EgFsCwd);

extern ECS_TAG_DECLARE(EgFsCreated);
extern ECS_TAG_DECLARE(EgFsModified);
extern ECS_TAG_DECLARE(EgFsAccessed);

extern ECS_TAG_DECLARE(EgFsOwner);
extern ECS_TAG_DECLARE(EgFsDomain);

// File type:
extern ECS_TAG_DECLARE(EgFsType);
extern ECS_TAG_DECLARE(EgFsTypeUnknown);
extern ECS_TAG_DECLARE(EgFsTypeDir);
extern ECS_TAG_DECLARE(EgFsTypeExe);
extern ECS_TAG_DECLARE(EgFsTypeLangC);


extern ECS_TAG_DECLARE(EgFsRoot);
extern ECS_TAG_DECLARE(EgFsLoad);
extern ECS_TAG_DECLARE(EgFsSave);



// Must be child of EgFsMonitor to be able to monitor a directory:
extern ECS_COMPONENT_DECLARE(EgFsMonitorDir);

extern ECS_COMPONENT_DECLARE(EgFsSize);






void EgFsImport(ecs_world_t *world);


