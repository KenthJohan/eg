#pragma once

#include <SDL2/SDL.h>
#include "microui.h"

void mu_backend_events(mu_Context *ctx, SDL_Event *e);

void mu_backend_render(mu_Context *ctx);