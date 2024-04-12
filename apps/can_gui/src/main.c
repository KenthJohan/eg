#include <SDL2/SDL.h>
#include <stdio.h>
#include <flecs.h>
#include "renderer.h"
#include "microui.h"
#include "microui_sdl.h"
#include "demo_window.h"
#include "gui_can.h"
#include "Can.h"
#include "GuiCan.h"

typedef struct {
	mu_Context muctx;
	int dummy;
} muext_context_t;

static int text_width(mu_Font font, const char *text, int len)
{
	if (len == -1) {
		len = strlen(text);
	}
	return r_get_text_width(text, len);
}

static int text_height(mu_Font font)
{
	return r_get_text_height();
}

static int compare_entity(ecs_entity_t e1, const void *v1, ecs_entity_t e2, const void *v2)
{
	return (e1 > e2) - (e1 < e2);
}

int main(int argc, char **argv)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	r_init("CAN GUI");
	mu_Context *ctx = malloc(sizeof(mu_Context));
	mu_init((mu_Context *)ctx);
	((mu_Context *)ctx)->text_width = text_width;
	((mu_Context *)ctx)->text_height = text_height;

	ecs_world_t *world = ecs_init();
	ECS_IMPORT(world, Can);
	ECS_IMPORT(world, GuiCan);
	// https://www.flecs.dev/explorer/?remote=true
	ecs_set(world, EcsWorld, EcsRest, {.port = 0});

	// ecs_singleton_set(world, GuiContext, {ctx});

	ecs_log_set_level(1);
	ecs_plecs_from_file(world, "config/signals.flecs");
	ecs_log_set_level(-1);

	// clang-format off
	ecs_query_t *q = ecs_query(world, {
		.filter.terms = {
			{.id = ecs_id(GuiSlider)},
			{.id = ecs_id(CanSignal)}
		},
		.order_by = compare_entity
		}
	);
	// clang-format on

	while (1) {
		ecs_os_sleep(0.0f, 100000.0f);
		// printf("ecs_progress\n");
		ecs_progress(world, 0.0f);

		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			mu_backend_events((mu_Context *)ctx, &e);
		}
		gui_can_progress((mu_Context *)ctx, world, q);
		// demo_window_progress((mu_Context*)ctx);

		mu_backend_render((mu_Context *)ctx);
	}

	return 0;
}
