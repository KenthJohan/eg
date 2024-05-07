#include "gui_signals.h"

#include <egquantities.h>
#include <egcan.h>

#include <egcolors/eg_color.h>

#define COLOR_RGBA(r,g,b,a) ((r) << 0 | (g) << 8 | (b) << 16 | (a) << 24)

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>

#include <stdlib.h>

#include "gui_plot.h"
#include "GuiCan.h"


void gui_plot_progress(ecs_world_t *world, ecs_query_t *q)
{
	assert(world);
	assert(q);
	ecs_iter_t it = ecs_query_iter(world, q);
	while (ecs_query_next(&it)) {
		EgCanSignal *s = ecs_field(&it, EgCanSignal, 1);
		GuiCanPlot *plot = ecs_field(&it, GuiCanPlot, 2);
		for (int i = 0; i < it.count; ++i, ++plot, ++s) {
			char const * name = ecs_get_name(world, it.entities[i]);
			igPlotLines_FloatPtr(name, plot->v.array, plot->v.count, 0, NULL, -255, 255, (ImVec2){0,200}, sizeof(float));
			igSeparator();
			if (plot->v.count > 1000) {
				plot->v.count = 0;
			}
		}
	}
}

ecs_query_t *gui_plot_query(ecs_world_t *world)
{
	// clang-format off
	ecs_query_t * q = ecs_query(world, {
		.filter.terms = {
			{.id = ecs_id(EgCanSignal), .src.flags = EcsSelf},
			{.id = ecs_id(GuiCanPlot), .src.flags = EcsSelf},
		}}
	);
	// clang-format on
	return q;
}