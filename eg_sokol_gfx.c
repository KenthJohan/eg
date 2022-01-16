#include "eg_sokol_gfx.h"
#include "eg_geometry.h"
#include "eg_window.h"
#include "eg_base.h"
#include "eg_userevent.h"
#include "eg_quantity.h"

#define SOKOL_IMPL
#define SOKOL_GLCORE33
#include "sokol/sokol_gfx.h"






typedef struct
{
	sg_pipeline pip;
	sg_bindings bind;
	sg_pass_action pass_action;
} grahpics_t;

ECS_COMPONENT_DECLARE(grahpics_t);




static void grahpics_create(grahpics_t * a)
{
	/* setup sokol_gfx */
	sg_setup(&(sg_desc){0});

	/* a vertex buffer */
	const float vertices[] = {
	// positions            // colors
	0.0f,  0.5f, 0.5f,     1.0f, 0.0f, 0.0f, 1.0f,
	0.5f, -0.5f, 0.5f,     0.0f, 1.0f, 0.0f, 1.0f,
	-0.5f, -0.5f, 0.5f,     0.0f, 0.0f, 1.0f, 1.0f
	};
	sg_buffer vbuf = sg_make_buffer(&(sg_buffer_desc){
	.data = SG_RANGE(vertices)
	});

	/* a shader */
	sg_shader shd = sg_make_shader(&(sg_shader_desc){
	.vs.source =
	"#version 330\n"
	"layout(location=0) in vec4 position;\n"
	"layout(location=1) in vec4 color0;\n"
	"out vec4 color;\n"
	"void main() {\n"
	"  gl_Position = position;\n"
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

	/* a pipeline state object (default render states are fine for triangle) */
	a->pip = sg_make_pipeline(&(sg_pipeline_desc){
	.shader = shd,
	.layout = {
	.attrs = {
	[0].format=SG_VERTEXFORMAT_FLOAT3,
	[1].format=SG_VERTEXFORMAT_FLOAT4
	}
	}
	});

	/* resource bindings */
	a->bind.vertex_buffers[0] = vbuf;

	/* default pass action (clear to grey) */
	ecs_os_memset(&a->pass_action, 0, sizeof(sg_pass_action));
}


static void frame(grahpics_t * a)
{
	int cur_width, cur_height;
	//glfwGetFramebufferSize(w, &cur_width, &cur_height);
	sg_begin_default_pass(&a->pass_action, cur_width, cur_height);
	sg_apply_pipeline(a->pip);
	sg_apply_bindings(&a->bind);
	sg_draw(0, 3, 1);
	sg_end_pass();
	sg_commit();
	//glfwSwapBuffers(w);
	//glfwPollEvents();
}











static void Create(ecs_iter_t *it)
{
	EG_ITER_INFO(it);
	EgWindow *w = ecs_term(it, EgWindow, 1);
	for (int i = 0; i < it->count; i ++)
	{
		ecs_entity_t e = it->entities[i];
		eg_gl_create_context(w + i);
		ecs_add(it->world, e, grahpics_t);
	}
}

static void Create1(ecs_iter_t *it)
{
	EG_ITER_INFO(it);
	grahpics_t *w = ecs_term(it, grahpics_t, 1);
	for (int i = 0; i < it->count; i ++)
	{
		grahpics_create(w + i);
	}
}



static void Update(ecs_iter_t *it)
{
	EG_ITER_INFO(it);
	EgWindow *w = ecs_term(it, EgWindow, 1);
	grahpics_t *g = ecs_term(it, grahpics_t, 2);
	for (int i = 0; i < it->count; i ++)
	{
		eg_gl_make_current(w + i);
		frame(g + i);
		eg_gl_swap_buffer(w + i);
	}
}










void FlecsComponentsEgSokolGfxImport(ecs_world_t *world)
{
	ECS_MODULE(world, FlecsComponentsEgSokolGfx);


	ECS_COMPONENT_DEFINE(world, grahpics_t);

	ECS_TRIGGER(world, Create, EcsOnSet, EgWindow);
	ECS_TRIGGER(world, Create1, EcsOnSet, grahpics_t);
	ECS_SYSTEM(world, Update, EcsOnUpdate,
	[in]  EgWindow,
	[out] grahpics_t);

}

