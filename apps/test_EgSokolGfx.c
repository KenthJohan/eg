#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "flecs.h"
#include "EgGeometries.h"
#include "EgWindows.h"
#include "EgEvents.h"
#include "EgQuantities.h"
#include "EgCamera.h"
#include "EgResources.h"
#include "eg_basics.h"


#if 1
#include "EgSdl.h"
#include "EgSokolGfx.h"
#include "EgSokolFetch.h"
#include "EgSokolDtx.h"
#endif





void myabort()
{
	abort();
}

// https://www.flecs.dev/explorer/?remote=true
int main(int argc, char *argv[])
{
	ecs_os_set_api_defaults();
	ecs_os_api_t os_api = ecs_os_api;
	os_api.abort_ = myabort;
	ecs_os_set_api(&os_api);

	ecs_log_set_level(0);
	ecs_world_t *world = ecs_init_w_args(argc, argv);
	ecs_singleton_set(world, EcsRest, {0});
	ECS_IMPORT(world, FlecsMonitor);
	ECS_IMPORT(world, FlecsUnits);

	ECS_IMPORT(world, EgWindows);
	ECS_IMPORT(world, EgSdl);
	ECS_IMPORT(world, EgSokolGfx);
	ECS_IMPORT(world, EgSokolFetch);
	ECS_IMPORT(world, EgSokolDtx);
	ECS_IMPORT(world, EgGeometries);
	ECS_IMPORT(world, EgQuantities);
	ECS_IMPORT(world, EgCamera);
	ECS_IMPORT(world, EgResources);
	ECS_IMPORT(world, EgCamera);

	ecs_singleton_set(world, EgSokolFetchConfig, {.max_requests = 3, .num_channels = 1, .num_lanes = 1});



	/*
	{
		ecs_entity_t p = ecs_new(world, 0);
		ecs_set_name(world, p, "Picture1");
		ecs_set(world, p, EgPath, {"../eg/baboon1.png"});
		ecs_add(world, p, EgImage);
		ecs_add(world, p, EgUpdate);
	}
	*/

	{
		//ecs_entity_t p = ecs_new(world, 0);
		//ecs_set_name(world, p, "Picture2");
		//ecs_set(world, p, EgText, {"Hello"});
		//ecs_add(world, p, EgImage);
		//ecs_add(world, p, EgUpdate);
	}

	{
		ecs_entity_t window1 = ecs_new(world, 0);
		ecs_set_name(world, window1, "Window1");
		ecs_set(world, window1, EgRectangleI32, {800, 800});
		ecs_set(world, window1, EgWindow, {EG_WINDOW_OPENGL|EG_WINDOW_RESIZABLE, 0, false});
		ecs_set(world, window1, EgTitle, {"Window1"});
		ecs_set_pair(world, window1, EgSokolGfxConfig, EgUpdate, {});
		ecs_set_pair(world, window1, EgSokolDtxConfig, EgUpdate, {.context_pool_size = 1});
		ecs_add_pair(world, window1, EgOpenGLContext, EgUpdate);

		ecs_entity_t cam = ecs_new_w_pair(world, EcsChildOf, window1);
		ecs_set_name(world, cam, "Cam1");
		ecs_add(world, cam, EgCamera3D);
		ecs_set(world, cam, EgCamera3DKeyBindings, {
		{EG_KEY_W, EG_KEY_S},
		{EG_KEY_A, EG_KEY_D},
		{EG_KEY_SPACE, EG_KEY_LCTRL},
		{EG_KEY_LEFT, EG_KEY_RIGHT},
		{EG_KEY_UP, EG_KEY_DOWN},
		{EG_KEY_Q, EG_KEY_E}
		});
		ecs_set(world, cam, EgVelocity3F32, {0.0f, 0.0f, 0.0f});
		ecs_set(world, cam, EgPosition3F32, {0.0f, 0.0f, 0.0f});
		ecs_set(world, cam, EgQuaternionF32, {1.0f, 0.0f, 0.0f, 0.0f});
	}

	/*
	ecs_entity_t window2 = ecs_new(world, 0);
	ecs_set_name(world, window2, "Window2");
	ecs_set(world, window2, EgRectangleI32, {800, 800});
	ecs_set(world, window2, EgWindow, {EG_WINDOW_OPENGL|EG_WINDOW_RESIZABLE, 0, false});
	ecs_set(world, window2, EgTitle, {"Window2"});
	*/

	while (1)
	{
		ecs_os_sleep(0,100000);
		ecs_progress(world, 0);

	}

	return 0;
}
