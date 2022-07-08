#include "EgSdl.h"
#include "EgSdlw.h"
#include "EgGeometries.h"
#include "EgWindows.h"
#include "EgEvents.h"
#include "EgQuantities.h"
#include "eg_basics.h"
#include <SDL2/SDL.h>


void eg_gl_make_current(ecs_world_t * world, ecs_entity_t e)
{
	Eg_SDL_Window const * win = ecs_get(world, e, Eg_SDL_Window);
	EG_ASSERT(win);
	EG_ASSERT(win->window);
	EG_ASSERT(win->context);
	SDL_GL_MakeCurrent(win->window, win->context);
}

void eg_gl_create_context(ecs_world_t * world, ecs_entity_t e)
{
	Eg_SDL_Window * win = ecs_get_mut(world, e, Eg_SDL_Window);
	EG_ASSERT(win);
	EG_ASSERT(win->window);
	win->context = SDL_GL_CreateContext(win->window);
	EG_ASSERT(win->context);
}

void eg_gl_swap_buffer(ecs_world_t * world, ecs_entity_t e)
{
	Eg_SDL_Window const * win = ecs_get(world, e, Eg_SDL_Window);
	EG_ASSERT(win);
	EG_ASSERT(win->window);
	EG_ASSERT(win->context);
	SDL_GL_SwapWindow(win->window);
}




void EgSdlImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgSdl);
	ECS_IMPORT(world, EgSdlw);
	ECS_IMPORT(world, EgGeometries);
	ECS_IMPORT(world, EgWindows);
	ECS_IMPORT(world, EgEvents);
	ECS_IMPORT(world, EgQuantities);
	ecs_set_name_prefix(world, "Eg");
	SDL_Init(SDL_INIT_VIDEO);
}

