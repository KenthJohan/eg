#include "egimgui.h"
#include "egspatials.h"
#include "egshapes.h"



void sys1(ecs_world_t *world, ecs_query_t *q)
{
	assert(world);
	assert(q);
	// int n = ecs_query_entity_count(q);
	int n = 0;
	ecs_iter_t it = ecs_query_iter(world, q);
	while (ecs_query_next(&it)) {
		Position2 *p = ecs_field(&it, Position2, 1);
		Rectangle *r = ecs_field(&it, Rectangle, 2);
		for (int i = 0; i < it.count; ++i, ++p, ++r) {

		}
	}
}





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