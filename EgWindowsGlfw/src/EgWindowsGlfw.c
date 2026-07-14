#include "EgWindowsGlfw.h"

#include <EgWindows.h>

#include <stdlib.h>
#include <stdio.h>

#include <EgShapes.h>
#include <EgSpatials.h>
#include <EgButtons.h>
#include <EgBase.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

typedef struct
{
	char const *version;
} EgWindowsGlfwState;

typedef struct
{
	ecs_world_t *world;
	ecs_entity_t e_window;
} eg_glfw_userptr_t;

ECS_COMPONENT_DECLARE(EgWindowsGlfwState);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		printf("Escape key pressed. Closing window.\n");
	}
}

static void System_EgWindowsWindowGlfw_Create(ecs_iter_t *it)
{
	static int window_count = 0;
	// EgWindowsGlfwState *s = ecs_field(it, EgWindowsGlfwState, 0);                    // singleton
	EgWindowsWindowCreateInfo *create = ecs_field(it, EgWindowsWindowCreateInfo, 1); // self
	EgShapesRectangle *rect = ecs_field(it, EgShapesRectangle, 2);                   // self
	for (int i = 0; i < it->count; ++i, ++create, ++rect) {
		ecs_assert(window_count < 1, ECS_INTERNAL_ERROR, "Only one window is supported for now.");
		char const *name = ecs_get_name(it->world, it->entities[i]);
		name = name ? name : "EgWindowsWindow";
		GLFWwindow *window = glfwCreateWindow(rect->w, rect->h, name, NULL, NULL);
		if (window == NULL) {
			fprintf(stderr, "Failed to open GLFW window.\n");
			ecs_enable(it->world, it->entities[i], false);
			continue;
		}
		eg_glfw_userptr_t *user_ptr = ecs_os_malloc(sizeof(eg_glfw_userptr_t));
		user_ptr->world = it->world;
		user_ptr->e_window = it->entities[i];
		glfwSetWindowUserPointer(window, user_ptr);
		glfwSetKeyCallback(window, key_callback);
		window_count++;
		ecs_set(it->world, it->entities[i], EgWindowsWindow, {window});

		glfwMakeContextCurrent(window);

		if (!gladLoadGL()) {
			fprintf(stderr, "Failed to initialize glad\n");
			ecs_enable(it->world, it->entities[i], false);
			continue;
		}
		const char *gl_version = (const char *)glGetString(GL_VERSION);
		const char *glsl_version = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
		printf("OpenGL %s, GLSL %s\n", gl_version, glsl_version);

		ecs_set(it->world, it->entities[i], EgWindowsOpenGLContext, {NULL, gl_version, glsl_version});

	} // END FOR LOOP
}

void glfwErrorCallback(int error, const char *description)
{
	fprintf(stderr, "GLFW error occurred. Code: %d. Description: %s\n", error, description);
}

static void System_EgWindowsWindow_Update(ecs_iter_t *it)
{
	// EgWindowsGlfwState *s = ecs_field(it, EgWindowsGlfwState, 0); // singleton
	EgWindowsWindow *cw = ecs_field(it, EgWindowsWindow, 1);     // self
	EgShapesRectangle *cr = ecs_field(it, EgShapesRectangle, 2); // self
	for (int i = 0; i < it->count; ++i, ++cw, ++cr) {
		int width;
		int height;
		glfwGetWindowSize(cw->object, &width, &height);
		cr->w = width;
		cr->h = height;
		if (glfwWindowShouldClose(cw->object)) {
			ecs_add(it->world, it->entities[i], EgWindowsEventCloseRequest);
		}
		glfwSwapBuffers(cw->object);
	}
}

static void System_EgWindowsGlfwState_Update(ecs_iter_t *it)
{
	glfwPollEvents();
}

static void EgWindowsWindow_Remove(ecs_iter_t *it)
{
	EgWindowsWindow *cw = ecs_field(it, EgWindowsWindow, 0); // self
	for (int i = 0; i < it->count; ++i, ++cw) {
		void *usrptr = glfwGetWindowUserPointer(cw->object);
		if (usrptr) {
			ecs_os_free(usrptr);
		}
		glfwDestroyWindow(cw->object);
	}
}

void EgWindowsGlfwImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgWindowsGlfw);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgButtons);
	ecs_set_name_prefix(world, "EgWindowsGlfw");

	ECS_COMPONENT_DEFINE(world, EgWindowsGlfwState);

	ecs_struct(world,
	{.entity = ecs_id(EgWindowsGlfwState),
	.members = {
	{.name = "version", .type = ecs_id(ecs_string_t)},
	}});

	glfwSetErrorCallback(glfwErrorCallback);

	if (glfwInit() == 0) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		return;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// MSAA
	glfwWindowHint(GLFW_SAMPLES, 4);

	ecs_set(world, ecs_id(EgWindows), EgWindowsGlfwState, {glfwGetVersionString()});
	ecs_singleton_set(world, EgButtonsState, {.state = {0}});

	ecs_system(world,
	{.entity = ecs_entity(world, {.name = "System_EgWindowsWindow_Update", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_EgWindowsWindow_Update,
	.query.terms =
	{
	{.id = ecs_id(EgWindowsGlfwState), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn},
	{.id = ecs_id(EgWindowsWindow)},
	{.id = ecs_id(EgShapesRectangle)},
	}});

	ecs_system(world,
	{.entity = ecs_entity(world, {.name = "System_EgWindowsWindowGlfw_Create", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_EgWindowsWindowGlfw_Create,
	.query.terms =
	{
	{.id = ecs_id(EgWindowsGlfwState), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn},
	{.id = ecs_id(EgWindowsWindowCreateInfo), .src.id = EcsSelf},
	{.id = ecs_id(EgShapesRectangle), .src.id = EcsSelf},
	{.id = ecs_id(EgWindowsWindow), .oper = EcsNot}, // Adds this
	}});

	ecs_system(world,
	{.entity = ecs_entity(world, {.name = "System_EgWindowsGlfwState_Update", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_EgWindowsGlfwState_Update,
	.query.terms =
	{
	{.id = ecs_id(EgWindowsGlfwState), .src.id = ecs_id(EgWindows)},
	}});

	ecs_observer(world,
	{.query = {.terms = {{.id = ecs_id(EgWindowsWindow)}}},
	.events = {EcsOnRemove},
	.callback = EgWindowsWindow_Remove});
}
