#include "EgFs.h"
#include "EgQuantities.h"
#include "eg_dirwatch.h"




typedef struct
{
	int dummy;
} EgDirwatch;







ECS_TAG_DECLARE(EgFsAdded);
ECS_TAG_DECLARE(EgFsModified);
ECS_TAG_DECLARE(EgFsPath);
ECS_TAG_DECLARE(EgFsMonitor);




void System_Margin(ecs_iter_t *it)
{
	for (int i = 0; i < it->count; i ++)
	{
	}
}





void EgFsImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgFs);
	ecs_set_name_prefix(world, "Eg");
	ECS_IMPORT(world, EgQuantities);
	ECS_TAG_DEFINE(world, EgFsAdded);
	ECS_TAG_DEFINE(world, EgFsModified);
	ECS_TAG_DEFINE(world, EgFsPath);
	ECS_TAG_DEFINE(world, EgFsMonitor);


	ecs_entity_t a = ecs_new_entity(world, "my_path");
	ecs_set_pair(world, a, EgText, EgFsPath, {"./banana/hello.txt"});
	ecs_add(world, a, EgFsMonitor);


	ecs_observer(world, {
	.filter = { .terms = {
		{ .id = ecs_id(EgFsPath) },
		{ .id = ecs_id(EgFsMonitor) },
	}},
	.events = { EcsOnSet },
	.callback = System_Margin,
	.yield_existing = true
	});

}


