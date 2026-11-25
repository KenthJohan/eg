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

bool EgWindowsSdlGlCleanup(void *window, void *context, void (*destroy)(void *), void *resource)
{
	SDL_Window   *previous_window  = SDL_GL_GetCurrentWindow();
	SDL_GLContext previous_context = SDL_GL_GetCurrentContext();
	if (!SDL_GL_MakeCurrent(window, context)) {
		ecs_err("EgWindowsSdlGl: cannot make OpenGL context current: %s", SDL_GetError());
		return false;
	}
	destroy(resource);
	if (!SDL_GL_MakeCurrent(previous_window, previous_context)) {
		ecs_err("EgWindowsSdlGl: cannot restore OpenGL context: %s", SDL_GetError());
		return false;
	}
	return true;
}

static void System_Render(ecs_iter_t *it)
{
	EgWindowsWindow        *w = ecs_field_self(it, EgWindowsWindow, 0);
	EgWindowsOpenGLContext *g = ecs_field_self(it, EgWindowsOpenGLContext, 1);
	for (int i = 0; i < it->count; ++i, ++w, ++g) {
		ecs_entity_t e = it->entities[i];

		const EgWindowsFrameCallback *generic_callback              = ecs_get(it->world, e, EgWindowsFrameCallback);
		const EgWindowsFrameExecutor *executor                      = ecs_get(it->world, e, EgWindowsFrameExecutor);
		const EgWindowsNativeOwner *owner                            = ecs_get(it->world, e, EgWindowsNativeOwner);
		void (*generic_complete)(ecs_world_t *, ecs_entity_t, bool) = generic_callback ? generic_callback->complete : NULL;
		void (*generic_begin)(ecs_world_t *, ecs_entity_t)          = generic_callback ? generic_callback->begin : NULL;
		void (*generic_end)(ecs_world_t *, ecs_entity_t)            = generic_callback ? generic_callback->end : NULL;
		bool (*execute)(ecs_world_t *, ecs_entity_t)                = executor ? executor->execute : NULL;
		void (*complete)(ecs_world_t *, ecs_entity_t, bool)        = executor ? executor->complete : NULL;

		if (!w->object || !g->gl_context ||
		    (owner && (owner->window != w->object || owner->context != g->gl_context))) {
			ecs_err("EgWindowsSdlGl: invalid or replaced native window/context for entity %llu",
				(unsigned long long)e);
			if (complete)
				complete(it->world, e, false);
			if (generic_complete)
				generic_complete(it->world, e, false);
			continue;
		}
		if (!SDL_GL_MakeCurrent(w->object, g->gl_context)) {
			ecs_err("EgWindowsSdlGl: cannot make window context current: %s", SDL_GetError());
			if (complete)
				complete(it->world, e, false);
			if (generic_complete)
				generic_complete(it->world, e, false);
			continue;
		}
		if (generic_begin)
			generic_begin(it->world, e);

		bool rendered = !execute || execute(it->world, e);
		if (generic_end)
			generic_end(it->world, e);

		bool displayed = rendered && SDL_GL_SwapWindow(w->object);
		if (rendered && !displayed)
			ecs_err("EgWindowsSdlGl: cannot swap window: %s", SDL_GetError());
		if (complete)
			complete(it->world, e, displayed);
		if (generic_complete)
			generic_complete(it->world, e, displayed);
	} // END FOR LOOP
}

static void System_EgWindowsOpenGLContext_Create(ecs_iter_t *it)
{
	EgWindowsWindow              *w = ecs_field_self(it, EgWindowsWindow, 0);
	EgWindowsOpenGLContextCreate *g = ecs_field_self(it, EgWindowsOpenGLContextCreate, 1);

	(void)g; // Unused parameter

	for (int i = 0; i < it->count; ++i, ++w, ++g) {
		ecs_entity_t e = it->entities[i];

		SDL_GLContext context = w->object ? SDL_GL_CreateContext(w->object) : NULL;
		if (context == NULL) {
			ecs_err("SDL_GL_CreateContext() failed: %s", SDL_GetError());
			ecs_add(it->stage, it->entities[i], EcsAlertWarning);
			ecs_enable(it->stage, it->entities[i], false);
			continue;
		}
		GLint major = 0, minor = 0, profile = 0;
		glGetIntegerv(GL_MAJOR_VERSION, &major);
		glGetIntegerv(GL_MINOR_VERSION, &minor);
		glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile);
		if (major < 4 || (major == 4 && minor < 3) || !(profile & GL_CONTEXT_CORE_PROFILE_BIT)) {
			ecs_err("EgWindowsSdlGl: OpenGL 4.3 core required, got %d.%d (profile mask 0x%x)",
			major, minor, (unsigned)profile);
			SDL_GL_DestroyContext(context);
			ecs_add(it->stage, e, EcsAlertWarning);
			ecs_enable(it->stage, e, false);
			continue;
		}
		char const *gl_version   = (char const *)glGetString(GL_VERSION);
		char const *glsl_version = (char const *)glGetString(GL_SHADING_LANGUAGE_VERSION);
		EgWindowsNativeOwner *owner = ecs_get_mut(it->world, e, EgWindowsNativeOwner);
		if (!owner) {
			ecs_set(it->stage, e, EgWindowsNativeOwner,
				{.window = w->object, .context = context});
		} else if ((!owner->window && !owner->owns_window && !owner->context)) {
			owner->window = w->object;
			owner->context = context;
		} else if (owner->window != w->object || owner->context) {
			ecs_err("EgWindowsSdlGl: invalid native ownership while creating context");
			SDL_GL_DestroyContext(context);
			ecs_enable(it->stage, e, false);
			continue;
		} else {
			owner->context = context;
		}

		ecs_set(it->stage, e, EgWindowsOpenGLContext,
		{
		.gl_context    = context,
		.gl_version    = gl_version,
		.glsl_version  = glsl_version,
		.cleanup       = EgWindowsSdlGlCleanup,
		});
	} // END FOR LOOP
}

void EgWindowsSdlGlMakeCurrent(EgWindowsWindow *window, EgWindowsOpenGLContext *context)
{
	SDL_GL_MakeCurrent(window->object, context->gl_context);
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
