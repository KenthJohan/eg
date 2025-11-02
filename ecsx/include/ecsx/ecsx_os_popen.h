#pragma once
#define _GNU_SOURCE // For popen on some systems, must be before flecs include
#include <flecs.h>

int ecsx_os_popen_to_strbuf(const char *command, int *out_exit_code, ecs_strbuf_t *str);
