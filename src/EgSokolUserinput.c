#include "EgSokolUserinput.h"

#include <flecs.h>
#include "EgQuantities.h"
#include "EgUserinput.h"



void sokol_input_action(const sapp_event* evt, ecs_world_t *world)
{
	EgPosition_V2F32 *p = ecs_get_mut(world, ecs_id(EgMouse), EgPosition_V2F32);
	EgVelocity_V2F32 *pd = ecs_get_mut(world, ecs_id(EgMouse), EgVelocity_V2F32);
	EgMouse *m = ecs_get_mut(world, ecs_id(EgMouse), EgMouse);
	EgKeyboard *k = ecs_get_mut(world, ecs_id(EgKeyboard), EgKeyboard);
	p->x = evt->mouse_x;
	p->y = evt->mouse_y;
	pd->dx = evt->mouse_dx;
	pd->dy = evt->mouse_dy;
	switch (evt->type) {
	case SAPP_EVENTTYPE_MOUSE_DOWN:
		if (evt->mouse_button == SAPP_MOUSEBUTTON_LEFT){m->left = 1;}
		if (evt->mouse_button == SAPP_MOUSEBUTTON_RIGHT){m->right = 1;}
		break;
	case SAPP_EVENTTYPE_MOUSE_UP:
		if (evt->mouse_button == SAPP_MOUSEBUTTON_LEFT){m->left = 0;}
		if (evt->mouse_button == SAPP_MOUSEBUTTON_RIGHT){m->right = 0;}
		break;
	case SAPP_EVENTTYPE_MOUSE_SCROLL:
		break;
	case SAPP_EVENTTYPE_KEY_UP:
		if(evt->key_code < EG_KEYBOARD_SIZE)
		{
			k->keys[evt->key_code] = 0;
		}
		break;
	case SAPP_EVENTTYPE_KEY_DOWN:
		if(evt->key_code < EG_KEYBOARD_SIZE)
		{
			k->keys[evt->key_code] = 1;
		}
		break;
	case SAPP_EVENTTYPE_RESIZED: {
		break;
	}
	default:
		break;
	}
}

void EgSokolUserinputImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgSokolUserinput);
	ECS_IMPORT(world, EgQuantities);
	ECS_IMPORT(world, EgUserinput);

}
