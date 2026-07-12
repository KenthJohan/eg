#include "EgWindowsSdlGl.h"

#include <stdlib.h>
#include <stdio.h>

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_opengl.h>

#include <EgShapes.h>
#include <EgSpatials.h>
#include <EgKeyboards.h>
#include <EgBase.h>
#include <EgWindows.h>

static void System_Render(ecs_iter_t *it)
{
	EgWindowsWindow *cw = ecs_field(it, EgWindowsWindow, 0);               // self, in
	EgWindowsOpenGLContext *gl = ecs_field(it, EgWindowsOpenGLContext, 1); // self, in
	for (int i = 0; i < it->count; ++i) {
		SDL_GL_MakeCurrent(cw[i].object, gl[i].gl_context);
		glClearColor(0.0f, 0.0f, 1.0f, 1.0f); // Blue
		SDL_GL_SwapWindow(cw[i].object);
	} // END FOR LOOP
}

void EgWindowsSdlGlImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgWindowsSdlGl);
	ECS_IMPORT(world, EgWindows);
	ECS_IMPORT(world, EgKeyboards);
	ecs_set_name_prefix(world, "EgWindowsSdlGl");

	ecs_system(world,
	{.entity = ecs_entity(world, {.name = "System_EgWindowsWindow_Create", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_Render,
	.query.terms =
	{
	{.id = ecs_id(EgWindowsWindow), .src.id = EcsSelf},
	{.id = ecs_id(EgWindowsOpenGLContext), .src.id = EcsSelf},
	}});
}
