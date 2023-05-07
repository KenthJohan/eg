#include "EgFs.h"
#include "EgStr.h"

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
ECS_TAG_DECLARE(EgFsTypeUnknown);
ECS_TAG_DECLARE(EgFsTypeDir);
ECS_TAG_DECLARE(EgFsTypeExe);
ECS_TAG_DECLARE(EgFsTypeLangC);

ECS_COMPONENT_DECLARE(EgFsMonitorDir);
ECS_COMPONENT_DECLARE(EgFsSize);




void System_File_Tagging(ecs_iter_t *it)
{
	EgText *path = ecs_field(it, EgText, 1);
	for (int i = 0; i < it->count; i ++)
	{
		if(path->value == NULL) {continue;}
		ecs_entity_t e = it->entities[i];
		char const * ext = strrchr(path[i].value, '.');
		if(ecs_os_strcmp(ext, ".exe") == 0)
		{
			ecs_add_pair(it->world, e, EgFsType, EgFsTypeExe);
		}
		else if(ecs_os_strcmp(ext, ".c") == 0)
		{
			ecs_add_pair(it->world, e, EgFsType, EgFsTypeLangC);
		}
		else
		{
			ecs_add_pair(it->world, e, EgFsType, EgFsTypeUnknown);
		}
	}
}










void EgFsImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgFs);
	ECS_IMPORT(world, EgStr);
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
	ECS_TAG_DEFINE(world, EgFsTypeUnknown);
	ECS_TAG_DEFINE(world, EgFsTypeDir);
	ECS_TAG_DEFINE(world, EgFsTypeExe);
	ECS_TAG_DEFINE(world, EgFsTypeLangC);
	
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

	/*
	ecs_observer(world, {
		.entity = ecs_entity(world, {
		.name = "System_File_Tagging",
		}),
		.filter.terms = {
		{.id = ecs_pair(ecs_id(EgText), EgFsPath) },
		{.id = EgFsFile },
		},
		.events = { EcsOnSet },
		.callback = System_File_Tagging
	});
	*/

	ecs_system(world, {
		.entity = ecs_entity(world, {
		.name = "System_File_Tagging",
		.add = { ecs_dependson(EcsOnUpdate) }
		}),
		.query.filter.terms = {
			{.id = ecs_pair(ecs_id(EgText), EgFsPath) },
			{.id = EgFsFile },
			{.id = ecs_pair(EgFsType, EcsWildcard), .oper=EcsNot },
		},
		.callback = System_File_Tagging
	});

}


