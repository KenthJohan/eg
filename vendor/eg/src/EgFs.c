#include "eg/EgFs.h"
#include "eg/EgStr.h"
#include <stdlib.h> 
#include <stdio.h> 

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






static char* load_from_file(const char *filename, int32_t * out_size)
{
    FILE* file;
    char* content = NULL;
    int32_t bytes;
    size_t size;

    /* Open file for reading */
    ecs_os_fopen(&file, filename, "r");
    if (!file) {
        ecs_err("%s (%s)", ecs_os_strerror(errno), filename);
        goto error;
    }

    /* Determine file size */
    fseek(file, 0, SEEK_END);
    bytes = (int32_t)ftell(file);
    if (bytes == -1) {
        goto error;
    }
    fseek(file, 0, SEEK_SET);

    /* Load contents in memory */
    content = ecs_os_malloc(bytes + 1);
	(*out_size) = bytes + 1;

    size = (size_t)bytes;
    if (!(size = fread(content, 1, size, file)) && bytes) {
        ecs_err("%s: read zero bytes instead of %d", filename, size);
        ecs_os_free(content);
        content = NULL;
        goto error;
    } else {
        content[size] = '\0';
    }

    fclose(file);

    return content;
error:
    ecs_os_free(content);
	(*out_size) = 0;
    return NULL;
}

static void replace_ab(char * str, char a, char b)
{
	char * p = str;
	while(p[0]) {
		if (p[0] == a) {
			p[0] = b;
		}
		p++;
	}
}





static void LoadFile(ecs_iter_t *it)
{
	EgText *path = ecs_field(it, EgText, 1);
	ecs_entity_t b = it->sources[0];
	for (int i = 0; i < it->count; ++i) {
		ecs_entity_t e = it->entities[i];

		// Remove loading action
		ecs_remove_id(it->world, e, EgFsLoad);

		char const * prefix = path->value;

		// Allocate a path
		char buf[128];
		char * name = ecs_get_path_w_sep(it->world, b, e, "/", prefix);
		replace_ab(name, ',', '.');
		ecs_os_sprintf(buf, "%s/%s", prefix, name);
		ecs_os_free(name);

		ecs_set_pair(it->world, e, EgText, EgFsPath, {buf});

        char resolved_path[256]; 
        realpath(buf, resolved_path);
		ecs_set_pair(it->world, e, EgText, EgFsPathReal, {resolved_path});

		int32_t size;
		char * content = load_from_file(buf, &size);
		if(content) {
			EgBuffer * cont = ecs_ensure(it->world, e, EgBuffer);
			cont->data = content;
			cont->size = size;
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
			{.id = ecs_pair(ecs_id(EgText), EgFsPath) },
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
	{.id = ecs_pair(ecs_id(EgText), EgFsRoot), .src.trav = EcsIsA, .src.flags = EcsUp},
	{.id = EgFsLoad}
	}});

}

