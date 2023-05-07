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
ECS_TAG_DECLARE(EgFsCwd);

ECS_TAG_DECLARE(EgFsOwner);
ECS_TAG_DECLARE(EgFsDomain);

ECS_TAG_DECLARE(EgFsType);
ECS_TAG_DECLARE(EgFsTypeDir);
ECS_TAG_DECLARE(EgFsTypeExe);
ECS_TAG_DECLARE(EgFsTypeC);

ECS_COMPONENT_DECLARE(EgFsMonitorDir);
ECS_COMPONENT_DECLARE(EgFsSize);

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
	
	ECS_TAG_DEFINE(world, EgFsCwd);
	
	ECS_TAG_DEFINE(world, EgFsOwner);
	ECS_TAG_DEFINE(world, EgFsDomain);

	ECS_ENTITY_DEFINE(world, EgFsType, EcsUnion);
	ECS_TAG_DEFINE(world, EgFsTypeDir);
	ECS_TAG_DEFINE(world, EgFsTypeExe);
	ECS_TAG_DEFINE(world, EgFsTypeC);
	
	ECS_COMPONENT_DEFINE(world, EgFsMonitorDir);
	ECS_COMPONENT_DEFINE(world, EgFsSize);

	ecs_struct(world, {
	.entity = ecs_id(EgFsMonitorDir),
	.members = {
	{ .name = "subtree", .type = ecs_id(ecs_bool_t) }
	}
	});

	ecs_struct(world, {
	.entity = ecs_id(EgFsSize),
	.members = {
	{ .name = "size", .type = ecs_id(ecs_i64_t), .unit = EcsBytes }
	}
	});

}


