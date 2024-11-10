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

#include <SDL3/SDL_test_common.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <flecs.h>

/* Regenerate the shaders with testgpu/build-shaders.sh */
#include "../shaders/testgpu_spirv.h"

#include "shader_spirv.h"
#include "vertex.h"
#include "matrix.h"

#include "eg_fs.h"

#define TESTGPU_SUPPORTED_FORMATS (SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXBC | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_METALLIB)

#define CHECK_CREATE(var, thing)                                         \
	{                                                                    \
		if (!(var)) {                                                    \
			SDL_Log("Failed to create %s: %s\n", thing, SDL_GetError()); \
			quit(2);                                                     \
		}                                                                \
	}

static Uint32 frames = 0;

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

static SDL_GPUDevice *gpu_device = NULL;
static RenderState render_state;
static SDLTest_CommonState *state = NULL;
static WindowState *window_states = NULL;

static void shutdownGPU(void)
{
	if (window_states) {
		int i;
		for (i = 0; i < state->num_windows; i++) {
			WindowState *winstate = &window_states[i];
			SDL_ReleaseGPUTexture(gpu_device, winstate->tex_depth);
			SDL_ReleaseGPUTexture(gpu_device, winstate->tex_msaa);
			SDL_ReleaseGPUTexture(gpu_device, winstate->tex_resolve);
			SDL_ReleaseWindowFromGPUDevice(gpu_device, state->windows[i]);
		}
		SDL_free(window_states);
		window_states = NULL;
	}

	SDL_ReleaseGPUBuffer(gpu_device, render_state.buf_vertex);
	SDL_ReleaseGPUGraphicsPipeline(gpu_device, render_state.pipeline);
	SDL_DestroyGPUDevice(gpu_device);

	SDL_zero(render_state);
	gpu_device = NULL;
}

/* Call this instead of exit(), so we can clean up SDL: atexit() is evil. */
static void
quit(int rc)
{
	shutdownGPU();
	SDLTest_CommonQuit(state);
	exit(rc);
}




static SDL_GPUTexture *
CreateDepthTexture(Uint32 drawablew, Uint32 drawableh)
{
	SDL_GPUTextureCreateInfo createinfo;
	SDL_GPUTexture *result;

	createinfo.type = SDL_GPU_TEXTURETYPE_2D;
	createinfo.format = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
	createinfo.width = drawablew;
	createinfo.height = drawableh;
	createinfo.layer_count_or_depth = 1;
	createinfo.num_levels = 1;
	createinfo.sample_count = render_state.sample_count;
	createinfo.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
	createinfo.props = 0;

	result = SDL_CreateGPUTexture(gpu_device, &createinfo);
	CHECK_CREATE(result, "Depth Texture")

	return result;
}

static SDL_GPUTexture *
CreateMSAATexture(Uint32 drawablew, Uint32 drawableh)
{
	SDL_GPUTextureCreateInfo createinfo;
	SDL_GPUTexture *result;

	if (render_state.sample_count == SDL_GPU_SAMPLECOUNT_1) {
		return NULL;
	}

	createinfo.type = SDL_GPU_TEXTURETYPE_2D;
	createinfo.format = SDL_GetGPUSwapchainTextureFormat(gpu_device, state->windows[0]);
	createinfo.width = drawablew;
	createinfo.height = drawableh;
	createinfo.layer_count_or_depth = 1;
	createinfo.num_levels = 1;
	createinfo.sample_count = render_state.sample_count;
	createinfo.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
	createinfo.props = 0;

	result = SDL_CreateGPUTexture(gpu_device, &createinfo);
	CHECK_CREATE(result, "MSAA Texture")

	return result;
}

static SDL_GPUTexture *
CreateResolveTexture(Uint32 drawablew, Uint32 drawableh)
{
	SDL_GPUTextureCreateInfo createinfo;
	SDL_GPUTexture *result;

	if (render_state.sample_count == SDL_GPU_SAMPLECOUNT_1) {
		return NULL;
	}

	createinfo.type = SDL_GPU_TEXTURETYPE_2D;
	createinfo.format = SDL_GetGPUSwapchainTextureFormat(gpu_device, state->windows[0]);
	createinfo.width = drawablew;
	createinfo.height = drawableh;
	createinfo.layer_count_or_depth = 1;
	createinfo.num_levels = 1;
	createinfo.sample_count = SDL_GPU_SAMPLECOUNT_1;
	createinfo.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
	createinfo.props = 0;

	result = SDL_CreateGPUTexture(gpu_device, &createinfo);
	CHECK_CREATE(result, "Resolve Texture")

	return result;
}

static void
Render(SDL_Window *window, const int windownum)
{
	WindowState *winstate = &window_states[windownum];
	SDL_GPUTexture *swapchainTexture;
	SDL_GPUColorTargetInfo color_target;
	SDL_GPUDepthStencilTargetInfo depth_target;
	float matrix_rotate[16], matrix_modelview[16], matrix_perspective[16], matrix_final[16];
	SDL_GPUCommandBuffer *cmd;
	SDL_GPURenderPass *pass;
	SDL_GPUBufferBinding vertex_binding;
	SDL_GPUBlitInfo blit_info;
	Uint32 drawablew, drawableh;

	/* Acquire the swapchain texture */

	cmd = SDL_AcquireGPUCommandBuffer(gpu_device);
	if (!cmd) {
		SDL_Log("Failed to acquire command buffer :%s", SDL_GetError());
		quit(2);
	}
	if (!SDL_AcquireGPUSwapchainTexture(cmd, state->windows[windownum], &swapchainTexture, &drawablew, &drawableh)) {
		SDL_Log("Failed to acquire swapchain texture: %s", SDL_GetError());
		quit(2);
	}

	if (swapchainTexture == NULL) {
		/* No swapchain was acquired, probably too many frames in flight */
		SDL_SubmitGPUCommandBuffer(cmd);
		return;
	}

	/*
	 * Do some rotation with Euler angles. It is not a fixed axis as
	 * quaterions would be, but the effect is cool.
	 */
	rotate_matrix((float)winstate->angle_x, 1.0f, 0.0f, 0.0f, matrix_modelview);
	rotate_matrix((float)winstate->angle_y, 0.0f, 1.0f, 0.0f, matrix_rotate);

	multiply_matrix(matrix_rotate, matrix_modelview, matrix_modelview);

	rotate_matrix((float)winstate->angle_z, 0.0f, 1.0f, 0.0f, matrix_rotate);

	multiply_matrix(matrix_rotate, matrix_modelview, matrix_modelview);

	/* Pull the camera back from the cube */
	matrix_modelview[14] -= 2.5f;

	perspective_matrix(45.0f, (float)drawablew / drawableh, 0.01f, 100.0f, matrix_perspective);
	multiply_matrix(matrix_perspective, matrix_modelview, (float *)&matrix_final);

	winstate->angle_x += 3;
	winstate->angle_y += 2;
	winstate->angle_z += 1;

	if (winstate->angle_x >= 360)
		winstate->angle_x -= 360;
	if (winstate->angle_x < 0)
		winstate->angle_x += 360;
	if (winstate->angle_y >= 360)
		winstate->angle_y -= 360;
	if (winstate->angle_y < 0)
		winstate->angle_y += 360;
	if (winstate->angle_z >= 360)
		winstate->angle_z -= 360;
	if (winstate->angle_z < 0)
		winstate->angle_z += 360;

	/* Resize the depth buffer if the window size changed */

	if (winstate->prev_drawablew != drawablew || winstate->prev_drawableh != drawableh) {
		SDL_ReleaseGPUTexture(gpu_device, winstate->tex_depth);
		SDL_ReleaseGPUTexture(gpu_device, winstate->tex_msaa);
		SDL_ReleaseGPUTexture(gpu_device, winstate->tex_resolve);
		winstate->tex_depth = CreateDepthTexture(drawablew, drawableh);
		winstate->tex_msaa = CreateMSAATexture(drawablew, drawableh);
		winstate->tex_resolve = CreateResolveTexture(drawablew, drawableh);
	}
	winstate->prev_drawablew = drawablew;
	winstate->prev_drawableh = drawableh;

	/* Set up the pass */

	SDL_zero(color_target);
	color_target.clear_color.a = 1.0f;
	if (winstate->tex_msaa) {
		color_target.load_op = SDL_GPU_LOADOP_CLEAR;
		color_target.store_op = SDL_GPU_STOREOP_RESOLVE;
		color_target.texture = winstate->tex_msaa;
		color_target.resolve_texture = winstate->tex_resolve;
		color_target.cycle = true;
		color_target.cycle_resolve_texture = true;
	} else {
		color_target.load_op = SDL_GPU_LOADOP_CLEAR;
		color_target.store_op = SDL_GPU_STOREOP_STORE;
		color_target.texture = swapchainTexture;
	}

	SDL_zero(depth_target);
	depth_target.clear_depth = 1.0f;
	depth_target.load_op = SDL_GPU_LOADOP_CLEAR;
	depth_target.store_op = SDL_GPU_STOREOP_DONT_CARE;
	depth_target.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
	depth_target.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
	depth_target.texture = winstate->tex_depth;
	depth_target.cycle = true;

	/* Set up the bindings */

	vertex_binding.buffer = render_state.buf_vertex;
	vertex_binding.offset = 0;

	/* Draw the cube! */

	SDL_PushGPUVertexUniformData(cmd, 0, matrix_final, sizeof(matrix_final));

	pass = SDL_BeginGPURenderPass(cmd, &color_target, 1, &depth_target);
	SDL_BindGPUGraphicsPipeline(pass, render_state.pipeline);
	SDL_BindGPUVertexBuffers(pass, 0, &vertex_binding, 1);
	SDL_DrawGPUPrimitives(pass, 36, 1, 0, 0);
	SDL_EndGPURenderPass(pass);

	/* Blit MSAA resolve target to swapchain, if needed */
	if (render_state.sample_count > SDL_GPU_SAMPLECOUNT_1) {
		SDL_zero(blit_info);
		blit_info.source.texture = winstate->tex_resolve;
		blit_info.source.w = drawablew;
		blit_info.source.h = drawableh;

		blit_info.destination.texture = swapchainTexture;
		blit_info.destination.w = drawablew;
		blit_info.destination.h = drawableh;

		blit_info.load_op = SDL_GPU_LOADOP_DONT_CARE;
		blit_info.filter = SDL_GPU_FILTER_LINEAR;

		SDL_BlitGPUTexture(cmd, &blit_info);
	}

	/* Submit the command buffer! */
	SDL_SubmitGPUCommandBuffer(cmd);

	++frames;
}



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
	CHECK_CREATE(gpu_device, "GPU device");

	/* Claim the windows */

	for (i = 0; i < state->num_windows; i++) {
		SDL_ClaimWindowForGPUDevice(
		gpu_device,
		state->windows[i]);
	}

	/* Create shaders */

	//vertex_shader = load_shader(true);
	vertex_shader = shader_spirv_compile(gpu_device, "shaders/cube", SDL_GPU_SHADERSTAGE_VERTEX);
	CHECK_CREATE(vertex_shader, "Vertex Shader")
	//fragment_shader = load_shader(false);
	fragment_shader = shader_spirv_compile(gpu_device, "shaders/cube", SDL_GPU_SHADERSTAGE_FRAGMENT);
	CHECK_CREATE(fragment_shader, "Fragment Shader")

	/* Create buffers */

	buffer_desc.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
	buffer_desc.size = sizeof(vertex_data);
	buffer_desc.props = 0;
	render_state.buf_vertex = SDL_CreateGPUBuffer(
	gpu_device,
	&buffer_desc);
	CHECK_CREATE(render_state.buf_vertex, "Static vertex buffer")

	SDL_SetGPUBufferName(gpu_device, render_state.buf_vertex, "космонавт");

	transfer_buffer_desc.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	transfer_buffer_desc.size = sizeof(vertex_data);
	transfer_buffer_desc.props = 0;
	buf_transfer = SDL_CreateGPUTransferBuffer(
	gpu_device,
	&transfer_buffer_desc);
	CHECK_CREATE(buf_transfer, "Vertex transfer buffer")

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
	CHECK_CREATE(render_state.pipeline, "Render Pipeline")

	/* These are reference-counted; once the pipeline is created, you don't need to keep these. */
	SDL_ReleaseGPUShader(gpu_device, vertex_shader);
	SDL_ReleaseGPUShader(gpu_device, fragment_shader);

	/* Set up per-window state */

	window_states = (WindowState *)SDL_calloc(state->num_windows, sizeof(WindowState));
	if (!window_states) {
		SDL_Log("Out of memory!\n");
		quit(2);
	}

	for (i = 0; i < state->num_windows; i++) {
		WindowState *winstate = &window_states[i];

		/* create a depth texture for the window */
		SDL_GetWindowSizeInPixels(state->windows[i], (int *)&drawablew, (int *)&drawableh);
		winstate->tex_depth = CreateDepthTexture(drawablew, drawableh);
		winstate->tex_msaa = CreateMSAATexture(drawablew, drawableh);
		winstate->tex_resolve = CreateResolveTexture(drawablew, drawableh);

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
	int msaa;
	int i;
	const SDL_DisplayMode *mode;
	Uint64 then, now;


	/* Initialize params */
	msaa = 0;

	/* Initialize test framework */
	state = SDLTest_CommonCreateState(argv, SDL_INIT_VIDEO);
	if (!state) {
		return 1;
	}
	for (i = 1; i < argc;) {
		int consumed;

		consumed = SDLTest_CommonArg(state, i);
		if (consumed == 0) {
			if (SDL_strcasecmp(argv[i], "--msaa") == 0) {
				++msaa;
				consumed = 1;
			} else {
				consumed = -1;
			}
		}
		if (consumed < 0) {
			static const char *options[] = {"[--msaa]", NULL};
			SDLTest_CommonLogUsage(state, argv[0], options);
			quit(1);
		}
		i += consumed;
	}

	state->skip_renderer = 1;
	state->window_flags |= SDL_WINDOW_RESIZABLE;

	if (!SDLTest_CommonInit(state)) {
		quit(2);
		return 0;
	}


	ECS_IMPORT(world, EgFs);


	mode = SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(state->windows[0]));
	SDL_Log("Screen bpp: %d\n", SDL_BITSPERPIXEL(mode->format));

	init_render_state(msaa);

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
				Render(state->windows[i], i);
			}
		}
	}


	/* Print out some timing information */
	now = SDL_GetTicks();
	if (now > then) {
		SDL_Log("%2.2f frames per second\n",
		((double)frames * 1000) / (now - then));
	}

	quit(0);
	return 0;
}

/* vi: set ts=4 sw=4 expandtab: */