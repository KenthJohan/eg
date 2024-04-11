#include "GuiCan.h"

#include "eg/Components.h"


ECS_COMPONENT_DECLARE(GuiContext);
ECS_COMPONENT_DECLARE(GuiSlider);



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



static void System_GuiBegin(ecs_iter_t *it)
{
	GuiContext *ctx = ecs_field(it, GuiContext, 1);
	mu_begin(ctx->muctx);
}

static void System_GuiEnd(ecs_iter_t *it)
{
	GuiContext *ctx = ecs_field(it, GuiContext, 1);
	mu_end(ctx->muctx);
}

static void System_GuiSlider(ecs_iter_t *it)
{
	GuiContext *ctx = ecs_field(it, GuiContext, 1);
	GuiSlider *window = ecs_field(it, GuiSlider, 2);
	for (int i = 0; i < it->count; ++i) {

	}
}


void GuiCanImport(ecs_world_t *world)
{
	ECS_MODULE(world, GuiCan);
	ECS_IMPORT(world, Can);
	ECS_COMPONENT_DEFINE(world, GuiContext);
	ECS_COMPONENT_DEFINE(world, GuiSlider);

	ecs_struct(world,
	{.entity = ecs_id(GuiSlider),
	.members = {
	{.name = "value", .type = ecs_id(ecs_u8_t)},
	}});



	ecs_system(world, {
	.entity = ecs_entity(world, {
		.name = "System_GuiBegin",
		.add = { ecs_dependson(EcsOnUpdate) }
	}),
	.query.filter.terms = {
		{ .id = ecs_id(GuiContext), .src.id = ecs_id(GuiContext) }
	},
	.callback = System_GuiBegin
	});

	ecs_system(world, {
	.entity = ecs_entity(world, {
		.name = "System_GuiSlider",
		.add = { ecs_dependson(EcsOnUpdate) }
	}),
	.query.filter.terms = {
		{ .id = ecs_id(GuiContext), .src.id = ecs_id(GuiContext) },
		{ .id = ecs_id(GuiSlider) }
	},
	.callback = System_GuiSlider
	});

	ecs_system(world, {
	.entity = ecs_entity(world, {
		.name = "System_GuiEnd",
		.add = { ecs_dependson(EcsOnUpdate) }
	}),
	.query.filter.terms = {
		{ .id = ecs_id(GuiContext), .src.id = ecs_id(GuiContext) }
	},
	.callback = System_GuiEnd
	});

}