#include "EgWindows.h"

#include <stdlib.h>
#include <stdio.h>

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_events.h>

#include <EgShapes.h>
#include <EgSpatials.h>
#include <EgButtons.h>
#include <EgBase.h>
#include <EgWindows.h>

// https://github.com/SanderMertens/flecs/blob/57ebed1083274ee4875631a940452f08ff08aef9/test/collections/src/Map.c#L54
// We need to map the SDL_WindowID to the ecs_entity_t so we can look up the entity from the SDL_WindowID
// This is needed for the SDL_EVENT_WINDOW_RESIZED event
static ecs_map_t static_window_map;

static void System_EgWindowsWindow_Create(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	EgWindowsWindowCreateInfo *create = ecs_field(it, EgWindowsWindowCreateInfo, 0);
	EgShapesRectangle *rect = ecs_field(it, EgShapesRectangle, 1);
	ecs_log_set_level(1);
	ecs_trace("System_EgWindowsWindow_Create() count:%i", it->count);
	ecs_log_push_(0);
	for (int i = 0; i < it->count; ++i, ++create, ++rect) {
		ecs_entity_t e = it->entities[i];
		ecs_trace("Entity: '%s'", ecs_get_name(world, e));
		ecs_log_push_(0);
		{
			SDL_PropertiesID props = SDL_CreateProperties();
			SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, ecs_get_name(world, e));
			SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, 100);
			SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, 100);
			SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, rect->w);
			SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, rect->h);
			SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, create->enable_opengl);
			SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE;
			SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_FLAGS_NUMBER, flags);

			if (create->enable_opengl) {
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
			}

			SDL_Window *window = SDL_CreateWindowWithProperties(props);
			if (window == NULL) {
				continue;
			}
			// Map the SDL_WindowID to the ecs_entity_t:
			SDL_WindowID id = SDL_GetWindowID(window);
			ecs_map_insert(&static_window_map, id, e);
			ecs_set(world, e, EgWindowsWindow, {.object = window});
			ecs_trace("SDL_CreateWindowWithProperties() -> %p", window);
		}
		ecs_log_pop_(0);

	} // END FOR LOOP
	ecs_log_pop_(0);
	ecs_log_set_level(0);
}

static void System_EgWindowsWindow_Rectangle(ecs_iter_t *it)
{
	EgWindowsWindow *cw = ecs_field(it, EgWindowsWindow, 0);
	EgShapesRectangle *cr = ecs_field(it, EgShapesRectangle, 1);
	for (int i = 0; i < it->count; ++i) {
		Uint32 w;
		Uint32 h;
		SDL_GetWindowSizeInPixels(cw[i].object, (int *)&w, (int *)&h);
		cr[i].w = w;
		cr[i].h = h;
	}
}

static void System_EgWindowsWindow_Position(ecs_iter_t *it)
{
	EgWindowsWindow *cw = ecs_field(it, EgWindowsWindow, 0); // self, in
	Position2 *cp = ecs_field(it, Position2, 1);             // self, out
	for (int i = 0; i < it->count; ++i) {
		int x;
		int y;
		SDL_GetWindowPosition(cw[i].object, (int *)&x, (int *)&y);
		cp[i].x = x;
		cp[i].y = y;
	}
}

static void System_EgWindowsWindow_Mouse(ecs_iter_t *it)
{
	Position2 *cp0 = ecs_field(it, Position2, 0); // parent
	Position2 *cp1 = ecs_field(it, Position2, 1); // self
	// EgWindowsMouse *cm = ecs_field(it, EgWindowsMouse, 2); // self
	for (int i = 0; i < it->count; ++i) {
		float x;
		float y;
		SDL_GetGlobalMouseState(&x, &y);
		cp1[i].x = x - cp0->x;
		cp1[i].y = y - cp0->y;
	}
}

static void handle_window_event(ecs_world_t *world, SDL_WindowEvent *event)
{
	ecs_map_val_t *ev = ecs_map_get(&static_window_map, event->windowID);
	if (ev == NULL) {
		ecs_err("handle_window_event: No entity found for SDL_WindowID: %i", event->windowID);
		return;
	}
	ecs_entity_t e = ev[0];
	switch (event->type) {
	case SDL_EVENT_WINDOW_RESIZED:
		printf("SDL_EVENT_WINDOW_RESIZED: %s\n", ecs_get_name(world, e));
		ecs_add(world, e, EgWindowsEventResize);
		break;
	case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
		printf("SDL_EVENT_WINDOW_CLOSE_REQUESTED: %s\n", ecs_get_name(world, e));
		ecs_add(world, e, EgWindowsEventCloseRequest);
		break;
	default:
		break;
	}
}

static void System_Events_Update(ecs_iter_t *it)
{
	EgButtonsState *s = ecs_field(it, EgButtonsState, 0); // Singleton
	for (int i = 0; i < EG_BUTTONS_KEYS_MAX; ++i) {
		s->state[i] &= ~(EG_BUTTONS_STATE_PRESSED | EG_BUTTONS_STATE_RELEASED);
	}

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		// SDLTest_CommonEvent(state, &event, &done);
		switch (event.type) {
		case SDL_EVENT_QUIT:
			break;
		case SDL_EVENT_WINDOW_RESIZED:
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			handle_window_event(it->world, &event.window);
			break;
		case SDL_EVENT_KEY_DOWN:
			// SDL_EVENT_KEY_DOWN can repeat.
			// Set oneshot key pressed state:
			if (s->state[event.key.scancode] & EG_BUTTONS_STATE_HELD) {
				s->state[event.key.scancode] &= ~EG_BUTTONS_STATE_PRESSED;
			} else {
				s->state[event.key.scancode] |= EG_BUTTONS_STATE_PRESSED;
			}
			s->state[event.key.scancode] |= EG_BUTTONS_STATE_HELD;
			break;
		case SDL_EVENT_KEY_UP:
			// SDL_EVENT_KEY_UP does not repeat.
			s->state[event.key.scancode] &= ~EG_BUTTONS_STATE_HELD;
			s->state[event.key.scancode] |= EG_BUTTONS_STATE_RELEASED;
			break;
		} // END SWITCH
	} // END WHILE
	// printf("EG_BUTTONS_STATE_RISING_EDGE: %i\n", s->scancode[SDL_SCANCODE_B] & EG_BUTTONS_STATE_RISING_EDGE);
}

static void System_Resize(ecs_iter_t *it)
{
	ecs_id_t pair = ecs_field_id(it, 1);
	ecs_entity_t food = ecs_pair_second(it->world, pair);
	ecs_trace("food: %s\n", ecs_get_name(it->world, food));
	ecs_add(it->world, food, EgBaseUpdate);
	for (int i = 0; i < it->count; ++i) {
		ecs_remove(it->world, it->entities[i], EgWindowsEventResize);
	}
}

void EgWindowsSdlImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgWindowsSdl);
	ECS_IMPORT(world, EgWindows);
	ECS_IMPORT(world, EgButtons);
	ecs_set_name_prefix(world, "EgWindowsSdl");

	ecs_map_init(&static_window_map, NULL);

	ecs_struct(world,
	{.entity = ecs_id(EgWindowsWindow),
	.members = {
	{.name = "object", .type = ecs_id(ecs_uptr_t)}}});

	ecs_struct(world,
	{.entity = ecs_id(EgWindowsWindowCreateInfo),
	.members = {
	{.name = "debug", .type = ecs_id(ecs_bool_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgWindowsMouse),
	.members = {
	{.name = "dummy", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_system(world,
	{.entity = ecs_entity(world, {.name = "System_EgWindowsWindow_Create", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_EgWindowsWindow_Create,
	.query.terms =
	{
	{.id = ecs_id(EgWindowsWindowCreateInfo), .src.id = EcsSelf},
	{.id = ecs_id(EgShapesRectangle), .src.id = EcsSelf},
	{.id = ecs_id(EgWindowsWindow), .oper = EcsNot}, // Adds this
	}});

	ecs_system(world,
	{.entity = ecs_entity(world, {.name = "System_EgWindowsWindow_Rectangle", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_EgWindowsWindow_Rectangle,
	.query.terms =
	{
	{.id = ecs_id(EgWindowsWindow), .src.id = EcsSelf},
	{.id = ecs_id(EgShapesRectangle), .src.id = EcsSelf},
	}});

	ecs_system(world,
	{.entity = ecs_entity(world, {.name = "System_EgWindowsWindow_Position", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_EgWindowsWindow_Position,
	.query.terms =
	{
	{.id = ecs_id(EgWindowsWindow), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(Position2), .src.id = EcsSelf, .inout = EcsOut},
	}});

	ecs_system(world,
	{.entity = ecs_entity(world, {.name = "System_EgWindowsWindow_Mouse", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_EgWindowsWindow_Mouse,
	.query.terms =
	{
	{.id = ecs_id(Position2), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn},
	{.id = ecs_id(Position2), .src.id = EcsSelf, .inout = EcsOut},
	{.id = ecs_id(EgWindowsMouse), .src.id = EcsSelf, .inout = EcsIn},
	}});

	ecs_system(world,
	{.entity = ecs_entity(world, {.name = "System_Events_Update", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_Events_Update,
	.query.terms =
	{
	{.id = ecs_id(EgButtonsState), .src.id = ecs_id(EgButtonsState), .inout = EcsInOut},
	}});

	ecs_system(world,
	{.entity = ecs_entity(world, {.name = "System_Resize", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_Resize,
	.query.terms =
	{
	{.id = ecs_id(EgWindowsEventResize), .src.id = EcsSelf, .inout = EcsOut},
	{.id = ecs_pair(EgWindowsEventResize, EcsWildcard), .src.id = EcsSelf, .inout = EcsOut},
	}});

	ecs_singleton_set(world, EgButtonsState, {.state = {0}});
}
