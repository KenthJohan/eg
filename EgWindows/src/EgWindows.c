#include "EgWindows.h"

#include <stdlib.h>
#include <stdio.h>


#include <EgShapes.h>
#include <EgSpatials.h>
#include <EgBase.h>

ECS_COMPONENT_DECLARE(EgWindowsWindow);
ECS_COMPONENT_DECLARE(EgWindowsWindowCreateInfo);
ECS_COMPONENT_DECLARE(EgWindowsMouse);
ECS_TAG_DECLARE(EgWindowsEventResize);




void EgWindowsImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgWindows);
	ecs_set_name_prefix(world, "EgWindows");


	ECS_COMPONENT_DEFINE(world, EgWindowsWindow);
	ECS_COMPONENT_DEFINE(world, EgWindowsWindowCreateInfo);
	ECS_COMPONENT_DEFINE(world, EgWindowsMouse);
	ECS_TAG_DEFINE(world, EgWindowsEventResize);

	ecs_struct(world,
	{.entity = ecs_id(EgWindowsWindow),
	.members = {
	{.name = "object", .type = ecs_id(ecs_uptr_t)}
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgWindowsWindowCreateInfo),
	.members = {
	{.name = "debug", .type = ecs_id(ecs_bool_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgWindowsMouse),
	.members = {
	{.name = "dummy", .type = ecs_id(ecs_i32_t)},
	}});

}
