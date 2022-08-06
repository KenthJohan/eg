#include "EgVkInstances.h"
#include "EgVk.h"
#include "EgTypes.h"
#include "EgPlatform.h"
#include "EgWindows.h"
#include "EgLogs.h"
#include "eg_util.h"
#include <stdio.h>


ECS_COMPONENT_DECLARE(EgVkInstances);


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback
(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* data, void* user)
{
	char const * s = "NONE";
	if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {s = "\033[36;1;4m" "VERBOSE" "\033[0m";}
	if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)    {s = "\033[32;1;4m" "INFO"    "\033[0m";}
	if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {s = "\033[33;1;4m" "WARNING" "\033[0m";}
	if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)   {s = "\033[31;1;4m" "ERROR"   "\033[0m";}
	EG_TRACE(user, ecs_id(EgVkInstances), "[%s] %s\n", s, data->pMessage);
	//std::cerr << "validation layer: " << data->pMessage << std::endl;
	return VK_FALSE;
}


static void System_createInstance1(ecs_iter_t *it)
{
	ecs_world_t * world = it->world;
	char const * extension_names[128];
	char const * layer_names[128];

	VkApplicationInfo *field_app = ecs_field(it, VkApplicationInfo, 1);
	for (int i = 0; i < it->count; i ++)
	{
		ecs_entity_t e = it->entities[i];
		ecs_doc_set_name(world, e, field_app[i].pApplicationName);

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugCreateInfo.pfnUserCallback = debugCallback;
		debugCreateInfo.pUserData = it->world;

		VkInstanceCreateInfo create = {};
		create.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create.pApplicationInfo = field_app + i;
		create.enabledExtensionCount = 0;
		create.enabledLayerCount = 0;
		create.ppEnabledExtensionNames = extension_names;
		create.ppEnabledLayerNames = layer_names;
		create.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;

		{
			const ecs_type_t *type = ecs_get_type(world, e);
			for(int i = 0; i < type->count; ++i)
			{
				if (ecs_is_valid(world, type->array[i]) == false) {continue;}
				if (ecs_has(world, type->array[i], EgVkExtension))
				{
					extension_names[create.enabledExtensionCount] = ecs_get_name(world, type->array[i]);
					create.enabledExtensionCount++;
				}
				else if (ecs_has(world, type->array[i], EgVkLayer))
				{
					layer_names[create.enabledLayerCount] = ecs_get_name(world, type->array[i]);
					create.enabledLayerCount++;
				}
			}
		}

		VkInstance instance;

		if (vkCreateInstance(&create, NULL, &instance) == VK_SUCCESS)
		{
			EG_TRACE(world, it->system, "vkCreateInstance -> %p OK\n", instance);
			ecs_set(world, e, EgVkInstance, {instance});
			ecs_add_pair(world, e, EgState, EgValid);
		}
		else
		{
			EG_TRACE(world, it->system, "vkCreateInstance Error\n");
			ecs_add_pair(world, e, EgState, EgError);
		}

	}
}


void createInstance1(ecs_world_t * world, ecs_entity_t windowe)
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	{
		ecs_entity_t r = ecs_lookup_fullpath(world, "eg.vk.layers.VK_LAYER_KHRONOS_validation");
		ecs_add_id(world, windowe, r);
	}

	{
		ecs_entity_t r = ecs_lookup_fullpath(world, "eg.vk.instance.extensions."VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		ecs_add_id(world, windowe, r);
	}



	{
		//ecs_entity_t r = ecs_lookup_fullpath(world, "eg.vk.extensions."VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		//ecs_add_id(world, windowe, r);
	}

	{
		ecs_filter_t f = ECS_FILTER_INIT;
		ecs_filter_init(world, &(ecs_filter_desc_t){
		.storage = &f,
		.terms = {
		{ ecs_id(EgVkRequiredExtension) },
		}
		});
		eg_add_all_from_filter(world, windowe, &f);
		ecs_filter_fini(&f);
	}

	//while(1) ecs_progress(world, 0);

	ecs_set_ptr(world, windowe, VkApplicationInfo, &appInfo);

}

void EgVkInstancesImport(ecs_world_t *world)
{
	ECS_MODULE_DEFINE(world, EgVkInstances);
	ECS_IMPORT(world, EgVk);
	ECS_IMPORT(world, EgTypes);
	ecs_set_name_prefix(world, "EgVk");




	ECS_OBSERVER(world, System_createInstance1, EcsOnSet, VkApplicationInfo);
}










