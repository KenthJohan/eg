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

typedef struct
{
	ecs_i32_t capacity;
	ecs_i32_t count;
	SDL_Vertex * v;
} Eg_SDL_Mesh;


void Eg_SDL_Mesh_push(Eg_SDL_Mesh * m, SDL_Vertex * v, ecs_i32_t count)
{
	EG_ASSERT(m);
	EG_ASSERT(v);
	EG_ASSERT(m->v);
	ecs_i32_t left = m->capacity - m->count;
	ecs_i32_t n = ECS_MIN(left, count);
	if (n > 0)
	{
		ecs_os_memcpy(m->v + m->count, v, n * sizeof(SDL_Vertex));
		m->count += n;
	}
}


ECS_COMPONENT_DECLARE(Eg_SDL_Window);
ECS_COMPONENT_DECLARE(Eg_SDL_Mesh);



void Eg_SDL_Mesh_CTOR(Eg_SDL_Mesh * ptr)
{
	ecs_trace("Eg_SDL_Mesh::ECS_CTOR");
	ptr->v = NULL;
	ptr->count = 0;
	ptr->capacity = 0;
}

void Eg_SDL_Mesh_DTOR(Eg_SDL_Mesh * ptr)
{
	ecs_trace("Eg_SDL_Mesh::ECS_DTOR");
	if(ptr->v){ecs_os_free(ptr->v);}
	ptr->v = NULL;
	ptr->count = 0;
	ptr->capacity = 0;
}

void Eg_SDL_Mesh_MOVE(Eg_SDL_Mesh * dst, Eg_SDL_Mesh * src)
{
	ecs_trace("Eg_SDL_Mesh::ECS_MOVE");
	EG_ASSERT(dst);
	EG_ASSERT(src);
	if(dst->v){ecs_os_free(dst->v);}
	dst->v = src->v;
	dst->count = src->count;
	dst->capacity = src->capacity;
	src->v = NULL;
	src->count = 0;
	src->capacity = 0;
}

void Eg_SDL_Mesh_COPY(Eg_SDL_Mesh * dst, Eg_SDL_Mesh * src)
{
	ecs_trace("Eg_SDL_Mesh::ECS_COPY");
	EG_ASSERT(dst);
	EG_ASSERT(src);
	if(dst->v){ecs_os_free(dst->v);}
	if(src->v)
	{
		int32_t size = src->capacity * sizeof(SDL_Vertex);
		dst->v = ecs_os_calloc(size);
		ecs_os_memcpy(dst->v, src->v, size);
		dst->count = src->count;
		dst->capacity = src->capacity;
	}
	else
	{
        dst->v = NULL;
        dst->count = 0;
        dst->capacity = 0;
    }
}

ECS_CTOR(Eg_SDL_Mesh, ptr, {Eg_SDL_Mesh_CTOR(ptr);});
ECS_DTOR(Eg_SDL_Mesh, ptr, {Eg_SDL_Mesh_DTOR(ptr);});
ECS_MOVE(Eg_SDL_Mesh, dst, src, {Eg_SDL_Mesh_MOVE(dst, src);})
ECS_COPY(Eg_SDL_Mesh, dst, src, {Eg_SDL_Mesh_COPY(dst, src);})



















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
		Eg_SDL_Mesh m;
		m.capacity = 100;
		m.count = 0;
		m.v = ecs_os_calloc(m.capacity * sizeof(SDL_Vertex));
		ecs_set(it->world, it->entities[i], Eg_SDL_Mesh, {m.capacity, m.count, m.v});
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
    Eg_SDL_Window *w = ecs_term(it, Eg_SDL_Window, 1); // Parent
    Eg_SDL_Mesh *m = ecs_term(it, Eg_SDL_Mesh, 2); // Parent
    EgDraw *d = ecs_term(it, EgDraw, 3);
    EgRectangleF32 *r = ecs_term(it, EgRectangleF32, 4);
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
		
		Eg_SDL_Mesh_push(m+0, vert, 6);
	

/*
		SDL_Renderer *renderer = w[0].renderer;
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_RenderGeometry(renderer, NULL, vert, 6, NULL, 0);
		SDL_RenderPresent(renderer);
		*/

    }
}



static void Mesh(ecs_iter_t *it)
{
    Eg_SDL_Window *w = ecs_term(it, Eg_SDL_Window, 1); //Parent
    Eg_SDL_Mesh *m = ecs_term(it, Eg_SDL_Mesh, 2);
    for (int i = 0; i < it->count; i ++)
    {
		SDL_Renderer *renderer = w[0].renderer;
		SDL_Vertex * v = m[i].v;
		EG_ASSERT(renderer);
		EG_ASSERT(v);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_RenderGeometry(renderer, NULL, v, m[i].count, NULL, 0);
		SDL_RenderPresent(renderer);
		m[i].count = 0;
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
	ECS_COMPONENT_DEFINE(world, Eg_SDL_Mesh);

	ecs_set_component_actions(world, Eg_SDL_Mesh, {
	//.ctor = ecs_ctor(Eg_SDL_Mesh),
	ecs_default_ctor,
	.dtor = ecs_dtor(Eg_SDL_Mesh),
	.copy = ecs_copy(Eg_SDL_Mesh),
	.move = ecs_move(Eg_SDL_Mesh),
	});

    ECS_OBSERVER(world, Create_Window, EcsOnSet, EgWindow, EgRectangleI32);
	ECS_TRIGGER(world, Destroy_Window, EcsOnRemove, Eg_SDL_Window);
	ECS_SYSTEM(world, Update_Window, EcsOnUpdate, Eg_SDL_Window, EgWindow);
	ECS_SYSTEM(world, Draw, EcsOnUpdate, Eg_SDL_Window(parent), Eg_SDL_Mesh(parent), EgDraw, EgRectangleF32);
	ECS_SYSTEM(world, Mesh, EcsOnUpdate, Eg_SDL_Window, Eg_SDL_Mesh);
	
}

