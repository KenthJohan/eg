#pragma once

#include <SDL3/SDL_gpu.h>
#include "SDL_test_common.h"

typedef struct RenderState {
	SDL_GPUBuffer *buf_vertex;
	SDL_GPUGraphicsPipeline *pipeline;
	SDL_GPUSampleCount sample_count;
} RenderState;

typedef struct WindowState {
	int angle_x, angle_y, angle_z;
	SDL_GPUTexture *tex_depth, *tex_msaa, *tex_resolve;
	Uint32 prev_drawablew, prev_drawableh;
} WindowState;


/* Call this instead of exit(), so we can clean up SDL: atexit() is evil. */
void quit(int rc, RenderState * render_state, WindowState *window_states, SDLTest_CommonState *state, SDL_GPUDevice *gpu_device);