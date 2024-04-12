#include "gui_can.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "GuiCan.h"

static char logbuf[64000];
static int logbuf_updated = 0;

static void write_log(const char *text)
{
	if (logbuf[0]) {
		strcat(logbuf, "\n");
	}
	strcat(logbuf, text);
	logbuf_updated = 1;
}

static int uint8_slider(mu_Context *ctx, unsigned char *value, int low, int high)
{
	static float tmp;
	mu_push_id(ctx, &value, sizeof(value));
	tmp = *value;
	int res = mu_slider_ex(ctx, &tmp, low, high, 0, "%.0f", MU_OPT_ALIGNCENTER);
	*value = tmp;
	mu_pop_id(ctx);
	return res;
}





void gui_can_progress1(mu_Context *ctx, ecs_world_t *world, ecs_query_t *q)
{
	char buf[128];
	mu_layout_row(ctx, 2, (int[]){80, -1}, 0);
	ecs_iter_t it = ecs_query_iter(world, q);
	while (ecs_query_next(&it)) {
		GuiSlider *p = ecs_field(&it, GuiSlider, 1);
		CanSignal *c = ecs_field(&it, CanSignal, 2);
		for (int i = 0; i < it.count; ++i, ++p, ++c) {
			ecs_entity_t e = it.entities[i];
			char const * name = ecs_get_name(world, e);
			snprintf(buf, sizeof(buf), "%s popup", name);
			if (mu_button(ctx, name)) {
				if(ecs_has(world, e, GuiCanSignalInfo)) {
					ecs_remove(world, e, GuiCanSignalInfo);
				} else {
					ecs_set(world, e, GuiCanSignalInfo, {1});
				}
			}

			//mu_label(ctx, name);
			//char buf[128];
			//snprintf(buf, sizeof(buf), "%02X", c->canid);
			//mu_label(ctx, buf);
			uint8_slider(ctx, &p->value, 0, 255);
			//mu_draw_rect(ctx, mu_layout_next(ctx), ctx->style->colors[i]);
			/*
			uint8_slider(ctx, &ctx->style->colors[i].r, 0, 255);
			uint8_slider(ctx, &ctx->style->colors[i].g, 0, 255);
			uint8_slider(ctx, &ctx->style->colors[i].b, 0, 255);
			uint8_slider(ctx, &ctx->style->colors[i].a, 0, 255);
			mu_draw_rect(ctx, mu_layout_next(ctx), ctx->style->colors[i]);
			*/
		}
	}
}

void gui_can_progress2(mu_Context *ctx, ecs_world_t *world, ecs_query_t *q)
{
	char buf[128];
	mu_layout_row(ctx, 2, (int[]){80, -1}, 0);
	ecs_iter_t it = ecs_query_iter(world, q);
	while (ecs_query_next(&it)) {
		GuiSlider *p = ecs_field(&it, GuiSlider, 1);
		CanSignal *c = ecs_field(&it, CanSignal, 2);
		for (int i = 0; i < it.count; ++i, ++p, ++c) {

		}
	}
}













void gui_can_progress(mu_Context *ctx, ecs_world_t *world, ecs_query_t *q)
{
	mu_begin(ctx);
	/* do window */
	if (mu_begin_window_ex(ctx, "Demo Window", mu_rect(40, 40, 300, 450), MU_OPT_EXPANDED)) {
		mu_Container *win = mu_get_current_container(ctx);
		win->rect.w = mu_max(win->rect.w, 240);
		win->rect.h = mu_max(win->rect.h, 300);

		/* window info */
		if (mu_header(ctx, "Window Info")) {
			int sw = mu_get_current_container(ctx)->body.w * 0.14;

			gui_can_progress1(ctx, world, q);

			//uint8_slider(ctx, &ctx->style->colors[0].r, 0, 255);
			//mu_draw_rect(ctx, mu_layout_next(ctx), ctx->style->colors[0]);
		}
		mu_end_window(ctx);
	}
	mu_end(ctx);
}