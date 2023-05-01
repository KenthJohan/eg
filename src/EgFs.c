#include "EgFs.h"

ECS_TAG_DECLARE(EgFsAdded);
ECS_TAG_DECLARE(EgFsModified);
ECS_TAG_DECLARE(EgFsRemoved);
ECS_TAG_DECLARE(EgFsRenamedOld);
ECS_TAG_DECLARE(EgFsRenamedNew);
ECS_TAG_DECLARE(EgFsPath);
ECS_TAG_DECLARE(EgFsMonitor);
ECS_TAG_DECLARE(EgFsList);
ECS_TAG_DECLARE(EgFsDir);
ECS_TAG_DECLARE(EgFsFile);
ECS_COMPONENT_DECLARE(EgFsMonitorDir);


void EgFsImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgFs);
	ecs_set_name_prefix(world, "EgFs");

	ECS_TAG_DEFINE(world, EgFsAdded);
	ECS_TAG_DEFINE(world, EgFsModified);
	ECS_TAG_DEFINE(world, EgFsRemoved);
	ECS_TAG_DEFINE(world, EgFsRenamedOld);
	ECS_TAG_DEFINE(world, EgFsRenamedNew);

	ECS_TAG_DEFINE(world, EgFsPath);

	ECS_TAG_DEFINE(world, EgFsMonitor);

	ECS_TAG_DEFINE(world, EgFsList);

	ECS_TAG_DEFINE(world, EgFsDir);
	ECS_TAG_DEFINE(world, EgFsFile);
	
	ECS_COMPONENT_DEFINE(world, EgFsMonitorDir);

	ecs_struct(world, {
	.entity = ecs_id(EgFsMonitorDir),
	.members = {
	{ .name = "subtree", .type = ecs_id(ecs_bool_t) }
	}
	});

}


