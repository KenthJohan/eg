#include "GsUserinput.h"
#include <gs/gs.h>
#include <gs/util/gs_idraw.h>
#include <gs/util/gs_gui.h>
#include <gs_ddt/gs_ddt.h>
#include <flecs.h>
#include "EgQuantities.h"
#include "EgUserinput.h"





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

	m->scroll_x = 0;
	m->scroll_y = 0;
	m->left = 0;
	m->right = 0;

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
				m->scroll_x += evt.mouse.wheel.x;
				m->scroll_y += evt.mouse.wheel.y;
				//gs_gui_input_scroll(ctx, (int32_t)(-evt.mouse.wheel.x * 30.f), (int32_t)(-evt.mouse.wheel.y * 30.f));
			}
			break;

			//case GS_PLATFORM_MOUSE_BUTTON_DOWN:
			case GS_PLATFORM_MOUSE_BUTTON_PRESSED:
			{
				if ( (1 << evt.mouse.button) & GS_GUI_MOUSE_LEFT )
				{
					m->left = EG_EDGE_RISING;
				}
				if ( (1 << evt.mouse.button) & GS_GUI_MOUSE_RIGHT )
				{
					m->right = EG_EDGE_RISING;
				}
				// int32_t code = 1 << evt.mouse.button;
				// gs_gui_input_mousedown(ctx, (int32_t)mouse_pos.x, (int32_t)mouse_pos.y, code);
			}
			break;

			case GS_PLATFORM_MOUSE_BUTTON_RELEASED:
			{
				if ( (1 << evt.mouse.button) & GS_GUI_MOUSE_LEFT )
				{
					m->left = EG_EDGE_FALLING;
				}
				if ( (1 << evt.mouse.button) & GS_GUI_MOUSE_RIGHT )
				{
					m->right = EG_EDGE_FALLING;
				}
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

void GsUserinputImport(ecs_world_t *world)
{
	ECS_MODULE(world, GsUserinput);
	ECS_IMPORT(world, EgQuantities);
	ECS_IMPORT(world, EgUserinput);

	ecs_entity_t e_Update_Keyboard = ecs_system(world, {.entity = ecs_entity(world, {.name = "Update_Keyboard",
		.add = {ecs_dependson(EcsPreUpdate)}}),
		.query.filter.terms = {
		{.id = ecs_id(EgPosition_V2F32), .src.id = ecs_id(EgMouse)},
		{.id = ecs_id(EgPosition_V2F32), .src.id = ecs_id(EgMouse)},
		{.id = ecs_id(EgMouse), .src.id = ecs_id(EgMouse)},
		{.id = ecs_id(EgKeyboard), .src.id = ecs_id(EgKeyboard)}
		},
		.callback = Update_Keyboard});

}
