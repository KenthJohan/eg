#include "platform.h"
#include "flecs.h"
#include "EgTypes.h"
#include "EgVk.h"
#include <GLFW/glfw3.h>

void platform_get_required_extension_names(ecs_world_t * world, ecs_entity_t e)
{
	uint32_t count = 0;
	const char** x = glfwGetRequiredInstanceExtensions(&count);
	for(uint32_t i = 0; i < count; ++i)
	{
		char const * name = x[i];
		ecs_entity_t r = ecs_entity_init(world, &(ecs_entity_desc_t)
		{
		.name = name,
		.add = {EgVkExtension, EgVkRequiredExtension}
		});
		ecs_add_id(world, e, r);
	}


}
