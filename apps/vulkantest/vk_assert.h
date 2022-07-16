#pragma once

#define VK_ASSERT_RESULT(x,title) (void)(((x) == VK_SUCCESS) || (vk_assert_result(__FILE__, __LINE__, (int)x, title),0) || (__builtin_trap(),0))
void vk_assert_result(char const * file, int line, int result, char const * title);

