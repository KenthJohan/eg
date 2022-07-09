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
	EG_ITER_INFO(it);
	EG_ASSERT(it->count == 1);
	EgWindow *window = ecs_term(it, EgWindow, 1);
	EgSokolDtxConfig *config = ecs_term(it, EgSokolDtxConfig, 2);
	for (int i = 0; i < it->count; i ++)
	{
		sdtx_setup(&(sdtx_desc_t)
		{
		.context_pool_size = config[i].context_pool_size,
		.fonts[0] = sdtx_font_oric()
		});
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



	ecs_system_init(world, &(ecs_system_desc_t) {
	.query.filter.expr = "EgWindow, EgSokolDtxConfig",
	.entity.add = {ecs_dependson(EcsOnUpdate)},
	.callback = System_Work
	});

	ecs_observer_init(world, &(ecs_observer_desc_t) {
	.filter.expr = "EgWindow, EgSokolDtxConfig",
	.events = {EcsOnSet},
	.callback = System_Init
	});

}
