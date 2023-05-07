#include "EgPlatform.h"
#include "flecs.h"
#include "EgTypes.h"
#include "EgVk.h"
#include "vk_assert.h"
#include "EgGeometries.h"
#include "EgWindows.h"
#include "EgVkInstances.h"
#include "eg_basics.h"
#include "EgLogs.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


typedef struct
{
	GLFWwindow * window;
} EgPlatformWindow;


ECS_COMPONENT_DECLARE(EgPlatformWindow);

ECS_DECLARE(EgPlatformLogVerbose);
ECS_DECLARE(EgPlatformLogInfo);
ECS_DECLARE(EgPlatformLogWarning);
ECS_DECLARE(EgPlatformLogError);



void platform_populate_required_extension_names(ecs_world_t * world)
{
	uint32_t count = 0;
	const char** x = glfwGetRequiredInstanceExtensions(&count);
	EG_ASSERT(x);
	for(uint32_t i = 0; i < count; ++i)
	{
		char const * name = x[i];
		ecs_entity_t r = ecs_entity_init(world, &(ecs_entity_desc_t)
		{
		.name = name,
		.add = {EgVkExtension, EgVkInstanceExtension, EgVkRequiredExtension}
		});
	}
}


void platform_create_surface(ecs_world_t * world, ecs_entity_t e)
{
/*
	VkResult result = glfwCreateWindowSurface(instance, window, NULL, &surface);
	VK_ASSERT_RESULT(result, "glfwCreateWindowSurface");
	*/
}

static void Observer_Window(ecs_iter_t *it)
{
	EgWindow *window = ecs_field(it, EgWindow, 1);
	for (int i = 0; i < it->count; i ++)
	{
		ecs_add(it->world, it->entities[i], EgPlatformWindow);
		ecs_add(it->world, it->entities[i], EgRectangleI32);
	}
}

/*
typedef struct
{
	ecs_world_t * world;
	ecs_entity_t entity;
} eg_world_entity_t;

static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	eg_world_entity_t * we = glfwGetWindowUserPointer(window);
	EgWindow * w = ecs_get_mut(we->world, we->entity, EgWindow);
	w->should_recreate_swapchain = true;
}
*/

static void Observer_Window1(ecs_iter_t *it)
{
	EgPlatformWindow *window = ecs_field(it, EgPlatformWindow, 1);
	for (int i = 0; i < it->count; i ++)
	{
		GLFWwindow * w = glfwCreateWindow(200, 100, "Vulkan", NULL, NULL);
		EG_EVENT_STRF(it->world, EgPlatformLogVerbose, "glfwCreateWindow : %p\n", w);
		window[i].window = w;
		ecs_add_pair(it->world, it->entities[i], EgState, EgValid);
		//eg_world_entity_t * we = ecs_os_malloc_t(eg_world_entity_t);
		//we->world = it->world;
		//we->entity = it->entities[i];
		//glfwSetWindowUserPointer(w, we);
		//glfwSetFramebufferSizeCallback(w, framebufferResizeCallback);
	}
}

static void Observer_Surface(ecs_iter_t *it)
{
	EgVkInstance *field_instance = ecs_field(it, EgVkInstance, 1);
	EgPlatformWindow *field_window = ecs_field(it, EgPlatformWindow, 2);
	for (int i = 0; i < it->count; i ++)
	{
		GLFWwindow * window = field_window[i].window;
		VkInstance instance = field_instance[i].instance;
		VkSurfaceKHR surface;
		VkResult result = glfwCreateWindowSurface(instance, window, NULL, &surface);
		EG_EVENT_STRF(it->world, EgPlatformLogVerbose, "glfwCreateWindowSurface %p %p : %i\n", window, instance, result);
		VK_ASSERT_RESULT(result, "glfwCreateWindowSurface");
		ecs_set(it->world, it->entities[i], EgVkSurfaceKHR, {surface});
	}
}

static void System_Window_Size(ecs_iter_t *it)
{
	EgPlatformWindow *field_pwindow = ecs_field(it, EgPlatformWindow, 1);
	EgRectangleI32 *field_rectangle = ecs_field(it, EgRectangleI32, 2);
	EgWindow *field_window = ecs_field(it, EgWindow, 3);
	for (int i = 0; i < it->count; i ++)
	{
		GLFWwindow * pwindow = field_pwindow[i].window;
		int width = 0;
		int height = 0;
		glfwGetFramebufferSize(pwindow, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(pwindow, &width, &height);
			glfwWaitEvents();
		}
		if ((field_rectangle[i].width != width) || (field_rectangle[i].height != height))
		{
			field_window[i].should_recreate_swapchain = true;
		}
		field_rectangle[i].width = width;
		field_rectangle[i].height = height;
	}
}



void eg_platform_wait_positive_framebuffer_size(ecs_world_t *world, ecs_entity_t e)
{
	GLFWwindow * window = ecs_get(world, e, EgPlatformWindow)->window;
	int width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	EG_EVENT_STRF(world, EgPlatformLogVerbose, "glfwGetFramebufferSize : %i %i\n", width, height);
}


void eg_platform_update()
{
	glfwPollEvents();
};


void EgPlatformImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgPlatform);
	ECS_IMPORT(world, EgVk);
	ECS_IMPORT(world, EgVkInstances);
	ECS_IMPORT(world, EgGeometries);
	ECS_IMPORT(world, EgWindows);
	ECS_IMPORT(world, EgTypes);
	ecs_set_name_prefix(world, "EgPlatform");
	ECS_COMPONENT_DEFINE(world, EgPlatformWindow);

	ECS_TAG_DEFINE(world, EgPlatformLogError);
	ECS_TAG_DEFINE(world, EgPlatformLogVerbose);
	ECS_TAG_DEFINE(world, EgPlatformLogInfo);
	ECS_TAG_DEFINE(world, EgPlatformLogWarning);

	ecs_doc_set_color(world, EgPlatformLogError,   EG_LOGS_COLOR_ERROR);
	ecs_doc_set_color(world, EgPlatformLogVerbose, EG_LOGS_COLOR_VERBOSE);
	ecs_doc_set_color(world, EgPlatformLogInfo,    EG_LOGS_COLOR_INFO);
	ecs_doc_set_color(world, EgPlatformLogWarning, EG_LOGS_COLOR_WARNING);

	ecs_add(world, EgPlatformLogError,    EgEventsIdling);
	ecs_add(world, EgPlatformLogError,    EgEventsPrint);
	ecs_add(world, EgPlatformLogVerbose,  EgEventsPrint);
	ecs_add(world, EgPlatformLogInfo,     EgEventsPrint);
	ecs_add(world, EgPlatformLogWarning,  EgEventsPrint);

	glfwInit();
	EG_EVENT_STRF(world, EgPlatformLogVerbose, "glfwInit\n");

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	EG_EVENT_STRF(world, EgPlatformLogVerbose, "glfwWindowHint\n");

	ECS_OBSERVER(world, Observer_Window, EcsOnAdd, EgWindow);
	ECS_OBSERVER(world, Observer_Window1, EcsOnAdd, EgPlatformWindow);
	ECS_OBSERVER(world, Observer_Surface, EcsOnSet, EgVkInstance, EgPlatformWindow);
	ECS_SYSTEM(world, System_Window_Size, EcsOnUpdate, EgPlatformWindow, EgRectangleI32, EgWindow);


}