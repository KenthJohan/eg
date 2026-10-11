#include "EgUi.h"

#include <EgPhysics.h>
#include <EgButtons.h>
#include <EgIntersects.h>
#include <ecsx.h>



void EgUiImport(ecs_world_t *world)
{
	ECS_IMPORT(world, EgPhysics);
	ECS_IMPORT(world, EgShapes);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgButtons);
	ECS_IMPORT(world, EgIntersects);
	ECS_IMPORT(world, EgUiLocalMouse);
	ECS_IMPORT(world, EgUiButtons);
	ECS_IMPORT(world, EgUiFlows);
	ECS_IMPORT(world, EgUiFreeforms);
	ECS_IMPORT(world, EgUiBounds);

	ECS_MODULE(world, EgUi);
	ecs_set_name_prefix(world, "EgUi");
}
