#include "EgUi.h"

#include <EgButtons.h>
#include <ecsx.h>

ECS_TAG_DECLARE(EgUiMouse);
ECS_COMPONENT_DECLARE(EgUiAt);
ECS_COMPONENT_DECLARE(EgUiDir);

void EgUiImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgUi);
	ecs_set_name_prefix(world, "EgUi");
	ECS_TAG_DEFINE(world, EgUiMouse);
	ecs_add_id(world, EgUiMouse, EcsTraversable);

	ECS_COMPONENT_DEFINE(world, EgUiAt);
	ecs_enum_init(world,
	&(ecs_enum_desc_t){
	.entity    = ecs_id(EgUiAt),
	.constants = {
	{.name = "Middle", .value = EgUiAtMiddle},
	{.name = "TopLeft", .value = EgUiAtTopLeft},
	{.name = "Top", .value = EgUiAtTop},
	{.name = "TopRight", .value = EgUiAtTopRight},
	{.name = "Left", .value = EgUiAtLeft},
	{.name = "Right", .value = EgUiAtRight},
	{.name = "BottomLeft", .value = EgUiAtBottomLeft},
	{.name = "Bottom", .value = EgUiAtBottom},
	{.name = "BottomRight", .value = EgUiAtBottomRight},
	}});

	ECS_COMPONENT_DEFINE(world, EgUiDir);
	ecs_enum_init(world,
	&(ecs_enum_desc_t){
	.entity    = ecs_id(EgUiDir),
	.constants = {
	{.name = "None", .value = EgUiDirNone},
	{.name = "Right", .value = EgUiDirRight},
	{.name = "Left", .value = EgUiDirLeft},
	{.name = "Up", .value = EgUiDirUp},
	{.name = "Down", .value = EgUiDirDown},
	}});

	ECS_IMPORT(world, EgShapes);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgButtons);
	ECS_IMPORT(world, EgUiLocalMouse);
	ECS_IMPORT(world, EgUiFlows);
	ECS_IMPORT(world, EgUiFreeforms);
	ECS_IMPORT(world, EgUiBounds);
	ECS_IMPORT(world, EgUiScriptEvents);
	ecs_set_scope(world, ecs_id(EgUi));
	ecs_set_name_prefix(world, "EgUi");
}
