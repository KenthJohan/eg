#include "EgUi/EgUiScriptEvents.h"
#include "EgUi/EgUiLocalMouse.h"
#include <EgUi.h>

#include <EgButtons.h>
#include <ecsx.h>

ECS_TAG_DECLARE(EgUiScriptEventsTarget);
ECS_COMPONENT_DECLARE(EgUiScriptEventsPicker);

#define EG_UI_SCANCODE_LCTRL  224
#define EG_UI_SCANCODE_LSHIFT 225
#define EG_UI_SCANCODE_LALT   226
#define EG_UI_SCANCODE_LGUI   227

typedef struct {
	uint8_t      prev_mouse[EG_BUTTONS_MOUSE_MAX];
	float        screen_x;
	float        screen_y;
} EgUiScriptEventsContext;

static void EgUiScriptEvents_Emit(ecs_iter_t *it)
{
	EgUiScriptEventsContext *ctx = it->ctx;
	ecs_world_t             *w   = it->world;
	EgButtonsState const    *s   = ecs_field(it, EgButtonsState, 0);

	ecs_entity_t target = 0;
	EgUiScriptEventsPicker const *picker = ecs_singleton_get(w, EgUiScriptEventsPicker);
	if (picker && picker->pick &&
		!picker->pick(w, &target, &ctx->screen_x, &ctx->screen_y, picker->ctx)) {
		target = 0;
	}
	EcsScriptMouseEvent e = {
	.target   = target,
	.screen_x = ctx->screen_x,
	.screen_y = ctx->screen_y,
	.button   = 0,
	.ctrl     = !!(s->scancode[EG_UI_SCANCODE_LCTRL] & EG_BUTTONS_STATE_HELD),
	.shift    = !!(s->scancode[EG_UI_SCANCODE_LSHIFT] & EG_BUTTONS_STATE_HELD),
	.alt      = !!(s->scancode[EG_UI_SCANCODE_LALT] & EG_BUTTONS_STATE_HELD),
	.meta     = !!(s->scancode[EG_UI_SCANCODE_LGUI] & EG_BUTTONS_STATE_HELD),
	};

	// SDL mouse buttons are 1-based (1 = left), flecs buttons are 0-based.
	for (int32_t i = 1; i < EG_BUTTONS_MOUSE_MAX && i <= 32; ++i) {
		bool held = !!(s->mouse[i] & EG_BUTTONS_STATE_HELD);
		if (held) {
			e.buttons |= 1u << (i - 1);
		}
		if (held != !!(ctx->prev_mouse[i] & EG_BUTTONS_STATE_HELD)) {
			e.button = i - 1;
		}
		ctx->prev_mouse[i] = s->mouse[i];
	}

	if (target) {
		EgUiLocalMousePosition const *l = ecs_get(w, target, EgUiLocalMousePosition);
		e.local_x = l ? l->x : e.screen_x;
		e.local_y = l ? l->y : e.screen_y;
	}
	ecs_script_mouse_event(w, &e);
}

static void EgUiScriptEventsContext_Free(void *ptr)
{
	EgUiScriptEventsContext *ctx = ptr;
	ecs_os_free(ctx);
}

void EgUiScriptEventsImport(ecs_world_t *world)
{
	ECS_IMPORT(world, EgButtons);
	ECS_IMPORT(world, EgUiLocalMouse);
	ECS_IMPORT(world, FlecsScriptInput);

	ECS_MODULE(world, EgUiScriptEvents);
	ecs_set_name_prefix(world, "EgUiScriptEvents");

	ECS_TAG_DEFINE(world, EgUiScriptEventsTarget);
	ECS_COMPONENT_DEFINE(world, EgUiScriptEventsPicker);

	EgUiScriptEventsContext *ctx = ecs_os_calloc_t(EgUiScriptEventsContext);

	// Runs every frame (not only on hits) so leave and release reach the script.
	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiScriptEvents_Emit"}),
	.phase       = EcsPreStore,
	.callback    = EgUiScriptEvents_Emit,
	.immediate   = true,
	.ctx         = ctx,
	.ctx_free    = EgUiScriptEventsContext_Free,
	.query.terms = {
	{.id = ecs_id(EgButtonsState), .src.id = ecs_id(EgButtonsState), .inout = EcsIn},
	}});
}
