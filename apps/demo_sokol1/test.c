#define SOKOL_GLCORE33
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_gp.h"
#include "sokol_glue.h"
#include "sokol_log.h"
#include "sokol_fetch.h"
#include "flecs.h"
#include "HandmadeMath.h"
#include "stb_image.h"
#include "loadpng-sapp.glsl.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

const char* fileutil_get_path(const char* filename, char* buf, size_t buf_size) {
	snprintf(buf, buf_size, "%s", filename);
	return buf;
}


static struct {
	float rx, ry;
	sg_pass_action pass_action;
	sg_pipeline pip;
	sg_bindings bind;
	uint8_t file_buffer[256 * 1024];
} state;


typedef struct {
	float x, y, z;
	int16_t u, v;
} vertex_t;


static void fetch_callback(const sfetch_response_t* response) {
	if (response->fetched) {
		/* the file data has been fetched, since we provided a big-enough
		   buffer we can be sure that all data has been loaded here
		*/
		int png_width, png_height, num_channels;
		const int desired_channels = 4;
		stbi_uc* pixels = stbi_load_from_memory(
			response->data.ptr,
			(int)response->data.size,
			&png_width, &png_height,
			&num_channels, desired_channels);
		if (pixels) {
			/* ok, time to actually initialize the sokol-gfx texture */
			sg_init_image(state.bind.fs_images[SLOT_tex], &(sg_image_desc){
				.width = png_width,
				.height = png_height,
				.pixel_format = SG_PIXELFORMAT_RGBA8,
				.min_filter = SG_FILTER_LINEAR,
				.mag_filter = SG_FILTER_LINEAR,
				.data.subimage[0][0] = {
					.ptr = pixels,
					.size = (size_t)(png_width * png_height * 4),
				}
			});
			stbi_image_free(pixels);
		}
	}
	else if (response->failed) {
		// if loading the file failed, set clear color to red
		state.pass_action = (sg_pass_action) {
			.colors[0] = { .action = SG_ACTION_CLEAR, .value = { 1.0f, 0.0f, 0.0f, 1.0f } }
		};
	}
}


void frame(ecs_world_t *world)
{
	assert(world);
	ecs_progress(world, 0);
	/* pump the sokol-fetch message queues, and invoke response callbacks */
	sfetch_dowork();

	/* compute model-view-projection matrix for vertex shader */
	const float t = (float)(sapp_frame_duration() * 60.0);
	hmm_mat4 proj = HMM_Perspective(60.0f, sapp_widthf()/sapp_heightf(), 0.01f, 10.0f);
	hmm_mat4 view = HMM_LookAt(HMM_Vec3(0.0f, 1.5f, 6.0f), HMM_Vec3(0.0f, 0.0f, 0.0f), HMM_Vec3(0.0f, 1.0f, 0.0f));
	hmm_mat4 view_proj = HMM_MultiplyMat4(proj, view);
	vs_params_t vs_params;
	state.rx += 1.0f * t; state.ry += 2.0f * t;
	hmm_mat4 rxm = HMM_Rotate(state.rx, HMM_Vec3(1.0f, 0.0f, 0.0f));
	hmm_mat4 rym = HMM_Rotate(state.ry, HMM_Vec3(0.0f, 1.0f, 0.0f));
	hmm_mat4 model = HMM_MultiplyMat4(rxm, rym);
	vs_params.mvp = HMM_MultiplyMat4(view_proj, model);

	sg_begin_default_pass(&state.pass_action, sapp_width(), sapp_height());
	sg_apply_pipeline(state.pip);
	sg_apply_bindings(&state.bind);
	sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &SG_RANGE(vs_params));
	sg_draw(0, 36, 1);
	sg_end_pass();
	sg_commit();
}


void init(ecs_world_t *world)
{
	assert(world);
   /* setup sokol-gfx and the optional debug-ui*/
	sg_setup(&(sg_desc){
		.context = sapp_sgcontext(),
		.logger.func = slog_func,
	});
	/* setup sokol-fetch with the minimal "resource limits" */
	sfetch_setup(&(sfetch_desc_t){
		.max_requests = 1,
		.num_channels = 1,
		.num_lanes = 1,
		.logger.func = slog_func,
	});

	/* pass action for clearing the framebuffer to some color */
	state.pass_action = (sg_pass_action) {
		.colors[0] = { .action = SG_ACTION_CLEAR, .value = { 0.125f, 0.25f, 0.35f, 1.0f } }
	};

	/* Allocate an image handle, but don't actually initialize the image yet,
	   this happens later when the asynchronous file load has finished.
	   Any draw calls containing such an "incomplete" image handle
	   will be silently dropped.
	*/
	state.bind.fs_images[SLOT_tex] = sg_alloc_image();

	/* cube vertex buffer with packed texcoords */
	const vertex_t vertices[] = {
		/* pos                  uvs */
		{ -1.0f, -1.0f, -1.0f,      0,     0 },
		{  1.0f, -1.0f, -1.0f,  32767,     0 },
		{  1.0f,  1.0f, -1.0f,  32767, 32767 },
		{ -1.0f,  1.0f, -1.0f,      0, 32767 },

		{ -1.0f, -1.0f,  1.0f,      0,     0 },
		{  1.0f, -1.0f,  1.0f,  32767,     0 },
		{  1.0f,  1.0f,  1.0f,  32767, 32767 },
		{ -1.0f,  1.0f,  1.0f,      0, 32767 },

		{ -1.0f, -1.0f, -1.0f,      0,     0 },
		{ -1.0f,  1.0f, -1.0f,  32767,     0 },
		{ -1.0f,  1.0f,  1.0f,  32767, 32767 },
		{ -1.0f, -1.0f,  1.0f,      0, 32767 },

		{  1.0f, -1.0f, -1.0f,      0,     0 },
		{  1.0f,  1.0f, -1.0f,  32767,     0 },
		{  1.0f,  1.0f,  1.0f,  32767, 32767 },
		{  1.0f, -1.0f,  1.0f,      0, 32767 },

		{ -1.0f, -1.0f, -1.0f,      0,     0 },
		{ -1.0f, -1.0f,  1.0f,  32767,     0 },
		{  1.0f, -1.0f,  1.0f,  32767, 32767 },
		{  1.0f, -1.0f, -1.0f,      0, 32767 },

		{ -1.0f,  1.0f, -1.0f,      0,     0 },
		{ -1.0f,  1.0f,  1.0f,  32767,     0 },
		{  1.0f,  1.0f,  1.0f,  32767, 32767 },
		{  1.0f,  1.0f, -1.0f,      0, 32767 },
	};
	state.bind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
		.data = SG_RANGE(vertices),
		.label = "cube-vertices"
	});

	/* create an index buffer for the cube */
	const uint16_t indices[] = {
		0, 1, 2,  0, 2, 3,
		6, 5, 4,  7, 6, 4,
		8, 9, 10,  8, 10, 11,
		14, 13, 12,  15, 14, 12,
		16, 17, 18,  16, 18, 19,
		22, 21, 20,  23, 22, 20
	};
	state.bind.index_buffer = sg_make_buffer(&(sg_buffer_desc){
		.type = SG_BUFFERTYPE_INDEXBUFFER,
		.data = SG_RANGE(indices),
		.label = "cube-indices"
	});
	
	/* a pipeline state object */
	state.pip = sg_make_pipeline(&(sg_pipeline_desc){
		.shader = sg_make_shader(loadpng_shader_desc(sg_query_backend())),
		.layout = {
			.attrs = {
				[ATTR_vs_pos].format = SG_VERTEXFORMAT_FLOAT3,
				[ATTR_vs_texcoord0].format = SG_VERTEXFORMAT_SHORT2N
			}
		},
		.index_type = SG_INDEXTYPE_UINT16,
		.cull_mode = SG_CULLMODE_BACK,
		.depth = {
			.compare = SG_COMPAREFUNC_LESS_EQUAL,
			.write_enabled = true
		},
		.label = "cube-pipeline"
	});

	/* start loading the PNG file, we don't need the returned handle since
	   we can also get that inside the fetch-callback from the response
	   structure.
		- NOTE that we're not using the user_data member, since all required
		  state is in a global variable anyway
	*/
	char path_buf[512];
	sfetch_send(&(sfetch_request_t){
		.path = fileutil_get_path("../../content/baboon.png", path_buf, sizeof(path_buf)),
		.callback = fetch_callback,
		.buffer = SFETCH_RANGE(state.file_buffer)
	});
}


void cleanup(ecs_world_t *world)
{
	assert(world);
	sfetch_shutdown();
	sg_shutdown();
}



