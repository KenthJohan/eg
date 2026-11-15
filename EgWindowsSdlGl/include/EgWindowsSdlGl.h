#pragma once
#include <flecs.h>

bool EgWindowsSdlGlCleanup(void *window, void *context, void (*destroy)(void *), void *resource);
void EgWindowsSdlGlImport(ecs_world_t *world);
