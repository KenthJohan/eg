#include "eg_sdl.h"
#include "eg_geometry.h"
#include "eg_window.h"
#include "eg_base.h"
#include "eg_userevent.h"
#include "eg_quantity.h"
#include <SDL2/SDL.h>



typedef struct
{
	SDL_Window * window;
	SDL_Renderer *renderer;
	ecs_u32_t elapsed_milliseconds;
	const Uint8 *keys;
} Eg_SDL_Window;

typedef struct
{
	ecs_i32_t capacity;
	ecs_i32_t count;
	SDL_Vertex * v;
} Eg_SDL_Mesh;


static void Eg_SDL_Mesh_push(Eg_SDL_Mesh * m, SDL_Vertex * v, ecs_i32_t count)
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



static void Eg_SDL_Mesh_CTOR(Eg_SDL_Mesh * ptr)
{
	EG_TRACE("Eg_SDL_Mesh::ECS_CTOR");
	ptr->v = NULL;
	ptr->count = 0;
	ptr->capacity = 0;
}

static void Eg_SDL_Mesh_DTOR(Eg_SDL_Mesh * ptr)
{
	EG_TRACE("Eg_SDL_Mesh::ECS_DTOR");
	if(ptr->v){ecs_os_free(ptr->v);}
	ptr->v = NULL;
	ptr->count = 0;
	ptr->capacity = 0;
}

static void Eg_SDL_Mesh_MOVE(Eg_SDL_Mesh * dst, Eg_SDL_Mesh * src)
{
	EG_TRACE("Eg_SDL_Mesh::ECS_MOVE");
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

static void Eg_SDL_Mesh_COPY(Eg_SDL_Mesh * dst, Eg_SDL_Mesh * src)
{
	EG_TRACE("Eg_SDL_Mesh::ECS_COPY");
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









#define MAX_WINDOWS 16
static ecs_entity_t g_sdl_wents[MAX_WINDOWS];
static SDL_Window * g_sdl_windows[MAX_WINDOWS];








static void Create_Window(ecs_iter_t *it)
{
	EG_ITER_INFO(it);
	ecs_world_t * world = it->world;
	EgWindow *w = ecs_term(it, EgWindow, 1);
	EgRectangleI32 *r = ecs_term(it, EgRectangleI32, 2);
	for (int i = 0; i < it->count; i ++)
	{
		ecs_entity_t e = it->entities[i];
		w[i].counter = 0;
		EG_TRACE("SDL_CreateWindow 0x%x", e);
		// https://wiki.libsdl.org/SDL_CreateWindow
		SDL_Window * window = SDL_CreateWindow(
		"Undefined",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		r[i].width,
		r[i].height,
		SDL_WINDOW_OPENGL
		);
		int id = SDL_GetWindowID(window);
		g_sdl_wents[id] = e;
		g_sdl_windows[id] = window;
		EG_ASSERT(window);
		SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		EG_ASSERT(renderer);
		Uint8 const * keys = SDL_GetKeyboardState(NULL);
		ecs_set(world, e, Eg_SDL_Window, {window, renderer, 0, keys});
		Eg_SDL_Mesh m;
		m.capacity = 6*100;
		m.count = 0;
		m.v = ecs_os_calloc(m.capacity * sizeof(SDL_Vertex));
		ecs_set(world, e, Eg_SDL_Mesh, {m.capacity, m.count, m.v});
	}
}


static void Update_Title(ecs_iter_t *it)
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


static void Destroy_Window(ecs_iter_t *it)
{
	EG_ITER_INFO(it);
	Eg_SDL_Window *w = ecs_term(it, Eg_SDL_Window, 1);
	for (int i = 0; i < it->count; i ++)
	{
		ecs_entity_t e = it->entities[i];
		if (w[i].window)
		{
			// https://wiki.libsdl.org/SDL_GetWindowTitle
			char const * title = SDL_GetWindowTitle(w[i].window);
			EG_TRACE("SDL_DestroyWindow 0x%x : %s", e, title);
			SDL_DestroyWindow(w[i].window);
			w[i].window = NULL;
		}
	}
}


static void Update_Window(ecs_iter_t *it)
{
	Eg_SDL_Window *s = ecs_term(it, Eg_SDL_Window, 1);
	EgWindow *w = ecs_term(it, EgWindow, 2);
	for (int i = 0; i < it->count; i ++)
	{
		ecs_entity_t e = it->entities[i];
		w[i].counter++;
		// https://wiki.libsdl.org/SDL_GetTicks
		s[i].elapsed_milliseconds = SDL_GetTicks();
		if(s[i].elapsed_milliseconds > 1000*10)
		{
			//ecs_remove(it->world, it->entities[i], Eg_SDL_Window);
			//ecs_delete(it->world, it->entities[i]);
		}
		if (s[i].window == NULL)
		{
			EG_TRACE("window is null");
		}
		if (w[i].should_destroy)
		{
			EG_TRACE("should_destroy is true. Deleting entity 0x%016x", e);
			ecs_delete(it->world, e);
		}
	}
}


static void Vertex_Rect(SDL_Vertex v[6], EgPosition2F32 * p, EgRectangleF32 * r)
{
	v[0].position.x = p->x;
	v[0].position.y = p->y;
	v[1].position.x = p->x + r->width;
	v[1].position.y = p->y;
	v[2].position.x = p->x + r->width;
	v[2].position.y = p->y + r->height;

	v[3].position.x = p->x;
	v[3].position.y = p->y;
	v[4].position.x = p->x;
	v[4].position.y = p->y + r->height;
	v[5].position.x = p->x + r->width;
	v[5].position.y = p->y + r->height;
}


static void Draw_Rectangle(ecs_iter_t *it)
{
	Eg_SDL_Mesh *m = ecs_term(it, Eg_SDL_Mesh, 1); // Parent
	EgPosition2F32 *p = ecs_term(it, EgPosition2F32, 2);
	EgRectangleF32 *r = ecs_term(it, EgRectangleF32, 3);
	for (int i = 0; i < it->count; i ++)
	{
		SDL_Vertex vert[6];
		Vertex_Rect(vert, p + i, r + i);
		vert[0].color.r = 255;
		vert[0].color.g = 0;
		vert[0].color.b = 0;
		vert[0].color.a = 255;
		vert[1].color.r = 0;
		vert[1].color.g = 0;
		vert[1].color.b = 255;
		vert[1].color.a = 255;
		vert[2].color.r = 0;
		vert[2].color.g = 255;
		vert[2].color.b = 0;
		vert[2].color.a = 255;
		vert[3].color.r = 255;
		vert[3].color.g = 100;
		vert[3].color.b = 100;
		vert[3].color.a = 255;
		vert[4].color.r = 100;
		vert[4].color.g = 100;
		vert[4].color.b = 255;
		vert[4].color.a = 255;
		vert[5].color.r = 100;
		vert[5].color.g = 255;
		vert[5].color.b = 100;
		vert[5].color.a = 255;
		Eg_SDL_Mesh_push(m+0, vert, 6);
	}
}



static void Render_Mesh(ecs_iter_t *it)
{
	//EG_ITER_INFO(it);
	Eg_SDL_Window *w = ecs_term(it, Eg_SDL_Window, 1);
	Eg_SDL_Mesh *m = ecs_term(it, Eg_SDL_Mesh, 2);
	for (int i = 0; i < it->count; i ++)
	{
		SDL_Renderer *renderer = w[i].renderer;
		SDL_Vertex * v = m[i].v;
		EG_ASSERT(renderer);
		EG_ASSERT(v);
		if (m[i].count > 0)
		{
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderClear(renderer);
			SDL_RenderGeometry(renderer, NULL, v, m[i].count, NULL, 0);
			SDL_RenderPresent(renderer);
			m[i].count = 0;
		}
	}
}



static void Change_Window_Size(ecs_iter_t *it)
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


static void Update_Window_Size(ecs_iter_t *it)
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


static void Update_UserEvent(ecs_iter_t *it)
{
	EgUserEvent *input = ecs_term(it, EgUserEvent, 1); //Singleton
	memset(input->keyboard_up, 0, sizeof(ecs_u64_t)*EG_NUM_KEYS64);
	memset(input->keyboard_down, 0, sizeof(ecs_u64_t)*EG_NUM_KEYS64);
	SDL_Event event;
	while(SDL_PollEvent(&event))
	{
		switch(event.type)
		{
		case SDL_WINDOWEVENT:
			//EG_TRACE("SDL_WINDOWEVENT");
			if(event.window.event == SDL_WINDOWEVENT_CLOSE)
			{
				ecs_entity_t e = g_sdl_wents[event.window.windowID];
				EgWindow * w = ecs_get_mut(it->world, e, EgWindow, NULL);
				w->should_destroy = true;
				SDL_Window * win = SDL_GetWindowFromID(event.window.windowID);
				EG_TRACE("SDL_WINDOWEVENT_CLOSE %i %p", event.window.windowID, win);
				//SDL_DestroyWindow(win);
			}
			break;
		case SDL_QUIT:
			EG_TRACE("SDL_QUIT");
			//ecs_delete(it->world, it->entities[i]);
			break;
		case SDL_KEYDOWN:
			if(event.key.keysym.scancode < EG_NUM_KEYS)
			{
				EG_U64BITSET_ON(input->keyboard, event.key.keysym.scancode);
				EG_U64BITSET_ON(input->keyboard_down, event.key.keysym.scancode);
			}
			break;
		case SDL_KEYUP:
			if(event.key.keysym.scancode < EG_NUM_KEYS)
			{
				EG_U64BITSET_OFF(input->keyboard, event.key.keysym.scancode);
				EG_U64BITSET_ON(input->keyboard_up, event.key.keysym.scancode);
			}
			break;
		case SDL_MOUSEMOTION:
			input->mouse_x = event.motion.x;
			input->mouse_y = event.motion.y;
			input->mouse_dx = event.motion.xrel;
			input->mouse_dy = event.motion.yrel;
			//EG_TRACE("%i %i %i %i", input->mouse_x, input->mouse_y, input->mouse_dx, input->mouse_dy);
			break;
		}
	}
}


void FlecsComponentsEgSdlImport(ecs_world_t *world)
{
	ECS_MODULE(world, FlecsComponentsEgSdl);
	ECS_IMPORT(world, FlecsComponentsEgGeometry);
	ECS_IMPORT(world, FlecsComponentsEgWindow);
	ECS_IMPORT(world, FlecsComponentsEgUserinput);
	ECS_IMPORT(world, FlecsComponentsEgQuantity);
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

	
	ecs_singleton_set(world, EgUserEvent, { 0 });
	
	ECS_TRIGGER(world, Destroy_Window, EcsOnRemove, Eg_SDL_Window);

	ECS_OBSERVER(world, Create_Window, EcsOnAdd,
	[out] EgWindow,
	[in]  EgRectangleI32);
	ECS_OBSERVER(world, Change_Window_Size, EcsOnSet,
	[out] Eg_SDL_Window,
	[in]  EgRectangleI32);
	ECS_OBSERVER(world, Update_Title, EcsOnSet,
	[out] Eg_SDL_Window,
	[in]  EgTitle);
	ECS_SYSTEM(world, Update_Window, EcsOnUpdate,
	[inout] Eg_SDL_Window,
	[inout] EgWindow);
	ECS_SYSTEM(world, Draw_Rectangle, EcsOnUpdate,
	[out] Eg_SDL_Mesh(parent),
	[in]  EgPosition2F32,
	[in]  EgRectangleF32);
	ECS_SYSTEM(world, Render_Mesh, EcsOnUpdate,
	[in]  Eg_SDL_Window,
	[out] Eg_SDL_Mesh);
	ECS_SYSTEM(world, Update_UserEvent, EcsOnUpdate,
	[out] $EgUserEvent);
	ECS_SYSTEM(world, Update_Window_Size, EcsOnUpdate,
	[in]  Eg_SDL_Window,
	[out] EgRectangleI32);
	
}

