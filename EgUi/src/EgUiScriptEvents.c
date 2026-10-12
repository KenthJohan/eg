#include "EgUi/EgUiScriptEvents.h"
#include "EgUi/EgUiLocalMouse.h"
#include <EgUi.h>

#include <EgButtons.h>
#include <EgIntersects.h>
#include <EgSpatials.h>
#include <ecsx.h>

ECS_TAG_DECLARE(EgUiScriptEventsTarget);

#define EG_UI_SCANCODE_LCTRL  224
#define EG_UI_SCANCODE_LSHIFT 225
#define EG_UI_SCANCODE_LALT   226
#define EG_UI_SCANCODE_LGUI   227

typedef struct {
	ecs_query_t *targets;
	uint8_t      prev_mouse[EG_BUTTONS_MOUSE_MAX];
	float        screen_x;
	float        screen_y;
} EgUiScriptEventsContext;

// Finds the hit target: deepest in the hierarchy, last in query order on ties.
static ecs_entity_t EgUiScriptEvents_PickTarget(ecs_world_t *world, ecs_query_t *query, EgUiScriptEventsContext *ctx)
{
	ecs_entity_t best       = 0;
	int32_t      best_depth = -1;
	ecs_iter_t   qit        = ecs_query_iter(world, query);
	ecs_iter_t  *it         = &qit;
	while (ecs_query_next(it)) {
		EgIntersectsRectangleBorder const *b = ecs_field_self(it, EgIntersectsRectangleBorder, 1);
		Position2 const                   *p = ecs_field_shared(it, Position2, 3);
		ctx->screen_x                        = p->x;
		ctx->screen_y                        = p->y;
		for (int32_t i = 0; i < it->count; ++i) {
			if (!EG_INTERSECTS_RECTANGLE_BORDER_IS_INSIDE(b[i])) {
				continue;
			}
			int32_t depth = ecs_get_depth(world, it->entities[i], EcsChildOf);
			if (depth >= best_depth) {
				best_depth = depth;
				best       = it->entities[i];
			}
		}
	}
	return best;
}

static void EgUiScriptEvents_Emit(ecs_iter_t *it)
{
	EgUiScriptEventsContext *ctx = it->ctx;
	ecs_world_t             *w   = it->world;
	EgButtonsState const    *s   = ecs_field(it, EgButtonsState, 0);

	ecs_entity_t target = EgUiScriptEvents_PickTarget(w, ctx->targets, ctx);

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

	// Entities get the event in their own local space, then it propagates to the ancestors.
	for (ecs_entity_t t = target;; ) {
		if (t) {
			EgUiLocalMousePosition const *l = ecs_get(w, t, EgUiLocalMousePosition);
			e.local_x                       = l ? l->x : 0.0f;
			e.local_y                       = l ? l->y : 0.0f;
		}
		e.target = t;
		if (!ecs_script_mouse_event(w, &e) || !t) {
			break;
		}
		t = ecs_get_parent(w, t);
		if (!t) {
			break;
		}
	}
}

static void EgUiScriptEventsContext_Free(void *ptr)
{
	EgUiScriptEventsContext *ctx = ptr;
	ecs_query_fini(ctx->targets);
	ecs_os_free(ctx);
}

void EgUiScriptEventsImport(ecs_world_t *world)
{
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgButtons);
	ECS_IMPORT(world, EgIntersects);
	ECS_IMPORT(world, EgUiLocalMouse);
	ECS_IMPORT(world, FlecsScriptEvent);

	ECS_MODULE(world, EgUiScriptEvents);
	ecs_set_name_prefix(world, "EgUiScriptEvents");

	ECS_TAG_DEFINE(world, EgUiScriptEventsTarget);

	EgUiScriptEventsContext *ctx = ecs_os_calloc_t(EgUiScriptEventsContext);
	ctx->targets                 = ecs_query(world,
	{.terms = {
	 {.id = EgUiScriptEventsTarget},
	 {.id = ecs_id(EgIntersectsRectangleBorder), .src.id = EcsSelf, .inout = EcsIn},
	 {.id = ecs_id(EgUiLocalMousePosition), .src.id = EcsSelf, .inout = EcsIn},
	 {.id = ecs_id(Position2), .trav = EgUiMouse, .src.id = EcsUp, .inout = EcsIn},
	 {.id = EcsDisabled, .trav = EcsChildOf, .src.id = EcsUp, .oper = EcsNot},
	 }});

	// Runs every frame (not only on hits) so leave and release reach the script.
	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity        = ecs_entity(world, {.name = "EgUiScriptEvents_Emit"}),
	.phase         = EcsPreStore,
	.callback      = EgUiScriptEvents_Emit,
	.immediate     = true,
	.ctx           = ctx,
	.ctx_free      = EgUiScriptEventsContext_Free,
	.query.terms   = {
	{.id = ecs_id(EgButtonsState), .src.id = ecs_id(EgButtonsState), .inout = EcsIn},
	}});
}
