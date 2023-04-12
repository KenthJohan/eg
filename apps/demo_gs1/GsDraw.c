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




int compare_position(ecs_entity_t e1, const EgZIndex *p1, ecs_entity_t e2, const EgZIndex *p2)
{
	(void)e1;
	(void)e2;
	return (p1->z > p2->z);
}


void Draw_Rectangle(ecs_iter_t *it)
{
	gs_immediate_draw_t          *gsi = ecs_field(it, GsImmediateDraw, 1)->ptr;
	const EgPositionGlobal_V2F32 *p   = ecs_field(it, EgPositionGlobal_V2F32, 2);
	const EgRectangle_V2F32      *r   = ecs_field(it, EgRectangle_V2F32, 3);
	const EgColorRGBA_V4U8       *c   = ecs_field(it, EgColorRGBA_V4U8, 4);
	int c_self = ecs_field_is_self(it, 4);
	ecs_assert(gsi != NULL, ECS_INVALID_PARAMETER, NULL);
	ecs_assert(p != NULL, ECS_INVALID_PARAMETER, NULL);
	ecs_assert(r != NULL, ECS_INVALID_PARAMETER, NULL);
	ecs_assert(c != NULL, ECS_INVALID_PARAMETER, NULL);
	for (int i = 0; i < it->count; i++)
	{
		gs_vec2 xy = {p[i].x, p[i].y};
		gs_vec2 wh = {r[i].w, r[i].h};
		EgColorRGBA_V4U8 * cc = c + (i * c_self);
		gs_color_t color = (gs_color_t){cc->r, cc->g, cc->b, cc->a};
		gsi_rectvd(gsi, xy, wh, gs_v2(0.f, 0.f), gs_v2(1.f, 1.f), color, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
	}
}


void Draw_Text(ecs_iter_t *it)
{
	gs_immediate_draw_t          *gsi  = ecs_field(it, GsImmediateDraw, 1)->ptr;
	const EgPositionGlobal_V2F32 *p    = ecs_field(it, EgPositionGlobal_V2F32, 2);
	const EgColorRGBA_V4U8       *c    = ecs_field(it, EgColorRGBA_V4U8, 3);
	const EgText                 *text = ecs_field(it, EgText, 4);
	ecs_assert(gsi != NULL, ECS_INVALID_PARAMETER, NULL);
	ecs_assert(p != NULL, ECS_INVALID_PARAMETER, NULL);
	ecs_assert(c != NULL, ECS_INVALID_PARAMETER, NULL);
	ecs_assert(text != NULL, ECS_INVALID_PARAMETER, NULL);
	for (int i = 0; i < it->count; i++)
	{
		gsi_text(gsi, p[i].x, p[i].y, text[i].value, NULL, false, c[i].r, c[i].g, c[i].b, c[i].a);
	}
}





void GsDrawImport(ecs_world_t *world)
{
	ECS_MODULE(world, GsDraw);
	ECS_IMPORT(world, EgQuantities);
	ECS_IMPORT(world, EgUserinput);
	ECS_IMPORT(world, EgGui);


	ECS_COMPONENT_DEFINE(world, GsImmediateDraw);


	ecs_system(world, {
		.entity = ecs_entity(world, {.name = "Draw_Rectangle1",
		.add = {ecs_dependson(EcsOnUpdate)}}),
		.query = {
			.filter.terms = {
				{.id = ecs_id(GsImmediateDraw), .inout = EcsIn, .src.flags = EcsUp},
				{.id = ecs_id(EgPositionGlobal_V2F32), .inout = EcsIn},
				{.id = ecs_id(EgRectangle_V2F32), .inout = EcsIn},
				{.id = ecs_pair(ecs_id(EgColorRGBA_V4U8), EgGuiMouseOver1), .inout = EcsIn},
				{.id = ecs_id(EgGuiMouseOver1)},
				{.id = ecs_id(EgZIndex)},
				//Not used. Order by breadth-first order (cascade):
				//{.id = ecs_id(EgPositionGlobal_V2F32), .inout = EcsIn,.src.flags = EcsParent | EcsCascade,.oper = EcsOptional},
			},
			.order_by = (ecs_order_by_action_t)compare_position,
			.order_by_component = ecs_id(EgZIndex),
		},
		.callback = Draw_Rectangle,
	});
		
	ecs_system(world, {
		.entity = ecs_entity(world, {.name = "Draw_Rectangle",
		.add = {ecs_dependson(EcsOnUpdate)}}),
		.query = {
			.filter.terms = {
				{.id = ecs_id(GsImmediateDraw), .inout = EcsIn, .src.flags = EcsUp},
				{.id = ecs_id(EgPositionGlobal_V2F32), .inout = EcsIn},
				{.id = ecs_id(EgRectangle_V2F32), .inout = EcsIn},
				{.id = ecs_id(EgColorRGBA_V4U8), .inout = EcsIn},
				{.id = ecs_id(EgGuiMouseOver1), .oper = EcsNot},
				{.id = ecs_id(EgZIndex)},
				//Not used. Order by breadth-first order (cascade):
				//{.id = ecs_id(EgPositionGlobal_V2F32), .inout = EcsIn,.src.flags = EcsParent | EcsCascade,.oper = EcsOptional},
			},
			.order_by = (ecs_order_by_action_t)compare_position,
			.order_by_component = ecs_id(EgZIndex),
		},
		.callback = Draw_Rectangle,
	});


	ecs_system(world, {
		.entity = ecs_entity(world, {.name = "Draw_Text",
		.add = {ecs_dependson(EcsOnUpdate)}}),
		.query = {
			.filter.terms = {
				{.id = ecs_id(GsImmediateDraw), .inout = EcsIn, .src.flags = EcsUp},
				{.id = ecs_id(EgPositionGlobal_V2F32), .inout = EcsIn},
				{.id = ecs_id(EgColorRGBA_V4U8), .inout = EcsIn},
				{.id = ecs_id(EgText), .inout = EcsIn},
			},
		},
		.callback = Draw_Text,
	});

}
