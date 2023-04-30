#include "EgFs.h"
#include "EgQuantities.h"





ECS_TAG_DECLARE(EgFsAdded);
ECS_TAG_DECLARE(EgFsModified);
ECS_TAG_DECLARE(EgFsRemoved);
ECS_TAG_DECLARE(EgFsRenamedOld);
ECS_TAG_DECLARE(EgFsRenamedNew);

ECS_TAG_DECLARE(EgFsPath);
ECS_TAG_DECLARE(EgFsMonitorInstance);
ECS_TAG_DECLARE(EgFsMonitorDir);







void EgFsImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgFs);
	ecs_set_name_prefix(world, "EgFs");
	ECS_IMPORT(world, EgQuantities);
	ECS_TAG_DEFINE(world, EgFsAdded);
	ECS_TAG_DEFINE(world, EgFsModified);
	ECS_TAG_DEFINE(world, EgFsRemoved);
	ECS_TAG_DEFINE(world, EgFsRenamedOld);
	ECS_TAG_DEFINE(world, EgFsRenamedNew);
	ECS_TAG_DEFINE(world, EgFsPath);
	ECS_TAG_DEFINE(world, EgFsMonitorInstance);
	ECS_TAG_DEFINE(world, EgFsMonitorDir);

}


