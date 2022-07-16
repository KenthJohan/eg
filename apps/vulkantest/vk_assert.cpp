#include "vk_assert.h"
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

void vk_assert_result(char const * file, int line, int result, char const * title)
{
	char const * rmsg = string_VkResult((VkResult)result);
	fprintf(stderr, "%s:%i: %s: %s(%i)\n", file, line, title, rmsg, result);
}

