/*
  Copyright (C) 1997-2024 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/

#include <stdlib.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <flecs.h>

/* Regenerate the shaders with testgpu/build-shaders.sh */
#include "../shaders/testgpu_spirv.h"

#include "shader_spirv.h"
#include "vertex.h"
#include "matrix.h"

#include "EgFs.h"
#include "EgDisplay.h"
#include "main_render.h"
#include "main_types.h"
#include "SDL_test_common.h"


#define TESTGPU_SUPPORTED_FORMATS (SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXBC | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_METALLIB)


static Uint32 frames = 0;


static SDL_GPUDevice *gpu_device = NULL;
static RenderState render_state;
static SDLTest_CommonState *state = NULL;
static WindowState *window_states = NULL;











static void
init_render_state(int msaa)
{
	SDL_GPUCommandBuffer *cmd;
	SDL_GPUTransferBuffer *buf_transfer;
	void *map;
	SDL_GPUTransferBufferLocation buf_location;
	SDL_GPUBufferRegion dst_region;
	SDL_GPUCopyPass *copy_pass;
	SDL_GPUBufferCreateInfo buffer_desc;
	SDL_GPUTransferBufferCreateInfo transfer_buffer_desc;
	SDL_GPUGraphicsPipelineCreateInfo pipelinedesc;
	SDL_GPUColorTargetDescription color_target_desc;
	Uint32 drawablew, drawableh;
	SDL_GPUVertexAttribute vertex_attributes[2];
	SDL_GPUVertexBufferDescription vertex_buffer_desc;
	SDL_GPUShader *vertex_shader;
	SDL_GPUShader *fragment_shader;
	int i;

	gpu_device = SDL_CreateGPUDevice(
	TESTGPU_SUPPORTED_FORMATS,
	true,
	state->gpudriver);
	if (gpu_device == NULL) {
		quit(2, &render_state, window_states, state, gpu_device);
	}

	/* Claim the windows */

	for (i = 0; i < state->num_windows; i++) {
		SDL_ClaimWindowForGPUDevice(
		gpu_device,
		state->windows[i]);
	}

	/* Create shaders */

	//vertex_shader = load_shader(true);
	vertex_shader = shader_spirv_compile(gpu_device, "shaders/cube", SDL_GPU_SHADERSTAGE_VERTEX);
	if (vertex_shader == NULL) {
		quit(2, &render_state, window_states, state, gpu_device);
	}
	

	//fragment_shader = load_shader(false);
	fragment_shader = shader_spirv_compile(gpu_device, "shaders/cube", SDL_GPU_SHADERSTAGE_FRAGMENT);
	if (fragment_shader == NULL) {
		quit(2, &render_state, window_states, state, gpu_device);
	}

	/* Create buffers */
	buffer_desc.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
	buffer_desc.size = sizeof(vertex_data);
	buffer_desc.props = 0;
	render_state.buf_vertex = SDL_CreateGPUBuffer(
	gpu_device,
	&buffer_desc);
	if (render_state.buf_vertex == NULL) {
		quit(2, &render_state, window_states, state, gpu_device);
	}

	SDL_SetGPUBufferName(gpu_device, render_state.buf_vertex, "космонавт");

	transfer_buffer_desc.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	transfer_buffer_desc.size = sizeof(vertex_data);
	transfer_buffer_desc.props = 0;
	buf_transfer = SDL_CreateGPUTransferBuffer(
	gpu_device,
	&transfer_buffer_desc);

	if (buf_transfer == NULL) {
		quit(2, &render_state, window_states, state, gpu_device);
	}

	/* We just need to upload the static data once. */
	map = SDL_MapGPUTransferBuffer(gpu_device, buf_transfer, false);
	SDL_memcpy(map, vertex_data, sizeof(vertex_data));
	SDL_UnmapGPUTransferBuffer(gpu_device, buf_transfer);

	cmd = SDL_AcquireGPUCommandBuffer(gpu_device);
	copy_pass = SDL_BeginGPUCopyPass(cmd);
	buf_location.transfer_buffer = buf_transfer;
	buf_location.offset = 0;
	dst_region.buffer = render_state.buf_vertex;
	dst_region.offset = 0;
	dst_region.size = sizeof(vertex_data);
	SDL_UploadToGPUBuffer(copy_pass, &buf_location, &dst_region, false);
	SDL_EndGPUCopyPass(copy_pass);
	SDL_SubmitGPUCommandBuffer(cmd);

	SDL_ReleaseGPUTransferBuffer(gpu_device, buf_transfer);

	/* Determine which sample count to use */
	render_state.sample_count = SDL_GPU_SAMPLECOUNT_1;
	if (msaa && SDL_GPUTextureSupportsSampleCount(
	            gpu_device,
	            SDL_GetGPUSwapchainTextureFormat(gpu_device, state->windows[0]),
	            SDL_GPU_SAMPLECOUNT_4)) {
		render_state.sample_count = SDL_GPU_SAMPLECOUNT_4;
	}

	/* Set up the graphics pipeline */

	SDL_zero(pipelinedesc);
	SDL_zero(color_target_desc);

	color_target_desc.format = SDL_GetGPUSwapchainTextureFormat(gpu_device, state->windows[0]);

	pipelinedesc.target_info.num_color_targets = 1;
	pipelinedesc.target_info.color_target_descriptions = &color_target_desc;
	pipelinedesc.target_info.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
	pipelinedesc.target_info.has_depth_stencil_target = true;

	pipelinedesc.depth_stencil_state.enable_depth_test = true;
	pipelinedesc.depth_stencil_state.enable_depth_write = true;
	pipelinedesc.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_LESS_OR_EQUAL;

	pipelinedesc.multisample_state.sample_count = render_state.sample_count;

	pipelinedesc.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

	pipelinedesc.vertex_shader = vertex_shader;
	pipelinedesc.fragment_shader = fragment_shader;

	vertex_buffer_desc.slot = 0;
	vertex_buffer_desc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
	vertex_buffer_desc.instance_step_rate = 0;
	vertex_buffer_desc.pitch = sizeof(VertexData);

	vertex_attributes[0].buffer_slot = 0;
	vertex_attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	vertex_attributes[0].location = 0;
	vertex_attributes[0].offset = 0;

	vertex_attributes[1].buffer_slot = 0;
	vertex_attributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	vertex_attributes[1].location = 1;
	vertex_attributes[1].offset = sizeof(float) * 3;

	pipelinedesc.vertex_input_state.num_vertex_buffers = 1;
	pipelinedesc.vertex_input_state.vertex_buffer_descriptions = &vertex_buffer_desc;
	pipelinedesc.vertex_input_state.num_vertex_attributes = 2;
	pipelinedesc.vertex_input_state.vertex_attributes = (SDL_GPUVertexAttribute *)&vertex_attributes;

	pipelinedesc.props = 0;

	render_state.pipeline = SDL_CreateGPUGraphicsPipeline(gpu_device, &pipelinedesc);
	if (render_state.pipeline == NULL) {
		quit(2, &render_state, window_states, state, gpu_device);
	}


	/* These are reference-counted; once the pipeline is created, you don't need to keep these. */
	SDL_ReleaseGPUShader(gpu_device, vertex_shader);
	SDL_ReleaseGPUShader(gpu_device, fragment_shader);

	/* Set up per-window state */

	window_states = (WindowState *)SDL_calloc(state->num_windows, sizeof(WindowState));
	if (!window_states) {
		SDL_Log("Out of memory!\n");
		quit(2, &render_state, window_states, state, gpu_device);
	}

	for (i = 0; i < state->num_windows; i++) {
		WindowState *winstate = &window_states[i];

		/* create a depth texture for the window */
		SDL_GetWindowSizeInPixels(state->windows[i], (int *)&drawablew, (int *)&drawableh);
		winstate->tex_depth = CreateDepthTexture(&render_state, state, gpu_device, drawablew, drawableh);
		winstate->tex_msaa = CreateMSAATexture(&render_state, state, gpu_device, drawablew, drawableh);
		winstate->tex_resolve = CreateResolveTexture(&render_state, state, gpu_device, drawablew, drawableh);

		/* make each window different */
		winstate->angle_x = (i * 10) % 360;
		winstate->angle_y = (i * 20) % 360;
		winstate->angle_z = (i * 30) % 360;
	}
}







int main(int argc, char *argv[])
{

	ecs_world_t * world = ecs_init();
	ECS_IMPORT(world, FlecsUnits);
	ECS_IMPORT(world, FlecsDoc);
	ecs_set(world, EcsWorld, EcsRest, {.port = 0});
	printf("Remote: %s\n", "https://www.flecs.dev/explorer/?remote=true");

	ecs_log_set_level(0);
	ecs_script_run_file(world, "config/hello.flecs");
	ecs_log_set_level(-1);

	int done = 0;
	int i;
	const SDL_DisplayMode *mode;
	Uint64 then, now;

	/* Initialize test framework */
	state = SDLTest_CommonCreateState(argv, SDL_INIT_VIDEO);
	if (!state) {
		return 1;
	}


	state->verbose = VERBOSE_VIDEO | VERBOSE_MODES | VERBOSE_RENDER | VERBOSE_EVENT | VERBOSE_AUDIO | VERBOSE_MOTION;
	state->skip_renderer = 1;
	state->window_flags |= SDL_WINDOW_RESIZABLE;

	if (!SDLTest_CommonInit(state)) {
		quit(2, &render_state, window_states, state, gpu_device);
		return 0;
	}


	ECS_IMPORT(world, EgFs);
	ECS_IMPORT(world, EgDisplay);


	mode = SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(state->windows[0]));
	SDL_Log("Screen bpp: %d\n", SDL_BITSPERPIXEL(mode->format));

	init_render_state(0);

	/* Main render loop */
	frames = 0;
	then = SDL_GetTicks();
	done = 0;


	while (!done) {
		ecs_progress(world, 0.0f);
		SDL_Event event;
		int i;

		/* Check for events */
		while (SDL_PollEvent(&event) && !done) {
			SDLTest_CommonEvent(state, &event, &done);
		}
		if (!done) {
			for (i = 0; i < state->num_windows; ++i) {
				main_render(state, &render_state, state->windows, gpu_device, window_states, i);
			}
		}
	}


	/* Print out some timing information */
	now = SDL_GetTicks();
	if (now > then) {
		SDL_Log("%2.2f frames per second\n",
		((double)frames * 1000) / (now - then));
	}


	quit(0, &render_state, window_states, state, gpu_device);
	return 0;
}

/* vi: set ts=4 sw=4 expandtab: */