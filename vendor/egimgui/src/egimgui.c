#include "egimgui.h"










void EgImguiImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgImgui);
	ecs_set_name_prefix(world, "EgImgui");

	//ECS_COMPONENT_DEFINE(world, EgCanEpoll);


	// clang-format off
	/*
	ecs_struct(world,
	{.entity = ecs_id(EgCanSignal),
	.members = {
	{.name = "canid", .type = ecs_id(ecs_u32_t)},
	{.name = "rx", .type = ecs_id(ecs_i32_t)},
	{.name = "tx", .type = ecs_id(ecs_i32_t)},
	{.name = "byte_offset", .type = ecs_id(ecs_i32_t)},
	{.name = "min", .type = ecs_id(ecs_i32_t)},
	{.name = "max", .type = ecs_id(ecs_i32_t)},
	{.name = "gui_index", .type = ecs_id(ecs_i32_t)},
	}});
	*/
	// clang-format on
}