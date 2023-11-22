#pragma once
#include "flecs.h"
#include <SDL2/SDL.h>

typedef struct
{
	SDL_Window * window;
	SDL_GLContext * context;
	ecs_u32_t elapsed_milliseconds;
	const Uint8 *keys;
} Eg_SDL_Window;

extern ECS_COMPONENT_DECLARE(Eg_SDL_Window);

void EgSdlwImport(ecs_world_t *world);


