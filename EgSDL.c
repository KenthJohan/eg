#include "EgSDL.h"
#include "EgSDLW.h"
#include "EgGeometries.h"
#include "EgWindows.h"
#include "EgBasics.h"
#include "EgEvents.h"
#include "EgQuantities.h"
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
	Eg_SDL_Window * win = ecs_get_mut(world, e, Eg_SDL_Window, NULL);
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








static void System_Update_Title(ecs_iter_t *it)
{
	EG_ITER_INFO(it);
	Eg_SDL_Window *w = ecs_term(it, Eg_SDL_Window, 1);
	EgTitle *title = ecs_term(it, EgTitle, 2);
	for (int i = 0; i < it->count; i ++)
	{
		if(w[i].window)
		{
			SDL_SetWindowTitle(w[i].window, title[i].value);
		}
	}
}










static void System_Change_Window_Size(ecs_iter_t *it)
{
	EG_ITER_INFO(it);
	Eg_SDL_Window *w = ecs_term(it, Eg_SDL_Window, 1);
	EgRectangleI32 *r = ecs_term(it, EgRectangleI32, 2);
	for (int i = 0; i < it->count; i ++)
	{
		//https://wiki.libsdl.org/SDL_SetWindowSize
		if (w[i].window)
		{
			SDL_SetWindowSize(w[i].window, r[i].width, r[i].height);
		}
	}
}


static void System_Update_Window_Size(ecs_iter_t *it)
{
	//EG_ITER_INFO(it);
	Eg_SDL_Window *w = ecs_term(it, Eg_SDL_Window, 1);
	EgRectangleI32 *r = ecs_term(it, EgRectangleI32, 2);
	for (int i = 0; i < it->count; i ++)
	{
		//https://wiki.libsdl.org/SDL_SetWindowSize
		if (w[i].window)
		{
			int width;
			int height;
			SDL_GetWindowSize(w[i].window, &width, &height);
			r[i].width = width;
			r[i].height = height;
		}
	}
}





void EgSDLImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgSDL);
	ECS_IMPORT(world, EgSDLW);
	ECS_IMPORT(world, EgGeometries);
	ECS_IMPORT(world, EgWindows);
	ECS_IMPORT(world, EgEvents);
	ECS_IMPORT(world, EgQuantities);
	ecs_set_name_prefix(world, "Eg");


	SDL_Init(SDL_INIT_VIDEO);





	//ECS_TRIGGER(world, System_Destroy_Window, EcsOnRemove, Eg_SDL_Window);




	ecs_observer_init(world, &(ecs_observer_desc_t) {
	.filter.terms = {
	{ .id = ecs_id(Eg_SDL_Window), .inout = EcsOut },
	{ .id = ecs_id(EgRectangleI32), .inout = EcsIn}
	},
	.events = {EcsOnSet},
	.callback = System_Change_Window_Size
	});

	ecs_observer_init(world, &(ecs_observer_desc_t) {
	.filter.terms = {
	{ .id = ecs_id(Eg_SDL_Window), .inout = EcsOut },
	{ .id = ecs_id(EgTitle), .inout = EcsIn}
	},
	.events = {EcsOnSet},
	.callback = System_Update_Title
	});



	ecs_system_init(world, &(ecs_system_desc_t) {
	.query.filter.terms = {
	{ .id = ecs_id(Eg_SDL_Window), .inout = EcsIn },
	{ .id = ecs_id(EgRectangleI32), .inout = EcsOut}
	},
	.entity.add = {EcsOnUpdate},
	.callback = System_Update_Window_Size
	});




	/*
	ECS_OBSERVER(world, System_Create_Window, EcsOnAdd,
	[inout] EgWindow,
	[in]    EgRectangleI32);
	ECS_OBSERVER(world, System_Change_Window_Size, EcsOnSet,
	[out] Eg_SDL_Window,
	[in]  EgRectangleI32);
	ECS_OBSERVER(world, System_Update_Title, EcsOnSet,
	[out] Eg_SDL_Window,
	[in]  EgTitle);
	ECS_SYSTEM(world, System_Update_Window, EcsOnUpdate,
	[inout] Eg_SDL_Window,
	[inout] EgWindow);
	ECS_SYSTEM(world, System_Update_Window_Size, EcsOnUpdate,
	[in]  Eg_SDL_Window,
	[out] EgRectangleI32);
	ECS_SYSTEM(world, System_Update_UserEvent, EcsOnUpdate,
	[out] EgUserEvent($));
	*/



	
}

