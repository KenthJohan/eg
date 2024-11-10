#pragma once
#include "main_types.h"
#include "SDL_test_common.h"

void main_render(SDLTest_CommonState *state, RenderState * render_state, SDL_Window ** windows, SDL_GPUDevice *gpu_device, WindowState * window_states, const int windownum);


SDL_GPUTexture * CreateDepthTexture(RenderState * render_state, SDLTest_CommonState *state, SDL_GPUDevice *gpu_device, Uint32 drawablew, Uint32 drawableh);

SDL_GPUTexture * CreateMSAATexture(RenderState * render_state, SDLTest_CommonState *state, SDL_GPUDevice *gpu_device, Uint32 drawablew, Uint32 drawableh);

SDL_GPUTexture * CreateResolveTexture(RenderState * render_state, SDLTest_CommonState *state, SDL_GPUDevice *gpu_device, Uint32 drawablew, Uint32 drawableh);