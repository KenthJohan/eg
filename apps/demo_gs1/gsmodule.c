#include "gsmodule.h"
#include <gs/gs.h>
#include <gs/util/gs_idraw.h>
#include <gs/util/gs_gui.h>
#include <gs_ddt/gs_ddt.h>
#include <flecs.h>
#include "EgQuantities.h"

ECS_COMPONENT_DECLARE(GsmoduleDraw);









void Draw_Rectangle(ecs_iter_t *it)
{
	gs_immediate_draw_t *gsi = ecs_field(it, GsmoduleDraw, 1)->ptr;
	const EgV2F32 *p = ecs_field(it, EgV2F32, 2);
	const EgV2F32 *r = ecs_field(it, EgV2F32, 3);
	const EgV4U8 *c = ecs_field(it, EgV4U8, 4);
	const EgHover *h = ecs_field(it, EgHover, 5);
	ecs_assert(gsi != NULL, ECS_INVALID_PARAMETER, NULL);
	ecs_assert(p != NULL, ECS_INVALID_PARAMETER, NULL);
	ecs_assert(r != NULL, ECS_INVALID_PARAMETER, NULL);
	ecs_assert(c != NULL, ECS_INVALID_PARAMETER, NULL);
	for (int i = 0; i < it->count; i++)
	{
		gs_vec2 xy = {p[i].x, p[i].y};
		gs_vec2 wh = {r[i].x, r[i].y};
		gs_color_t color;
		if(h->entity == it->entities[i])
		{
			color = (gs_color_t){255,255,255,255};
		}
		else
		{
			color = (gs_color_t){c[i].x, c[i].y, c[i].z, c[i].w};
		}
		gsi_rectvd(gsi, xy, wh, gs_v2(0.f, 0.f), gs_v2(1.f, 1.f), color, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
	}
}


void Draw_Text(ecs_iter_t *it)
{
	gs_immediate_draw_t *gsi = ecs_field(it, GsmoduleDraw, 1)->ptr;
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



void Update_Keyboard(ecs_iter_t *it)
{
	EgV2F32 *p = ecs_field(it, EgV2F32, 1);
	EgV2F32 *v = ecs_field(it, EgV2F32, 2);
	EgMouse *m = ecs_field(it, EgMouse, 3);
	EgKeyboard *state = ecs_field(it, EgKeyboard, 4);

	ecs_assert(p != NULL, ECS_INVALID_PARAMETER, NULL);
	ecs_assert(m != NULL, ECS_INVALID_PARAMETER, NULL);
	ecs_assert(state != NULL, ECS_INVALID_PARAMETER, NULL);

	gs_platform_mouse_delta(&v[0].x, &v[0].y);
	gs_vec2 mouse_pos = gs_platform_mouse_positionv();
	p[0].x = mouse_pos.x;
	p[0].y = mouse_pos.y;

	gs_platform_event_t evt = gs_default_val();
	while (gs_platform_poll_events(&evt, false))
	{
		switch (evt.type)
		{
		case GS_PLATFORM_EVENT_MOUSE:
		{
			switch (evt.mouse.action)
			{
			case GS_PLATFORM_MOUSE_MOVE:
			{
				// ctx->mouse_pos = evt.mouse.move;
			}
			break;

			case GS_PLATFORM_MOUSE_WHEEL:
			{
				// gs_gui_input_scroll(ctx, (int32_t)(-evt.mouse.wheel.x * 30.f), (int32_t)(-evt.mouse.wheel.y * 30.f));
			}
			break;

			case GS_PLATFORM_MOUSE_BUTTON_DOWN:
			case GS_PLATFORM_MOUSE_BUTTON_PRESSED:
			{
				m->left = (1 << evt.mouse.button) & GS_GUI_MOUSE_LEFT;
				m->right = (1 << evt.mouse.button) & GS_GUI_MOUSE_RIGHT;
				// int32_t code = 1 << evt.mouse.button;
				// gs_gui_input_mousedown(ctx, (int32_t)mouse_pos.x, (int32_t)mouse_pos.y, code);
			}
			break;

			case GS_PLATFORM_MOUSE_BUTTON_RELEASED:
			{
				m->left = 0;
				m->right = 0;
				// int32_t code = 1 << evt.mouse.button;
				// gs_gui_input_mouseup(ctx, (int32_t)mouse_pos.x, (int32_t)mouse_pos.y, code);
			}
			break;

			case GS_PLATFORM_MOUSE_ENTER:
			{
				// If there are user callbacks, could trigger them here
			}
			break;

			case GS_PLATFORM_MOUSE_LEAVE:
			{
				// If there are user callbacks, could trigger them here
			}
			break;

			default:
				break;
			}
		}
		break;

		case GS_PLATFORM_EVENT_TEXT:
		{
			// Input text
			// char txt[2] = {(char)(evt.text.codepoint & 255), 0};
			// gs_gui_input_text(ctx, txt);
		}
		break;

		case GS_PLATFORM_EVENT_KEY:
		{
			switch (evt.key.action)
			{
			case GS_PLATFORM_KEY_DOWN:
			case GS_PLATFORM_KEY_PRESSED:
			{
				// gs_gui_input_keydown(ctx, key_map[evt.key.keycode & 511]);
				state->keys[ECS_MIN(evt.key.keycode & 511, (EG_KEYBOARD_SIZE - 1))] = 1;
			}
			break;

			case GS_PLATFORM_KEY_RELEASED:
			{
				// gs_gui_input_keyup(ctx, key_map[evt.key.keycode & 511]);
				state->keys[ECS_MIN(evt.key.keycode & 511, (EG_KEYBOARD_SIZE - 1))] = 0;
			}
			break;

			default:
				break;
			}
		}
		break;

		case GS_PLATFORM_EVENT_WINDOW:
		{
			switch (evt.window.action)
			{
			default:
				break;
			}
		}
		break;

		default:
			break;
		}
	}
}

void GsmoduleImport(ecs_world_t *world)
{
	ECS_MODULE(world, Gsmodule);
	ECS_IMPORT(world, EgQuantities);

	ECS_COMPONENT_DEFINE(world, GsmoduleDraw);

	ecs_entity_t e_Draw_Rectangle = ecs_system(world, {
		.entity = ecs_entity(world, {.name = "Draw_Rectangle",
		.add = {ecs_dependson(EcsOnUpdate)}}),
		.query.filter.terms = {
		{.id = ecs_id(GsmoduleDraw), .inout = EcsIn, .src.flags = EcsUp},
		{.id = ecs_pair(ecs_id(EgV2F32), EgPosition), .inout = EcsIn},
		{.id = ecs_pair(ecs_id(EgV2F32), EgRectangle), .inout = EcsIn},
		{.id = ecs_pair(ecs_id(EgV4U8), EgColor), .inout = EcsIn},
		{.id = ecs_id(EgHover), .inout = EcsIn, .src.id = ecs_id(EgMouse)},
		//Not used. Order by breadth-first order (cascade):
        {.id = ecs_pair(ecs_id(EgV2F32), EgPosition), .inout = EcsIn,.src.flags = EcsParent | EcsCascade,.oper = EcsOptional},
		},
		.callback = Draw_Rectangle,
		});


	ecs_entity_t e_Draw_Text = ecs_system(world, {
		.entity = ecs_entity(world, {.name = "Draw_Text",
		.add = {ecs_dependson(EcsOnUpdate)}}),
		.query.filter.terms = {
		{.id = ecs_id(GsmoduleDraw), .inout = EcsIn, .src.flags = EcsUp},
		{.id = ecs_pair(ecs_id(EgV2F32), EgPosition), .inout = EcsIn},
		{.id = ecs_pair(ecs_id(EgV4U8), EgColor), .inout = EcsIn},
		{.id = ecs_id(EgText), .inout = EcsIn},
		},
		.callback = Draw_Text,
		});



	ecs_entity_t e_Update_Keyboard = ecs_system(world, {.entity = ecs_entity(world, {.name = "Update_Keyboard",
		.add = {ecs_dependson(EcsOnUpdate)}}),
		.query.filter.terms = {
		{.id = ecs_pair(ecs_id(EgV2F32), EgPosition), .src.id = ecs_id(EgMouse)},
		{.id = ecs_pair(ecs_id(EgV2F32), EgVelocity), .src.id = ecs_id(EgMouse)},
		{.id = ecs_id(EgMouse), .src.id = ecs_id(EgMouse)},
		{.id = ecs_id(EgKeyboard), .src.id = ecs_id(EgKeyboard)}
		},
		.callback = Update_Keyboard});

}
