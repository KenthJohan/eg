#include "EgWindowsSdlGl.h"

#include <stdlib.h>
#include <stdio.h>

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_version.h>

#include <EgShapes.h>
#include <EgSpatials.h>
#include <EgButtons.h>
#include <EgBase.h>
#include <EgWindows.h>
#include <ecsx.h>

static void inner_systems_run(ecs_iter_t *it)
{
	while (ecs_query_next(it)) {
		for (int i = 0; i < it->count; ++i) {
			ecs_entity_t e = it->entities[i];
			ecs_run(it->world, e, 0.0f, NULL);
		}
	}
}

static void System_Render(ecs_iter_t *it)
{
	EgWindowsWindow        *w = ecs_field_self(it, EgWindowsWindow, 0);
	EgWindowsOpenGLContext *g = ecs_field_self(it, EgWindowsOpenGLContext, 1);
	EgShapesRectangle      *r = ecs_field_self(it, EgShapesRectangle, 2);
	for (int i = 0; i < it->count; ++i, ++w, ++g, ++r) {
		ecs_entity_t e = it->entities[i];
		SDL_GL_MakeCurrent(w->object, g->gl_context);
		glViewport(0, 0, (GLsizei)r->w, (GLsizei)r->h);
		glClearColor(0.0f, 0.0f, 1.0f, 1.0f); // Blue
		glClear(GL_COLOR_BUFFER_BIT);

		ecs_iter_t it2 = ecs_query_iter(it->world, g->inner_systems);
		ecs_iter_set_group(&it2, e);
		inner_systems_run(&it2);

		SDL_GL_SwapWindow(w->object);
	} // END FOR LOOP
}

static void System_EgWindowsOpenGLContext_Create(ecs_iter_t *it)
{
	EgWindowsWindow              *w = ecs_field_self(it, EgWindowsWindow, 0);
	EgWindowsOpenGLContextCreate *g = ecs_field_self(it, EgWindowsOpenGLContextCreate, 1);

	(void)g; // Unused parameter

	for (int i = 0; i < it->count; ++i, ++w, ++g) {
		ecs_entity_t e = it->entities[i];

		SDL_GLContext context = SDL_GL_CreateContext(w->object);
		if (context == NULL) {
			ecs_err("SDL_GL_CreateContext() failed: %s", SDL_GetError());
			ecs_add(it->world, it->entities[i], EcsAlertWarning);
			ecs_enable(it->world, it->entities[i], false);
			continue;
		}
		char const *gl_version   = (char const *)glGetString(GL_VERSION);
		char const *glsl_version = (char const *)glGetString(GL_SHADING_LANGUAGE_VERSION);

		ecs_query_t *q = ecs_query(it->world,
		{
		.terms = {
		{.id = EcsSystem}},
		.group_by = EcsChildOf,
		});

		ecs_set(it->stage, e, EgWindowsOpenGLContext,
		{
		.gl_context    = context,
		.gl_version    = gl_version,
		.glsl_version  = glsl_version,
		.inner_systems = q,
		});
	} // END FOR LOOP
}

void EgWindowsSdlGlImport(ecs_world_t *world)
{
	ECS_IMPORT(world, EgWindows);
	ECS_IMPORT(world, EgButtons);
	ECS_IMPORT(world, FlecsAlerts);

	ECS_MODULE(world, EgWindowsSdlGl);
	ecs_set_name_prefix(world, "EgWindowsSdlGl");

	const int compiled = SDL_VERSION;
	const int linked   = SDL_GetVersion();
	ecs_log(-1, "EgWindowsSdlGl imported (compiled SDL version: %d.%d.%d, linked SDL version: %d.%d.%d)",
	SDL_VERSIONNUM_MAJOR(compiled), SDL_VERSIONNUM_MINOR(compiled), SDL_VERSIONNUM_MICRO(compiled),
	SDL_VERSIONNUM_MAJOR(linked), SDL_VERSIONNUM_MINOR(linked), SDL_VERSIONNUM_MICRO(linked));

	ecs_system(world,
	{.entity  = ecs_entity(world, {.name = "System_Render"}),
	.phase    = EcsPostFrame,
	.callback = System_Render,
	.query.terms =
	{
	{.id = ecs_id(EgWindowsWindow), .src.id = EcsSelf},
	{.id = ecs_id(EgWindowsOpenGLContext), .src.id = EcsSelf},
	{.id = ecs_id(EgShapesRectangle), .src.id = EcsSelf},
	}});

	ecs_system(world,
	{.entity     = ecs_entity(world, {.name = "System_EgWindowsOpenGLContext_Create"}),
	.phase       = EcsOnUpdate,
	.callback    = System_EgWindowsOpenGLContext_Create,
	.immediate   = true,
	.query.terms = {
	{.id = ecs_id(EgWindowsWindow), .src.id = EcsSelf},
	{.id = ecs_id(EgWindowsOpenGLContextCreate), .src.id = EcsSelf},
	{.id = ecs_id(EgWindowsOpenGLContext), .oper = EcsNot}, // Adds this
	}});
}
