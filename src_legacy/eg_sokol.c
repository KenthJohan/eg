#include "eg_sokol.h"
#include "eg_geometry.h"
#include "eg_window.h"
#include "eg_base.h"
#include "eg_userevent.h"
#include "eg_quantity.h"

/*
https://github.com/floooh/sokol-samples/blob/c502caae4c21411f3d50e372ffb2897e53eeb635/sapp/noentry-sapp.c

*/
#define SOKOL_NO_ENTRY
#define SOKOL_IMPL
#define SOKOL_GFX_IMPL
#define SOKOL_GLCORE33
#include "sokol/sokol_app.h"
#include "sokol/sokol_gfx.h"
#include "sokol/sokol_glue.h"
#define SOKOL_DEBUGTEXT_IMPL
#include "sokol/util/sokol_debugtext.h"
typedef struct {
	ecs_world_t *world;
	ecs_app_desc_t *desc;
} sokol_app_ctx_t;
static sokol_app_ctx_t sokol_app_ctx;


typedef struct {
	float rx, ry;
	sg_pipeline pip;
	sg_bindings bind;
} app_state_t;





sg_pass_action pass_action;

static void sokol_input_action(const sapp_event* evt, sokol_app_ctx_t *ctx)
{
	ecs_trace("input");
}

void init(void* user_data) {
	app_state_t* state = (app_state_t*) user_data;
	sg_setup(&(sg_desc){
	.context = sapp_sgcontext()
	});

	/* cube vertex buffer */
	float vertices[] = {
	-1.0, -1.0, -1.0,   1.0, 0.5, 0.0, 1.0,
	1.0, -1.0, -1.0,   1.0, 0.5, 0.0, 1.0,
	1.0,  1.0, -1.0,   1.0, 0.5, 0.0, 1.0,
	-1.0,  1.0, -1.0,   1.0, 0.5, 0.0, 1.0,

	-1.0, -1.0,  1.0,   0.5, 1.0, 0.0, 1.0,
	1.0, -1.0,  1.0,   0.5, 1.0, 0.0, 1.0,
	1.0,  1.0,  1.0,   0.5, 1.0, 0.0, 1.0,
	-1.0,  1.0,  1.0,   0.5, 1.0, 0.0, 1.0,

	-1.0, -1.0, -1.0,   0.0, 0.5, 1.0, 1.0,
	-1.0,  1.0, -1.0,   0.0, 0.5, 1.0, 1.0,
	-1.0,  1.0,  1.0,   0.0, 0.5, 1.0, 1.0,
	-1.0, -1.0,  1.0,   0.0, 0.5, 1.0, 1.0,

	1.0, -1.0, -1.0,    1.0, 0.5, 0.5, 1.0,
	1.0,  1.0, -1.0,    1.0, 0.5, 0.5, 1.0,
	1.0,  1.0,  1.0,    1.0, 0.5, 0.5, 1.0,
	1.0, -1.0,  1.0,    1.0, 0.5, 0.5, 1.0,

	-1.0, -1.0, -1.0,   0.5, 0.5, 1.0, 1.0,
	-1.0, -1.0,  1.0,   0.5, 0.5, 1.0, 1.0,
	1.0, -1.0,  1.0,   0.5, 0.5, 1.0, 1.0,
	1.0, -1.0, -1.0,   0.5, 0.5, 1.0, 1.0,

	-1.0,  1.0, -1.0,   0.5, 1.0, 0.5, 1.0,
	-1.0,  1.0,  1.0,   0.5, 1.0, 0.5, 1.0,
	1.0,  1.0,  1.0,   0.5, 1.0, 0.5, 1.0,
	1.0,  1.0, -1.0,   0.5, 1.0, 0.5, 1.0
	};
	sg_buffer vbuf = sg_make_buffer(&(sg_buffer_desc){
	.data = SG_RANGE(vertices)
	});

	/* create an index buffer for the cube */
	uint16_t indices[] = {
	0, 1, 2,  0, 2, 3,
	6, 5, 4,  7, 6, 4,
	8, 9, 10,  8, 10, 11,
	14, 13, 12,  15, 14, 12,
	16, 17, 18,  16, 18, 19,
	22, 21, 20,  23, 22, 20
	};
	sg_buffer ibuf = sg_make_buffer(&(sg_buffer_desc){
	.type = SG_BUFFERTYPE_INDEXBUFFER,
	.data = SG_RANGE(indices)
	});

	/* create shader */
	sg_shader shd = sg_make_shader(noentry_shader_desc(sg_query_backend()));

	/* create pipeline object */
	state->pip = sg_make_pipeline(&(sg_pipeline_desc){
	.layout = {
	/* test to provide buffer stride, but no attr offsets */
	.buffers[0].stride = 28,
	.attrs = {
	[ATTR_vs_position].format = SG_VERTEXFORMAT_FLOAT3,
	[ATTR_vs_color0].format = SG_VERTEXFORMAT_FLOAT4
	}
	},
	.shader = shd,
	.index_type = SG_INDEXTYPE_UINT16,
	.cull_mode = SG_CULLMODE_BACK,
	.depth = {
	.compare = SG_COMPAREFUNC_LESS_EQUAL,
	.write_enabled = true,
	},
	});

	/* setup resource bindings */
	state->bind = (sg_bindings) {
	.vertex_buffers[0] = vbuf,
	.index_buffer = ibuf
};
}

static void sokol_frame_action(void)
{
	float g = pass_action.colors[0].value.g + 0.01f;
	pass_action.colors[0].value.g = (g > 1.0f) ? 0.0f : g;
	sg_begin_default_pass(&pass_action, sapp_width(), sapp_height());
	sg_end_pass();
	sg_commit();
}

static void cleanup(void)
{
	sg_shutdown();
}

static int sokol_run_action(ecs_world_t *world,ecs_app_desc_t *desc)
{
	sokol_app_ctx = ((sokol_app_ctx_t)
	{
	.world = world,
	.desc = desc
	});

	char * title = "test";
	int width = 800;
	int height = 600;
	sapp_run(&(sapp_desc)
	{
	.frame_userdata_cb = (void(*)(void*))sokol_frame_action,
	.event_userdata_cb = (void(*)(const sapp_event*, void*))sokol_input_action,
	.user_data = &sokol_app_ctx,
	.window_title = title,
	.width = width,
	.height = height,
	.sample_count = 1,
	.high_dpi = true,
	.gl_force_gles2 = false
	});
}


void FlecsComponentsEgSokolImport(ecs_world_t *world)
{
	ECS_MODULE(world, FlecsComponentsEgSokol);
	ecs_app_set_run_action(sokol_run_action);
}

