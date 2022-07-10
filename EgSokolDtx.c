#include "EgSokolDtx.h"
#include "EgResources.h"
#include "EgWindows.h"
#include "EgQuantities.h"
#include "EgGeometries.h"
#include "eg_basics.h"
#include "sokol/sokol_fetch.h"
#include "libs/stb/stb_image.h"
#include "sokol_source.h"



ECS_DECLARE(EgSokolDtxDraw);
ECS_COMPONENT_DECLARE(EgSokolDtxConfig);


static void System_Work(ecs_iter_t *it)
{
	//EgWindow *window = ecs_term(it, EgWindow, 1);
	//EgSokolDtxConfig *config = ecs_term(it, EgSokolDtxConfig, 2);
}

static void System_Init(ecs_iter_t *it)
{
	//EG_ITER_INFO(it);
	EG_ASSERT(it->count == 1);
	EgWindow *window = ecs_term(it, EgWindow, 1);
	EgSokolDtxConfig *config = ecs_term(it, EgSokolDtxConfig, 2);
	for (int i = 0; i < it->count; i ++)
	{
		EG_TRACE("sdtx_setup");
		sdtx_setup(&(sdtx_desc_t)
		{
		.context_pool_size = config[i].context_pool_size,
		.fonts[0] = sdtx_font_oric()
		});
	}
}

static void System_Draw(ecs_iter_t *it)
{
	EgPosition2F32 const * p        = ecs_term(it, EgPosition2F32, 1);
	EgPosition2F32       * p_out    = ecs_term(it, EgPosition2F32, 2);
	EgPosition2F32 const * p_parent = ecs_term(it, EgPosition2F32, 3);
	for (int i = 0; i < it->count; i ++)
	{
		p_out[i].x = p[i].x;
		p_out[i].y = p[i].y;
		if (p_parent)
		{
			p_out[i].x += p_parent->x;
			p_out[i].y += p_parent->y;
		}
	}
}


void EgSokolDtxImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgSokolDtx);
	ECS_IMPORT(world, EgResources);
	ECS_IMPORT(world, EgQuantities);
	ecs_set_name_prefix(world, "Eg");

	ECS_COMPONENT_DEFINE(world, EgSokolDtxConfig);
	ECS_TAG_DEFINE(world, EgSokolDtxDraw);


	/*
	ecs_query_t *q = ecs_query_init(world, &(ecs_query_desc_t){
	.filter.terms = {
	// Read from entity's Local position
	{ .id = ecs_pair(ecs_id(EgPosition2F32), EgLocal), .inout = EcsIn },
	// Write to entity's World position
	{ .id = ecs_pair(ecs_id(EgPosition2F32), EgGlobal), .inout = EcsOut },

	// Read from parent's World position
	{
	.id = ecs_pair(ecs_id(EgPosition2F32), EgGlobal),
	.inout = EcsIn,
	// Get from the parent, in breadth-first order (cascade)
	.subj.set.mask = EcsParent | EcsCascade,
	// Make parent term optional so we also match the root (sun)
	.oper = EcsOptional
	},
	{EgSokolDtxDraw}
	}
	});
	*/


	ecs_system_init(world, &(ecs_system_desc_t) {
	.query.filter.expr = "EgWindow, EgSokolDtxConfig, EgSokolGfxConfig, EgGfx, (eg.windows.OpenGLContext, eg.resources.Valid)",
	.entity.add = {ecs_dependson(EcsOnUpdate)},
	.callback = System_Work
	});

	ecs_observer_init(world, &(ecs_observer_desc_t) {
	.filter.expr = "EgWindow, EgSokolDtxConfig, EgSokolGfxConfig, EgGfx, (eg.windows.OpenGLContext, eg.resources.Valid)",
	.events = {EcsOnSet},
	.callback = System_Init
	});

}
