#define _GNU_SOURCE


#include "egfs.h"
#include <egstr.h>
#include <stdlib.h>
#include <stdio.h>


#include "eg_fs.h"

ECS_TAG_DECLARE(EgFsAdded);
ECS_TAG_DECLARE(EgFsModified);
ECS_TAG_DECLARE(EgFsRemoved);
ECS_TAG_DECLARE(EgFsRenamedOld);
ECS_TAG_DECLARE(EgFsRenamedNew);
ECS_TAG_DECLARE(EgFsPath);
ECS_TAG_DECLARE(EgFsPathReal);
ECS_TAG_DECLARE(EgFsMonitor);
ECS_TAG_DECLARE(EgFsList);
ECS_TAG_DECLARE(EgFsDir);
ECS_TAG_DECLARE(EgFsFile);
ECS_TAG_DECLARE(EgFsCwd);
ECS_TAG_DECLARE(EgFsCreated);
ECS_TAG_DECLARE(EgFsModified);
ECS_TAG_DECLARE(EgFsAccessed);

ECS_TAG_DECLARE(EgFsOwner);
ECS_TAG_DECLARE(EgFsDomain);

ECS_TAG_DECLARE(EgFsType);
ECS_TAG_DECLARE(EgFsTypeUnknown);
ECS_TAG_DECLARE(EgFsTypeDir);
ECS_TAG_DECLARE(EgFsTypeExe);
ECS_TAG_DECLARE(EgFsTypeLangC);

ECS_TAG_DECLARE(EgFsRoot);
ECS_TAG_DECLARE(EgFsLoad);
ECS_TAG_DECLARE(EgFsSave);

ECS_COMPONENT_DECLARE(EgFsMonitorDir);
ECS_COMPONENT_DECLARE(EgFsSize);

static void System_File_Tagging(ecs_iter_t *it)
{
	EgStrText *path = ecs_field(it, EgStrText, 1);
	for (int i = 0; i < it->count; i++) {
		if (path->value == NULL) {
			continue;
		}
		ecs_entity_t e = it->entities[i];
		char const *ext = strrchr(path[i].value, '.');
		if (ecs_os_strcmp(ext, ".exe") == 0) {
			ecs_add_pair(it->world, e, EgFsType, EgFsTypeExe);
		} else if (ecs_os_strcmp(ext, ".c") == 0) {
			ecs_add_pair(it->world, e, EgFsType, EgFsTypeLangC);
		} else {
			ecs_add_pair(it->world, e, EgFsType, EgFsTypeUnknown);
		}
	}
}





static void LoadFile(ecs_iter_t *it)
{
	EgStrText *rootpath = ecs_field(it, EgStrText, 1);
	ecs_world_t *world = it->world;

	char pathbuf[128] = {0};

	for (int i = 0; i < it->count; ++i) {
		ecs_entity_t e = it->entities[i];

		// Remove loading action
		ecs_remove_id(world, e, EgFsLoad);

		char const *prefix = rootpath->value;

		{
			// Set nice name for flecs explorer:
			char const *name = ecs_get_name(world, e);
			ecs_os_strncpy(pathbuf, name, sizeof(pathbuf));
			eg_str_replace_ab(pathbuf, ',', '.');
			ecs_doc_set_name(world, e, pathbuf);
		}

		{
			// Generate path from flecs name:
			ecs_entity_t rootnode = it->sources[0];
			char *name = ecs_get_path_w_sep(world, rootnode, e, "/", prefix);
			eg_str_replace_ab(name, ',', '.');
			ecs_os_sprintf(pathbuf, "%s/%s", prefix, name);
			ecs_os_free(name);
			ecs_set_pair(world, e, EgStrText, EgFsPath, {pathbuf});
		}


		{
			// Generate realpath from relative path:
			char resolved_path[256];
			realpath(pathbuf, resolved_path);
			ecs_set_pair(world, e, EgStrText, EgFsPathReal, {resolved_path});
		}

		{
			// Try load content of the file
			int32_t size;
			char *content = eg_fs_readfile_and_size(pathbuf, &size);
			if (content) {
				EgStrBuffer *cont = ecs_ensure(world, e, EgStrBuffer);
				cont->data = content;
				cont->size = size;
			}
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
	ECS_TAG_DEFINE(world, EgFsPathReal);

	ECS_TAG_DEFINE(world, EgFsMonitor);

	ECS_TAG_DEFINE(world, EgFsList);

	ECS_TAG_DEFINE(world, EgFsDir);
	ECS_TAG_DEFINE(world, EgFsFile);

	ECS_TAG_DEFINE(world, EgFsCwd);
	ECS_TAG_DEFINE(world, EgFsCreated);
	ECS_TAG_DEFINE(world, EgFsModified);
	ECS_TAG_DEFINE(world, EgFsAccessed);

	ECS_TAG_DEFINE(world, EgFsOwner);
	ECS_TAG_DEFINE(world, EgFsDomain);

	ECS_ENTITY_DEFINE(world, EgFsType, EcsUnion);
	ECS_TAG_DEFINE(world, EgFsTypeUnknown);
	ECS_TAG_DEFINE(world, EgFsTypeDir);
	ECS_TAG_DEFINE(world, EgFsTypeExe);
	ECS_TAG_DEFINE(world, EgFsTypeLangC);

	ECS_TAG_DEFINE(world, EgFsRoot);
	ECS_TAG_DEFINE(world, EgFsLoad);
	ECS_TAG_DEFINE(world, EgFsSave);

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

	ecs_system(world, {
		.entity = ecs_entity(world, {
		.name = "System_File_Tagging",
		.add = { ecs_dependson(EcsOnUpdate) }
		}),
		.query.filter.terms = {
			{.id = ecs_pair(ecs_id(EgStrText), EgFsPath) },
			{.id = EgFsFile },
			{.id = ecs_pair(EgFsType, EcsWildcard), .oper=EcsNot }, // Adds this
		},
		.callback = System_File_Tagging
	});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "LoadFile", .add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = LoadFile,
	.query.filter.terms = {
	{.id = ecs_pair(ecs_id(EgStrText), EgFsRoot), .src.trav = EcsIsA, .src.flags = EcsUp},
	{.id = EgFsLoad}
	}});
}
