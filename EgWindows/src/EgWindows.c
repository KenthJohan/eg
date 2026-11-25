#include "EgWindows.h"

#include <stdlib.h>
#include <stdio.h>

#include <EgShapes.h>
#include <EgSpatials.h>
#include <EgBase.h>

ECS_COMPONENT_DECLARE(EgWindowsWindow);
ECS_COMPONENT_DECLARE(EgWindowsInputState);
ECS_COMPONENT_DECLARE(EgWindowsFrameCallback);
ECS_COMPONENT_DECLARE(EgWindowsFrameExecutor);
ECS_COMPONENT_DECLARE(EgWindowsNativeOwner);
ECS_COMPONENT_DECLARE(EgWindowsWindowCreateInfo);
ECS_COMPONENT_DECLARE(EgWindowsOpenGLContext);
ECS_COMPONENT_DECLARE(EgWindowsOpenGLContextCreate);
ECS_COMPONENT_DECLARE(EgWindowsMouse);
ECS_TAG_DECLARE(EgWindowsEventResize);
ECS_TAG_DECLARE(EgWindowsCloseRequest);
ECS_DECLARE(EgWindows);

void EgWindowsImport(ecs_world_t *world)
{
	ECS_MODULE_DEFINE(world, EgWindows);
	ecs_set_name_prefix(world, "EgWindows");

	ECS_COMPONENT_DEFINE(world, EgWindowsWindow);
	ECS_COMPONENT_DEFINE(world, EgWindowsInputState);
	ECS_COMPONENT_DEFINE(world, EgWindowsFrameCallback);
	ECS_COMPONENT_DEFINE(world, EgWindowsFrameExecutor);
	ECS_COMPONENT_DEFINE(world, EgWindowsNativeOwner);
	ECS_COMPONENT_DEFINE(world, EgWindowsWindowCreateInfo);
	ECS_COMPONENT_DEFINE(world, EgWindowsOpenGLContext);
	ECS_COMPONENT_DEFINE(world, EgWindowsOpenGLContextCreate);
	ECS_COMPONENT_DEFINE(world, EgWindowsMouse);
	ECS_TAG_DEFINE(world, EgWindowsEventResize);
	ECS_TAG_DEFINE(world, EgWindowsCloseRequest);

	ecs_struct(world,
	{.entity = ecs_id(EgWindowsWindow),
	.members = {
	{.name = "object", .type = ecs_id(ecs_uptr_t)},
	{.name = "terminate_on_close", .type = ecs_id(ecs_bool_t)},
	{.name = "width", .type = ecs_id(ecs_i32_t)},
	{.name = "height", .type = ecs_id(ecs_i32_t)},
	{.name = "pixel_width", .type = ecs_id(ecs_i32_t)},
	{.name = "pixel_height", .type = ecs_id(ecs_i32_t)},
	{.name = "mouse_x", .type = ecs_id(ecs_f32_t)},
	{.name = "mouse_y", .type = ecs_id(ecs_f32_t)},
	{.name = "mouse_focus", .type = ecs_id(ecs_bool_t)},
	{.name = "keyboard_focus", .type = ecs_id(ecs_bool_t)},
	{.name = "hidden", .type = ecs_id(ecs_bool_t)},
	{.name = "minimized", .type = ecs_id(ecs_bool_t)},
	{.name = "valid", .type = ecs_id(ecs_bool_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgWindowsWindowCreateInfo),
	.members = {
	{.name = "debug", .type = ecs_id(ecs_bool_t)},
	{.name = "enable_opengl", .type = ecs_id(ecs_bool_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgWindowsOpenGLContext),
	.members = {
	{.name = "gl_context", .type = ecs_id(ecs_uptr_t)},
	{.name = "gl_version", .type = ecs_id(ecs_string_t)},
	{.name = "glsl_version", .type = ecs_id(ecs_string_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgWindowsOpenGLContextCreate),
	.members = {
	{.name = "dummy", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgWindowsMouse),
	.members = {
	{.name = "dummy", .type = ecs_id(ecs_i32_t)},
	}});
}
