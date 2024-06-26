#include "GuiCan.h"

#include <egcan.h>
#include <egquantities.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <assert.h>

ECS_COMPONENT_DECLARE(GuiCanTableRow);
ECS_COMPONENT_DECLARE(GuiCanPlot);
ECS_TAG_DECLARE(GuiCanSelected);

ECS_CTOR(GuiCanPlot, ptr, {
	printf("GuiCanPlot CTOR\n");
	ecs_vec_init_t(NULL, &ptr->v, ecs_f32_t, 0);
})

ECS_DTOR(GuiCanPlot, ptr, {
	printf("GuiCanPlot DTOR\n");
	ecs_vec_fini_t(NULL, &ptr->v, ecs_f32_t);
})

ECS_COPY(GuiCanPlot, dst, src, {
	printf("GuiCanPlot COPY\n");
	dst->v = ecs_vec_copy_shrink_t(NULL, &src->v, ecs_f32_t);
})

ECS_MOVE(GuiCanPlot, dst, src, {
	printf("GuiCanPlot MOVE\n");
	ecs_vec_fini_t(NULL, &dst->v, ecs_f32_t);
	*dst = *src;
	ecs_os_memset_t(src, 0, GuiCanPlot);
})

static void System_GuiCanPlot(ecs_iter_t *it)
{
	GuiCanPlot *d = ecs_field(it, GuiCanPlot, 1);                               // self
	EgCanId *c = ecs_field(it, EgCanId, 2);                                     // self
	EgCanSignal *s = ecs_field(it, EgCanSignal, 3);                             // self
	EgQuantitiesRangedGeneric *v = ecs_field(it, EgQuantitiesRangedGeneric, 4); // self
	for (int i = 0; i < it->count; ++i, ++d, ++c, ++s, ++v) {
		if (c->n != d->last_index) {
			// TODO: Support all types:
			ecs_vec_append_t(NULL, &d->v, ecs_f32_t)[0] = v->rx.val_f32;
			d->last_index = c->n;
			d->min = v->min.val_f32;
			d->max = v->max.val_f32;
		}
	}
}

void GuiCanImport(ecs_world_t *world)
{
	ECS_MODULE(world, GuiCan);
	ECS_IMPORT(world, EgCan);
	ecs_set_name_prefix(world, "GuiCan");

	ECS_COMPONENT_DEFINE(world, GuiCanTableRow);
	ECS_COMPONENT_DEFINE(world, GuiCanPlot);
	ECS_TAG_DEFINE(world, GuiCanSelected);

	ecs_struct(world,
	{.entity = ecs_id(GuiCanTableRow),
	.members = {
	{.name = "index", .type = ecs_id(ecs_i32_t)},
	{.name = "kind", .type = ecs_id(ecs_entity_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(GuiCanPlot),
	.members = {
	{.name = "v_array", .type = ecs_id(ecs_uptr_t)},
	{.name = "v_count", .type = ecs_id(ecs_i32_t)},
	{.name = "v_size", .type = ecs_id(ecs_i32_t)},
#ifdef FLECS_SANITIZE
	{.name = "v_elem_size", .type = ecs_id(ecs_size_t)},
#endif
	{.name = "last_index", .type = ecs_id(ecs_u32_t)},
	{.name = "min", .type = ecs_id(ecs_f32_t)},
	{.name = "min", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_set_hooks(world, GuiCanPlot, {.ctor = ecs_ctor(GuiCanPlot), .move = ecs_move(GuiCanPlot), .copy = ecs_copy(GuiCanPlot), .dtor = ecs_dtor(GuiCanPlot)});

	// clang-format off
	ecs_system(world, {.entity = ecs_entity(world, {.name = "System_GuiCanPlot",
		.add = {ecs_dependson(EcsOnUpdate)}}),
		.query.filter.terms = {
		{.id = ecs_id(GuiCanPlot), .src.flags = EcsSelf},
		{.id = ecs_id(EgCanId), .src.flags = EcsSelf},
		{.id = ecs_id(EgCanSignal), .src.flags = EcsSelf},
		{.id = ecs_id(EgQuantitiesRangedGeneric), .src.flags = EcsSelf},
		},
		.callback = System_GuiCanPlot});
	// clang-format on

	/*

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
	*/
}