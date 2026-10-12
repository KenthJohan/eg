#include "EgUi.h"

#include <EgButtons.h>
#include <EgIntersects.h>
#include <ecsx.h>

ECS_TAG_DECLARE(EgUiMouse);

void EgUiImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgUi);
	ecs_set_name_prefix(world, "EgUi");
	ECS_TAG_DEFINE(world, EgUiMouse);
	ecs_add_id(world, EgUiMouse, EcsTraversable);

	ECS_IMPORT(world, EgShapes);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgButtons);
	ECS_IMPORT(world, EgIntersects);
	ECS_IMPORT(world, EgUiLocalMouse);
	ECS_IMPORT(world, EgUiButtons);
	ECS_IMPORT(world, EgUiFlows);
	ECS_IMPORT(world, EgUiFreeforms);
	ECS_IMPORT(world, EgUiBounds);
	ECS_IMPORT(world, EgUiScriptEvents);
	ecs_set_scope(world, ecs_id(EgUi));
	ecs_set_name_prefix(world, "EgUi");
}
