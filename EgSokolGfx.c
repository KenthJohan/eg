#include "EgSokolGfx.h"
#include "EgGeometries.h"
#include "EgWindows.h"
#include "EgEvents.h"
#include "EgQuantities.h"
#include "EgResources.h"
#include "EgSokolFetch.h"
#include "eg_basics.h"
#include "sokol_source.h"
#include "libs/stb/stb_image.h"



#define ATTR_vs_pos (0)
#define ATTR_vs_texcoord0 (1)
#define SLOT_tex (0)
#define SLOT_vs_params (0)

#define SOKOL_SHDC_ALIGN(a) __attribute__((aligned(a)))
SOKOL_SHDC_ALIGN(16) typedef struct vs_params_t {
	hmm_mat4 mvp;
} vs_params_t;

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
ECS_COMPONENT_DECLARE(EgSokolGfxConfig);


typedef struct {
	float x, y, z;
	int16_t u, v;
} vertex_t;




static void init(ecs_world_t * world)
{
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

	sg_shader shd = sg_make_shader(&(sg_shader_desc){
	.attrs[0].name = "pos",
	.attrs[1].name = "texcoord0",
	.vs.entry = "main",
	.vs.uniform_blocks[0].size = 64,
	.vs.uniform_blocks[0].layout = SG_UNIFORMLAYOUT_STD140,
	.vs.uniform_blocks[0].uniforms[0].name = "vs_params",
	.vs.uniform_blocks[0].uniforms[0].type = SG_UNIFORMTYPE_FLOAT4,
	.vs.uniform_blocks[0].uniforms[0].array_count = 4,
	.fs.entry = "main",
	.fs.images[0].name = "tex",
	.fs.images[0].image_type = SG_IMAGETYPE_2D,
	.fs.images[0].sampler_type = SG_SAMPLERTYPE_FLOAT,
	.label = "loadpng_shader",
	.vs.source =
	"#version 330\n"
	"uniform vec4 vs_params[4];"
	"layout(location=0) in vec4 pos;\n"
	"layout(location=1) in vec2 texcoord0;\n"
	"out vec2 uv;\n"
	"void main() {\n"
	"  gl_Position = mat4(vs_params[0], vs_params[1], vs_params[2], vs_params[3]) * pos;\n"
	"  uv = texcoord0;\n"
	"}\n",
	.fs.source =
	"#version 330\n"
	"uniform sampler2D tex;"
	"in vec2 uv;"
	"layout(location = 0) out vec4 frag_color;\n"
	"void main() {\n"
	"  frag_color = texture(tex, uv);\n"
	"}\n",
	});

	/* a pipeline state object */
	g_state.pip = sg_make_pipeline(&(sg_pipeline_desc){
	.shader = shd,
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



	{
		ecs_entity_t p = ecs_new(world, 0);
		ecs_set_name(world, p, "Image");
		ecs_set(world, p, EgPath, {"../eg/baboon1.png"});
		ecs_add(world, p, EgImage);
		ecs_add_pair(world, p, EgState, EgUpdate);

		ecs_entity_t e = ecs_new_w_pair(world, EcsChildOf, p);
		ecs_set_name(world, e, "Texture");
		ecs_set(world, e, EgTexture, {g_state.bind.fs_images[SLOT_tex].id, SG_PIXELFORMAT_RGBA8, SG_FILTER_LINEAR, SG_FILTER_LINEAR});
		ecs_add_pair(world, e, EgState, EgUpdate);
	}

}




#define NUM_COLORS (10)
static const sg_color pal[NUM_COLORS] = {
SG_RED, SG_GREEN, SG_BLUE, SG_YELLOW, SG_TURQUOISE,
SG_VIOLET, SG_SILVER, SG_SALMON, SG_PERU, SG_MAGENTA,
};

static const char* names[NUM_COLORS] = {
"RED", "GREEN", "BLUE", "YELLOW", "TURQOISE",
"VIOLET", "SILVER", "SALMON", "PERU", "MAGENTA"
};
static void frame(float w, float h, float duration)
{
	sdtx_canvas(w * 0.5f, h * 0.5f);
	sdtx_origin(3, 3);
	sdtx_color3f(1.0f, 1.0f, 1.0f);
	sdtx_puts("Color names must match\nquad color on same line:\n\n\n");
	for (int i = 0; i < NUM_COLORS; i++)
	{
		sdtx_color3f(pal[i].r, pal[i].g, pal[i].b);
		sdtx_puts(names[i]);
		sdtx_crlf(); sdtx_crlf();
	}

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

	sdtx_draw();
	sg_end_pass();
	sg_commit();
}

static void cleanup(void)
{
	sfetch_shutdown();
	sg_shutdown();
}







static void System_Create(ecs_iter_t *it)
{
	//EG_ITER_INFO(it);
	EgWindow *window = ecs_term(it, EgWindow, 1);
	EgSokolGfxConfig *config = ecs_term(it, EgSokolGfxConfig, 2);
	for (int i = 0; i < it->count; i ++)
	{
		ecs_entity_t e = it->entities[i];
		EgGfx * g = ecs_get_mut(it->world, e, EgGfx);
		memset(g, 0, sizeof(EgGfx));

		// An OpenGLContext need to be created for this window before calling sg_setup.
		EG_TRACE("sg_setup %i", ecs_has(it->world, e, EgOpenGLContext));
		EG_ASSERT(ecs_has_pair(it->world, e, EgOpenGLContext, EgOpenGLContext));
		sg_setup(&(sg_desc) {});

		init(it->world);
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
	ECS_IMPORT(world, EgWindows);
	ECS_IMPORT(world, EgResources);
	ECS_IMPORT(world, EgGeometries);
	ECS_IMPORT(world, EgSokolFetch);

	ecs_set_name_prefix(world, "Eg");
	ECS_ENTITY_DEFINE(world, EgSokolGfxState, 0);
	ecs_add_id(world, EgSokolGfxState, EcsUnion);
	ECS_COMPONENT_DEFINE(world, EgGfx);
	ECS_COMPONENT_DEFINE(world, EgSokolGfxConfig);





	ecs_system_init(world, &(ecs_system_desc_t) {
	.query.filter.expr = "EgWindow, EgSokolGfxConfig, (eg.windows.OpenGLContext, eg.resources.Valid), !EgGfx",
	.entity.add = {ecs_dependson(EcsOnUpdate)},
	.callback = System_Create
	});



	ecs_system_init(world, &(ecs_system_desc_t) {
	.query.filter.expr = "EgWindow, EgRectangleI32, EgGfx, (eg.windows.OpenGLContext, eg.resources.Valid)",
	/*
	.query.filter.terms = {
	{ .id = ecs_id(EgWindow), .inout = EcsIn},
	{ .id = ecs_id(EgRectangleI32), .inout = EcsIn},
	{ .id = ecs_id(EgGfx), .inout = EcsOut}
	},
	*/
	.entity.add = {ecs_dependson(EcsOnUpdate)},
	.callback = System_Update
	});


}

