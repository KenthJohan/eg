#include "egimgui.h"

#include <egspatials.h>
#include <egshapes.h>
#include <egquantities.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>


ecs_query_t * egimgui_query1(ecs_world_t *world)
{
	ecs_query_t * q = ecs_query(world, {
		.filter.terms = {
			{.id = ecs_id(Position2)},
			{.id = ecs_id(Rectangle)},
			{.id = ecs_id(EgQuantitiesProgress)}
		}
		}
	);
	return q;
}


void egimgui_progress1(ecs_world_t *world, ecs_query_t *q)
{
	assert(world);
	assert(q);
	// int n = ecs_query_entity_count(q);
	//int n = 0;
	ecs_iter_t it = ecs_query_iter(world, q);
	while (ecs_query_next(&it)) {
		Position2 *p = ecs_field(&it, Position2, 1);
		Rectangle *r = ecs_field(&it, Rectangle, 2);
		EgQuantitiesProgress *q = ecs_field(&it, EgQuantitiesProgress, 3);
		for (int i = 0; i < it.count; ++i, ++p, ++r, ++q) {
			ImDrawList * a = igGetWindowDrawList();
			ImVec2 sp;
			igGetCursorScreenPos(&sp);
			float x = sp.x + p->x;
            float y = sp.y + p->y;
			static float sz = 36.0f;
			static float th = 3.0f;
			//static int ngon_sides = 6;
			//ImDrawList_AddCircle(a, (ImVec2){x + sz*0.5f, y + sz*0.5f}, sz*0.5f, 0xFFFFFFFF, 12, th);
			//ImDrawList_AddCircle(a, (ImVec2){x + sz*1.5f - 20, y + sz*1.5f}, sz*1.5f, 0xFFFFFFFF, 12, th);
			//ImDrawList_AddText_Vec2(a, (ImVec2){x + sz*1.5f - 20, y + sz*1.5f}, 0xFFFFFFFF, "Hej!", NULL);
			float d = q->max - q->min;
			if(d > __FLT32_MIN__) {
				float v = q->value - q->min;
				float h = r->h * v / d;
				ImDrawList_AddRectFilled(a, (ImVec2){x, y + r->h}, (ImVec2){x + r->w, y + r->h - h}, 0xFFFFFFFF, 0.0f, 0);
			}
			ImDrawList_AddRect(a, (ImVec2){x, y}, (ImVec2){x + r->w, y + r->h}, 0xFFFFFFFF, 0.0f, 0, 1.0f);
			char buf[128];
			snprintf(buf, sizeof(buf), "%f", q->value);
			ImDrawList_AddText_Vec2(a, (ImVec2){x + r->w/2, y + r->h/2}, 0xFF0000FF, buf, NULL);
		}
	}
}





void EgImguiImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgImgui);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgShapes);
	ECS_IMPORT(world, EgQuantities);
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