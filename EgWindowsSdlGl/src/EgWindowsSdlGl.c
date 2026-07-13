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

static void System_EgWindowsOpenGLContext_Create(ecs_iter_t *it)
{
	EgWindowsWindow *cw = ecs_field(it, EgWindowsWindow, 0);                             // self, in
	EgWindowsOpenGLContextCreate *info = ecs_field(it, EgWindowsOpenGLContextCreate, 1); // self, in
	for (int i = 0; i < it->count; ++i) {
		SDL_GLContext context = SDL_GL_CreateContext(cw[i].object);
		char const *gl_version = (char const *)glGetString(GL_VERSION);
		char const *glsl_version = (char const *)glGetString(GL_SHADING_LANGUAGE_VERSION);
		ecs_set(it->world, it->entities[i], EgWindowsOpenGLContext, {.gl_context = context, .gl_version = gl_version, .glsl_version = glsl_version});
	} // END FOR LOOP
}

void EgWindowsSdlGlImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgWindowsSdlGl);
	ECS_IMPORT(world, EgWindows);
	ECS_IMPORT(world, EgKeyboards);
	ecs_set_name_prefix(world, "EgWindowsSdlGl");

	ecs_system(world,
	{.entity = ecs_entity(world, {.name = "System_Render", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_Render,
	.query.terms =
	{
	{.id = ecs_id(EgWindowsWindow), .src.id = EcsSelf},
	{.id = ecs_id(EgWindowsOpenGLContext), .src.id = EcsSelf},
	}});

	ecs_system(world,
	{.entity = ecs_entity(world, {.name = "System_EgWindowsOpenGLContext_Create", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_EgWindowsOpenGLContext_Create,
	.query.terms =
	{
	{.id = ecs_id(EgWindowsWindow), .src.id = EcsSelf},
	{.id = ecs_id(EgWindowsOpenGLContextCreate), .src.id = EcsSelf},
	{.id = ecs_id(EgWindowsOpenGLContext), .oper = EcsNot}, // Adds this
	}});
}
