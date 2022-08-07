#include "EgVkInstances.h"
#include "EgVk.h"
#include "EgTypes.h"
#include "EgPlatform.h"
#include "EgWindows.h"
#include "EgLogs.h"
#include "EgVkLayers.h"
#include "vk_assert.h"
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


VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != NULL) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

static void Observer_debug(ecs_iter_t *it)
{
	EgVkInstance * d = ecs_field(it, EgVkInstance, 1);
	for (int i = 0; i < it->count; i ++)
	{
		VkInstance instance = d[i].instance;
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugCreateInfo.pfnUserCallback = debugCallback;
		debugCreateInfo.pUserData = it->world;
		VkResult result = CreateDebugUtilsMessengerEXT(instance, &debugCreateInfo, NULL, &(d[i].debug_messenger));
		EG_TRACE(it->world, it->system, "vkCreateDebugUtilsMessengerEXT : %i\n", result);
		VK_ASSERT_RESULT(result, "CreateDebugUtilsMessengerEXT");
	}

}



static void Observer_createInstance1(ecs_iter_t *it)
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
			ecs_set(world, e, EgVkInstance, {instance, NULL});
			ecs_add_pair(world, e, EgState, EgValid);
		}
		else
		{
			EG_TRACE(world, it->system, "vkCreateInstance Error\n");
			ecs_add_pair(world, e, EgState, EgError);
		}

	}
}




void EgVkInstancesImport(ecs_world_t *world)
{
	ECS_MODULE_DEFINE(world, EgVkInstances);
	ECS_IMPORT(world, EgVk);
	ECS_IMPORT(world, EgTypes);
	ECS_IMPORT(world, EgVkLayers);
	ecs_set_name_prefix(world, "EgVk");




	ECS_OBSERVER(world, Observer_createInstance1, EcsOnSet, VkApplicationInfo);
	ECS_OBSERVER(world, Observer_debug, EcsOnSet, EgVkInstance, eg.vk.layers.VK_LAYER_KHRONOS_validation);
	//ECS_OBSERVER(world, Observer_debug, EcsOnSet, EgVkInstance);

	/*
	ecs_observer_init(world, &(ecs_observer_desc_t){
	.filter.terms = {
	{ecs_id(EgVkInstance)},
	{ecs_id(EgVkInstance)},
	},
	.events = {EcsOnAdd},
	.callback = Observer_debug
	});
	*/
}










