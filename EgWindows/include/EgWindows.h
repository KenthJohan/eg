#pragma once
#include <flecs.h>

typedef struct
{
	void *object;
	bool should_close;
} EgWindowsWindow;

typedef struct
{
	bool debug;
} EgWindowsWindowCreateInfo;

typedef struct
{
	char const *gl_version;
	char const *glsl_version;
} EgWindowsOpenGLContext;

typedef struct
{
	int32_t dummy;
} EgWindowsMouse;

extern ECS_COMPONENT_DECLARE(EgWindowsWindow);
extern ECS_COMPONENT_DECLARE(EgWindowsWindowCreateInfo);
extern ECS_COMPONENT_DECLARE(EgWindowsOpenGLContext);
extern ECS_COMPONENT_DECLARE(EgWindowsMouse);
extern ECS_DECLARE(EgWindows);

void EgWindowsImport(ecs_world_t *world);
