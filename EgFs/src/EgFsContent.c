#include "EgFsContent.h"
#include "EgFs.h"
#include <ecsx.h>

void EgFsContent_Load(ecs_iter_t *it)
{
	int32_t loglvl = 0;

	ecs_world_t *world = it->world;

	for (int i = 0; i < it->count; ++i) {
		ecs_entity_t e = it->entities[i];

		// ecs_ensure works whether EgFsContent already exists (reload) or not (initial add)
		EgFsContent *c = ecs_ensure(world, e, EgFsContent);

		if (c->data) {
			// Free previous content
			ecs_log(loglvl, "free %i bytes previous EgFsContent for entity '%s'", c->size, ecs_get_name(world, e));
			ecs_os_free(c->data);
			c->data = NULL;
			c->size = 0;
		}
		ecs_log(loglvl, "loading content for entity '%s'", ecs_get_name(world, e));

		// Paths are defined as entities and its hierarchy mirrors the filesystem structure.
		// Allocate and build path from entity hierarchy, seperated by "/" and prefixed by "./"
		char *path = ecs_get_path_w_sep(world, EgFsCwd, e, "/", "./");

		ecsx_pathkind_t path_type = ecsx_pathkind_get_path_type(path);

		size_t size    = 0;
		void  *content = NULL;
		if (path_type == ECSX_PATHKIND_FILE) {
			content = ecsx_file_load_alloc(path, &size);
		}
		ecs_os_free(path);

		if (!content) {
			ecs_err("failed to load file for entity '%s'", ecs_get_name(world, e));
			continue;
		}

		c->data = content;
		c->size = (uint32_t)size;
		ecs_log(loglvl, "loaded %u bytes into EgFsContent for entity '%s'", c->size, ecs_get_name(world, e));
		ecs_modified(world, e, EgFsContent);

		// Observer_OnModify_extra(world, e);
		// Add EgFsDump to dump content in System_Dump
		// ecs_add(world, e, EgFsDump);
	}
}
