#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#define HANDMADE_MATH_IMPLEMENTATION
#define HANDMADE_MATH_NO_SSE
#include "../HandmadeMath.h"

#include "../sokol/sokol_gfx.h"
#include "../sokol/sokol_glue.h"
#include "../sokol/util/sokol_debugtext.h"
#include "debugtext-context-sapp.glsl.h"

/* different clear colors for 3 windows */
sg_pass_action pass_actions[3] = {
[0] = {
.colors[0] = { .action=SG_ACTION_CLEAR, .value={ 0.5f, 0.5f, 1.0f, 1.0f } }
},
[1] = {
.colors[0] = { .action=SG_ACTION_CLEAR, .value={ 1.0f, 0.5f, 0.5f, 1.0f } }
},
[2] = {
.colors[0] = { .action=SG_ACTION_CLEAR, .value={ 0.5f, 1.0f, 0.5f, 1.0f } }
}
};

/* vertex shader params */
typedef struct {
	hmm_mat4 mvp;
} params_t;

#define WINDOW_COUNT 2



int any_notnull(void ** objects, int n)
{
	for (int i = 0; i < n; i++)
	{
		if (objects[i] != NULL){return 1;}
	}
	return 0;
}


int main(int argc, char **argv)
{
	/* setup GLFW and create a couple of windows each with its own GL context behind */
	const int WIDTH = 512;
	const int HEIGHT = 384;

	(void)argc, (void)argv;
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}


	SDL_Window *windows[WINDOW_COUNT];
	SDL_GLContext contexts[WINDOW_COUNT];
	for(int i = 0; i < WINDOW_COUNT; ++i)
	{
		char title[32];
		snprintf(title, sizeof(title), "window%d", i);
		windows[i] = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_OPENGL);
		if(!windows[i])
		{
			fprintf(stderr, "SDL_CreateWindow error: %s\n", SDL_GetError());
			return 1;
		}
		contexts[i] = SDL_GL_CreateContext(windows[i]);
		if(!contexts[i])
		{
			fprintf(stderr, "SDL_GL_CreateContext error: %s\n", SDL_GetError());
			return 1;
		}
	}

	/* setup sokol-gfx */
	sg_setup(&(sg_desc) {0});

	/* cube vertices and indices */
	static const float vertices[] = {
	-1.0, -1.0, -1.0,   1.0, 0.0, 0.0, 1.0,
	1.0, -1.0, -1.0,   1.0, 0.0, 0.0, 1.0,
	1.0,  1.0, -1.0,   1.0, 0.0, 0.0, 1.0,
	-1.0,  1.0, -1.0,   1.0, 0.0, 0.0, 1.0,

	-1.0, -1.0,  1.0,   0.0, 1.0, 0.0, 1.0,
	1.0, -1.0,  1.0,   0.0, 1.0, 0.0, 1.0,
	1.0,  1.0,  1.0,   0.0, 1.0, 0.0, 1.0,
	-1.0,  1.0,  1.0,   0.0, 1.0, 0.0, 1.0,

	-1.0, -1.0, -1.0,   0.0, 0.0, 1.0, 1.0,
	-1.0,  1.0, -1.0,   0.0, 0.0, 1.0, 1.0,
	-1.0,  1.0,  1.0,   0.0, 0.0, 1.0, 1.0,
	-1.0, -1.0,  1.0,   0.0, 0.0, 1.0, 1.0,

	1.0, -1.0, -1.0,    1.0, 0.5, 0.0, 1.0,
	1.0,  1.0, -1.0,    1.0, 0.5, 0.0, 1.0,
	1.0,  1.0,  1.0,    1.0, 0.5, 0.0, 1.0,
	1.0, -1.0,  1.0,    1.0, 0.5, 0.0, 1.0,

	-1.0, -1.0, -1.0,   0.0, 0.5, 1.0, 1.0,
	-1.0, -1.0,  1.0,   0.0, 0.5, 1.0, 1.0,
	1.0, -1.0,  1.0,   0.0, 0.5, 1.0, 1.0,
	1.0, -1.0, -1.0,   0.0, 0.5, 1.0, 1.0,

	-1.0,  1.0, -1.0,   1.0, 0.0, 0.5, 1.0,
	-1.0,  1.0,  1.0,   1.0, 0.0, 0.5, 1.0,
	1.0,  1.0,  1.0,   1.0, 0.0, 0.5, 1.0,
	1.0,  1.0, -1.0,   1.0, 0.0, 0.5, 1.0
	};
	static const uint16_t indices[] = {
	0, 1, 2,  0, 2, 3,
	6, 5, 4,  7, 6, 4,
	8, 9, 10,  8, 10, 11,
	14, 13, 12,  15, 14, 12,
	16, 17, 18,  16, 18, 19,
	22, 21, 20,  23, 22, 20
	};

	/* create resources per GL context */
	sg_context ctx[3];
	sg_bindings bind[3] = { {0} };
	sg_pipeline pip[3] = { {0} };
	for (int i = 0; i < 3; i++)
	{
		/* for each GL context, setup a sokol context */
		SDL_GL_MakeCurrent(windows[1], contexts[1]);
		ctx[i] = sg_setup_context();
		/* create the usual resources per context (buffers, shader, pipeline) */
		bind[i].vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
		.data = SG_RANGE(vertices)
		});
		bind[i].index_buffer = sg_make_buffer(&(sg_buffer_desc){
		.type = SG_BUFFERTYPE_INDEXBUFFER,
		.data = SG_RANGE(indices)
		});
		sg_shader shd = sg_make_shader(&(sg_shader_desc){
		.vs.uniform_blocks[0] = {
		.size = sizeof(params_t),
		.uniforms = {
		[0] = { .name="mvp", .type=SG_UNIFORMTYPE_MAT4 },
		}
		},
		.vs.source =
		"#version 330\n"
		"uniform mat4 mvp;\n"
		"layout(location=0) in vec4 position;\n"
		"layout(location=1) in vec4 color0;\n"
		"out vec4 color;\n"
		"void main() {\n"
		"  gl_Position = mvp * position;\n"
		"  color = color0;\n"
		"}\n",
		.fs.source =
		"#version 330\n"
		"in vec4 color;\n"
		"out vec4 frag_color;\n"
		"void main() {\n"
		"  frag_color = color;\n"
		"}\n"
		});
		pip[i] = sg_make_pipeline(&(sg_pipeline_desc){
		.layout = {
		/* test to provide buffer stride, but no attr offsets */
		.buffers[0].stride = 28,
		.attrs = {
		[0].format=SG_VERTEXFORMAT_FLOAT3,
		[1].format=SG_VERTEXFORMAT_FLOAT4
		}
		},
		.shader = shd,
		.index_type = SG_INDEXTYPE_UINT16,
		.depth = {
		.compare = SG_COMPAREFUNC_LESS_EQUAL,
		.write_enabled = true,
		},
		.cull_mode = SG_CULLMODE_BACK,
		});
	}

	/* view-projection matrix */
	hmm_mat4 proj = HMM_Perspective(60.0f, (float)WIDTH/(float)HEIGHT, 0.01f, 10.0f);
	hmm_mat4 view = HMM_LookAt(HMM_Vec3(0.0f, 1.5f, 6.0f), HMM_Vec3(0.0f, 0.0f, 0.0f), HMM_Vec3(0.0f, 1.0f, 0.0f));
	hmm_mat4 view_proj = HMM_MultiplyMat4(proj, view);

	/* run until all windows are closed */
	params_t vs_params;
	float rx = 0.0f, ry = 0.0f;
	while (any_notnull((void**)windows, WINDOW_COUNT))
	{


		SDL_Event ev;
		while(SDL_PollEvent(&ev))
		{
			if(ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_q)
			{
				break;
			}
		}


		/*
		if (ev.type == SDL_WINDOWEVENT && ev.window.event == SDL_WINDOWEVENT_CLOSE)
		{
			if (SDL_GetWindowID(myWindowA) == ev.window.windowID)
			{
			}
		}
		*/

		/* rotated model matrix */
		rx += 1.0f; ry += 2.0f;
		hmm_mat4 rxm = HMM_Rotate(rx, HMM_Vec3(1.0f, 0.0f, 0.0f));
		hmm_mat4 rym = HMM_Rotate(ry, HMM_Vec3(0.0f, 1.0f, 0.0f));
		hmm_mat4 model = HMM_MultiplyMat4(rxm, rym);
		vs_params.mvp = HMM_MultiplyMat4(view_proj, model);

		for (int i = 0; i < 3; i++)
		{
			if (windows[i])
			{
				/* switch GL context and sokol context */
				SDL_GL_MakeCurrent(windows[i], contexts[i]);
				sg_activate_context(ctx[i]);

				/* do the usual pass rendering */
				int w;
				int h;
				SDL_GetWindowSize(windows[i], &w, &h);
				sg_begin_default_pass(&pass_actions[i], w, h);
				sg_apply_pipeline(pip[i]);
				sg_apply_bindings(&bind[i]);
				sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &SG_RANGE(vs_params));
				sg_draw(0, 36, 1);
				sg_end_pass();
				sg_commit();

				SDL_GL_SwapWindow(windows[1]);
				/*
				if (glfwWindowShouldClose(w[i]))
				{
					sg_discard_context(ctx[i]);
					SDL_GL_DeleteContext(contexts[i]);
					SDL_DestroyWindow(windows[i]);
					w[i] = 0;
				}
				*/
			}
		}
	}
	sg_shutdown();
	return 0;
}
