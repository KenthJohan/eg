#include "EgSokolGfx.h"
#include "EgGeometries.h"
#include "EgWindows.h"
#include "EgBasics.h"
#include "EgEvents.h"
#include "EgQuantities.h"
#include "sokol_source.h"






typedef struct
{
	sg_pipeline pip;
	sg_bindings bind;
	sg_pass_action pass_action;
} EgGfx;

ECS_COMPONENT_DECLARE(EgGfx);




static void grahpics_create(EgGfx * a)
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
	ecs_os_memset(&a->bind, 0, sizeof(sg_bindings));
	a->bind.vertex_buffers[0] = vbuf;

	/* default pass action (clear to grey) */
	ecs_os_memset(&a->pass_action, 0, sizeof(sg_pass_action));
}


static void frame(EgGfx * a, int cur_width, int cur_height)
{
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











static void System_Create(ecs_iter_t *it)
{
	EG_ITER_INFO(it);
	EgWindow *w = ecs_term(it, EgWindow, 1);
	for (int i = 0; i < it->count; i ++)
	{
		ecs_entity_t e = it->entities[i];
		eg_gl_create_context(it->world, e);
		EgGfx * g = ecs_get_mut(it->world, e, EgGfx, NULL);
		grahpics_create(g);
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
		frame(g + i, r[i].width, r[i].height);
		eg_gl_swap_buffer(it->world, e);
	}
}










void EgSokolGfxImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgSokolGfx);
	ECS_COMPONENT_DEFINE(world, EgGfx);

	ECS_SYSTEM(world, System_Create, EcsOnLoad,
	[in]   EgWindow,
	[out] !EgGfx);

	ECS_SYSTEM(world, System_Update, EcsOnUpdate,
	[in]  EgWindow,
	[in]  EgRectangleI32,
	[out] EgGfx);

}

