#include "eg_sdl.h"
#include "eg_geometry.h"
#include "eg_window.h"
#include "eg_base.h"
#include <SDL2/SDL.h>



typedef struct
{
	SDL_Window * window;
	SDL_Renderer *renderer;
	ecs_u32_t elapsed_milliseconds;
} Eg_SDL_Window;
ECS_COMPONENT_DECLARE(Eg_SDL_Window);









static void Create_Window(ecs_iter_t *it)
{
    EgWindow *w = ecs_term(it, EgWindow, 1);
    EgRectangleI32 *r = ecs_term(it, EgRectangleI32, 2);
    for (int i = 0; i < it->count; i ++)
    {
		char title[128];
		if (w[i].title) {snprintf(title, 128, "%", title);}
		else {snprintf(title, 128, "Undefined title %s:%i", __FILE__, __LINE__);}
		ecs_trace("Creating SDL Window 0x%x : %s", it->entities[i], title);
		// https://wiki.libsdl.org/SDL_CreateWindow
		SDL_Window * window = SDL_CreateWindow(
			title,
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			r[i].width,
			r[i].height,
			SDL_WINDOW_OPENGL
		);
		EG_ASSERT(window);
		SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		EG_ASSERT(renderer);
		ecs_set(it->world, it->entities[i], Eg_SDL_Window, {window, renderer, 0});
    }
}


static void Destroy_Window(ecs_iter_t *it)
{
    Eg_SDL_Window *w = ecs_term(it, Eg_SDL_Window, 1);
    for (int i = 0; i < it->count; i ++)
    {
		SDL_Window * window = w[i].window;
		// https://wiki.libsdl.org/SDL_GetWindowTitle
		char const * title = SDL_GetWindowTitle(window);
		ecs_trace("Removing SDL Window 0x%x : %s", it->entities[i], title);
		SDL_DestroyWindow(window);
    }
}


static void Update_Window(ecs_iter_t *it)
{
    Eg_SDL_Window *s = ecs_term(it, Eg_SDL_Window, 1);
    EgWindow *w = ecs_term(it, EgWindow, 2);
    for (int i = 0; i < it->count; i ++)
    {
		uint64_t * userinput = w[i].userinput;
		// https://wiki.libsdl.org/SDL_GetTicks
		s[i].elapsed_milliseconds = SDL_GetTicks();
		if(s[i].elapsed_milliseconds > 1000*10)
		{
			//ecs_remove(it->world, it->entities[i], Eg_SDL_Window);
			ecs_delete(it->world, it->entities[i]);
		}
		
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
			case SDL_KEYDOWN:
				if(event.key.keysym.sym < (64*4)){EG_USRINPUT_ADD(userinput, event.key.keysym.sym);}
				break;
			case SDL_KEYUP:
				if(event.key.keysym.sym < (64*4)){EG_USRINPUT_DEL(userinput, event.key.keysym.sym);}
				break;
			}
		}
		
		if(EG_USRINPUT_GET(userinput, SDLK_ESCAPE))
		{
			ecs_delete(it->world, it->entities[i]);
		}
		

    }
}


static void Draw(ecs_iter_t *it)
{
    Eg_SDL_Window *w = ecs_term(it, Eg_SDL_Window, 1); //Parent
    EgDraw *d = ecs_term(it, EgDraw, 2);
    EgRectangleF32 *r = ecs_term(it, EgRectangleF32, 3);
    for (int i = 0; i < it->count; i ++)
    {
		SDL_Vertex vert[6];
		
		
		vert[0].position.x = 0;
		vert[0].position.y = 0;
		vert[0].color.r = 255;
		vert[0].color.g = 0;
		vert[0].color.b = 0;
		vert[0].color.a = 255;
		vert[1].position.x = 0;
		vert[1].position.y = r[i].height;
		vert[1].color.r = 0;
		vert[1].color.g = 0;
		vert[1].color.b = 255;
		vert[1].color.a = 255;
		vert[2].position.x = r[i].width;
		vert[2].position.y = r[i].height;
		vert[2].color.r = 0;
		vert[2].color.g = 255;
		vert[2].color.b = 0;
		vert[2].color.a = 255;
		
		vert[3].position.x = 0;
		vert[3].position.y = 0;
		vert[3].color.r = 255;
		vert[3].color.g = 100;
		vert[3].color.b = 100;
		vert[3].color.a = 255;
		vert[4].position.x = r[i].width;
		vert[4].position.y = 0;
		vert[4].color.r = 100;
		vert[4].color.g = 100;
		vert[4].color.b = 255;
		vert[4].color.a = 255;
		vert[5].position.x = r[i].width;
		vert[5].position.y = r[i].height;
		vert[5].color.r = 100;
		vert[5].color.g = 255;
		vert[5].color.b = 100;
		vert[5].color.a = 255;
		
		SDL_Renderer *renderer = w[0].renderer;
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_RenderGeometry(renderer, NULL, vert, 6, NULL, 0);
		SDL_RenderPresent(renderer);
    }
}


void FlecsComponentsEgSDLImport(ecs_world_t *world)
{
	ECS_MODULE(world, FlecsComponentsEgSDL);
	ECS_IMPORT(world, FlecsComponentsEgGeometry);
	ECS_IMPORT(world, FlecsComponentsEgWindow);
	ecs_set_name_prefix(world, "Eg");
	
	SDL_Init(SDL_INIT_VIDEO);
	
	ECS_COMPONENT_DEFINE(world, Eg_SDL_Window);

    ECS_OBSERVER(world, Create_Window, EcsOnSet, EgWindow, EgRectangleI32);
	ECS_TRIGGER(world, Destroy_Window, EcsOnRemove, Eg_SDL_Window);
	ECS_SYSTEM(world, Update_Window, EcsOnUpdate, Eg_SDL_Window, EgWindow);
	ECS_SYSTEM(world, Draw, EcsOnUpdate, Eg_SDL_Window(parent), EgDraw, EgRectangleF32);
	
}

