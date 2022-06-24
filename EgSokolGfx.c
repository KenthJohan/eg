#include "EgSokolGfx.h"
#include "EgGeometries.h"
#include "EgWindows.h"
#include "EgEvents.h"
#include "EgQuantities.h"
#include "eg_basics.h"



#include "sokol_source.h"

#define HANDMADE_MATH_IMPLEMENTATION
#define HANDMADE_MATH_NO_SSE
#include "HandmadeMath.h"
#include "libs/stb/stb_image.h"
#include "libs/dbgui/dbgui.h"
#include "libs/util/fileutil.h"



#include "loadpng-sapp.glsl.h"




typedef struct
{
	float rx, ry;
	sg_pass_action pass_action;
	sg_pipeline pip;
	sg_bindings bind;
	uint8_t file_buffer[256 * 1024];
} EgGfx;
static EgGfx g_state = {};


ECS_COMPONENT_DECLARE(EgGfx);



typedef struct {
	float x, y, z;
	int16_t u, v;
} vertex_t;



static void fetch_callback(const sfetch_response_t*);



static void init()
{
	sg_setup(&(sg_desc) {
	});
	/* setup sokol-gfx and the optional debug-ui*/
	/*
	sg_setup(&(sg_desc){
	.context = sapp_sgcontext()
	});
	__dbgui_setup(sapp_sample_count());
	*/

	/* setup sokol-fetch with the minimal "resource limits" */
	sfetch_setup(&(sfetch_desc_t){
	.max_requests = 1,
	.num_channels = 1,
	.num_lanes = 1
	});

	/* pass action for clearing the framebuffer to some color */
	g_state.pass_action = ((sg_pass_action){.colors[0] = { .action = SG_ACTION_CLEAR, .value = { 0.125f, 0.25f, 0.35f, 1.0f } }});
	g_state.bind.fs_images[SLOT_tex] = sg_alloc_image();

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
	g_state.bind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
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
	g_state.bind.index_buffer = sg_make_buffer(&(sg_buffer_desc){
	.type = SG_BUFFERTYPE_INDEXBUFFER,
	.data = SG_RANGE(indices),
	.label = "cube-indices"
	});

	/* a pipeline state object */
	g_state.pip = sg_make_pipeline(&(sg_pipeline_desc){
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
	.path = fileutil_get_path("../eg/baboon.png", path_buf, sizeof(path_buf)),
	.callback = fetch_callback,
	.buffer_ptr = g_state.file_buffer,
	.buffer_size = sizeof(g_state.file_buffer),
	});
}

/* The fetch-callback is called by sokol_fetch.h when the data is loaded,
   or when an error has occurred.
*/

static void fetch_callback(const sfetch_response_t* response)
{
	if (response->fetched)
	{
		/* the file data has been fetched, since we provided a big-enough
		   buffer we can be sure that all data has been loaded here
		*/
		int png_width = 0;
		int png_height = 0;
		int num_channels = 0;
		const int desired_channels = 4;
		stbi_uc* pixels = stbi_load_from_memory(
		response->buffer_ptr,
		(int)response->fetched_size,
		&png_width, &png_height,
		&num_channels, desired_channels);
		if (pixels)
		{
			/* ok, time to actually initialize the sokol-gfx texture */
			sg_init_image(g_state.bind.fs_images[SLOT_tex], &(sg_image_desc){
			.width = png_width,
			.height = png_height,
			.pixel_format = SG_PIXELFORMAT_RGBA8,
			.min_filter = SG_FILTER_LINEAR,
			.mag_filter = SG_FILTER_LINEAR,
			.data.subimage[0][0] =
			{
			.ptr = pixels,
			.size = (size_t)(png_width * png_height * 4),
			}
			});
			stbi_image_free(pixels);
		}
	}
	else if (response->failed)
	{
		// if loading the file failed, set clear color to red
		g_state.pass_action = ((sg_pass_action) {.colors[0] = { .action = SG_ACTION_CLEAR, .value = { 1.0f, 0.0f, 0.0f, 1.0f } }});
	}
}

/* The frame-function is fairly boring, note that no special handling is
   needed for the case where the texture isn't loaded yet.
   Also note the sfetch_dowork() function, this is usually called once a
   frame to pump the sokol-fetch message queues.
*/
static void frame(float w, float h, float duration)
{
	/* pump the sokol-fetch message queues, and invoke response callbacks */
	sfetch_dowork();

	/* compute model-view-projection matrix for vertex shader */
	const float t = (float)(duration * 0.0001f);
	hmm_mat4 proj = HMM_Perspective(60.0f, w/h, 0.01f, 10.0f);
	hmm_mat4 view = HMM_LookAt(HMM_Vec3(0.0f, 1.5f, 6.0f), HMM_Vec3(0.0f, 0.0f, 0.0f), HMM_Vec3(0.0f, 1.0f, 0.0f));
	hmm_mat4 view_proj = HMM_MultiplyMat4(proj, view);
	vs_params_t vs_params;
	g_state.rx += 1.0f * t;
	g_state.ry += 2.0f * t;
	hmm_mat4 rxm = HMM_Rotate(g_state.rx, HMM_Vec3(1.0f, 0.0f, 0.0f));
	hmm_mat4 rym = HMM_Rotate(g_state.ry, HMM_Vec3(0.0f, 1.0f, 0.0f));
	hmm_mat4 model = HMM_MultiplyMat4(rxm, rym);
	vs_params.mvp = HMM_MultiplyMat4(view_proj, model);

	sg_begin_default_pass(&g_state.pass_action, w, h);
	sg_apply_pipeline(g_state.pip);
	sg_apply_bindings(&(g_state.bind));
	sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &SG_RANGE(vs_params));
	sg_draw(0, 36, 1);
	__dbgui_draw();
	sg_end_pass();
	sg_commit();
}

static void cleanup(void)
{
	__dbgui_shutdown();
	sfetch_shutdown();
	sg_shutdown();
}







static void System_Create(ecs_iter_t *it)
{
	EG_ITER_INFO(it);
	EgWindow *w = ecs_term(it, EgWindow, 1);
	for (int i = 0; i < it->count; i ++)
	{
		ecs_entity_t e = it->entities[i];
		eg_gl_create_context(it->world, e);
		EgGfx * g = ecs_get_mut(it->world, e, EgGfx, NULL);
		memset(g, 0, sizeof(EgGfx));
		init();
	}
}



static void System_Update(ecs_iter_t *it)
{
	//EG_ITER_INFO(it);
	EgWindow *w = ecs_term(it, EgWindow, 1);
	EgRectangleI32 *r = ecs_term(it, EgRectangleI32, 2);
	EgGfx *g = ecs_term(it, EgGfx, 3);
	for (int i = 0; i < it->count; i ++)
	{
		ecs_entity_t e = it->entities[i];
		eg_gl_make_current(it->world, e);
		frame(r[i].width, r[i].height, w->counter);
		eg_gl_swap_buffer(it->world, e);
	}
}










void EgSokolGfxImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgSokolGfx);
	ECS_COMPONENT_DEFINE(world, EgGfx);

	ecs_system_init(world, &(ecs_system_desc_t) {
	.query.filter.terms = {
	{ .id = ecs_id(EgWindow), .inout = EcsIn},
	{ .id = ecs_id(EgGfx), .inout = EcsOut, .oper = EcsNot}
	},
	.entity.add = {EcsOnLoad},
	.callback = System_Create
	});

	ecs_system_init(world, &(ecs_system_desc_t) {
	.query.filter.terms = {
	{ .id = ecs_id(EgWindow), .inout = EcsIn},
	{ .id = ecs_id(EgRectangleI32), .inout = EcsIn},
	{ .id = ecs_id(EgGfx), .inout = EcsOut}
	},
	.entity.add = {EcsOnUpdate},
	.callback = System_Update
	});


}

