#include "EgVkDevices.h"
#include "EgVk.h"
#include "EgTypes.h"
#include "EgPlatform.h"
#include "EgLogs.h"
#include "eg_util.h"
#include <stdio.h>




void fill(ecs_iter_t * it)
{
	EgIndex * index = ecs_field(it, EgIndex, 1);
	EgVkQueueFamilyProperties * prop = ecs_field(it, EgVkQueueFamilyProperties, 2);
	for (int32_t i = 0; i < it->count; ++i)
	{

	}
}

void createLogicalDevice()
{

}


void EgVkDevicesImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgVkDevices);
	ECS_IMPORT(world, EgVk);
	ECS_IMPORT(world, EgTypes);
	ecs_set_name_prefix(world, "EgVk");



}















