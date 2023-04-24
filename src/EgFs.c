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


	eg_dirwatch_bootstrap(world);
	ecs_entity_t w = eg_dirwatch_init(world, 0);
	ecs_entity_t e1 = eg_dirwatch_add(world, w, 0, "./");


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


