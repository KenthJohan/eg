#include "EgSdlw.h"
#include "EgGeometries.h"
#include "EgWindows.h"
#include "EgEvents.h"
#include "EgQuantities.h"
#include "eg_basics.h"

ECS_COMPONENT_DECLARE(Eg_SDL_Window);


static ecs_sparse_t *g_windows; // g_windows<ecs_entity_t>



static void System_Create_Window(ecs_iter_t *it)
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
		SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE
		);

		bool added = false;
		Eg_SDL_Window *sdlwin = ecs_get_mut(world, e, Eg_SDL_Window, &added);
		EG_ASSERT(sdlwin);
		sdlwin->window = window;
		sdlwin->keys = SDL_GetKeyboardState(NULL);
		int id = SDL_GetWindowID(window);
		flecs_sparse_set(g_windows, ecs_entity_t, id, &e);
		EG_TRACE("");
	}
}


static void System_Destroy_Window(ecs_iter_t *it)
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
			if (w[i].context)
			{
				EG_TRACE("SDL_GL_DeleteContext 0x%x : %s", e, title);
				SDL_GL_DeleteContext(w[i].context);
			}
			if (w[i].window)
			{
				EG_TRACE("SDL_DestroyWindow 0x%x : %s", e, title);
				SDL_DestroyWindow(w[i].window);
			}
		}
	}
}


static void System_Update_Window(ecs_iter_t *it)
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
			int id = SDL_GetWindowID(s[i].window);
			flecs_sparse_remove(g_windows, id);
			ecs_delete(it->world, e);
		}
		if (s[i].window)
		{
			// https://github.com/libsdl-org/SDL/issues/1059
			w[i].flags = SDL_GetWindowFlags(s[i].window);
		}
	}
}


static void System_Update_UserEvent(ecs_iter_t *it)
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
				int id = event.window.windowID;
				ecs_entity_t winent = *flecs_sparse_get(g_windows, ecs_entity_t, id);
				EgWindow * w = ecs_get_mut(it->world, winent, EgWindow, NULL);
				w->should_destroy = true;
				EG_TRACE("SDL_WINDOWEVENT_CLOSE %i %p", event.window.windowID, SDL_GetWindowFromID(event.window.windowID));
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



void EgSdlwImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgSdlw);
	ECS_IMPORT(world, EgWindows);
	ECS_IMPORT(world, EgGeometries);
	ECS_IMPORT(world, EgEvents);
	ecs_set_name_prefix(world, "Eg");
	ECS_COMPONENT_DEFINE(world, Eg_SDL_Window);

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(Eg_SDL_Window),
	.members = {
	{ .name = "window", .type = ecs_id(ecs_uptr_t) },
	{ .name = "context", .type = ecs_id(ecs_uptr_t) },
	{ .name = "elapsed_milliseconds", .type = ecs_id(ecs_u32_t) },
	{ .name = "keys", .type = ecs_id(ecs_u8_t), 2 }, // TODO: SDL_NUM_SCANCODES
	}
	});



	g_windows = flecs_sparse_new(SDL_Window*);

	ecs_trigger_init(world, &(ecs_trigger_desc_t) {
	.term = { .id = ecs_id(Eg_SDL_Window), .inout = EcsInOut },
	.events = {EcsOnRemove},
	.callback = System_Destroy_Window
	});

	ecs_observer_init(world, &(ecs_observer_desc_t) {
	.filter.terms = {
	{ .id = ecs_id(EgWindow), .inout = EcsInOut },
	{ .id = ecs_id(EgRectangleI32), .inout = EcsIn}
	},
	.events = {EcsOnAdd},
	.callback = System_Create_Window
	});

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
	{ .id = ecs_id(Eg_SDL_Window), .inout = EcsInOut },
	{ .id = ecs_id(EgWindow), .inout = EcsInOut}
	},
	.entity.add = {ecs_dependson(EcsOnUpdate)},
	.callback = System_Update_Window
	});


	ecs_system_init(world, &(ecs_system_desc_t) {
	.query.filter.expr = "[out] EgUserEvent($)",
	.entity.add = {ecs_dependson(EcsOnUpdate)},
	.callback = System_Update_UserEvent
	});

	ecs_system_init(world, &(ecs_system_desc_t) {
	.query.filter.terms = {
	{ .id = ecs_id(Eg_SDL_Window), .inout = EcsIn },
	{ .id = ecs_id(EgRectangleI32), .inout = EcsOut}
	},
	.entity.add = {ecs_dependson(EcsOnUpdate)},
	.callback = System_Update_Window_Size
	});



	ecs_singleton_set(world, EgUserEvent, { 0 });

}

