#include "EgFs.h"
#include "EgQuantities.h"





ECS_TAG_DECLARE(EgFsAdded);
ECS_TAG_DECLARE(EgFsModified);
ECS_TAG_DECLARE(EgFsPath);
ECS_TAG_DECLARE(EgFsMonitorInstance);
ECS_TAG_DECLARE(EgFsMonitorDir);
ECS_TAG_DECLARE(EgDirReq);
ECS_TAG_DECLARE(EgDirRes);



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
	ECS_TAG_DEFINE(world, EgFsMonitorInstance);
	ECS_TAG_DEFINE(world, EgFsMonitorDir);

}


