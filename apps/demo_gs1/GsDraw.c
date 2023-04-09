#include "GsDraw.h"
#include <gs/gs.h>
#include <gs/util/gs_idraw.h>
#include <gs/util/gs_gui.h>
#include <gs_ddt/gs_ddt.h>
#include <flecs.h>
#include "EgQuantities.h"
#include "EgUserinput.h"
#include "EgGui.h"

ECS_COMPONENT_DECLARE(GsImmediateDraw);




void Draw_Rectangle(ecs_iter_t *it)
{
	gs_immediate_draw_t *gsi = ecs_field(it, GsImmediateDraw, 1)->ptr;
	const EgV2F32 *p = ecs_field(it, EgV2F32, 2);
	const EgV2F32 *r = ecs_field(it, EgV2F32, 3);
	const EgV4U8 *c = ecs_field(it, EgV4U8, 4);
	ecs_assert(gsi != NULL, ECS_INVALID_PARAMETER, NULL);
	ecs_assert(p != NULL, ECS_INVALID_PARAMETER, NULL);
	ecs_assert(r != NULL, ECS_INVALID_PARAMETER, NULL);
	ecs_assert(c != NULL, ECS_INVALID_PARAMETER, NULL);
	for (int i = 0; i < it->count; i++)
	{
		gs_vec2 xy = {p[i].x, p[i].y};
		gs_vec2 wh = {r[i].x, r[i].y};
		gs_color_t color = (gs_color_t){c[i].x, c[i].y, c[i].z, c[i].w};
		gsi_rectvd(gsi, xy, wh, gs_v2(0.f, 0.f), gs_v2(1.f, 1.f), color, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
	}
}


void Draw_Rectangle_Border(ecs_iter_t *it)
{
	gs_immediate_draw_t *gsi = ecs_field(it, GsImmediateDraw, 1)->ptr;
	const EgV2F32 *p = ecs_field(it, EgV2F32, 2);
	const EgV2F32 *r = ecs_field(it, EgV2F32, 3);
	const EgGuiBorder4 *b = ecs_field(it, EgGuiBorder4, 4);
	ecs_assert(gsi != NULL, ECS_INVALID_PARAMETER, NULL);
	ecs_assert(p != NULL, ECS_INVALID_PARAMETER, NULL);
	ecs_assert(r != NULL, ECS_INVALID_PARAMETER, NULL);
	for (int i = 0; i < it->count; i++)
	{
		gs_vec2 xy = {p[i].x, p[i].y};
		gs_vec2 wh = {r[i].x, r[i].y};
		gs_color_t color;
		gsi_rectvd(gsi, xy, wh, gs_v2(0.f, 0.f), gs_v2(1.f, 1.f), color, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
		gsi_rectvd(gsi, xy, wh, gs_v2(0.f, 0.f), gs_v2(1.f, 1.f), color, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
	}
}












void Draw_Text(ecs_iter_t *it)
{
	gs_immediate_draw_t *gsi = ecs_field(it, GsImmediateDraw, 1)->ptr;
	const EgV2F32 *p = ecs_field(it, EgV2F32, 2);
	const EgV4U8 *c = ecs_field(it, EgV4U8, 3);
	const EgText *text = ecs_field(it, EgText, 4);
	ecs_assert(gsi != NULL, ECS_INVALID_PARAMETER, NULL);
	ecs_assert(p != NULL, ECS_INVALID_PARAMETER, NULL);
	ecs_assert(c != NULL, ECS_INVALID_PARAMETER, NULL);
	ecs_assert(text != NULL, ECS_INVALID_PARAMETER, NULL);
	for (int i = 0; i < it->count; i++)
	{
		gsi_text(gsi, p[i].x, p[i].y, text[i].value, NULL, false, c[i].x, c[i].y, c[i].z, c[i].w);
	}
}





void GsDrawImport(ecs_world_t *world)
{
	ECS_MODULE(world, GsDraw);
	ECS_IMPORT(world, EgQuantities);
	ECS_IMPORT(world, EgUserinput);
	ECS_IMPORT(world, EgGui);


	ECS_COMPONENT_DEFINE(world, GsImmediateDraw);

	ecs_entity_t e_Draw_Rectangle = ecs_system(world, {
		.entity = ecs_entity(world, {.name = "Draw_Rectangle",
		.add = {ecs_dependson(EcsOnUpdate)}}),
		.query.filter.terms = {
		{.id = ecs_id(GsImmediateDraw), .inout = EcsIn, .src.flags = EcsUp},
		{.id = ecs_pair(ecs_id(EgV2F32), EgPosition), .inout = EcsIn},
		{.id = ecs_pair(ecs_id(EgV2F32), EgRectangle), .inout = EcsIn},
		{.id = ecs_pair(ecs_id(EgV4U8), EgColor), .inout = EcsIn},
		//Not used. Order by breadth-first order (cascade):
        {.id = ecs_pair(ecs_id(EgV2F32), EgPosition), .inout = EcsIn,.src.flags = EcsParent | EcsCascade,.oper = EcsOptional},
		},
		.callback = Draw_Rectangle,
		});

	ecs_entity_t e_Draw_Text = ecs_system(world, {
		.entity = ecs_entity(world, {.name = "Draw_Text",
		.add = {ecs_dependson(EcsOnUpdate)}}),
		.query.filter.terms = {
		{.id = ecs_id(GsImmediateDraw), .inout = EcsIn, .src.flags = EcsUp},
		{.id = ecs_pair(ecs_id(EgV2F32), EgPosition), .inout = EcsIn},
		{.id = ecs_pair(ecs_id(EgV4U8), EgColor), .inout = EcsIn},
		{.id = ecs_id(EgText), .inout = EcsIn},
		},
		.callback = Draw_Text,
		});

}
