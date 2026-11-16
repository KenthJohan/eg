#pragma once
#include <flecs.h>
#include <EgWindows.h>

bool EgWindowsSdlGlCleanup(void *window, void *context, void (*destroy)(void *), void *resource);

void EgWindowsSdlGlMakeCurrent(EgWindowsWindow * window, EgWindowsOpenGLContext * context);


void EgWindowsSdlGlImport(ecs_world_t *world);
